#pragma once
#include <CellStruct.h>
#include <Point2D.h>

struct MapPathCellElement
{
	int Distance { -1 };
	int X { -1 };
	int Y { -1 };

public :

	COMPILETIMEEVAL MapPathCellElement() noexcept  = default;
	COMPILETIMEEVAL MapPathCellElement(int nDistance, short x, short y) noexcept
		: Distance { nDistance }, X {x} , Y {y}
	{ }

	COMPILETIMEEVAL MapPathCellElement(const MapPathCellElement& other) = default;
	COMPILETIMEEVAL MapPathCellElement& operator=(const MapPathCellElement& other) = default;
	COMPILETIMEEVAL ~MapPathCellElement() = default;

	//need to define a == operator so it can be used in array classes
	COMPILETIMEEVAL bool operator==(const MapPathCellElement& other) const {
		return (X == other.X && Y == other.Y);
	}

	//unequality
	COMPILETIMEEVAL bool operator!=(const MapPathCellElement& other) const {
		return (X != other.X || Y != other.Y);
	}

	COMPILETIMEEVAL bool operator<(const MapPathCellElement& other) const {
		return (Distance < other.Distance);
	}

	COMPILETIMEEVAL bool operator>(const MapPathCellElement& other) const {
		return (Distance > other.Distance);
	}

	COMPILETIMEEVAL OPTIONALINLINE CellStruct ToCellStruct() const {
		return { (short)X  ,(short)Y };
	}

	COMPILETIMEEVAL OPTIONALINLINE Point2D ToPoints() const {
		return { X  ,Y };
	}

};