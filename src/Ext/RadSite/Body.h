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
#ifndef ENABLE_NEWHOOKS
	static constexpr size_t ExtOffset = 0x44;
#endif

	class ExtData final : public Extension<base_type>
	{
	public:
		RadTypeClass* Type;
		WeaponTypeClass* Weapon;
		TechnoClass* TechOwner;
		bool NoOwner;
		int Spread;

		ExtData(base_type* OwnerObject) : Extension<base_type>(OwnerObject)
			, Type { nullptr }
			, Weapon { nullptr }
			, TechOwner { nullptr }
			, NoOwner { true }
			, Spread { 0 }
		{}

		virtual ~ExtData() = default;
		void InvalidatePointer(void* ptr, bool bRemoved);

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		void InitializeConstants();
		void Uninitialize() { }

		void CreateLight();
		void Add(int amount);
		void SetRadLevel(int amount);
		const double GetRadLevelAt(CellStruct const& cell);

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	static void CreateInstance(const CellStruct& location, int spread, int amount, WeaponTypeExt::ExtData* pWeaponExt , TechnoClass* const pTech);

	class ExtContainer final : public Container<RadSiteExt,true,true , true>
	{
	public:
		ExtContainer();
		~ExtContainer();

	};

	static ExtContainer ExtMap;

	static ExtData* GetExtData(base_type const* pTr);

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static CoordStruct __fastcall GetAltCoords_Wrapper(RadSiteClass* pThis, void* _) {
		 return Map.TryGetCellAt(pThis->BaseCell)->GetCoords();
	}
};
