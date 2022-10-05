#pragma once

#include <ObjectClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <Notifications.h>

class DECLSPEC_UUID("31694969-717f-5d7d-a049-5c7184fec604")
ShieldObject final : public ObjectClass
{
public:

	static DynamicVectorClass<ShieldObject*> Array;

	//IUnknown
	virtual HRESULT __stdcall QueryInterface(REFIID iid, void** ppvObject) { return S_OK; }
	virtual ULONG __stdcall AddRef() { return 0; }
	virtual ULONG __stdcall Release() { return 0; }

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) { return S_OK; }

	//IPersistStream
	virtual HRESULT __stdcall IsDirty() { return S_OK; }

	virtual HRESULT __stdcall Load(IStream* pStm) { return S_OK; }
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) { return S_OK; }

	virtual HRESULT __stdcall GetSizeMax(ULARGE_INTEGER* pcbSize) { return S_OK; }

	//IRTTITypeInfo
	virtual AbstractType __stdcall What_Am_I() const { return static_cast<AbstractType>(74); }
	virtual int __stdcall Fetch_ID() const { return 0; }
	virtual void __stdcall Create_ID() { }

	//INoticeSink
	virtual bool __stdcall INoticeSink_Unknown(DWORD dwUnknown) { return 0; }

	//INoticeSource
	virtual void __stdcall INoticeSource_Unknown() { }

	//Destructor
	virtual ~ShieldObject() {

		AbstractClass::AnnounceExpiredPointer(true);
		ShieldObject::Array.Remove(this);
		PointerExpiredNotification::NotifyInvalidObject->Remove(this);

	}

	//AbstractClass
	virtual void Init() { }
	virtual void PointerExpired(AbstractClass* pAbstract, bool removed) {

		if (Owner == pAbstract)
			Owner = nullptr;
	}

	virtual AbstractType WhatAmI() const { return static_cast<AbstractType>(74); }
	virtual int Size() const { return sizeof(*this); }
	virtual void CalculateChecksum(Checksummer& checksum) const {}
	virtual int GetOwningHouseIndex() const { return 0; }
	virtual HouseClass* GetOwningHouse() const { return nullptr;}
	virtual int GetArrayIndex() const { return 0; }
	virtual bool IsDead() const { return 0; }
	virtual CoordStruct GetCoords() const { return CoordStruct::Empty; }  //center coords
	virtual CoordStruct GetDestination(TechnoClass* pDocker = nullptr) const { return CoordStruct::Empty;} // where this is moving, or a building's dock for a techno. iow, a rendez-vous point
	virtual bool IsOnFloor() const { return 0; }
	virtual bool IsInAir() const { return 0; }
	virtual CoordStruct* GetCenterCoords(CoordStruct* pCrd) const { return nullptr;} //GetCoords__
	virtual void Update(){}

	//ObjectClass
	virtual void AnimPointerExpired(AnimClass* pAnim) { }
	virtual bool IsSelectable() const { return 0; }
	virtual VisualType VisualCharacter(VARIANT_BOOL SpecificOwner, HouseClass* WhoIsAsking) const { return VisualType::Indistinct; }
	virtual SHPStruct* GetImage() const { return nullptr;}
	virtual Action MouseOverCell(CellStruct const& cell, bool checkFog = false, bool ignoreForce = false) const { return Action::None; }
	virtual Action MouseOverObject(ObjectClass const* pObject, bool ignoreForce = false) const { return Action::None; }
	virtual Layer InWhichLayer() const { return Layer::None; }
	virtual bool IsSurfaced() { return 0; } // opposed to being submerged

	virtual bool IsStrange() const { return 0; }

	virtual TechnoTypeClass* GetTechnoType() const { return Owner ? Owner->GetTechnoType() : nullptr; }
	virtual ObjectTypeClass* GetType() const { return nullptr;}
	virtual DWORD GetTypeOwners() const { return 0;} // returns the data for IndexBitfield<HouseTypeClass*>
	virtual const wchar_t* GetUIName() const { return nullptr;}
	virtual bool CanBeRepaired() const { return 0; }
	virtual bool CanBeSold() const { return 0; }
	virtual bool IsActive() const { return 0; } //Can_Player_Fire ; this fucking naming is confusing , wtf

	// can the current player control this unit? (owned by him, not paralyzed, not spawned, not warping, not slaved...)
	virtual bool IsControllable() const { return 0; }

	// stupid! return this->GetCoords(pCrd);
	virtual CoordStruct* GetTargetCoords(CoordStruct* pCrd) const { return nullptr; }

	// gets a building's free dock coordinates for a unit. falls back to this->GetCoords(pCrd);
	virtual CoordStruct* GetDockCoords(CoordStruct* pCrd, TechnoClass* docker) const { return nullptr; }

	// stupid! guess what happens again?
	virtual CoordStruct* GetRenderCoords(CoordStruct* pCrd) const { return nullptr; } //GetPosition_2
	virtual CoordStruct* GetFLH(CoordStruct* pDest, int idxWeapon, CoordStruct BaseCoords) const { return nullptr; }
	virtual CoordStruct* GetExitCoords(CoordStruct* pCrd, DWORD dwUnk) const { return nullptr; }
	virtual int GetYSort() const { return 0; }
	virtual bool IsOnBridge(TechnoClass* pDocker = nullptr) const { return 0; } // pDocker is passed to GetDestination
	virtual bool IsStandingStill() const { return 0; }
	virtual bool IsDisguised() const { return 0; } //__Has_Disguise 0xC4
	virtual bool IsDisguisedAs(HouseClass* target) const { return 0; } // only works correctly on infantry!
	virtual ObjectTypeClass* GetDisguise(bool DisguisedAgainstAllies) const { return nullptr; } //.__Show_As_Type 0xCC
	virtual HouseClass* GetDisguiseHouse(bool DisguisedAgainstAllies) const { return nullptr; } // __Show_As___targetable 0xD0

	// remove object from the map
	virtual bool Limbo() { return 0; }

	// place the object on the map
	virtual bool Unlimbo(const CoordStruct& Crd, DirType dFaceDir) { return 0; }

	// cleanup things (lose line trail, deselect, etc). Permanently: destroyed/removed/gone opposed to just going out of sight.
	virtual void Disappear(bool permanently) { }

	virtual void RegisterDestruction(TechnoClass* Destroyer) { }

	// maybe Object instead of Techno? Raises Map Events, grants veterancy, increments house kill counters
	virtual void RegisterKill(HouseClass* Destroyer) { }// ++destroyer's kill counters , etc

	virtual bool SpawnParachuted(const CoordStruct& coords) { return false; };
	virtual void DropAsBomb() {}
	virtual void MarkAllOccupationBits(const CoordStruct& coords){}
	virtual void UnmarkAllOccupationBits(const CoordStruct& coords){}
	virtual void UnInit() {}  // RemoveThis_DeleteThis 0xF8
	virtual void Reveal(){} // uncloak when object is bumped, damaged, detected, ... , DoShimmer  0xFC
	virtual KickOutResult KickOutUnit(TechnoClass* pTechno, CellStruct Cell) { return KickOutResult::Succeeded;  } //Exit Object 0x100
	virtual bool DrawIfVisible(RectangleStruct* pBounds, bool EvenIfCloaked, DWORD dwUnk3) const { return 0; }
	virtual CellStruct const* GetFoundationData(bool includeBib = false) const { return nullptr; }
	virtual void DrawBehind(Point2D* pLocation, RectangleStruct* pBounds) const{}
	virtual void DrawExtras(Point2D* pLocation, RectangleStruct* pBounds) const{} // draws ivan bomb, health bar, talk bubble, etc
	virtual void DrawIt(Point2D* pLocation, RectangleStruct* pBounds) const{}
	virtual void DrawAgain(const Point2D& location, const RectangleStruct& bounds) const{} // just forwards the call to Draw
	virtual void Undiscover(){} //hidden
	virtual void See(DWORD dwUnk, DWORD dwUnk2){}
	virtual bool UpdatePlacement(PlacementType value) { return 0; }
	virtual RectangleStruct* GetDimensions(RectangleStruct* pRect) const { return nullptr;}
	virtual RectangleStruct* GetRenderDimensions(RectangleStruct* pRect) { return nullptr;}
	virtual void DrawRadialIndicator(DWORD dwUnk){}
	virtual void MarkForRedraw(){}
	virtual bool CanBeSelected() const { return 0; }
	virtual bool CanBeSelectedNow() const { return 0; }
	virtual bool CellClickedAction(Action action, CellStruct* pCell, CellStruct* pCell1, bool bUnk) { return 0; } //vt_entry_140
	virtual bool ObjectClickedAction(Action action, ObjectClass* pTarget, bool bUnk) { return 0; }
	virtual void Flash(int Duration){}
	virtual bool Select() { return 0; }
	virtual void Deselect(){}
	virtual DamageState IronCurtain(int nDuration, HouseClass* pSource, bool ForceShield) RT(DamageState);
	virtual void StopAirstrikeTimer(){}
	virtual void StartAirstrikeTimer(int Duration){}
	virtual bool IsIronCurtained() const { return 0; }
	virtual bool IsCloseEnough3D(DWORD dwUnk, DWORD dwUnk2) const { return 0; }
	virtual int GetWeaponRange(int idxWeapon) const { return 0;}
	virtual DamageState ReceiveDamage(int* pDamage, int DistanceFromEpicenter, WarheadTypeClass* pWH,
	  ObjectClass* Attacker, bool IgnoreDefenses, bool PreventPassengerEscape, HouseClass* pAttackingHouse) RT(DamageState);
	virtual void Destroy(){}
	virtual void Scatter(const CoordStruct& crd, bool ignoreMission, bool ignoreDestination){}
	virtual bool Ignite() { return 0; }
	virtual void Extinguish(){}
	virtual DWORD GetPointsValue() const { return 0; }
	virtual Mission GetCurrentMission() const { return Mission::None; }
	virtual void RestoreMission(Mission mission){}
	virtual void UpdatePosition(int dwUnk){} // PCP
	virtual BuildingClass* FindFactory(bool allowOccupied, bool requirePower) const { return nullptr;} //who can build me
	virtual RadioCommand ReceiveCommand(TechnoClass* pSender, RadioCommand command, AbstractClass*& pInOut) { return RadioCommand::AnswerInvalid; }//receive message
	virtual bool DiscoveredBy(HouseClass* pHouse) { return 0; }
	virtual void SetRepairState(int state){} // 0 - off, 1 - on, -1 - toggle
	virtual void Sell(int Control){} // -1 if Mission::Deconst , 0 if not Mission::Deconst , > 1 Play Generic Click
	virtual void AssignPlanningPath(signed int idxPath, signed char idxWP){}
	virtual void vt_entry_1A8(DWORD dwUnk){}
	virtual Move IsCellOccupied(CellClass* pDestCell, int facing, int level, CellClass* pSourceCell, bool alt) const { return Move::OK; } //can_enter_cell
	virtual DWORD vt_entry_1B0(DWORD dwUnk, DWORD dwUnk2, DWORD dwUnk3, DWORD dwUnk4, DWORD dwUnk5) { return 0; }
	virtual void SetLocation(const CoordStruct& crd){}

	// these two work through the object's Location
	virtual CellStruct* GetMapCoords(CellStruct* pUCell) const { return nullptr;}
	virtual CellClass* GetCell() const { return nullptr;}

	// these two call ::GetCoords_() instead
	virtual CellStruct* GetMapCoordsAgain(CellStruct* pUCell) const { return nullptr;}
	virtual CellClass* GetCellAgain() const { return nullptr;}

	virtual int GetHeight() const { return 0; }
	virtual void SetHeight(DWORD dwUnk){}
	virtual int GetZ() const { return 0; }
	virtual bool IsBeingWarpedOut() const { return 0; }
	virtual bool IsWarpingIn() const { return 0; }
	virtual bool IsWarpingSomethingOut() const { return 0; }
	virtual bool IsNotWarping() const { return 0; }
	virtual LightConvertClass* GetRemapColour() const { return nullptr;}

	static ShieldObject* CreateMe() {
		return GameCreate<ShieldObject>();
	}

	ShieldObject()  noexcept
		: ObjectClass()
		, Owner { nullptr }
	{
		CreateID();
		Init();
		ShieldObject::Array.AddItem(this);
		PointerExpiredNotification::NotifyInvalidObject->Add(this);

	}

	ShieldObject(TechnoClass* pTechno)  noexcept
		: ObjectClass()
		, Owner { pTechno }
	{
		CreateID();
		Init();
		ShieldObject::Array.AddItem(this);
		PointerExpiredNotification::NotifyInvalidObject->Add(this);
	}

protected:
	explicit __forceinline ShieldObject(noinit_t) noexcept
		: ObjectClass(noinit_t())
	{ }
public:
	TechnoClass* Owner;
};