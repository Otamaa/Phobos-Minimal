#pragma once

#include <Base/Always.h>
#include <TechnoClass.h>

struct WeaponPacks
{
	BYTE idxSlot_Wave;
	BYTE idxSlot_Beam;
	BYTE idxSlot_Warp;
	BYTE idxSlot_Parasite;
	BYTE idxSlot_5;
	BYTE idxSlot_6;
	BYTE idxSlot_7;
	BYTE idxSlot_8;
};

struct AresTechnoExtData
{
	TechnoClass* AttachedObject;
	DWORD dword4;
	BYTE idxSlot_Wave;
	BYTE idxSlot_Beam;
	BYTE idxSlot_Warp;
	BYTE idxSlot_Parasite;
	BuildingClass* GarrisonedIn;
	DWORD dword10;
	Mission EMPLastMission;
	DWORD dword18;
	DWORD dword1C;
	DWORD dword20;
	DWORD dword24;
	DWORD dword28;
	DWORD dword2C;
	WORD word30;
	BYTE gap32[2];
	DWORD dword34;
	DWORD dword38;
	DWORD dword3C;
	DWORD dword40;
	DWORD dword44;
	DWORD dword48;
	DWORD dword4C;
	DWORD dword50;
	DWORD dword54;
	DWORD dword58;
	DWORD dword5C;
	DWORD dword60;
	DWORD dword64;
	DWORD dword68;
	DWORD dword6C;
	DWORD dword70;
	BYTE gap74[4];
	double ROFMult;
	double FirePowerMult;
	double ArmorMult;
	double SpeedMult;
	BYTE byte98;
	BYTE word99;
	BYTE word9A;
	BYTE Survivors_Done;
	BYTE DriverKilled;
	BYTE gap9D;
	BYTE InitialPayloadCreated;
	DWORD dwordA0;
	DWORD dwordA4;
	DWORD dwordA8;
	DWORD dwordAC;
	BYTE byteB0;
};