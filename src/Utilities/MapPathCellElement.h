#pragma once
#include <CellStruct.h>
#include <Point2D.h>

struct MapPathCellElement
{
	int Distance { -1 };
	int X { -1 };
	int Y { -1 };

public :

	constexpr MapPathCellElement() noexcept  = default;
	constexpr MapPathCellElement(int nDistance, short x, short y) noexcept
		: Distance { nDistance }, X {x} , Y {y}
	{ }

	constexpr MapPathCellElement(const MapPathCellElement& other) = default;
	constexpr MapPathCellElement& operator=(const MapPathCellElement& other) = default;
	constexpr ~MapPathCellElement() = default;

	//need to define a == operator so it can be used in array classes
	constexpr bool operator==(const MapPathCellElement& other) const {
		return (X == other.X && Y == other.Y);
	}

	//unequality
	constexpr bool operator!=(const MapPathCellElement& other) const {
		return (X != other.X || Y != other.Y);
	}

	constexpr bool operator<(const MapPathCellElement& other) const {
		return (Distance < other.Distance);
	}

	constexpr bool operator>(const MapPathCellElement& other) const {
		return (Distance > other.Distance);
	}

	constexpr inline CellStruct ToCellStruct() const {
		return { (short)X  ,(short)Y };
	}

	constexpr inline Point2D ToPoints() const {
		return { X  ,Y };
	}

};