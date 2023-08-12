#pragma once
#include "UnitTrackerClass.h"

#include <Helpers/Macro.h>
#include <Utilities/Debug.h>

PhobosUnitTrackerClass* PhobosUnitTrackerClass::Initialize()
{
	this->Array = {};
	this->IsNetworkFormat = false;
	return this;
}

void PhobosUnitTrackerClass::Clear()
{
	this->Array.clear();
}

void PhobosUnitTrackerClass::ToPCFormat()
{
	if (this->IsNetworkFormat)
	{
		for (int i = 0; i < this->Array.size(); ++i)
			this->Array[i] = PhobosUnitTrackerClass::ntohl(this->Array[i]);
	}

	this->IsNetworkFormat = false;
}

void PhobosUnitTrackerClass::ToNetworkFormat()
{
	if (!this->IsNetworkFormat)
	{
		for (int i = 0; i < this->Array.size(); ++i)
			this->Array[i] = PhobosUnitTrackerClass::htonl(this->Array[i]);
	}

	this->IsNetworkFormat = true;
}

void PhobosUnitTrackerClass::DecrementUnitTotal(int nUnit)
{
	if (nUnit < this->Array.size())
		--this->Array[nUnit];
}

void PhobosUnitTrackerClass::ClearUnitTotal()
{
	for (auto& nData : this->Array)
		nData = 0;
}

void PhobosUnitTrackerClass::IncrementUnitTotal(int nUnit)
{
	if (nUnit < this->Array.size())
		++this->Array[nUnit];
}

int* PhobosUnitTrackerClass::GetAllTotals()
{
	return this->Array.data();
}

void PhobosUnitTrackerClass::PopulateUnitCounts(int nCount)
{
	if(nCount > this->Array.size())
	{
		this->Clear();
		this->Array.resize(nCount);
	}
}

int PhobosUnitTrackerClass::GetUnitCounts() const
{
	int nRet = 0;

	for (int i = 0; i < this->Array.size(); ++i)
		nRet += this->Array[i];

	return nRet;
}
//
//DEFINE_HOOK(0x748FD0, UnitTrackerClass_CTOR, 0x5)
//{
//	GET(PhobosUnitTrackerClass*, pThis, ECX);
//
//	R->EAX(pThis->Initialize());
//
//	return 0x749000;
//}
//
//DEFINE_HOOK(0x749010, UnitTrackerClass_DTOR, 0x0)
//{
//	GET(PhobosUnitTrackerClass*, pThis, ECX);
//
//	pThis->Clear();
//
//	return 0;
//}
//
//DEFINE_HOOK(0x749150, UnitTrackerClass_ToPCFormat, 0x5)
//{
//	GET(PhobosUnitTrackerClass*, pThis, ECX);
//
//	pThis->ToPCFormat();
//
//	return 0x74919A;
//}
//
//DEFINE_HOOK(0x749100, UnitTrackerClass_ToNetworkFormat, 0xA)
//{
//	GET(PhobosUnitTrackerClass*, pThis, ECX);
//
//	pThis->ToNetworkFormat();
//
//	return 0x749142;
//}
//
//DEFINE_HOOK(0x749040, UnitTrackerClass_DecrementUnitTotal, 0xA)
//{
//	GET(PhobosUnitTrackerClass*, pThis, ECX);
//	GET_STACK(int, nUnit, 0x4);
//
//	pThis->DecrementUnitTotal(nUnit);
//
//	return 0x749051;
//}
//
//DEFINE_HOOK(0x7490D0, UnitTrackerClass_ClearUnitTotal, 0x6)
//{
//	GET(PhobosUnitTrackerClass*, pThis, ECX);
//
//	pThis->ClearUnitTotal();
//
//	return 0x7490F4;
//}
//
//DEFINE_HOOK(0x749024, UnitTrackerClass_IncrementUnitTotal, 0x6)
//{
//	GET(PhobosUnitTrackerClass*, pThis, ECX);
//	GET(int, nArrayIdx, EAX);
//
//	pThis->IncrementUnitTotal(nArrayIdx);
//
//	return 0x749031;
//}
//
//DEFINE_HOOK(0x7490C2, UnitTrackerClass_GetAllTotals, 0x0)
//{
//	GET(PhobosUnitTrackerClass*, pThis, EAX);
//	R->EAX(pThis->GetAllTotals());
//
//	return 0;
//}
//
//DEFINE_HOOK(0x4F638F, HouseClass_Init_PopulateTracker, 0x6)
//{
//	GET(HouseClass*, pThis, EBP);
//
//	const int nAircraftTypeCount = AircraftTypeClass::Array->Count;
//	pThis->BuiltAircraftTypes.PopulateUnitCount(nAircraftTypeCount);
//	pThis->KilledAircraftTypes.PopulateUnitCount(nAircraftTypeCount);
//
//	const int nInfantryTypeCount = InfantryTypeClass::Array->Count;
//	pThis->BuiltInfantryTypes.PopulateUnitCount(nInfantryTypeCount);
//	pThis->KilledInfantryTypes.PopulateUnitCount(nInfantryTypeCount);
//
//	const int nUnitTypeCount = UnitTypeClass::Array->Count;
//	pThis->BuiltUnitTypes.PopulateUnitCount(nUnitTypeCount);
//	pThis->KilledUnitTypes.PopulateUnitCount(nUnitTypeCount);
//
//	const int nBuildingTypeCount = BuildingTypeClass::Array->Count;
//	pThis->BuiltBuildingTypes.PopulateUnitCount(nBuildingTypeCount);
//	pThis->KilledBuildingTypes.PopulateUnitCount(nBuildingTypeCount);
//	pThis->CapturedBuildings.PopulateUnitCount(nBuildingTypeCount);
//
//	pThis->CollectedCrates.PopulateUnitCount(19);
//
//	return 0x4F643B;
//}

DEFINE_HOOK(0x749060, UnitTrackerClass_PopulateUnitCounts, 0xB)
{
	GET(UnitTrackerClass*, pThis, ECX);
	GET_STACK(int, nCount, 0x4);

	const auto nParent = (*(uintptr_t*)((char*)pThis)) - sizeof(HouseClass);
	if (const auto pParents = reinterpret_cast<HouseClass* const>(nParent)) {
		Debug::Log("Boom ! \n");
	}

	//pThis->PopulateUnitCounts(nCount);
	return 0x0;
	//return 0x74908E;
}
//
//DEFINE_HOOK(0x7490A0, UnitTrackerClass_GetUnitCounts, 0x6)
//{
//	GET(UnitTrackerClass*, pThis, ECX);
//
//	R->EAX(pThis->GetUnitCounts());
//
//	return 0x7490B8;
//}

//DEFINE_HOOK(0x5031E6, HouseClass_Load_UnitTrackers, 0x6)
//{
//	GET(HouseClass*, pThis, ESI);
//	GET(IStream*, pStm, EDI);
//
//	reinterpret_cast<PhobosUnitTrackerClass*>(&pThis->BuiltInfantryTypes)->Load(pStm);
//	reinterpret_cast<PhobosUnitTrackerClass*>(&pThis->BuiltUnitTypes)->Load(pStm);
//	reinterpret_cast<PhobosUnitTrackerClass*>(&pThis->BuiltAircraftTypes)->Load(pStm);
//	reinterpret_cast<PhobosUnitTrackerClass*>(&pThis->BuiltBuildingTypes)->Load(pStm);
//	reinterpret_cast<PhobosUnitTrackerClass*>(&pThis->KilledInfantryTypes)->Load(pStm);
//	reinterpret_cast<PhobosUnitTrackerClass*>(&pThis->KilledUnitTypes)->Load(pStm);
//	reinterpret_cast<PhobosUnitTrackerClass*>(&pThis->KilledAircraftTypes)->Load(pStm);
//	reinterpret_cast<PhobosUnitTrackerClass*>(&pThis->KilledBuildingTypes)->Load(pStm);
//	reinterpret_cast<PhobosUnitTrackerClass*>(&pThis->CapturedBuildings)->Load(pStm);
//	reinterpret_cast<PhobosUnitTrackerClass*>(&pThis->CollectedCrates)->Load(pStm);
//
//	return 0;
//}
//
//DEFINE_HOOK(0x5040A2, HouseClass_Save_UnitTrackers, 0x6)
//{
//	GET(HouseClass*, pThis, EDI);
//	GET(IStream*, pStm, ESI);
//
//	reinterpret_cast<PhobosUnitTrackerClass*>(&pThis->BuiltInfantryTypes)->Save(pStm);
//	reinterpret_cast<PhobosUnitTrackerClass*>(&pThis->BuiltUnitTypes)->Save(pStm);
//	reinterpret_cast<PhobosUnitTrackerClass*>(&pThis->BuiltAircraftTypes)->Save(pStm);
//	reinterpret_cast<PhobosUnitTrackerClass*>(&pThis->BuiltBuildingTypes)->Save(pStm);
//	reinterpret_cast<PhobosUnitTrackerClass*>(&pThis->KilledInfantryTypes)->Save(pStm);
//	reinterpret_cast<PhobosUnitTrackerClass*>(&pThis->KilledUnitTypes)->Save(pStm);
//	reinterpret_cast<PhobosUnitTrackerClass*>(&pThis->KilledAircraftTypes)->Save(pStm);
//	reinterpret_cast<PhobosUnitTrackerClass*>(&pThis->KilledBuildingTypes)->Save(pStm);
//	reinterpret_cast<PhobosUnitTrackerClass*>(&pThis->CapturedBuildings)->Save(pStm);
//	reinterpret_cast<PhobosUnitTrackerClass*>(&pThis->CollectedCrates)->Save(pStm);
//
//	return 0;
//}