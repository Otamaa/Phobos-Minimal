#pragma once
#include <CellStruct.h>
#include <Point2D.h>

struct MapPathCellElement
{

	int Distance { -1 };
	int X { -1 };
	int Y { -1 };

public :

	MapPathCellElement() noexcept  = default;
	MapPathCellElement(int nDistance, short x, short y) noexcept
		: Distance { nDistance }, X {x} , Y {y}
	{ }

	//need to define a == operator so it can be used in array classes
	bool operator==(const MapPathCellElement& other) const {
		return (X == other.X && Y == other.Y);
	}

	//unequality
	bool operator!=(const MapPathCellElement& other) const {
		return (X != other.X || Y != other.Y);
	}

	bool operator<(const MapPathCellElement& other) const {
		return (Distance < other.Distance);
	}

	bool operator>(const MapPathCellElement& other) const {
		return (Distance > other.Distance);
	}

	inline CellStruct ToCellStruct() const {
		return { (short)X  ,(short)Y };
	}

	inline Point2D ToPoints() const {
		return { X  ,Y };
	}

};