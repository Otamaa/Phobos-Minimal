#pragma once
#include <InfantryClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <ExtraHeaders/CompileTimeDirStruct.h>
#include <Ext/Foot/Body.h>

class InfantryExtData : public FootExtData
{
public:
	using base_type = InfantryClass;
	static constexpr unsigned Marker = UuidFirstPart<base_type>::value;

public:

#pragma region ClassMembers
	bool IsUsingDeathSequence;
	int CurrentDoType;
	bool SkipTargetChangeResetSequence;
	BuildingClass* GarrisonedIn; //C
#pragma endregion

public:
	InfantryExtData(InfantryClass* pObj) : FootExtData(pObj),
		IsUsingDeathSequence(false),
		CurrentDoType(-1),
		SkipTargetChangeResetSequence(false),
		GarrisonedIn(nullptr)
	{
		this->Name = pObj->Type->ID;
		this->AbsType = InfantryClass::AbsID;
	}
	InfantryExtData(InfantryClass* pObj, noinit_t nn) : FootExtData(pObj, nn) { }

	virtual ~InfantryExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override
	{
		this->FootExtData::InvalidatePointer(ptr, bRemoved);
		AnnounceInvalidPointer(GarrisonedIn, ptr, bRemoved);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->FootExtData::LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		const_cast<InfantryExtData*>(this)->FootExtData::SaveToStream(Stm);
		const_cast<InfantryExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const
	{
		this->FootExtData::CalculateCRC(crc);
	}

	virtual InfantryClass* This() const override { return reinterpret_cast<InfantryClass*>(this->FootExtData::This()); }
	virtual const InfantryClass* This_Const() const override { return reinterpret_cast<const InfantryClass*>(this->FootExtData::This_Const()); }

private:
	template <typename T>
	void Serialize(T& Stm);
};

class InfantryExtContainer final : public Container<InfantryExtData>
{
public:
	static InfantryExtContainer Instance;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static void InvalidatePointer(AbstractClass* const ptr, bool bRemoved)
	{
		for (auto& ext : Array)
		{
			ext->InvalidatePointer(ptr, bRemoved);
		}
	}
};

class InfantryTypeExtData;
class NOVTABLE FakeInfantryClass : public InfantryClass
{
public:
	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, BOOL clearDirty);

	void _Dummy(Mission, bool) RX;
	void _DummyScatter(const CoordStruct& crd, bool ignoreMission, bool ignoreDestination) RX;
	bool _Unlimbo(const CoordStruct& Crd, DirType dFaceDir);
	int _SelectWeaponAgainst(AbstractClass* pTarget);
	WeaponStruct* _GetDeployWeapon();

	DamageState _IronCurtain(int nDur, HouseClass* pSource, bool bIsFC)
	{
		if (this->Type->Engineer && this->TemporalTargetingMe && this->Destination)
		{
			if (auto const pCell = this->GetCell())
			{
				if (auto const pBld = pCell->GetBuilding())
				{
					if (this->Destination == pBld && pBld->Type->BridgeRepairHut)
					{
						return DamageState::Unaffected;
					}
				}
			}
		}

		return this->TechnoClass::IronCurtain(nDur, pSource, bIsFC);
	}

	void _DestroyThis(char flag) JMP_THIS(0x523350);
	void _Detach(AbstractClass* target, bool all);

	InfantryExtData* _GetExtData()
	{
		return *reinterpret_cast<InfantryExtData**>(((DWORD)this) + AbstractExtOffset);
	}

	InfantryTypeExtData* _GetTypeExtData() {
		return *reinterpret_cast<InfantryTypeExtData**>(((DWORD)this->Type) + AbstractExtOffset);
	}
};
static_assert(sizeof(FakeInfantryClass) == sizeof(InfantryClass), "Invalid Size !");