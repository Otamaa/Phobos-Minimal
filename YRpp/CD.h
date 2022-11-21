#pragma once

#include <YRPPCore.h>

class CDDriveManagerClass
{
public:
	//Static
	static CDDriveManagerClass* Global()
		{ return *((CDDriveManagerClass**)0x89E414); }

protected:
	//CTOR
	CDDriveManagerClass()
		{ JMP_THIS(0x4E6070); }

public:
	/*
	Retrieves the number of the currently inserted disc
	0 = RA2 Allied,
	1 = RA2 Soviet,
	2 = YR
	*/
	int GetCDNumber()
		{ JMP_THIS(0x4A80D0); }

	//Properties

public:

	int CDDriveNames [26]; //int + 'A' would be the drive's name
	int NumCDDrives;
	DWORD unknown_6C;
};

class CD
{
public:
	static constexpr reference<int, 0x81C1D0> const Disk { };
	static constexpr reference<bool, 0x89E3A0> const IsLocal { };


	virtual bool ForceAvailable(int disk) R0;
	virtual bool InsertCDDialog(int disk) R0;
	virtual void SwapToDisk(int disk) RX;

public:

	DWORD unknown_04;

protected:
	CD() RX;
};
