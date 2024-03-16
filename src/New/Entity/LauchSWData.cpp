#include "LauchSWData.h"

#include <SuperWeaponTypeClass.h>

bool LauchSWData::Read(INI_EX& exINI, const char* pID, int Prefix, SuperWeaponTypeClass* pReaded)
{
	if (!pReaded)
		return false;

	//char nBuff[0x80];
	LaunchWhat = pReaded->ArrayIndex;

	std::string _buff = std::format("LaunchSW{}.WaitForCharge", Prefix);
	//IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "LaunchSW%d.WaitForCharge", Prefix);
	LaunchWaitcharge.Read(exINI, pID, _buff.c_str());

	_buff = std::format("LaunchSW{}.ResetCharge", Prefix);
	//IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "LaunchSW%d.ResetCharge", Prefix);
	LaunchResetCharge.Read(exINI, pID, _buff.c_str());

	_buff = std::format("LaunchSW{}.Grant", Prefix);
	//IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "LaunchSW%d.Grant", Prefix);
	LaunchGrant.Read(exINI, pID, _buff.c_str());

	_buff = std::format("LaunchSW{}.Grant.RepaintSidebar", Prefix);
	//IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "LaunchSW%d.Grant.RepaintSidebar", Prefix);
	LaunchGrant_RepaintSidebar.Read(exINI, pID, _buff.c_str());

	_buff = std::format("LaunchSW{}.Grant.OneTime", Prefix);
	//IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "LaunchSW%d.Grant.OneTime", Prefix);
	LaunchGrant_OneTime.Read(exINI, pID, _buff.c_str());

	_buff = std::format("LaunchSW{}.Grant.OnHold", Prefix);
	//IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "LaunchSW%d.Grant.OnHold", Prefix);
	LaunchGrant_OnHold.Read(exINI, pID, _buff.c_str());

	_buff = std::format("LaunchSW{}.Manual", Prefix);
	//IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "LaunchSW%d.Manual", Prefix);
	LaunchSW_Manual.Read(exINI, pID, _buff.c_str());

	_buff = std::format("LaunchSW{}.IgnoreInhibitors", Prefix);
	//IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "LaunchSW%d.IgnoreInhibitors", Prefix);
	LaunchSW_IgnoreInhibitors.Read(exINI, pID, _buff.c_str());

	_buff = std::format("LaunchSW{}.IgnoreDesignators", Prefix);
	//IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "LaunchSW%d.IgnoreDesignators", Prefix);
	LaunchSW_IgnoreDesignators.Read(exINI, pID, _buff.c_str());

	_buff = std::format("LaunchSW{}.IgnoreMoney", Prefix);
	//IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "LaunchSW%d.IgnoreMoney", Prefix);
	LauchSW_IgnoreMoney.Read(exINI, pID, _buff.c_str());

	_buff = std::format("LaunchSW{}.DisplayMoney", Prefix);
	//IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "LaunchSW%d.DisplayMoney", Prefix);
	LaunchSW_DisplayMoney.Read(exINI, pID, _buff.c_str());

	if(LaunchSW_DisplayMoney) {
		_buff = std::format("LaunchSW{}.DisplayMoney.Houses", Prefix);
		//IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "LaunchSW%d.DisplayMoney.Houses", Prefix);
		LaunchSW_DisplayMoney_Houses.Read(exINI, pID, _buff.c_str());

		_buff = std::format("LaunchSW{}.DisplayMoney.Offset", Prefix);
		//IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "LaunchSW%d.DisplayMoney.Offset", Prefix);
		LaunchSW_DisplayMoney_Offset.Read(exINI, pID, _buff.c_str());
	}

	_buff = std::format("LaunchSW{}.Owner", Prefix);
	//IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "LaunchSW%d.Owner", Prefix);
	LauchhSW_Owner.Read(exINI, pID, _buff.c_str());

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
		.Process(LauchhSW_Owner)
		.Success()
		//&& Stm.RegisterChange(this)
		; // announce this type
}