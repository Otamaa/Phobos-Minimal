#pragma once

#include "Commands.h"
#include <Utilities/GeneralUtils.h>
#include <Ext/Rules/Body.h>

class ShowHealthPercentCommandClass : public PhobosCommandClass
{
public:

	// CommandClass
	virtual const char* GetName() const override
	{
		return "Show Health Percent";
	}

	virtual const wchar_t* GetUIName() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_SHOWHEALTHPERCENT", L"Show Health Percent");
	}

	virtual const wchar_t* GetUICategory() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_INTERFACE", L"Interface");
	}

	virtual const wchar_t* GetUIDescription() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_SHOWHEALTHPERCENT_DESC", L"Show health percent of all technos if available.");
	}

	virtual void Execute(WWKey eInput) const override
	{
		Phobos::Otamaa::ShowHealthPercentEnabled = !Phobos::Otamaa::ShowHealthPercentEnabled;
	}
};
