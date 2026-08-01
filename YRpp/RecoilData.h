#pragma once

#include <ASMMacros.h>
#include <GeneralDefinitions.h>
#include <TurretControl.h>

struct RecoilData
{
	enum class RecoilState : unsigned int
	{
		Inactive = 0,
		Compressing = 1,
		Holding = 2,
		Recovering = 3,
	};

	TurretControl Turret;
	float TravelPerFrame;
	float TravelSoFar;
	RecoilState State;
	int TravelFramesLeft;

	void Update()
	{ JMP_THIS(0x70ED10); }

	void Fire()
	{ JMP_THIS(0x70ECE0); }
};