#pragma once

#include <MapClass.h>

class CCINIClass;
class ObjectTypeClass;

class NOVTABLE DisplayClass : public MapClass
{
public:
	//Static
	static constexpr constant_ptr<DisplayClass, 0x87F7E8u> const Instance{};
	static constexpr constant_ptr<DisplayClass, 0x87F7E8u> const Global{};
	
	//WIP: DisplayClass::TacticalClass goes HERE

	bool ProcessClickCoords(Point2D *src, CellStruct *XYdst, CoordStruct *XYZdst, ObjectClass **Target, BYTE *a5, BYTE *a6)
		{ JMP_THIS(0x692300); }

	static LayerClass* GetLayer(Layer lyr)
	{
		if(lyr >= Layer::Underground && lyr <= Layer::Top)
			return reinterpret_cast<LayerClass**>(0x8A0360)[static_cast<int>(lyr)];
		else
			return nullptr;
	}

	// the foundation for placement with green/red
	void  SetActiveFoundation(CellStruct *Coords)
		{ JMP_THIS(0x4A8BF0); }

	//Destructor
	virtual ~DisplayClass() RX;

	//GScreenClass
	//MapClass
	//DisplayClass
	virtual HRESULT Load(IStream* pStm) RX;
	virtual HRESULT Save(IStream* pStm) RX;
	virtual void LoadFromINI(CCINIClass* pINI) RX; //Loads the map from a map file.
	virtual const wchar_t* GetToolTip(UINT nDlgID) R0;
	virtual void CloseWindow() RX; //prolly wrong naming
	virtual void vt_entry_8C() RX;
	virtual bool MapCell(CellStruct* pMapCoord, HouseClass* pHouse) R0; //vt_entry_90
	virtual bool RevealFogShroud(CellStruct* pMapCoord, HouseClass* pHouse, bool bIncreaseShroudCounter) R0; //vt_entry_94
	virtual bool MapCellFoggedness(CellStruct* pMapCoord, HouseClass* pHouse) R0; //vt_entry_98
	virtual bool MapCellVisibility(CellStruct* pMapCoord, HouseClass* pHouse) R0; //vt_entry_9C
	virtual MouseCursorType GetLastMouseCursor() = 0;
	virtual bool ScrollMap(DWORD dwUnk1, DWORD dwUnk2, DWORD dwUnk3) R0;
	virtual void Set_View_Dimensions(const RectangleStruct& rect) RX;
	virtual void vt_entry_AC(DWORD dwUnk) RX;
	virtual void vt_entry_B0(DWORD dwUnk) RX;
	virtual void vt_entry_B4(Point2D* pPoint) RX;

	//Decides which mouse pointer to set and then does it.
	//Mouse is over cell pMapCoords which is bShrouded and holds pObject.
	virtual bool ConvertAction(const CellStruct& cell, bool bShrouded, ObjectClass* pObject, Action action, bool dwUnk) RX;
	virtual void LeftMouseButtonDown(const Point2D& point) RX;
	virtual void LeftMouseButtonUp(const CoordStruct& coords, const CellStruct& cell, ObjectClass* pObject, Action action, DWORD dwUnk2) RX;
	virtual void RightMouseButtonUp(DWORD dwUnk) RX;

	//Non-virtual

	Action DecideAction(const CellStruct& cell, ObjectClass* pObject, DWORD dwUnk)
		{ JMP_THIS(0x692610); }

	/* pass in CurrentFoundationData and receive the width/height of a bounding rectangle in cells */
	CellStruct* FoundationBoundsSize(CellStruct& outBuffer, CellStruct const* const pFoundationData) const
		{ JMP_THIS(0x4A94F0); }

	CellStruct FoundationBoundsSize(CellStruct const* const pFoundationData) const {
		CellStruct outBuffer;
		FoundationBoundsSize(outBuffer, pFoundationData);
		return outBuffer;
	}

	void RemoveObject(ObjectClass* pObject) const 
	   { JMP_THIS(0x4A9770); }

	void SubmitObject(ObjectClass* pObject) const 
	   { JMP_THIS(0x4A9720); }

	/* marks or unmarks the cells pointed to by CurrentFoundationData as containing a building */
	void MarkFoundation(CellStruct * BaseCell, bool Mark)
		{ JMP_THIS(0x4A95A0); }

	void SetCursorShape2(CellStruct* pWhere) const
		{ JMP_THIS(0x4A8D50); }

	void SetCursorPos(CellStruct* pRet , CellStruct* pIn ) const
		{ JMP_THIS(0x4A91B0); }

protected:
	//Constructor
	DisplayClass() {}	//don't need this

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:
	CellStruct CurrentFoundation_CenterCell;	//Currently placing the building here
	CellStruct CurrentFoundation_TopLeftOffset;		// offset from center cell of the current foundation (under the mouse) to the top left cell
	CellStruct* CurrentFoundation_Data;	//Foundation data of the building we're currently placing (note: limited to 120 cells)
	bool unknown_1180;
	bool unknown_1181;
	CellStruct CurrentFoundationCopy_CenterCell; // this is a copy of the CurrentFoundation data above..
	CellStruct CurrentFoundationCopy_TopLeftOffset;
	CellStruct * CurrentFoundationCopy_Data; // (note: limited to 50 [!] cells)
	DWORD unknown_1190;
	DWORD unknown_1194;
	DWORD unknown_1198;
	bool FollowObject;
	ObjectClass* ObjectToFollow;
	ObjectClass* CurrentBuilding;		//Building we're currently placing
	ObjectTypeClass* CurrentBuildingType;	//Type of that building
	DWORD unknown_11AC;
	bool RepairMode;
	bool SellMode;
	bool PowerToggleMode;
	bool PlanningMode;
	bool PlaceBeaconMode;
	int CurrentSWTypeIndex;	//Index of the SuperWeaponType we have currently selected
	DWORD unknown_11BC;
	DWORD unknown_11C0;
	DWORD unknown_11C4;
	DWORD unknown_11C8;
	bool unknown_bool_11CC;
	bool unknown_bool_11CD;
	bool unknown_bool_11CE;
	bool DraggingRectangle; //11CF IsRubberBand
	bool IsTentative; //11D0
	bool IsShadowPresen; //11D1
	DWORD unknown_11D4;
	DWORD unknown_11D8;
	DWORD unknown_11DC;
	DWORD unknown_11E0;
	PROTECTED_PROPERTY(DWORD, padding_11E4);
};
