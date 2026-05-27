#pragma once

#include <Commands/Commands.h>

class MapSnapshotCommandClass : public PhobosCommandClass
{
public:

	virtual const char* GetName() const override;
	virtual const wchar_t* GetUIName() const override;
	virtual const wchar_t* GetUICategory() const override;
	virtual const wchar_t* GetUIDescription() const override;
	virtual void Execute(WWKey dwUnk) const override;
};
