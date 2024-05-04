#pragma once
#include <BuildingClass.h>
#include <HouseClass.h>
#include <TiberiumClass.h>
#include <FactoryClass.h>

#include <Helpers/Macro.h>
#include <Utilities/TemplateDef.h>

#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/BuildingType/Body.h>

#include <Misc/Ares/Hooks/Classes/PrismForwarding.h>

class BuildingExtData final
{
public:
	static constexpr size_t Canary = 0x87654321;
	using base_type = BuildingClass;

#ifndef aaa
	static constexpr size_t ExtOffset = 0x71C; //ares
#endif

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };

public:
	BuildingTypeExtData* Type { nullptr };
	TechnoExtData* TechnoExt { nullptr };
	PrismForwarding PrismForwarding {};
	bool DeployedTechno { false };
	int LimboID { -1 };
	int GrindingWeapon_LastFiredFrame { 0 };
	BuildingClass* CurrentAirFactory { nullptr };
	int AccumulatedIncome { 0 };
	bool IsCreatedFromMapFile { false };

	std::vector<AnimClass*> DamageFireAnims { };
	CDTimerClass AutoSellTimer { };
	bool LighningNeedUpdate { false };
	bool TogglePower_HasPower { true };
	bool Silent { false };

	OptionalStruct<int, true> C4Damage { };
	HouseClass* C4Owner { nullptr };
	WarheadTypeClass* C4Warhead { nullptr };
	WarheadTypeClass* ReceiveDamageWarhead { nullptr };
	std::vector<int> DockReloadTimers {};
	HouseClass* OwnerBeforeRaid { nullptr };

	CDTimerClass CashUpgradeTimers[3] {};
	int SensorArrayActiveCounter { 0 };
	std::vector<bool> StartupCashDelivered {};
	bool SecretLab_Placed { false };
	bool AboutToChronoshift { false };
	bool IsFromSW { false };
	bool BeignMCEd { true }; //this tag only use to fix
	//https://github.com/Phobos-developers/Phobos/issues/1146

	HelperedVector<TechnoClass*> RegisteredJammers { };
	int GrindingWeapon_AccumulatedCredits { 0 };
	int LastFlameSpawnFrame { 0 };

	Handle<AnimClass*, UninitAnim> SpyEffectAnim { nullptr };
	int SpyEffectAnimDuration {};

	BuildingExtData() noexcept = default;
	~BuildingExtData() noexcept = default;

	void InitializeConstant();
	void InvalidatePointer(AbstractClass* ptr, bool bRemoved);
	static bool InvalidateIgnorable(AbstractClass* ptr);
	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	bool HasSuperWeapon(int index, bool withUpgrades) const;
	bool RubbleYell(bool beingRepaired) const;

	void DisplayIncomeString();
	void UpdatePoweredKillSpawns() const;
	void UpdateAutoSellTimer();
	void UpdateSpyEffecAnimDisplay();

	constexpr FORCEINLINE static size_t size_Of()
	{
		return sizeof(BuildingExtData) -
			(4u //AttachedToObject
			 );
	}

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
	static void ApplyLimboKill(ValueableVector<int>& LimboIDs, Valueable<AffectedHouse>& Affects, HouseClass* pTargetHouse, HouseClass* pAttackerHouse);

	static int GetFirstSuperWeaponIndex(BuildingClass* pThis);
	static SuperClass* GetFirstSuperWeapon(BuildingClass* pThis);
	static void UpdateSecretLab(BuildingClass* pThis);
	static bool ReverseEngineer(BuildingClass* pBuilding, TechnoClass* Victim);

	static const std::vector<CellStruct> GetFoundationCells(BuildingClass* pThis, CellStruct baseCoords, bool includeOccupyHeight = false);

private:
	template <typename T>
	void Serialize(T& Stm);
};

class BuildingExtContainer final : public Container<BuildingExtData>
{
public:
	static BuildingExtContainer Instance;

	CONSTEXPR_NOCOPY_CLASSB(BuildingExtContainer , BuildingExtData, "BuildingClass");
};