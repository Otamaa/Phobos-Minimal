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
#include <Ext/Building/Body.h>

class BuildingExt
{
public:
	static constexpr size_t Canary = 0x87654321;
	using base_type = BuildingClass;
	//static constexpr size_t ExtOffset = 0x6FC;

	class ExtData final : public Extension<BuildingClass>
	{
	public:

		bool DeployedTechno;
		int LimboID;
		int GrindingWeapon_LastFiredFrame;
		BuildingClass* CurrentAirFactory;
		int AccumulatedGrindingRefund;

		DynamicVectorClass<AnimClass*> DamageFireAnims;
		TimerStruct AutoSellTimer;
		bool LighningNeedUpdate;

		ExtData(BuildingClass* OwnerObject) : Extension<BuildingClass>(OwnerObject)
			, DeployedTechno { false }
			, LimboID { -1 }
			, GrindingWeapon_LastFiredFrame { 0 }
			, CurrentAirFactory { nullptr }
			, AccumulatedGrindingRefund { 0 }
			, DamageFireAnims { }
			, AutoSellTimer { }
			, LighningNeedUpdate { false }
		{ }

		virtual ~ExtData() override = default;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;
		virtual bool InvalidateIgnorable(void* const ptr) const override;
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		virtual void InitializeConstants() override;

		bool HasSuperWeapon(int index, bool withUpgrades) const;
		bool RubbleYell(bool beingRepaired);


		void UpdatePoweredKillSpawns();
		void UpdateAutoSellTimer();

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<BuildingExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

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
};