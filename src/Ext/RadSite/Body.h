#pragma once

#include <RadSiteClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <Ext/WeaponType/Body.h>

class RadTypeClass;

class RadSiteExt
{
public:
	static constexpr size_t Canary = 0x87654321;
	using base_type = RadSiteClass;
#ifdef ENABLE_NEWHOOKS
	static constexpr size_t ExtOffset = sizeof(base_type);
#endif

	class ExtData final : public Extension<base_type>
	{
	public:
		RadTypeClass* Type;
		WeaponTypeClass* Weapon;
		TechnoClass* TechOwner;
		bool NoOwner;

		ExtData(base_type* OwnerObject) : Extension<base_type>(OwnerObject)
			, Type { nullptr }
			, Weapon { nullptr }
			, TechOwner { nullptr }
			, NoOwner { true }
		{}

		virtual ~ExtData() = default;
		virtual void InvalidatePointer(void* ptr, bool bRemoved);

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		virtual void InitializeConstants() override;
		virtual void Uninitialize() override { }

		void CreateLight();
		void Add(int amount);
		void SetRadLevel(int amount);
		const double GetRadLevelAt(CellStruct const& cell);

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	static void CreateInstance(const CellStruct& location, int spread, int amount, WeaponTypeExt::ExtData* pWeaponExt , TechnoClass* const pTech);

	class ExtContainer final : public Container<RadSiteExt>
	{
	public:
		ExtContainer();
		~ExtContainer();

		virtual bool InvalidateExtDataIgnorable(void* ptr) const override {
			auto const abs = static_cast<AbstractClass*>(ptr)->WhatAmI();
			switch (abs)
			{
			case AbstractType::Building:
			case AbstractType::Aircraft:
			case AbstractType::Unit:
			case AbstractType::Infantry: {
				return false;
			}
			}
			return true;
		}
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;
	};

	static ExtContainer ExtMap;

	static ExtData* GetExtData(base_type const* pTr);

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static CoordStruct* __fastcall GetAltCoords_Wrapper(RadSiteClass* pThis, void* _ ,CoordStruct* pBuffer) {
		 Map.TryGetCellAt(pThis->BaseCell)->GetCoords(pBuffer);
		 return pBuffer;
	}
};
