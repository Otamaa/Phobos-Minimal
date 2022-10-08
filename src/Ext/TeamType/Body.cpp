#include "Body.h"

#ifdef ENABLE_NEWHOOKS
TeamTypeExt::ExtContainer TeamTypeExt::ExtMap;

void TeamTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->Get();
	const char* pSection = pThis->ID;

	if (!pINI->GetSection(pSection))
		return;

	INI_EX exINI(pINI);
	this->AI_SafeDIstance.Read(exINI, pSection, "AISafeDistance");
	this->AI_FriendlyDistance.Read(exINI, pSection, "AIFriendlyDistance");
	this->AttackWaypoint_AllowCell.Read(exINI, pSection, "AttackWaypoint.AllowCell");
}

// =============================
// load / save

template <typename T>
void TeamTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->AI_SafeDIstance)
		.Process(this->AI_FriendlyDistance)
		.Process(this->AttackWaypoint_AllowCell)
		;

}

void TeamTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<TeamTypeClass>::Serialize(Stm);
	this->Serialize(Stm);
}

void TeamTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<TeamTypeClass>::Serialize(Stm);
	this->Serialize(Stm);
}

bool TeamTypeExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool TeamTypeExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

TeamTypeExt::ExtContainer::ExtContainer() : Container("TeamTypeClass") { }
TeamTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks
//ToDo : Check Size !

DEFINE_HOOK(0x6F08E4, TeamTypeClass_CTOR, 0x5)
{
	//Debug::Log("%s Executed ! \n", __FUNCTION__);
	GET(TeamTypeClass*, pItem, ESI);
//doesnt work ?
#ifdef ENABLE_NEWHOOKS_
	TeamTypeExt::ExtMap.JustAllocate(pItem, pItem, "Trying To Allocate from nullptr !");
#else
	TeamTypeExt::ExtMap.FindOrAllocate(pItem);
#endif
	return 0;
}

DEFINE_HOOK(0x6F0926, TeamTypeClass_DTOR, 0x7)
{
	//Debug::Log("%s Executed ! \n", __FUNCTION__);
	GET(TeamTypeClass*, pItem, ESI);
	TeamTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x6F1BB0, TeamTypeClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x6F1B90, TeamTypeClass_SaveLoad_Prefix, 0x8)
{
	//Debug::Log("%s Executed ! \n", __FUNCTION__);
	GET_STACK(TeamTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);
	TeamTypeExt::ExtMap.PrepareStream(pItem, pStm);
	return 0;
}

DEFINE_HOOK(0x6F1C35, TeamTypeClass_Load_Suffix, 0x5)
{
	//Debug::Log("%s Executed ! \n", __FUNCTION__);
	TeamTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x6F1BAA, TeamTypeClass_Save_Suffix, 0x5)
{
	//Debug::Log("%s Executed ! \n", __FUNCTION__);
	TeamTypeExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK(0x6F1479, TeamTypeClass_LoadFromINI, 0xA)
{
	//Debug::Log("%s Executed ! \n", __FUNCTION__);
	GET(TeamTypeClass*, pItem, ESI);
	GET(CCINIClass*, pINI, EBX);
	TeamTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0x0;
}

DEFINE_HOOK(0x6F181B, TeamTypeClass_WriteToINI, 0x8)
{
	//Debug::Log("%s Executed ! \n", __FUNCTION__);
	GET(TeamTypeClass*, pItem, ESI);
	GET(CCINIClass*, pINI, EBX);

	if (const auto pExt = TeamTypeExt::ExtMap.Find(pItem)) {
		auto const pSection = pItem->ID;

		if(pExt->AI_SafeDIstance.isset())
			pINI->WriteInteger(pSection, "AISafeDistance", pExt->AI_SafeDIstance.Get(), false);

		if(pExt->AI_FriendlyDistance.isset())
			pINI->WriteInteger(pSection, "AIFriendlyDistance", pExt->AI_FriendlyDistance.Get(), false);

		pINI->WriteInteger(pSection, "AttackWaypoint.AllowCell", pExt->AttackWaypoint_AllowCell.Get(), true);
	}

	return 0x0;
}
#endif