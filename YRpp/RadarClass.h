#pragma once

#include <DisplayClass.h>
#include <RectangleStruct.h>
#include <Audio.h>
#include <HashTable.h>

class Surface;
struct RadarTrackingStruct;
class NOVTABLE RadarClass : public DisplayClass
{
public:
	//WIP: RadarClass::RTacticalClass goes HERE

	//Static
	static COMPILETIMEEVAL constant_ptr<RadarClass, 0x87F7E8u> const Instance{};
	static COMPILETIMEEVAL constant_ptr<RadarClass, 0x87F7E8u> const Global{};

	//this only few , there is a lot of Radar stuff around .data
	//only add needed one atm
	static COMPILETIMEEVAL reference<float, 0x880C70u> const RadarSizeFactor_{};
	static COMPILETIMEEVAL reference<float, 0x880C74u> const RadarZoomFactor_{};
	static COMPILETIMEEVAL reference<int, 0x880C78u> const Radar_dword1490_x{};
	static COMPILETIMEEVAL reference<int, 0x880C7Cu> const Radar_dword1494_y{};
	static COMPILETIMEEVAL reference<int, 0x880C80u> const Radar_dword1498_z{};
	static COMPILETIMEEVAL reference<RectangleStruct, 0x880C84u> const Radar_Rect149C{};

	static COMPILETIMEEVAL reference <Surface*, 0x880A04u> const RadarEvenSurface{};
	static COMPILETIMEEVAL reference <Surface*, 0x880A08u> const RadarEvenSurface_B{};

	//Destructor
	virtual ~RadarClass() JMP_THIS(0x6587A0);

	//GScreenClass
	virtual void One_Time() override JMP_THIS(0x652CF0);
	virtual void Init_Clear() override JMP_THIS(0x652DE0);
	virtual void Init_IO() override JMP_THIS(0x653010);
	virtual void Update(const int& keyCode, const Point2D& mouseCoords) override JMP_THIS(0x653850);
	virtual void Draw(DWORD dwUnk) override JMP_THIS(0x653100);

	//MapClass
	virtual void CreateEmptyMap(const RectangleStruct& pMapRect, bool reuse, char nLevel, bool bUnk2) override
		{ JMP_THIS(0x653F50); }

	virtual void SetVisibleRect(const RectangleStruct& mapRect) override
		{ JMP_THIS(0x654490); }

	//DisplayClass
	virtual HRESULT Load(IStream* pStm) override JMP_THIS(0x6568A0);
	virtual HRESULT Save(IStream* pStm) override JMP_THIS(0x656AC0);
	virtual const wchar_t* GetToolTip(UINT nDlgID) override JMP_THIS(0x658770);
	virtual void CloseWindow() override JMP_THIS(0x654320); //prolly wrong naming
	virtual bool MapCell(CellStruct& cell, HouseClass* pHouse) override JMP_THIS(0x653810);
	virtual bool RevealFogShroud(CellStruct& cell, HouseClass* pHouse, bool bIncreaseShroudCounter) override JMP_THIS(0x653830);

	//RadarClass
	virtual void DisposeOfArt() JMP_THIS(0x652D90);
	virtual CellStruct* vt_entry_CC(CellStruct* out_pUnk, Point2D& pPoint) JMP_THIS(0x653760);
	virtual void vt_entry_D0(DWORD dwUnk) JMP_THIS(0x653F70);
	virtual void Init_For_House() JMP_THIS(0x652E90);

	void Push_Cell(CellStruct* a2)JMP_THIS(0x6551C0);

	void UpdateRadarStatus(bool status) { JMP_THIS(0x656DF0); }
	//Non-virtual
protected:
	//Constructor
	RadarClass() noexcept	//don't need this
	{ JMP_THIS(0x652960); }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:
	DWORD unknown_11E4;
	DWORD unknown_11E8;
	DWORD unknown_11EC;
	DWORD unknown_11F0;
	DWORD unknown_11F4;
	DWORD unknown_11F8;
	DWORD unknown_11FC;
	DWORD unknown_1200;
	DWORD unknown_1204;
	DWORD unknown_1208;
	RectangleStruct unknown_rect_120C;
	Surface* unknown_121C;
	Surface* unknown_1220;
	DynamicVectorClass<CellStruct> unknown_cells_1124;
	DWORD unknown_123C;
	DWORD unknown_1240;
	DWORD unknown_1244;
	DWORD unknown_1248;
	DWORD unknown_124C;
	DWORD unknown_1250;
	DWORD unknown_1254;
	HashTable<RadarTrackingStruct, TechnoClass*>* unknown_1258;
	DynamicVectorClass<Point2D> unknown_points_125C;
	DWORD unknown_1274;
	DynamicVectorClass<Point2D> FoundationTypePixels[22];
	float RadarSizeFactor;
	int unknown_int_148C;
	DWORD unknown_1490;
	DWORD unknown_1494;
	DWORD unknown_1498;
	RectangleStruct unknown_rect_149C;
	DWORD unknown_14AC;
	DWORD unknown_14B0;
	DWORD unknown_14B4;
	DWORD unknown_14B8;
	bool unknown_bool_14BC;
	bool unknown_bool_14BD;
	AudioController unknow_audio_14C0;
	//DWORD unknown_14C0;
	//DWORD unknown_14C4;
	//DWORD unknown_14C8;
	//DWORD unknown_14CC;
	//DWORD unknown_14D0;
	int unknown_int_14D4;
	bool IsAvailableNow;
	bool unknown_bool_14D9;
	bool unknown_bool_14DA;
	RectangleStruct unknown_rect_14DC;
	RectangleStruct unknown_rect_14EC;
	DWORD RadarAnimFrame; //0x14FC
	CDTimerClass unknown_timer_1500;
};

static_assert(sizeof(RadarClass) == 0x150C, "Invalid Size !");