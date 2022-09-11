#pragma once

/*
These globals are so important and fundamental that no other files should be
included for them to be available.
*/
namespace Unsorted
{
	static const int& CurrentFrame = *reinterpret_cast<int*>(0xA8ED84);
	// The length of a cell in its isometric projection
	// If an object's Height is above this value it's considered as in-air
	constexpr int CellHeight = 208;
	// Leptons of a cell's diagonal /2 /sin(60deg)
	// LeptonsPerCell *sqrt(2) /2 */ (sqrt(3)/2)
	// 256 * sqrt(2/3)

	// The height in the middle of a cell with a slope of 30 degrees
	constexpr int LevelHeight = 104;
	// tan(deg2rad(90) - deg2rad(60)) * sqrt(2 * 256 ^ 2) * 0.5
	// cot(deg2rad(30)) * diagonal_leptons_per_cell * 0.5
	// sqrt(3)/3 * 362.038 * 0.5

	// Leptons per cell.
	constexpr int LeptonsPerCell = 256;
	constexpr double d_LeptonsPerCell = 256.0;

	// Cell width in pixels.
	constexpr int CellWidthInPixels = 60;

	// Cell height in pixels.
	constexpr int CellHeightInPixels = 30;
}
