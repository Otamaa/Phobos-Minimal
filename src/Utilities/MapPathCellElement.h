#pragma once
#include <CellStruct.h>
#include <Point2D.h>

class MapPathCellElement
{
	int Distance { -1 };
	Point2D Points { -1 , -1 };

public:

	//need to define a == operator so it can be used in array classes
	bool operator==(const MapPathCellElement& other) const {
		return (Points == other.Points);
	}

	//unequality
	bool operator!=(const MapPathCellElement& other) const {
		return (Points != other.Points);
	}

	bool operator<(const MapPathCellElement& other) const {
		return (Distance < other.Distance);
	}

	bool operator>(const MapPathCellElement& other) const {
		return (Distance > other.Distance);
	}

	CellStruct ToCellStruct() const {
		return { (short)Points.X  ,(short)Points.Y };
	}
};