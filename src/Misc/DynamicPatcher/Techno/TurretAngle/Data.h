#pragma once

#include <Utilities/TemplateDefB.h>

enum class DeathZoneAction : size_t
{
	CLEAR, BLOCK, TURN
};

struct TurretAngleData
{
	bool Enable;

	int DefaultAngle;

	bool AngleLimit;
	Point2D Angle;
	DeathZoneAction Action;

	bool AutoTurn;
	Point2D SideboardAngleL;
	Point2D SideboardAngleR;

	static int GetTurnAngle(int targetBodyDelta, int min, int max)
	{
		int turnAngle = 0;
		int length = max - min;
		if ((targetBodyDelta - min) < (length / 2))
		{
			turnAngle = min;
		}
		else
		{
			turnAngle = max;
		}
		return turnAngle;
	}
};