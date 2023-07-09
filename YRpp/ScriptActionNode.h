#pragma once

#include <ASMMacros.h>

struct ScriptActionNode
{
public:
	unsigned int BuildINIEntry(char* pTr)
	{ JMP_THIS(0x723CE0); }

	ScriptActionNode(const char* pTr) noexcept
	{ JMP_THIS(0x723CA0); }

	ScriptActionNode(int Act, int Arg) noexcept :
		Action { Act }, Argument { Arg }
	{
	}

	ScriptActionNode() noexcept = default;
	~ScriptActionNode() noexcept = default;

	bool operator==(ScriptActionNode const& rhs) const
	{
		//return Action == rhs.Action && Argument == rhs.Argument;
		return false; // so , umm we dont really care actually ,.. but , eh whatever
	}

	bool operator!=(ScriptActionNode const& rhs) const
	{
		//return !((*this) == rhs);
		return true; // so , umm we dont really care actually ,.. but , eh whatever
	}

public:
	TeamMissionType Action { TeamMissionType::none };
	int Argument { 0 };
};