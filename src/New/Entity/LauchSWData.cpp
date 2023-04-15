#include "LauchSWData.h"

#include <SuperWeaponTypeClass.h>

bool LauchSWData::Read(INI_EX& exINI, const char* pID, int Prefix, SuperWeaponTypeClass* pReaded)
{
	char nBuff[0x80];
	LaunchWhat = pReaded->ArrayIndex;

	Valueable<bool> bool_Dummy { LaunchWaitcharge };
	IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "LaunchSW%d.WaitForCharge", Prefix);
	bool_Dummy.Read(exINI, pID, nBuff);
	LaunchWaitcharge = bool_Dummy.Get();

	bool_Dummy = LaunchResetCharge;
	IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "LaunchSW%d.ResetCharge", Prefix);
	bool_Dummy.Read(exINI, pID, nBuff);
	LaunchResetCharge = bool_Dummy.Get();

	bool_Dummy = LaunchGrant;
	IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "LaunchSW%d.Grant", Prefix);
	bool_Dummy.Read(exINI, pID, nBuff);
	LaunchGrant = bool_Dummy.Get();

	bool_Dummy = LaunchGrant_RepaintSidebar;
	IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "LaunchSW%d.Grant.RepaintSidebar", Prefix);
	bool_Dummy.Read(exINI, pID, nBuff);
	LaunchGrant_RepaintSidebar = bool_Dummy.Get();

	bool_Dummy = LaunchGrant_OneTime;
	IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "LaunchSW%d.Grant.OneTime", Prefix);
	bool_Dummy.Read(exINI, pID, nBuff);
	LaunchGrant_OneTime = bool_Dummy.Get();

	bool_Dummy = LaunchGrant_OnHold;
	IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "LaunchSW%d.Grant.OnHold", Prefix);
	bool_Dummy.Read(exINI, pID, nBuff);
	LaunchGrant_OnHold = bool_Dummy.Get();

	bool_Dummy = LaunchSW_Manual;
	IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "LaunchSW%d.Manual", Prefix);
	bool_Dummy.Read(exINI, pID, nBuff);
	LaunchSW_Manual = bool_Dummy.Get();

	bool_Dummy = LaunchSW_IgnoreInhibitors;
	IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "LaunchSW%d.IgnoreInhibitors", Prefix);
	bool_Dummy.Read(exINI, pID, nBuff);
	LaunchSW_IgnoreInhibitors = bool_Dummy.Get();

	bool_Dummy =  LaunchSW_IgnoreDesignators;
	IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "LaunchSW%d.IgnoreDesignators", Prefix);
	bool_Dummy.Read(exINI, pID, nBuff);
	LaunchSW_IgnoreDesignators = bool_Dummy.Get();

	bool_Dummy = LauchSW_IgnoreMoney;
	IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "LaunchSW%d.IgnoreMoney", Prefix);
	bool_Dummy.Read(exINI, pID, nBuff);
	LauchSW_IgnoreMoney = bool_Dummy.Get();

	bool_Dummy = LaunchSW_DisplayMoney;
	IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "LaunchSW%d.DisplayMoney", Prefix);
	bool_Dummy.Read(exINI, pID, nBuff);
	LaunchSW_DisplayMoney = bool_Dummy.Get();

	Valueable<AffectedHouse> dummy_AffectHouse { LaunchSW_DisplayMoney_Houses };
	IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "LaunchSW%d.DisplayMoney.Houses", Prefix);
	dummy_AffectHouse.Read(exINI, pID, nBuff);
	LaunchSW_DisplayMoney_Houses = dummy_AffectHouse;

	Valueable<Point2D> dummy_DisplayOffs { LaunchSW_DisplayMoney_Offset };
	IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "LaunchSW%d.DisplayMoney.Offset", Prefix);
	dummy_DisplayOffs.Read(exINI, pID, nBuff);
	LaunchSW_DisplayMoney_Offset = dummy_DisplayOffs;

	return true;
}

bool LauchSWData::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Serialize(Stm);
}

bool LauchSWData::Save(PhobosStreamWriter& Stm) const
{
	return const_cast<LauchSWData*>(this)->Serialize(Stm);
}

template <typename T>
bool LauchSWData::Serialize(T& Stm)
{
	return Stm
		.Process(LaunchWhat)
		.Process(LaunchWaitcharge)
		.Process(LaunchResetCharge)
		.Process(LaunchGrant)
		.Process(LaunchGrant_RepaintSidebar)
		.Process(LaunchGrant_OneTime)
		.Process(LaunchGrant_OnHold)
		.Process(LaunchSW_Manual)
		.Process(LaunchSW_IgnoreInhibitors)
		.Process(LaunchSW_IgnoreDesignators)
		.Process(LauchSW_IgnoreMoney)
		.Process(LaunchSW_DisplayMoney)
		.Process(LaunchSW_DisplayMoney_Houses)
		.Process(LaunchSW_DisplayMoney_Offset)
		.Success()
		;
}