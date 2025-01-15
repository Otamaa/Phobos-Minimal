#pragma once
#include <AnimClass.h>

#include <Utilities/Container.h>
#include <Utilities/OptionalStruct.h>
#include <Utilities/TemplateDef.h>

#include <New/AnonymousType/SpawnsStatus.h>

#include <ParticleSystemClass.h>

class HouseClass;
class AnimExtData final //: public Extension<AnimClass>
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
	SpawnsStatus SpawnsStatusData {};

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
			 );
	}

	static const std::pair<bool, OwnerHouseKind> SetAnimOwnerHouseKind(AnimClass* pAnim, HouseClass* pInvoker, HouseClass* pVictim, bool defaultToVictimOwner = true);
	static const std::pair<bool, OwnerHouseKind> SetAnimOwnerHouseKind(AnimClass* pAnim, HouseClass* pInvoker, HouseClass* pVictim, TechnoClass* pTechnoInvoker, bool defaultToVictimOwner = true , bool forceOwnership = false);
	static TechnoClass* GetTechnoInvoker(AnimClass* pThis);
	static AbstractClass* GetTarget(AnimClass* const);
	static void ChangeAnimType(AnimClass* pAnim, AnimTypeClass* pNewType, bool resetLoops, bool restart);
	static DWORD DealDamageDelay(AnimClass* pThis);
	static bool OnExpired(AnimClass* pThis, bool LandIsWater, bool EligibleHeight);
	static bool OnMiddle(AnimClass* pThis);
	static bool OnMiddle_SpawnSmudge(AnimClass* pThis, CellClass* pCell, Point2D nOffs);
	static void OnInit(AnimClass* pThis, CoordStruct* pCoord);

	static Layer __fastcall GetLayer_patch(AnimClass* pThis, void* _);

	static void SpawnFireAnims(AnimClass* pThis);
private:
	template <typename T>
	void Serialize(T& Stm);
};

class AnimTypeExtData;
class FakeAnimClass : public AnimClass
{
public:
	OPTIONALINLINE static std::vector<AnimExtData*> Pool;

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
		AnimExtData* val = nullptr;
		if (!Pool.empty())
		{
			val = Pool.front();
			Pool.erase(Pool.begin());
			//re-init
		}
		else
		{
			val = DLLAllocWithoutCTOR<AnimExtData>();
		}

		if (val)
		{
			val->AnimExtData::AnimExtData();
			val->AttachedToObject = key;
			return val;
		}

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
			Item->~AnimExtData();
			Item->AttachedToObject = nullptr;
			Pool.push_back(Item);
			FakeAnimClass::ClearExtAttribute(key);
		}
	}

	static void Clear()
	{
		if (!Pool.empty())
		{
			auto ptr = Pool.front();
			Pool.erase(Pool.begin());
			if (ptr)
			{
				delete ptr;
			}
		}
	}

	FORCEDINLINE HouseClass* _GetOwningHouse() {
		return this->Owner;
	}

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);

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