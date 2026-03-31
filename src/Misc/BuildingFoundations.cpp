#include "BuildingFoundations.h"


// FoundationCells - Interior cell coordinates (which tiles are occupied)
// Array starts at address: 0089C900

#define SENTINEL { -1 , -1}

/*
 * Index 0: Empty/None
 * Visual: (no cells)
 */
static COMPILETIMEEVAL FoundationStruct FoundationCells_0 = { {
	{0, 0}, SENTINEL, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

/*
 * Index 1: 1x1 Horizontal
 * Visual:
 *   0 1
 * 0 █
 */
static COMPILETIMEEVAL FoundationStruct FoundationCells_1 = { {
	{0, 0}, {1, 0}, SENTINEL, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

/*
 * Index 2: 1x1 Vertical
 * Visual:
 *   0
 * 0 █
 * 1
 */
static COMPILETIMEEVAL FoundationStruct FoundationCells_2 = { {
	{0, 0}, {0, 1}, SENTINEL, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

/*
 * Index 3: 2x2
 * Visual:
 *   0 1
 * 0 █ █
 * 1 █ █
 */
static COMPILETIMEEVAL FoundationStruct FoundationCells_3 = { {
	{0, 0}, {1, 0}, {0, 1}, {1, 1}, SENTINEL,
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

/*
 * Index 4: 2x3
 * Visual:
 *   0 1
 * 0 █ █
 * 1 █ █
 * 2 █ █
 */
static COMPILETIMEEVAL FoundationStruct FoundationCells_4 = { {
	{0, 0}, {1, 0}, {0, 1}, {1, 1}, {0, 2},
	{1, 2}, SENTINEL, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

/*
 * Index 5: 3x2
 * Visual:
 *   0 1 2
 * 0 █ █ █
 * 1 █ █ █
 */
static COMPILETIMEEVAL FoundationStruct FoundationCells_5 = { {
	{0, 0}, {1, 0}, {2, 0}, {0, 1}, {1, 1},
	{2, 1}, SENTINEL, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

/*
 * Index 6: 3x3
 * Visual:
 *   0 1 2
 * 0 █ █ █
 * 1 █ █ █
 * 2 █ █ █
 */
static COMPILETIMEEVAL FoundationStruct FoundationCells_6 = { {
	{0, 0}, {1, 0}, {2, 0}, {0, 1}, {1, 1},
	{2, 1}, {0, 2}, {1, 2}, {2, 2}, SENTINEL,
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

/*
 * Index 7: 3x5
 * Visual:
 *   0 1 2
 * 0 █ █ █
 * 1 █ █ █
 * 2 █ █ █
 * 3 █ █ █
 * 4 █ █ █
 */
static COMPILETIMEEVAL FoundationStruct FoundationCells_7 = { {
	{0, 0}, {1, 0}, {2, 0}, {0, 1}, {1, 1},
	{2, 1}, {0, 2}, {1, 2}, {2, 2}, {0, 3},
	{1, 3}, {2, 3}, {0, 4}, {1, 4}, {2, 4},
	SENTINEL, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

/*
 * Index 8: 4x2
 * Visual:
 *   0 1 2 3
 * 0 █ █ █ █
 * 1 █ █ █ █
 */
static COMPILETIMEEVAL FoundationStruct FoundationCells_8 = { {
	{0, 0}, {1, 0}, {2, 0}, {3, 0}, {0, 1},
	{1, 1}, {2, 1}, {3, 1}, SENTINEL, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

/*
 * Index 9: 3x3 L-Shape
 * Visual:
 *   0 1 2
 * 0 █ █
 * 1 █ █
 * 2 █ █ █
 */
static COMPILETIMEEVAL FoundationStruct FoundationCells_9 = { {
	{0, 0}, {1, 0}, {0, 1}, {1, 1}, {0, 2},
	{1, 2}, {2, 2}, SENTINEL, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

/*
 * Index 10: 1x3 Vertical
 * Visual:
 *   0
 * 0 █
 * 1 █
 * 2 █
 */
static COMPILETIMEEVAL FoundationStruct FoundationCells_10 = { {
	{0, 0}, {0, 1}, {0, 2}, SENTINEL, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

/*
 * Index 11: 3x1 Horizontal
 * Visual:
 *   0 1 2
 * 0 █ █ █
 */
static COMPILETIMEEVAL FoundationStruct FoundationCells_11 = { {
	{0, 0}, {1, 0}, {2, 0}, SENTINEL, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

/*
 * Index 12: 4x3
 * Visual:
 *   0 1 2 3
 * 0 █ █ █ █
 * 1 █ █ █ █
 * 2 █ █ █ █
 */
static COMPILETIMEEVAL FoundationStruct FoundationCells_12 = { {
	{0, 0}, {1, 0}, {2, 0}, {3, 0}, {0, 1},
	{1, 1}, {2, 1}, {3, 1}, {0, 2}, {1, 2},
	{2, 2}, {3, 2}, SENTINEL, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

/*
 * Index 13: 1x4 Vertical
 * Visual:
 *   0
 * 0 █
 * 1 █
 * 2 █
 * 3 █
 */
static COMPILETIMEEVAL FoundationStruct FoundationCells_13 = { {
	{0, 0}, {0, 1}, {0, 2}, {0, 3}, SENTINEL,
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

/*
 * Index 14: 1x5 Vertical
 * Visual:
 *   0
 * 0 █
 * 1 █
 * 2 █
 * 3 █
 * 4 █
 */
static COMPILETIMEEVAL FoundationStruct FoundationCells_14 = { {
	{0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4},
	SENTINEL, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

/*
 * Index 15: 2x6 Vertical
 * Visual:
 *   0 1
 * 0 █ █
 * 1 █ █
 * 2 █ █
 * 3 █ █
 * 4 █ █
 * 5 █ █
 */
static COMPILETIMEEVAL FoundationStruct FoundationCells_15 = { {
	{0, 0}, {1, 0}, {0, 1}, {1, 1}, {0, 2},
	{1, 2}, {0, 3}, {1, 3}, {0, 4}, {1, 4},
	{0, 5}, {1, 5}, SENTINEL, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

/*
 * Index 16: 2x5 Vertical
 * Visual:
 *   0 1
 * 0 █ █
 * 1 █ █
 * 2 █ █
 * 3 █ █
 * 4 █ █
 */
static COMPILETIMEEVAL FoundationStruct FoundationCells_16 = { {
	{0, 0}, {1, 0}, {0, 1}, {1, 1}, {0, 2},
	{1, 2}, {0, 3}, {1, 3}, {0, 4}, {1, 4},
	SENTINEL, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

/*
 * Index 17: 5x3
 * Visual:
 *   0 1 2 3 4
 * 0 █ █ █ █ █
 * 1 █ █ █ █ █
 * 2 █ █ █ █ █
 */
static COMPILETIMEEVAL FoundationStruct FoundationCells_17 = { {
	{0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0},
	{0, 1}, {1, 1}, {2, 1}, {3, 1}, {4, 1},
	{0, 2}, {1, 2}, {2, 2}, {3, 2}, {4, 2},
	SENTINEL, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

/*
 * Index 18: 4x4
 * Visual:
 *   0 1 2 3
 * 0 █ █ █ █
 * 1 █ █ █ █
 * 2 █ █ █ █
 * 3 █ █ █ █
 */
static COMPILETIMEEVAL FoundationStruct FoundationCells_18 = { {
	{0, 0}, {1, 0}, {2, 0}, {3, 0}, {0, 1},
	{1, 1}, {2, 1}, {3, 1}, {0, 2}, {1, 2},
	{2, 2}, {3, 2}, {0, 3}, {1, 3}, {2, 3},
	{3, 3}, SENTINEL, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

/*
 * Index 19: 3x4
 * Visual:
 *   0 1 2
 * 0 █ █ █
 * 1 █ █ █
 * 2 █ █ █
 * 3 █ █ █
 */
static COMPILETIMEEVAL FoundationStruct FoundationCells_19 = { {
	{0, 0}, {1, 0}, {2, 0}, {0, 1}, {1, 1},
	{2, 1}, {0, 2}, {1, 2}, {2, 2}, {0, 3},
	{1, 3}, {2, 3}, SENTINEL, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

/*
 * Index 20: 6x4
 * Visual:
 *   0 1 2 3 4 5
 * 0 █ █ █ █ █ █
 * 1 █ █ █ █ █ █
 * 2 █ █ █ █ █ █
 * 3 █ █ █ █ █ █
 */
static COMPILETIMEEVAL FoundationStruct FoundationCells_20 = { {
	{0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0},
	{5, 0}, {0, 1}, {1, 1}, {2, 1}, {3, 1},
	{4, 1}, {5, 1}, {0, 2}, {1, 2}, {2, 2},
	{3, 2}, {4, 2}, {5, 2}, {0, 3}, {1, 3},
	{2, 3}, {3, 3}, {4, 3}, {5, 3}, SENTINEL,
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

/*
 * Index 21: Terminator
 */
static COMPILETIMEEVAL FoundationStruct FoundationCells_21 = { {
	SENTINEL, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

std::array<FoundationStruct, 22> FoundationDataStruct::Cells = { {
	FoundationCells_0, FoundationCells_1, FoundationCells_2,
	FoundationCells_3, FoundationCells_4, FoundationCells_5,
	FoundationCells_6, FoundationCells_7, FoundationCells_8,
	FoundationCells_9, FoundationCells_10, FoundationCells_11,
	FoundationCells_12, FoundationCells_13, FoundationCells_14,
	FoundationCells_15, FoundationCells_16, FoundationCells_17,
	FoundationCells_18, FoundationCells_19, FoundationCells_20,
	FoundationCells_21
} };


//====================================================================
// FoundationOutlines - Border/perimeter coordinates for rendering
// Array starts at address: 0089D368

/*
 * Index 0: 1x1 Foundation Border
 * Visual:
 *     -1  0  1
 * -1   B  B  B
 *  0  B  █  B
 *  1   B  B  B
 *
 * B = Border points (outline)
 * █ = Cell (interior)
 */
static COMPILETIMEEVAL FoundationStruct FoundationOutlines_0 = { {
	{0, 1}, {-1, 1}, {1, 1}, {-1, 0}, {1, 0},
	{-1, -1}, {0, -1}, {1, -1}, SENTINEL, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

/*
 * Index 1: 2x1 Foundation Border
 * Visual:
 *     -1  0  1  2
 * -1   B  B  B  B
 *  0  B  █  █  B
 *  1   B  B  B  B
 */
static COMPILETIMEEVAL FoundationStruct FoundationOutlines_1 = { {
	{0, 1}, {1, 1}, {-1, 1}, {2, 1}, {-1, 0},
	{2, 0}, {0, -1}, {1, -1}, {-1, -1}, {2, -1},
	SENTINEL, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

/*
 * Index 2: 1x2 Foundation Border
 * Visual:
 *     -1  0  1
 * -1   B  B  B
 *  0  B  █  B
 *  1  B  █  B
 *  2   B  B  B
 */
static COMPILETIMEEVAL FoundationStruct FoundationOutlines_2 = { {
	{0, 2}, {1, 2}, {-1, 2}, {-1, 1}, {1, 1},
	{-1, 0}, {1, 0}, {0, -1}, {-1, -1}, {1, -1},
	SENTINEL, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

/*
 * Index 3: 2x2 Foundation Border
 * Visual:
 *     -1  0  1  2
 * -1   B  B  B  B
 *  0  B  █  █  B
 *  1  B  █  █  B
 *  2   B  B  B  B
 */
static COMPILETIMEEVAL FoundationStruct FoundationOutlines_3 = { {
	{0, 2}, {1, 2}, {-1, 2}, {2, 2}, {-1, 1},
	{2, 1}, {-1, 0}, {2, 0}, {0, -1}, {1, -1},
	{-1, -1}, {2, -1}, SENTINEL, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

/*
 * Index 4: 2x3 Foundation Border
 * Visual:
 *     -1  0  1  2
 * -1   B  B  B  B
 *  0  B  █  █  B
 *  1  B  █  █  B
 *  2  B  █  █  B
 *  3   B  B  B  B
 */
static COMPILETIMEEVAL FoundationStruct FoundationOutlines_4 = { {
	{0, 3}, {1, 3}, {-1, 3}, {2, 3}, {-1, 2},
	{2, 2}, {-1, 1}, {2, 1}, {-1, 0}, {2, 0},
	{0, -1}, {1, -1}, {-1, -1}, {2, -1}, SENTINEL,
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

/*
 * Index 5: 3x2 Foundation Border
 * Visual:
 *     -1  0  1  2  3
 * -1   B  B  B  B  B
 *  0  B  █  █  █  B
 *  1  B  █  █  █  B
 *  2   B  B  B  B  B
 */
static COMPILETIMEEVAL FoundationStruct FoundationOutlines_5 = { {
	{0, 2}, {1, 2}, {2, 2}, {-1, 2}, {3, 2},
	{-1, 1}, {3, 1}, {-1, 0}, {3, 0}, {0, -1},
	{1, -1}, {2, -1}, {-1, -1}, {3, -1}, SENTINEL,
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

/*
 * Index 6: 3x3 Foundation Border
 * Visual:
 *     -1  0  1  2  3
 * -1   B  B  B  B  B
 *  0  B  █  █  █  B
 *  1  B  █  █  █  B
 *  2  B  █  █  █  B
 *  3   B  B  B  B  B
 */
static COMPILETIMEEVAL FoundationStruct FoundationOutlines_6 = { {
	{0, 3}, {1, 3}, {2, 3}, {-1, 3}, {3, 3},
	{-1, 2}, {3, 2}, {-1, 1}, {3, 1}, {-1, 0},
	{3, 0}, {0, -1}, {1, -1}, {2, -1}, {-1, -1},
	{3, -1}, SENTINEL, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

/*
 * Index 7: 4x6 Foundation Border (Special tall building)
 * Visual:
 *     -1  0  1  2  3
 * -1   B  B  B  B  B
 *  0  B  █  █  █  B
 *  1  B  █  █  █  B
 *  2  B  █  █  █  B
 *  3  B  █  █  █  B
 *  4  B  █  █  █  B
 *  5   B  B  B  B  B
 */
static COMPILETIMEEVAL FoundationStruct FoundationOutlines_7 = { {
	{-1, -1}, {0, -1}, {1, -1}, {2, -1}, {3, -1},
	{-1, 0}, {3, 0}, {-1, 1}, {3, 1}, {-1, 2},
	{3, 2}, {-1, 3}, {3, 3}, {-1, 4}, {3, 4},
	{-1, 5}, {0, 5}, {1, 5}, {2, 5}, {3, 5},
	SENTINEL, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

/*
 * Index 8: 5x2 Foundation Border
 * Visual:
 *     -1  0  1  2  3  4
 * -1   B  B  B  B  B  B
 *  0  B  █  █  █  █  B
 *  1  B  █  █  █  █  B
 *  2   B  B  B  B  B  B
 */
static COMPILETIMEEVAL FoundationStruct FoundationOutlines_8 = { {
	{0, 2}, {1, 2}, {2, 2}, {3, 2}, {-1, 2},
	{4, 2}, {-1, 1}, {4, 1}, {-1, 0}, {4, 0},
	{0, -1}, {1, -1}, {2, -1}, {3, -1}, {-1, -1},
	{4, -1}, SENTINEL, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

/*
 * Index 9: Empty/Null Border
 */
static COMPILETIMEEVAL FoundationStruct FoundationOutlines_9 = { {
	{0, 0}, SENTINEL, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

/*
 * Index 10: 2x4 Vertical Foundation Border
 * Visual:
 *     -1  0  1
 * -1   B  B  B
 *  0  B  █  B
 *  1  B  █  B
 *  2  B  █  B
 *  3   B  B  B
 */
static COMPILETIMEEVAL FoundationStruct FoundationOutlines_10 = { {
	{-1, -1}, {0, -1}, {1, -1}, {-1, 0}, {1, 0},
	{-1, 1}, {1, 1}, {-1, 2}, {1, 2}, {-1, 3},
	{0, 3}, {1, 3}, SENTINEL, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

/*
 * Index 11: 4x2 Horizontal Foundation Border
 * Visual:
 *     -1  0  1  2  3
 * -1   B  B  B  B  B
 *  0  B  █  █  █  B
 *  1   B  B  B  B  B
 */
static COMPILETIMEEVAL FoundationStruct FoundationOutlines_11 = { {
	{-1, -1}, {0, -1}, {1, -1}, {2, -1}, {3, -1},
	{-1, 0}, {3, 0}, {-1, 1}, {0, 1}, {1, 1},
	{2, 1}, {3, 1}, SENTINEL, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

/*
 * Index 12: 3x3 Foundation Border (Duplicate)
 * Visual: Same as Index 6
 */
static COMPILETIMEEVAL FoundationStruct FoundationOutlines_12 = { {
	{0, 3}, {1, 3}, {2, 3}, {-1, 3}, {3, 3},
	{-1, 2}, {3, 2}, {-1, 1}, {3, 1}, {-1, 0},
	{3, 0}, {0, -1}, {1, -1}, {2, -1}, {-1, -1},
	{3, -1}, SENTINEL, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

/*
 * Index 13: 2x5 Vertical Foundation Border
 * Visual:
 *     -1  0  1
 * -1   B  B  B
 *  0  B  █  B
 *  1  B  █  B
 *  2  B  █  B
 *  3  B  █  B
 *  4   B  B  B
 */
static COMPILETIMEEVAL FoundationStruct FoundationOutlines_13 = { {
	{-1, -1}, {0, -1}, {1, -1}, {-1, 0}, {1, 0},
	{-1, 1}, {1, 1}, {-1, 2}, {1, 2}, {-1, 3},
	{1, 3}, {-1, 4}, {0, 4}, {1, 4}, SENTINEL,
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

/*
 * Index 14: 2x6 Vertical Foundation Border
 * Visual:
 *     -1  0  1
 * -1   B  B  B
 *  0  B  █  B
 *  1  B  █  B
 *  2  B  █  B
 *  3  B  █  B
 *  4  B  █  B
 *  5   B  B  B
 */
static COMPILETIMEEVAL FoundationStruct FoundationOutlines_14 = { {
	{-1, -1}, {0, -1}, {1, -1}, {-1, 0}, {1, 0},
	{-1, 1}, {1, 1}, {-1, 2}, {1, 2}, {-1, 3},
	{1, 3}, {-1, 4}, {1, 4}, {-1, 5}, {0, 5},
	{1, 5}, SENTINEL, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

/*
 * Index 15: 3x7 Foundation Border (Special - has gap)
 * Visual:
 *     -1  0  1  2
 * -1   B  B  B  B
 *  0  B  █  █  B
 *  1  B  █  █  B
 *  2  B  █  █  B
 *  3  B  █  █  B
 *  4  (gap in Y=4)
 *  5  B  █  █  B
 *  6   B  B  B  B
 */
static COMPILETIMEEVAL FoundationStruct FoundationOutlines_15 = { {
	{-1, -1}, {0, -1}, {1, -1}, {2, -1}, {-1, 0},
	{2, 0}, {-1, 1}, {2, 1}, {-1, 2}, {2, 2},
	{-1, 3}, {2, 3}, {-1, 5}, {2, 5}, {-1, 6},
	{0, 6}, {1, 6}, {2, 6}, SENTINEL, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

/*
 * Index 16: 3x6 Foundation Border (Special - has gap)
 * Visual:
 *     -1  0  1  2
 * -1   B  B  B  B
 *  0  B  █  █  B
 *  1  B  █  █  B
 *  2  B  █  █  B
 *  3  B  █  █  B
 *  4  (gap in Y=4)
 *  5   B  B  B  B
 */
static COMPILETIMEEVAL FoundationStruct FoundationOutlines_16 = { {
	{-1, -1}, {0, -1}, {1, -1}, {2, -1}, {-1, 0},
	{2, 0}, {-1, 1}, {2, 1}, {-1, 2}, {2, 2},
	{-1, 3}, {2, 3}, {-1, 5}, {0, 5}, {1, 5},
	{2, 5}, SENTINEL, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

/*
 * Index 17: 6x4 Foundation Border
 * Visual:
 *     -1  0  1  2  3  4  5
 * -1   B  B  B  B  B  B  B
 *  0  B  █  █  █  █  █  B
 *  1  B  █  █  █  █  █  B
 *  2  B  █  █  █  █  █  B
 *  3   B  B  B  B  B  B  B
 */
static COMPILETIMEEVAL FoundationStruct FoundationOutlines_17 = { {
	{-1, -1}, {0, -1}, {1, -1}, {2, -1}, {3, -1},
	{4, -1}, {5, -1}, {-1, 0}, {5, 0}, {-1, 1},
	{5, 1}, {-1, 2}, {5, 2}, {-1, 3}, {0, 3},
	{1, 3}, {2, 3}, {3, 3}, {4, 3}, {5, 3},
	SENTINEL, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

/*
 * Index 18: 5x5 Foundation Border (Special square)
 * Visual:
 *     -1  0  1  2  3  4
 * -1   B  B  B  B  B  B
 *  0  B  █  █  █  █  B
 *  1  B  █  █  █  █  B
 *  2  B  █  █  █  █  B
 *  3  B  █  █  █  █  B
 *  4   B  B  B  B  B  B
 */
static COMPILETIMEEVAL FoundationStruct FoundationOutlines_18 = { {
	{-1, -1}, {0, -1}, {1, -1}, {2, -1}, {3, -1},
	{4, -1}, {-1, 4}, {0, 4}, {1, 4}, {2, 4},
	{3, 4}, {4, 4}, {-1, 0}, {-1, 1}, {-1, 2},
	{-1, 3}, {-1, 4}, {4, 0}, {4, 1}, {4, 2},
	{4, 3}, {4, 4}, SENTINEL, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

/*
 * Index 19: 3x4 Foundation Border
 * Visual:
 *     -1  0  1  2  3
 * -1   B  B  B  B  B
 *  0  B  █  █  █  B
 *  1  B  █  █  █  B
 *  2  B  █  █  █  B
 *  3  (skipped)
 *  4   B  B  B  B  B
 */
static COMPILETIMEEVAL FoundationStruct FoundationOutlines_19 = { {
	{0, 4}, {1, 4}, {2, 4}, {-1, 4}, {3, 4},
	{-1, 2}, {3, 2}, {-1, 1}, {3, 1}, {-1, 0},
	{3, 0}, {0, -1}, {1, -1}, {2, -1}, {-1, -1},
	{3, -1}, SENTINEL, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

/*
 * Index 20: Special Foundation Border (Single point)
 * Visual: Only one border point at {2, -1}
 */
static COMPILETIMEEVAL FoundationStruct FoundationOutlines_20 = { {
	{2, -1}, SENTINEL, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

/*
 * Index 21: Terminator/Null Border
 */
static COMPILETIMEEVAL FoundationStruct FoundationOutlines_21 = { {
	SENTINEL, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };


//===========================================================
//
// Fix #1: Index 12 - 4x3 Foundation
/*
BEFORE:
	 -1  0  1  2  3
 -1   B  B  B  B
  0  B  █  █  █  B
  1  B  █  █  █  B
  2  B  █  █  █  B
  3   B  B  B  B

AFTER:
	 -1  0  1  2  3  4
 -1   B  B  B  B  B
  0  B  █  █  █  █  B
  1  B  █  █  █  █  B
  2  B  █  █  █  █  B
  3   B  B  B  B  B
*/
static COMPILETIMEEVAL FoundationStruct FoundationOutlines_12_Fixed = { {
	{0, 3}, {1, 3}, {2, 3}, {-1, 3}, {3, 3},
	{-1, 2}, {4, 2}, {-1, 1}, {4, 1}, {-1, 0},
	{4, 0}, {0, -1}, {1, -1}, {2, -1}, {-1, -1},
	{3, -1}, {4, -1}, {4, 3}, SENTINEL, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

// Fix #2: Index 15 - 2x6 Foundation (3x7 with gap)
/*
BEFORE:
	 -1  0  1  2
 -1   B  B  B  B
  0  B  █  █  B
  1  B  █  █  B
  2  B  █  █  B
  3  B  █  █  B
  4  ?        ?
  5  B  █  █  B
  6   B  B  B  B

AFTER:
	 -1  0  1  2
 -1   B  B  B  B
  0  B  █  █  B
  1  B  █  █  B
  2  B  █  █  B
  3  B  █  █  B
  4  B        B
  5  B  █  █  B
  6   B  B  B  B
*/
static COMPILETIMEEVAL FoundationStruct FoundationOutlines_15_Fixed = { {
	{-1, -1}, {0, -1}, {1, -1}, {2, -1}, {-1, 0},
	{2, 0}, {-1, 1}, {2, 1}, {-1, 2}, {2, 2},
	{-1, 3}, {2, 3}, {-1, 5}, {2, 5}, {-1, 6},
	{0, 6}, {1, 6}, {2, 6}, {-1, 4}, {2, 4},
	SENTINEL, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

// Fix #3: Index 16 - 2x5 Foundation (3x6 with gap)
/*
BEFORE:
	 -1  0  1  2
 -1   B  B  B  B
  0  B  █  █  B
  1  B  █  █  B
  2  B  █  █  B
  3  B  █  █  B
  4  ?        ?
  5   B  B  B  B

AFTER:
	 -1  0  1  2
 -1   B  B  B  B
  0  B  █  █  B
  1  B  █  █  B
  2  B  █  █  B
  3  B  █  █  B
  4  B        B
  5   B  B  B  B
*/
static COMPILETIMEEVAL FoundationStruct FoundationOutlines_16_Fixed = { {
	{-1, -1}, {0, -1}, {1, -1}, {2, -1}, {-1, 0},
	{2, 0}, {-1, 1}, {2, 1}, {-1, 2}, {2, 2},
	{-1, 3}, {2, 3}, {-1, 5}, {0, 5}, {1, 5},
	{2, 5}, {-1, 4}, {2, 4}, SENTINEL, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

// Fix #4: Index 18 - 4x4 Foundation
/*
BEFORE:
	 -1  0  1  2  3  4
 -1   B  B  B  B  B
  0  B  █  █  █  █  B
  1  B  █  █  █  █  B
  2  B  █  █  █  █  B
  3  B  █  █  █  █  B
  4   B  B  B  B  B  B
	  B  (duplicate)

AFTER:
	 -1  0  1  2  3  4
 -1   B  B  B  B  B
  0  B  █  █  █  █  B
  1  B  █  █  █  █  B
  2  B  █  █  █  █  B
  3  B  █  █  █  █  B
  4   B  B  B  B  B  B
*/
static COMPILETIMEEVAL FoundationStruct FoundationOutlines_18_Fixed = { {
	{-1, -1}, {0, -1}, {1, -1}, {2, -1}, {3, -1},
	{4, -1}, {-1, 4}, {0, 4}, {1, 4}, {2, 4},
	{3, 4}, {4, 4}, {-1, 0}, {-1, 1}, {-1, 2},
	{-1, 3}, {4, 3}, {4, 0}, {4, 1}, {4, 2},
	SENTINEL, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

// Fix #5: Index 19 - 3x4 Foundation
/*
BEFORE:
	 -1  0  1  2  3
 -1   B  B  B  B
  0  B  █  █  █  B
  1  B  █  █  █  B
  2  B  █  █  █  B
  3  ?           ?
  4   B  B  B  B

AFTER:
	 -1  0  1  2  3
 -1   B  B  B  B  B
  0  B  █  █  █  B
  1  B  █  █  █  B
  2  B  █  █  █  B
  3  B           B
  4   B  B  B  B  B
*/
static COMPILETIMEEVAL FoundationStruct FoundationOutlines_19_Fixed = { {
	{0, 4}, {1, 4}, {2, 4}, {-1, 4}, {3, 4},
	{-1, 2}, {3, 2}, {-1, 1}, {3, 1}, {-1, 0},
	{3, 0}, {0, -1}, {1, -1}, {2, -1}, {-1, -1},
	{3, -1}, {-1, 3}, {3, 3}, SENTINEL, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

// Fix #6: Index 20 - 6x4 Foundation
/*
BEFORE:
	 Only (2,-1) and sentinel - COMPLETELY BROKEN

AFTER:
	 -1  0  1  2  3  4  5  6
 -1   B  B  B  B  B  B  B  B
  0  B  █  █  █  █  █  █  B
  1  B  █  █  █  █  █  █  B
  2  B  █  █  █  █  █  █  B
  3  B  █  █  █  █  █  █  B
  4   B  B  B  B  B  B  B  B
*/
static COMPILETIMEEVAL FoundationStruct FoundationOutlines_20_Fixed = { {
	{2, -1}, {-1, -1}, {0, -1}, {1, -1}, {3, -1},
	{4, -1}, {5, -1}, {6, -1}, {-1, 0}, {6, 0},
	{-1, 1}, {6, 1}, {-1, 2}, {6, 2}, {-1, 3},
	{6, 3}, {-1, 4}, {0, 4}, {1, 4}, {2, 4},
	{3, 4}, {4, 4}, {5, 4}, {6, 4}, SENTINEL,
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
} };

std::array<FoundationStruct, 22> FoundationDataStruct::Outlines = { {
	FoundationOutlines_0, FoundationOutlines_1, FoundationOutlines_2,
	FoundationOutlines_3, FoundationOutlines_4, FoundationOutlines_5,
	FoundationOutlines_6, FoundationOutlines_7, FoundationOutlines_8,
	FoundationOutlines_9, FoundationOutlines_10, FoundationOutlines_11,
	FoundationOutlines_12_Fixed, FoundationOutlines_13, FoundationOutlines_14,
	FoundationOutlines_15_Fixed, FoundationOutlines_16_Fixed, FoundationOutlines_17,
	FoundationOutlines_18_Fixed, FoundationOutlines_19_Fixed, FoundationOutlines_20_Fixed,
	FoundationOutlines_21
} };