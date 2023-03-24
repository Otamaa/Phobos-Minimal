#pragma once

#include <GeneralDefinitions.h>
#include <GeneralStructures.h>
#include <AbstractClass.h>
#include <ColorScheme.h>
#include <Helpers/CompileTime.h>
#include <RectangleStruct.h>

struct TacticalSelectableStruct
{
	TechnoClass* Techno { nullptr };
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

class DECLSPEC_UUID("CF56B38A-240D-11D2-817C-006008055BB5")
	NOVTABLE TacticalClass : public AbstractClass
{
public:

	static constexpr reference<TacticalClass*, 0x887324u> const Instance{};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x6DBCE0);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x6DBD20);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x6DBE00);

	//Destructor
	virtual ~TacticalClass() override JMP_THIS(0x6DC470);

	//AbstractClass
	virtual void PointerExpired(AbstractClass* pAbstract, bool removed) override JMP_THIS(0x6DA560);
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
	bool CoordsToClient(CoordStruct const& coords, Point2D* pOutClient) const
		{ JMP_THIS(0x6D2140); }

	bool CoordsToClient(CoordStruct const* coords, Point2D* pOutClient) const
		{ JMP_THIS(0x6D2140); }

    Point2D CoordsToView(CoordStruct const& coords)
    {
        Point2D Buffer;
        this->CoordsToClient(coords, &Buffer);
        return Buffer;
    }

	Point2D* CoordsToScreen(Point2D* pDest, CoordStruct* pSource)
		{ JMP_THIS(0x6D1F10); }

	Point2D CoordsToScreen(CoordStruct* nSource)
	{
		Point2D Buffer;
		this->CoordsToScreen(&Buffer, nSource);
		return Buffer;
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

	Point2D * AdjustForZShapeMove(Point2D* pDest, Point2D* pClient)
		{ JMP_THIS(0x6D1FE0); }

	// convert xyz height to xy height?
	static int __fastcall AdjustForZ(int Height)
		{ JMP_STD(0x6D20E0); }

	static void __fastcall PrintTimer(int arg1, ColorScheme* scheme, int interval, const wchar_t* string, LARGE_INTEGER* pBlinkTimer, bool* pBlinkState)
		{ JMP_STD(0x6D4B50); }

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

	static int DrawTimer(int index, ColorScheme *Scheme, int Time, wchar_t *Text, Point2D *someXY1, Point2D *someXY2)
		{ JMP_STD(0x64DB50); }

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

	RectangleStruct VisibleArea() const
	{ return RectangleStruct{ TacticalPos.X ,TacticalPos.Y , LastTacticalPos.X ,LastTacticalPos.Y }; }

	Point2D* ApplyOffsetPixel_(Point2D* pRet ,Point2D* pOffset)
	{ JMP_THIS(0x6D2070); }

	Point2D ApplyOffsetPixel(Point2D Input)
	{
		Point2D nBuffer;
		ApplyOffsetPixel_(&nBuffer, &Input);
		return nBuffer;
	}

	void Coordoordmap_math_6D62E0(int v16, int v37, int* x2, int* a4) const {
		JMP_THIS(0x6D62E0);
	}

	int GetRamp(CoordStruct* pCoord) const { JMP_THIS(0x6D6AD0); }
	
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