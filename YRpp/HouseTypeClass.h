/*
	ObjectTypes are initialized by INI files.
*/

#pragma once

#include <AbstractTypeClass.h>
#include <Helpers\String.h>

class AircraftTypeClass;
class InfantryTypeClass;
class UnitTypeClass;

class DECLSPEC_UUID("1DD43928-046B-11D2-ACA4-006008055BB5")
	NOVTABLE HouseTypeClass : public AbstractTypeClass
{
public:
	static const AbstractType AbsID = AbstractType::HouseType;

	//Array
	static constexpr constant_ptr<DynamicVectorClass<HouseTypeClass*>, 0xA83C98u> const Array {};

	static NOINLINE HouseTypeClass* __fastcall Find(const char* pID)
	{
		//if(!CRT::strcmpi(pID , GameStrings::RandomStr())) {
		//	return Array->Items[ScenarioClass::Instance->Random.RandomFromMax(Array->Count-1)];
		//}

		for (auto pItem : *Array){
			if (!CRT::strcmpi(pItem->ID, pID))
				return pItem;
		}

		return nullptr;
	}

	static HouseTypeClass* __fastcall FindOrAllocate(const char* pID) {
		JMP_STD(0x512680);
	}

	static NOINLINE int __fastcall FindIndexById(const char* pID)
	{
		if (!pID)
			return -1;

		for (int i = 0; i < Array->Count; ++i)
		{
			if (!CRT::strcmpi(Array->Items[i]->ID, pID))
			{
				return i;
			}
		}

		return -1;
	}

	static int __fastcall __fastcall FindIndexByIdAndName(const char* pID) {
		JMP_STD(0x5117D0);
	}

	//IUnknown
	virtual HRESULT __stdcall QueryInterface(REFIID iid, void** ppvObject) JMP_STD(0x5125A0);
	virtual ULONG __stdcall AddRef() { return true; }
	virtual ULONG __stdcall Release() { return true; }

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) JMP_STD(0x512640);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) R0;
	virtual HRESULT __stdcall Save(IStream* pStm,BOOL fClearDirty) R0;

	//Destructor
	virtual ~HouseTypeClass() JMP_THIS(0x512760);

	//AbstractClass
	virtual AbstractType WhatAmI() const RT(AbstractType);
	virtual int	Size() const R0;

	//AbstractTypeClass
	virtual bool LoadFromINI(CCINIClass* pINI) JMP_THIS(0x511850);
	//helpers
	HouseTypeClass* FindParentCountry() const {
		return HouseTypeClass::Find(this->ParentCountry);
	}

	int FindParentCountryIndex() const {
		return HouseTypeClass::FindIndexByIdAndName(this->ParentCountry);
	}

	static signed int __fastcall FindIndexOfNameShiftToTheRightOnce(const char* pName)
		{ JMP_STD(0x48DEB0); }

	//Constructor
	HouseTypeClass(const char* pID) noexcept
		: HouseTypeClass(noinit_t())
	{ JMP_THIS(0x5113F0); }

	HouseTypeClass(IStream* pStm) noexcept
		: HouseTypeClass(noinit_t())
	{ JMP_THIS(0x511650); }

protected:
	explicit __forceinline HouseTypeClass(noinit_t) noexcept
		: AbstractTypeClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	FixedString<25> ParentCountry;
	PROTECTED_PROPERTY(BYTE, align_B1[3]);
	int            ArrayIndex;
	int            ArrayIndex2; //dunno why
	int            SideIndex;
	int            ColorSchemeIndex;
	PROTECTED_PROPERTY(DWORD, align_C4); // can be used as EXTData

	//are these unused TS leftovers?
	double         FirepowerMult;
	double         GroundspeedMult;
	double         AirspeedMult;
	double         ArmorMult;
	double         ROFMult;
	double         CostMult;
	double         BuildtimeMult;
	//---

	float          ArmorInfantryMult;
	float          ArmorUnitsMult;
	float          ArmorAircraftMult;
	float          ArmorBuildingsMult;
	float          ArmorDefensesMult;

	float          CostInfantryMult;
	float          CostUnitsMult;
	float          CostAircraftMult;
	float          CostBuildingsMult;
	float          CostDefensesMult;

	float          SpeedInfantryMult;
	float          SpeedUnitsMult;
	float          SpeedAircraftMult;

	float          BuildtimeInfantryMult;
	float          BuildtimeUnitsMult;
	float          BuildtimeAircraftMult;
	float          BuildtimeBuildingsMult;
	float          BuildtimeDefensesMult;

	float          IncomeMult;

	TypeList<InfantryTypeClass*> VeteranInfantry;
	TypeList<UnitTypeClass*> VeteranUnits;
	TypeList<AircraftTypeClass*> VeteranAircraft;

	char Suffix [4];

	char           Prefix;
	bool           Multiplay;
	bool           MultiplayPassive;
	bool           WallOwner;
	bool           SmartAI; //"smart"?
	PROTECTED_PROPERTY(BYTE, padding_1A9[7]);
};
