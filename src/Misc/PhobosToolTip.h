#pragma once

#include <SidebarClass.h>
#include <SuperWeaponTypeClass.h>
#include <TechnoTypeClass.h>

#include <Phobos.h>

#include <string>

struct StripClass;

class PhobosToolTip
{
public:
	static PhobosToolTip Instance;

private:
	inline int GetBuildTime(TechnoTypeClass* pType) const;
	inline int GetPower(TechnoTypeClass* pType) const;
	static int TickTimeToSeconds(int tickTime);
public:
	inline bool IsEnabled() const;
	constexpr inline const wchar_t* GetBuffer() const
	{
		return this->TextBuffer.c_str();
	}

	void HelpText(const BuildType& cameo);
	void HelpText(TechnoTypeClass* pType);
	void HelpText(SuperClass* pSuper);
	// Properties
public:
	std::wstring TextBuffer {};

public:
	bool IsCameo { false };
	bool SlaveDraw { false };
};