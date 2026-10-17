#pragma once

#include "Commands.h"

/// Hotkey "Select Same Variant": the native T-select filtered by the REAL weapon of
/// Gunner=yes vehicles (IFVs). One press = on screen, second press within 500 ms = whole map.
/// Single vs double press is just a timer inside Execute: no second command is needed for it.
class VariantSelectCommandClass : public CommandClass
{
public:
	//CommandClass
	virtual const char* GetName() const override;
	virtual const wchar_t* GetUIName() const override;
	virtual const wchar_t* GetUICategory() const override;
	virtual const wchar_t* GetUIDescription() const override;
	virtual void Execute(WWKey dwUnk) const override;
};