/*
	Cells
*/

#pragma once

#include <AbstractClass.h>
#include <RectangleStruct.h>
#include <array>

struct TileTypeData
{
	TileType Type;
	DWORD Data;
};

class NOVTABLE PixelFXClass
{
public:

	virtual ~PixelFXClass() RX;

	PixelFXClass(int entry) { JMP_THIS(0x631E30); }
	PixelFXClass() { JMP_THIS(0x631E10); }

	void CalculateColor(int timing) { JMP_THIS(0x631E50); }
	void SetOffsets(Point2D nLoc) { JMP_THIS(0x631F10); }
	bool IsToDraw(int timing) { JMP_THIS(0x631EE0); }

private:

	void Init(int entry) { JMP_THIS(0x631D40); }

public:
	int FinalRGB[3];
	DWORD BitMask;
	DWORD BitmaskOffset;
	int MaxRGB[3];
	int WorkingRGB[3];
	int XOffset;
	int YOffset;
	int Timing38;
};

static_assert(sizeof(PixelFXClass) == 0x3C);

//forward declarations
class ObjectClass;
class TechnoClass;
class BuildingClass;
class BuildingTypeClass;
class BulletTypeClass;
class UnitClass;
class InfantryClass;
class AircraftClass;
class TerrainClass;
class LightConvertClass;
class RadSiteClass;
class FootClass;
class TubeClass;
class FoggedObjectClass;
class TagClass;
class TiberiumClass;

class DECLSPEC_UUID("C1BF99CE-1A8C-11D2-8175-006008055BB5")
	NOVTABLE CellClass : public AbstractClass
{
public:
	static const AbstractType AbsID = AbstractType::Cell;
	static constexpr inline DWORD vtable = 0x7E4EEC;

	// the height of a bridge in leptons
	static inline constexpr int BridgeLevels = 4;
	static inline constexpr int BridgeHeight = BridgeLevels * Unsorted::LevelHeight;
	static constexpr constant_ptr<CellClass,0xABDC50u> const Instance{};
	static constexpr reference<CoordStruct , 0x89E9F0u , 5u> const StoppingCoords{};
	static constexpr reference<const char* const, 0x81DA28u , 12u> const LandTypeToStrings {};
	static constexpr reference<const char* const, 0x81DA58u, 8u> const SpeedTypeToStrings {};
	static constexpr reference<const char* const, 0x81DA78u, 5u> const LayerToStrings {};
	static constexpr reference<const char* const, 0x7E1B60u, 5u> const EdgeToStrings {};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x485200);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x4839F0);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x483C10);

	//Destructor
	virtual ~CellClass() JMP_THIS(0x487E80);

	//AbstractClass
	virtual AbstractType WhatAmI() const override { return AbstractType::Cell; }
	virtual int Size() const override R0;

	//virtual CoordStruct* GetAltCoords(CoordStruct* pCrd) const override JMP_THIS(0x486890); //GetCoords__

	// non-virtual
	static std::array<const DWORD, 21> TileArray;

	//Using typedef resulting on dll address wtf , so this weird code
	//Don't inline this , it will messup the stacks

	bool NOINLINE TileIs(TileType tileType) const
	{
		if (!(int)tileType)
			return false;

		using fp_type = bool(__fastcall*)(const CellClass*, void*);
		return reinterpret_cast<fp_type>(TileArray[(int)tileType])(this, nullptr);
	}

	#define ISTILE(tileset, addr) \
	bool Tile_Is_ ## tileset() const \
		{ JMP_THIS(addr); }

	ISTILE(Tunnel, 0x484AB0);
	ISTILE(Water, 0x485060);
	ISTILE(Blank, 0x486380);
	ISTILE(Ramp, 0x4863A0);
	ISTILE(Cliff, 0x4863D0);
	ISTILE(Shore, 0x4865B0);
	ISTILE(Wet, 0x4865D0);
	ISTILE(MiscPave, 0x486650);
	ISTILE(Pave, 0x486670);
	ISTILE(DirtRoad, 0x486690);
	ISTILE(PavedRoad, 0x4866D0);
	ISTILE(PavedRoadEnd, 0x4866F0);
	ISTILE(PavedRoadSlope, 0x486710);
	ISTILE(Median, 0x486730);
	ISTILE(Bridge, 0x486750);
	ISTILE(WoodBridge, 0x486770);
	ISTILE(ClearToSandLAT, 0x486790);
	ISTILE(Green, 0x4867B0);
	ISTILE(NotWater, 0x4867E0);
	ISTILE(DestroyableCliff, 0x486900);

	#undef ISTILE
	// get content objects
	TechnoClass* FindTechnoNearestTo(Point2D const& offsetPixel, bool alt, TechnoClass const* pExcludeThis = nullptr) const
		{ JMP_THIS(0x47C3D0); }

	ObjectClass* FindObjectOfType(AbstractType abs, bool alt) const
		{ JMP_THIS(0x47C4D0); }

	BuildingClass* GetBuilding() const
		{ JMP_THIS(0x47C520); }

	UnitClass* GetUnit(bool alt) const
		{ JMP_THIS(0x47EBA0); }

	InfantryClass* GetInfantry(bool alt) const
		{ JMP_THIS(0x47EC40); }

	AircraftClass* GetAircraft(bool alt) const
		{ JMP_THIS(0x47EBF0); }

	TerrainClass* GetTerrain(bool alt) const
		{ JMP_THIS(0x47C550); }

	/* craziest thing... first iterates Content looking to Aircraft,
	 * failing that, calls FindTechnoNearestTo,
	 * if that fails too, reiterates Content looking for Terrain
	 */
	ObjectClass* GetSomeObject(Point2D const& offsetPixel, bool alt) const {
		JMP_THIS(0x47C5A0);
	}

	// misc
	void SetWallOwner()
		{ JMP_THIS(0x47D210); }

	void IncreaseShroudCounter()
		{ JMP_THIS(0x487690); }

	void ReduceShroudCounter()
		{ JMP_THIS(0x487630); }

	bool IsShrouded() const
		{ JMP_THIS(0x487950); }

	void Unshroud()
		{ JMP_THIS(0x4876F0); }

	RectangleStruct* OverlayShapeRect(RectangleStruct* pRet)
		{ JMP_THIS(0x47FDE0); }

	RectangleStruct GetOverlayShapeRect()
	{
		RectangleStruct nBuffer;
		this->OverlayShapeRect(&nBuffer);
		return nBuffer;
	}

	bool IsFogged() const // Check Fog maybe?
		{ JMP_THIS(0x4879B0); }

	void FogCell() const
		{ JMP_THIS(0x486A70); }

	void CleanFog() const
		{ JMP_THIS(0x486BF0); }

	void ClearFoggedObjects() const
		{ JMP_THIS(0x486C50); }

	// adjusts LAT
	void SetupLAT() const
		{ JMP_THIS(0x47CA80); }

	// checks for nearby cliff impassability, calls SetupLAT(), sets up TubeClass if tunnel, cell anim if attached, etc
	void Setup(DWORD dwUnk) const
		{ JMP_THIS(0x47D2B0); }

	void BlowUpBridge() const
		{ JMP_THIS(0x47DD70); }

	bool CanThisExistHere(SpeedType SpeedType, BuildingTypeClass* pObject, HouseClass* pOwner) const
		{ JMP_THIS(0x47C620); }

	bool IsClearToMove(SpeedType loco, bool ignoreinfantry, bool ignorevehicles, int zone, MovementZone check, int level, bool bool1) const
		{ JMP_THIS(0x4834A0); }

	// those unks are passed to TechnoClass::Scatter in that same order
	void ScatterContent(const CoordStruct &crd, bool ignoreMission, bool ignoreDestination, bool alt) const
		{ JMP_THIS(0x481670); }

	CellClass* GetNeighbourCell(FacingType facing) const
		{ JMP_THIS(0x481810); }

	//TompsonIDB
	CellClass* GetAdjacentCell(FacingType facing) const
		{ JMP_THIS(0x481810); }

	// called whenever anything moves, first to remove threat from source cell, second time to add threat to dest cell
	void UpdateThreat(unsigned int SourceHouse, int ThreatLevel) const
		{ JMP_THIS(0x481870); }

	void CollectCrate(FootClass* pCollector) const
		{ JMP_THIS(0x481A00); }

	void ProcessColourComponents(int* arg0, int* pIntensity, int* pAmbient, int* a5, int* a6, int* tintR, int* tintG, int* tintB) const
		{ JMP_THIS(0x484180); }

	TubeClass* GetTunnel() const
		{ JMP_THIS(0x484F20); }

	RectangleStruct* GetContainingRect(RectangleStruct* dest) const
		{ JMP_THIS(0x47FB90); }

	RectangleStruct* GetTileRect(RectangleStruct& dest) const
		{ JMP_THIS(0x48001F); }

	// don't laugh, it returns the uiname of contained tiberium... which nobody ever sets
	const wchar_t* GetUIName() const
		{ JMP_THIS(0x484FF0); }

	// returns whether a cell behaves as if it contained overlay (for gates and wall towers)
	bool ConnectsToOverlay(int idxOverlay = -1, int direction = -1) const
		{ JMP_THIS(0x480510); }

	// returns the tiberium's index in OverlayTypes
	int GetContainedTiberiumIndex() const
		{ JMP_THIS(0x485010); }

	int GetContainedTiberiumValue() const
		{ JMP_THIS(0x485020); }

	bool SpreadTiberium(bool forced) const
		{ JMP_THIS(0x483780); }

	// add or create tiberium of the specified type
	bool IncreaseTiberium(int idxTiberium, int amount) const
		{ JMP_THIS(0x487190); }

	// decreases thze tiberium in the cell
	void ReduceTiberium(int amount) const
		{ JMP_THIS(0x480A80); }

	bool CanTiberiumGerminate(TiberiumClass* tib) const
		{ JMP_THIS(0x4838E0); }

	void SetMapCoords(const CoordStruct& coords) const
		{ JMP_THIS(0x485240); }

	// in leptons
	CoordStruct* Get3DCoords(CoordStruct* result) const
		{ JMP_THIS(0x486840); }

	// depends on one of the cell flags being set
	CoordStruct* Get3DCoords2(CoordStruct* result) const
		{ JMP_THIS(0x486890); }

	// used by ambient waves and stuff
	CoordStruct* Get3DCoords3(CoordStruct* result) const
		{ JMP_THIS(0x480A30); }

	int GetFloorHeight(Point2D const& subcoords) const
		{ JMP_THIS(0x47B3A0); }

	// used by ambient waves and stuff in leptons
	// use GetCoords() instead
	//CoordStruct* GetCenterCoords(CoordStruct* pOutBuffer) const
	//	{ JMP_THIS(0x486840); }

	CoordStruct* GetCellCoords(CoordStruct* pOutBuffer) const
		{ JMP_THIS(0x480A30); }

	//CoordStruct GetCenterCoords() const
	//{
	//	CoordStruct buffer;
	//	GetCenterCoords(&buffer);
	//	return buffer;
	//}

	void ActivateVeins() const
		{ JMP_THIS(0x486920); }

	void RedrawForVeins() const
		{ JMP_THIS(0x485AF0); }

	bool IsVeinsExistHere() const
		{ JMP_THIS(0x485460); }
	//
	// cloak generators
	bool CloakGen_InclHouse(unsigned int idx) const
		{ return ((1 << idx) & this->CloakedByHouses) != 0; }

	void CloakGen_AddHouse(unsigned int idx)
		{ this->CloakedByHouses |= 1 << idx; }

	void CloakGen_RemHouse(unsigned int idx)
		{ this->CloakedByHouses &= ~(1 << idx); }

	// unused, returns 0 if that house doesn't have cloakgens covering this cell or Player has sensors over this cell
	bool DrawObjectsCloaked(int OwnerHouseIdx) const
		{ JMP_THIS(0x486800); }

	// sensors
	bool Sensors_InclHouse(unsigned int idx) const
		{ JMP_THIS(0x4870D0); }
		//{ return this->SensorsOfHouses[idx] > 0; }

	void Sensors_AddOfHouse(unsigned int idx)
		{ ++this->SensorsOfHouses[idx]; }

	void Sensors_RemOfHouse(unsigned int idx)
		{ --this->SensorsOfHouses[idx]; }

	// disguise sensors
	bool DisguiseSensors_InclHouse(unsigned int idx) const
		{ return this->DisguiseSensorsOfHouses[idx] > 0; }

	void DisguiseSensors_AddOfHouse(unsigned int idx)
		{ ++this->DisguiseSensorsOfHouses[idx]; }

	void DisguiseSensors_RemOfHouse(unsigned int idx)
		{ --this->DisguiseSensorsOfHouses[idx]; }

	// Rad Sites
	void SetRadSite(RadSiteClass* pRad)
		{ this->RadSite = pRad; }

	RadSiteClass* GetRadSite() const
		{ return this->RadSite; }

	bool IsRadiated() const
		{ JMP_THIS(0x487C90); }

	int GetCurrentRadLevel() const
		{ JMP_THIS(0x487CB0); }

	void RadLevel_Increase(double amount) const
		{ JMP_THIS(0x487CE0); }

	void RadLevel_Decrease(double amount) const
		{ JMP_THIS(0x487D00); }

	// helper
	bool ContainsBridge() const
		{ return (this->Flags & CellFlags::Bridge) != CellFlags::Empty; }

	bool ContainsBridgeEx() const
		{ return (this->Flags & CellFlags::BridgeWithBody) != CellFlags::Empty; }

	bool ContainsBridgeBody() const
		{ return (this->Flags & CellFlags::BridgeBody) != CellFlags::Empty; }

	// helper mimicking game's behaviour
	ObjectClass* GetContent() const
		{ return this->ContainsBridge() ? this->AltObject : this->FirstObject; }

	ObjectClass* GetContentB() const
	{ return (this->ContainsBridgeEx()) ? this->AltObject : this->FirstObject; }

	int GetLevel() const
		{ return this->Level + (this->ContainsBridge() ? BridgeLevels : 0); }

	static CoordStruct Cell2Coord(const CellStruct &cell, int z = 0)
	{
		return { cell.X * 256 + 128  , cell.Y * 256 + 128 ,z};
	}

	static CellStruct Coord2Cell(const CoordStruct &crd)
	{
		CellStruct ret;
		ret.X = static_cast<short>(crd.X / 256);
		ret.Y = static_cast<short>(crd.Y / 256);
		return ret;
	}

	CoordStruct FixHeight(CoordStruct crd) const
	{
		if(this->ContainsBridge()) {
			crd.Z += BridgeHeight;
		}
		return crd;
	}

	// helper - gets coords and fixes height for bridge
	CoordStruct GetCoordsWithBridge() const
	{
		CoordStruct buffer =  this->GetCoords();
		return FixHeight(buffer);
	}

	CoordStruct GetCoordsWithBridge(CoordStruct* pBuffer)
	{
		CoordStruct buffer = this->GetCoords();
		buffer = FixHeight(buffer);
		pBuffer = &buffer;
		return buffer;
	}

	void MarkForRedraw() const
		{ JMP_THIS(0x486E70); }

	void ChainReaction() {
		CellStruct* cell = &this->MapCoords;
		SET_REG32(ecx, cell);
		CALL(0x489270);
	}

	CoordStruct* FindInfantrySubposition(CoordStruct* pOutBuffer, const CoordStruct& coords, bool ignoreContents, bool alt, bool useCellCoords)
		{ JMP_THIS(0x481180); }

	CoordStruct FindInfantrySubposition(const CoordStruct& coords, bool ignoreContents, bool alt, bool useCellCoords , int Zcoords) {
		CoordStruct outBuffer;
		this->FindInfantrySubposition(&outBuffer, coords, ignoreContents, alt, useCellCoords);
		outBuffer.Z = Zcoords;
		return outBuffer;
	}

	bool TryAssignJumpjet(FootClass* pObject) const
		{ JMP_THIS(0x487D70); }

	void AddContent(ObjectClass* Content, bool onBridge) const
		{ JMP_THIS(0x47E8A0); }

	void RemoveContent(ObjectClass* pContent, bool onBridge) const
		{ JMP_THIS(0x47EA90); }

	void ReplaceTag(TagClass* pTag) const
		{ JMP_THIS(0x485250) }

	void UpdateCellLighting() const
		{ JMP_THIS(0x484680); }

	void CalculateLightSourceLighting(int& nIntensity, int& nAmbient, int& Red1, int& Green1, int& Blue1, int& Red2, int& Green2, int& Blue2) const
		{ JMP_THIS(0x484180); }

	void InitLightConvert(LightConvertClass* pDrawer = nullptr, int nIntensity = 0x10000,
		int nAmbient = 0, int Red1 = 1000, int Green1 = 1000, int Blue1 = 1000) const
		{ JMP_THIS(0x483E30); }

	void DrawOverlay(const Point2D& Location, const RectangleStruct& Bound) const
		{ JMP_THIS(0x47F6A0); }

	void DrawOverlayShadow(const Point2D& Location, const RectangleStruct& Bound) const
		{ JMP_THIS(0x47F510); }

	bool IsBuildable() const
		{ JMP_THIS(0x487C10); }

	int GetZPosAdjent() const
		{ JMP_THIS(0x485080); }

	int GetZPosAdjentWithBridge()const
	{
		auto ret = this->GetZPosAdjent();

		if (this->ContainsBridge())
			ret += 412;

		return ret;
	}

	void CellColor(ColorStruct& arg0 , ColorStruct& arg1) const
		{ JMP_THIS(0x47C060); }

	unsigned int MinimapCellColor(void* a1, bool a2 = false) const
		{ JMP_THIS(0x47BDB0); }

	ObjectClass* Cell_Occupier(bool alt = false) const
		{ return alt ? AltObject : FirstObject; }

	unsigned int ReduceWall(int nDamage = -1) const { JMP_THIS(0x480CB0); }

	char DrawIt_47EC90(CoordStruct& nCoord, RectangleStruct& nRect, bool bBlit) const
		{ JMP_THIS(0x47EC90);}

	RectangleStruct* GetTileRect(RectangleStruct* pRet) const
		{ JMP_THIS(0x47FF80); }

	RectangleStruct GetTileRect() const
	{
		RectangleStruct nBuff;
		GetTileRect(&nBuff);
		return nBuff;
	}

	bool HasTiberium() const
		{ JMP_THIS(0x487DF0); }

	bool HasWeed() const
		{ return LandType == LandType::Weeds; }

	bool operator != (const CellClass & cell) const { return cell.MapCoords.DifferTo(MapCoords); }
	bool operator == (const CellClass & cell) const { return cell.MapCoords.SimilarTo(MapCoords); }
	bool IsValidMapCoords() const  { return MapCoords.IsValid(); }
	int GetCliffIndex_() const { JMP_THIS(0x487D50); }
	CellClass* GetBulletObstacleCell(CellClass* cell, CoordStruct coord, BulletTypeClass* bullet, HouseClass* house) const { JMP_THIS(0x4CC360); }

	bool CellClass_Tube_484AE0() const { JMP_THIS(0x484AE0); }
	bool CellClass_Tube_484D60() const { JMP_THIS(0x484D60); }
	bool CellClass_484F10(InfantryClass* pInf) const { JMP_THIS(0x484F10); }

protected:
	//Constructor
	CellClass() noexcept
		: CellClass(noinit_t())
	{ JMP_THIS(0x47BBF0); }

	explicit __forceinline CellClass(noinit_t) noexcept
		: AbstractClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	CellStruct MapCoords;	//Where on the map does this Cell lie?
	DynamicVectorClass<FoggedObjectClass*>* FoggedObjects;
	CellClass*         BridgeOwnerCell;
	DWORD              unknown_30;
	LightConvertClass* LightConvert; //TileDrawer
	int                IsoTileTypeIndex;	//What tile is this Cell?
	TagClass*          AttachedTag;			// The cell tag
	BuildingTypeClass* Rubble;				// The building type that provides the rubble image
	int                OverlayTypeIndex;	//What Overlay lies on this Cell?
	int                SmudgeTypeIndex;	//What Smudge lies on this Cell?

	DWORD              Passability;
	int                WallOwnerIndex; // Which House owns the wall placed in this Cell?
	                                              // Determined by finding the nearest BuildingType and taking its owner
	int                InfantryOwnerIndex;
	int                AltInfantryOwnerIndex;
	DWORD              __lastVisiblityUpdate; //5C
	DWORD              unknown_60;
	DWORD              RedrawFrame;
	RectangleStruct    InViewportRect;
	DWORD              CloakedByHouses;	//Is this cell in a cloak generator's radius? One bit per House.

	// Is this cell in range of some SensorsSight= equipment? One Word(!) per House, ++ and -- per unit.
	PROTECTED_PROPERTY(unsigned short , SensorsOfHouses[0x18]); // ! 24 houses instead of 32 like cloakgen
	// use Sensors_ funcs above

	// Is this cell in range of some DetectDisguise= equipment? One Word(!) per House, ++ and -- per unit.
	PROTECTED_PROPERTY(unsigned short, DisguiseSensorsOfHouses[0x18]);// ! 24 houses instead of 32 like cloakgen
	// use DisguiseSensors_ funcs above

	DWORD              BaseSpacerOfHouses; // & (1 << HouseX->ArrayIndex) == base spacing dummy for HouseX
	FootClass*         Jumpjet; // a jumpjet occupying this cell atm

	ObjectClass*       FirstObject;	//The first Object on this Cell. NextObject functions as a linked list.
	ObjectClass*       AltObject;

	LandType           LandType;	//What type of floor is this Cell?
	double             RadLevel;	//The level of radiation on this Cell.
	RadSiteClass*      RadSite;	//A pointer to the responsible RadSite.
	PixelFXClass*	   PixelFX;
	int                OccupyHeightsCoveringMe;
	DWORD              Intensity;
	WORD               Ambient;
	WORD			   Intensity_Normal;
	WORD               Intensity_Terrain;
	WORD               Color1_Blue;
	WORD               Color2_Red;
	WORD               Color2_Green;
	WORD               Color2_Blue;
	signed short	   TubeIndex; // !@#% Westwood braindamage, can't use > 127! (movsx eax, al)

	char               RedrawCountMAYBE; //unknown_118
	char               IsIceGrowthAllowed; //unknown_119
	char               Height;
	char               Level;

	BYTE               SlopeIndex;  // this + 2 == cell's slope shape as reflected by PLACE.SHP
	BYTE               unknown_11D;

	unsigned char      OverlayData;	//Powerup ,OverlayData The crate type on this cell. Also indicates some other weird properties ,OverlayData

	BYTE               SmudgeData;
	char               Visibility; //Shroudedness trust me, you don't wanna know... if you do, see 0x7F4194 and cry
	char               Foggedness; // same value as above: -2: Occluded completely, -1: Visible, 0...48: frame in fog.shp or shroud.shp
	BYTE               BlockedNeighbours; // number of somehow occupied cells next to this
	PROTECTED_PROPERTY(BYTE, align_123);

	// SubOccupations - 0x1 Center 0x2 Top(Abandoned) 0x4 Right 0x8 Left 0x10 Down / Terrains
	// 0x20 Units 0x40 Aircrafts 0x80 Buildings
	DWORD              OccupationFlags;
	DWORD              AltOccupationFlags;

	AltCellFlags	   AltFlags;	// related to Flags below
	int                ShroudCounter;
	DWORD              GapsCoveringThisCell; // actual count of gapgens in this cell, no idea why they need a second layer
	bool               VisibilityChanged;
	PROTECTED_PROPERTY(BYTE,     align_139[0x3]);
	DWORD              unknown_13C;

	CellFlags          Flags;	//Various settings.
	PROTECTED_PROPERTY(DWORD,     padding_144);
};

static_assert(sizeof(CellClass) == 0x148, "Invalid size.");