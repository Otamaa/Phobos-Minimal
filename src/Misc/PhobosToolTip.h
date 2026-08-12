#pragma once


#include <string>

struct StripClass;
class TechnoTypeClass;
class SuperClass;
struct BuildType;
class HouseClass;
class TechnoTypeExtData;
class SWTypeExtData;
class PhobosToolTip
{
public:
	static PhobosToolTip Instance;

public:
	static int GetBuildTime(TechnoTypeClass* pType);
	static int GetBuildTime(TechnoTypeClass* pType, HouseClass* pOwner);
	static int GetPower(TechnoTypeClass* pType);
	static int TickTimeToSeconds(int tickTime);

private:

	OPTIONALINLINE const wchar_t* GetUIDescription(TechnoTypeExtData* pData) const;
	OPTIONALINLINE const wchar_t* GetUnbuildableUIDescription(TechnoTypeExtData* pData) const;
	OPTIONALINLINE const wchar_t* GetUIDescription(SWTypeExtData* pData) const;

public:

	COMPILETIMEEVAL FORCEDINLINE const wchar_t* GetBuffer() const
	{
		return this->TextBuffer.c_str();
	}

	void HelpText(const BuildType* cameo);
	void HelpText(TechnoTypeClass* pType);
	void HelpText(SuperClass* pSuper);

	// Properties
public:
	std::wstring TextBuffer {};
	bool IsCameo { false };
	bool SlaveDraw { false };
};