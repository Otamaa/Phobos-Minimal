#include "Body.h"

#include <Ext/WarheadType/Body.h>

#include <New/Type/RadTypeClass.h>
#include <LightSourceClass.h>
#include <Utilities/Macro.h>
#include <Notifications.h>

void RadSiteExt::ExtData::InvalidatePointer(AbstractClass* ptr, bool bRemoved)
{
	AnnounceInvalidPointer(TechOwner, ptr , bRemoved);
	AnnounceInvalidPointer(HouseOwner, ptr);
}

void RadSiteExt::CreateInstance(CoordStruct const& nCoord, int spread, int amount, WeaponTypeExt::ExtData* pWeaponExt, TechnoClass* const pTech)
{
	// use real ctor
	const auto pRadExt = RadSiteExt::ExtMap.TryFind(GameCreate<RadSiteClass>());

	if (!pRadExt)
		Debug::FatalErrorAndExit("Uneable To find Ext for Radsite ! \n");

	//Adding Owner to RadSite, from bullet
	if (pWeaponExt)
	{
		pRadExt->Weapon = pWeaponExt->Get();
		pRadExt->Type = pWeaponExt->RadType.Get(RadTypeClass::Find(RADIATION_SECTION));
		pRadExt->NoOwner = pWeaponExt->Rad_NoOwner.Get();
	}
	else
	{
		pRadExt->Type = RadTypeClass::Find(RADIATION_SECTION);
	}

	if (pTech && pRadExt->Type->GetHasInvoker() && !pRadExt->NoOwner && pRadExt->Type->GetHasOwner())
	{
		pRadExt->HouseOwner = pTech->GetOwningHouse();
		pRadExt->TechOwner = pTech;
	}

	pRadExt->Get()->BaseCell = CellClass::Coord2Cell(nCoord);
	pRadExt->Get()->SetSpread(spread);
	pRadExt->SetRadLevel(amount);
	pRadExt->CreateLight();

}

//RadSiteClass Activate , Rewritten
void RadSiteExt::ExtData::CreateLight()
{
	const auto pThis = this->Get();
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
		if (auto const pLight = GameCreate<LightSourceClass>(pCell->GetCoordsWithBridge(), pThis->SpreadInLeptons, static_cast<int>(nLightFactor), nTintBuffer))
		{
			pThis->LightSource = pLight;
			pLight->DetailLevel = 0;
			pLight->Activate(false);
			pThis->Radiate();
		}
	}
}

// Rewrite because of crashing craziness
void RadSiteExt::ExtData::Add(int amount)
{
	const auto pThis = this->Get();
	pThis->Deactivate();
	const auto nInput = int(double(pThis->RadLevel * pThis->RadTimeLeft) / (double)pThis->RadDuration) + amount;
	pThis->RadLevel = nInput;
	const auto nInput_2 = nInput * this->Type->GetDurationMultiple();
	pThis->RadDuration = nInput_2;
	pThis->RadTimeLeft = nInput_2;
	this->CreateLight();
}

void RadSiteExt::ExtData::SetRadLevel(int amount)
{
	const auto pThis = this->Get();
	const auto nMax = this->Type->GetLevelMax();
	const auto nDecidedamount = MinImpl(amount,  nMax);
	const int mult = this->Type->GetDurationMultiple();
	pThis->RadLevel = nDecidedamount;
	pThis->RadDuration = mult * nDecidedamount;
	pThis->RadTimeLeft = mult * nDecidedamount;
}

// helper function provided by AlexB
const double RadSiteExt::ExtData::GetRadLevelAt(CellStruct const& cell)
{
	const auto pThis = this->Get();
	const auto currentLevel = pThis->GetCurrentRadLevel();

	if (currentLevel <= 0)
		return 0.0;

	const auto nMax = static_cast<double>(pThis->Spread);
	const auto nDistance = cell.DistanceFrom(pThis->BaseCell);

	if (!nMax && !nDistance)
		return currentLevel;

	return (nDistance > nMax)
		? 0.0 : (nMax - nDistance) / nMax * currentLevel;
}

bool NOINLINE IsFiniteNumber(double x) {
	return (x <= DBL_MAX && x >= -DBL_MAX);
}

const double RadSiteExt::ExtData::GetRadLevelAt(double distance)
{
	const auto pThis = this->Get();
	const auto currentLevel = pThis->GetCurrentRadLevel();

	if (currentLevel <= 0)
		return 0.0;

	const auto nMax = static_cast<double>(pThis->Spread);

	if (!nMax && !distance)
		return currentLevel;

	const auto result =  (distance > nMax)
		? 0.0 : (nMax - distance) / nMax * currentLevel;

	//if (!IsFiniteNumber(result))
	//	DebugBreak();

	return result;
}

//return false mean it is already death
const RadSiteExt::ExtData::DamagingState RadSiteExt::ExtData::ApplyRadiationDamage(TechnoClass* pTarget, int damage, int distance)
{
	const auto pWarhead = this->Type->GetWarhead();
	if (!pTarget->IsAlive || pTarget->InLimbo || !pTarget->Health || pTarget->IsSinking || pTarget->IsCrashing)
		return RadSiteExt::ExtData::DamagingState::Dead;

	auto const pUnit = specific_cast<UnitClass*>(pTarget);

	if ((pUnit && pUnit->DeathFrameCounter > 0))
		return RadSiteExt::ExtData::DamagingState::Ignore;

	{
		if (!this->Type->GetWarheadDetonate())
		{
			HouseClass* const pOwner = this->TechOwner ? this->TechOwner->Owner : this->HouseOwner;
			const auto result = pTarget->ReceiveDamage(&damage, distance, pWarhead, this->TechOwner, false, true, pOwner);

			if (result != DamageState::NowDead && result != DamageState::PostMortem)
				return RadSiteExt::ExtData::DamagingState::Continue;
			else if (result == DamageState::PostMortem)
				return RadSiteExt::ExtData::DamagingState::Ignore;
		}
		else
		{
			auto const coords = pTarget->GetCoords();
			HouseClass* const pOwner = this->TechOwner ? this->TechOwner->Owner : this->HouseOwner;
			WarheadTypeExt::DetonateAt(pWarhead, pTarget, coords , this->TechOwner, damage , pOwner);

			if ((pUnit && pUnit->DeathFrameCounter > 0))
				return RadSiteExt::ExtData::DamagingState::Ignore;
		}
	}

	const auto res =  pTarget->IsAlive && !pTarget->InLimbo && pTarget->Health > 0 && !pTarget->IsSinking && !pTarget->IsCrashing;

	return res ? RadSiteExt::ExtData::DamagingState::Continue : RadSiteExt::ExtData::DamagingState::Dead;
}

// =============================
// load / save

template <typename T>
void RadSiteExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)
		.Process(this->Weapon)
		.Process(this->Type)
		.Process(this->TechOwner)
		.Process(this->HouseOwner)
		.Process(this->NoOwner)
		//.Process(this->Spread)
		;
}

// =============================
// container
RadSiteExt::ExtContainer RadSiteExt::ExtMap;

RadSiteExt::ExtContainer::ExtContainer() : Container("RadSiteClass") { };
RadSiteExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x65B28D, RadSiteClass_CTOR, 0x6)
{
	if (!Phobos::Otamaa::DisableCustomRadSite)
	{
		GET(RadSiteClass*, pThis, ESI);
		RadSiteExt::ExtMap.Allocate(pThis);
		PointerExpiredNotification::NotifyInvalidObject->Add(pThis);
	}

	return 0;
}

DEFINE_HOOK(0x65B2F4, RadSiteClass_DTOR, 0x5)
{
	if (!Phobos::Otamaa::DisableCustomRadSite)
	{
		GET(RadSiteClass*, pThis, ECX);
		PointerExpiredNotification::NotifyInvalidObject->Remove(pThis);
		RadSiteExt::ExtMap.Remove(pThis);
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

		RadSiteExt::ExtMap.PrepareStream(pItem, pStm);
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
		RadSiteExt::ExtMap.LoadStatic();

	//return 0;
	return 0x65B43F;
}

DEFINE_HOOK(0x65B464, RadSiteClass_Save_Suffix, 0x5)
{
	GET(const HRESULT , nRes, EAX);

	if(SUCCEEDED(nRes)) {
		if (!Phobos::Otamaa::DisableCustomRadSite) {
			RadSiteExt::ExtMap.SaveStatic();
		}
	}

	return 0;
}

//
//#ifdef AAENABLE_NEWHOOKS
//DEFINE_HOOK(0x65B4B0, RadSiteClass_GetSpread_Replace, 0x4)
//{
//	GET(RadSiteClass*, pThis, ECX);
//	if (!Phobos::Otamaa::DisableCustomRadSite)
//	{
//		R->EAX(RadSiteExt::ExtMap.Find(pThis)->Spread);
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
//		auto nSpread = RadSiteExt::ExtMap.Find(pThis)->Spread;
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
//		RadSiteExt::ExtMap.Find(pThis)->Spread = spread;
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
		RadSiteExt::ExtMap.InvalidatePointerFor(pThis, pTarget, bRemove);
	}
}

DEFINE_JUMP(VTABLE, 0x7F0838, GET_OFFSET(RadSiteClass_Detach));

static HouseClass* __fastcall RadSiteClass_OwningHouse(RadSiteClass* pThis, void* _)
{
	if (!Phobos::Otamaa::DisableCustomRadSite){
		if (const auto pExt = RadSiteExt::ExtMap.Find(pThis)){
			return pExt->HouseOwner;
		}
	}

	return nullptr;
}

DEFINE_JUMP(VTABLE, 0x7F084C, GET_OFFSET(RadSiteClass_OwningHouse));