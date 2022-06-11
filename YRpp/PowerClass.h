#pragma once

#include <RadarClass.h>

class NOVTABLE PowerClass : public RadarClass
{
public:
	//Static
	static constexpr constant_ptr<PowerClass, 0x87F7E8u> const Instance{};

	//Destructor
	virtual ~PowerClass() RX;

	//Non-virtual

protected:
	//Constructor
	PowerClass() {}	//don't need this

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:
	bool IsToRedraw; //150C
	PROTECTED_PROPERTY(BYTE, align_150D[3])
	TimerStruct _TopBarBlinkDelayTimer; //1510
	DWORD _TopBarBlinkCountdown; //151C
	TimerStruct _BarUpdateTimer; //1520
	DWORD _GreenBars;  //152C
	DWORD _YellowBars; //1530
	DWORD _RedBars; //1534
	bool _BarsAreUpdating; //1538
	PROTECTED_PROPERTY(BYTE, align_1539[3])
	int PowerOutput;
	int PowerDrain;
};
