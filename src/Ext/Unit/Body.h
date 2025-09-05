#pragma once

#include <Ext/Foot/Body.h>
#include <UnitClass.h>

class UnitExtData : public FootExtData
{
public:
	using base_type = UnitClass;
	static constexpr unsigned Marker = UuidFirstPart<base_type>::value;

public:

	UnitExtData(UnitClass* pObj) : FootExtData(pObj) { }
	UnitExtData(UnitClass* pObj, noinit_t nn) : FootExtData(pObj, nn) { }

	virtual ~UnitExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override
	{
		this->FootExtData::InvalidatePointer(ptr, bRemoved);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->FootExtData::LoadFromStream(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		const_cast<UnitExtData*>(this)->FootExtData::SaveToStream(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const
	{
		this->FootExtData::CalculateCRC(crc);
	}
	virtual UnitClass* This() const override { return reinterpret_cast<UnitClass*>(this->FootExtData::This()); }
	virtual const UnitClass* This_Const() const override { return reinterpret_cast<const UnitClass*>(this->FootExtData::This_Const()); }
};

class UnitExtContainer final : public Container<UnitExtData>
{
public:
	static UnitExtContainer Instance;

	static void Clear()
	{
		Array.clear();
	}

	static bool LoadGlobals(PhobosStreamReader& Stm)
	{
		return true;
	}

	static bool SaveGlobals(PhobosStreamWriter& Stm)
	{
		return true;
	}

	static void InvalidatePointer(AbstractClass* const ptr, bool bRemoved)
	{
		for (auto& ext : Array)
		{
			ext->InvalidatePointer(ptr, bRemoved);
		}
	}

	static bool HasDeployingAnim(UnitTypeClass* pUnitType);
	static bool CheckDeployRestrictions(FootClass* pUnit, bool isDeploying);
	static void CreateDeployingAnim(UnitClass* pUnit, bool isDeploying);
};

class UnitTypeExtData;
class NOVTABLE FakeUnitClass : public UnitClass
{
public:
	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, BOOL clearDirty);

	bool _Paradrop(CoordStruct* pCoords);
	CoordStruct* _GetFLH(CoordStruct* buffer, int wepon, CoordStruct base);
	int _Mission_Attack();
	int _Mission_AreaGuard();

	void _Deploy();
	void _UnDeploy();

	void _SetOccupyBit(CoordStruct* pCrd);
	void _ClearOccupyBit(CoordStruct* pCrd);

	void _Detach(AbstractClass* target, bool all);

	FORCEDINLINE UnitExtData* _GetExtData() {
		return *reinterpret_cast<UnitExtData**>(((DWORD)this) + AbstractExtOffset);
	}

	FORCEDINLINE UnitTypeExtData* _GetTypeExtData() {
		return *reinterpret_cast<UnitTypeExtData**>(((DWORD)this->Type) + AbstractExtOffset);
	}
};
static_assert(sizeof(FakeUnitClass) == sizeof(UnitClass), "Invalid Size !");