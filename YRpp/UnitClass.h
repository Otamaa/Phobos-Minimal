/*
	Ground & Naval Vehicles
*/

#pragma once

#include <FootClass.h>
#include <UnitTypeClass.h>

//forward declarations
class EBolt;

class DECLSPEC_UUID("0E272DCA-9C0F-11D1-B709-00A024DDAFD1")
	NOVTABLE UnitClass : public FootClass
{
public:
	static const AbstractType AbsID = AbstractType::Unit;

	//Static
	static COMPILETIMEEVAL constant_ptr<DynamicVectorClass<UnitClass*>, 0x8B4108u> const Array{};
	static COMPILETIMEEVAL OPTIONALINLINE DWORD vtable = 0x7F5C70;

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x746DE0);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_THIS(0x744470);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_THIS(0x744600);

	//Destructor
	virtual ~UnitClass() override JMP_THIS(0x746E80);

	//AbstractClass
	virtual void Init() override JMP_THIS(0x746810);
	virtual void PointerExpired(AbstractClass* pAbstract, bool bremoved) override JMP_THIS(0x7446E0);
	virtual AbstractType WhatAmI() const override RT(AbstractType);
	virtual int Size() const override R0;
	virtual void Update() override JMP_THIS(0x7360C0);

	//ObjectClass
	virtual Action MouseOverObject(ObjectClass const* pObject, bool ignoreForce = false) const override JMP_THIS(0x73FD50);
	virtual DamageState ReceiveDamage(int* pDamage, int DistanceFromEpicenter, WarheadTypeClass* pWH,
	ObjectClass* Attacker, bool IgnoreDefenses, bool PreventPassengerEscape, HouseClass* pAttackingHouse) override JMP_THIS(0x737C90);

	//TechnoClass
	virtual InfantryTypeClass* GetCrew() const override { JMP_THIS(0x740EE0); }
	virtual int SelectWeapon(AbstractClass* pTarget) const override JMP_THIS(0x746CD0);
	virtual FireError GetFireError(AbstractClass *pTarget, int nWeaponIndex, bool ignoreRange) const override JMP_THIS(0x740FD0);
	virtual bool SetOwningHouse(HouseClass* pHouse, bool announce = true) override JMP_THIS(0x7463A0);

	//FootClass
	virtual bool ApproachTarget(bool bSomething) override JMP_THIS(0x7414E0);

	//UnitClass
	// main drawing functions - Draw() calles one of these, they call parent's Draw_A_smth
	virtual void DrawAsVXL(Point2D Coords, RectangleStruct BoundingRect, int Brightness, int Tint)
		{ JMP_THIS(0x73B470); }

	virtual void DrawAsSHP(Point2D Coords, RectangleStruct BoundingRect, int Brightness, int Tint)
		{ JMP_THIS(0x73C5F0); }

	virtual void DrawObject(Surface* pSurface, Point2D Coords, RectangleStruct CacheRect, int Brightness, int Tint)
		{ JMP_THIS(0x73B140); }

	// non-virtual
	bool IsDeactivated() const
		{ JMP_THIS(0x70FBD0); }

	void UpdateTube() JMP_THIS(0x7359F0);
	void UpdateRotation() JMP_THIS(0x736990);
	void UpdateEdgeOfWorld() JMP_THIS(0x736C10); // inlined in game
	void UpdateFiring() JMP_THIS(0x736DF0);
	void UpdateVisceroid() JMP_THIS(0x737180);
	void UpdateDisguise() JMP_THIS(0x7468C0);
	//void Update() JMP_THIS(0x7360C0);

	void Explode() JMP_THIS(0x738680);

	bool GotoClearSpot() JMP_THIS(0x738D30);
	bool TryToDeploy() JMP_THIS(0x7393C0);
	void Deploy() JMP_THIS(0x739AC0);
	void Undeploy() JMP_THIS(0x739CD0);

	bool Harvesting() JMP_THIS(0x73D450);

	bool FlagAttach(int nHouseIdx) JMP_THIS(0x740DF0);
	bool FlagRemove() JMP_THIS(0x740E20);

	void APCCloseDoor() JMP_THIS(0x740E60); // inlined in game
	void APCOpenDoor() JMP_THIS(0x740E80); // inlined in game

	static void __fastcall ReadINI(CCINIClass* pINI) JMP_FAST(0x743270);
	static void __fastcall WriteINI(CCINIClass* pINI) JMP_FAST(0x7436E0);

	bool ShouldCrushIt(TechnoClass* pTarget) JMP_THIS(0x7438F0);

	AbstractClass* AssignDestination_7447B0(AbstractClass* pTarget) JMP_THIS(0x7447B0);
	bool AStarAttempt(const CellStruct& cell1, const CellStruct& cell2) JMP_THIS(0x746000);

	//Constructor
	UnitClass(UnitTypeClass* pType, HouseClass* pOwner) noexcept : UnitClass(noinit_t())
		{ JMP_THIS(0x7353C0); }

protected:
	explicit __forceinline UnitClass(noinit_t) noexcept
		: FootClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	int CurrentFiringFrame;
	UnitTypeClass* Type;
	UnitClass* FollowerCar; // groovy - link defined in the map's [Units] section, looked up on startup
	int FlagHouseIndex; //Carrying the flag of this House
	bool IsFollowerCar; // yes, this vehicle has a another vehicle (a train's locomotive towing the cars)
	bool Unloading;
	bool IsHarvesting; //6D2
	bool TerrainPalette;
	int unknown_int_6D4;
	int DeathFrameCounter;
	EBolt* ElectricBolt; //Unit is the owner of this
	bool Deployed;
	bool Deploying;
	bool Undeploying;
	BYTE padding_6E3;
	int NonPassengerCount; // Set when unloading passengers. Units with TurretCount>0 will not unload the gunner.

	wchar_t ToolTipText[0x100];
};
static_assert(offsetof(UnitClass, padding_6E3) == 0x6E3, "ClassMember Shifted !");
static_assert(sizeof(UnitClass) == 0x8E8,"Invalid size.");