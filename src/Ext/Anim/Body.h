#pragma once
#include <AnimClass.h>
#include <ParticleSystemClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <Ext/AnimType/Body.h>

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

		ExtData(base_type* OwnerObject) : Extension<base_type>(OwnerObject)
			, Something { 0,0,0 }
			, DeathUnitFacing { }
			, DeathUnitTurretFacing { }
			, Invoker { nullptr }
			, AttachedSystem {}
		{ }

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

	static AnimExt::ExtData* GetExtData(base_type* pThis);

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
#ifndef ENABLE_NEWEXT
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
	struct AnimCellUpdater
	{
		static std::vector<CellClass*> Marked;

		static void Update() { }

		static void Invalidate() { }

		static void Print() {
			for (auto const& pair : Marked) {
				auto nCoord = pair->GetCoords();
				Debug::Log("Cell[%x] at [%d,%d,%d] Has Twinkle Anim ! \n", pair , nCoord.X , nCoord.Y , nCoord.Z);
			}
		}

		static void Clear() {
			Marked.clear();
		}
	};

	static Layer __fastcall GetLayer_patch(AnimClass* pThis, void* _)
	{
		if (pThis->OwnerObject) {
			const auto pExt = AnimTypeExt::ExtMap.Find(pThis->Type);

			if (!pExt || !pExt->Layer_UseObjectLayer.isset()) {
				return Layer::Ground;
			}

			if (pExt->Layer_UseObjectLayer.Get()) {
				if (auto const pFoot = generic_cast<FootClass*>(pThis->OwnerObject)) {
					if (auto const pLocomotor = pFoot->Locomotor.get())
						return pLocomotor->In_Which_Layer();
				}
				else if (auto const pBullet = specific_cast<BulletClass*>(pThis->OwnerObject))
					return pBullet->InWhichLayer();

				return pThis->OwnerObject->ObjectClass::InWhichLayer();
			}
		}

		return pThis->Type ? pThis->Type->Layer : Layer::Air;
	}

	static HouseClass* __fastcall GetOwningHouse_Wrapper(AnimClass* pThis, void* _)
	{
		return pThis->Owner;
	}

};
