#pragma once

#include <GScreenClass.h>
#include <ArrayClasses.h>
#include <CellClass.h>
#include <AnimTypeClass.h>
#include <Interface/IGameMap.h>
#include <SpecificStructures.h>
#include <Powerups.h>
#include <HashTable.h>

class BulletTypeClass;
class ObjectClass;
class WarheadTypeClass;
class WeaponTypeClass;

// Terrain ground type
struct GroundType
{
	std::array<float, 8u> Cost; // Terrain speed multipliers.
	char Build; // Can build on this terrain?
public:

	static COMPILETIMEEVAL reference<GroundType, 0x89EA40u, 12u> const Array {};

	static COMPILETIMEEVAL FORCEDINLINE GroundType* Get(LandType land) {
		return &Array[(int)land];
	}

	static LandType __fastcall GetLandTypeFromName(const char* name) {
		JMP_FAST(0x48DF80);
	}

	static COMPILETIMEEVAL FORCEDINLINE float GetCost(LandType land, SpeedType speed) {
		return Get(land)->Cost[(int)speed];
	}
};
static_assert(sizeof(GroundType) == 0x24, "Invalid Size !");

struct CrackedIceStruct
{
	bool operator==(const CrackedIceStruct& src) const { return false; }
	bool operator!=(const CrackedIceStruct& src) const { return true; }

	CellStruct Pos;
	int CrackingAtFrames; // cracking frame?
};

struct CellLevelPassabilityStruct
{
	char CellPassability;
	char CellLevel;
	unsigned short ZoneArrayIndex;
};

struct GlobalPassabilityData
{
	WORD data[4];
	bool byte1;
	bool byte2;
};
static_assert(sizeof(GlobalPassabilityData) == 0xA);

//ZoneConnectionClass - Holding zone connection info from tubes or bridges (probably used for pathfinding)
struct ZoneConnectionClass
{
	CellStruct	FromMapCoords;
	CellStruct	ToMapCoords;
	bool		unknown_bool_08;
	CellClass*	Cell;

	//need to define a == operator so it can be used in array classes
	bool operator==(const ZoneConnectionClass &other) const {
		return (FromMapCoords.SimilarTo(other.FromMapCoords)
			&& ToMapCoords.SimilarTo(other.ToMapCoords)
			&& unknown_bool_08 == other.unknown_bool_08
			&& Cell == other.Cell);
	}
};
static_assert(sizeof(ZoneConnectionClass) == 0x10, "Invalid Size !");

struct SubzoneConnectionStruct
{
	DWORD NeighborSubzoneIndex;  // Index of connected subzone (was unknown_dword_0)
	BYTE ConnectionPenaltyFlag;   // If set, adds 0.001 penalty to path cost (was unknown_byte_4)

	//need to define a == operator so it can be used in array classes
	bool operator==(const SubzoneConnectionStruct &other) const {
		return (NeighborSubzoneIndex == other.NeighborSubzoneIndex
			&& ConnectionPenaltyFlag == other.ConnectionPenaltyFlag);
	}
};
static_assert(sizeof(SubzoneConnectionStruct) == 0x8, "Invalid Size !");

struct SubzoneTrackingStruct
{
	static COMPILETIMEEVAL reference<DynamicVectorClass<SubzoneTrackingStruct>, 0x87F874 ,3u> const Array {};

	DynamicVectorClass<SubzoneConnectionStruct> SubzoneConnections;
	WORD ParentZoneIndex;          // Index into parent hierarchy level (was unknown_word_18)
	DWORD MovementCostType;        // Index into adjustments_7E3794 array for movement cost (was unknown_dword_1C)
	DWORD unknown_dword_20;

	//need to define a == operator so it can be used in array classes
	bool operator==(const SubzoneTrackingStruct &other) const {
		return (ParentZoneIndex == other.ParentZoneIndex
			&& MovementCostType == other.MovementCostType
			&& unknown_dword_20 == other.unknown_dword_20);
	}
};
static_assert(sizeof(SubzoneTrackingStruct) == 0x24, "Invalid Size !");

// helper class with static methods to detect projectile collisions
struct TrajectoryHelper
{
	// whether the bullet hit a cliff when moving from pBefore to pAfter
	static bool __fastcall IsCliffHit(
		CellClass const* pSource, CellClass const* pBefore,
		CellClass const* pAfter)
	{ JMP_FAST(0x4CC680); }

	// whether the bullet hit a wall when traversing through pCheck
	static bool __fastcall IsWallHit(
		CellClass const* pSource, CellClass const* pCheck,
		CellClass const* pTarget, HouseClass const* pOwner)
	{ JMP_FAST(0x4CC6D0); }

	// returns the cell at crdCur if it contains an obstacle, nullptr otherwise
	static CellClass* __fastcall GetObstacle(
		CellClass const* pCellSource, CellClass const* pCellTarget,
		CellClass const* pCellBullet, CoordStruct crdCur,
		BulletTypeClass const* pType, HouseClass const* pOwner)
	{ JMP_FAST(0x4CC360); }

	// assumes linear movement, returns the first cell that has a cliff or wall
	// in it, a nullptr otherwise.
	static CellClass* __fastcall FindFirstObstacle(
		CoordStruct const& crdSrc, CoordStruct const& crdTarget,
		BulletTypeClass const* pType, HouseClass const* pOwner)
	{ JMP_FAST(0x4CC100); }

	// if the warhead can destroy walls, walls don't count as obstacle
	static CellClass* __fastcall FindFirstImpenetrableObstacle(
		CoordStruct const& crdSrc, CoordStruct const& crdTarget,
		WeaponTypeClass const* pWeapon, HouseClass const* pOwner)
	{ JMP_FAST(0x4CC310); }
};

class LayerClass : public DynamicVectorClass<ObjectClass*>
{
public:
	virtual bool AddObject(ObjectClass* pObject, bool sorted)
		{ JMP_THIS(0x5519B0); }

	virtual void RemoveAll()
		{ this->clear(); }

	virtual void vt_entry_24()
		{ }

	HRESULT Load(IStream* pStm)
		{ JMP_THIS(0x551B90); }

	HRESULT Save(IStream* pStm)
		{ JMP_THIS(0x551B20); }

	void Sort()
		{ JMP_THIS(0x551A30); }
};
static_assert(sizeof(LayerClass) == 0x18, "Invalid Size !");

class LogicClass : public LayerClass
{
public:

	virtual bool AddObject(ObjectClass* pObject, bool sorted) override
		{ JMP_THIS(0x55BAA0); }

	virtual void PointerGotInvalid(AbstractClass* pInvalid, bool removed)
		{ JMP_THIS(0x55B880); }

	void RemoveObject(ObjectClass* pObject)
		{ JMP_THIS(0x55BAE0); }

	void Update()
		{ JMP_THIS(0x55AFB0); }

//	static COMPILETIMEEVAL reference<LogicClass, 0x87F778u> const Instance {};

};
static_assert(sizeof(LogicClass) == 0x18, "Invalid Size !");

#include <Dimensions.h>

class NOVTABLE MapClass : public GScreenClass
{
public:
	//Static
	//static COMPILETIMEEVAL reference<int, 0x87F914u> const MapCellWidth{};
	//static COMPILETIMEEVAL reference<int, 0x87F918u> const MapCellHeight{};
	static COMPILETIMEEVAL reference<Dimensions, 0x87F914u> const MapCellDimension {};
	static COMPILETIMEEVAL reference<RectangleStruct, 0x87F8E4> const MapLocalSize {};
	static COMPILETIMEEVAL reference<RectangleStruct, 0x87F8D4> const MapSize {};

	static COMPILETIMEEVAL constant_ptr<MapClass, 0x87F7E8u> const Instance {};
	static COMPILETIMEEVAL reference<CellClass, 0xABDC50u> const InvalidCell {};
	static COMPILETIMEEVAL reference<LogicClass, 0x87F778u> const Logics {};
	static OPTIONALINLINE COMPILETIMEEVAL int const MaxCells = 0x40000;

	// WARNING: This is DEPRECATED - use MapClass::Instance->LevelAndPassabilityStruct2pointer_70 instead!
	// The address 0x87F858 contains a POINTER, not the array itself.
	// constant_ptr returns (GlobalPassabilityData*)0x87F858 which is wrong - it should dereference the pointer.
	// Correct usage: MapClass::Instance->LevelAndPassabilityStruct2pointer_70[index]
	static COMPILETIMEEVAL constant_ptr<GlobalPassabilityData, 0x87F858u> const GlobalPassabilityDatas {};

	// this actually points to 5 vectors, one for each layer
	static COMPILETIMEEVAL reference<LayerClass, 0x8A0360u, 5u> const ObjectsInLayers {};

	static LayerClass* GetLayer(Layer lyr)
	{
		return (lyr >= Layer::Underground && lyr <= Layer::Top)
			? &ObjectsInLayers[static_cast<int>(lyr)]
			: nullptr;
	}

	/// <summary>
	/// Some sort of hardcoded constant lookup matrix with rows (0-8) representing CellClass Passability(Type) and columns are MovementZones, used to determine pathfinding behaviour.
	/// </summary>
	static COMPILETIMEEVAL reference<std::array<int,8u>, 0x82A594u, 13u> const MovementAdjustArray { };

	//IGameMap
	virtual BOOL __stdcall Is_Visible(CellStruct cell) override R0;

	//Destructor
	virtual ~MapClass() override JMP_THIS(0x588BF0);

	//GScreenClass
	virtual void One_Time() override JMP_THIS(0x565800);
	virtual void Init_Clear() override JMP_THIS(0x5659F0);

	//MapClass
	virtual void AllocateCells() JMP_THIS(0x565AA0);
	virtual void DestructCells() JMP_THIS(0x565B00);
	virtual void ConstructCells() JMP_THIS(0x565BC0);
	virtual void PointerGotInvalid(AbstractClass* ptr, bool bUnk) JMP_THIS(0x577920);
	virtual bool DraggingInProgress() JMP_THIS(0x4AEBE0);
	virtual void UpdateCrates() JMP_THIS(0x56BBE0);
	virtual void CreateEmptyMap(const RectangleStruct& mapRect, bool reuse, char nLevel, bool bUnk2) JMP_THIS(0x565C10);
	virtual void SetVisibleRect(const RectangleStruct& mapRect) JMP_THIS(0x567230);

	//GetCellAt
	//CellClass* operator[] (const CoordStruct&  cell)
	//{ JMP_THIS(0x565730); }

	// Get cellclasspointer with cellstruct but it will return to instance pointer if invalid !
	COMPILETIMEEVAL FORCEDINLINE CellClass* GetCellAt(CoordStruct* pCoord)
	{
		CellStruct cell = CellClass::Coord2Cell(*pCoord);
		return GetCellAt(cell);
	}

	// Get cellclasspointer with cellstruct but it will return to instance pointer if invalid !
	COMPILETIMEEVAL FORCEDINLINE CellClass* GetCellAt(CellStruct* pCellStruct) const
	{
		return GetCellAt(*pCellStruct);
	}

	//GetCellAt
	//CellClass* operator[] (const CellStruct&  cell)
	//{ JMP_THIS(0x5657A0); }

	//Non-virtuals

	// Get cellclasspointe with cellstruct Pointer but it will return nullptr if invalid !
	// this one usually used on operator[]
	COMPILETIMEEVAL CellClass* TryGetCellAt(const CellStruct& MapCoords) const
	{
		int idx = GetCellIndex(MapCoords);
		return (idx >= 0 && idx < Cells.Capacity) ? Cells.Items[idx] : nullptr;
	}

	COMPILETIMEEVAL CellClass* TryGetCellAt(CellStruct* MapCoords) const
	{
		int idx = GetCellIndex(MapCoords);
		return (idx >= 0 && idx < Cells.Capacity) ? Cells.Items[idx] : nullptr;
	}
	// ??
	// COMPILETIMEEVAL CellClass* TryGetCellAtB(const CellStruct& MapCoords) const
	// {
	// 	int idx = GetCellIndex(MapCoords);
	// 	return (idx >= 0 && idx < MaxCells) ? Cells.Items[idx] : nullptr;
	// }
	// Get cellclasspointer with coords but it will return nullptr if invalid !
	COMPILETIMEEVAL CellClass* TryGetCellAt(const CoordStruct& Crd) const
	{
		CellStruct cell = CellClass::Coord2Cell(Crd);
		return TryGetCellAt(cell);
	}

	// Get cellclasspointer with coords but it will return nullptr if invalid !
	COMPILETIMEEVAL CellClass* TryGetCellAt(CoordStruct* Crd) const
	{
		CellStruct cell = CellClass::Coord2Cell(Crd);
		return TryGetCellAt(cell);
	}

	// Get cellclasspointer with cellstruct but it will return to instance pointer if invalid !
	COMPILETIMEEVAL CellClass* GetCellAt(const CellStruct& MapCoords) const
	{
		auto pCell = TryGetCellAt(MapCoords);

		if (!pCell)
		{
			pCell = CellClass::Instance();
			pCell->MapCoords = MapCoords;
		}

		return pCell;
	}

	// Get cellclasspointer with coords but it will return to instance pointer if invalid !
	COMPILETIMEEVAL FORCEDINLINE CellClass* GetCellAt(const CoordStruct& Crd) const
	{
		CellStruct cell = CellClass::Coord2Cell(Crd);
		return GetCellAt(cell);
	}

	// Get cellclasspointer with pointe2d location but it will return to instance pointer if invalid !
	COMPILETIMEEVAL CellClass* GetTargetCell(Point2D& location)
	{
		CellStruct cell = {
			static_cast<short>(location.X / 256),
			static_cast<short>(location.Y / 256)
		};

		return GetCellAt(cell);
	}

	// Is cellclass pointer is valid after using `TryGetCellAt` !
	COMPILETIMEEVAL FORCEDINLINE bool CellExists(const CellStruct& MapCoords) const
	{
		return TryGetCellAt(MapCoords) != nullptr;
	}

	int GetThreatPosed(const CoordStruct& coord, HouseClass* pHouse) const
	{
		auto cell = CellClass::Coord2Cell(coord);
		return GetThreatPosed(cell, pHouse);

	}

	int GetThreatPosed(const CellStruct& cell, HouseClass* pHouse) const
	{ JMP_THIS(0x56BCD0); }

	bool IsLocationShrouded(const CoordStruct& crd) const
	{ JMP_THIS(0x586360); }

	static COMPILETIMEEVAL FORCEDINLINE int GetCellIndex(const CellStruct& MapCoords)
	{
		return MapCoords.X + (MapCoords.Y << 9);
	}

	static COMPILETIMEEVAL FORCEDINLINE int GetCellIndex(CellStruct* MapCoords)
	{
		return MapCoords->X + (MapCoords->Y << 9);
	}

	// gets a coordinate in a random direction a fixed distance in leptons away from coords
	static CoordStruct* __fastcall GetRandomCoordsNear(CoordStruct& outBuffer, const CoordStruct& coords, int distance, bool center)
	{
		JMP_FAST(0x49F420);
	}

	static CoordStruct* __fastcall GetRandomCoordsNear(CoordStruct* poutBuffer, const CoordStruct& coords, int distance, bool center)
	{
		JMP_FAST(0x49F420);
	}

	// gets a coordinate in a random direction a fixed distance in leptons away from coords
	static FORCEDINLINE CoordStruct GetRandomCoordsNear(const CoordStruct& coords, int distance, bool center)
	{
		CoordStruct outBuffer;
		GetRandomCoordsNear(outBuffer, coords, distance, center);
		return outBuffer;
	}

	static CoordStruct* __stdcall PickInfantrySublocation(CoordStruct& outBuffer, const CoordStruct& coords, bool ignoreContents = false)
	{ JMP_STD(0x4ACA10); }

	static FORCEDINLINE CoordStruct PickInfantrySublocation(const CoordStruct& coords, bool ignoreContents = false)
	{
		CoordStruct outBuffer;
		PickInfantrySublocation(outBuffer, coords, ignoreContents);
		return outBuffer;
	}

	static void __fastcall UnselectAll()
	{ JMP_FAST(0x48DC90); }

	static CellStruct* __fastcall GetAdjacentCell(CellStruct* ret, CellStruct* from, FacingType dir)
	{
		JMP_FAST(0x487EA0);
	}

	void CenterMap()
	{ JMP_THIS(0x4AE290); }

	void CellIteratorReset()
	{ JMP_THIS(0x578350); }

	CellClass* CellIteratorNext()
	{ JMP_THIS(0x578290); }

	// the key damage delivery
	/*! The key damage delivery function.
		\param Coords Location of the impact/center of damage.
		\param Damage Amount of damage to deal.
		\param SourceObject The object which caused the damage to be delivered (iow, the shooter).
		\param WH The warhead to use to apply the damage.
		\param AffectsTiberium If this is false, Tiberium=yes is ignored.
		\param SourceHouse The house to which SourceObject belongs, the owner/bringer of damage.
	*/
	// static DamageAreaResult __fastcall DamageArea(
	// 	const CoordStruct& Coords,
	// 	int Damage,
	// 	TechnoClass* SourceObject,
	// 	WarheadTypeClass *WH,
	// 	bool AffectsTiberium,
	// 	HouseClass* SourceHouse)
	// 		{ JMP_STD(0x489280); }

	// static DamageAreaResult __fastcall DamageArea(
	// 	CoordStruct* pCoords,
	// 	int Damage,
	// 	TechnoClass* SourceObject,
	// 	WarheadTypeClass* WH,
	// 	bool AffectsTiberium,
	// 	HouseClass* SourceHouse)
	// 	{ JMP_STD(0x489280); }
	/*
	 * Picks the appropriate anim from WH's AnimList= based on damage dealt and land type (Conventional= )
	 * so after DamageArea:
	 * if(AnimTypeClass *damageAnimType = SelectDamageAnimation(...)) {
	 * 	GameCreate<AnimClass>(damageAnimType, location);
	 * }
	 */
	static AnimTypeClass* __fastcall SelectDamageAnimation
	(int Damage, WarheadTypeClass* WH, LandType LandType, const CoordStruct& coords)
	{
		JMP_FAST(0x48A4F0);
	}

	static AnimTypeClass* __fastcall SelectDamageAnimation
	(int Damage, WarheadTypeClass* WH, LandType LandType, CoordStruct* pCoords)
	{
		JMP_FAST(0x48A4F0);
	}

	static void __fastcall FlashbangWarheadAt
	(int Damage, WarheadTypeClass* WH, CoordStruct coords, bool Force = 0, SpotlightFlags CLDisableFlags = SpotlightFlags::None)
	{
		JMP_FAST(0x48A620);
	}

	static FORCEDINLINE void FlashbangWarheadAt
	(int Damage, WarheadTypeClass* WH, CoordStruct* pCoord, bool Force, SpotlightFlags CLDisableFlags)
	{
		auto nCoord = *pCoord;
		FlashbangWarheadAt(Damage, WH, nCoord, Force, CLDisableFlags);
	}

	// get the damage a warhead causes to specific armor
	//static int __fastcall GetTotalDamage(int damage, const WarheadTypeClass* pWarhead, Armor armor, int distance)
	//{ JMP_STD(0x489180); }

	//static int __fastcall ModifyDamage(int damage, const WarheadTypeClass* pWarhead, Armor armor, int distance)
	//{ JMP_STD(0x489180); }

	//static FORCEDINLINE void ModifyDamage(args_ReceiveDamage* const args, Armor armor)
	//{ *args->Damage = ModifyDamage(*args->Damage, args->WH, armor, args->DistanceToEpicenter); }

	//static FORCEDINLINE void GetTotalDamage(args_ReceiveDamage* const args, Armor armor)
	//{ *args->Damage = ModifyDamage(*args->Damage, args->WH, armor, args->DistanceToEpicenter); }

	static void __fastcall AtomDamage(int OwnedHouse, CellStruct& nCell)
	{ JMP_FAST(0x4251F0); }

	int GetCellFloorHeight(const CoordStruct& crd) const
	{ JMP_THIS(0x578080); }

	int GetCellFloorHeight(CoordStruct* pCrd) const
	{ JMP_THIS(0x578080); }

	CellStruct* PickCellOnEdge(CellStruct& buffer, Edge Edge, const CellStruct& CurrentLocation, const CellStruct& Fallback,
		SpeedType SpeedType, bool ValidateReachability, MovementZone MovZone) const
	{
		JMP_THIS(0x4AA440);
	}

	CellStruct FORCEDINLINE PickCellOnEdge(Edge Edge, const CellStruct& CurrentLocation, const CellStruct& Fallback,
		SpeedType SpeedType, bool ValidateReachability, MovementZone MovZone) const
	{
		CellStruct buffer;
		this->PickCellOnEdge(buffer, Edge, CurrentLocation, Fallback, SpeedType, ValidateReachability, MovZone);
		return buffer;
	}

	// Pathfinding voodoo
	// do not touch them, mmkay, they trigger ZoneConnection recalc which is a MUST for firestorm to work

	void Update_Pathfinding_1()
	{ JMP_THIS(0x56C510); }

	void Update_Pathfinding_2(const DynamicVectorClass<CellStruct>& where)
	{ JMP_THIS(0x586990); }

	// Find nearest spot
	CellStruct* NearByLocation(CellStruct& outBuffer, const CellStruct& position, SpeedType SpeedType, ZoneType a5, MovementZone MovementZone, bool alt, int SpaceSizeX, int SpaceSizeY, bool disallowOverlay, bool a11, bool requireBurrowable, bool allowBridge, const CellStruct& closeTo, bool a15, bool buildable)
	{ JMP_THIS(0x56DC20); }

	CellStruct FORCEDINLINE NearByLocation(const CellStruct& position, SpeedType SpeedType, ZoneType a5, MovementZone MovementZone, bool alt, int SpaceSizeX, int SpaceSizeY, bool disallowOverlay, bool a11, bool requireBurrowable, bool allowBridge, const CellStruct& closeTo, bool a15, bool buildable)
	{
		CellStruct outBuffer;
		NearByLocation(outBuffer, position, SpeedType, a5, MovementZone, alt, SpaceSizeX, SpaceSizeY, disallowOverlay, a11, requireBurrowable, allowBridge, closeTo, a15, buildable);
		return outBuffer;
	}

	void  AddContentAt(CellStruct* coords, TechnoClass* Content)
	{ JMP_THIS(0x5683C0); }

	void RemoveContentAt(CellStruct* coords, TechnoClass* Content)
	{ JMP_THIS(0x5687F0); }


	bool IsWithinUsableArea(const CellStruct& cell, bool checkLevel) const
	{ JMP_THIS(0x578460); }

	bool IsWithinUsableArea(CellClass* pCell, bool checkLevel) const
	{ JMP_THIS(0x578540); }

	bool IsWithinUsableArea(const CoordStruct& coords) const
	{ JMP_THIS(0x5785F0); }

	bool CoordinatesLegal(const CellStruct& cell) const
	{ JMP_THIS(0x568300); }

	bool CoordinatesLegal(CellStruct* pCell) const
	{ JMP_THIS(0x568300); }

	CellClass* MapClass_findnearbyshroud_580BC0(AbstractClass* arg0) const
	{ JMP_THIS(0x580BC0); }

	// Called on wall state updates etc. when the wall hasn't been removed.
	void RecalculateZones(CellStruct const& cell) const
	{ JMP_THIS(0x56D5A0); }

	// Called on wall state updates etc. when the wall HAS been removed.
	void ResetZones(CellStruct const& cell) const
	{ JMP_THIS(0x56D460); }

	// Called on wall state updates etc
	void RecalculateSubZones(CellStruct const& cell) const
	{ JMP_THIS(0x584550); }

	// ====================================
	//         FIRESTORM RELATED
	// ====================================

	CoordStruct* FindFirstFirestorm(
		CoordStruct* pOutBuffer, const CoordStruct& start,
		const CoordStruct& end, HouseClass const* pHouse = nullptr) const
	{
		JMP_THIS(0x5880A0);
	}

	CoordStruct FORCEDINLINE FindFirstFirestorm(
		const CoordStruct& start, const CoordStruct& end,
		HouseClass const* pHouse = nullptr) const
	{
		CoordStruct outBuffer;
		FindFirstFirestorm(&outBuffer, start, end, pHouse);
		return outBuffer;
	}

	// ====================================
	//        MAP REVEAL BRAINDAMAGE
	// ====================================

	/*
	 * TechnoClass::Fire uses this for RevealOnFire on player's own units (radius = 3)
	 * TechnoClass::See uses this on all (singleCampaign || !MultiplayPassive) units
	 * TalkBubble uses this to display the unit to the player
	 */
	void RevealArea1(
		CoordStruct* Coords,
		int Radius,
		HouseClass* OwnerHouse,
		CellStruct arg4,
		BYTE RevealByHeight,
		BYTE arg6,
		BYTE arg7,
		BYTE arg8)
	{
		JMP_THIS(0x5673A0);
	}

	/*
	 * these come in pairs - first the last argument is 0 and then 1

	 * AircraftClass::Fire - reveal the target area to the owner (0,0,0,1,x)
	 * AircraftClass::See - reveal shroud when on the ground (arg,arg,0,1,x), and fog always (0,0,1,(height < flightlevel/2),x)
	 * AnimClass::AnimClass - reveal area to player if anim->Type = [General]DropZoneAnim= (radius = Rules->DropZoneRadius /256) (0,0,0,1,x)
	 * BuildingClass::Place - reveal (r = 1) to player if this is ToTile and owned by player (0,0,0,1,x)
	 * BuildingClass::Unlimbo - reveal (radius = this->Type->Sight ) to owner (0,0,0,1,x)
	 * PsychicReveal launch - reveal to user (0,0,0,0,x)
	 * ActionClass::RevealWaypoint - reveal RevealTriggerRadius= to player (0,0,0,1,x)
	 * ActionClass::RevealZoneOfWaypoint - reveal (r = 2) to player (0,0,0,1,x)
	 */
	void RevealArea2(
		CoordStruct* Coords,
		int Radius,
		HouseClass* OwnerHouse,
		DWORD /*CellStruct*/ arg4,
		BYTE RevealByHeight,
		BYTE arg6,
		BYTE arg7,
		BYTE arg8)
	{
		JMP_THIS(0x5678E0);
	}

	/*
	 * AircraftClass::SpyPlaneApproach
	 * AircraftClass::SpyPlaneOverfly
	 * AircraftClass::Carryall_Unload
	 * BuildingClass::Place - RevealToAll
	 * Foot/Infantry Class::Update/UpdatePosition
	 * MapClass::RevealArea0 calls this to do the work
	 * ParasiteClass::Infect/PointerGotInvalid
	 * TechnoClass::Unlimbo
	 * TechnoClass::Fire uses this (r = 4) right after using RevealArea0, wtfcock
	 */
	void RevealArea3(CoordStruct* Coords, int Height, int Radius, bool SkipReveal)
	{ JMP_THIS(0x567DA0); }

	void Reveal(HouseClass* pHouse)
	{ JMP_THIS(0x577D90); }

	void Reveal_B(HouseClass* pHouse)
	{ JMP_THIS(0x577F30); }

	void Reshroud(HouseClass* pHouse)
	{ JMP_THIS(0x577AB0); }

	int GetZPos(CoordStruct* Coords)
	{ JMP_THIS(0x578080); }

	// these two VERY slowly reprocess the map after gapgen state changes
	void Map_AI() //657CE0
	{ JMP_THIS(0x657CE0); }

	//GScreenClass::MarkNeedsRedraw
	void RedrawSidebar(int mode)
	{ JMP_THIS(0x4F42F0); }

	ObjectClass* NextObject(ObjectClass* pCurrentObject)
	{ JMP_THIS(0x4AA2B0); }

	void SetTogglePowerMode(int mode)
	{ JMP_THIS(0x4AC820); }

	void SetPlaceBeaconMode(int mode)
	{ JMP_THIS(0x4AC960); }

	void SetSellMode(int mode)
	{ JMP_THIS(0x4AC660); }

	void SetWaypointMode(int mode, bool somebool)
	{ JMP_THIS(0x4AC700); }

	void SetRepairMode(int mode)
	{ JMP_THIS(0x4AC8C0); }

	void DestroyCliff(CellClass* Cell)
	{ JMP_THIS(0x581140); }

	bool IsLocationFogged(const CoordStruct& coord)
	{ JMP_THIS(0x5865E0); }

	bool IsLocationFogged(CoordStruct&& coord)
	{ return IsLocationFogged(coord); }

	void RevealCheck(CellClass* pCell, HouseClass* pHouse, bool bUnk)
	{ JMP_THIS(0x5865F0); }

	// returns false if visitor should wait for a gate to open, true otherwise
	bool MakeTraversable(ObjectClass const* pVisitor, CellStruct const& cell) const
	{ JMP_THIS(0x578AD0); }

	void BuildingToFirestormWall(CellStruct const& cell, HouseClass* pHouse, BuildingTypeClass* pBldType)
	{ JMP_THIS(0x588570); }

	void BuildingToWall(CellStruct const& cell, HouseClass* pHouse, BuildingTypeClass* pBldType)
	{ JMP_THIS(0x588750); }

	bool Place_Crate(CellStruct cell, PowerupEffects crateType)
	{ JMP_THIS(0x56BEC0); }

	bool Remove_Crate(CellStruct* where)
	{ JMP_THIS(0x56C020); }

	bool Place_Random_Crate()
	{ JMP_THIS(0x56BD40); }

	int GetMapZone(CellStruct* where, MovementZone movementZone, bool isBridge)
	{ JMP_THIS(0x56D230); }

	int GetMapZone(const CellStruct& MapCoords, MovementZone movementZone, bool isBridge)
	{ JMP_THIS(0x56D230); }

	ZoneType GetMovementZoneType(const CellStruct& MapCoords, MovementZone movementZone, bool isBridge) const
	{ JMP_THIS(0x56D230); }

	ZoneType GetMovementZoneType(CellStruct* where, MovementZone movementZone, bool isBridge) const
	{ JMP_THIS(0x56D230); }

	int Zone_56DA10(CellStruct* where, int nLessCond, int nZoneConnectionIndex)  const
	{ JMP_THIS(0x56DA10); }

	CellStruct* SubZone_5833F0(CellStruct* a1, CellStruct* a2, int a3) const
	{ JMP_THIS(0x5833F0); }

	CellStruct SubZone_5835D0(CellStruct* a1, CellStruct* a2, int a4) const
	{ JMP_THIS(0x5835D0); }

	int Index(CellClass* ptr) const { return Cells.find(ptr); }
	int Index(CellClass& ptr) const { return Cells.find(&ptr); }

	bool IsValidCell(CellStruct* cell) { JMP_THIS(0x5657E0); }
	bool IsValidCell(CellStruct const& nCoord) { JMP_THIS(0x5657E0); }

	bool CanMoveHere(const CellStruct& position, int arg4, int a3, SpeedType SpeedType, int zone, MovementZone MovementZone, int level, bool bool1, char a9)
	{ JMP_THIS(0x56E7C0); }

	bool IsValid(CoordStruct const& nCoord) const
	{
		JMP_THIS(0x568350);
	}

	bool Is_In_Same_Zone_56D100(CellStruct* cell1, CellStruct* cell2, MovementZone mzone, bool isbridge1, bool isbridge2, bool skip_zone) const
	{
		JMP_THIS(0x56D100);
	}

	bool IsBrideRepairNeeded(CellStruct const& nCell)
	{ JMP_THIS(0x587410); }

	bool IsLinkedBridgeDestroyed(const CellStruct& cell) const
	{ JMP_THIS(0x587410); }

	CellStruct* Localsize_586AC0(CellStruct* buffer, CellStruct* from, bool dec) const
	{ JMP_THIS(0x586AC0); }

	CellStruct Localsize_586AC0(CellStruct* from, bool dec) const
	{
		CellStruct nBuffer;
		this->Localsize_586AC0(&nBuffer, from, dec);
		return nBuffer;
	}

	int MapClass_zone_56D3F0(CellStruct* cell) const
	{
		JMP_THIS(0x56D3F0);
	}

	bool findsoemthing_587180(CellStruct* a2) const
	{
		JMP_THIS(0x587180);
	}

	bool checkcells_57BAA0(CellStruct* a3) const
	{
		JMP_THIS(0x57BAA0);
	}

	bool checkcells_57CCF0(CellStruct* a3) const
	{
		JMP_THIS(0x57CCF0);
	}

	int subZone_585F40(HouseClass* arg0, int arg1, int arg2, int arg4) {
		JMP_THIS(0x585F40);
	}

	void RepairWoodBridgeAt(CellStruct const& cell)
	{ JMP_THIS(0x570050); }

	void RepairConcreteBridgeAt(CellStruct const& cell)
	{ JMP_THIS(0x573540); }

	void DestroyWoodBridgeAt(CellStruct const& cell)
	{ JMP_THIS(0x574C20); }

	void DestroyConcreteBridgeAt(CellStruct const& cell)
	{ JMP_THIS(0x574000); }

	DWORD*  Clear_SubzoneTracking()
	{ JMP_THIS(0x581F50); }

	//find_type 0 - 3 ,range and threadposed related
	static BuildingClass* __fastcall FindEnemyBuilding(BuildingTypeClass* type, HouseClass* house, TechnoClass* attacker, int find_type, bool OnlyTargetHouseEnemy)
		{ JMP_FAST(0x6EEBD0); }

	static BuildingClass* __fastcall FindOwnBuilding(BuildingTypeClass* type, DWORD* unusedptr, TechnoClass* attacker, int find_type)
		{ JMP_FAST(0x6EEEA0); }

protected:
	//Constructor
	MapClass() {}	//don't need this

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:
	DWORD unknown_10;
	HashTable<DWORD,DWORD>* unknown_pointer_14;
	void* MovementZones [13];
	DWORD somecount_4C;
	DynamicVectorClass<ZoneConnectionClass> ZoneConnections;
	CellLevelPassabilityStruct* LevelAndPassability;
	int ValidMapCellCount;
	GlobalPassabilityData* LevelAndPassabilityStruct2pointer_70;
	DWORD unknown_74;
	DWORD unknown_78;
	DWORD unknown_7C;
	HashTable<DWORD, SubzoneConnectionStruct>* unknown_80[3]; // somehow connected to the 3 vectors below
	DynamicVectorClass<SubzoneTrackingStruct> SubzoneTracking1;
	DynamicVectorClass<SubzoneTrackingStruct> SubzoneTracking2;
	DynamicVectorClass<SubzoneTrackingStruct> SubzoneTracking3;
	DynamicVectorClass<CellStruct> CellStructs1;
	RectangleStruct MapRect;
	RectangleStruct VisibleRect;
	int CellIterator_NextX;
	int CellIterator_NextY;
	int CellIterator_CurrentY;
	CellClass* CellIterator_NextCell;
	int ZoneIterator_X;
	int ZoneIterator_Y;
	LTRBStruct MapCoordBounds; // the minimum and maximum cell struct values
	int TotalValue;
	VectorClass<CellClass*> Cells;
	int MaxLevel;
	int MaxWidth;
	int MaxHeight;
	int MaxNumCells;
	Powerups Crates [0x100];
	BOOL Redraws;
	DynamicVectorClass<CellStruct> TaggedCells;
};
static_assert(sizeof(MapClass) == 0x1174, "Invalid Size !");