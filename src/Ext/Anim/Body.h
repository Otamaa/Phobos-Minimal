#pragma once
#include <AnimClass.h>

#include <Utilities/PooledContainer.h>
#include <Utilities/OptionalStruct.h>
#include <Utilities/TemplateDef.h>
//#include <New/AnonymousType/SpawnsStatus.h>

#include <ParticleSystemClass.h>

#include <Ext/Object/Body.h>

class HouseClass;
class AnimExtData : public ObjectExtData
{
public:
	using base_type = AnimClass;
	static constexpr unsigned Marker = UuidFirstPart<base_type>::value;

public:

#pragma region ClassMembers

	OptionalStruct<CoordStruct, true> BackupCoords;
	OptionalStruct<DirType, true> DeathUnitFacing;
	OptionalStruct<DirStruct, true> DeathUnitTurretFacing;
	TechnoClass* Invoker;
	bool OwnerSet;
	bool AllowCreateUnit;
	bool WasOnBridge;

	// This is a failsafe that is only set if this is a building animation
	// and the building is not on same cell as the animation.
	BuildingClass* ParentBuilding;

	ParticleSystemClass* AttachedSystem;
	CoordStruct CreateUnitLocation;

	bool DelayedFireRemoveOnNoDelay;
	StageClass	DamagingState;
	Point2D AEDrawOffset;
#pragma endregion

public:

	AnimExtData(AnimClass* pObj) : ObjectExtData(pObj)
		, BackupCoords {}
		, DeathUnitFacing {}
		, DeathUnitTurretFacing {}
		, Invoker { nullptr }
		, OwnerSet { false }
		, AllowCreateUnit { false }
		, WasOnBridge { false }
		, ParentBuilding { nullptr }
		, AttachedSystem { nullptr }
		, CreateUnitLocation {}
		, DelayedFireRemoveOnNoDelay { false }
		, DamagingState { }
		, AEDrawOffset {}
	{
		this->AOName = pObj->Type->ID;
		this->AbsType = AnimClass::AbsID;
	}

	AnimExtData(AnimClass* pObj, noinit_t nn) : ObjectExtData(pObj, nn) { }

	virtual ~AnimExtData()
	{
		// mimicking how this thing does , since the detach seems not properly handle these
		if (auto pAttach = AttachedSystem)
		{
			pAttach->Owner = nullptr;
			pAttach->UnInit();
			pAttach->TimeToDie = true;
		}
	}

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override;

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->ObjectExtData::LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		this->ObjectExtData::SaveToStream(Stm);
		const_cast<AnimExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const
	{
		this->ObjectExtData::CalculateCRC(crc);
	}

	virtual AnimClass* This() const override { return reinterpret_cast<AnimClass*>(this->ObjectExtData::This()); }
	virtual const AnimClass* This_Const() const override { return reinterpret_cast<const AnimClass*>(this->ObjectExtData::This_Const()); }

public:

	static const std::pair<bool, OwnerHouseKind> SetAnimOwnerHouseKind(AnimClass* pAnim, HouseClass* pInvoker, HouseClass* pVictim, bool defaultToVictimOwner);
	static const std::pair<bool, OwnerHouseKind> SetAnimOwnerHouseKind(AnimClass* pAnim, HouseClass* pInvoker, HouseClass* pVictim, TechnoClass* pTechnoInvoker, bool defaultToVictimOwner , bool forceOwnership);
	static TechnoClass* GetTechnoInvoker(AnimClass* pThis);
	static AbstractClass* GetTarget(AnimClass* const);
	static void ChangeAnimType(AnimClass* pAnim, AnimTypeClass* pNewType, bool resetLoops, bool restart);
	static void DealDamageDelay(AnimClass* pThis);
	static bool OnExpired(AnimClass* pThis, bool LandIsWater, bool EligibleHeight);
	static bool OnMiddle(AnimClass* pThis);
	static bool OnMiddle_SpawnSmudge(AnimClass* pThis, CellClass* pCell, Point2D nOffs);
	static void OnInit(AnimClass* pThis, CoordStruct* pCoord);
	static void CreateRandomAnim(Iterator<AnimTypeClass*> AnimList, CoordStruct coords, TechnoClass* pTechno = nullptr, HouseClass* pHouse = nullptr, bool ownedObject = false);

	static Layer __fastcall GetLayer_patch(AnimClass* pThis, void* _);

	static void SpawnFireAnims(AnimClass* pThis);

public:

	void CreateAttachedSystem();
	void OnStart() { };
	void OnMiddle() { };
	void OnEnd() { };
	void OnTypeChange();

private:
	template <typename T>
	void Serialize(T& Stm);
};

class AnimExtContainer final : public Container<AnimExtData>
{
public:
	static AnimExtContainer Instance;
	static std::list<AnimClass*> AnimsWithAttachedParticles;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static void InvalidatePointer(AbstractClass* const ptr, bool bRemoved)
	{
		for (auto& ext : Array) {
			ext->InvalidatePointer(ptr, bRemoved);
		}
	}
};

class AnimTypeExtData;
class NOVTABLE FakeAnimClass : public AnimClass
{
public:


	FORCEDINLINE HouseClass* _GetOwningHouse() {
		return this->Owner;
	}

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, BOOL clearDirty);

	void _Middle();
	void _Start();
	void _AI();

	void _ApplyVeinsDamage();
	void _ApplyDeformTerrrain();
	void _ApplyHideIfNoOre();
	void _ApplySpawns(CoordStruct& coord);

	void _CreateFootApplyOccupyBits();
	void _CreateFoot();

	void _SpreadTiberium(CoordStruct& coords , bool isOnbridge);
	void _PlayExtraAnims(bool onWater , bool onBridge);
	void _DrawTrailerAnim();
	CoordStruct* __GetCenterCoords(CoordStruct* pBuffer);

	int _BounceAI();

	FORCEDINLINE AnimClass* _AsAnim() const {
		return (AnimClass*)this;
	}

	FORCEDINLINE AnimExtData* _GetExtData() {
		return *reinterpret_cast<AnimExtData**>(((DWORD)this) + AbstractExtOffset);
	}

	FORCEDINLINE AnimTypeExtData* _GetTypeExtData() {
		return *reinterpret_cast<AnimTypeExtData**>(((DWORD)this->Type) + AbstractExtOffset);
	}
};
static_assert(sizeof(FakeAnimClass) == sizeof(AnimClass), "Invalid Size !");