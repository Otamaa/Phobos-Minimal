#include "Body.h"

#include <Ext/WarheadType/Body.h>

#include <New/Type/RadTypeClass.h>
#include <LightSourceClass.h>
#include <Utilities/Macro.h>
#include <Notifications.h>

RadSiteExt::ExtContainer RadSiteExt::ExtMap;

void RadSiteExt::ExtData::InitializeConstants()
{ }

void RadSiteExt::ExtData::InvalidatePointer(void* ptr, bool bRemoved)
{
	if (InvalidateIgnorable(ptr))
		return;

	AnnounceInvalidPointer(TechOwner, ptr);
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
	const auto nDuration = pThis->RadDuration;

	pThis->RadLevelTimer.Start(nLevelDelay);
	pThis->RadLightTimer.Start(nLightDelay);
	pThis->Intensity = static_cast<int>(nLightFactor);
	pThis->LevelSteps = nDuration / nLevelDelay;
	pThis->IntensitySteps = nDuration / nLightDelay;
	pThis->IntensityDecrement = (int)(nLightFactor) / (nDuration / nLightDelay);
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
	const auto nInput = ((pThis->RadLevel * pThis->RadTimeLeft) / pThis->RadDuration) + amount;
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

	return (nDistance > nMax) 
		? 0.0 : (nMax - nDistance) / nMax * currentLevel;
}

const bool RadSiteExt::ExtData::ApplyRadiationDamage(TechnoClass* pTarget, int damage, int distance)
{
	const auto pWarhead = this->Type->GetWarhead();

	if (!this->Type->GetWarheadDetonate())
	{
		HouseClass* const pOwner = this->TechOwner ? this->TechOwner->Owner : this->HouseOwner;
		if (pTarget->ReceiveDamage(&damage, distance, pWarhead, this->TechOwner, false, true, pOwner) == DamageState::NowDead)
			return false;
	}
	else
	{
		auto const coords = pTarget->GetCoords();
		WarheadTypeExt::DetonateAt(pWarhead, pTarget, coords , this->TechOwner, damage);

		if (!pTarget->IsAlive)
			return false;
	}

	return true;
}

// =============================
// load / save

template <typename T>
void RadSiteExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Weapon)
		.Process(this->Type)
		.Process(this->TechOwner)
		.Process(this->HouseOwner)
		.Process(this->NoOwner)
		//.Process(this->Spread)
		;
}

void RadSiteExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<RadSiteClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void RadSiteExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<RadSiteClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

//void RadSiteExt::ExtContainer::InvalidatePointer(void* ptr, bool bRemoved) { }

bool RadSiteExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool RadSiteExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

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
		if (auto pExt = RadSiteExt::ExtMap.Find(pThis))
			pExt->InvalidatePointer(pTarget, bRemove);
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