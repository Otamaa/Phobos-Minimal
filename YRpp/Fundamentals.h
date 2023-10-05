#pragma once

namespace Unsorted
{
	static inline constexpr int DefaultDockDir = 0x4000;

	// The length of a cell in its isometric projection
	// If an object's Height is above this value it's considered as in-air
	static inline constexpr int CellHeight = 208;
	// Leptons of a cell's diagonal /2 /sin(60deg)
	// LeptonsPerCell *sqrt(2) /2 */ (sqrt(3)/2)
	// 256 * sqrt(2/3)

	// The height in the middle of a cell with a slope of 30 degrees
	static inline constexpr int LevelHeight = 104;
	// tan(deg2rad(90) - deg2rad(60)) * sqrt(2 * 256 ^ 2) * 0.5
	// cot(deg2rad(30)) * diagonal_leptons_per_cell * 0.5
	// sqrt(3)/3 * 362.038 * 0.5

	// Leptons per cell.
	static inline constexpr int LeptonsPerCell = 256;
	static inline constexpr double d_LeptonsPerCell = 256.0;

	// Cell width in pixels.
	static inline constexpr int CellWidthInPixels = 60;

	// Cell height in pixels.
	static inline constexpr int CellHeightInPixels = 30;

	static inline constexpr int PixelLeptonWidth = LeptonsPerCell / CellWidthInPixels;
	static inline constexpr int PixelLeptonHeight = LeptonsPerCell / CellHeightInPixels;

	static inline constexpr int HeightMax = 728;
	static inline constexpr int BridgeLevels = 4;
	static inline constexpr int BridgeHeight = BridgeLevels * LevelHeight;
	static inline constexpr double GameMagicNumbr_ = 0.1657281546769136;
};