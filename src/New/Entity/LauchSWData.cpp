#include "LauchSWData.h"

bool LauchSWData::Read(INI_EX& exINI, const char* pID, int Prefix)
{
	char nBuff[0x100];

	Nullable<SuperWeaponTypeClass*> LaunchWhat_Dummy { };
	_snprintf(nBuff, sizeof(nBuff), "LaunchSW%d.Type", Prefix);
	LaunchWhat_Dummy.Read(exINI, pID, nBuff, true);

	if (!LaunchWhat_Dummy.isset() || !LaunchWhat_Dummy.Get())
		return false;

	LaunchWhat = LaunchWhat_Dummy->ArrayIndex;

	Valueable<bool> bool_Dummy { LaunchWaitcharge };
	_snprintf(nBuff, sizeof(nBuff), "LaunchSW%d.WaitForCharge", Prefix);
	bool_Dummy.Read(exINI, pID, nBuff);
	LaunchWaitcharge = bool_Dummy.Get();

	bool_Dummy = LaunchResetCharge;
	_snprintf(nBuff, sizeof(nBuff), "LaunchSW%d.ResetCharge", Prefix);
	bool_Dummy.Read(exINI, pID, nBuff);
	LaunchResetCharge = bool_Dummy.Get();

	bool_Dummy = LaunchGrant;
	_snprintf(nBuff, sizeof(nBuff), "LaunchSW%d.Grant", Prefix);
	bool_Dummy.Read(exINI, pID, nBuff);
	LaunchGrant = bool_Dummy.Get();

	bool_Dummy = LaunchGrant_RepaintSidebar;
	_snprintf(nBuff, sizeof(nBuff), "LaunchSW%d.Grant.RepaintSidebar", Prefix);
	bool_Dummy.Read(exINI, pID, nBuff);
	LaunchGrant_RepaintSidebar = bool_Dummy.Get();

	bool_Dummy = LaunchGrant_OneTime;
	_snprintf(nBuff, sizeof(nBuff), "LaunchSW%d.Grant.OneTime", Prefix);
	bool_Dummy.Read(exINI, pID, nBuff);
	LaunchGrant_OneTime = bool_Dummy.Get();

	bool_Dummy = LaunchGrant_OnHold;
	_snprintf(nBuff, sizeof(nBuff), "LaunchSW%d.Grant.OnHold", Prefix);
	bool_Dummy.Read(exINI, pID, nBuff);
	LaunchGrant_OnHold = bool_Dummy.Get();

	bool_Dummy = LaunchSW_Manual;
	_snprintf(nBuff, sizeof(nBuff), "LaunchSW%d.Manual", Prefix);
	bool_Dummy.Read(exINI, pID, nBuff);
	LaunchSW_Manual = bool_Dummy.Get();

	bool_Dummy = LaunchSW_IgnoreInhibitors;
	_snprintf(nBuff, sizeof(nBuff), "LaunchSW%d.IgnoreInhibitors", Prefix);
	bool_Dummy.Read(exINI, pID, nBuff);
	LaunchSW_IgnoreInhibitors = bool_Dummy.Get();

	bool_Dummy =  LaunchSW_IgnoreDesignators;
	_snprintf(nBuff, sizeof(nBuff), "LaunchSW%d.IgnoreDesignators", Prefix);
	bool_Dummy.Read(exINI, pID, nBuff);
	LaunchSW_IgnoreDesignators = bool_Dummy.Get();

	bool_Dummy = LauchSW_IgnoreMoney;
	_snprintf(nBuff, sizeof(nBuff), "LaunchSW%d.IgnoreMoney", Prefix);
	bool_Dummy.Read(exINI, pID, nBuff);
	LauchSW_IgnoreMoney = bool_Dummy.Get();

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
		.Success()
		;
}