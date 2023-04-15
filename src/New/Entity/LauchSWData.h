#pragma once


#include <Utilities/TemplateDef.h>

class SuperWeaponTypeClass;
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

	bool LaunchSW_DisplayMoney;
	AffectedHouse LaunchSW_DisplayMoney_Houses;
	Point2D LaunchSW_DisplayMoney_Offset;

	bool Read(INI_EX& exINI, const char* pID, int Prefix , SuperWeaponTypeClass* pReaded);

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

		, LaunchSW_DisplayMoney { false }
		, LaunchSW_DisplayMoney_Houses { AffectedHouse::All }
		, LaunchSW_DisplayMoney_Offset {  0 , 0  }
	{ }

	virtual ~LauchSWData() = default;
	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

	template <typename T>
	bool Serialize(T& Stm);
};
