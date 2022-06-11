#pragma once
#include <SuperWeaponTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class SuperClass;
class SWTypeExt
{
public:
	using base_type = SuperWeaponTypeClass;

	class ExtData final : public Extension<SuperWeaponTypeClass>
	{
	public:

		Valueable<int> Money_Amount;
		Valueable<CSFText> UIDescription;
		Valueable<int> CameoPriority;
		ValueableVector<TechnoTypeClass*> LimboDelivery_Types;
		ValueableVector<int> LimboDelivery_IDs;
		ValueableVector<float> LimboDelivery_RollChances;
		Valueable<AffectedHouse> LimboKill_Affected;
		ValueableVector<int> LimboKill_IDs;
		Valueable<double> RandomBuffer;

		ValueableVector<ValueableVector<int>> LimboDelivery_RandomWeightsData;

		ValueableVector<TechnoTypeClass*> SW_Inhibitors;
		Valueable<bool> SW_AnyInhibitor;

		#pragma region Otamaa
		Nullable<SHPStruct*> GClock_Shape;
		Nullable<int> GClock_Transculency;
		CustomPalette GClock_Palette;
		Valueable<bool> ChargeTimer;
		Valueable<bool> ChargeTimer_Backwards;
		#pragma endregion

		ExtData(SuperWeaponTypeClass* OwnerObject) : Extension<SuperWeaponTypeClass>(OwnerObject)
			, Money_Amount { 0 }
			, UIDescription {}
			, CameoPriority { 0 }
			, LimboDelivery_Types {}
			, LimboDelivery_IDs {}
			, LimboDelivery_RollChances {}
			, LimboKill_Affected { AffectedHouse::Owner }
			, LimboKill_IDs {}
			, RandomBuffer { 0.0 }
			, LimboDelivery_RandomWeightsData {}

			, SW_Inhibitors {}
			, SW_AnyInhibitor { false }

			, GClock_Shape { }
			, GClock_Transculency { }
			, GClock_Palette { }
			, ChargeTimer { false }
			, ChargeTimer_Backwards { false }
		{ }


		void FireSuperWeapon(SuperClass* pSW, HouseClass* pHouse, CoordStruct coords);

		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual ~ExtData() = default;
		virtual size_t Size() const { return sizeof(*this); }
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;

		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
	private:
		void ApplyLimboDelivery(HouseClass* pHouse);
		void ApplyLimboKill(HouseClass* pHouse);

		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<SWTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static bool IsInhibitor(SWTypeExt::ExtData* pSWType, HouseClass* pOwner, TechnoClass* pTechno);
	static bool HasInhibitor(SWTypeExt::ExtData* pSWType, HouseClass* pOwner, const CellStruct& Coords);
	static bool IsInhibitorEligible(SWTypeExt::ExtData* pSWType, HouseClass* pOwner, const CellStruct& Coords, TechnoClass* pTechno);
};