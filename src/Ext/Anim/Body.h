#pragma once
#include <AnimClass.h>

#include <Utilities/Container.h>
#include <Utilities/OptionalStruct.h>
#include <Utilities/TemplateDef.h>

#include <New/AnonymousType/SpawnsStatus.h>

class HouseClass;
class ParticleSystemClass;
class AnimExtData final //: public Extension<AnimClass>
{
public:
	using base_type = AnimClass;
	static constexpr size_t Canary = 0xAADAAAA;

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
	static bool InvalidateIgnorable(AbstractClass* ptr);
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

	constexpr FORCEINLINE static size_t size_Of()
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

class AnimExtContainer final : public Container<AnimExtData>
{
public:

	//all inactive pointer will be on the back
	static std::vector<AnimExtData*> Pool;
	static AnimExtContainer Instance;

	AnimExtData* AllocateUnchecked(AnimClass* key)
	{
		AnimExtData* val = nullptr;
		if (!Pool.empty()) {
			val = Pool.front();
			Pool.erase(Pool.begin());
			//re-init
		} else {
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

	AnimExtData* Allocate(AnimClass* key)
	{
		if (!key || Phobos::Otamaa::DoingLoadGame)
			return nullptr;

		this->ClearExtAttribute(key);

		if (AnimExtData* val = AllocateUnchecked(key))
		{
			this->SetExtAttribute(key, val);
			return val;
		}

		return nullptr;
	}

	void Remove(AnimClass* key)
	{
		if (AnimExtData* Item = TryFind(key))
		{
			Item->~AnimExtData();
			Item->AttachedToObject = nullptr;
			Pool.push_back(Item);
			this->ClearExtAttribute(key);
		}
	}

	void Clear()
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

	//CONSTEXPR_NOCOPY_CLASSB(AnimExtContainer , AnimExtData, "AnimClass");
};