#include "Body.h"

#include <Ext/BuildingType/Body.h>

#include <Ext/Aircraft/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/House/Body.h>
#include <Ext/HouseType/Body.h>

#include <InfantryClass.h>

ASMJIT_PATCH(0x517D69, InfantryClass_Init_InitialStrength, 0x6)
{
	GET(InfantryClass*, pThis, ESI);

	const auto strength = TechnoExtData::GetInitialStrength(pThis->Type, pThis->Type->Strength);
	pThis->Health = strength;
	pThis->EstimatedHealth = strength;

	return 0;
}

ASMJIT_PATCH(0x7355BA, UnitClass_Init_InitialStrength, 0x6)
{
	GET(UnitClass*, pThis, ESI);
	GET(UnitTypeClass*, pType, EAX);

	if(TechnoTypeExtContainer::Instance.Find(pType)->Initial_DriverKilled)
		TechnoExtContainer::Instance.Find(pThis)->Is_DriverKilled = true;

	R->EAX(TechnoTypeExtContainer::Instance.Find(pType)->InitialStrength.Get(pType->Strength));
	return 0x7355C0;
}

ASMJIT_PATCH(0x414051, AircraftClass_Init_InitialStrength, 0x6)
{
	GET(AircraftClass*, pThis, ESI);
	GET(AircraftTypeClass*, pType, EAX);

	if (TechnoTypeExtContainer::Instance.Find(pType)->Initial_DriverKilled)
		TechnoExtContainer::Instance.Find(pThis)->Is_DriverKilled = true;

	R->EAX(TechnoTypeExtContainer::Instance.Find(pType)->InitialStrength.Get(pType->Strength));
	return 0x414057;
}

#include <Ext/Building/Body.h>


ASMJIT_PATCH(0x442C40, BuildingClass_Init_Log, 0x8)
{
	GET_STACK(DWORD, caller, 0x0);
	GET(BuildingClass*, pThis, ESI);
	
	//the CTOR can be loaded from TFactory class with nullptr both type and owner
	//so dont bother to do these

	if (!pThis->Owner && pThis->Type) {
		Debug::Log("Missing Ownership %s[%s]/n", pThis->Type->ID, pThis->Type->Name);
		return 0x442D86; //someone messed up, but dont crash
	} else if (!pThis->Owner && !pThis->Type) {
		return 0x442D86; //dont bother
	}

	pThis->TechnoClass::Init();

	auto pBldExt = BuildingExtContainer::Instance.Find(pThis);

	pBldExt->MyPrismForwarding = std::make_unique<PrismForwarding>();
	pBldExt->MyPrismForwarding->Owner = pThis;

	pThis->OwnerCountryIndex = pThis->Owner->Type->ParentIdx;
	pThis->Owner->AddTracking(pThis);

	auto const pBldTypeExt = BuildingTypeExtContainer::Instance.Find(pThis->Type);

	const int __HP = pBldTypeExt->InitialStrength.Get(pThis->Type->Strength);
	pThis->Health = __HP;
	pThis->EstimatedHealth = __HP;

	int ammo = pThis->Type->InitialAmmo;
	if(pThis->Type->InitialAmmo  == -1)
		ammo = pThis->Type->Ammo;
	pThis->Ammo = ammo;

	pThis->PrimaryFacing.Set_ROT(pThis->Type->ROT);

	if(pThis->Type->LoadBuildup()){
		pThis->Type->ClearBuildUp();
		pThis->IsAllowedToSell = !pThis->Type->Unsellable;
	}else{
		pThis->IsAllowedToSell = false;
	}

	if (pThis->Type->Cloakable) {
		pThis->Cloakable = true;
	}

	if(pThis->Owner->WarFactoryInfiltrated){
		if(!pThis->Type->Naval && pThis->Type->Trainable && pThis->Type->UndeploysInto){
			pThis->Veterancy.Veterancy = 1.0f;
		}
	}

	if (HouseTypeExtContainer::Instance.Find(pThis->Owner->Type)->VeteranBuildings.Contains(pThis->Type)) {
		pThis->Veterancy.Veterancy = 1.0f;
	}

	if (pThis->Type->Trainable && HouseExtContainer::Instance.Find(pThis->Owner)->Is_ConstructionYardSpied)
		pThis->Veterancy.Veterancy = 1.0f;


	HouseExtData::ApplyAcademy(pThis->Owner, pThis, AbstractType::Building);
	if(pThis->Type->SecretLab){
		BuildingClass::Secrets->push_back(pThis);
	}

	return 0x442D86;
}

// ASMJIT_PATCH(0x442CCF, BuildingClass_Init_Sellable, 0x7)
// {
// 	GET(BuildingClass*, pThis, ESI);
// 	pThis->IsAllowedToSell = !pThis->Type->Unsellable;
// 	return 0x0;
// }


/* #183 - cloakable on Buildings and Aircraft */
// ASMJIT_PATCH(0x442CE0, BuildingClass_Init_Cloakable, 0x6)
// {
// 	GET(BuildingClass*, Item, ESI);
//
// 	if (Item->Type->Cloakable)
// 	{
// 		Item->Cloakable = true;
// 	}
//
// 	return 0;
// }

// ASMJIT_PATCH(0x442D1B, BuildingClass_Init_Academy, 6)
// {
// 	GET(BuildingClass*, pThis, ESI);

// 	if (!pThis->Owner)
// 		return 0x0;

// 	if (HouseTypeExtContainer::Instance.Find(pThis->Owner->Type)->VeteranBuildings.Contains(pThis->Type))
// 	{
// 		pThis->Veterancy.Veterancy = 1.0f;
// 	}

// 	if (pThis->Type->Trainable && HouseExtContainer::Instance.Find(pThis->Owner)->Is_ConstructionYardSpied)
// 		pThis->Veterancy.Veterancy = 1.0f;


// 	HouseExtData::ApplyAcademy(pThis->Owner, pThis, AbstractType::Building);

// 	return 0;
// }

// ASMJIT_PATCH(0x442C40, BuildingClass_Init, 0x8)
// {
// 	GET(BuildingClass*, pThis, ESI);

// 	if (!pThis->Owner)
// 		Debug::FatalErrorAndExit("Missing Ownership %s[%s]/n", pThis->Type ? pThis->Type->ID : "Unknown", pThis->Type ? pThis->Type->Name : "Unknown");

// 	pThis->TechnoClass::Init();

// 	auto pBldExt = BuildingExtContainer::Instance.Find(pThis);

// 	pBldExt->MyPrismForwarding = std::make_unique<PrismForwarding>();
// 	pBldExt->MyPrismForwarding->Owner = pThis;

// 	pThis->OwnerCountryIndex = pThis->Owner->Type->ParentIdx;
// 	pThis->Owner->AddTracking(pThis);

// 	if (!pThis->Type)
// 		return 0x442D1Bl;

// 	auto const pBldTypeExt = BuildingTypeExtContainer::Instance.Find(pThis->Type);

// 	R->EAX(pThis->Type);
// 	R->ECX(pBldTypeExt->InitialStrength.Get(pThis->Type->Strength));
// 	return 0x442C7B;
// }