#pragma once

#include "Commands.h"

class PlaceVeinholeMonster : public PhobosCommandClass
{
public:

	static bool Given;

	// CommandClass
	virtual const char* GetName() const override;
	virtual const wchar_t* GetUIName() const override;
	virtual const wchar_t* GetUICategory() const override;
	virtual const wchar_t* GetUIDescription() const override;
	virtual void Execute(WWKey eInput) const override;
};
