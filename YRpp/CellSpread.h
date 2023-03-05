#pragma once

#include <GeneralStructures.h>
#include <Unsorted.h>

class CellSpread
{
public:
	// between -256 -> 256
	static constexpr reference<CellStruct, 0x89F6D8, 8u> const AdjacentCoord{};
	// between -1 -> 1
	static constexpr reference<CellStruct, 0x89F688, 8u> const AdjacentCell{};
	static constexpr reference<size_t, 0x7ED3D0, 12u> const CellNums{};
	static constexpr reference<CellStruct, 0xABD490, 369u> const CellOfssets{};

	static size_t NumCells(size_t nSpread) {
		if (nSpread > CellNums.size()) {
			return nSpread * 256;
		}
		return CellNums[nSpread];
	}

	static size_t NumCellsFromFloat(float nSpread) {
		return Game::F2I((nSpread + 0.99));
	}

	static const CellStruct GetCell(size_t n) {
		if (n > CellOfssets.size()) {
			return CellStruct::Empty;
		}

		return CellOfssets[n];
	}

	static const CellStruct GetNeighbourOffset(size_t direction) {
		if(direction > AdjacentCoord.size()) {
			return CellStruct::Empty;
		}

		return AdjacentCoord[direction];
	}

	static size_t GetDistance(int dx, int dy) {
		auto x = static_cast<size_t>(std::abs(dx));
		auto y = static_cast<size_t>(std::abs(dy));

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
