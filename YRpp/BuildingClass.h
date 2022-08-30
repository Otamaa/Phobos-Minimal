/*
	Buildings
*/

#pragma once

#include <TechnoClass.h>
#include <BuildingTypeClass.h>
#include <BuildingLightClass.h>
#include <ProgressTimer.h>
class FactoryClass;
class InfantryClass;
class LightSourceClass;
class FoggedObjectClass;

enum class BStateType : unsigned int
{
	Construction = 0x0,
	Idle = 0x1,
	Active = 0x2,
	Full = 0x3,
	Aux1 = 0x4,
	Aux2 = 0x5,
	Count = 0x6,
	None = 0xFFFFFFFF,
};

class DECLSPEC_UUID("0E272DC6-9C0F-11D1-B709-00A024DDAFD1")
	NOVTABLE BuildingClass : public TechnoClass
{
public:
	static const AbstractType AbsID = AbstractType::Building;

	//Static
	static constexpr constant_ptr<DynamicVectorClass<BuildingClass*>, 0xA8EB40u> const Array{};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) R0;

	//IPersistStream
	//Destructor
	virtual ~BuildingClass() RX;

	//AbstractClass
	virtual AbstractType WhatAmI() const RT(AbstractType);
	virtual int	Size() const R0;

	//ObjectClass
	//MissionClass
	//TechnoClass
	virtual InfantryTypeClass* GetCrew() const override { JMP_THIS(0x44EB53); }
	virtual void Destroyed(ObjectClass* Killer) RX;
	virtual bool ForceCreate(CoordStruct& coord, DWORD dwUnk = 0) R0;

	//BuildingClass
	virtual CellStruct* FindExitCell(CellStruct* pCellStruct, DWORD dwUnk, DWORD dwUnk2) const R0;
	virtual int vt_entry_4D8(ObjectClass* pObj) const R0;
	virtual void Place(bool captured) RX;
	virtual void UpdateConstructionOptions() RX;
	virtual void DrawFogged(const Point2D& point, const RectangleStruct& rect) RX;
	virtual CellStruct* vt_entry_4E8(CellStruct* pCellStruct, DWORD dwUnk) const R0;
	virtual void vt_entry_4EC(DWORD dwUnk, DWORD dwUnk2, DWORD dwUnk3, DWORD dwUnk4) RX;
	virtual bool TogglePrimaryFactory() R0;
	virtual void SensorArrayActivate(CellStruct cell=CellStruct::Empty) RX;
	virtual void SensorArrayDeactivate(CellStruct cell=CellStruct::Empty) RX;
	virtual void DisguiseDetectorActivate(CellStruct cell=CellStruct::Empty) RX;
	virtual void DisguiseDetectorDeactivate(CellStruct cell=CellStruct::Empty) RX;
	virtual DWORD vt_entry_504() R0;

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

	CoordStruct* GetBuildingCenterCoords___(CoordStruct* nReturn) const
	{ JMP_THIS(0x447AC0); }

	CoordStruct GetCenterCoords___() const
	{
		CoordStruct ret;
		GetBuildingCenterCoords___(&ret);
		return ret;
	}

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
	TimerStruct C4Timer;
	int BState;
	int QueueBState;
	DWORD OwnerCountryIndex;
	InfantryClass* C4AppliedBy;
    DWORD LastStrength; //544
	AnimClass* FirestormAnim; //pointer
	AnimClass* PsiWarnAnim; //pointer
    TimerStruct PlacementDelay; //550

// see eBuildingAnims above for slot index meanings
	AnimClass * Anims [0x15];
	bool AnimStates [0x15]; // one flag for each of the above anims (whether the anim was enabled when power went offline?)

	PROTECTED_PROPERTY(char , align_5C5[3]);

	AnimClass * DamageFireAnims [0x8];
	bool RequiresDamageFires; // if set, ::Update spawns damage fire anims and zeroes it

	//5E8 - 5F8 ????????
	BuildingTypeClass * Upgrades [0x3];

	int FiringSWType; // type # of sw being launched
    DWORD upgrade_5FC;
	BuildingLightClass* Spotlight;
	RepeatableTimerStruct GateTimer;
	LightSourceClass * LightSource; // tiled light , LightIntensity > 0
	DWORD LaserFenceFrame; // 0-7 for active directionals, 8/12 for offline ones, check ntfnce.shp or whatever
	DWORD FirestormWallFrame; // anim data for firestorm active animations
	ProgressTimer RepairProgress; // for hospital, armory, unitrepair etc
	RectangleStruct unknown_rect_63C;
	CoordStruct unknown_coord_64C;
	int unknown_int_658;
	DWORD unknown_65C;
	bool HasPower;
	bool IsOverpowered;

	// each powered unit controller building gets this set on power activation and unset on power outage
	bool RegisteredAsPoweredUnitSource;

	DWORD SupportingPrisms;
	bool HasExtraPowerBonus;
	bool HasExtraPowerDrain;
	DynamicVectorClass<InfantryClass*> Overpowerers;
	DynamicVectorClass<InfantryClass*> Occupants;
	int FiringOccupantIndex; // which occupant should get XP, which weapon should be fired (see 6FF074)

	AudioController Audio7;
	AudioController Audio8;

	bool WasOnline; // the the last state when Update()ing. if this changed since the last Update(), UpdatePowered is called.
	bool ShowRealName;
	bool BeingProduced;
	bool ShouldRebuild;
	bool HasEngineer; // used to pass the NeedsEngineer check
	TimerStruct CashProductionTimer;
    bool IsAllowedToSell; //6DC
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
    bool __hasBuildupData_6E9;
    bool StuffEnabled; // status set by EnableStuff() and DisableStuff()
	char HasCloakingData; // some fugly buffers
	byte CloakRadius; // from Type->CloakRadiusInCells
	char Translucency;
	DWORD StorageFilledSlots; // the old "silo needed" logic
	TechnoTypeClass * SecretProduction; // randomly assigned secret lab bonus, used if SecretInfantry, SecretUnit, and SecretBuilding are null
	ColorStruct ColorAdd;
	int IsAirstrikeTargetingMe; //6FC
	short unknown_short_700;
	BYTE UpgradeLevel; // as defined by Type->UpgradesToLevel=
	char GateStage;
	PrismChargeState PrismStage;
	CoordStruct PrismTargetCoords;
	DWORD DelayBeforeFiring; //714

	int BunkerState; // used in UpdateBunker and friends 0x718
	DWORD field_71C;  //unused , can be used to store ExtData
};

static_assert(sizeof(BuildingClass) == 0x720 , "Invalid Size");