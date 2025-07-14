#pragma once
#include <AnimClass.h>

#include <Utilities/PooledContainer.h>
#include <Utilities/OptionalStruct.h>
#include <Utilities/TemplateDef.h>
//#include <New/AnonymousType/SpawnsStatus.h>

#include <ParticleSystemClass.h>

class HouseClass;
class AnimExtData
{
public:
	using base_type = AnimClass;
	static COMPILETIMEEVAL size_t Canary = 0xAADAAAA;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:

	OptionalStruct<CoordStruct, true> BackupCoords {};
	OptionalStruct<DirType, true> DeathUnitFacing {};
	OptionalStruct<DirStruct, true> DeathUnitTurretFacing {};
	TechnoClass* Invoker { nullptr };
	bool OwnerSet { false };
	bool AllowCreateUnit { false };
	bool WasOnBridge { false };

	// This is a failsafe that is only set if this is a building animation
	// and the building is not on same cell as the animation.
	BuildingClass* ParentBuilding { nullptr };

	ParticleSystemClass* AttachedSystem { nullptr };
	CoordStruct CreateUnitLocation {};
	//SpawnsStatus SpawnsStatusData {};

	bool DelayedFireRemoveOnNoDelay { false };
	bool IsAttachedEffectAnim { false };
	bool IsShieldIdleAnim { false };

	StageClass	DamagingState { };

	void InvalidatePointer(AbstractClass* ptr, bool bRemoved);

	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	void CreateAttachedSystem();

	~AnimExtData()
	{
		// mimicking how this thing does , since the detach seems not properly handle these
		if (auto pAttach = AttachedSystem)
		{
			pAttach->Owner = nullptr;
			pAttach->UnInit();
			pAttach->TimeToDie = true;
		}
	}

	COMPILETIMEEVAL FORCEDINLINE static size_t size_Of()
	{
		return sizeof(AnimExtData) -
			(4u //AttachedToObject
			 -4u //inheritance
				);
	}

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

	static Layer __fastcall GetLayer_patch(AnimClass* pThis, void* _);

	static void SpawnFireAnims(AnimClass* pThis);

public:
	void OnStart() { };
	void OnMiddle() { };
	void OnEnd() { };
	void OnTypeChange();

private:
	template <typename T>
	void Serialize(T& Stm);
};

class AnimTypeExtData;
class NOVTABLE FakeAnimClass : public AnimClass
{
public:
	static std::list<FakeAnimClass*> AnimsWithAttachedParticles;
	static ObjectPool<AnimExtData, true> pools;

	static COMPILETIMEEVAL FORCEDINLINE void ClearExtAttribute(AnimClass* key)
	{
		(*(uintptr_t*)((char*)key + AbstractExtOffset)) = 0;
	}

	static COMPILETIMEEVAL FORCEDINLINE void SetExtAttribute(AnimClass* key, AnimExtData* val)
	{
		(*(uintptr_t*)((char*)key + AbstractExtOffset)) = (uintptr_t)val;
	}

	static COMPILETIMEEVAL FORCEDINLINE AnimExtData* GetExtAttribute(AnimClass* key)
	{
		return (AnimExtData*)(*(uintptr_t*)((char*)key + AbstractExtOffset));
	}

	static COMPILETIMEEVAL AnimExtData* TryFind(AnimClass* key)
	{
		if (!key)
			return nullptr;

		return FakeAnimClass::GetExtAttribute(key);
	}

	static AnimExtData* AllocateUnchecked(AnimClass* key)
	{
		if (AnimExtData* val = pools.allocate())
		{
			val->AttachedToObject = key;
			return val;
		}

		Debug::FatalErrorAndExit("The amount of [AnimExtData] is exceeded the ObjectPool size %d !", pools.getPoolSize());
		return nullptr;
	}

	static AnimExtData* Allocate(AnimClass* key)
	{
		if (!key || Phobos::Otamaa::DoingLoadGame)
			return nullptr;

		FakeAnimClass::ClearExtAttribute(key);

		if (AnimExtData* val = AllocateUnchecked(key))
		{
			FakeAnimClass::SetExtAttribute(key, val);
			return val;
		}

		return nullptr;
	}

	static void Remove(AnimClass* key)
	{
		if (AnimExtData* Item = FakeAnimClass::TryFind(key))
		{
			pools.deallocate(Item);
			FakeAnimClass::ClearExtAttribute(key);
		}
	}

	static void Clear()
	{
		AnimsWithAttachedParticles.clear();
	}

	FORCEDINLINE HouseClass* _GetOwningHouse() {
		return this->Owner;
	}

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);
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

	static bool LoadGlobals(PhobosStreamReader& Stm)
	{
		Stm.Process(AnimsWithAttachedParticles);
		return true;
	}

	static bool SaveGlobals(PhobosStreamWriter& Stm)
	{
		Stm.Process(AnimsWithAttachedParticles);
		return true;
	}
};
static_assert(sizeof(FakeAnimClass) == sizeof(AnimClass), "Invalid Size !");