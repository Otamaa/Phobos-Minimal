#pragma once


#include <Utilities/TemplateDef.h>

class SuperWeaponTypeClass;
struct LauchSWData final
{
	int LaunchWhat;
	Valueable<bool> LaunchWaitcharge;
	Valueable<bool> LaunchResetCharge;
	Valueable<bool> LaunchGrant;
	Valueable<bool> LaunchGrant_RepaintSidebar;
	Valueable<bool> LaunchGrant_OneTime;
	Valueable<bool> LaunchGrant_OnHold;
	Valueable<bool> LaunchSW_Manual;
	Valueable<bool> LaunchSW_IgnoreInhibitors;
	Valueable<bool> LaunchSW_IgnoreDesignators;
	Valueable<bool> LauchSW_IgnoreMoney;

	Valueable<bool> LaunchSW_DisplayMoney;
	Valueable<AffectedHouse> LaunchSW_DisplayMoney_Houses;
	Valueable<Point2D> LaunchSW_DisplayMoney_Offset;

	Valueable<OwnerHouseKind> LauchhSW_Owner;

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
		, LaunchSW_DisplayMoney_Offset { {  0 , 0  } }
		, LauchhSW_Owner { OwnerHouseKind::Invoker }
	{ }

	virtual ~LauchSWData() = default;

	LauchSWData(const LauchSWData& other) = default;
	LauchSWData& operator=(const LauchSWData& other) = default;

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

	template <typename T>
	bool Serialize(T& Stm);
};
