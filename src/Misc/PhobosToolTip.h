#pragma once

#include <Phobos.h>

#include <string>

struct StripClass;
class TechnoTypeClass;
class SuperClass;
struct BuildType;

class PhobosToolTip
{
public:
	static PhobosToolTip Instance;

private:
	inline int GetBuildTime(TechnoTypeClass* pType) const;
	inline int GetPower(TechnoTypeClass* pType) const;
	static int TickTimeToSeconds(int tickTime);

public:
	constexpr FORCEINLINE bool IsEnabled() const {
		return Phobos::UI::ExtendedToolTips;
	}

	constexpr FORCEINLINE const wchar_t* GetBuffer() const
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