#include "SaveVariablesToFile.h"

#include <Ext/Scenario/Body.h>
#include <HouseClass.h>
#include <Utilities/GeneralUtils.h>

const char* SaveVariablesToFileCommandClass::GetName() const
{
	return "Save Variables to File";
}

const wchar_t* SaveVariablesToFileCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_SAVE_VARIABLES", L"Save Variables to File");
}

const wchar_t* SaveVariablesToFileCommandClass::GetUICategory() const
{
	return CATEGORY_DEVELOPMENT;
}

const wchar_t* SaveVariablesToFileCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_SAVE_VARIABLES_DESC", L"Save local & global variables to an INI file.");
}

void SaveVariablesToFileCommandClass::Execute(WWKey eInput) const
{
	if (this->CheckDebugDeactivated())
		return;

	auto pMessage = GeneralUtils::LoadStringUnlessMissing("MSG:VariablesSaved", L"Variables saved.");
	GeneralUtils::PrintMessage(pMessage);
	ScenarioExtData::SaveVariablesToFile(false);
	ScenarioExtData::SaveVariablesToFile(true);
}
