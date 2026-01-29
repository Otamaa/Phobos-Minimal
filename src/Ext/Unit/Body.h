#pragma once

#include <Ext/Foot/Body.h>
#include <UnitClass.h>

class UnitExtData : public FootExtData
{
public:
	using base_type = UnitClass;
	static COMPILETIMEEVAL const char* ClassName = "UnitExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "UnitClass";
	static COMPILETIMEEVAL unsigned Marker = UuidFirstPart<base_type>::value;
	static COMPILETIMEEVAL auto Marker_str = to_hex_string<Marker>();

public:
#pragma region classMembers

	CDTimerClass SimpleDeployerAnimationTimer;
	CDTimerClass UnitAutoDeployTimer;
	CDTimerClass Convert_Deploy_Delay;

	// if the unit marks cell occupation flags, this is set to whether it uses the "high" occupation members
	OptionalStruct<bool, true> AltOccupation;

#pragma endregion

public:
	UnitExtData(UnitClass* pObj) : FootExtData(pObj),
		SimpleDeployerAnimationTimer(),
		UnitAutoDeployTimer(),
		Convert_Deploy_Delay(),
		AltOccupation()
	{
		this->CurrentType = pObj->Type;
		this->Name = pObj->Type->ID;
		this->AbsType = UnitClass::AbsID;
	}

	UnitExtData(UnitClass* pObj, noinit_t nn) : FootExtData(pObj, nn) { }

	virtual ~UnitExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override
	{
		this->FootExtData::InvalidatePointer(ptr, bRemoved);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->FootExtData::LoadFromStream(Stm);
		Stm.Process(this->AltOccupation);
		Stm.Process(this->SimpleDeployerAnimationTimer);
		Stm.Process(this->UnitAutoDeployTimer);
		Stm.Process(this->Convert_Deploy_Delay);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		const_cast<UnitExtData*>(this)->FootExtData::SaveToStream(Stm);
		Stm.Process(this->AltOccupation);
		Stm.Process(this->SimpleDeployerAnimationTimer);
		Stm.Process(this->UnitAutoDeployTimer);
		Stm.Process(this->Convert_Deploy_Delay);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const
	{
		this->FootExtData::CalculateCRC(crc);
	}

	UnitClass* This() const { return reinterpret_cast<UnitClass*>(this->AttachedToObject); }
	const UnitClass* This_Const() const { return reinterpret_cast<const UnitClass*>(this->AttachedToObject); }

public:

};

class UnitExtContainer final : public Container<UnitExtData>
{
public:
	static COMPILETIMEEVAL const char* ClassName = "UnitExtContainer";

public:
	static UnitExtContainer Instance;

	virtual bool LoadAll(const json& root);
	virtual bool SaveAll(json& root);

	static bool HasDeployingAnim(TechnoTypeClass* pUnitType);
	static bool CheckDeployRestrictions(FootClass* pUnit, bool isDeploying);
	static void CreateDeployingAnim(UnitClass* pUnit, bool isDeploying);
};

class UnitTypeExtData;
class NOVTABLE FakeUnitClass : public UnitClass
{
public:
	bool _Paradrop(CoordStruct* pCoords);
	CoordStruct* _GetFLH(CoordStruct* buffer, int wepon, CoordStruct base);
	int _Mission_Attack();
	int _Mission_AreaGuard();

	void _Deploy();
	void _UnDeploy();

	void _SetOccupyBit(CoordStruct* pCrd);
	void _ClearOccupyBit(CoordStruct* pCrd);

	void _Detach(AbstractClass* target, bool all);
	DamageState _Take_Damage(int* damage, int distance, WarheadTypeClass* warhead, TechnoClass* source, bool ignoreDefenses, bool PreventsPassengerEscape, HouseClass* sourceHouse);

	FORCEDINLINE UnitExtData* _GetExtData() {
		return *reinterpret_cast<UnitExtData**>(((DWORD)this) + AbstractExtOffset);
	}

	FORCEDINLINE UnitTypeExtData* _GetTypeExtData() {
		return *reinterpret_cast<UnitTypeExtData**>(((DWORD)this->Type) + AbstractExtOffset);
	}
};
static_assert(sizeof(FakeUnitClass) == sizeof(UnitClass), "Invalid Size !");