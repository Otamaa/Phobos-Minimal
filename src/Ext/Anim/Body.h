#pragma once
#include <AnimClass.h>
#include <ParticleSystemClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <Ext/AnimType/Body.h>
#include "AnimSpawner.h"

class AnimExt
{
public:
	static constexpr size_t Canary = 0xAAAAAAAA;
	using base_type = AnimClass;
//#ifdef ENABLE_NEWEXT
	static constexpr size_t ExtOffset = 0xD0;
//#endif

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
			, AttachedSystem {}
			, OwnerSet { false } 
			//, SpawnData { }
			, ParentBuilding {}
		{ 
			//SpawnData = std::make_unique<AnimSpawner>(OwnerObject);
		}

		virtual ~ExtData()
		{
			DeleteAttachedSystem();
		}

		void InvalidatePointer(void* const ptr, bool bRemoved);
		void InitializeConstants();
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

		void CreateAttachedSystem(AnimTypeExt::ExtData* pData = nullptr);
		void DeleteAttachedSystem();

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<AnimExt
//#ifdef ENABLE_NEWEXT
, true
, true
//#endif
	>
	{
	public:
		ExtContainer();
		~ExtContainer();
#ifdef ENABLE_NEWEXT
		bool InvalidateExtDataIgnorable(void* const ptr) const
		{
			auto const abs = static_cast<AbstractClass*>(ptr)->WhatAmI();
			switch (abs)
			{
			case AbstractType::Building:
			case AbstractType::Infantry:
			case AbstractType::Unit:
			case AbstractType::Aircraft:
			case AbstractType::ParticleSystem:
				return false;
			default:
				return true;
			}

		}
#endif
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
	static const bool SetAnimOwnerHouseKind(AnimClass* pAnim,AnimTypeExt::ExtData* pExt, HouseClass* pInvoker, HouseClass* pVictim, bool defaultToVictimOwner = true);
	static const bool SetAnimOwnerHouseKind(AnimClass* pAnim, HouseClass* pInvoker, HouseClass* pVictim, bool defaultToVictimOwner = true);
	static const bool SetAnimOwnerHouseKind(AnimClass* pAnim, HouseClass* pInvoker, HouseClass* pVictim,TechnoClass* pTechnoInvoker, bool defaultToVictimOwner = true);
	static TechnoClass* GetTechnoInvoker(const AnimClass* const pThis ,bool DealthByOwner);
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
