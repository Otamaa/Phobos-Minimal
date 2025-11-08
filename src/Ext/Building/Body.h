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
	static constexpr unsigned Marker = UuidFirstPart<base_type>::value;

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
	OptionalStruct<int, true> C4Damage;
	HouseClass* C4Owner;
	WarheadTypeClass* C4Warhead;
	WarheadTypeClass* ReceiveDamageWarhead;
	std::vector<int> DockReloadTimers;
	HouseClass* OwnerBeforeRaid;
	std::array<CDTimerClass, 3u> CashUpgradeTimers;
	int SensorArrayActiveCounter;
	bool SecretLab_Placed;
	bool AboutToChronoshift;
	bool IsFromSW;
	bool BeignMCEd; //this tag only use to fix
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
		C4Damage(),
		C4Owner(nullptr),
		C4Warhead(nullptr),
		ReceiveDamageWarhead(nullptr),
		DockReloadTimers(),
		OwnerBeforeRaid(nullptr),
		CashUpgradeTimers(),
		SensorArrayActiveCounter(0),
		SecretLab_Placed(false),
		AboutToChronoshift(false),
		IsFromSW(false),
		BeignMCEd(true),
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

	virtual BuildingClass* This() const override { return reinterpret_cast<BuildingClass*>(this->TechnoExtData::This()); }
	virtual const BuildingClass* This_Const() const override { return reinterpret_cast<const BuildingClass*>(this->TechnoExtData::This_Const()); }

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

private:
	template <typename T>
	void Serialize(T& Stm);
};

class BuildingExtContainer final : public Container<BuildingExtData>
{
public:
	static BuildingExtContainer Instance;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static void InvalidatePointer(AbstractClass* const ptr, bool bRemoved)
	{
		for (auto& ext : Array) {
			ext->InvalidatePointer(ptr, bRemoved);
		}
	}
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
		Point2D z_shape_offs,
		BlitterFlags flags);

	bool _SetOwningHouse(HouseClass* pHouse, bool announce)
	{
		const bool res = this->BuildingClass::SetOwningHouse(pHouse, announce);

		// Fix : update powered anims
		if (res && (this->Type->Powered || this->Type->PoweredSpecial))
			this->UpdatePowerDown();

		return res;
	}

	void _OnFireAI();
	void _DrawVisible(Point2D* pLocation , RectangleStruct* pBounds);
	void _DrawStuffsWhenSelected(Point2D* pPoint, Point2D* pOriginalPoint, RectangleStruct* pRect);

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, BOOL clearDirty);

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
