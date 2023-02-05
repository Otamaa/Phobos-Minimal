/*
	Base class for buildable objects
*/

#pragma once

#include <Audio.h>
#include <RadioClass.h>
#include <TechnoTypeClass.h>
#include <Helpers/Template.h>
#include <ProgressTimer.h>
#include <TransitionTimer.h>
#include <FacingClass.h>

//forward declarations
class AirstrikeClass;
class AnimClass;
class BulletClass;
class BuildingClass;
class CellClass;
class HouseClass;
class FootClass;
class HouseClass;
class InfantryTypeClass;
class ObjectTypeClass;
class ParticleSystemClass;
class SpawnManagerClass;
class WaveClass;
class EBolt;
struct NetworkEvent;
class SlaveManagerClass;
class CaptureManagerClass;
class RadBeam;
class PlanningTokenClass;
class LaserDrawClass;
class TemporalClass;
class TeamClass;
struct VeterancyStruct
{
	VeterancyStruct() = default;

	explicit VeterancyStruct(double value) noexcept {
		this->Add(value);
	}

	void Add(int ownerCost, int victimCost) noexcept {
		this->Add(static_cast<double>(victimCost)
			/ (ownerCost * RulesClass::Instance->VeteranRatio));
	}

	void Add(double value) noexcept {
		auto val = this->Veterancy + value;

		if(val > RulesClass::Instance->VeteranCap) {
			val = RulesClass::Instance->VeteranCap;
		}

		this->Veterancy = static_cast<float>(val);
	}

	void Add_Fix(double value) noexcept
	{
		auto nVal = (float)(this->Veterancy + abs(value));

		this->Veterancy = Math::clamp(nVal, 0.0f, 0.2f);
	}

	Rank GetRemainingLevel() const noexcept {
		if(this->Veterancy >= 2.0f) {
			return Rank::Elite;
		}

		if(this->Veterancy >= 1.0f) {
			return Rank::Veteran;
		}

		return Rank::Rookie;
	}

	bool IsNegative() const noexcept {
		return this->Veterancy < 0.0f;
	}

	bool IsRookie() const noexcept {
		return this->Veterancy >= 0.0f && this->Veterancy < 1.0f;
	}

	bool IsVeteran() const noexcept {
		return this->Veterancy >= 1.0f && this->Veterancy < 2.0f;
	}

	bool IsElite() const noexcept {
		return this->Veterancy >= 2.0f;
	}

	void Reset() noexcept {
		this->Veterancy = 0.0f;
	}

	void SetRookie(bool notReally = true) noexcept {
		this->Veterancy = notReally ? -0.25f : 0.0f;
	}

	void SetVeteran(bool yesReally = true) noexcept {
		this->Veterancy = yesReally ? 1.0f : 0.0f;
	}

	void SetElite(bool yesReally = true) noexcept {
		this->Veterancy = yesReally ? 2.0f : 0.0f;
	}

	float Veterancy{ 0.0f };
};
static_assert(sizeof(VeterancyStruct) == 0x4);

class PassengersClass
{
public:
	int NumPassengers;
	FootClass* FirstPassenger;

	void AddPassenger(FootClass* pPassenger)
		{ JMP_THIS(0x4733A0); }

	FootClass* GetFirstPassenger() const
		{ return this->FirstPassenger; }

	FootClass* RemoveFirstPassenger()
		{ JMP_THIS(0x473430); }

	int GetTotalSize() const
		{ JMP_THIS(0x473460); }

	int IndexOf(FootClass* candidate) const
		{ JMP_THIS(0x473500); }

	void Detach(FootClass* candidate) const
		{ JMP_THIS(0x4734B0);}

	void RemovePassenger(FootClass* pPassenger) const
		{ JMP_THIS(0x4734B0); }

	//PassengersClass() noexcept : NumPassengers(0), FirstPassenger(nullptr) {};
	//~PassengersClass() noexcept = default;

};
static_assert(sizeof(PassengersClass) == 0x8);

struct FlashData
{
	int DurationRemaining;
	bool FlashingNow;

	bool Update()
		{ JMP_THIS(0x4CC770); }
};
static_assert(sizeof(FlashData) == 0x8);

struct RecoilData
{
	enum class RecoilState : unsigned int {
		Inactive = 0,
		Compressing = 1,
		Holding = 2,
		Recovering = 3,
	};

	TurretControl Turret;
	float TravelPerFrame;
	float TravelSoFar;
	RecoilState State;
	int TravelFramesLeft;

	void Update()
		{ JMP_THIS(0x70ED10); }

	void Fire()
		{ JMP_THIS(0x70ECE0); }
};

class NOVTABLE TechnoClass : public RadioClass
{
public:

	static const auto AbsDerivateID = AbstractFlags::Techno;

	static constexpr constant_ptr<DynamicVectorClass<TechnoClass*>, 0xA8EC78u> const Array{};

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x70BF50);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x70C250);

	//Destructor
	virtual ~TechnoClass() override JMP_THIS(0x7106E0);

	//AbstractClass
	virtual void Init() override { JMP_THIS(0x6F3F40); }
	virtual void PointerExpired(AbstractClass* pAbstract, bool removed) override JMP_THIS(0x7077C0);
	virtual int GetOwningHouseIndex() const override JMP_THIS(0x6F9DB0);//{ return this->Owner->ArrayIndex; }
	//virtual HouseClass* GetOwningHouse() const override { return this->Owner; }
	virtual void Update() override JMP_THIS(0x6F9E50);

	//ObjectClass
	virtual void AnimPointerExpired(AnimClass* pAnim) override JMP_THIS(0x710410);

	// remove object from the map
	virtual bool Limbo() override JMP_THIS(0x6F6AC0);

	// place the object on the map
	virtual bool Unlimbo(const CoordStruct& Crd, DirType dFaceDir) override JMP_THIS(0x6F6CA0);

	//MissionClass
	virtual void Override_Mission(Mission mission, AbstractClass* tarcom = nullptr, AbstractClass* navcom = nullptr) override JMP_THIS(0x7013A0); //Vt_1F4
	virtual bool Mission_Revert() override JMP_THIS(0x7013E0);

	//TechnoClass
	virtual bool IsUnitFactory() const R0;
	virtual bool IsCloakable() const R0;
	virtual bool CanScatter() const R0;
	virtual bool BelongsToATeam() const R0;
	virtual bool ShouldSelfHealOneStep() const R0;
	virtual bool IsVoxel() const R0;
	virtual bool vt_entry_29C() R0;
	virtual bool IsReadyToCloak() const R0; //ShouldBeCloaked
	virtual bool ShouldNotBeCloaked() const R0;
	virtual DirStruct* TurretFacing(DirStruct* pBuffer) const R0;
	virtual bool IsArmed() const R0; // GetWeapon(primary) && GetWeapon(primary)->WeaponType
	virtual bool vt_entry_2B0() const R0;
	virtual double GetStoragePercentage() const R0;
	virtual int GetPipFillLevel() const R0;
	virtual int GetRefund() const R0;
	virtual int GetThreatValue() const R0;
	virtual bool IsInSameZoneAs(AbstractClass* pTarget) R1;
	virtual DirStruct* GetFacingAgainst(DirStruct* pRet, AbstractClass* pTarget) JMP_THIS(0x6FDA00);
	virtual bool IsInSameZone(const CoordStruct* nZone) JMP_THIS(0x707F60);
	virtual int GetCrewCount() const R0;
	virtual int GetAntiAirValue() const R0;
	virtual int GetAntiArmorValue() const R0;
	virtual int GetAntiInfantryValue() const R0;
	virtual void GotHijacked() RX;
	virtual int SelectWeapon(AbstractClass *pTarget) const JMP_THIS(0x6F3330);
	virtual NavalTargetingType SelectNavalTargeting(AbstractClass *pTarget) const JMP_THIS(0x6F3820);
	virtual int GetZAdjustment() const R0;
	virtual ZGradient GetZGradient() const RT(ZGradient);
	virtual CellStruct* GetSomeCellStruct(CellStruct* buffer) const R0;
	virtual void SetLastCellStruct(CellStruct Buffer) RX;
	virtual CellStruct FindExitCell(TechnoClass* pDocker, CellStruct nDefault) const; //
	virtual CoordStruct * vt_entry_300(CoordStruct * Buffer, DWORD dwUnk2) const R0;
	virtual DWORD vt_entry_304(DWORD dwUnk, DWORD dwUnk2) const R0;
	virtual FacingClass* GetRealFacing(FacingClass* pBuffer) const R0;
	virtual InfantryTypeClass* GetCrew() const { JMP_THIS(0x707D20); }
	virtual bool vt_entry_310() const R0;
	virtual bool CanDeploySlashUnload() const R0;
	virtual int GetROF(int nWeapon) const R0;
	virtual int GetGuardRange(int dwUnk) const R0;
	virtual bool vt_entry_320() const R0;
	virtual bool IsRadarVisible(int* pOutDetection) const R0; // out value will be set to 1 if unit is cloaked and 2 if it is subterranean, otherwise it's unchanged
	virtual bool IsSensorVisibleToPlayer() const R0;
	virtual bool IsSensorVisibleToHouse(HouseClass *House) const R0;
	virtual bool IsEngineer() const R0;
	virtual void ProceedToNextPlanningWaypoint() RX;
	virtual DWORD ScanForTiberium(DWORD dwUnk, DWORD dwUnk2, DWORD dwUnk3) const R0;
	virtual bool EnterGrinder() R0;
	virtual bool EnterBioReactor() R0;
	virtual bool EnterTankBunker() R0;
	virtual bool EnterBattleBunker() R0;
	virtual bool GarrisonStructure() R0;
	virtual bool IsPowerOnline() const R0;
	virtual void QueueVoice(int idxVoc) RX;
	virtual int VoiceEnter() R0;
	virtual int VoiceHarvest() R0;
	virtual int VoiceSelect() R0;
	virtual int VoiceCapture() R0;
	virtual int VoiceMove() R0;
	virtual int VoiceDeploy() R0;
	virtual int VoiceAttack(ObjectClass *pTarget) R0;
	virtual bool ClickedEvent(NetworkEvents event) R0;

	// depending on the mission you click, cells/Target are not always needed
	virtual bool ClickedMission(Mission Mission, ObjectClass *pTarget, CellClass * TargetCell, CellClass *NearestTargetCellICanEnter) R0;
	virtual bool IsUnderEMP() const R0;
	virtual bool IsParalyzed() const R0;
	virtual bool CanCheer() const R0;
	virtual void Cheer(bool Force) RX;
	virtual int GetDefaultSpeed() const R0;
	virtual void DecreaseAmmo() RX;
	virtual void AddPassenger(FootClass* pPassenger) RX;
	virtual bool CanDisguiseAs(AbstractClass*pTarget) const R0;
	virtual bool TargetAndEstimateDamage(DWORD dwUnk, DWORD dwUnk2) R0;
	virtual void Stun() RX;//DWORD vt_entry_3A0() R0;
	virtual bool TriggersCellInset(AbstractClass *pTarget) R0;
	virtual bool IsCloseEnough(AbstractClass *pTarget, int idxWeapon) const R0;
	virtual bool IsCloseEnoughToAttack(AbstractClass *pTarget) const R0; //In_Range
	virtual bool IsCloseEnoughToAttackCoords(const CoordStruct& Coords) const R0;
	virtual DWORD vt_entry_3B4(DWORD dwUnk) const R0;
	virtual void Destroyed(ObjectClass *Killer) = 0;
	virtual FireError GetFireErrorWithoutRange(AbstractClass *pTarget, int nWeaponIndex) const RT(FireError);
	virtual FireError GetFireError(AbstractClass *pTarget, int nWeaponIndex, bool ignoreRange) const RT(FireError); //CanFire
	virtual CellClass* SelectAutoTarget(TargetFlags TargetFlags, int CurrentThreat, bool OnlyTargetHouseEnemy) R0; //Greatest_Threat
	virtual void SetTarget(AbstractClass *pTarget) RX;
	virtual BulletClass* Fire(AbstractClass* pTarget, int nWeaponIndex) JMP_THIS(0x6FDD50);
	virtual void Guard() RX; // clears target and destination and puts in guard mission
	virtual bool SetOwningHouse(HouseClass* pHouse, bool announce = true) R0;
	virtual void vt_entry_3D8(DWORD dwUnk, DWORD dwUnk2, DWORD dwUnk3) RX;
	virtual bool Crash(ObjectClass *Killer) R0;
	virtual bool IsAreaFire() const R0;
	virtual int IsNotSprayAttack() const R0;
	virtual int vt_entry_3E8() R0;
	virtual int IsNotSprayAttack2() const R0;
	virtual WeaponStruct* GetDeployWeapon() const R0;
	virtual WeaponStruct* GetPrimaryWeapon() const JMP_THIS(0x70E1A0); //Get_Primary_Weapon
	virtual WeaponStruct* GetWeapon(int nWeaponIndex) const R0;
	virtual bool HasTurret() const R0;
	virtual bool CanOccupyFire() const R0;
	virtual int GetOccupyRangeBonus() const R0;
	virtual int GetOccupantCount() const R0;
	virtual void OnFinishRepair() RX;
	virtual void UpdateCloak(bool bUnk = 1) RX;
	virtual void CreateGap() RX;
	virtual void DestroyGap() RX;
	virtual void RockByValue(Point3D* Points, float bFactor = 1.5f, bool bHalf = false) RX;//virtual void vt_entry_41C() RX;
	virtual void Sensed() RX;
	virtual void Reload() RX;
	virtual void vt_entry_428() RX;

	// Returns target's coordinates if on attack mission & have target, otherwise own coordinates.
	virtual CoordStruct* GetAttackCoords(CoordStruct* pCrd) const R0;
	virtual bool IsNotWarpingIn() const R0;
	virtual bool vt_entry_434(DWORD dwUnk) const R0;
	virtual void DrawActionLines(bool Force, DWORD dwUnk2) RX;
	virtual DWORD GetDisguiseFlags(DWORD existingFlags) const R0;
	virtual bool IsClearlyVisibleTo(HouseClass *House) const R0; // can House see right through my disguise?
	virtual void DrawVoxel(const VoxelStruct& Voxel, DWORD dwUnk2, short Facing,
		const IndexClass<int, int>& VoxelIndex, const RectangleStruct& Rect, const Point2D& nLocation,
		const Matrix3D& Matrix, int Intensity, DWORD dwUnk9, DWORD dwUnk10) RX;
	virtual void vt_entry_448(DWORD dwUnk, DWORD dwUnk2) RX;
	virtual void DrawHealthBar(Point2D *pLocation, RectangleStruct *pBounds, bool bUnk3) const RX;
	virtual void DrawPipScalePips(Point2D *pLocation, Point2D *pOriginalLocation, RectangleStruct *pBounds) const RX;
	virtual void DrawVeterancyPips(Point2D *pLocation, RectangleStruct *pBounds) const RX;
	virtual void DrawExtraInfo(Point2D const& location, Point2D const& originalLocation, RectangleStruct const& bounds) const RX;
	virtual void Uncloak(bool bPlaySound) RX;
	virtual void Cloak(bool bPlaySound) RX;
	virtual DWORD vt_entry_464(DWORD dwUnk) const R0;
	virtual void UpdateRefinerySmokeSystems() RX;
	virtual DWORD DisguiseAs(AbstractClass* pTarget) R0;
	virtual void ClearDisguise() RX;
	virtual bool IsItTimeForIdleActionYet() const R0;
	virtual bool UpdateIdleAction() R0;
	virtual void vt_entry_47C(DWORD dwUnk) RX;
	virtual void SetDestination(AbstractClass* pDest, bool bUnk) RX;
	virtual bool EnterIdleMode(bool Initial, int nUnknown) R0;//virtual bool vt_entry_484(DWORD dwUnk, DWORD dwUnk2) R0;
	virtual void UpdateSight(bool Incremental, int unusedarg3, bool UseThisHouseInstead, HouseClass* dwUnk4, int OverrideSight) RX; //70AF50
	virtual void vt_entry_48C(DWORD dwUnk, DWORD dwUnk2, DWORD dwUnk3, DWORD dwUnk4) RX;
	virtual bool ForceCreate(CoordStruct& coord, DWORD dwUnk = 0) R0;
	virtual void RadarTrackingStart() RX;
	virtual void RadarTrackingStop() RX;
	virtual void RadarTrackingFlash() RX;
	virtual void RadarTrackingUpdate(bool bUnk) RX;
	virtual void vt_entry_4A4(DWORD dwUnk) RX;
	virtual void vt_entry_4A8() RX;
	virtual bool vt_entry_4AC() const R0;
	virtual bool vt_entry_4B0() const R0;
	virtual int vt_entry_4B4() const R0;
	virtual CoordStruct* vt_entry_4B8(CoordStruct* pCrd) R0;
	virtual bool CanUseWaypoint() const R0;
	virtual bool CanAttackOnTheMove() const R0;
	virtual bool vt_entry_4C4() const R0;
	virtual bool vt_entry_4C8() R0;
	virtual void vt_entry_4CC() RX;
	virtual bool vt_entry_4D0() R0;

	//non-virtual
	bool sub_703B10() const JMP_THIS(0x703B10);
	int sub_703E70() const JMP_THIS(0x703E70);
	int sub_704000() const JMP_THIS(0x704000);
	int sub_704240() const JMP_THIS(0x704240);
	bool sub_70D8F0() JMP_THIS(0x70D8F0);
	bool sub_70DCE0() const { return this->CurrentTurretNumber != -1; }

	bool IsDrainSomething()
		{ return this->DrainTarget != nullptr; }

	int GetCurrentTurretNumber() const
		{ JMP_THIS(0x70DCF0); }

	bool HasMultipleTurrets() const
		{ JMP_THIS(0x70DC60); }

	bool IsDeactivated() const
		{ JMP_THIS(0x70FBD0); }

	// (re-)starts the reload timer
	void StartReloading()
		{ JMP_THIS(0x6FB080); }

	bool ShouldSuppress(CellStruct *coords) const
		{ JMP_THIS(0x6F79A0); }

	bool IsMindControlled() const
		{ JMP_THIS(0x7105E0); }

	bool CanBePermaMindControlled() const
		{ JMP_THIS(0x53C450); }

	LaserDrawClass* CreateLaser(AbstractClass* pTarget, int idxWeapon, WeaponTypeClass* pWeapon, const CoordStruct& Coords)
	{ JMP_THIS(0x6FD210); }

	EBolt* CreateEbolt(AbstractClass* pTarget, WeaponTypeClass* pWeapon, const CoordStruct& Coords)
	{ JMP_THIS(0x6FD460); }

	/*
	 *  Cell->AddThreat(this->Owner, -this->ThreatPosed);
	 *  this->ThreatPosed = 0;
	 *  int Threat = this->CalculateThreat(); // this is another gem of a function, to be revealed another time...
	 *  this->ThreatPosed = Threat;
	 *  Cell->AddThreat(this->Owner, Threat);
	 */
	void UpdateThreatInCell(CellClass *Cell)
		{ JMP_THIS(0x70F6E0); }

// CanTargetWhatAmI is a bitfield, if(!(CanTargetWhatAmI & (1 << tgt->WhatAmI())) { fail; }

// slave of the next one
	bool CanAutoTargetObject(
		TargetFlags targetFlags,
		int canTargetWhatAmI,
		int wantedDistance,
		TechnoClass* pTarget,
		int* pThreatPosed,
		DWORD dwUnk,
		CoordStruct* pSourceCoords) const
			{ JMP_THIS(0x6F7CA0); }

// called by AITeam Attack Target Type and autoscan
	bool TryAutoTargetObject(
		TargetFlags targetFlags,
		int canTargetWhatAmI,
		CellStruct* pCoords,
		DWORD dwUnk1,
		DWORD* dwUnk2,
		int* pThreatPosed,
		DWORD dwUnk3)
			{ JMP_THIS(0x6F8960); }

	int EvaluateJustCell(CellStruct& coords) const
	{ JMP_THIS(0x6F8C10); }

	void Reactivate()
		{ JMP_THIS(0x70FBE0); }

	void Deactivate()
		{ JMP_THIS(0x70FC90); }


	// this should be the transport, but it's unused
	// marks passenger as "InOpenTopped" for targeting, range scanning and other purposes
	void EnteredOpenTopped(TechnoClass* pWho)
		{ JMP_THIS(0x710470); }

	// this should be the transport, but it's unused
	// reverses the above
	void ExitedOpenTopped(TechnoClass* pWho)
		{ JMP_THIS(0x7104A0); }

	// called when the source unit dies - passengers are about to get kicked out, this basically calls ->ExitedOpenTransport on each passenger
	void MarkPassengersAsExited()
		{ JMP_THIS(0x7104C0); }

	// for gattlings
	void SetCurrentWeaponStage(int idx)
		{ JMP_THIS(0x70DDD0); }

	void SetFocus(AbstractClass* pFocus)
		{ JMP_THIS(0x70C610); }

	//void DrawObject(SHPStruct* pSHP, int nFrame, Point2D* pLocation, RectangleStruct* pBounds,
	//	int, int, int nZAdjust, ZGradient eZGradientDescIdx, int, int nBrightness, int TintColor,
	//	SHPStruct* pZShape, int nZFrame, int nZOffsetX, int nZOffsetY, int);

	int sub_70DE00(int State)
		{ JMP_THIS(0x70DE00); }

	int __fastcall ClearPlanningTokens(NetworkEvent* pEvent)
		{ JMP_STD(0x6386E0); }

	void SetTargetForPassengers(AbstractClass* pTarget)
		{ JMP_THIS(0x710550); }

	// returns the house that created this object (factoring in Mind Control)
	HouseClass * GetOriginalOwner()
		{ JMP_THIS(0x70F820); }

	void FireDeathWeapon(int additionalDamage)
		{ JMP_THIS(0x70D690); }

	bool HasAbility(AbilityType ability) const
		{ JMP_THIS(0x70D0D0); }

	void ClearSidebarTabObject() const
		{ JMP_THIS(0x734270); }

	int GetIonCannonValue(AIDifficulty difficulty) const;
	int GetIonCannonValue(AIDifficulty difficulty, int maxHealth) const;
	//MissionControlClass* GetMissionControlCurrent() const;
	//double GetCurrentMissionRate() const;

	//70BCB0
	CoordStruct* GetMovingTargetCoords(CoordStruct* pBuffer)
	{ JMP_THIS(0x70BCB0); }

	// Returns target's coordinates if on attack mission & have target, otherwise own coordinates.
	CoordStruct GetAttackCoords()
	{
		CoordStruct pBuffer;
		this->GetAttackCoords(&pBuffer);
		return pBuffer;
	}

	DirStruct TurretFacing() const
	{
		DirStruct ret;
		this->TurretFacing(&ret);
		return ret;
	}

	FacingClass GetRealFacing() const {
		FacingClass ret;
		this->GetRealFacing(&ret);
		return ret;
	}

//	BulletClass* FireAt(AbstractClass* aTarget, int nWhich) const
//	{ JMP_THIS(0x6FDD50); }

	int CombatDamage(int nWhich = -1) const
	{ JMP_THIS(0x6F3970); }

	bool MoveOnLinked() const
	{ JMP_THIS(0x70D8F0); }

	void LocomotorImblued(bool remove) const
	{ JMP_THIS(0x70FEE0); }

	void ImbueLocomotor(FootClass* pTarget, _GUID LocoId)const
	{ JMP_THIS(0x710000); }

	void ReleaseCaptureManager() const;
	void SuspendWorkSlaveManager() const;
	void ResumeWorkSlaveManager() const;
	void DetechMyTemporal() const;

	InfantryTypeClass* GetCrewType() const
	{ JMP_THIS(0x707D20); }

	void KillCargo(TechnoClass* pSource) const
	{ JMP_THIS(0x707CB0); }

	void KillPassengers(TechnoClass* pSource) const
	{ JMP_THIS(0x707CB0); }

	int TimeToBuild() const
	{ JMP_THIS(0x6F47A0); }

	CellStruct* NearbyLocation(CellStruct* pRet, TechnoClass* pOtherTechno)
	{ JMP_THIS(0x703590); }

	bool MoveOnToLinkedBuilding() const
	{ JMP_THIS(0x70D8F0); }

	//Gattling stuffs
	void AdjustGattlingValue(int nHowMuch) const
	{ JMP_THIS(0x70DE40); }

	int GetCurrentGattlingValue() const
	{ JMP_THIS(0x70DDF0); }

	int GetCurrentGattlingStage() const
	{ JMP_THIS(0x70DDC0); }

	void SetGattlingValue(int nInput) const
	{ JMP_THIS(0x70DE00); }

	void IncreaseGattlingValue(int nInput) const
	{ JMP_THIS(0x70DE20); }

	void SetGattlingStage(int nInput) const
	{ JMP_THIS(0x70DDD0); }

	void GattlingAI() const
	{ JMP_THIS(0x70DE70); }
	//
	void DrawExtraInfo(Point2D const& location, Point2D const* originalLocation, RectangleStruct const* bounds)
	{ DrawExtraInfo(location,*originalLocation,*bounds); }

	void Techno_70E280(AbstractClass* pTarget) const
	{ JMP_THIS(0x70E280); }

	//AbstractClass
	void TechnoClass_AI() const JMP_THIS(0x6F9E50);

	//MissionClass
	void TechnoClass_Override_Mission(Mission mission, AbstractClass* tarcom = nullptr, AbstractClass* navcom = nullptr) const JMP_THIS(0x7013A0); //Vt_1F4

	//bool IsLocked() const {
	//	return *reinterpret_cast<bool*>(reinterpret_cast<DWORD>(this) + 0x3D5);
	//}

	bool CanThisCloakByDefault() {
		return (GetTechnoType()) && (GetTechnoType()->Cloakable || HasAbility(AbilityType::Cloak));
	}

	LightConvertClass* GetDrawer() const
		{ JMP_THIS(0x705D70); }

	bool InRange(CoordStruct& location, AbstractClass* pTarget, WeaponTypeClass* pWeapon) {
		JMP_THIS(0x6F7220);
	}

	unsigned int EvaluateTargetElevation(AbstractClass* pTarget) const {
		JMP_THIS(0x6F6F60);
	}

	void CreateTalkBubble(int frame) const {
		JMP_THIS(0x70F120);
	}

	void RemoveFromTargetingAndTeam() const {
		JMP_THIS(0x70D4A0);
	}

	void SetTargetingDelay() const {
		JMP_THIS(0x70F770)
	}

	int GetAirstrikeTint(int nIn) const {
		JMP_THIS(0x70E4B0);
	}

	int GetIronCurtainTint(int nInt) const {
		JMP_THIS(0x70E380);
	}

	int GetEffectTintIntensity(int currentIntensity) const
	{ JMP_THIS(0x70E360); }

	int GetInvulnerabilityTintIntensity(int currentIntensity) const
	{ JMP_THIS(0x70E380); }

	int GetAirstrikeTintIntensity(int currentIntensity) const
	{ JMP_THIS(0x70E4B0); }

	bool DoOnLinked() const {
		JMP_THIS(0x70D7E0);
	}

	void UpdateIronCurtainTimer() const { JMP_THIS(0x70E5A0); }
	void UpdateAirstrikeTimer() const { JMP_THIS(0x70E920); }

	int GetElevationValue(TechnoClass* const pAgainst) const {
		JMP_THIS(0x6F70E0);
	}

	WeaponStruct* GetTurrentWeapon() const //Vtable_GetPrimaryWeapon
	{ JMP_THIS(0x70E1A0); }

	bool IsThisReadyToCloak() const
	{ JMP_THIS(0x6FBDC0); }

	bool ThisShouldNotCloak() const
	{ JMP_THIS(0x6FBC90); }

	//Constructor
	TechnoClass(HouseClass* pOwner) noexcept
		: TechnoClass(noinit_t())
	{ JMP_THIS(0x6F2B40); }

protected:
	explicit __forceinline TechnoClass(noinit_t) noexcept
		: RadioClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	DECLARE_PROPERTY(FlashData, Flashing);
	DECLARE_PROPERTY(ProgressTimer, Animation); // how the unit animates
	DECLARE_PROPERTY(PassengersClass, Passengers);
	TechnoClass*     Transporter; // unit carrying me
	int              __LastGuardAreaTargetingFrame_120;
	int              CurrentTurretNumber; // for IFV/gattling/charge turrets
	int              __TurretWeapon2_128;
	AnimClass*       BehindAnim;
	AnimClass*       DeployAnim;
	bool             InAir;
	int              CurrentWeaponNumber; // for IFV/gattling 138
	Rank             CurrentRanking; // only used for promotion detection
	int              CurrentGattlingStage;
	int              GattlingValue; // sum of RateUps and RateDowns
	int              TurretAnimFrame;
	HouseClass*      InitialOwner; // only set in ctor
	DECLARE_PROPERTY(VeterancyStruct, Veterancy);
	PROTECTED_PROPERTY(DWORD, align_154);
	double           ArmorMultiplier;
	double           FirepowerMultiplier;
	DECLARE_PROPERTY(TimerStruct, IdleActionTimer); // MOO CDTimerClass
	DECLARE_PROPERTY(TimerStruct, RadarFlashTimer);  //FrameTimerClass
	DECLARE_PROPERTY(TimerStruct, TargetingTimer); //Duration = 45 on init!
	DECLARE_PROPERTY(TimerStruct, IronCurtainTimer);
	DECLARE_PROPERTY(TimerStruct, IronTintTimer); // how often to alternate the effect color
	int              IronTintStage; // ^
	DECLARE_PROPERTY(TimerStruct, AirstrikeTimer);
	DECLARE_PROPERTY(TimerStruct, AirstrikeTintTimer); // tracks alternation of the effect color
	DWORD            AirstrikeTintStage; //  ^
	int              ForceShielded;	//0 or 1, NOT a bool - is this under ForceShield as opposed to IC?
	bool             Deactivated; //Robot Tanks without power for instance
	PROTECTED_PROPERTY(BYTE, align1C_9_A_B[3]);
	TechnoClass*     DrainTarget; // eg Disk -> PowerPlant, this points to PowerPlant
	TechnoClass*     DrainingMe;  // eg Disk -> PowerPlant, this points to Disk
	AnimClass*       DrainAnim;
	bool             Disguised;
	PROTECTED_PROPERTY(BYTE, align1D_9_A_B[3]);
	DWORD            DisguiseCreationFrame;
	DECLARE_PROPERTY(TimerStruct, InfantryBlinkTimer); // Rules->InfantryBlinkDisguiseTime , detects mirage firing per description
	DECLARE_PROPERTY(TimerStruct, DisguiseBlinkTimer); // disguise disruption timer
	bool             UnlimboingInfantry; //1F8
	PROTECTED_PROPERTY(BYTE, align1F_9_A_B[3]);
	DECLARE_PROPERTY(TimerStruct, ReloadTimer);//CDTimerClass
	DECLARE_PROPERTY(Point2D, __RadarPos);

	// WARNING! this is actually an index of HouseTypeClass es, but it's being changed to fix typical WW bugs.
	DECLARE_PROPERTY(IndexBitfield<HouseClass *>, DisplayProductionTo); // each bit corresponds to one player on the map, telling us whether that player has (1) or hasn't (0) spied this building, and the game should display what's being produced inside it to that player. The bits are arranged by player ID, i.e. bit 0 refers to house #0 in HouseClass::Array, 1 to 1, etc.; query like ((1 << somePlayer->ArrayIndex) & someFactory->DisplayProductionToHouses) != 0

	int              Group; //0-9, assigned by CTRL+Number, these kinds // also set by aimd TeamType->Group !
	/*  Focus on RA1 Source , called `ArchiveTarget` ,  
		For units in area guard mode, this is the recorded home position. The guarding
		unit will try to stay near this location in the course of it's maneuvers. This is
		also used to record a pending transport for those passengers that are waiting for
		the transport to become available. It is also used by harvesters so that they know
		where to head back to after unloading.
	*/
	AbstractClass*   Focus;
	HouseClass*      Owner;
	CloakState       CloakState;
	DECLARE_PROPERTY(ProgressTimer, CloakProgress); //StageClass, phase from [opaque] -> [fading] -> [transparent] , [General]CloakingStages= long
	DECLARE_PROPERTY(TimerStruct, CloakDelayTimer); // delay before cloaking again
	float            WarpFactor; // don't ask! set to 0 in CTOR, never modified, only used as ((this->Fetch_ID) + this->WarpFactor) % 400 for something in cloak ripple
	bool             unknown_bool_250;
	PROTECTED_PROPERTY(BYTE, align25_1_2_3[3]);
	CoordStruct      LastSightCoords;
	int              LastSightRange;
	int              LastSightHeight;
	bool             GapSuperCharged; // GapGenerator, when SuperGapRadiusInCells != GapRadiusInCells, you can deploy the gap to boost radius
	bool             GeneratingGap; // is currently generating gap
	PROTECTED_PROPERTY(BYTE, align26_A_B[2]);
	int              GapRadius;
	bool             BeingWarpedOut; // is being warped by CLEG used , for 70C5B0
	bool             WarpingOut; // phasing in after chrono-jump used , for 70C5C0
	bool             unknown_bool_272;
	BYTE             unused_273;
	TemporalClass*   TemporalImUsing; // CLEG attacking Power Plant : CLEG's this
	TemporalClass*   TemporalTargetingMe; 	// CLEG attacking Power Plant : PowerPlant's this
	bool             IsImmobilized; // by chrono aftereffects ,27C
	PROTECTED_PROPERTY(BYTE, align27_D_E_F[3]);
	DWORD            unknown_280;
	int              ChronoLockRemaining; // 284 countdown after chronosphere warps things around
	CoordStruct      ChronoDestCoords; // teleport loco and chsphere set this
	AirstrikeClass*  Airstrike; //Boris
	bool             Berzerk;
	PROTECTED_PROPERTY(BYTE, align29_9_A_B[3]);
	int            	BerzerkDurationLeft;
	int            	SprayOffsetIndex; // hardcoded array of xyz offsets for sprayattack, 0 - 7, see 6FE0AD
	bool             Uncrushable; // DeployedCrushable fiddles this, otherwise all 0
//	PROTECTED_PROPERTY(BYTE, align2A_5_6_7[3]);
 // unless source is Pushy=
 // abs_Infantry source links with abs_Unit target and vice versa - can't attack others until current target flips
 // no checking whether source is Infantry, but no update for other types either
 // old Brute hack
	FootClass*       DirectRockerLinkedUnit;
	FootClass*       LocomotorTarget; // mag->LocoTarget = victim
	FootClass*       LocomotorSource; // victim->LocoSource = mag
	AbstractClass*   Target; //if attacking ,tarcom
	AbstractClass*   LastTarget; //suspendedtarcom
	CaptureManagerClass* CaptureManager; //for Yuris
	TechnoClass*     MindControlledBy;
	bool             MindControlledByAUnit;
	PROTECTED_PROPERTY(BYTE, align2C_5_6_7[3]);
	AnimClass*       MindControlRingAnim;
	HouseClass*      MindControlledByHouse; //used for a TAction
	SpawnManagerClass* SpawnManager;
	TechnoClass*     SpawnOwner; // on DMISL , points to DRED and such
	SlaveManagerClass* SlaveManager;
	TechnoClass*     SlaveOwner; // on SLAV, points to YAREFN
	HouseClass*      OriginallyOwnedByHouse; //used for mind control

		//units point to the Building bunkering them, building points to Foot contained within
	TechnoClass*     BunkerLinkedItem;

	float            PitchAngle; // not exactly, and it doesn't affect the drawing, only internal state of a dropship
	DECLARE_PROPERTY(TimerStruct, ArmTimer); //DiskLaserTimer
	int              ROF;
	int              Ammo;
	int              Value; //,PurchasePrice set to actual cost when this gets queued in factory, updated only in building's 42C


	ParticleSystemClass* FireParticleSystem;
	ParticleSystemClass* SparkParticleSystem;
	ParticleSystemClass* NaturalParticleSystem;
	ParticleSystemClass* DamageParticleSystem;
	ParticleSystemClass* RailgunParticleSystem;
	ParticleSystemClass* unk1ParticleSystem;
	ParticleSystemClass* unk2ParticleSystem;
	ParticleSystemClass* FiringParticleSystem;

	WaveClass*       Wave; //Beams


	// rocking effect
	float            AngleRotatedSideways; // in this frame, in radians - if abs() exceeds pi/2, it dies
	float            AngleRotatedForwards; // same

	// set these and leave the previous two alone!
	// if these are set, the unit will roll up to pi/4, by this step each frame, and balance back
	float            RockingSidewaysPerFrame; // left to right - positive pushes left side up
	float            RockingForwardsPerFrame; // back to front - positive pushes ass up

	int              HijackerInfantryType; // mutant hijacker

	DECLARE_PROPERTY(StorageClass, Tiberium);
	PROTECTED_PROPERTY(DWORD , unknown_34C);

	DECLARE_PROPERTY(TransitionTimer, UnloadTimer); // times the deploy, unload, etc. cycles ,DoorClass

	DECLARE_PROPERTY(FacingClass, BarrelFacing);
	DECLARE_PROPERTY(FacingClass, PrimaryFacing);
	DECLARE_PROPERTY(FacingClass, SecondaryFacing);
	int              CurrentBurstIndex;
	DECLARE_PROPERTY(TimerStruct, TargetLaserTimer); //FrameTimerClass
	short            weapon_sound_number_3C8;
	WORD             __shipsink_3CA;
	bool             CountedAsOwned; // is this techno contained in OwningPlayer->Owned... counts?
	bool             IsSinking;
	bool             WasSinkingAlready; // if(IsSinking && !WasSinkingAlready) { play SinkingSound; WasSinkingAlready = 1; }
	bool             __ProtectMe_3CF;
	bool             IsUseless; //3D0
	bool			 IsTickedOff; //HasBeenAttacked //3D1
	bool			 Cloakable; //3D2
	bool			 IsPrimaryFactory; //3D3
	//bool			 IsLoaner; // 3D4
	//bool			 IsLocked; // 3D5
	bool			 Spawned; // 3D6
	bool             IsInPlayfield; // 3D7
	DECLARE_PROPERTY(RecoilData, TurretRecoil);
	DECLARE_PROPERTY(RecoilData, BarrelRecoil);
	bool             IsTethered; //418
	bool             RADIO_26_27_419;
	bool             IsOwnedByCurrentPlayer;
	bool             DiscoveredByCurrentPlayer;
	bool             DiscoveredByComputer;
	bool             unknown_bool_41D;
	bool             unknown_bool_41E;
	bool             unknown_bool_41F;
	bool             SightIncrease; // used for LeptonsPerSightIncrease
	bool             RecruitableA; // these two are like Lenny and Carl, weird purpose and never seen separate
	bool             RecruitableB; // they're usually set on preplaced objects in maps
	bool             IsRadarTracked;
	bool             IsOnCarryall;
	bool             IsCrashing;
	bool             WasCrashingAlready;
	bool             IsBeingManipulated;
	TechnoClass*     BeingManipulatedBy; // set when something is being molested by a locomotor such as magnetron
	                                       // the pointee will be marked as the killer of whatever the victim falls onto
	HouseClass*      ChronoWarpedByHouse;
	bool             _Mission_Patrol_430;
	bool             IsMouseHovering;
	bool             parasitecontrol_byte432;
	//bool			 byte_433;
	TeamClass*       OldTeam;
	//bool			 IsTracked;
	//bool			 IsTechnician;
	bool             CountedAsOwnedSpecial; // for absorbers, infantry uses this to manually control OwnedInfantry count
	bool             Absorbed; // in UnitAbsorb/InfantryAbsorb or smth, lousy memory
	bool             forceattackforcemovefirendlytarget_bool_43A;
	int            __RadialFireCounter_43C;
	DECLARE_PROPERTY(DynamicVectorClass<int>, CurrentTargetThreatValues);
	DECLARE_PROPERTY(DynamicVectorClass<AbstractClass*>, CurrentTargets);

 // if DistributedFire=yes, this is used to determine which possible targets should be ignored in the latest threat scan
	DECLARE_PROPERTY(DynamicVectorClass<AbstractClass*>, AttackedTargets);

	DECLARE_PROPERTY(AudioController, Audio3);

	bool            __IsTurretTurning_49C; // Turret is moving?
	bool            TurretIsRotating;

	DECLARE_PROPERTY(AudioController, Audio4);

	bool             GattlingAudioPlayed; //4B8
	DWORD            unknown_4BC;

	DECLARE_PROPERTY(AudioController, Audio5);

	bool             gattlingsound_4D4;
	DWORD            unknown_4D8;

	DECLARE_PROPERTY(AudioController, Audio6);

	int            	QueuedVoiceIndex;
	int            	__LastVoicePlayed; //4F4
	bool             deploy_bool_4F8;
	BYTE			 pad_4F9[3];
	DWORD            __creationframe_4FC;	//gets initialized with the current Frame, but this is NOT a TimerStruct!
	BuildingClass*   LinkedBuilding; // 500 BuildingClass*
	int            	EMPLockRemaining;
	int            	ThreatPosed; // calculated to include cargo etc
	bool            ShouldLoseTargetNow;
	BYTE			 pad_50D[3];
	RadBeam*         FiringRadBeam;
	PlanningTokenClass* PlanningToken;
	ObjectTypeClass* Disguise;
	HouseClass*      DisguisedAsHouse;
};

static_assert(sizeof(TechnoClass) == 0x520, "Invalid size.");
