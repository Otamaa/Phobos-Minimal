#pragma once

#include <GeneralDefinitions.h>
#include <GeneralStructures.h>
#include <AbstractClass.h>
#include <ColorScheme.h>
#include <Helpers/CompileTime.h>
#include <RectangleStruct.h>
#include <Unsorted.h>

class ObjectClass;
struct TacticalSelectableStruct
{
	ObjectClass* Techno { nullptr };
	Point2D Point { };

	bool operator==(TacticalSelectableStruct const& rhs) const {
		return (Point == rhs.Point) && Techno == rhs.Techno;
	}

	bool operator!=(TacticalSelectableStruct const& rhs) const
	{ return !((*this) == rhs); }
};
static_assert(sizeof(TacticalSelectableStruct) == 0xC, "Invalid Size !");

class DSurface;
class CellClass;
class BuildingClass;
class DECLSPEC_UUID("CF56B38A-240D-11D2-817C-006008055BB5")
	NOVTABLE TacticalClass : public AbstractClass
{
public:

	static COMPILETIMEEVAL reference<TacticalClass*, 0x887324u> const Instance{};
	static COMPILETIMEEVAL reference<BuildingClass*, 0x88098Cu> const DisplayPendingObject{};
	static	COMPILETIMEEVAL reference<RectangleStruct, 0xB0CE28u> const view_bound { };

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x6DBCE0);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x6DBD20);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x6DBE00);

	//Destructor
	virtual ~TacticalClass() override JMP_THIS(0x6DC470);

	//AbstractClass
	virtual void PointerExpired(AbstractClass* pAbstract, bool bremoved) override JMP_THIS(0x6DA560);
	virtual AbstractType WhatAmI() const override { return AbstractType::TacticalMap; }
	virtual int Size() const override { return 0xE18; }
	virtual void Update() override JMP_THIS(0x6D2540);

	//TacticalClass
	virtual bool Tactical_draw_line_6DBB60(RectangleStruct* a2, RectangleStruct* a3, int color, bool bool1) R0;

	void SetTacticalPosition(CoordStruct* pCoord)
		{ JMP_THIS(0x6D6070); }

	CellStruct* CoordsToCell(CellStruct* pDest, CoordStruct* pSource)
		{ JMP_THIS(0x6D6590); }

	// returns whether coords are visible at the moment
	COMPILETIMEEVAL bool IsCoordsToClientVisible(CoordStruct const& coords) const
	{
		Point2D point = CoordsToScreen(coords) - this->TacticalPos;
		return point.X >= -360 && point.X <= view_bound->Width + 360
			&& point.Y >= -180 && point.Y <= view_bound->Height + 180;
	}

	COMPILETIMEEVAL bool IsCoordsToClientVisible(CoordStruct const* coords) const
	{
		Point2D point = CoordsToScreen(*coords) - this->TacticalPos;
		return point.X >= -360 && point.X <= view_bound->Width + 360
			&& point.Y >= -180 && point.Y <= view_bound->Height + 180;
	}

   COMPILETIMEEVAL Point2D CoordsToView(CoordStruct const& coords) {
        return this->CoordsToClient(coords);
    }

	COMPILETIMEEVAL std::pair<Point2D, bool> GetCoordsToClientSituation(const CoordStruct& coords) const {
		Point2D point = CoordsToScreen(coords) - this->TacticalPos;
		const bool visible = point.X >= -360 && point.X <= view_bound->Width + 360
			&& point.Y >= -180 && point.Y <= view_bound->Height + 180;
		return { point, visible };
	}

	COMPILETIMEEVAL Point2D CoordsToClient(const CoordStruct& coords) const {
		return CoordsToScreen(coords) - this->TacticalPos;
	}

	//Point2D* CoordsToScreen(Point2D* pDest, CoordStruct* pSource)
	//	{ JMP_THIS(0x6D1F10); }

	//Point2D CoordsToScreen(CoordStruct* nSource)
	//{
	//	Point2D Buffer;
	//	this->CoordsToScreen(&Buffer, nSource);
	//	return Buffer;
	//}

	COMPILETIMEEVAL static Point2D CoordsToScreen(const CoordStruct& coord)
	{
		auto [x, y] = AdjustForZShapeMove(coord.X, coord.Y);
		return { x, y - Game::AdjustHeight(coord.Z) };
	}

	COMPILETIMEEVAL static Point2D CoordsToScreen(const CoordStruct* pPoord)
	{
		auto [x, y] = AdjustForZShapeMove(pPoord->X, pPoord->Y);
		return { x, y - Game::AdjustHeight(pPoord->Z) };

	}

	CoordStruct* ClientToCoords(CoordStruct* pOutBuffer, Point2D const& client) const
		{ JMP_THIS(0x6D2280); }

	CoordStruct ClientToCoords(Point2D const& client) const {
		CoordStruct buffer;
		this->ClientToCoords(&buffer, client);
		return buffer;
	}

	char GetOcclusion(const CellStruct& cell, bool fog) const
		{ JMP_THIS(0x6D8700); }

	//Point2D * AdjustForZShapeMove(Point2D* pDest, Point2D* pClient)
	//	{ JMP_THIS(0x6D1FE0); }

	COMPILETIMEEVAL static Point2D AdjustForZShapeMove(int x, int y)
	{
		return {
			(-Unsorted::CellWidthInPixels * y / 2 + Unsorted::CellWidthInPixels * x / 2) / Unsorted::LeptonsPerCell,
			(Unsorted::CellHeightInPixels * y / 2 + Unsorted::CellHeightInPixels * x / 2) / Unsorted::LeptonsPerCell
		};
	}

	//static void __fastcall PrintTimer(int arg1, ColorScheme* scheme, int interval, const wchar_t* string, LARGE_INTEGER* pBlinkTimer, bool* pBlinkState)
	//	{ JMP_STD(0x6D4B50); }

	void FocusOn(CoordStruct* pDest, int Velocity)
		{ JMP_THIS(0x6D2420); }

	void AddSelectable(TechnoClass* pTechno, int x, int y)
	{ JMP_THIS(0x6D9EF0) };

	// called when area needs to be marked for redrawing due to external factors
	// - alpha lights, terrain changes like cliff destruction, etc
	void RegisterDirtyArea(RectangleStruct Area, bool bUnk)
		{ JMP_THIS(0x6D2790); }

	void RegisterCellAsVisible(CellClass* pCell)
		{ JMP_THIS(0x6DA7D0) };

	//static int __fastcall DrawTimer(int index, ColorScheme *Scheme, int Time, wchar_t *Text, Point2D *someXY1, Point2D *someXY2)
	//	{ JMP_STD(0x6D4B50); }

	/*
	*   TacticalRenderMode_0_ALL = 0x0,
	*	TacticalRenderMode_TERRAIN = 0x1,
	*	TacticalRenderMode_MOVING_ANIMATING = 0x2,
	*	TacticalRenderMode_3_ALL = 0x3,
	*	TacticalRenderMode_STOPDRAWING = 0x4,
	*	TacticalRenderMode_5 = 0x5,
	*/
	void Render(DSurface* pSurface, bool flag, int eMode)
		{ JMP_THIS(0x6D3D10); }

	CellStruct* Coordmap_viewportpos_tocellpos_Click_Cell_Calc(CellStruct& retstr, Point2D& a3)
		{ JMP_THIS(0x6D6590); }

	void DrawLaserfencePlacement(bool bBlit,CellStruct nLoc)
		{ JMP_THIS(0x6D5730); }

	void DrawFirewallPlacement(bool bBlit, CellStruct nLoc)
		{ JMP_THIS(0x6D59D0); }

	void DrawWallPlacement(bool bBlit, CellStruct nLoc)
		{ JMP_THIS(0x6D5C50); }

	COMPILETIMEEVAL RectangleStruct VisibleArea() const
	{ return { TacticalPos.X ,TacticalPos.Y , LastTacticalPos.X ,LastTacticalPos.Y }; }

	Point2D* ApplyOffsetPixel_(Point2D* pRet ,Point2D* pOffset)
	{ JMP_THIS(0x6D2070); }

	Point2D ApplyOffsetPixel(Point2D Input)
	{
		Point2D nBuffer;
		ApplyOffsetPixel_(&nBuffer, &Input);
		return nBuffer;
	}

	Point2D* ApplyMatrix_Pixel(Point2D *coords, Point2D *offset)
		{ JMP_THIS(0x6D2070); }

	Point2D ApplyMatrix_Pixel(Point2D Input)
	{
		Point2D nBuffer;
		ApplyMatrix_Pixel(&nBuffer, &Input);
		return nBuffer;
	}

	Point2D ApplyMatrix_Pixel_inl(const Point2D& offset) {
		auto&& temp = this->IsoTransformMatrix * Vector3D<float>{(float)offset.X, (float)offset.Y, 0};
		return { (int)temp.X,(int)temp.Y };
	}

	void Coordoordmap_math_6D62E0(int v16, int v37, int* x2, int* a4) const {
		JMP_THIS(0x6D62E0);
	}

	int GetRamp(CoordStruct* pCoord) const { JMP_THIS(0x6D6AD0); }

	void DrawAllTacticalText(wchar_t* text) const {
		JMP_THIS(0x6D4E20);
	}

	TacticalClass() noexcept
		: TacticalClass(noinit_t())
	{ JMP_THIS(0x6D1C20); }

protected:

	explicit __forceinline TacticalClass(noinit_t) noexcept
		: AbstractClass(noinit_t())
	{ }

public:

	wchar_t ScreenText[64];
	int EndGameGraphicsFrame;
	int LastAIFrame;
	bool field_AC;
	bool field_AD;
	PROTECTED_PROPERTY(char, gap_AE[2]);
	Point2D TacticalPos;
	Point2D LastTacticalPos;
	double ZoomInFactor;
	Point2D Point_C8;
	Point2D Point_D0;
	float field_D8;
	float field_DC;
	int VisibleCellCount;
	CellClass * VisibleCells [800];
	Point2D TacticalCoord1;
	DWORD field_D6C;
	DWORD field_D70;
	Point2D TacticalCoord2;
	bool field_D7C;
	bool Redrawing; // set while redrawing - cheap mutex // TacticalPosUpdated
	PROTECTED_PROPERTY(char, gap_D7E[2]);
	RectangleStruct ContainingMapCoords;
	LTRBStruct Band;
	DWORD MouseFrameIndex;
	CDTimerClass StartTime;
	int SelectableCount;
	Matrix3D Unused_Matrix3D; //DB4
	Matrix3D IsoTransformMatrix; //DE4
	DWORD field_E14;

};

static_assert(sizeof(TacticalClass) == 0xE18, "Invalid Size !");