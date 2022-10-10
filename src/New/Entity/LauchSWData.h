#pragma once


#include <Utilities/TemplateDef.h>

struct LauchSWData final
{
	int LaunchWhat;
	bool LaunchWaitcharge;
	bool LaunchResetCharge;
	bool LaunchGrant;
	bool LaunchGrant_RepaintSidebar;
	bool LaunchGrant_OneTime;
	bool LaunchGrant_OnHold;
	bool LaunchSW_Manual;
	bool LaunchSW_IgnoreInhibitors;
	bool LaunchSW_IgnoreDesignators;
	bool LauchSW_IgnoreMoney;

	bool Read(INI_EX& exINI, const char* pID, int Prefix);

	LauchSWData() : LaunchWhat { -1 }
		, LaunchWaitcharge { false }
		, LaunchResetCharge { false }
		, LaunchGrant { false }
		, LaunchGrant_RepaintSidebar { true }
		, LaunchGrant_OneTime { true }
		, LaunchGrant_OnHold { false }
		, LaunchSW_Manual { false }
		, LaunchSW_IgnoreInhibitors { false }
		, LaunchSW_IgnoreDesignators { true }
		, LauchSW_IgnoreMoney { false }
	{ }

	virtual ~LauchSWData() = default;
	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

	template <typename T>
	bool Serialize(T& Stm);
};
