#include "Body.h"

#include <Ext/WarheadType/Body.h>
#include <Ext/Cell/Body.h>

#include <New/Type/RadTypeClass.h>
#include <LightSourceClass.h>
#include <Utilities/Macro.h>
#include <Notifications.h>

void RadSiteExtData::InvalidatePointer(AbstractClass* ptr, bool bRemoved)
{
	AnnounceInvalidPointer(TechOwner, ptr , bRemoved);
	AnnounceInvalidPointer(HouseOwner, ptr);
	
	damageCounts.erase((BuildingClass*)ptr);
}

void RadSiteExtData::CreateInstance(CellClass* pCell , int spread, int amount, WeaponTypeExtData* pWeaponExt, TechnoClass* const pTech)
{
	// use real ctor
	const auto pRadExt = RadSiteExtContainer::Instance.Find(GameCreate<RadSiteClass>());

	//Adding Owner to RadSite, from bullet
	if (pWeaponExt)
	{
		pRadExt->Weapon = pWeaponExt->AttachedToObject;
		pRadExt->Type = pWeaponExt->RadType.Get(0);
		pRadExt->NoOwner = pWeaponExt->Rad_NoOwner.Get();
	}
	else
	{
		pRadExt->Type = RadTypeClass::Array.begin()->get();
	}

	if (pTech && pRadExt->Type->GetHasInvoker() && !pRadExt->NoOwner && pRadExt->Type->GetHasOwner())
	{
		pRadExt->HouseOwner = pTech->GetOwningHouse();
		pRadExt->TechOwner = pTech;
	}

	pRadExt->CreationFrame = Unsorted::CurrentFrame;
	CellExtContainer::Instance.Find(pCell)->RadSites.push_back(pRadExt->AttachedToObject);
	pRadExt->AttachedToObject->BaseCell = pCell->MapCoords;
	pRadExt->AttachedToObject->SetSpread(spread);
	pRadExt->SetRadLevel(amount);
	pRadExt->CreateLight();
}

//RadSiteClass Activate , Rewritten
void RadSiteExtData::CreateLight()
{
	const auto pThis = this->AttachedToObject;
	const auto nLevelDelay = this->Type->GetLevelDelay();
	const auto nLightDelay = this->Type->GetLightDelay();
	const auto nRadcolor = this->Type->GetColor();

	//if(Phobos::Otamaa::IsAdmin)
	//	Debug::LogInfo("RadSite [%s] CreateLight With Color [%d , %d , %d] ", Type->Name.data(), nRadcolor.R, nRadcolor.G, nRadcolor.B);

	const auto nTintFactor = this->Type->GetTintFactor();
	const auto nRadLevelFactor = pThis->RadLevel * this->Type->GetLightFactor();
	const auto nLightFactor = std::clamp(nRadLevelFactor , 1.0 , 2000.0 );
	const auto nDuration = (double)pThis->RadDuration;

	pThis->RadLevelTimer.Start(nLevelDelay);
	pThis->RadLightTimer.Start(nLightDelay);
	pThis->Intensity = int(nLightFactor);
	pThis->LevelSteps = int(nDuration / (double)nLevelDelay);
	pThis->IntensitySteps = int(nDuration / (double)nLightDelay);
	pThis->IntensityDecrement = int(nLightFactor / (nDuration / (double)nLightDelay));
	const TintStruct nTintBuffer { nRadcolor  , nTintFactor };

	pThis->Tint = nTintBuffer;

	if (pThis->LightSource)
	{
		pThis->LightSource->ChangeLevels(static_cast<int>(nLightFactor), nTintBuffer, false);
		pThis->Radiate();
	}
	else
	{
		auto const pCell = MapClass::Instance->GetCellAt(pThis->BaseCell);
		{
			auto const pLight = GameCreate<LightSourceClass>(pCell->GetCoordsWithBridge(), pThis->SpreadInLeptons, static_cast<int>(nLightFactor), nTintBuffer);
			pThis->LightSource = pLight;
			pLight->DetailLevel = 0;
			pLight->Activate(false);
			pThis->Radiate();
		}
	}
}

// Rewrite because of crashing craziness
void RadSiteExtData::Add(int amount)
{
	const auto pThis = this->AttachedToObject;
	pThis->Deactivate();
	const auto nInput = int(double(pThis->RadLevel * pThis->RadTimeLeft) / (double)pThis->RadDuration) + amount;
	pThis->RadLevel = nInput;
	const auto nInput_2 = nInput * this->Type->GetDurationMultiple();
	pThis->RadDuration = nInput_2;
	pThis->RadTimeLeft = nInput_2;
	this->CreateLight();
	this->CreationFrame = Unsorted::CurrentFrame;
}

void RadSiteExtData::SetRadLevel(int amount)
{
	const auto pThis = this->AttachedToObject;
	const auto nMax = this->Type->GetLevelMax();
	const auto nDecidedamount = MinImpl(amount,  nMax);
	const int mult = this->Type->GetDurationMultiple();
	pThis->RadLevel = nDecidedamount;
	pThis->RadDuration = mult * nDecidedamount;
	pThis->RadTimeLeft = mult * nDecidedamount;
}

// helper function provided by AlexB
const double RadSiteExtData::GetRadLevelAt(CellStruct const& cell)
{
	return this->GetRadLevelAt(this->AttachedToObject->BaseCell.DistanceFrom(cell));
}

bool NOINLINE IsFiniteNumber(double x) {
	return (x <= DBL_MAX && x >= -DBL_MAX);
}

const double RadSiteExtData::GetRadLevelAt(double distance)
{
	const auto pThis = this->AttachedToObject;
	const auto nMax = static_cast<double>(pThis->Spread);
	double radLevel = pThis->RadLevel;

	//  will produce `-nan(ind)` result if both dist and max is zero
	// and used on formula below this check
	// ,.. -Otamaa
	if (distance && nMax) {

		//distance is too far
		if (distance > nMax) {
			return 0.0;
		}
		else {
			radLevel = (nMax - distance) / nMax * pThis->RadLevel;
		}
	}

	// Vanilla YR stores & updates the decremented RadLevel on CellClass.
	// Because we're not storing multiple radiation site data on CellClass (yet?)
	// we need to fully recalculate this stuff every time we need the radiation level for a cell coord - Starkku
	const auto frame_Step = (Unsorted::CurrentFrame - this->CreationFrame);
	const int stepCount = frame_Step ? frame_Step / this->Type->GetLevelDelay() : 0;

	if(radLevel && pThis->LevelSteps)
		radLevel -= (radLevel / pThis->LevelSteps) * stepCount;

	return radLevel;
}

//return false mean it is already death
const RadSiteExtData::DamagingState RadSiteExtData::ApplyRadiationDamage(TechnoClass* pTarget, int damage, int distance)
{
	const auto pWarhead = this->Type->GetWarhead();
	if (!pTarget->IsAlive || pTarget->InLimbo || !pTarget->Health || pTarget->IsSinking || pTarget->IsCrashing)
		return RadSiteExtData::DamagingState::Dead;

	auto const pUnit = cast_to<UnitClass*, false>(pTarget);

	if ((pUnit && pUnit->DeathFrameCounter > 0))
		return RadSiteExtData::DamagingState::Ignore;

	{
		if (!this->Type->GetWarheadDetonate())
		{
			HouseClass* const pOwner = this->TechOwner ? this->TechOwner->Owner : this->HouseOwner;
			const auto result = pTarget->ReceiveDamage(&damage, distance, pWarhead, this->TechOwner, false, true, pOwner);

			if (result != DamageState::NowDead && result != DamageState::PostMortem)
				return RadSiteExtData::DamagingState::Continue;
			else if (result == DamageState::PostMortem)
				return RadSiteExtData::DamagingState::Ignore;
		}
		else
		{
			auto const coords = pTarget->GetCoords();
			HouseClass* const pOwner = this->TechOwner ? this->TechOwner->Owner : this->HouseOwner;
			WarheadTypeExtData::DetonateAt(pWarhead, pTarget, coords , this->TechOwner, damage , pOwner);

			if ((pUnit && pUnit->DeathFrameCounter > 0))
				return RadSiteExtData::DamagingState::Ignore;
		}
	}

	const auto res =  pTarget->IsAlive && !pTarget->InLimbo && pTarget->Health > 0 && !pTarget->IsSinking && !pTarget->IsCrashing;

	return res ? RadSiteExtData::DamagingState::Continue : RadSiteExtData::DamagingState::Dead;
}

// =============================
// load / save

template <typename T>
void RadSiteExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)
		.Process(this->Weapon, true)
		.Process(this->Type, true)
		.Process(this->TechOwner, true)
		.Process(this->HouseOwner, true)
		.Process(this->NoOwner)
		.Process(this->CreationFrame)
		.Process(this->damageCounts)
		;
}

// =============================
// container
RadSiteExtContainer RadSiteExtContainer::Instance;

// =============================
// container hooks

DEFINE_HOOK(0x65B243, RadSiteClass_CTOR, 0x6)
{

	GET(RadSiteClass*, pThis, ESI);
	RadSiteExtContainer::Instance.Allocate(pThis);

	return 0;
}

DEFINE_HOOK(0x65B344, RadSiteClass_DTOR, 0x6)
{
	GET(RadSiteClass*, pThis, ESI);
	const auto pBaseCell = MapClass::Instance->TryGetCellAt(pThis->BaseCell);

	if (pBaseCell) {
		CellExtContainer::Instance.Find(pBaseCell)->RadSites.remove(pThis);
	}

	for (CellRangeEnumerator it(pThis->BaseCell, pThis->Spread + 0.5); it; it++) {
		if (const auto pCell = MapClass::Instance->TryGetCellAt(*it)) {
			CellExtContainer::Instance.Find(pCell)->RadLevels.remove_all_if([pThis](auto& level) { return level.Rad == pThis; });
		}
	}

	RadSiteExtContainer::Instance.Remove(pThis);

	return 0;
}

#include <Misc/Hooks.Otamaa.h>

HRESULT __stdcall FakeRadSiteClass::_Load(IStream* pStm)
{

	RadSiteExtContainer::Instance.PrepareStream(this, pStm);
	HRESULT res = this->RadSiteClass::Load(pStm);

	if (SUCCEEDED(res))
		RadSiteExtContainer::Instance.LoadStatic();

	return res;
}

HRESULT __stdcall FakeRadSiteClass::_Save(IStream* pStm, bool clearDirty)
{

	RadSiteExtContainer::Instance.PrepareStream(this, pStm);
	HRESULT res = this->RadSiteClass::Save(pStm, clearDirty);

	if (SUCCEEDED(res))
		RadSiteExtContainer::Instance.SaveStatic();

	return res;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F0824, FakeRadSiteClass::_Load)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F0828, FakeRadSiteClass::_Save)

//#ifdef AAENABLE_NEWHOOKS
//DEFINE_HOOK(0x65B4B0, RadSiteClass_GetSpread_Replace, 0x4)
//{
//	GET(RadSiteClass*, pThis, ECX);
//	if (!Phobos::Otamaa::DisableCustomRadSite)
//	{
//		R->EAX(RadSiteExtContainer::Instance.Find(pThis)->Spread);
//		return 0x65B4B3;
//	}
//	return 0x0;
//}
//
//DEFINE_HOOK_AGAIN(0x65BD14, RadSiteClass_Spread_Replace, 0x5)
//DEFINE_HOOK(0x65B9D4, RadSiteClass_Spread_Replace, 0x5)
//{
//	GET(RadSiteClass*, pThis, ECX);
//	if (!Phobos::Otamaa::DisableCustomRadSite)
//	{
//		auto nSpread = RadSiteExtContainer::Instance.Find(pThis)->Spread;
//		R->ESI(nSpread);
//		R->EDX(R->EDX<int>() - nSpread);
//		return R->Origin() == 0x65B9D4 ? 0x65B9D9 : 0x65BD19;
//	}
//	return 0x0;
//}
//
//DEFINE_HOOK(0x65B4D4, RadSiteClass_SetSpread, 0x7)
//{
//	GET(RadSiteClass*, pThis, ECX);
//	GET_STACK(int, spread, 0x4);
//	if (!Phobos::Otamaa::DisableCustomRadSite)
//	{
//		RadSiteExtContainer::Instance.Find(pThis)->Spread = spread;
//		pThis->SpreadInLeptons = (spread << 8) + 128;
//		return 0x65B4E2;
//	}
//
//	return 0x0;
//}
//#endif

void FakeRadSiteClass::_Detach(AbstractClass* pTarget, bool bRemove)
{
	RadSiteExtContainer::Instance.InvalidatePointerFor(this, pTarget, bRemove);
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F0838, FakeRadSiteClass::_Detach);

HouseClass* FakeRadSiteClass::_GetOwningHouse()
{
	return RadSiteExtContainer::Instance.Find(this)->HouseOwner;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F084C, FakeRadSiteClass::_GetOwningHouse);