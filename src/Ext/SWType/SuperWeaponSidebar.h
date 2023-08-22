#pragma once

#include "Body.h"

class SuperWeaponSidebar
{
	int MaxPerRow { 5 };
	int MaxShown { 6 };
	std::vector<SuperClass*> Supers {};
	RectangleStruct AffectedArea {};

	DSurface* Drawer { DSurface::Composite() };
	RectangleStruct* Bounds { &DSurface::ViewBounds() };
	COLORREF ToolTipColor { (DWORD)-1 };
	HouseClass* Owner { nullptr };
	BitFont* Font { BitFont::Instance() };

	SHPStruct* Bars[3] { nullptr };
	ConvertClass* BarsPalette[3] { nullptr };

	bool NeedSort { false };
public:

	static constexpr int cameoWidth = 60;
	static constexpr int cameoHeight = 48;

	enum BarPos : int
	{
		Top, Bottom, Right
	};

	void AddSuper(SuperClass* pSuper);
	void RemoveSuper(SuperClass* pSuper);
	bool IsClippedToTheArea();
	void OnHover();
	void onClick();
	void onRelease();
	void Draws();

private:
	static std::unique_ptr<SuperWeaponSidebar> Data;
public:

	static SuperWeaponSidebar* Instance();
	static void Clear();
	static void ReadFromINI();

protected:

	bool DrawCameos(SuperClass* pSuper, RectangleStruct* pDestRect, Point2D* pLoc);
	void DrawToolTip(SuperClass* pSuper, Point2D* pLoc);
	std::pair<SHPStruct*, ConvertClass*> GetBarData(BarPos pos);
	void DrawBordeBar(BarPos pos, Point2D* pLoc);

};