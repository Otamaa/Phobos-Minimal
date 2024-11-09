#pragma once

#include <YRPPCore.h>
#include <Helpers/CompileTime.h>

class CDDriveManagerClass
{
public:
	//Static
	static constexpr reference<CDDriveManagerClass*, 0x89E414u> const Instance_unknown {}; //???
	static constexpr reference<CDDriveManagerClass, 0xA8E8E8> const Instance {};

protected:
	//CTOR
	CDDriveManagerClass();

public:
	/*
	Retrieves the number of the currently inserted disc
	0 = RA2 Allied,
	1 = RA2 Soviet,
	2 = YR
	*/
	int GetCDNumber()
		;//{ JMP_THIS(0x4A80D0); }

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

	virtual bool ForceAvailable(int cdNumber);// JMP_THIS(0x4790E0);
	virtual bool InsertCDDialog();// JMP_THIS(0x479110);
	virtual void SwapToDisk();// JMP_THIS(0x4791F0);

public:

	DWORD unknown_04;

//protected:
//	CD() RX;
};
