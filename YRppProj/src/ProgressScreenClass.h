#pragma once

#include <GeneralDefinitions.h>
#include <Helpers/CompileTime.h>
#include <Interface/IDontKnow.h>

class LoadProgressManager;
struct SHPStruct;
class ALIGN(4) NOVTABLE ProgressScreenClass  : public INoticeSource {
public:

	static constexpr reference<ProgressScreenClass, 0xAC4F58u> const Instance{};

	virtual void SetLoadManager(void* mngr);//643E80
	virtual void DeInt();

	ProgressScreenClass(); //6429E0
	virtual ~ProgressScreenClass();

	void SetSide(int idx)
		;// { JMP_THIS(0x642B10); }

	int GetSide()
		;//{ JMP_THIS(0x642B20); }


public:

	LoadProgressManager *LoadManager;
	double PlayerProgresses[8];
	int MainProgress;
	int field_4C;
	DWORD PlayerStartSpot; // bah, I have multiple definitions of this in my IDB, can't be bothered to fix it now
	SHPStruct *someSHP;
	char field_58;
	char field_59;
	char field_5A;
	char field_5B;
	int field_5C;
	char field_60;
	byte TotalPlayers;
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
};
static_assert(sizeof(ProgressScreenClass) == 0x84 , "Invalid Size!");