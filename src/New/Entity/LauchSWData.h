#pragma once

#include <Utilities/TemplateDef.h>

struct LauchSWData
{
	SuperWeaponTypeClass* LaunchWhat;
	bool LaunchWaitcharge;
	bool LaunchResetCharge;
	bool LaunchGrant;
	bool LaunchGrant_RepaintSidebar;
	bool LaunchGrant_OneTime;
	bool LaunchGrant_OnHold;
	bool LaunchSW_Manual;
	bool LaunchSW_IgnoreInhibitors;
	bool LauchSW_IgnoreMoney;

	bool Read(INI_EX& exINI, const char* pID, int Prefix);

	LauchSWData() : LaunchWhat { nullptr }
		, LaunchWaitcharge { false }
		, LaunchResetCharge { false }
		, LaunchGrant { false }
		, LaunchGrant_RepaintSidebar { true }
		, LaunchGrant_OneTime { true }
		, LaunchGrant_OnHold { false }
		, LaunchSW_Manual { false }
		, LaunchSW_IgnoreInhibitors { false }
		, LauchSW_IgnoreMoney { false }
	{ }

	template <typename T>
	void Serialize(T& Stm);
};
