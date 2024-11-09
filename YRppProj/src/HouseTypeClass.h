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
	static constexpr inline DWORD vtable = 0x7EAB58;

	//Array
	static constexpr constant_ptr<DynamicVectorClass<HouseTypeClass*>, 0xA83C98u> const Array {};

	IMPL_Find(HouseTypeClass)

	static HouseTypeClass* __fastcall FindOrAllocate(const char* pID)
		;// { JMP_STD(0x512680); }

	IMPL_FindIndexById(HouseTypeClass)

	static int __fastcall FindIndexByIdAndName(const char* pID)
		;// { JMP_STD(0x5117D0); }

	//IUnknown
	virtual HRESULT __stdcall QueryInterface(REFIID iid, void** ppvObject) override;// JMP_STD(0x5125A0);
	virtual ULONG __stdcall AddRef() override { return true; }
	virtual ULONG __stdcall Release() override { return true; }

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override;//  JMP_STD(0x512640);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override;// JMP_STD(0x512290);
	virtual HRESULT __stdcall Save(IStream* pStm,BOOL fClearDirty) override;// JMP_STD(0x512480);

	//Destructor
	virtual ~HouseTypeClass();

	//AbstractClass
	virtual AbstractType WhatAmI() const override  RT(AbstractType);
	virtual int	Size() const override  R0;

	//AbstractTypeClass
	virtual bool LoadFromINI(CCINIClass* pINI) override;//  JMP_THIS(0x511850);

	//helpers
	HouseTypeClass* FindParentCountry() const {
		return HouseTypeClass::Find(this->ParentCountry);
	}

	int FindParentCountryIndex() const {
		return HouseTypeClass::FindIndexByIdAndName(this->ParentCountry);
	}

	static signed int __fastcall FindIndexOfNameShiftToTheRightOnce(const char* pName)
		;// { JMP_STD(0x48DEB0); }

	HouseTypeClass(const char* pID);
	HouseTypeClass(IStream* pStm);

public:

	FixedString<25> ParentCountry;
	BYTE align_B1[3];
	int            ArrayIndex;
	int            ArrayIndex2; //dunno why
	int            SideIndex;
	int            ColorSchemeIndex;
	DWORD align_C4; // can be used as EXTData

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
	BYTE padding_1A9[7];
};
