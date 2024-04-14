#pragma once
#include <AnimClass.h>

#include <Helpers/Macro.h>

#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <New/AnonymousType/SpawnsStatus.h>

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

	Handle<ParticleSystemClass*, UninitAttachedSystem> AttachedSystem {};
	CoordStruct CreateUnitLocation {};
	SpawnsStatus SpawnsStatusData {};

	AnimExtData() noexcept = default;
	~AnimExtData() noexcept = default;

	void InvalidatePointer(AbstractClass* ptr, bool bRemoved);
	static bool InvalidateIgnorable(AbstractClass* ptr);
	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	void CreateAttachedSystem();

	constexpr FORCEINLINE static size_t size_Of()
	{
		return sizeof(AnimExtData) -
			(4u //AttachedToObject
			 );
	}

	static const std::pair<bool, OwnerHouseKind> SetAnimOwnerHouseKind(AnimClass* pAnim, HouseClass* pInvoker, HouseClass* pVictim, bool defaultToVictimOwner = true);
	static const std::pair<bool, OwnerHouseKind> SetAnimOwnerHouseKind(AnimClass* pAnim, HouseClass* pInvoker, HouseClass* pVictim, TechnoClass* pTechnoInvoker, bool defaultToVictimOwner = true);
	static TechnoClass* GetTechnoInvoker(AnimClass* pThis, bool DealthByOwner);
	static AbstractClass* GetTarget(AnimClass* const);

	static DWORD DealDamageDelay(AnimClass* pThis);
	static bool OnExpired(AnimClass* pThis, bool LandIsWater, bool EligibleHeight);
	static bool OnMiddle(AnimClass* pThis);
	static bool OnMiddle_SpawnSmudge(AnimClass* pThis, CellClass* pCell, Point2D nOffs);
	static void OnInit(AnimClass* pThis, CoordStruct* pCoord);

	static Layer __fastcall GetLayer_patch(AnimClass* pThis, void* _);

	static HouseClass* __fastcall GetOwningHouse_Wrapper(AnimClass* pThis, void* _)
	{
		return pThis->Owner;
	}

private:
	template <typename T>
	void Serialize(T& Stm);
};

class AnimExtContainer final : public Container<AnimExtData>
{
public:

	static AnimExtContainer Instance;

	CONSTEXPR_NOCOPY_CLASSB(AnimExtContainer , AnimExtData, "AnimClass");
};