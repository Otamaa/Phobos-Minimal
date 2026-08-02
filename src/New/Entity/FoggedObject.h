#pragma once
#include <Utilities/VectorHelper.h>

#include <FacingClass.h>
#include <RectangleStruct.h>
#include <CoordStruct.h>
#include <CellStruct.h>
#include <RecoilData.h>

class AnimTypeClass;
class BuildingClass;
class BuildingTypeClass;
class CellClass;
class TerrainClass;
class TerrainTypeClass;
struct IStream;
class HouseClass;
class FoggedObject
{
public:
	static HelperedVector<FoggedObject*> FoggedObjects;

	static HRESULT SaveGlobal(IStream* pStm);
	static HRESULT LoadGlobal(IStream* pStm);

	static void Clear();

	explicit FoggedObject() noexcept;
	explicit FoggedObject(BuildingClass* pBld, bool IsVisible) noexcept;
	explicit FoggedObject(TerrainClass* pTerrain) noexcept;
	explicit FoggedObject(CellClass* pCell, bool IsOverlay) noexcept;

	HRESULT Load(IStream* pStm);
	HRESULT Save(IStream* pStm);

	virtual ~FoggedObject();

	void Render(const RectangleStruct& viewRect) const;

	static void RenderAll(const RectangleStruct& viewRect);
	static void SortForRender();

	static RectangleStruct Union(const RectangleStruct& rect1, const RectangleStruct& rect2);

	inline int GetIndexID() const;
protected:
	void RenderAsBuilding(const RectangleStruct& viewRect) const;
	void RenderAsSmudge(const RectangleStruct& viewRect) const;
	void RenderAsOverlay(const RectangleStruct& viewRect) const;
	void RenderAsTerrain(const RectangleStruct& viewRect) const;

	static char BuildingVXLDrawer[1824];
	static bool BuildingVXLDrawerReady;
public:
	enum class CoveredType : char
	{
		Building = 1,
		Terrain,
		Smudge,
		Overlay
	};

	CoordStruct Location {};
	CoveredType CoveredType {};
	RectangleStruct Bound {};
	bool Visible { true };

	union
	{
		struct
		{
			int Overlay;
			unsigned char OverlayData;
		} OverlayData;

		struct
		{
			TerrainTypeClass* Type;
			int Frame;
			bool Flat;
		} TerrainData;

		struct
		{
			HouseClass* Owner;
			BuildingTypeClass* Type;

			CellStruct BaseCoords;

			int ShapeFrame;
			FacingClass PrimaryFacing;
			FacingClass SecondaryFacing;

			FacingClass BarrelFacing;
			RecoilData TurretRecoil;
			RecoilData BarrelRecoil;
			bool IsFirestormWall;
			int TurretAnimFrame;
			struct
			{
				AnimTypeClass* AnimType;
				int AnimFrame;
				int ZAdjust;
			} Anims[21];
		} BuildingData;

		struct
		{
			int Smudge;
			int SmudgeData;
			int Height;
		} SmudgeData;

		BYTE _All[sizeof(BuildingData)] {};
	};
};