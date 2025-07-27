#pragma once


#include <Utilities/TemplateDef.h>

class SuperWeaponTypeClass;
struct LauchSWData final
{
	int LaunchWhat { -1 };
	Valueable<bool> LaunchWaitcharge { true };
	Valueable<bool> LaunchResetCharge { true };
	Valueable<bool> LaunchGrant { false };
	Valueable<bool> LaunchGrant_RepaintSidebar { true };
	Valueable<bool> LaunchGrant_OneTime { true };
	Valueable<bool> LaunchGrant_OnHold { false };
	Valueable<bool> LaunchSW_Manual { false };
	Valueable<bool> LaunchSW_IgnoreInhibitors { false };
	Valueable<bool> LaunchSW_IgnoreDesignators { true };
	Valueable<bool> LauchSW_IgnoreMoney { false };
	Valueable<bool> LauchSW_IgnoreBattleData { false };
	Valueable<bool> LaunchSW_DisplayMoney { false };
	Valueable<bool> LaunchSW_RealLauch { true };
	Valueable<AffectedHouse> LaunchSW_DisplayMoney_Houses { AffectedHouse::All };
	Valueable<Point2D> LaunchSW_DisplayMoney_Offset { {  0 , 0  } };

	Valueable<OwnerHouseKind> LauchhSW_Owner { OwnerHouseKind::Invoker };

	bool Read(INI_EX& exINI, const char* pID, int Prefix , SuperWeaponTypeClass* pReaded);
	bool ReadSingle(INI_EX& exINI, const char* pID, int Prefix);
	static bool ReadVector(std::vector<LauchSWData>& res , INI_EX& exINI, const char* pSection , bool CompatibilityMode);
	//LauchSWData() = default;
	//LauchSWData(INI_EX& exINI, const char* pID, int Prefix, SuperWeaponTypeClass* pReaded)
	//{
	//	this->Read(exINI, pID, Prefix, pReaded);
	//}

	//~LauchSWData() = default;

	//LauchSWData(const LauchSWData& other) = default;
	//LauchSWData(LauchSWData&& other) = default;
	//LauchSWData& operator=(const LauchSWData& other) = default;
	//LauchSWData& operator=(LauchSWData&& value) = default;

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

	template <typename T>
	bool Serialize(T& Stm);
};
