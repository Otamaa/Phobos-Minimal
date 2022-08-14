#include "Body.h"

#include <New/Type/RadTypeClass.h>
#include <LightSourceClass.h>

RadSiteExt::ExtContainer RadSiteExt::ExtMap;

void RadSiteExt::ExtData::InitializeConstants()
{
	this->Type = RadTypeClass::Find("Radiation");
}

RadSiteExt::ExtData* RadSiteExt::GetExtData(RadSiteExt::base_type const* pTr) {
	return RadSiteExt::ExtMap.Find(pTr);
}

void RadSiteExt::ExtData::InvalidatePointer(void* ptr, bool bRemoved)
{
	AnnounceInvalidPointer(Weapon,ptr);
	AnnounceInvalidPointer(TechOwner, ptr);
}

void RadSiteExt::CreateInstance(const CellStruct& location, int spread, int amount, WeaponTypeExt::ExtData* pWeaponExt, TechnoClass* const pTech)
{
	// use real ctor
	const auto pRadSite = GameCreate<RadSiteClass>();
	const auto pRadExt = RadSiteExt::GetExtData(pRadSite);

	if (!pRadExt)
		Debug::FatalErrorAndExit("Uneable To find Ext for [%x] Radsite ! \n", pRadSite);

	//Adding Owner to RadSite, from bullet
	if (pWeaponExt) {
		pRadExt->Weapon = pWeaponExt->Get();
		pRadExt->Type = pWeaponExt->RadType;
		pRadExt->NoOwner = pWeaponExt->Rad_NoOwner.Get();
	}

	if(pTech && pRadExt->Type->GetHasInvoker() && !pRadExt->NoOwner && pRadExt->Type->GetHasOwner()){
		pRadExt->TechOwner = pTech;
	}

	auto nLoc = location;
	pRadSite->SetBaseCell(&nLoc);
	pRadSite->SetSpread(spread);
	pRadExt->SetRadLevel(amount);
	pRadExt->CreateLight();

}

//RadSiteClass Activate , Rewritten
void RadSiteExt::ExtData::CreateLight()
{
	const auto pThis = Get();
	const auto nLevelDelay = Type->GetLevelDelay();
	const auto nLightDelay = Type->GetLightDelay();
	const auto nRadcolor = Type->GetColor();
	const auto nTintFactor = Type->GetTintFactor();

	const auto nLightFactor = Math::min(pThis->RadLevel * Type->GetLightFactor(), 2000.0);
	const auto nDuration = pThis->RadDuration;

	pThis->RadLevelTimer.Start(nLevelDelay);
	pThis->RadLightTimer.Start(nLightDelay);
	pThis->Intensity = static_cast<int>(nLightFactor);
	pThis->LevelSteps = nDuration / nLevelDelay;
	pThis->IntensitySteps = nDuration / nLightDelay;
	pThis->IntensityDecrement = (int)(nLightFactor) / (nDuration / nLightDelay);

	TintStruct nTintBuffer {};
	nTintBuffer.Red = static_cast<int>(Math::min(((1000 * nRadcolor.R) / 255) * nTintFactor, 2000.0));
	nTintBuffer.Green = static_cast<int>(Math::min(((1000 * nRadcolor.G) / 255) * nTintFactor, 2000.0));
	nTintBuffer.Blue = static_cast<int>(Math::min(((1000 * nRadcolor.B) / 255) * nTintFactor, 2000.0));

	pThis->Tint = nTintBuffer;

	if (pThis->LightSource)
	{
		pThis->LightSource->ChangeLevels(static_cast<int>(nLightFactor), nTintBuffer, false);
		pThis->Radiate();
	}
	else
	{
		if (auto const pLight = GameCreate<LightSourceClass>(pThis->GetCoords(), pThis->SpreadInLeptons, static_cast<int>(nLightFactor), nTintBuffer))
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
	const auto pThis = Get();
	pThis->Deactivate();
	const auto nInput = ((pThis->RadLevel * pThis->RadTimeLeft) / pThis->RadDuration) + amount;
	pThis->RadLevel = nInput;
	const auto nInput_2 = nInput * Type->GetDurationMultiple();
	pThis->RadDuration = nInput_2;
	pThis->RadTimeLeft = nInput_2;
	CreateLight();
}

void RadSiteExt::ExtData::SetRadLevel(int amount)
{
	const auto pThis = Get();
	amount = Math::min(amount, Type->GetLevelMax());
	const int mult = Type->GetDurationMultiple();
	pThis->RadLevel = amount;
	pThis->RadDuration = mult * amount;
	pThis->RadTimeLeft = mult * amount;
}

// helper function provided by AlexB
const double RadSiteExt::ExtData::GetRadLevelAt(CellStruct const& cell)
{
	const RadSiteClass* pThis = Get();
	const double nMax = static_cast<double>(pThis->SpreadInLeptons);
	const double nDistance = Map.GetCellAt(cell)->GetCoords()
		.DistanceFrom(pThis->GetCoords());
	return (nDistance > nMax || pThis->GetRadLevel() <= 0 ) ? 0.0 : (nMax - nDistance) / nMax * pThis->GetRadLevel();
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
		.Process(this->NoOwner)
		;
}

void RadSiteExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<RadSiteClass>::Serialize(Stm);
	this->Serialize(Stm);
}

void RadSiteExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<RadSiteClass>::Serialize(Stm);
	this->Serialize(Stm);
}

void RadSiteExt::ExtContainer::InvalidatePointer(void* ptr, bool bRemoved) { }

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

#ifdef ENABLE_NEWHOOKS
		RadSiteExt::ExtMap.JustAllocate(pThis, pThis->WhatAmI() == AbstractType::RadSite, "Trying To Allocate from unknown pointer !");
#else
		if (auto pRadExt = RadSiteExt::ExtMap.FindOrAllocate(pThis))
			pRadExt->InitializeConstants();
#endif

	}

	return 0;
}

DEFINE_HOOK(0x65B2F4, RadSiteClass_DTOR, 0x5)
{
	if (!Phobos::Otamaa::DisableCustomRadSite)
	{
		GET(RadSiteClass*, pThis, ECX);
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

DEFINE_HOOK(0x65B43F, RadSiteClass_Load_Suffix, 0x7)
{
	if (!Phobos::Otamaa::DisableCustomRadSite)
		RadSiteExt::ExtMap.LoadStatic();

	return 0;
}

DEFINE_HOOK(0x65B464, RadSiteClass_Save_Suffix, 0x5)
{
	if (!Phobos::Otamaa::DisableCustomRadSite)
		RadSiteExt::ExtMap.SaveStatic();

	return 0;
}