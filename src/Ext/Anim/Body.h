#pragma once
#include <AnimClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <Ext/AnimType/Body.h>

class AnimExt
{
public:
	using base_type = AnimClass;

	class ExtData final : public Extension<base_type>
	{
	public:
		short DeathUnitFacing;
		DirStruct DeathUnitTurretFacing;
		bool FromDeathUnit;
		bool DeathUnitHasTurret;
		TechnoClass* Invoker;

		ExtData(base_type* OwnerObject) : Extension<base_type>(OwnerObject)
			, DeathUnitFacing { 0 }
			, DeathUnitTurretFacing {}
			, FromDeathUnit { false }
			, DeathUnitHasTurret { false }
			, Invoker { nullptr }
		{}

		virtual ~ExtData() override = default;
		//virtual size_t GetSize() const override { return sizeof(*this); }
		virtual void InvalidatePointer(void* const ptr, bool bRemoved) override
		{
			if (Invoker == ptr)
				Invoker = nullptr;

		}

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;


	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	static AnimExt::ExtData* GetExtData(base_type* pThis);

	class ExtContainer final : public Container<AnimExt>
	{
	public:
		ExtContainer();
		~ExtContainer();

		virtual bool InvalidateExtDataIgnorable(void* const ptr) const override
		{
			auto const abs = static_cast<AbstractClass*>(ptr)->WhatAmI();
			switch (abs)
			{
			case AbstractType::Building:
			case AbstractType::Infantry:
			case AbstractType::Unit:
			case AbstractType::Aircraft:
				return false;
			default:
				return true;
			}
		}
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
	static const bool SetAnimOwnerHouseKind(AnimClass* pAnim,AnimTypeExt::ExtData* pExt, HouseClass* pInvoker, HouseClass* pVictim, bool defaultToVictimOwner = true);
	static const bool SetAnimOwnerHouseKind(AnimClass* pAnim, HouseClass* pInvoker, HouseClass* pVictim, bool defaultToVictimOwner = true);
	static const bool SetAnimOwnerHouseKind(AnimClass* pAnim, HouseClass* pInvoker, HouseClass* pVictim,TechnoClass* pTechnoInvoker, bool defaultToVictimOwner = true);
	static TechnoClass* GetTechnoInvoker(AnimClass* pThis,bool DealthByOwner);
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
					if (auto pLocomotor = pFoot->Locomotor.get())
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
