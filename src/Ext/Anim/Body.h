#pragma once
#include <AnimClass.h>

#include <Helpers/Macro.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDef.h>
#include <Ext/Object/Body.h>

class ParticleSystemClass;
class AnimExt
{
public:

	class ExtData final : public Extension<AnimClass>
	{
	public:
		using base_type = AnimClass;
		static constexpr size_t Canary = 0xAAAAAAAA;

	public:
		OptionalStruct<CoordStruct, true> BackupCoords {};
		OptionalStruct<DirType, true> DeathUnitFacing {};
		OptionalStruct<DirStruct, true> DeathUnitTurretFacing {};
		TechnoClass* Invoker { nullptr };
		bool OwnerSet { false };
		bool AllowCreateUnit { false };

		// This is a failsafe that is only set if this is a building animation
		// and the building is not on same cell as the animation.
		BuildingClass* ParentBuilding { nullptr };

		UniqueParticleSystemClassPtr AttachedSystem {};
		CoordStruct CreateUnitLocation {};

		ExtData(base_type* OwnerObject) : Extension<AnimClass>(OwnerObject)
		{}

		virtual ~ExtData() override = default;

		void InvalidatePointer(AbstractClass* ptr, bool bRemoved);
		static bool InvalidateIgnorable(AbstractClass* ptr);
		void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
		void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

		void CreateAttachedSystem();
	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<AnimExt::ExtData>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static const std::pair<bool, OwnerHouseKind> SetAnimOwnerHouseKind(AnimClass* pAnim, HouseClass* pInvoker, HouseClass* pVictim, bool defaultToVictimOwner = true);
	static const std::pair<bool, OwnerHouseKind> SetAnimOwnerHouseKind(AnimClass* pAnim, HouseClass* pInvoker, HouseClass* pVictim, TechnoClass* pTechnoInvoker, bool defaultToVictimOwner = true);
	static TechnoClass* GetTechnoInvoker(AnimClass* pThis, bool DealthByOwner);
	static AbstractClass* GetTarget(AnimClass* const);

	static DWORD DealDamageDelay(AnimClass* pThis);
	static bool OnExpired(AnimClass* pThis, bool LandIsWater, bool EligibleHeight);
	static bool OnMiddle(AnimClass* pThis);
	static bool OnMiddle_SpawnParticle(AnimClass* pThis, CellClass* pCell, Point2D nOffs);
	static void OnInit(AnimClass* pThis, CoordStruct* pCoord);

	static Layer __fastcall GetLayer_patch(AnimClass* pThis, void* _);

	static HouseClass* __fastcall GetOwningHouse_Wrapper(AnimClass* pThis, void* _) {
		return pThis->Owner;
	}
};
