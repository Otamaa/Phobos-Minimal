#pragma once
#include <BuildingClass.h>
#include <HouseClass.h>
#include <TiberiumClass.h>
#include <FactoryClass.h>

#include <Helpers/Macro.h>

#include <Utilities/TemplateDef.h>
#include <Utilities/PlacingBuildingStruct.h>

#include <Ext/Techno/Body.h>

#include <New/Entity/PrismForwarding.h>
#include <New/Entity/PowerPlantEnhancerClass.h>

class BuildingTypeExtData;
class InfantryClass;
class BuildingExtData : public TechnoExtData
{
public:
	using base_type = BuildingClass;
	static COMPILETIMEEVAL const char* ClassName = "BuildingExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "BuildingClass";
	

public:
#pragma region ClassMember

	// ============================================================
	// Custom Classes
	// ============================================================ 
	PowerPlantEnhancerClass PowerPlantEnhancer;

	// ============================================================
	// 8-byte aligned: Pointers
	// ============================================================
	BuildingClass* CurrentAirFactory {};
	HouseClass* C4Owner {};
	WarheadTypeClass* C4Warhead {};
	HouseClass* OwnerBeforeRaid {};
	FactoryClass* FactoryBuildingMe {};

	// ============================================================
	// 8-byte aligned: unique_ptr
	// ============================================================
	std::unique_ptr<PrismForwarding> MyPrismForwarding {};

	// ============================================================
	// 8-byte aligned: Handle wrapper
	// ============================================================
	Handle<AnimClass*, UninitAnim> SpyEffectAnim {};

	// ============================================================
	// 24-byte aligned: Vectors (grouped together)
	// ============================================================
	std::vector<AnimClass*> DamageFireAnims {};
	std::vector<int> DockReloadTimers {};
	HelperedVector<TechnoClass*> RegisteredJammers {};
	std::vector<BuildingClass*> airFactoryBuilding {};

	// ============================================================
	// Large compound: std::array of CDTimerClass
	// ============================================================
	std::array<CDTimerClass, 3u> CashUpgradeTimers {};

	// ============================================================
	// CDTimerClass
	// ============================================================
	CDTimerClass AutoSellTimer {};

	// ============================================================
	// 4-byte aligned: int
	// ============================================================
	int LimboID { -1 };
	int GrindingWeapon_LastFiredFrame {};
	int AccumulatedIncome {};
	int SensorArrayActiveCounter {};
	int GrindingWeapon_AccumulatedCredits {};
	int LastFlameSpawnFrame {};
	int SpyEffectAnimDuration {};
	int PoweredUpToLevel {};
	int TurretAnimIdleFrame {};
	int TurretAnimFiringFrame { -1 };

	// ============================================================
	// 1-byte aligned: bool (packed together at the end)
	// ============================================================
	bool DeployedTechno {};
	bool IsCreatedFromMapFile {};
	bool LighningNeedUpdate {};  // typo: should be "LightningNeedUpdate"
	bool TogglePower_HasPower { true };
	bool Silent {};
	bool SecretLab_Placed {};
	bool AboutToChronoshift {};
	bool IsFromSW {};
	bool FreeUnitDone {};
	bool SeparateRepair {};
	bool IsFiringNow {};
	// 11 bools = 11 bytes, pads to 12 for 4-byte alignment
#pragma endregion

public:
	BuildingExtData(BuildingClass* pObj);

	BuildingExtData(BuildingClass* pObj, noinit_t nn) : TechnoExtData(pObj, nn) { }

	virtual ~BuildingExtData();

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved, AbstractType  type) override;

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->TechnoExtData::LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		this->TechnoExtData::SaveToStream(Stm);
		const_cast<BuildingExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const {
		this->TechnoExtData::CalculateCRC(crc);
	}

	FORCEDINLINE BuildingClass* This() const { return reinterpret_cast<BuildingClass*>(this->AttachedToObject); }
	FORCEDINLINE const BuildingClass* This_Const() const { return reinterpret_cast<const BuildingClass*>(this->AttachedToObject); }
	FORCEDINLINE BuildingTypeExtData* GetTypeExtData() const { return ((BuildingTypeExtData*)TypeExtData); }

	bool HasSuperWeapon(int index, bool withUpgrades) const;
	bool RubbleYell(bool beingRepaired) const;

	void DisplayIncomeString();
	void UpdatePoweredKillSpawns() const;
	void UpdateAutoSellTimer();
	void UpdateSpyEffecAnimDisplay();
	void UpdateMainEvaVoice();

public:
	static void StoreTiberium(BuildingClass* pThis, float amount, int idxTiberiumType, int idxStorageTiberiumType);
	static void UpdatePrimaryFactoryAI(BuildingClass* pThis);
	static int CountOccupiedDocks(BuildingClass* pBuilding);
	static bool HasFreeDocks(BuildingClass* pBuilding);
	static bool CanGrindTechno(BuildingClass* pBuilding, TechnoClass* pTechno);
	static bool CanGrindTechnoSimplified(BuildingClass* pBuilding, TechnoClass* pTechno);
	static bool DoGrindingExtras(BuildingClass* pBuilding, TechnoClass* pTechno, int nRefundAmounts);
	static CoordStruct GetCenterCoords(BuildingClass* pThis, bool includeBib = false);
	static bool HandleInfiltrate(BuildingClass* pBuilding, HouseClass* pInfiltratorHouse);
	static void LimboDeliver(BuildingTypeClass* pType, HouseClass* pOwner, int ID);
	static void LimboKill(BuildingClass* pBld);

	static void ApplyLimboKill(int LimboID, int count, Valueable<AffectedHouse>& Affects, HouseClass* pTargetHouse, HouseClass* pAttackerHouse);

	static int GetFirstSuperWeaponIndex(BuildingClass* pThis);
	static SuperClass* GetFirstSuperWeapon(BuildingClass* pThis);
	static void UpdateSecretLab(BuildingClass* pThis);
	static bool ReverseEngineer(BuildingClass* pBuilding, TechnoClass* Victim);

	static const std::vector<CellStruct> GetFoundationCells(BuildingClass* pThis, CellStruct baseCoords, bool includeOccupyHeight = false);
	static bool BuildingHasPower(BuildingClass* pThis);
	static FacingType GetPoseDir(AircraftClass* pAir, BuildingClass* pBld);

	static void ClearPlacingBuildingData(PlacingBuildingStruct* const pPlace)
	{
		pPlace->Type = nullptr;
		pPlace->DrawType = nullptr;
		pPlace->Times = 0;
		pPlace->TopLeft = CellStruct::Empty;
		pPlace->Timer.Stop();
	}

	static void ClearCurrentBuildingData(DisplayClass* const pDisplay)
	{
		pDisplay->SetActiveFoundation(nullptr);
		pDisplay->CurrentBuilding = nullptr;
		pDisplay->CurrentBuildingType = nullptr;
		pDisplay->CurrentBuildingOwnerArrayIndexCopy = -1;

		if (!Unsorted::MAP_DEBUG_MODE.get())
		{
			pDisplay->SetCursorShape2(nullptr);
			pDisplay->CurrentBuildingCopy = nullptr;
			pDisplay->CurrentBuildingTypeCopy = nullptr;
		}
	}

	template <bool slam>
	static inline void PlayConstructionYardAnim(BuildingClass* const pFactory);

	static bool CheckBuildingFoundation(BuildingTypeClass* const pBuildingType, const CellStruct topLeftCell, HouseClass* const pHouse, bool& noOccupy);

	static DWORD GetFirewallFlags(BuildingClass* pThis);
	static void ImmolateVictims(TechnoClass* pThis);
	static bool ImmolateVictim(TechnoClass* pThis, ObjectClass* const pVictim, bool const destroy = true);
	static void UpdateFirewall(BuildingClass* pThis, bool const changedState);
	static void UpdateFirewallLinks(BuildingClass* pThis);
	static bool IsActiveFirestormWall(BuildingClass* const pBuilding, HouseClass const* const pIgnore);
	static bool sameTrench(BuildingClass* currentBuilding, BuildingClass* targetBuilding);
	static bool canLinkTo(BuildingClass* currentBuilding, BuildingClass* targetBuilding);
	static void BuildLines(BuildingClass* theBuilding, CellStruct selectedCell, HouseClass* buildingOwner);
	static int GetImageFrameIndex(BuildingClass* pThis);
	static int GetTurretFrame(BuildingClass* pThis);

private:
	template <typename T>
	void Serialize(T& Stm);
};

class BuildingExtContainer final : public Container<BuildingExtData>
{
public:

	static COMPILETIMEEVAL const char* ClassName = "BuildingExtContainer";

public:
	static BuildingExtContainer Instance;

	virtual bool LoadAll(const PhobosStreamReader& stm) { return true; }
	virtual bool SaveAll(PhobosStreamWriter& stm){ return true; }
};

class NOVTABLE FakeBuildingClass : public BuildingClass
{
public:

	int _Mission_Repair();
	void _Detach(AbstractClass* target, bool all);
	bool _IsFactory();
	int _Mission_Missile();
	void _Spawn_Refinery_Smoke_Particles();
	void _DetachAnim(AnimClass* pAnim);
	DamageState _ReceiveDamage(int* Damage, int DistanceToEpicenter , WarheadTypeClass* WH , TechnoClass* Attacker , bool IgnoreDefenses , bool PreventsPassengerEscape , HouseClass* SourceHouse);
	int _GetAirstrikeInvulnerabilityIntensity(int currentIntensity) const;
	void _OnFinishRepairB(InfantryClass* pEngineer);
	void _OnFinishRepair();
	void UnloadOccupants(bool assignMission, bool killIfStuck);
	void _Draw_It(Point2D* xdrawpoint, RectangleStruct* xcliprect);
	void _TechnoClass_Draw_Object(SHPStruct* shapefile,
		int shapenum,
		Point2D* xy,
		RectangleStruct* rect,
		DirType rotation,  //unused
		int scale, //unused
		int height_adjust,
		ZGradient a8,
		bool useZBuffer,
		int lightLevel,
		int tintLevel,
		SHPStruct* z_shape,
		int z_shape_framenum,
		int z_shape_offs_x,
		int z_shape_offs_y,
		BlitterFlags flags);

	void _DrawRadialIndicator(int val);
	int _BuildingClass_GetRangeOfRadial();
	int __GetPower();

	bool _SetOwningHouse(HouseClass* pHouse, bool announce)
	{
		const bool res = this->BuildingClass::SetOwningHouse(pHouse, announce);

		// If we're supposed to be playing buildup during/after owner change reset any changes to mission or BState made during owner change.
		if (res && this->CurrentMission == Mission::Construction && this->BState == BStateType::Construction) {
			this->IsReadyToCommence = false;
			this->QueueBState = BStateType::None;
			this->QueuedMission = Mission::None;
		}

		// Fix : update powered anims
		//if (res && (this->Type->Powered || this->Type->PoweredSpecial))
		//	this->UpdatePowerDown();

		return res;
	}

	void _OnFireAI();
	void _DrawExtras(Point2D* pLocation, RectangleStruct* pBounds);
	void _DrawVisible(Point2D* pLocation , RectangleStruct* pBounds);
	void _DrawStuffsWhenSelected(Point2D* pPoint, Point2D* pOriginalPoint, RectangleStruct* pRect);
	KickOutResult __ExitObject(TechnoClass* object, CellStruct exitCell);

	InfantryTypeClass* __GetCrew();
	int  __GetCrewCount();
	const wchar_t* __GetUIName();

	FORCEDINLINE BuildingClass* _AsBuilding() const {
		return (BuildingClass*)this;
	}

	FORCEDINLINE BuildingExtData* _GetExtData() {
		return *reinterpret_cast<BuildingExtData**>(((DWORD)this) + AbstractExtOffset);
	}

	FORCEDINLINE const BuildingExtData* _GetExtData() const{
		return *reinterpret_cast<const BuildingExtData**>(((DWORD)this) + AbstractExtOffset);
	}

	FORCEDINLINE TechnoExtData* _GetTechnoExtData() {
		return ((TechnoExtData*)this->_GetExtData());
	}

	FORCEDINLINE const TechnoExtData* _GetTechnoExtData() const {
		return ((const TechnoExtData*)this->_GetExtData());
	}

	FORCEDINLINE BuildingTypeExtData* _GetTypeExtData() {
		return ((BuildingTypeExtData*)(this->_GetExtData()->TypeExtData));
	}
};
static_assert(sizeof(FakeBuildingClass) == sizeof(BuildingClass), "Invalid Size !");
