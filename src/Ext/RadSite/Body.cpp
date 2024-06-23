#include "Body.h"

#include <Ext/WarheadType/Body.h>

#include <New/Type/RadTypeClass.h>
#include <LightSourceClass.h>
#include <Utilities/Macro.h>
#include <Notifications.h>

void RadSiteExtData::InvalidatePointer(AbstractClass* ptr, bool bRemoved)
{
	AnnounceInvalidPointer(TechOwner, ptr , bRemoved);
	AnnounceInvalidPointer(HouseOwner, ptr);
}

void RadSiteExtData::CreateInstance(CoordStruct const& nCoord, int spread, int amount, WeaponTypeExtData* pWeaponExt, TechnoClass* const pTech)
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
		pRadExt->Type = RadTypeClass::Array[0].get();
	}

	if (pTech && pRadExt->Type->GetHasInvoker() && !pRadExt->NoOwner && pRadExt->Type->GetHasOwner())
	{
		pRadExt->HouseOwner = pTech->GetOwningHouse();
		pRadExt->TechOwner = pTech;
	}

	pRadExt->CreationFrame = Unsorted::CurrentFrame;
	pRadExt->AttachedToObject->BaseCell = CellClass::Coord2Cell(nCoord);
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
	//	Debug::Log("RadSite [%s] CreateLight With Color [%d , %d , %d] \n", Type->Name.data(), nRadcolor.R, nRadcolor.G, nRadcolor.B);

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

	if (distance && nMax)
		radLevel = (distance > nMax) ? 0.0 : (nMax - distance) / nMax * pThis->RadLevel;


	// Vanilla YR stores & updates the decremented RadLevel on CellClass.
	// Because we're not storing multiple radiation site data on CellClass (yet?)
	// we need to fully recalculate this stuff every time we need the radiation level for a cell coord - Starkku
	int stepCount = (Unsorted::CurrentFrame - this->CreationFrame) / this->Type->GetLevelDelay();
	radLevel -= (radLevel / pThis->LevelSteps) * stepCount;

	return radLevel;
}

//return false mean it is already death
const RadSiteExtData::DamagingState RadSiteExtData::ApplyRadiationDamage(TechnoClass* pTarget, int damage, int distance)
{
	const auto pWarhead = this->Type->GetWarhead();
	if (!pTarget->IsAlive || pTarget->InLimbo || !pTarget->Health || pTarget->IsSinking || pTarget->IsCrashing)
		return RadSiteExtData::DamagingState::Dead;

	auto const pUnit = specific_cast<UnitClass*>(pTarget);

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
		.Process(this->Weapon)
		.Process(this->Type)
		.Process(this->TechOwner)
		.Process(this->HouseOwner)
		.Process(this->NoOwner)
		.Process(this->CreationFrame)
		;
}

// =============================
// container
RadSiteExtContainer RadSiteExtContainer::Instance;

// =============================
// container hooks

DEFINE_HOOK(0x65B243, RadSiteClass_CTOR, 0x6)
{
	if (!Phobos::Otamaa::DisableCustomRadSite)
	{
		GET(RadSiteClass*, pThis, ESI);
		RadSiteExtContainer::Instance.Allocate(pThis);
		PointerExpiredNotification::NotifyInvalidObject->Add(pThis);
	}

	return 0;
}

DEFINE_HOOK(0x65B344, RadSiteClass_DTOR, 0x6)
{
	if (!Phobos::Otamaa::DisableCustomRadSite)
	{
		GET(RadSiteClass*, pThis, ESI);
		PointerExpiredNotification::NotifyInvalidObject->Remove(pThis);
		RadSiteExtContainer::Instance.Remove(pThis);
	}

	return 0;
}

DEFINE_HOOK_AGAIN(0x65B3D0, RadSiteClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x65B450, RadSiteClass_SaveLoad_Prefix, 0x8)
{
	if (!Phobos::Otamaa::DisableCustomRadSite)
	{
		GET_STACK(RadSiteClass*, pItem, 0x4);
		GET_STACK(IStream*, pStm, 0x8);

		RadSiteExtContainer::Instance.PrepareStream(pItem, pStm);
	}

	return 0;
}

// Before :
 // DEFINE_HOOK(0x65B43F, RadSiteClass_Load_Suffix, 0x7)
DEFINE_HOOK(0x65B431 , RadSiteClass_Load_Suffix , 0x9)
{
	GET(RadSiteClass*, pThis, ESI);

	SwizzleManagerClass::Instance->Swizzle((void**)&pThis->LightSource);

	if (!Phobos::Otamaa::DisableCustomRadSite)
		RadSiteExtContainer::Instance.LoadStatic();

	//return 0;
	return 0x65B43F;
}

DEFINE_HOOK(0x65B464, RadSiteClass_Save_Suffix, 0x5)
{
	GET(const HRESULT , nRes, EAX);

	if(SUCCEEDED(nRes)) {
		if (!Phobos::Otamaa::DisableCustomRadSite) {
			RadSiteExtContainer::Instance.SaveStatic();
		}
	}

	return 0;
}

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

static void __fastcall RadSiteClass_Detach(RadSiteClass* pThis, void* _, AbstractClass* pTarget, bool bRemove)
{
	if (!Phobos::Otamaa::DisableCustomRadSite){
		RadSiteExtContainer::Instance.InvalidatePointerFor(pThis, pTarget, bRemove);
	}
}

DEFINE_JUMP(VTABLE, 0x7F0838, GET_OFFSET(RadSiteClass_Detach));

static HouseClass* __fastcall RadSiteClass_OwningHouse(RadSiteClass* pThis, void* _)
{
	if (!Phobos::Otamaa::DisableCustomRadSite){
		if (const auto pExt = RadSiteExtContainer::Instance.Find(pThis)){
			return pExt->HouseOwner;
		}
	}

	return nullptr;
}

DEFINE_JUMP(VTABLE, 0x7F084C, GET_OFFSET(RadSiteClass_OwningHouse));