#pragma once

#include "Commands.h"

// Command to toggle Observer UI overlay display mode
class ToggleObserverUICommandClass : public CommandClass
{
public:
	// CommandClass
	virtual const char* GetName() const override;
	virtual const wchar_t* GetUIName() const override;
	virtual const wchar_t* GetUICategory() const override;
	virtual const wchar_t* GetUIDescription() const override;
	virtual void Execute(WWKey eInput) const override;
};
