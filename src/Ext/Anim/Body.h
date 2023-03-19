#pragma once
#include <AnimClass.h>

#include <Helpers/Macro.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDef.h>

#include "AnimSpawner.h"

class ParticleSystemClass;
class AnimExt
{
public:
	using base_type = AnimClass;
	static constexpr size_t Canary = 0xAAAAAAAA;
	//static constexpr size_t ExtOffset = 0xD0;

	class ExtData final : public Extension<base_type>
	{
	public:
		CoordStruct Something;
		OptionalStruct<short , true> DeathUnitFacing;
		OptionalStruct<DirStruct, true> DeathUnitTurretFacing;
		TechnoClass* Invoker;
		ParticleSystemClass* AttachedSystem;
		bool OwnerSet;
		//std::unique_ptr<AnimSpawner> SpawnData;
		
		// This is a failsafe that is only set if this is a building animation 
		// and the building is not on same cell as the animation.
		BuildingClass* ParentBuilding;
		ExtData(base_type* OwnerObject) : Extension<base_type>(OwnerObject)
			, Something { 0,0,0 }
			, DeathUnitFacing { }
			, DeathUnitTurretFacing { }
			, Invoker { nullptr }
			, AttachedSystem { nullptr }
			, OwnerSet { false } 
			//, SpawnData { }
			, ParentBuilding {}
		{ 
			//SpawnData = std::make_unique<AnimSpawner>(OwnerObject);
		}

		virtual ~ExtData() override { DeleteAttachedSystem(); }
		virtual void InvalidatePointer(void* const ptr, bool bRemoved) override;
		virtual bool InvalidateIgnorable(void* const ptr) const override;
		virtual void InitializeConstants() override;
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

		void CreateAttachedSystem();
		void DeleteAttachedSystem();

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<AnimExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
	static const bool SetAnimOwnerHouseKind(AnimClass* pAnim, HouseClass* pInvoker, HouseClass* pVictim, bool defaultToVictimOwner = true);
	static const bool SetAnimOwnerHouseKind(AnimClass* pAnim, HouseClass* pInvoker, HouseClass* pVictim,TechnoClass* pTechnoInvoker, bool defaultToVictimOwner = true);
	static TechnoClass* GetTechnoInvoker(AnimClass* pThis ,bool DealthByOwner);
	static AbstractClass* GetTarget(AnimClass* const);

	static bool DealDamageDelay(AnimClass* pThis);
	static bool OnExpired(AnimClass* pThis, bool LandIsWater, bool EligibleHeight);
	static bool OnMiddle(AnimClass* pThis);
	static bool OnMiddle_SpawnParticle(AnimClass* pThis, CellClass* pCell, Point2D nOffs);
	static void OnInit(AnimClass* pThis, CoordStruct* pCoord);

	//ToDo :
	// utilize this ,..
	// here as dummy atm
	//struct AnimCellUpdater
	//{
	//	static std::vector<CellClass*> Marked;

	//	static void Update() { }

	//	static void Invalidate() { }

	//	static void Print() {
	//		for (auto const& pair : Marked) {
	//			auto nCoord = pair->GetCoords();
	//			Debug::Log("Cell[%x] at [%d,%d,%d] Has Twinkle Anim ! \n", pair , nCoord.X , nCoord.Y , nCoord.Z);
	//		}
	//	}

	//	static void Clear() {
	//		Marked.clear();
	//	}
	//};

	static Layer __fastcall GetLayer_patch(AnimClass* pThis, void* _);

	static HouseClass* __fastcall GetOwningHouse_Wrapper(AnimClass* pThis, void* _)
	{
		return pThis->Owner;
	}

};
