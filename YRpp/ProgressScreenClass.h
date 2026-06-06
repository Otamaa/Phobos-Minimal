#pragma once

#include <GeneralDefinitions.h>
#include <Helpers/CompileTime.h>

class LoadProgressManager;
struct SHPStruct;
class ALIGN(4) NOVTABLE ProgressScreenClass {
public:

	virtual void SetLoadmanager(LoadProgressManager* pManager) RX;

	static COMPILETIMEEVAL reference<ProgressScreenClass, 0xAC4F58u> const Instance{};

	void SetSide(int idx)
		{ JMP_THIS(0x642B10); }

	int GetSide()
		{ JMP_THIS(0x642B20); }

	void  Init(double progress, char playercount, HWND hwnd) {
		JMP_THIS(0x642A60);
	}

public:

	LoadProgressManager *LoadManager;
	double PlayerProgresses[8];
	double MainProgress;
	int field_4C;
	void *PlayerStartSpot; // bah, I have multiple definitions of this in my IDB, can't be bothered to fix it now
	SHPStruct *someSHP;
	char field_58;
	char field_59;
	char field_5A;
	char field_5B;
	int field_5C;
	char field_60;
	char TotalPlayers;
	char field_62;
	char field_63;
	HWND hWnd;
	int field_68;
	int field_6C;
	char field_70;
	char field_71;
	char field_72;
	char field_73;
	int field_74;
	int field_78;
	int field_7C;
	int PlayerSide; // !! this is set to campaign -> CD for singleplay

protected:
	ProgressScreenClass() = default;
	~ProgressScreenClass()
	{
		if (field_58) {
			GameDelete(someSHP);
			field_58 = 0;
		}

		someSHP = 0;
	}
};
//idk , this tripped 
//static_assert(sizeof(ProgressScreenClass) == 0x84 , "Invalid Size!");