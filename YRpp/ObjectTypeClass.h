/*
	ObjectTypes are initialized by INI files.
*/

#pragma once

#include <AbstractTypeClass.h>
#include <FileSystem.h>
#include <VoxelIndex.h>
#include <IndexClass.h>

//forward declarations
class TechnoTypeClass;
class HouseTypeClass;
class ObjectClass;
class BuildingClass;

//#pragma pack(push, 4)
class NOVTABLE ObjectTypeClass : public AbstractTypeClass
{
public:

	static const AbstractBaseType AbsTypeBase = AbstractBaseType::ObjectType;
	static constexpr constant_ptr<DynamicVectorClass<ObjectTypeClass*>, 0xAC1418u> const Array {};

	IMPL_Find(ObjectTypeClass)
	IMPL_FindByName(ObjectTypeClass)
	IMPL_FindOrAllocate(ObjectTypeClass)

	static int __fastcall FindIndexById(const char* pID) {
		JMP_STD(0x5F9990);
	}

	//static
	//static void LoadPips() JMP_STD(0x5F76B0);
	//static void sub_5F77F0() JMP_STD(0x5F77F0);
	//static int ReleaseAllVoxelCaches() JMP_STD(0x5F99E0);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x5F9720);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x5F9950);
	virtual HRESULT __stdcall GetSizeMax(ULARGE_INTEGER* pcbSize) override JMP_STD(0x5F9970);

	//Destructor
	virtual ~ObjectTypeClass() JMP_THIS(0x5F9AE0);

	//AbstractTypeClass
	virtual bool LoadFromINI(CCINIClass* pINI) override JMP_THIS(0x5F92D0);

	//ObjectTypeClass
	virtual CoordStruct* vt_entry_6C(CoordStruct* pDest, CoordStruct* pSrc) const R0; //Coord_Fixup
	virtual DWORD GetOwners() const R0;
	virtual int GetPipMax() const R0;
	virtual void PixelDimensions(Point3D* pDest) const RX;  //pixel dimension , 0x78
	virtual void Dimension2(CoordStruct* pDest) RX; //was LeptonDimensions
	virtual bool SpawnAtMapCoords(CellStruct* pMapCoords, HouseClass* pOwner) R0;
	virtual int GetActualCost(HouseClass* pHouse) const R0;
	virtual int GetBuildSpeed() const R0;
	virtual ObjectClass* CreateObject(HouseClass* pOwner) R0;
	virtual CellStruct * GetFoundationData(bool IncludeBib) const R0;
	virtual BuildingClass* FindFactory(bool allowOccupied, bool requirePower, bool requireCanBuild, HouseClass const* pHouse) const R0;
	virtual SHPStruct* GetCameo() const R0;
	virtual SHPStruct* GetImage() const R0;

	static BuildCat __fastcall IsBuildCat5(AbstractType abstractID, int idx)
		{ JMP_STD(0x5004E0); }

	static TechnoTypeClass * __fastcall FetchTechnoType(AbstractType abstractID, int idx)
		{ JMP_STD(0x48DCD0); }

	void sub_5F8080()
		{ JMP_THIS(0x5F8080); }

	void LoadVoxel__() const
		{ JMP_THIS(0x5F8110); }

	bool LoadTurret(const char* pID, int TurretIndex)
		{ JMP_THIS(0x5F7A90); }

	bool LoadBARL(const char* pID, int BARLIndex)
	{ JMP_THIS(0x5F7DB0); }

	bool LoadVehicleImage() //Only UnitType
		{ JMP_THIS(0x5F8CE0); }

	//Constructor
	ObjectTypeClass(const char* pID) noexcept
		: ObjectTypeClass(noinit_t())
	{ JMP_THIS(0x5F7090); }

	ObjectTypeClass(IStream* pStm) noexcept
		: ObjectTypeClass(noinit_t())
	{ JMP_THIS(0x5F7320); }

protected:
	explicit __forceinline ObjectTypeClass(noinit_t) noexcept
		: AbstractTypeClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	ColorStruct RadialColor;
	BYTE          unused_9B;
	Armor         Armor;
	int           Strength;
	SHPStruct*    Image;
	bool          ImageAllocated;
	PROTECTED_PROPERTY(BYTE, align_A9[3]);
	SHPStruct*    AlphaImage;
	VoxelStruct MainVoxel;
	VoxelStruct TurretVoxel; //also used for WO voxels
	VoxelStruct BarrelVoxel;

	VoxelStruct ChargerTurrets [0x12];
	VoxelStruct ChargerBarrels [0x12];

	bool          NoSpawnAlt;
	PROTECTED_PROPERTY(BYTE, align_1E9[3]);
	int           MaxDimension;
	int           CrushSound; //index
	int           AmbientSound; //index

	char ImageFile [0x19];

	bool           AlternateArcticArt;
	bool           ArcticArtInUse; //not read from ini

	char AlphaImageFile [0x19];

	bool           Theater;
	bool           Crushable;
	bool           Bombable;
	bool           RadarInvisible;
	bool           Selectable;
	bool           LegalTarget;
	bool           Insignificant;
	bool           Immune;
	bool           IsLogic; // add objects to the logic vector IsSentient
	bool           AllowCellContent;
	bool           Voxel;
	bool           NewTheater;
	bool           HasRadialIndicator;
	bool           IgnoresFirestorm;
	bool           UseLineTrail;
	ColorStruct    LineTrailColor;
	PROTECTED_PROPERTY(BYTE, align_23E[2]);
	int            LineTrailColorDecrement;

	union
	{
		struct VoxelCaches
		{
			IndexClass<MainVoxelIndexKey, VoxelCacheStruct*> Main;
			IndexClass<TurretWeaponVoxelIndexKey, VoxelCacheStruct*> TurretWeapon;
			IndexClass<ShadowVoxelIndexKey, VoxelCacheStruct*> Shadow;
			IndexClass<TurretBarrelVoxelIndexKey, VoxelCacheStruct*> TurretBarrel;
		} VoxelCaches;

		IndexClass<int, int> VoxelCaches_[4];
	};
};
//#pragma pack(pop)

static_assert(sizeof(ObjectTypeClass) == 0x294);