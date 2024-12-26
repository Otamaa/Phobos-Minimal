#pragma once

#include <CRT.h>
#include <TechnoTypeClass.h>
class OverlayTypeClass;
class IsometricTileTypeClass;

struct BuildingAnimStruct
{
	char Anim[0x10];
	char Damaged[0x10];
	char Garrisoned[0x10];
	Point2D Position;
	int ZAdjust;
	int YSort;
	bool Powered;
	bool PoweredLight;
	bool PoweredEffect;
	bool PoweredSpecial;
};

struct BuildingAnimFrameStruct
{
	DWORD dwUnknown;
	int FrameCount;
	int FrameDuration;
};

struct FoundationStruct
{
   CellStruct Datas[30u];
};
static_assert(sizeof(FoundationStruct) == 0x78);

class DECLSPEC_UUID("AE8B33DB-061C-11D2-ACA4-006008055BB5")
	NOVTABLE BuildingTypeClass : public TechnoTypeClass
{
public:
	static const AbstractType AbsID = AbstractType::BuildingType;
	static constexpr inline DWORD vtable = 0x7E4570;

	//Array
	static constexpr constant_ptr<DynamicVectorClass<BuildingTypeClass*>, 0xA83C68u> const Array {};

	IMPL_Find(BuildingTypeClass)

	static BuildingTypeClass* __fastcall FindOrAllocate(const char* pID) {
		JMP_STD(0x4653C0);
	}

	static int __fastcall FindIndexById(const char* pID) {
		JMP_STD(0x45E7B0);
	}

	static constexpr reference<int, 0x89DDB8u> const HeightInLeptons{};
	static constexpr reference<FoundationStruct, 0x89D368u, 22u> const FoundationOutlinesData {};
	static constexpr reference<FoundationStruct, 0x89C900u, 22u> const FoundationlinesData {};


	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override R0;

	//IPersistStream
	//Destructor
	virtual ~BuildingTypeClass() RX;

	//AbstractClass
	virtual AbstractType WhatAmI() const override RT(AbstractType);
	virtual int Size() const override R0;

	//AbstractTypeClass
	//ObjectTypeClass
	virtual bool SpawnAtMapCoords(CellStruct* pMapCoords, HouseClass* pOwner) override R0;
	virtual ObjectClass* CreateObject(HouseClass* pOwner) override R0;

	//TechnoTypeClass
	//BuildingTypeClass
	virtual SHPStruct* LoadBuildup() const JMP_THIS(0x465960);

	//non-virtual
	unsigned int GetDeployFacing_() const
		{ JMP_THIS(0x465D70); }

	bool FlushPlacement(CellStruct* pCell ,HouseClass* pWho) const
	{ JMP_THIS(0x45EE70); }

	short GetFoundationWidth() const
		{ JMP_THIS(0x45EC90); }

	short GetFoundationHeight(bool bIncludeBib) const
		{ JMP_THIS(0x45ECA0); }

	bool IsVehicle() const
		{ JMP_THIS(0x465D40); }

	bool IsUndeployable() const
		{ JMP_THIS(0x465D40); }

	bool CanPlaceHere(CellStruct* cell, HouseClass* owner) const
		{ JMP_THIS(0x464AC0); }

	//
	void UnInitUnderRoofDoorAnim() const
		{ JMP_THIS(0x465C70); }

	void UnInitRoofDeployingAnim() const
		{ JMP_THIS(0x465C30); }

	void UnInitRubble() const
		{ JMP_THIS(0x465BF0); }

	void UnInitUnderDoorAnim() const
		{ JMP_THIS(0x465BB0); }

	void UnInitDeployAnim() const
		{ JMP_THIS(0x465B70); }

	void UnInitBibShape() const
		{ JMP_THIS(0x465B30); }

	void ClearBuildUp() const
		{ JMP_THIS(0x465AF0); }
	//

	//
	void InitBuildingAnim() const
		{ JMP_THIS(0x45E8B0); }

	void LoadShapeData() const
		{ JMP_THIS(0x45F230); }

	void LoadVoxelData() const
		{ JMP_THIS(0x45FA90); }
	//

	bool CanLeaveRubble(SHPStruct** pImage , int* pFrames) const
		{ JMP_THIS(0x45F160); }

	bool CanLeaveRubble_(RectangleStruct* pRect) const
		{ JMP_THIS(0x45F1D0); }

	// helpers
	bool __forceinline HasSuperWeapon(int index) const {
		return (this->SuperWeapon == index || this->SuperWeapon2 == index);
	}

	bool __forceinline HasSuperWeapon() const {
		return (this->SuperWeapon != -1 || this->SuperWeapon2 != -1);
	}

	bool __forceinline CanTogglePower() const {
		return this->TogglePower && (this->PowerDrain > 0 || this->Powered);
	}

	__forceinline BuildingAnimStruct& GetBuildingAnim(BuildingAnimSlot slot) {
		return this->BuildingAnim[static_cast<int>(slot)];
	}

	__forceinline const BuildingAnimStruct& GetBuildingAnim(BuildingAnimSlot slot) const {
		return this->BuildingAnim[static_cast<int>(slot)];
	}

	CoordStruct GetFixUpCoords() const
	{
		CoordStruct Buffer;
		auto Coo = this->GetCoords();
		this->vt_entry_6C(&Buffer, &Coo);
		return Buffer;
	}

	//Empty :kekw:
	static void __fastcall InitOneTimeData() {
		JMP_STD(0x45E750);
	}

	//Constructor
	BuildingTypeClass(const char* pID) noexcept
		: BuildingTypeClass(noinit_t())
	{ JMP_THIS(0x45DD90); }

	BuildingTypeClass(IStream* pStm) noexcept
		: BuildingTypeClass(noinit_t())
	{ JMP_THIS(0x45E520); }

protected:
	explicit __forceinline BuildingTypeClass(noinit_t) noexcept
		: TechnoTypeClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	int ArrayIndex;
	CellStruct* FoundationData;
	SHPStruct* Buildup;
	bool BuildupLoaded;
	BuildCat BuildCat;
	CoordStruct HalfDamageSmokeLocation1;
	CoordStruct HalfDamageSmokeLocation2;
	DWORD align_E24;  //unused , can be used to store ExtData
	double GateCloseDelay;
	int LightVisibility;
	int LightIntensity;
	int LightRedTint;
	int LightGreenTint;
	int LightBlueTint;
	Point2D PrimaryFirePixelOffset;
	Point2D SecondaryFirePixelOffset;
	OverlayTypeClass* ToOverlay;
	IsometricTileTypeClass* ToTile;
	char BuildupFile [0x10];
	int BuildupSound;
	int PackupSound;
	int CreateUnitSound;
	int UnitEnterSound;
	int UnitExitSound;
	int WorkingSound;
	int NotWorkingSound;
	char PowersUpBuilding [0x18];
	UnitTypeClass* FreeUnit;
	InfantryTypeClass* SecretInfantry;
	UnitTypeClass* SecretUnit;
	BuildingTypeClass* SecretBuilding;
	int field_EB0;
	int Adjacent;
	AbstractType Factory;
	CoordStruct TargetCoordOffset;
	CoordStruct ExitCoord;
	CellStruct* FoundationOutside;
	int StartFacing; //ED8
	int DeployFacing;
	int PowerBonus;
	int PowerDrain;
	int ExtraPowerBonus;
	int ExtraPowerDrain;
	Foundation Foundation;
	int Height;
	int OccupyHeight;
	int MidPoint;
	int DoorStages;

	BuildingAnimFrameStruct BuildingAnimFrame[6];

	BuildingAnimStruct BuildingAnim[0x15];

	int Upgrades;
	SHPStruct* DeployingAnim;
	bool DeployingAnimLoaded;
	SHPStruct* UnderDoorAnim;
	bool UnderDoorAnimLoaded;
	SHPStruct* Rubble;
	bool RubbleLoaded;
	SHPStruct* RoofDeployingAnim;
	bool RoofDeployingAnimLoaded;
	SHPStruct* UnderRoofDoorAnim;
	bool UnderRoofDoorAnimLoaded;
	SHPStruct* DoorAnim;
	SHPStruct* SpecialZOverlay;
	int SpecialZOverlayZAdjust;
	SHPStruct* BibShape;
	bool BibShapeLoaded;
	int NormalZAdjust;
	int AntiAirValue;
	int AntiArmorValue;
	int AntiInfantryValue;
	Point2D ZShapePointMove;
	int unknown_1538;
	int unknown_153C;
	int unknown_1540;
	int unknown_1544;
	WORD ExtraLight;
	bool TogglePower;
	bool HasSpotlight;
	bool IsTemple;
	bool IsPlug;
	bool HoverPad;
	bool BaseNormal;
	bool EligibileForAllyBuilding;
	bool EligibleForDelayKill;
	bool NeedsEngineer;
	int CaptureEvaEvent;
	int ProduceCashStartup;
	int ProduceCashAmount;
	int ProduceCashDelay;
	int InfantryGainSelfHeal;
	int UnitsGainSelfHeal;
	int RefinerySmokeFrames;
	bool Bib;
	bool Wall;
	bool Capturable;
	bool Powered;
	bool PoweredSpecial;
	bool Overpowerable;
	bool Spyable;
	bool CanC4;
	bool WantsExtraSpace;
	bool Unsellable;
	bool ClickRepairable;
	bool CanBeOccupied;
	bool CanOccupyFire;
	int MaxNumberOccupants;
	bool ShowOccupantPips;
	Point2D MuzzleFlash[0xA];

	Point2D DamageFireOffset[8];

	Point2D QueueingCell;
	int NumberImpassableRows;

	Point2D AddOccupy[8];
	Point2D RemoveOccupy[8];

	bool Radar;
	bool SpySat;
	bool ChargeAnim;
	bool IsAnimDelayedFire;
	bool SiloDamage;
	bool UnitRepair;
	bool UnitReload;
	bool Bunker;
	bool Cloning;
	bool Grinding;
	bool UnitAbsorb;
	bool InfantryAbsorb;
	bool SecretLab;
	bool DoubleThick;
	bool Flat;
	bool DockUnload;
	bool Recoilless;
	bool HasStupidGuardMode;
	bool BridgeRepairHut;
	bool Gate;
	bool SAM;
	bool ConstructionYard;
	bool NukeSilo;
	bool Refinery;
	bool Weeder;
	bool WeaponsFactory;
	bool LaserFencePost;
	bool LaserFence;
	bool FirestormWall;
	bool Hospital;
	bool Armory;
	bool EMPulseCannon;
	bool TickTank;
	bool TurretAnimIsVoxel;
	bool BarrelAnimIsVoxel;
	bool CloakGenerator;
	bool SensorArray;
	bool ICBMLauncher;
	bool Artillary;
	bool Helipad;
	bool OrePurifier;
	bool FactoryPlant;
	float InfantryCostBonus;
	float UnitsCostBonus;
	float AircraftCostBonus;
	float BuildingsCostBonus;
	float DefensesCostBonus;
	bool GDIBarracks;
	bool NODBarracks;
	bool YuriBarracks;
	float ChargedAnimTime;
	int DelayedFireDelay;
	int SuperWeapon;
	int SuperWeapon2;
	int GateStages;
	int PowersUpToLevel;
	bool DamagedDoor;
	bool InvisibleInGame;
	bool TerrainPalette;
	bool PlaceAnywhere;
	bool ExtraDamageStage;
	bool AIBuildThis;
	bool IsBaseDefense;
	BYTE CloakRadiusInCells;
	bool ConcentricRadialIndicator;
	int PsychicDetectionRadius;
	int BarrelStartPitch;
	char VoxelBarrelFile[16];
	DWORD field_1724;
	double double_1728;
	CoordStruct VoxelBarrelOffsetToPitchPivotPoint;
	CoordStruct VoxelBarrelOffsetToRotatePivotPoint;
	CoordStruct VoxelBarrelOffsetToBuildingPivotPoint;
	CoordStruct VoxelBarrelOffsetToBarrelEnd;
	bool DemandLoad;
	bool DemandLoadBuildup;
	bool FreeBuildup;
	bool IsThreatRatingNode;
	bool PrimaryFireDualOffset;
	bool ProtectWithWall;
	bool CanHideThings;
	bool CrateBeneath;
	bool LeaveRubble;
	bool CrateBeneathIsMoney;
	char TheaterSpecificID [0x13];
	int NumberOfDocks;
	VectorClass<CoordStruct> DockingOffsets;
	PRIVATE_PROPERTY(DWORD, align_1794);

};
static_assert(sizeof(BuildingTypeClass) == 0x1798, "Invalid Size !");