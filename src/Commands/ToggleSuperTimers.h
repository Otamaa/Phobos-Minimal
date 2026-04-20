#pragma once

#include <Commands/Commands.h>

/**
 *  Toggles the visibility of the super weapon timers on the tactical view.
 */
class ToggleSuperTimersCommandClass : public PhobosCommandClass
{
public:

	virtual const char* GetName() const override;
	virtual const wchar_t* GetUIName() const override;
	virtual const wchar_t* GetUICategory() const override;
	virtual const wchar_t* GetUIDescription() const override;
	virtual void Execute(WWKey dwUnk) const override;

public:
	static bool ShowSuperWeaponTimers;
};