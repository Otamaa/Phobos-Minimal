#pragma once

#include <AbstractClass.h>
#include <Helpers/CompileTime.h>
#include <RectangleStruct.h>

// Forward declarations
class BuildingClass;
class TerrainClass;
class ObjectTypeClass;
class BuildingTypeClass;
class TerrainTypeClass;
class AnimTypeClass;

class NOVTABLE FoggedObjectClass : public AbstractClass
{
public:
	static const AbstractType AbsID = AbstractType::FoggedObject;

	static constexpr constant_ptr<DynamicVectorClass<FoggedObjectClass*>, 0x8B3D10> Array {};

	struct DrawRecord
	{
		union
		{
			ObjectTypeClass* ObjectType;
			BuildingTypeClass* BuildingType;
			TerrainTypeClass* TerrainType;
			AnimTypeClass* AnimType;
		};
		int FrameIndex;
		bool IsFireStormWall;
		int zAdjust;

		bool operator==(const DrawRecord&) const { return true; };
	};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) R0;

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) R0;
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) R0;

	//Destructor
	virtual ~FoggedObjectClass() RX;

	//AbstractClass
	virtual AbstractType WhatAmI() const RT(AbstractType);
	virtual int	Size() const R0;

	virtual CellStruct* GetMapCoords(CellStruct* pUCell) const R0;

	CellStruct GetMapCoords() const
	{
		CellStruct ret;
		this->GetMapCoords(&ret);
		return ret;
	}

	//Constructor
	FoggedObjectClass();

	// for building
	FoggedObjectClass(BuildingClass* pBuilding, bool translucent);

	// for terrain
	FoggedObjectClass(TerrainClass* pTerrain);

	// for overlay
	FoggedObjectClass(const CoordStruct& location, int overlayIndex, int powerup);

	// for smudge
	FoggedObjectClass(const CoordStruct* location, int smudgeIndex, int frameIdx);

	// helper
	FoggedObjectClass& operator=(FoggedObjectClass&& foggedObject) noexcept
	{
		OverlayIndex = foggedObject.OverlayIndex;
		Owner = foggedObject.Owner;
		Powerup = foggedObject.Powerup;
		CoveredAbstractType = foggedObject.CoveredAbstractType;
		Location = foggedObject.Location;
		Bound = foggedObject.Bound;
		Level = foggedObject.Level;
		SmudgeIndex = foggedObject.SmudgeIndex;
		SmudgeFrameIndex = foggedObject.SmudgeFrameIndex;
		Translucent = foggedObject.Translucent;
		DrawRecords.Swap(foggedObject.DrawRecords);

		return *this;
	}

	FoggedObjectClass(const FoggedObjectClass& foggedObject) noexcept
	{
		*this = foggedObject;
	}
	FoggedObjectClass& operator=(const FoggedObjectClass& foggedObject) noexcept
	{
		OverlayIndex = foggedObject.OverlayIndex;
		Owner = foggedObject.Owner;
		Powerup = foggedObject.Powerup;
		CoveredAbstractType = foggedObject.CoveredAbstractType;
		Location = foggedObject.Location;
		Bound = foggedObject.Bound;
		Level = foggedObject.Level;
		SmudgeIndex = foggedObject.SmudgeIndex;
		SmudgeFrameIndex = foggedObject.SmudgeFrameIndex;
		Translucent = foggedObject.Translucent;
		DrawRecords = foggedObject.DrawRecords;
		return *this;
	}


	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================
public:
	int OverlayIndex;
	HouseClass* Owner;
	int Powerup;
	AbstractType CoveredAbstractType;
	CoordStruct Location;
	RectangleStruct Bound;
	int Level;
	int SmudgeIndex;
	int SmudgeFrameIndex;
	DynamicVectorClass<DrawRecord> DrawRecords;
	char Translucent;
	// never used in YR
	char field_75;
	char field_76;
	char field_77;

};