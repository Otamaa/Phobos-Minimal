#include "LauchSWData.h"

#include <SuperWeaponTypeClass.h>

bool LauchSWData::ReadVector(std::vector<LauchSWData>& res , INI_EX& exINI, const char* pSection , bool CompatibilityMode){
	res.clear();

	if(!CompatibilityMode) {
		for (size_t i = 0; ; ++i) {
			SuperWeaponTypeClass* LaunchWhat_Dummy;
			std::string _base_key("LaunchSW");
			_base_key += std::to_string(i);

			if (!detail::read(LaunchWhat_Dummy, exINI, pSection, (_base_key + ".Type").c_str(), true) || !LaunchWhat_Dummy)
				break;

			res.emplace_back().Read(exINI, pSection, i, LaunchWhat_Dummy);
		}
	} else {
		ValueableVector<SuperWeaponTypeClass*> Lch_dummies {};

		Lch_dummies.Read(exINI, pSection , "LaunchSW" , true);

		for(auto pSW : Lch_dummies){
			if(pSW){
				res.emplace_back().Read(exINI, pSection,-1 , pSW);
			}
		}
	}

	return true;
}

bool LauchSWData::ReadSingle(INI_EX& exINI, const char* pID, int Prefix){
	std::string _buff = "LaunchSW";

	if(Prefix != -1){
		_buff += std::to_string(Prefix);
	}

	LaunchWaitcharge.Read(exINI, pID, (_buff + ".WaitForCharge").c_str());
	LaunchResetCharge.Read(exINI, pID, (_buff + ".ResetCharge").c_str());
	//
	std::string _buff_grant = (_buff + ".Grant");
	LaunchGrant.Read(exINI, pID, _buff_grant.c_str());
	LaunchGrant_RepaintSidebar.Read(exINI, pID, (_buff_grant + ".RepaintSidebar").c_str());
	LaunchGrant_OneTime.Read(exINI, pID, (_buff_grant + ".OneTime").c_str());
	LaunchGrant_OnHold.Read(exINI, pID, (_buff_grant + ".OnHold").c_str());
	//
	LaunchSW_RealLauch.Read(exINI, pID, (_buff + ".RealLaunch").c_str());
	LaunchSW_Manual.Read(exINI, pID, (_buff + ".Manual").c_str());
	LaunchSW_IgnoreInhibitors.Read(exINI, pID, (_buff + ".IgnoreInhibitors").c_str());
	LaunchSW_IgnoreDesignators.Read(exINI, pID, (_buff + ".IgnoreDesignators").c_str());
	LauchSW_IgnoreMoney.Read(exINI, pID, (_buff + ".IgnoreMoney").c_str());
	LauchSW_IgnoreBattleData.Read(exINI, pID, (_buff + ".IgnoreBattlePoints").c_str());
	//
	std::string _buff_DisplayMoney = (_buff + ".DisplayMoney");
	LaunchSW_DisplayMoney.Read(exINI, pID, _buff_DisplayMoney.c_str());

	if(LaunchSW_DisplayMoney) {
		LaunchSW_DisplayMoney_Houses.Read(exINI, pID, (_buff_DisplayMoney + ".Houses").c_str());
		LaunchSW_DisplayMoney_Offset.Read(exINI, pID, (_buff_DisplayMoney + ".Offset").c_str());
	}
	//
	LauchhSW_Owner.Read(exINI, pID, (_buff + ".Owner").c_str());

	return true;
}

bool LauchSWData::Read(INI_EX& exINI, const char* pID, int Prefix, SuperWeaponTypeClass* pReaded)
{
	LaunchWhat = pReaded->ArrayIndex;
	return this->ReadSingle(exINI,pID , Prefix);
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
		.Process(LauchSW_IgnoreBattleData)
		.Process(LaunchSW_DisplayMoney)
		.Process(LaunchSW_RealLauch)
		.Process(LaunchSW_DisplayMoney_Houses)
		.Process(LaunchSW_DisplayMoney_Offset)
		.Process(LauchhSW_Owner)
		.Success()
		//&& Stm.RegisterChange(this)
		; // announce this type
}