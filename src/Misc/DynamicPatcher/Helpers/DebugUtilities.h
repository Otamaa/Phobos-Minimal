#pragma once
#ifdef COMPILE_PORTED_DP_FEATURES
#include <CoordStruct.h>
#include <LaserDrawClass.h>
#include <AbstractClass.h>
#include <vector>
#include <MapClass.h>

struct DebugUtilities
{
	static void MarkLocation(CoordStruct location, ColorStruct color, int beamHeight = 1000, int thickness = 4, int duration = 10)
	{
		ColorStruct innerColor { color.R, color.G, color.B };
		ColorStruct outerColor { color.R / 2, color.G / 2, color.B / 2 };
		ColorStruct outerSpread { color.R / 4, color.G / 4, color.B / 4 };
		CoordStruct nOffset { 0, 0, beamHeight };

		if (auto pMarkLaser = GameCreate<LaserDrawClass>(location, location + nOffset, innerColor, outerColor, outerSpread, duration))
		{
			pMarkLaser->Thickness = thickness;
			pMarkLaser->IsHouseColor = true;
		}
	}

	static void MarkTarget(AbstractClass* pTarget, ColorStruct color, int beamHeight = 1000, int thickness = 4, int duration = 10)
	{
		CoordStruct location = pTarget->GetCoords();

		MarkLocation(location, color, beamHeight, duration);
	}

	static void HighlightObjectBlock_Draw(ColorStruct color, CoordStruct from, CoordStruct to, int thickness, int duration)
	{
		ColorStruct innerColor { color.R, color.G, color.B };
		ColorStruct outerColor { color.R / 2, color.G / 2, color.B / 2 };
		ColorStruct outerSpread { color.R / 4, color.G / 4, color.B / 4 };

		if (auto pMarkLaser = GameCreate<LaserDrawClass>(from, to, innerColor, outerColor, outerSpread, duration))
		{
			pMarkLaser->Thickness = thickness;
			pMarkLaser->IsHouseColor = true;
		}
	}

	/*
	static void HighlightObject(ObjectBlock block, ColorStruct color, int thickness = 4, int duration = 10)
	{
		std::vector<CoordStruct> points { };
		int offset = 200;
		std::array<CellStruct, 4> Offs =
		{{
			{offset, offset} , {-offset, offset} ,
			{ -offset, -offset } , {offset, -offset}
		}};

		for(auto& cellOffset : Offs)
		{
			if (auto pCell = MapClass::Instance->TryGetCellAt(block.Center + cellOffset))
			{
				points.push_back(pCell->GetCoords());
			}
		}

		for (int i = 0; i < points.size(); i++)
		{
			HighlightObjectBlock_Draw(color,points[i], points[(i + 1) % points.size()], thickness,duration);
		}
	}*/
};
#endif