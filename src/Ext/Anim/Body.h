#pragma once
#include <AnimClass.h>

#include <Helpers/Macro.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDef.h>

#include <Ext/AnimType/Body.h>

class AnimExt
{
public:
	using base_type = AnimClass;

	class ExtData final : public TExtension<base_type>
	{
	public:
		short DeathUnitFacing;
		DirStruct DeathUnitTurretFacing;
		bool FromDeathUnit;
		bool DeathUnitHasTurret;
		TechnoClass* Invoker;

		ExtData(base_type* OwnerObject) : TExtension<base_type>(OwnerObject)
			, DeathUnitFacing { 0 }
			, DeathUnitTurretFacing {}
			, FromDeathUnit { false }
			, DeathUnitHasTurret { false }
			, Invoker { nullptr }
		{}

		virtual ~ExtData() = default;
		virtual size_t GetSize() const override { return sizeof(*this); }
		virtual void InvalidatePointer(void* const ptr, bool bRemoved) override
		{

			if (Invoker == ptr)
				Invoker = nullptr;

		}

		virtual void LoadFromStream(PhobosStreamReader& Stm) override { this->Serialize(Stm); }
		virtual void SaveToStream(PhobosStreamWriter& Stm) override { this->Serialize(Stm); }


	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	__declspec(noinline) static AnimExt::ExtData* GetExtData(base_type* pThis)
	{
		return pThis && pThis->WhatAmI() == AbstractType::Anim ? reinterpret_cast<AnimExt::ExtData*>
			(ExtensionWrapper::GetWrapper(pThis)->ExtensionObject) : nullptr;
	}

	class ExtContainer final : public TExtensionContainer<AnimExt>
	{
	public:
		ExtContainer();
		~ExtContainer();

	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static const bool SetAnimOwnerHouseKind(AnimClass* pAnim, HouseClass* pInvoker, HouseClass* pVictim, bool defaultToVictimOwner = true);
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
};
