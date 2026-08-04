#include "Body.h"

 #include <Utilities/Patch.h>
#include <Utilities/Macro.h>

bool TeamTypeExtData::LoadFromINI(CCINIClass* pINI, bool parseFailAddr)
{
	if (parseFailAddr)
		return false;

	auto pThis = this->This();
	const char* pSection = pThis->ID;
	INI_EX exINI(pINI);
	this->IsDischargedMemberAutocreateRecruitable.Read(exINI, pSection, "IsDischargedMemberAutocreateRecruitable");
	this->AI_SafeDIstance.Read(exINI, pSection, "AISafeDistance");
	this->AI_FriendlyDistance.Read(exINI, pSection, "AIFriendlyDistance");
	this->AttackWaypoint_AllowCell.Read(exINI, pSection, "AttackWaypoint.AllowCell");
	return true;
}

// =============================
// load / save

template <typename T>
void TeamTypeExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->AI_SafeDIstance)
		.Process(this->AI_FriendlyDistance)
		.Process(this->IsDischargedMemberAutocreateRecruitable)
		.Process(this->AttackWaypoint_AllowCell)
		.Process(this->OriginalScriptTypeIndex)
		.Process(this->OriginalTaskForceIndex)
		;

}

// =============================
// container
TeamTypeExtContainer TeamTypeExtContainer::Instance;

void TeamTypeExtContainer::LoadFromINI(TeamTypeClass* key, CCINIClass* pINI, bool parseFailAddr)
{
	if (auto ptr = this->Find(key))
	{
		if (!pINI)
		{
			return;
		}


		// Rules first 
		// Other files 
		// when this doesnt match the case it will causing weirdd issues like some value wont be initialized or replaced to default value after parsing
		switch (ptr->Initialized)
		{
		case InitState::Blank:
		{
			if (pINI == CCINIClass::INI_Rules())
			{
				ptr->SetInitState(InitState::Inited);
				//ptr->Initialize();
			}
			[[fallthrough]];
		}
		case InitState::Inited:
		case InitState::Ruled:
		{
			ptr->LoadFromINI(pINI, parseFailAddr);
			ptr->SetInitState(InitState::Ruled);
			[[fallthrough]];
		}
		default:
			break;
		}
	}

}

void TeamTypeExtContainer::WriteToINI(TeamTypeClass* key, CCINIClass* pINI)
{

	if (auto ptr = this->TryFind(key))
	{
		if (!pINI)
		{
			return;
		}

		ptr->WriteToINI(pINI);
	}
}
// =============================
// container hooks
//ToDo : Check Size !

ASMJIT_PATCH(0x6F08E4, TeamTypeClass_CTOR, 0x5)
{
	GET(TeamTypeClass*, pItem, ESI);

	if (!Phobos::Otamaa::DoingLoadGame)
		TeamTypeExtContainer::Instance.Allocate(pItem);

	return 0;
}

ASMJIT_PATCH(0x6F0926, TeamTypeClass_DTOR, 0x7)
{
	GET(TeamTypeClass*, pItem, ESI);
	TeamTypeExtContainer::Instance.Remove(pItem);
	return 0;
}ASMJIT_PATCH_AGAIN(0x6F2106, TeamTypeClass_DTOR, 0x7)

HRESULT __stdcall FakeTeamTypeClass::__Load(IStream* pStm)
{
	HRESULT hr = this->TeamTypeClass::Load(pStm);

	if (SUCCEEDED(hr)) {
		if (!TeamTypeExtContainer::Instance.LoadByKey(this, pStm))
			return PHOBOS_E_EXTDATA_LOAD_FAILED;
	}

	return hr;
}
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F47E4, FakeTeamTypeClass::__Load)

HRESULT __stdcall FakeTeamTypeClass::__Save(IStream* pStm, BOOL fClearDirty)
{
	HRESULT hr = this->TeamTypeClass::Save(pStm, fClearDirty);

	if (SUCCEEDED(hr)) {
		if (!TeamTypeExtContainer::Instance.SaveByKey(this, pStm))
			return PHOBOS_E_EXTDATA_SAVE_FAILED;
	}

	return hr;
}
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F47E8, FakeTeamTypeClass::__Save)

ASMJIT_PATCH(0x6F1528, TeamTypeClass_LoadFromINI, 0xA)
{
	GET(TeamTypeClass*, pItem, ESI);
	GET(CCINIClass*, pINI, EBX);
	TeamTypeExtContainer::Instance.LoadFromINI(pItem, pINI , R->Origin() == 0x6F1535);
	return 0x0;
}ASMJIT_PATCH_AGAIN(0x6F1535, TeamTypeClass_LoadFromINI, 0xA)

//ASMJIT_PATCH(0x6F1836, TeamTypeClass_WriteToINI, 0x6)
//{
//	GET(TeamTypeClass*, pItem, ESI);
//	GET_STACK(CCINIClass*, pINI,STACK_OFFSET(0x10 , 0x4));
//	GET(const char*, pSection, EDI);
//
//	if (pItem->TaskForce) {
//		pINI->WriteString(pSection, "TaskForce", pItem->TaskForce->ID);
//	}
//
//	if (const auto pExt = TeamTypeExt::ExtMap.Find(pItem)) {
//
//		//if(pExt->AI_SafeDIstance.isset())
//		//	pINI->WriteInteger(pSection, "AISafeDistance", pExt->AI_SafeDIstance.Get(), false);
//
//		//if(pExt->AI_FriendlyDistance.isset())
//		//	pINI->WriteInteger(pSection, "AIFriendlyDistance", pExt->AI_FriendlyDistance.Get(), false);
//
//		//if(pExt->AttackWaypoint_AllowCell.isset())
//		//	pINI->WriteInteger(pSection, "AttackWaypoint.AllowCell", pExt->AttackWaypoint_AllowCell.Get(), true);
//	}
//
//	return 0x6F1851;
//}
