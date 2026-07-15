#pragma once

#include <array>

#include <Unsorted.h>
#include <CellStruct.h>
#include <Point2D.h>
#include <DirStruct.h>

class CellSpread
{
public:
	// between -256 -> 256
	// static COMPILETIMEEVAL reference<Point2D, 0x89F6D8, 8u> const AdjacentPoint{};
	static COMPILETIMEEVAL std::array<Point2D, 8> const AdjacentPoint { {
		{   0, -256 },   // North
		{ 256, -256 },   // North-East
		{ 256,    0 },   // East
		{ 256,  256 },   // South-East
		{   0,  256 },   // South
		{-256,  256 },   // South-West
		{-256,    0 },   // West
		{-256, -256 },   // North-West
	} };

	// between -1 -> 1
	// static COMPILETIMEEVAL reference<CellStruct, 0x89F688, 8u> const AdjacentCell{};
	static COMPILETIMEEVAL std::array<CellStruct, 8> const AdjacentCell { {
		{  0, -1 },   // North
		{  1, -1 },   // North-East
		{  1,  0 },   // East
		{  1,  1 },   // South-East
		{  0,  1 },   // South
		{ -1,  1 },   // South-West
		{ -1,  0 },   // West
		{ -1, -1 },   // North-West
	} };

	// The values are increasing distance thresholds used when expanding searches
	// outward in concentric rings. Keeping this table identical to the original
	// preserves the game's search behavior.
	// static COMPILETIMEEVAL reference<size_t, 0x7ED3D0, 12u> const CellNums{};
	static COMPILETIMEEVAL std::array<int, 12> const CellNums  { {
		  1,
		  9,
		 21,
		 37,
		 61,
		 89,
		121,
		161,
		205,
		253,
		309,
		369,
	} };

	// static COMPILETIMEEVAL reference<CellStruct, 0xABD490, 369u> const CellOfssets{};
	static COMPILETIMEEVAL std::array<CellStruct, 370> const CellOfssets { {
		{0, 0},        {1, -1},      {0, -1},      {-1, -1},
		{-1, 0},       {1, 0},       {-1, 1},      {0, 1},
		{1, 1},        {-1, -2},     {0, -2},      {1, -2},
		{-2, -1},      {2, -1},      {-2, 0},      {2, 0},
		{-2, 1},       {2, 1},       {-1, 2},      {0, 2},
		{1, 2},        {-1, -3},     {0, -3},      {1, -3},
		{-2, -2},      {2, -2},      {-3, -1},     {3, -1},
		{-3, 0},       {3, 0},       {-3, 1},      {3, 1},
		{-2, 2},       {2, 2},       {-1, 3},      {0, 3},
		{1, 3},        {-1, -4},     {0, -4},      {1, -4},
		{-3, -3},      {-2, -3},     {2, -3},      {3, -3},
		{-3, -2},      {3, -2},      {-4, -1},     {4, -1},
		{-4, 0},       {4, 0},       {-4, 1},      {4, 1},
		{-3, 2},       {3, 2},       {-3, 3},      {-2, 3},
		{2, 3},        {3, 3},       {-1, 4},      {0, 4},
		{1, 4},        {-1, -5},     {0, -5},      {1, -5},
		{-3, -4},      {-2, -4},     {2, -4},      {3, -4},
		{-4, -3},      {4, -3},      {-4, -2},     {4, -2},
		{-5, -1},      {5, -1},      {-5, 0},      {5, 0},
		{-5, 1},       {5, 1},       {-4, 2},      {4, 2},
		{-4, 3},       {4, 3},       {-3, 4},      {-2, 4},
		{2, 4},        {3, 4},       {-1, 5},      {0, 5},
		{1, 5},        {-1, -6},     {0, -6},      {1, -6},
		{-3, -5},      {-2, -5},     {2, -5},      {3, -5},
		{-4, -4},      {4, -4},      {-5, -3},     {5, -3},
		{-5, -2},      {5, -2},      {-6, -1},     {6, -1},
		{-6, 0},       {6, 0},       {-6, 1},      {6, 1},
		{-5, 2},       {5, 2},       {-5, 3},      {5, 3},
		{-4, 4},       {4, 4},       {-3, 5},      {-2, 5},
		{2, 5},        {3, 5},       {-1, 6},      {0, 6},
		{1, 6},        {-1, -7},     {0, -7},      {1, -7},
		{-3, -6},      {-2, -6},     {2, -6},      {3, -6},
		{-5, -5},      {-4, -5},     {4, -5},      {5, -5},
		{-5, -4},      {5, -4},      {-6, -3},     {6, -3},
		{-6, -2},      {6, -2},      {-7, -1},     {7, -1},
		{-7, 0},       {7, 0},       {-7, 1},      {7, 1},
		{-6, 2},       {6, 2},       {-6, 3},      {6, 3},
		{-5, 4},       {5, 4},       {-5, 5},      {-4, 5},
		{4, 5},        {5, 5},       {-3, 6},      {-2, 6},
		{2, 6},        {3, 6},       {-1, 7},      {0, 7},
		{1, 7},        {-1, -8},     {0, -8},      {1, -8},
		{-3, -7},      {-2, -7},     {2, -7},      {3, -7},
		{-5, -6},      {-4, -6},     {4, -6},      {5, -6},
		{-6, -5},      {6, -5},      {-6, -4},     {6, -4},
		{-7, -3},      {7, -3},      {-7, -2},     {7, -2},
		{-8, -1},      {8, -1},      {-8, 0},      {8, 0},
		{-8, 1},       {8, 1},       {-7, 2},      {7, 2},
		{-7, 3},       {7, 3},       {-6, 4},      {6, 4},
		{-6, 5},       {6, 5},       {-5, 6},      {-4, 6},
		{4, 6},        {5, 6},       {-3, 7},      {-2, 7},
		{2, 7},        {3, 7},       {-1, 8},      {0, 8},
		{1, 8},        {-1, -9},     {0, -9},      {1, -9},
		{-3, -8},      {-2, -8},     {2, -8},      {3, -8},
		{-5, -7},      {-4, -7},     {4, -7},      {5, -7},
		{-6, -6},      {6, -6},      {-7, -5},     {7, -5},
		{-7, -4},      {7, -4},      {-8, -3},     {8, -3},
		{-8, -2},      {8, -2},      {-9, -1},     {9, -1},
		{-9, 0},       {9, 0},       {-9, 1},      {9, 1},
		{-8, 2},       {8, 2},       {-8, 3},      {8, 3},
		{-7, 4},       {7, 4},       {-7, 5},      {7, 5},
		{-6, 6},       {6, 6},       {-5, 7},      {-4, 7},
		{4, 7},        {5, 7},       {-3, 8},      {-2, 8},
		{2, 8},        {3, 8},       {-1, 9},      {0, 9},
		{1, 9},        {-1, -10},    {0, -10},     {1, -10},
		{-3, -9},      {-2, -9},     {2, -9},      {3, -9},
		{-5, -8},      {-4, -8},     {4, -8},      {5, -8},
		{-7, -7},      {-6, -7},     {6, -7},      {7, -7},
		{-7, -6},      {7, -6},      {-8, -5},     {8, -5},
		{-8, -4},      {8, -4},      {-9, -3},     {9, -3},
		{-9, -2},      {9, -2},      {-10, -1},    {10, -1},
		{-10, 0},      {10, 0},      {-10, 1},     {10, 1},
		{-9, 2},       {9, 2},       {-9, 3},      {9, 3},
		{-8, 4},       {8, 4},       {-8, 5},      {8, 5},
		{-7, 6},       {7, 6},       {-7, 7},      {-6, 7},
		{6, 7},        {7, 7},       {-5, 8},      {-4, 8},
		{4, 8},        {5, 8},       {-3, 9},      {-2, 9},
		{2, 9},        {3, 9},       {-1, 10},     {0, 10},
		{1, 10},       {0, 11},      {0, -11},     {-1, 11},
		{1, 11},       {-1, -11},    {1, -11},     {-2, 11},
		{2, 11},       {-2, -11},    {2, -11},     {-3, 11},
		{3, 11},       {-3, -11},    {-3, 11},     {-4, 9},
		{4, 9},        {-4, -9},     {4, -9},      {-5, 9},
		{5, 9},        {-5, -9},     {5, -9},      {-6, 8},
		{6, 8},        {-6, -8},     {6, -8},      {-7, 8},
		{7, 8},        {-7, -8},     {7, -8},      {-8, 7},
		{8, 7},        {-8, -7},     {8, -7},      {-8, 6},
		{8, 6},        {-8, -6},     {8, -6},      {-9, 5},
		{9, 5},        {-9, -5},     {9, -5},      {-9, 4},
		{9, 4},        {-9, -4},     {9, -4},      {-10, 3},
		{10, 3},       {-10, -3},    {10, -3},     {-10, 2},
		{10, 2},       {-10, -2},    {10, -2},     {-11, 1},
		{11, 1},       {-11, -1},    {11, -1},     {11, 0},
		{-11, 0},      {0, 0}        // terminal entry
	} };

	static COMPILETIMEEVAL FORCEDINLINE size_t NumCells(size_t nSpread) {
		return CellNums[nSpread];
	}

	static COMPILETIMEEVAL FORCEDINLINE size_t NumCellsFromFloat(float nSpread) {
		return int(nSpread);
	}

	static COMPILETIMEEVAL FORCEDINLINE CellStruct GetCell(size_t n) {
		return CellOfssets[n];
	}

	static COMPILETIMEEVAL CellStruct GetNeighbourOffset(size_t direction) {
		if(direction >= AdjacentCell.size()) {
			return CellStruct::Empty;
		}

		return AdjacentCell[direction];
	}

	static COMPILETIMEEVAL Point2D GetNeighbourPointOffset(size_t direction)
	{
		if (direction >= AdjacentPoint.size()) {
			return Point2D::Empty;
		}

		return AdjacentPoint[direction];
	}

	static COMPILETIMEEVAL Point2D GetNeighbourPointOffset(FacingType direction)
	{
		if ((size_t)direction >= AdjacentPoint.size()) {
			return Point2D::Empty;
		}

		return AdjacentPoint[(size_t)direction];
	}

	static COMPILETIMEEVAL CellStruct GetNeighbourOffset(FacingType direction)
	{
		if ((size_t)direction >= AdjacentCell.size()) {
			return CellStruct::Empty;
		}

		return AdjacentCell[(size_t)direction];
	}

	static size_t GetDistance(int dx, int dy) {
		auto x = static_cast<size_t>(Math::abs(dx));
		auto y = static_cast<size_t>(Math::abs(dy));

		// distance is longer component plus
		// half the shorter component
		if(x > y) {
			return x + y / 2;
		} else {
			return y + x / 2;
		}
	};

	static size_t GetDistance(const CellStruct &offset) {
		return GetDistance(offset.X, offset.Y);
	};

	static size_t GetDistance(const Point2D& offset) {
		return GetDistance(offset.X / 256, offset.Y / 256);
	};
};
