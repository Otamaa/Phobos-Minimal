#include "AIControl.h"

#include <HouseClass.h>
#include <MessageListClass.h>
#include <RulesClass.h>

#include <Utilities/GeneralUtils.h>

const char* AIControlCommandClass::GetName() const
{
	return "AIControl";
}

const wchar_t* AIControlCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_AI_CONTROL", L"AI Control");
}

const wchar_t* AIControlCommandClass::GetUICategory() const
{
	return CATEGORY_DEVELOPMENT;
}

const wchar_t* AIControlCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_AI_CONTROL_DESC", L"Let the AI assume control.");
}

void AIControlCommandClass::Execute(WWKey dwUnk) const
{
	if (!Phobos::Otamaa::AllowAIControl) {
		return;
	}

	HouseClass* pPlayer = HouseClass::CurrentPlayer();

	if (pPlayer->IsHumanPlayer && pPlayer->IsInPlayerControl)
	{
		//let AI assume control
		pPlayer->IsHumanPlayer = pPlayer->IsInPlayerControl = false;
		pPlayer->Production = pPlayer->AutocreateAllowed = true;

		//give full capabilities
		pPlayer->StaticData.IQLevel = RulesClass::Global()->MaxIQLevels;
		pPlayer->IQLevel2 = RulesClass::Global()->MaxIQLevels;
		pPlayer->AIDifficulty = AIDifficulty::Hard;	//brutal!

		//notify
		MessageListClass::Instance->PrintMessage(L"AI assumed control!");

	}
	else
	{
		//re-assume control
		pPlayer->IsHumanPlayer = pPlayer->IsInPlayerControl = true;
		pPlayer->Production = pPlayer->AutocreateAllowed = false;

		//make it a vegetable
		pPlayer->StaticData.IQLevel = 0;
		pPlayer->IQLevel2 = 0;
		pPlayer->AIDifficulty = AIDifficulty::Normal;

		//notify
		MessageListClass::Instance->PrintMessage(L"Player assumed control!");
	}
}