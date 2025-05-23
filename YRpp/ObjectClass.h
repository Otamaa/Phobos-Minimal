/*
	Base class for all game objects.
*/

#pragma once

#include <AbstractClass.h>
#include <Audio.h>
#include <Helpers/Template.h>

struct RectangleStruct;
class ObjectTypeClass;
class TechnoTypeClass;
class WarheadTypeClass;
class BuildingTypeClass;
class InfantryTypeClass;
struct SHPStruct;
class LightConvertClass;
class AnimClass;
class BuildingClass;
class BombClass;
class CellClass;
class TechnoClass;
class HouseTypeClass;
class LineTrail;
struct WeaponStruct;
class TagClass;

class NOVTABLE ObjectClass : public AbstractClass
{
public:
	static const auto AbsDerivateID = AbstractFlags::Object;

	//global arrays
	static COMPILETIMEEVAL reference<DynamicVectorClass<ObjectClass*>, 0xA8ECB8u> const CurrentObjects{};

	//IUnknown
	//virtual HRESULT __stdcall QueryInterface(REFIID iid, void** ppvObject) override R0;
	//virtual ULONG __stdcall AddRef() override R0;
	//virtual ULONG __stdcall Release() override R0;

	//IPersist
	//virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override R0;

	//IPersistStream
	//virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override { return AbstractClass::_Save(this,pStm,fClearDirty); }
	virtual HRESULT __stdcall Load(IStream* pStm)  override JMP_STD(0x5F5E80);

	//virtual HRESULT __stdcall GetSizeMax(ULARGE_INTEGER* pcbSize) override { return S_OK; }

	//IRTTITypeInfo
	//virtual AbstractType __stdcall What_Am_I() const override RT(AbstractType);
	//virtual int __stdcall Fetch_ID() const override R0;
	//virtual void __stdcall Create_ID() override RX;

	//INoticeSink
	//virtual bool __stdcall INoticeSink_Unknown(DWORD dwUnknown) override R0;

	//INoticeSource
	//virtual void __stdcall INoticeSource_Unknown() override RX;

	//Destructor
	virtual ~ObjectClass() JMP_THIS(0x5F6DC0);

	//AbstractClass
	virtual void PointerExpired(AbstractClass* pAbstract, bool bremoved) override JMP_THIS(0x5F5230);
	virtual bool IsDead() const override JMP_THIS(0x5F6690);
	virtual void Update() override JMP_THIS(0x5F3E70);

	//ObjectClass
	virtual void AnimPointerExpired(AnimClass* pAnim) JMP_THIS(0x5F6DA0);
	virtual bool IsSelectable() const R0;
	virtual VisualType VisualCharacter(VARIANT_BOOL SpecificOwner, HouseClass * WhoIsAsking) const RT(VisualType);
	virtual SHPStruct* GetImage() const R0;
	virtual Action MouseOverCell(CellStruct const& cell, bool checkFog = false, bool ignoreForce = false) const RT(Action);
	virtual Action MouseOverObject(ObjectClass const* pObject, bool ignoreForce = false) const RT(Action);
	virtual Layer InWhichLayer() const JMP_THIS(0x5F4260);
	virtual bool IsSurfaced() { JMP_THIS(0x5F6C10); }// opposed to being submerged

 /*
	Building returns if it is 1x1 and has UndeploysInto
	inf returns 0
	unit returns !NonVehicle
	Aircraft returns IsOnFloor()

  users include:
  452656 - is this building click-repairable
  440C26 - should this building get considered in BaseSpacing
  445A8E - -""-
  51E7D1 - can a VehicleThief be clicked to steal this unit
  51E4D9 - can an engi be clicked to enter this to fix/takeover
  51F0D3 - -""-
  51EA06 - can this building be C4'd?
  51E243 - can a VehicleThief steal this on his own decision
  4F93F3 - should this building's damage raise a BaseUnderAttack?
  442286 - -""-
  44296A - -""-
  741117 - can this be healed by a vehicle?
  6F8242 - can this aircraft be auto-target
  6F85BE - can this aircraft be auto-attacked
  */
	virtual bool IsStrange() const R0;

	virtual TechnoTypeClass* GetTechnoType() const R0;
	virtual ObjectTypeClass* GetType() const R0;

	virtual DWORD GetTypeOwners() const R0; // returns the data for IndexBitfield<HouseTypeClass*>
	virtual const wchar_t* GetUIName() const R0;
	virtual bool CanBeRepaired() const R0;
	virtual bool CanBeSold() const R0;
	virtual bool IsActive() const R0; //Can_Player_Fire ; this fucking naming is confusing , wtf

	// can the current player control this unit? (owned by him, not paralyzed, not spawned, not warping, not slaved...)
	virtual bool IsControllable() const R0;

	// On non-buildings this is same as GetCenterCoord(), on buildings it returns the target coordinate that is affected by TargetCoordOffset.
	virtual CoordStruct* GetTargetCoords(CoordStruct* pCrd) const R0; //0xA4

	// gets a building's free dock coordinates for a unit. falls back to this->GetCoords(pCrd);
	virtual CoordStruct* GetDockCoords(CoordStruct* pCrd, TechnoClass* docker) const R0; //0xA8

	// stupid! guess what happens again?
	virtual CoordStruct* GetRenderCoords(CoordStruct* pCrd) const { JMP_THIS(0x41BE00); } //0xAC , GetPosition_2
	virtual CoordStruct* GetFLH(CoordStruct *pDest, int idxWeapon, CoordStruct BaseCoords) const R0; //0xB0 mcoord_4263D0
	virtual CoordStruct* GetExitCoords(CoordStruct* pCrd, DWORD dwUnk) const R0; //0xB4 Exit_Coord
	virtual int GetYSort() const { JMP_THIS(0x5F6BD0); }
	virtual bool IsOnBridge(TechnoClass* pDocker = nullptr) const R0; // pDocker is passed to GetDestination
	virtual bool IsStandingStill() const R0;
	virtual bool IsDisguised() const R0; //__Has_Disguise 0xC4
	virtual bool IsDisguisedAs(HouseClass *target) const R0; // only works correctly on infantry!
	virtual ObjectTypeClass* GetDisguise(bool DisguisedAgainstAllies) const R0; //.__Show_As_Type 0xCC
	virtual HouseClass* GetDisguiseHouse(bool DisguisedAgainstAllies) const R0; // __Show_As___targetable 0xD0

	// remove object from the map
	virtual bool Limbo() R0;

	// place the object on the map
	virtual bool Unlimbo(const CoordStruct& Crd, DirType dFaceDir) JMP_THIS(0x5F4EC0);

	// cleanup things (lose line trail, deselect, etc). Permanently: destroyed/removed/gone opposed to just going out of sight.
	virtual void Disappear(bool permanently) RX;

	virtual void RegisterDestruction(TechnoClass *Destroyer) JMP_THIS(0x744720); //E0

	 // maybe Object instead of Techno? Raises Map Events, grants veterancy, increments house kill counters
	virtual void RegisterKill(HouseClass *Destroyer) RX; // ++destroyer's kill counters , etc E0

	virtual bool SpawnParachuted(const CoordStruct& coords) JMP_THIS(0x5F5940);
	virtual void DropAsBomb() RX;
	virtual void MarkAllOccupationBits(const CoordStruct& coords) RX;
	virtual void UnmarkAllOccupationBits(const CoordStruct& coords) RX;
	virtual void UnInit() JMP_THIS(0x5F65F0);  // RemoveThis_DeleteThis 0xF8
	virtual void Reveal() RX; // uncloak when object is bumped, damaged, detected, ... , DoShimmer  0xFC
	virtual KickOutResult KickOutUnit(TechnoClass* pTechno, CellStruct Cell) RT(KickOutResult); //Exit Object 0x100
	virtual bool DrawIfVisible(RectangleStruct *pBounds, bool EvenIfCloaked, DWORD dwUnk3) const JMP_THIS(0x5F4B10);
	virtual CellStruct const* GetFoundationData(bool includeBib = false) const R0;
	virtual void DrawBehind(Point2D* pLocation, RectangleStruct* pBounds) const RX;
	virtual void DrawExtras(Point2D* pLocation, RectangleStruct* pBounds) const RX; // draws ivan bomb, health bar, talk bubble, etc
	virtual void DrawIt(Point2D* pLocation, RectangleStruct* pBounds) const RX;
	virtual void DrawAgain(const Point2D& location, const RectangleStruct& bounds) const RX; // just forwards the call to Draw
	virtual void Undiscover() RX; //hidden
	virtual void See(DWORD dwUnk, DWORD dwUnk2) RX;
	virtual bool Mark(MarkType value) R0;
	virtual RectangleStruct* GetDimensions(RectangleStruct* pRect) const R0;
	virtual RectangleStruct* GetRenderDimensions(RectangleStruct* pRect) R0;
	virtual void DrawRadialIndicator(DWORD dwUnk) RX;
	virtual void MarkForRedraw() RX;
	virtual bool CanBeSelected() const R0;
	virtual bool CanBeSelectedNow() const R0;
	virtual bool CellClickedAction(Action action, CellStruct* pCell, CellStruct* pCell1, bool bUnk) R0; //vt_entry_140
	virtual bool ObjectClickedAction(Action action, ObjectClass* pTarget, bool bUnk) R0;
	virtual void Flash(int Duration) RX;
	virtual bool Select() R0;
	virtual void Deselect() RX;
	virtual DamageState IronCurtain(int nDuration, HouseClass *pSource, bool ForceShield) RT(DamageState);
	virtual void StopAirstrikeTimer() RX;
	virtual void StartAirstrikeTimer(int Duration) RX;
	virtual bool IsIronCurtained() const R0;
	virtual bool IsCloseEnough3D(DWORD dwUnk, DWORD dwUnk2) const R0;
	virtual int GetWeaponRange(int idxWeapon) const R0;
	virtual DamageState ReceiveDamage(int* pDamage, int DistanceFromEpicenter, WarheadTypeClass* pWH,
	  ObjectClass* Attacker, bool IgnoreDefenses, bool PreventPassengerEscape, HouseClass* pAttackingHouse) JMP_THIS(0x5F5390);
	virtual void Destroy() RX;
	virtual void Scatter(const CoordStruct &crd, bool ignoreMission, bool ignoreDestination) RX;
	virtual bool Ignite() R0;
	virtual void Extinguish() RX;
	virtual DWORD GetPointsValue() const R0;
	virtual Mission GetCurrentMission() const RT(Mission);
	virtual void RestoreMission(Mission mission) RX;
	virtual void UpdatePosition(PCPType dwUnk) RX; // PCP
	virtual BuildingClass* FindFactory(bool allowOccupied, bool requirePower) const R0; //who can build me
	virtual RadioCommand ReceiveCommand(TechnoClass* pSender, RadioCommand command, AbstractClass* &pInOut) RT(RadioCommand); //receive message
	virtual bool DiscoveredBy(HouseClass *pHouse) R0;
	virtual void SetRepairState(int state) RX; // 0 - off, 1 - on, -1 - toggle
	virtual void Sell(int Control) RX; // -1 if Mission::Deconst , 0 if not Mission::Deconst , > 1 Play Generic Click
	virtual void AssignPlanningPath(signed int idxPath, signed char idxWP) RX;
	virtual void MoveToDirection(FacingType facing) RX; // Vestigial, never called by the game.
	virtual Move IsCellOccupied(CellClass *pDestCell, FacingType facing, int level, CellClass* pSourceCell, bool alt) const RT(Move); //can_enter_cell
	virtual DWORD vt_entry_1B0(DWORD dwUnk, DWORD dwUnk2, DWORD dwUnk3, DWORD dwUnk4, DWORD dwUnk5) R0;
	virtual void SetLocation(const CoordStruct& crd) RX;

// these two work through the object's Location
	virtual CellStruct* GetMapCoords(CellStruct* pUCell) const R0;  // be aware that some objects have a different coordinate system , like AnimClass where it can use AttachedObject coordinates
	virtual CellClass* GetCell() const R0; // be aware that some objects have a different coordinate system , like AnimClass where it can use AttachedObject coordinates

// these two call ::GetCoords_() instead
	virtual CellStruct* GetMapCoordsAgain(CellStruct* pUCell) const R0;
	virtual CellClass* GetCellAgain() const R0;

	virtual int GetHeight() const R0;
	virtual void SetHeight(DWORD dwUnk) RX;
	virtual int GetZ() const R0;
	virtual bool IsBeingWarpedOut() const R0; //1D4 70C5B0
	virtual bool IsWarpingIn() const R0; // 70C5C0
	virtual bool IsWarpingSomethingOut() const R0; //70C5D0
	virtual bool IsNotWarping() const R0; //70C5F0
	virtual LightConvertClass *GetRemapColour() const R0;

	DirStruct* GetDirectionOverObject(DirStruct* pBuffer, AbstractClass* Target) const
		{ JMP_THIS(0x5F3DB0); }

	DirStruct GetDirectionOverObject(AbstractClass* Target) const
	{
		DirStruct pBuffer;
		this->GetDirectionOverObject(&pBuffer, Target);

		return pBuffer;
	}

	//CoordStruct GetCenterCoords() const
	//{
	//	CoordStruct ret;
	//	this->GetCenterCoords(&ret);
	//	return ret;
	//}

	CoordStruct* GetLocationCoords(CoordStruct* pRet) const
	{    //return this->Location
		JMP_THIS(0x5F65A0); }

	CoordStruct GetLocationCoords() const
	{
		CoordStruct nBuff;
		GetLocationCoords(&nBuff);
		return nBuff;
	}

	CoordStruct GetRenderCoords() const {
		CoordStruct ret;
		this->GetRenderCoords(&ret);
		return ret;
	}

	double GetHealthPercentage_() const;

	//game original func
	double GetHealthPercentage() const;

	bool IsRedHP() const
		{ JMP_THIS(0x5F5CD0); }

	bool IsYellowHP() const
		{ JMP_THIS(0x5F5D20); }

	bool IsGreenHP() const
		{ JMP_THIS(0x5F5D90); }

	bool IsGreenToYellowHP() const;

	bool IsFullHP() const;

	bool IsThisBreathing() const { JMP_THIS(0x6EF9E0); }

	HealthState GetHealthStatus() const
		{ JMP_THIS(0x5F5DD0); }

	void ReplaceTag(TagClass* pTag) //AttachTrigger
		{ JMP_THIS(0x5F5B50); }

	void AttachTrigger(TagClass* pTag) //
		{ JMP_THIS(0x5F5B50); }

	int GetCellLevel() const
		{ JMP_THIS(0x5F5F00); }

	CellStruct GetMapCoords() const {
		CellStruct ret;
		this->GetMapCoords(&ret);
		return ret;
	}

	CellStruct GetDestinationMapCoords() const {
		CellStruct ret;
		this->GetMapCoordsAgain(&ret);
		return ret;
	}

	CoordStruct GetDockCoords(TechnoClass* docker) const
	{
		CoordStruct ret;
		this->GetDockCoords(&ret, docker);
		return ret;
	}

	// On non-buildings this is same as GetCenterCoord(), on buildings it returns the target coordinate that is affected by TargetCoordOffset.
	CoordStruct GetTargetCoords() const
	{
		CoordStruct ret;
		this->GetTargetCoords(&ret);
		return ret;
	}

	//CoordStruct GetFLH(int idxWeapon, const CoordStruct& base) const {
	//	CoordStruct ret;
	//	this->GetFLH(&ret, idxWeapon, base);
	//	return ret;
	//}

	CellStruct InlineMapCoords() const {
		return { short(this->Location.X / 256) , short(this->Location.Y / 256) };
	}

	bool IsOnMyView() const;

	void AdjustStrength(double percentage) const
		{ JMP_THIS(0x5F5C80); }

	void SetHealthPercentage(double percentage) const
		{ JMP_THIS(0x5F5C80); }

	//only accept BuildingClass it seems
	void RemoveSidebarObject() const
		{ JMP_THIS(0x734270); }

	void MarkDownSetZ(int Z) const {
		JMP_THIS(0x5F6060);
	}

	bool IsCrushable(TechnoClass* pCrusher) {
		JMP_THIS(0x5F6CD0);
	}

	DamageState TakeDamage(int damage, WarheadTypeClass* pWH, bool crewed, bool ignoreDefenses = true, ObjectClass* pAttacker = nullptr, HouseClass* pAttackingHouse = nullptr) {
		return ReceiveDamage(&damage, 0, pWH, pAttacker, ignoreDefenses, crewed, pAttackingHouse);
	}

	DamageState TakeDamage(int damage, bool crewed, bool ignoreDefenses = true, ObjectClass* pAttacker = nullptr, HouseClass* pAttackingHouse = nullptr);

	// smooth operator
	const char* get_ID() const;

	Point2D GetScreenLocation() const;

	__declspec(property(get = GetHeight, put = SetHeight)) int HeightAGL;
	__declspec(property(get = GetZ, put = MarkDownSetZ)) int Height;
	__declspec(property(get = GetCoords, put = SetLocation)) CoordStruct PositionCoord;

//Constructor NEVER CALL IT DIRECTLY
	ObjectClass()  noexcept
		: ObjectClass(noinit_t())
	{ JMP_THIS(0x5F3900); }

protected:
	explicit __forceinline ObjectClass(noinit_t)  noexcept
		: AbstractClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	DWORD              unknown_24;
	DWORD              unknown_28;
	int                FallRate; //how fast is it falling down? only works if FallingDown is set below, and actually positive numbers will move the thing UPWARDS
	ObjectClass*       NextObject;	//Next Object in the same cell or transport. This is a linked list of Objects.
	TagClass*          AttachedTag; //Should be TagClass , TODO: change when implemented
	BombClass*         AttachedBomb; //Ivan's little friends.
	DECLARE_PROPERTY(AudioController, AmbientSoundController); // the "mofo" struct, evil evil stuff
	DECLARE_PROPERTY(AudioController, CustomSoundController); // the "mofo" struct, evil evil stuff
	int                CustomSound;
	bool               BombVisible; // In range of player's bomb seeing units, so should draw it
	PROTECTED_PROPERTY(BYTE, align_69[0x3]);
	int                Health;		//The current Health.
	int                EstimatedHealth; // used for auto-targeting threat estimation
	bool               IsOnMap; // has this object been placed on the map?
	PROTECTED_PROPERTY(BYTE, align_75[0x3]);
	DWORD              unknown_78;
	DWORD              unknown_7C;
	bool               NeedsRedraw;
	bool               InLimbo; // act as if it doesn't exist - e.g., post mortem state before being deleted
	bool               InOpenToppedTransport;
	bool               IsSelected;	//Has the player selected this Object?
	bool               HasParachute;	//Is this Object parachuting?
	PROTECTED_PROPERTY(BYTE, align_85[0x3]);
	AnimClass*         Parachute;		//Current parachute Anim.
	bool               OnBridge;
	bool               IsFallingDown;
	bool               WasFallingDown; // last falling state when FootClass::Update executed. used to find out whether it changed.
	bool               IsABomb; // if set, will explode after FallingDown brings it to contact with the ground
	bool               IsAlive;		//Self-explanatory.
	PROTECTED_PROPERTY(BYTE, align_91[0x3]);
	Layer              LastLayer;
	bool               IsInLogic; // has this object been added to the logic collection?
	bool               IsVisible; // was this object in viewport when drawn?
	PROTECTED_PROPERTY(BYTE, align_99[0x2]);
	CoordStruct        Location; //Absolute current 3D location (in leptons) , be aware that some objects have a different coordinate system , like AnimClass where it can use AttachedObject coordinates , use GetCoords() to get the correct coordinates
	LineTrail*         LineTrailer;
 };

static_assert(sizeof(ObjectClass) == 0xAC);
