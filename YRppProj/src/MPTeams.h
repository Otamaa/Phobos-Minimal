#pragma once

// these classes handle alliances between players, eg Team:A B C D in the frontend
#include <Base/Always.h>

class MPTeam
{
public:

	MPTeam(wchar_t** title, int idx);
	virtual ~MPTeam();

	virtual bool IsTeamIncluded(int idx);//JMP_THIS(0x5D8C90);
	virtual bool SetPlayerTeam(int idxPlayer);//JMP_THIS(0x5D8CB0);

	void AddToList(HWND hWnd)
		;//{ JMP_THIS(0x5D8D10); }

public:
	wchar_t* Title;
	int Index;
};

class MPCombatTeam : public MPTeam
{
public:
	//Destructor
	virtual ~MPCombatTeam() RX;
};


class MPSiegeDefenderTeam : public MPTeam
{
public:
	//Destructor
	virtual ~MPSiegeDefenderTeam();
	virtual bool IsTeamIncluded(int idx);//JMP_THIS(0x5CAE70);

	MPSiegeDefenderTeam();
};


class MPSiegeAttackerTeam : public MPTeam
{
public:

	virtual ~MPSiegeAttackerTeam();
	MPSiegeAttackerTeam();
};


class MPObserverTeam : public MPTeam
{
public:

	virtual ~MPObserverTeam();
	virtual bool IsTeamIncluded(int idx) { return this->MPTeam::IsTeamIncluded(idx); }

	MPObserverTeam();
};
