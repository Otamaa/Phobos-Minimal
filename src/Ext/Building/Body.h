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

#include <New/Entity/PrismForwarding.h>

class InfantryClass;
class BuildingExtData : public TechnoExtData
{
public:
	using base_type = BuildingClass;
	static COMPILETIMEEVAL const char* ClassName = "BuildingExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "BuildingClass";
	static COMPILETIMEEVAL unsigned Marker = UuidFirstPart<base_type>::value;
	static COMPILETIMEEVAL auto Marker_str = to_hex_string<Marker>();

public:

#pragma region ClassMember
	BuildingTypeExtData* Type;
	std::unique_ptr<PrismForwarding> MyPrismForwarding;
	bool DeployedTechno;
	int LimboID;
	int GrindingWeapon_LastFiredFrame;
	BuildingClass* CurrentAirFactory;
	int AccumulatedIncome;
	bool IsCreatedFromMapFile;
	std::vector<AnimClass*> DamageFireAnims;
	CDTimerClass AutoSellTimer;
	bool LighningNeedUpdate;
	bool TogglePower_HasPower;
	bool Silent;
	HouseClass* C4Owner;
	WarheadTypeClass* C4Warhead;
	std::vector<int> DockReloadTimers;
	HouseClass* OwnerBeforeRaid;
	std::array<CDTimerClass, 3u> CashUpgradeTimers;
	int SensorArrayActiveCounter;
	bool SecretLab_Placed;
	bool AboutToChronoshift;
	bool IsFromSW;
	HelperedVector<TechnoClass*> RegisteredJammers;
	int GrindingWeapon_AccumulatedCredits;
	int LastFlameSpawnFrame;
	Handle<AnimClass*, UninitAnim> SpyEffectAnim;
	int SpyEffectAnimDuration;
	int PoweredUpToLevel;
	FactoryClass* FactoryBuildingMe;
	std::vector<BuildingClass*> airFactoryBuilding;
	bool FreeUnitDone;
	bool SeparateRepair;
#pragma endregion

	bool HasSuperWeapon(int index, bool withUpgrades) const;
	bool RubbleYell(bool beingRepaired) const;

	void DisplayIncomeString();
	void UpdatePoweredKillSpawns() const;
	void UpdateAutoSellTimer();
	void UpdateSpyEffecAnimDisplay();
	void UpdateMainEvaVoice();

public:

	BuildingExtData(BuildingClass* pObj)
		: TechnoExtData(pObj),
		Type(nullptr),
		MyPrismForwarding(nullptr),
		DeployedTechno(false),
		LimboID(-1),
		GrindingWeapon_LastFiredFrame(0),
		CurrentAirFactory(nullptr),
		AccumulatedIncome(0),
		IsCreatedFromMapFile(false),
		DamageFireAnims(),
		AutoSellTimer(),
		LighningNeedUpdate(false),
		TogglePower_HasPower(true),
		Silent(false),
		C4Owner(nullptr),
		C4Warhead(nullptr),
		DockReloadTimers(),
		OwnerBeforeRaid(nullptr),
		CashUpgradeTimers(),
		SensorArrayActiveCounter(0),
		SecretLab_Placed(false),
		AboutToChronoshift(false),
		IsFromSW(false),
		RegisteredJammers(),
		GrindingWeapon_AccumulatedCredits(0),
		LastFlameSpawnFrame(0),
		SpyEffectAnim(nullptr),
		SpyEffectAnimDuration(0),
		PoweredUpToLevel(0),
		FactoryBuildingMe(nullptr),
		airFactoryBuilding(),
		FreeUnitDone(false),
		SeparateRepair(false)
	{
		this->CurrentType = pObj->Type;
		this->Type = BuildingTypeExtContainer::Instance.Find(pObj->Type);
		this->Name = pObj->Type->ID;
		this->AbsType = BuildingClass::AbsID;
	}

	BuildingExtData(BuildingClass* pObj, noinit_t nn) : TechnoExtData(pObj, nn) { }

	virtual ~BuildingExtData();

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override;

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

	BuildingClass* This() const { return reinterpret_cast<BuildingClass*>(this->AttachedToObject); }
	const BuildingClass* This_Const() const { return reinterpret_cast<const BuildingClass*>(this->AttachedToObject); }

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
	static void ApplyLimboKill(ValueableVector<int>& LimboIDs, Valueable<AffectedHouse>& Affects, HouseClass* pTargetHouse, HouseClass* pAttackerHouse);

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

		if (!Unsorted::ArmageddonMode)
		{
			pDisplay->SetCursorShape2(nullptr);
			pDisplay->CurrentBuildingCopy = nullptr;
			pDisplay->CurrentBuildingTypeCopy = nullptr;
		}
	}

	template <bool slam = false>
	static inline void PlayConstructionYardAnim(BuildingClass* const pFactory)
	{
		const auto pFactoryType = pFactory->Type;

		if (pFactoryType->ConstructionYard)
		{
			if constexpr (slam)
				VocClass::PlayGlobal(BuildingTypeExtContainer::Instance.Find(pFactoryType)->SlamSound.
					Get(RulesClass::Instance->BuildingSlam), Panning::Center, 1.0);

			pFactory->DestroyNthAnim(BuildingAnimSlot::PreProduction);
			pFactory->DestroyNthAnim(BuildingAnimSlot::Idle);

			const bool damaged = pFactory->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow;
			const auto pAnimName = damaged ? pFactoryType->BuildingAnim[8].Damaged : pFactoryType->BuildingAnim[8].Anim;

			if (pAnimName && *pAnimName)
				pFactory->PlayAnim(pAnimName, BuildingAnimSlot::Production, damaged, false);
		}
	}

	static bool CheckBuildingFoundation(BuildingTypeClass* const pBuildingType, const CellStruct topLeftCell, HouseClass* const pHouse, bool& noOccupy);
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

	virtual bool LoadAll(const json& root);
	virtual bool SaveAll(json& root);
};

class NOVTABLE FakeBuildingClass : public BuildingClass
{
public:
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
		return *reinterpret_cast<TechnoExtData**>(((TechnoExtData*)this));
	}

	FORCEDINLINE const TechnoExtData* _GetTechnoExtData() const {
		return *reinterpret_cast<const TechnoExtData**>(((TechnoExtData*)this));
	}

	FORCEDINLINE BuildingTypeExtData* _GetTypeExtData() {
		return ((FakeBuildingTypeClass*)this->Type)->_GetExtData();
	}
};
static_assert(sizeof(FakeBuildingClass) == sizeof(BuildingClass), "Invalid Size !");
