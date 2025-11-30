#pragma once

namespace Unsorted
{
	OPTIONALINLINE COMPILETIMEEVAL int DefaultDockDir = 0x4000;

	// The length of a cell in its isometric projection
	// If an object's Height is above this value it's considered as in-air
	OPTIONALINLINE COMPILETIMEEVAL int CellHeight = 208;
	// Leptons of a cell's diagonal /2 /sin(60deg)
	// LeptonsPerCell *sqrt(2) /2 */ (sqrt(3)/2)
	// 256 * sqrt(2/3)

	// The height in the middle of a cell with a slope of 30 degrees
	OPTIONALINLINE COMPILETIMEEVAL int LevelHeight = 104;
	// tan(deg2rad(90) - deg2rad(60)) * sqrt(2 * 256 ^ 2) * 0.5
	// cot(deg2rad(30)) * diagonal_leptons_per_cell * 0.5
	// sqrt(3)/3 * 362.038 * 0.5

	// Leptons per cell.
	OPTIONALINLINE COMPILETIMEEVAL int LeptonsPerCell = 256;
	OPTIONALINLINE COMPILETIMEEVAL double d_LeptonsPerCell = 256.0;

	// Cell width in pixels.
	OPTIONALINLINE COMPILETIMEEVAL int CellWidthInPixels = 60;

	// Cell height in pixels.
	OPTIONALINLINE COMPILETIMEEVAL int CellHeightInPixels = 30;

	OPTIONALINLINE COMPILETIMEEVAL int PixelLeptonWidth = LeptonsPerCell / CellWidthInPixels;
	OPTIONALINLINE COMPILETIMEEVAL int PixelLeptonHeight = LeptonsPerCell / CellHeightInPixels;

	OPTIONALINLINE COMPILETIMEEVAL int HeightMax = 728;
	OPTIONALINLINE COMPILETIMEEVAL int BridgeLevels = 4;
	OPTIONALINLINE COMPILETIMEEVAL int BridgeHeight = BridgeLevels * LevelHeight;
};