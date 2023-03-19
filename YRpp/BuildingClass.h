/*
	Buildings
*/

#pragma once

#include <TechnoClass.h>
#include <BuildingTypeClass.h>
#include <BuildingLightClass.h>
#include <ProgressTimer.h>
#include <GeneralStructures.h>
#include <ArrayClasses.h>

class FactoryClass;
class InfantryClass;
class LightSourceClass;
class FoggedObjectClass;

enum class BStateType : int
{
	None = -1,
	Construction = 0,
	Idle = 1,
	Active = 2,
	Full = 3,
	Aux1 = 4,
	Aux2 = 5,
	Count = 6
};

class DECLSPEC_UUID("0E272DC6-9C0F-11D1-B709-00A024DDAFD1")
	NOVTABLE BuildingClass : public TechnoClass
{
public:
	static const AbstractType AbsID = AbstractType::Building;

	//Static
	static constexpr constant_ptr<DynamicVectorClass<BuildingClass*>, 0xA8EB40u> const Array{};
	static constexpr inline DWORD vtable = 0x7E3EBC;

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x459E80);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x453E20);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x454190);

	//Destructor
	virtual ~BuildingClass() override JMP_THIS(0x459F20);

	//AbstractClass
	virtual void Init() override JMP_THIS(0x442C40);
	virtual void PointerExpired(AbstractClass* pAbstract, bool removed) override JMP_THIS(0x44E8F0);
	virtual AbstractType WhatAmI() const override { return AbstractType::Building; }
	virtual int	Size() const override { return 0x720; }
	virtual void Update() override JMP_THIS(0x43FB20);

	//ObjectClass
	//MissionClass
	//TechnoClass
	virtual InfantryTypeClass* GetCrew() const override { JMP_THIS(0x44EB53); }
	virtual void Destroyed(ObjectClass* Killer) override JMP_THIS(0x44D760);
	virtual bool ForceCreate(CoordStruct& coord, DWORD dwUnk = 0) override JMP_THIS(0x458A80);

	//BuildingClass
	virtual CellStruct FindBuildingExitCell(TechnoClass* pDocker, CellStruct nDefault) const JMP_THIS(0x44EFB0);
	virtual int DistanceToDockingCoord(ObjectClass* pObj) const JMP_THIS(0x447E00);
	virtual void Place(bool captured) JMP_THIS(0x445F80);
	virtual void UpdateConstructionOptions() JMP_THIS(0x4456D0);
	virtual void Draw(const Point2D& point, const RectangleStruct& rect) JMP_THIS(0x43DA80);
	virtual DirStruct FireAngleTo(ObjectClass* pObject) const JMP_THIS(0x43ED40);
	virtual void Destroy(DWORD dwUnused, TechnoClass* pTechno, bool NoSurvivor, CellStruct& cell) JMP_THIS(0x4415F0);
	virtual bool TogglePrimaryFactory() JMP_THIS(0x448160);
	virtual void SensorArrayActivate(CellStruct cell=CellStruct::Empty) JMP_THIS(0x455820);
	virtual void SensorArrayDeactivate(CellStruct cell=CellStruct::Empty) JMP_THIS(0x4556D0);
	virtual void DisguiseDetectorActivate(CellStruct cell=CellStruct::Empty) JMP_THIS(0x455A80);
	virtual void DisguiseDetectorDeactivate(CellStruct cell=CellStruct::Empty) JMP_THIS(0x455980);
	virtual int Building_Class452250() R0;

	// non-vt

	int GetCurrentFrame() { JMP_THIS(0x43EF90); }

	bool IsAllFogged() const { JMP_THIS(0x457A10); }

	void SetRallypoint(CellStruct* pTarget, bool bPlayEVA)
		{ JMP_THIS(0x443860); }

	void FreezeInFog(DynamicVectorClass<FoggedObjectClass*>* pFoggedArray, CellClass* pCell, bool Visible)
		{ JMP_THIS(0x457AA0); }

	// power up
	void GoOnline()
		{ JMP_THIS(0x452260); }
	void GoOffline()
		{ JMP_THIS(0x452360); }

	int GetPowerOutput() const
		{ JMP_THIS(0x44E7B0); }

	int GetPowerDrain() const
		{ JMP_THIS(0x44E880); }

	// Firewall aka FirestormWall
	// depending on what facings of this building
	// are connected to another FWall,
	// returns the index of the image file
	// to draw.
	DWORD GetFWFlags() const
		{ JMP_THIS(0x455B90); }

	void CreateEndPost(bool arg)
		{ JMP_THIS(0x4533A0); }

	// kick out content
	void UnloadBunker()
		{ JMP_THIS(0x4593A0); }

	// content is dead - chronosphered away or died inside
	void ClearBunker()
		{ JMP_THIS(0x459470); }

	// kick out content, remove anims, etc... don't ask me what's different from kick out
	void EmptyBunker()
		{ JMP_THIS(0x4595C0); }

	// called after destruction - CrateBeneath, resetting foundation'ed cells
	void AfterDestruction()
		{ JMP_THIS(0x441F60); }

	// destroys the specific animation (active, turret, special, etc)
	void DestroyNthAnim(BuildingAnimSlot Slot)
		{ JMP_THIS(0x451E40); }

	void PlayNthAnim(BuildingAnimSlot Slot, int effectDelay = 0) {
		bool Damaged = !this->IsGreenHP();
		bool Garrisoned = this->GetOccupantCount() > 0;

		auto& AnimData = this->Type->GetBuildingAnim(Slot);
		const char *AnimName = nullptr;
		if(Damaged) {
			AnimName = AnimData.Damaged;
		} else if(Garrisoned) {
			AnimName = AnimData.Garrisoned;
		} else {
			AnimName = AnimData.Anim;
		}
		if(AnimName && *AnimName) {
			this->PlayAnim(AnimName, Slot, Damaged, Garrisoned, effectDelay);
		}
	}

	void PlayAnim(const char* animName, BuildingAnimSlot Slot, bool Damaged, bool Garrisoned, int effectDelay = 0)
		{ JMP_THIS(0x451890); }

	// changes between building's damaged and undamaged animations.
	void ToggleDamagedAnims(bool isDamaged)
		{ JMP_THIS(0x451EE0); }

	// when the building is switched off
	void DisableStuff()
		{ JMP_THIS(0x452480); }

	// when the building is switched on
	void EnableStuff()
		{ JMP_THIS(0x452410); }

	// when the building is warped
	void DisableTemporal()
		{ JMP_THIS(0x4521C0); }

	// when the building warped back in
	void EnableTemporal()
		{ JMP_THIS(0x452210); }

	// returns Type->SuperWeapon, if its AuxBuilding is satisfied
	int FirstActiveSWIdx() const
		{ JMP_THIS(0x457630); }

	int GetShapeNumber() const
		{ JMP_THIS(0x43EF90); }

	void BeginMode(BStateType bType)
		{ JMP_THIS(0x447780); }

	// returns Type->SuperWeapon2, if its AuxBuilding is satisfied
	int SecondActiveSWIdx() const
		{ JMP_THIS(0x457690); }

	void FireLaser(CoordStruct Coords)
		{ JMP_THIS(0x44ABD0); }

	bool IsBeingDrained() const
		{ JMP_THIS(0x70FEC0); }

	bool UpdateBunker()
		{ JMP_THIS(0x458E50); }

	void KillOccupants(TechnoClass* pAssaulter)
		{ JMP_THIS(0x4585C0); }

	// returns false if this is a gate that needs time to open, true otherwise
	bool MakeTraversable()
		{ JMP_THIS(0x452540); }

	bool CheckFog()
		{ JMP_THIS(0x457A10); }

	Matrix3D* GetVoxelBarrelOffsetMatrix(Matrix3D& ret)
		{ JMP_THIS(0x458810); }

	// returns false if this is a gate that is closed, true otherwise
	bool IsTraversable() const { JMP_THIS(0x4525F0); }

	KickOutResult* ExitObject__(TechnoClass* pTech) const { JMP_THIS(0x4587D0); }

	// helpers
	bool HasSuperWeapon(int index) const {
		if(this->Type->HasSuperWeapon(index)) {
			return true;
		}

		for(auto pType : this->Upgrades) {
			if(pType && pType->HasSuperWeapon(index)) {
				return true;
			}
		}
		return false;
	}

	TechnoTypeClass* GetSecretProduction() const;

	AnimClass*& GetAnim(BuildingAnimSlot slot) {
		return this->Anims[static_cast<int>(slot)];
	}

	AnimClass* const& GetAnim(BuildingAnimSlot slot) const {
		return this->Anims[static_cast<int>(slot)];
	}

	bool& GetAnimState(BuildingAnimSlot slot) {
		return this->AnimStates[static_cast<int>(slot)];
	}

	bool const& GetAnimState(BuildingAnimSlot slot) const {
		return this->AnimStates[static_cast<int>(slot)];
	}

	CoordStruct GetBuildingCenterCoords(bool includeBib = false) const
	{
		CoordStruct ret = this->GetCoords();
		ret.X += this->Type->GetFoundationWidth() / 2;
		ret.Y += this->Type->GetFoundationHeight(includeBib) / 2;
		return ret;
	}

	// use GetCoords() instead
	//CoordStruct* GetBuildingCenterCoords___(CoordStruct* nReturn) const
	//{ JMP_THIS(0x447AC0); }

	//CoordStruct GetCenterCoords___() const
	//{
	//	CoordStruct ret;
	//	GetBuildingCenterCoords___(&ret);
	//	return ret;
	//}

	bool BuildingUnderAttack();

	void Placement_DrawIt_43D030(Point2D& nPoint, RectangleStruct& nRect)
	{ JMP_THIS(0x43D030); }

	//458810 voxel anim MTX

	//
	BuildingClass* GetNearbyLaserFence(CoordStruct* pCoord, bool Unk = true, int threatrange = -1) const
	{ JMP_THIS(0x452BB0); }

	void PlayNthAnim(BuildingAnimSlot nWhich ,bool bIsDamage , bool bIsGarrisoned , int nDelay) const
	{ JMP_THIS(0x451750); }

	bool Absorber() const { JMP_THIS(0x4598A0); }
	bool ClearFactoryBib() const { JMP_THIS(0x449540); }

	void GarrisonAI() const { JMP_THIS(0x458200); }

	bool IsFactory() const {
		JMP_THIS(0x455DA0);
	}

	void DestroyExtrasFunctions(AbstractClass* pKiller) const {
		JMP_THIS(0x442D90);
	}

	int GetRadiuses_4566B0() const { JMP_THIS(0x4566B0); }

	InfantryTypeClass* GetBuildingCrewType() const {//this was virtual , but , wahtever
		JMP_THIS(0x44EB10);
	}

	void Infiltrate(HouseClass* ByWho) const {
		JMP_THIS(0x4571E0);
	}

	bool Undeployable() const {
		JMP_THIS(0x449BC0);
	}

	void UpdateAnimations() { 
		JMP_THIS(0x4509D0); 
	}

	//Constructor
	BuildingClass(BuildingTypeClass* pType, HouseClass* pOwner) noexcept
		: BuildingClass(noinit_t())
	{ JMP_THIS(0x43B740); }

protected:
	explicit __forceinline BuildingClass(noinit_t) noexcept
		: TechnoClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	BuildingTypeClass* Type;
	FactoryClass* Factory;
	DECLARE_PROPERTY(TimerStruct, C4Timer);
	BStateType BState;
	BStateType QueueBState;
	DWORD OwnerCountryIndex;
	InfantryClass* C4AppliedBy;
	DWORD LastStrength; //544
	AnimClass* FirestormAnim; //pointer
	AnimClass* PsiWarnAnim; //pointer
	DECLARE_PROPERTY(TimerStruct, PlacementDelay); //550

	AnimClass * Anims [0x15];
	bool AnimStates [0x15]; // one flag for each of the above anims (whether the anim was enabled when power went offline?)

	PROTECTED_PROPERTY(BYTE, align_5C5[3]);

	//DWORD DamageFireAnims1; //0x5C8
	AnimClass * DamageFireAnims [0x8];
	bool RequiresDamageFires; // if set, ::Update spawns damage fire anims and zeroes it

	//5E8 - 5F8 ????????
	ArrayWrapper<BuildingTypeClass*, 3u> Upgrades;

	int FiringSWType; // type # of sw being launched
	DWORD upgrade_5FC;
	BuildingLightClass* Spotlight;
	DECLARE_PROPERTY(RepeatableTimerStruct, GateTimer);
	LightSourceClass * LightSource; // tiled light , LightIntensity > 0
	DWORD LaserFenceFrame; // 0-7 for active directionals, 8/12 for offline ones, check ntfnce.shp or whatever
	DWORD FirestormWallFrame; // anim data for firestorm active animations
	DECLARE_PROPERTY(ProgressTimer, RepairProgress); // for hospital, armory, unitrepair etc
	DECLARE_PROPERTY(RectangleStruct, unknown_rect_63C);
	DECLARE_PROPERTY(CoordStruct, unknown_coord_64C);
	DECLARE_PROPERTY(Point2D, unknown_point_658);
	bool HasPower;
	bool IsOverpowered;

	// each powered unit controller building gets this set on power activation and unset on power outage
	bool RegisteredAsPoweredUnitSource;

	DWORD SupportingPrisms;
	bool HasExtraPowerBonus;
	bool HasExtraPowerDrain;
	DECLARE_PROPERTY(DynamicVectorClass<InfantryClass*>, Overpowerers);
	DECLARE_PROPERTY(DynamicVectorClass<InfantryClass*>, Occupants);
	int FiringOccupantIndex; // which occupant should get XP, which weapon should be fired (see 6FF074)

	DECLARE_PROPERTY(AudioController, Audio7);
	DECLARE_PROPERTY(AudioController, Audio8);

	bool WasOnline; // the the last state when Update()ing. if this changed since the last Update(), UpdatePowered is called.
	bool ShowRealName; // is also NOMINAL under [Structures]
	bool BeingProduced; // is also AI_REBUILDABLE under [Structures]
	bool ShouldRebuild;// is also AI_REPAIRABLE under [Structures]
	bool HasEngineer; // used to pass the NeedsEngineer check
	DECLARE_PROPERTY(TimerStruct, CashProductionTimer);
	bool IsAllowedToSell; //6DC bool AI_Sellable; AI_SELLABLE under [Structures]
	bool IsReadyToCommence; //6DD
	bool NeedsRepairs; // AI handholder for repair logic,
	bool C4Applied;
	bool NoCrew;
	bool IsCharging; //6E1
	bool IsCharged;	//6E2
	bool HasBeenCaptured; // has this building changed ownership at least once? affects crew and repair.
	bool ActuallyPlacedOnMap;
	bool unknown_bool_6E5;
	bool IsDamaged; // AI handholder for repair logic,
	bool IsFogged;
	bool IsBeingRepaired; // show animooted repair wrench
	bool HasBuildup;
	bool StuffEnabled; // status set by EnableStuff() and DisableStuff()
	char HasCloakingData; // some fugly buffers
	byte CloakRadius; // from Type->CloakRadiusInCells
	char Translucency;
	DWORD StorageFilledSlots; // the old "silo needed" logic
	TechnoTypeClass * SecretProduction; // randomly assigned secret lab bonus, used if SecretInfantry, SecretUnit, and SecretBuilding are null
	DECLARE_PROPERTY(ColorStruct, ColorAdd);
	int IsAirstrikeTargetingMe; //6FC
	short unknown_short_700;
	BYTE UpgradeLevel; // as defined by Type->UpgradesToLevel=
	char GateStage;
	PrismChargeState PrismStage;
	DECLARE_PROPERTY(CoordStruct, PrismTargetCoords);
	DWORD DelayBeforeFiring; //714

	int BunkerState; // used in UpdateBunker and friends 0x718
};

static_assert(sizeof(BuildingClass) == 0x720 , "Invalid Size");