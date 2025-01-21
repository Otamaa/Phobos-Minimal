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
	static COMPILETIMEEVAL size_t Canary = 0x87654321;
	using base_type = BuildingClass;

	static COMPILETIMEEVAL size_t ExtOffset = 0x71C; //ares

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };

public:
	BuildingTypeExtData* Type { nullptr };
	TechnoExtData* TechnoExt { nullptr };
	std::unique_ptr<PrismForwarding> MyPrismForwarding {};
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
	int PoweredUpToLevel { 0 }; // Distinct from UpgradeLevel,
	//and set to highest PowersUpToLevel out of
	//applied upgrades regardless of how many are currently applied to this building.

	FactoryClass* FactoryBuildingMe {};

	~BuildingExtData() noexcept
	{
		this->SpyEffectAnim.SetDestroyCondition(!Phobos::Otamaa::ExeTerminated);
	}

	void InitializeConstant();
	void InvalidatePointer(AbstractClass* ptr, bool bRemoved);
	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	bool HasSuperWeapon(int index, bool withUpgrades) const;
	bool RubbleYell(bool beingRepaired) const;

	void DisplayIncomeString();
	void UpdatePoweredKillSpawns() const;
	void UpdateAutoSellTimer();
	void UpdateSpyEffecAnimDisplay();

	COMPILETIMEEVAL FORCEDINLINE static size_t size_Of()
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

	static bool CheckOccupierCanLeave(HouseClass* pBuildingHouse, HouseClass* pOccupierHouse);
	static bool CleanUpBuildingSpace(BuildingTypeClass* pBuildingType, CellStruct topLeftCell, HouseClass* pHouse, TechnoClass* pExceptTechno = nullptr);

private:
	template <typename T>
	void Serialize(T& Stm);
};

class BuildingExtContainer final : public Container<BuildingExtData>
{
public:
	OPTIONALINLINE static std::vector<BuildingExtData*> Pool;
	static BuildingExtContainer Instance;

	BuildingExtData* AllocateUnchecked(BuildingClass* key)
	{
		BuildingExtData* val = nullptr;
		if (!Pool.empty()) {
			val = Pool.front();
			Pool.erase(Pool.begin());
			//re-init
		} else {
			val = DLLAllocWithoutCTOR<BuildingExtData>();
		}

		if (val)
		{
			val->BuildingExtData::BuildingExtData();
			val->AttachedToObject = key;
			val->InitializeConstant();
			return val;
		}

		return nullptr;
	}

	BuildingExtData* Allocate(BuildingClass* key)
	{
		if (!key || Phobos::Otamaa::DoingLoadGame)
			return nullptr;

		this->ClearExtAttribute(key);

		if (BuildingExtData* val = AllocateUnchecked(key))
		{
			this->SetExtAttribute(key, val);
			return val;
		}

		return nullptr;
	}

	void Remove(BuildingClass* key)
	{
		if (BuildingExtData* Item = TryFind(key))
		{
			Item->~BuildingExtData();
			Item->AttachedToObject = nullptr;
			Pool.push_back(Item);
			this->ClearExtAttribute(key);
		}
	}

	void Clear()
	{
		if (!Pool.empty())
		{
			auto ptr = Pool.front();
			Pool.erase(Pool.begin());
			if (ptr)
			{
				delete ptr;
			}
		}
	}

	//CONSTEXPR_NOCOPY_CLASSB(BuildingExtContainer , BuildingExtData, "BuildingClass");
};

class FakeBuildingClass : public BuildingClass
{
public:
	void _Detach(AbstractClass* target, bool all);
	bool _IsFactory();
	CoordStruct* _GetFLH(CoordStruct* pCrd, int weaponIndex);
	int _Mission_Missile();
	void _Spawn_Refinery_Smoke_Particles();
	bool _SetOwningHouse(HouseClass* pHouse, bool announce)
	{
		const bool res = this->BuildingClass::SetOwningHouse(pHouse, announce);

		// Fix : update powered anims
		if (res && (this->Type->Powered || this->Type->PoweredSpecial))
			this->UpdatePowerDown();

		return res;
	}

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);

	FORCEDINLINE BuildingClass* _AsBuilding() const {
		return (BuildingClass*)this;
	}

	FORCEDINLINE BuildingExtData* _GetExtData() {
		return *reinterpret_cast<BuildingExtData**>(((DWORD)this) + BuildingExtData::ExtOffset);
	}

	FORCEDINLINE BuildingTypeExtData* _GetTypeExtData() {
		return ((FakeBuildingTypeClass*)this->Type)->_GetExtData();
	}
};
static_assert(sizeof(FakeBuildingClass) == sizeof(BuildingClass), "Invalid Size !");
