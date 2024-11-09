/*
	Radar notifications
*/

#pragma once

#include <GeneralDefinitions.h>
#include <Helpers/CompileTime.h>

struct RadarEventData
{
	int SuppressionDistance;
	int VisibilityDuration;
	int Duration;
	bool CanCreate;
};

class RadarEventClass
{
public:
	static constexpr constant_ptr<DynamicVectorClass<RadarEventClass*>, 0xB04DA8u> const Array {};
	static constexpr reference<RadarEventData ,0x7F0998u , 17u> const RadarEventInfo {};
	static constexpr reference<CellStruct, 0xB04D48, 8u> const LastRadarEventCells {};
	static constexpr reference<int , 0xB04DD8> const LastRadarEventIndex {};

	static bool Create(RadarEventType nType, CellStruct nMapCoords)
		;//{ PUSH_VAR32(nMapCoords); SET_REG32(ecx, nType); ASM_CALL(0x65FA70); }

	static void __fastcall Create(const CellStruct& nMapCoords)
		;//{ JMP_STD(0x660B80); }

	static void __fastcall Create(CellStruct* pMapCoords)
		;//{ JMP_STD(0x660B80); }


	RadarEventClass(int nType, CellStruct nMapCoords);
	~RadarEventClass();

public:

	RadarEventType Type;
	int RadarX;	//not sure
	int RadarY;	//not sure
	float Speed;
	float RotationValue;  //0.78539819
	float RotationSpeed; //Rule->RadarEventRotationSpeed
	float ColorValue;	//0.0
	float ColorSpeed; //Rule->RadarEventColorSpeed
	CellStruct MapCoords;
	CDTimerClass DurationTimer;	//Rotation timer?
	CDTimerClass VisibilityTimer;	//Color timer?
	bool Rotating;
	bool Visible;
};

static_assert(sizeof(RadarEventClass) == 0x40u, "Invalid size.");