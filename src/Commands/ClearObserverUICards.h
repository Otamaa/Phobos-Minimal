#pragma once

#include "Commands.h"

// Command to clear all open floating card windows in Observer UI
class ClearObserverUICardsCommandClass : public CommandClass
{
public:
	// CommandClass
	virtual const char* GetName() const override;
	virtual const wchar_t* GetUIName() const override;
	virtual const wchar_t* GetUICategory() const override;
	virtual const wchar_t* GetUIDescription() const override;
	virtual void Execute(WWKey eInput) const override;
};
