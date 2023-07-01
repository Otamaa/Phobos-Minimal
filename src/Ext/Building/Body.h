#pragma once
#include <BuildingClass.h>
#include <HouseClass.h>
#include <TiberiumClass.h>
#include <FactoryClass.h>

#include <Helpers/Macro.h>
#include <Utilities/TemplateDef.h>

#include <Ext/Abstract/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/BuildingType/Body.h>

class BuildingExt
{
public:
	class ExtData final : public Extension<BuildingClass>
	{
	public:
		static constexpr size_t Canary = 0x87654321;
		using base_type = BuildingClass;
		//static constexpr size_t ExtOffset = 0x6FC;

	public:
		BuildingTypeExt::ExtData* Type { nullptr };
		TechnoExt::ExtData* TechnoExt { nullptr };
		bool DeployedTechno { false };
		int LimboID { -1 };
		int GrindingWeapon_LastFiredFrame { 0 };
		BuildingClass* CurrentAirFactory { nullptr };
		int AccumulatedGrindingRefund { 0 };
		int AccumulatedIncome { 0 };
		bool IsCreatedFromMapFile { false };

		std::vector<AnimClass*> DamageFireAnims { };
		CDTimerClass AutoSellTimer { };
		bool LighningNeedUpdate { false };
		bool TogglePower_HasPower { false };
		bool Silent { false };

		OptionalStruct<int , true> C4Damage { };
		HouseClass* C4Owner { nullptr };
		WarheadTypeClass* C4Warhead { nullptr };
		WarheadTypeClass* ReceiveDamageWarhead { nullptr };

		ExtData(BuildingClass* OwnerObject) : Extension<BuildingClass>(OwnerObject)	{ }
		virtual ~ExtData() override = default;

		void InvalidatePointer(void* ptr, bool bRemoved);
		bool InvalidateIgnorable(void* ptr) const;
		void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
		void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

		bool HasSuperWeapon(int index, bool withUpgrades) const;
		bool RubbleYell(bool beingRepaired);

		void DisplayIncomeString();
		void UpdatePoweredKillSpawns();
		void UpdateAutoSellTimer();

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<BuildingExt::ExtData>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static void StoreTiberium(BuildingClass* pThis, float amount, int idxTiberiumType, int idxStorageTiberiumType);
	static void UpdatePrimaryFactoryAI(BuildingClass* pThis);
	static int CountOccupiedDocks(BuildingClass* pBuilding);
	static bool HasFreeDocks(BuildingClass* pBuilding);
	static bool CanGrindTechno(BuildingClass* pBuilding, TechnoClass* pTechno);
	static bool DoGrindingExtras(BuildingClass* pBuilding, TechnoClass* pTechno, int nRefundAmounts);
	static CoordStruct GetCenterCoords(BuildingClass* pThis, bool includeBib = false);
	static bool HandleInfiltrate(BuildingClass* pBuilding, HouseClass* pInfiltratorHouse);
	static void LimboDeliver(BuildingTypeClass* pType, HouseClass* pOwner, int ID);
	static void LimboKill(BuildingClass* pBld);
	static void ApplyLimboKill(ValueableVector<int>& LimboIDs,Valueable<AffectedHouse>& Affects , HouseClass* pTargetHouse , HouseClass* pAttackerHouse);

	static int GetFirstSuperWeaponIndex(BuildingClass* pThis);
	static SuperClass* GetFirstSuperWeapon(BuildingClass* pThis);
};