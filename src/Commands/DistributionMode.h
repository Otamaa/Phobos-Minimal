#pragma once

#include "Commands.h"

class ObjectClass;
struct DistributionMode {
	static void Draw(ObjectClass* const pTarget, const Action mouseAction);
	static void DrawRadialIndicator();

};
class DistributionMode1CommandClass : public PhobosCommandClass
{
public:
	static int Mode;

	virtual const char* GetName() const override;
	virtual const wchar_t* GetUIName() const override;
	virtual const wchar_t* GetUICategory() const override;
	virtual const wchar_t* GetUIDescription() const override;
	virtual void Execute(WWKey eInput) const override;
};

class DistributionMode2CommandClass : public PhobosCommandClass
{
public:
	static int Mode;

	virtual const char* GetName() const override;
	virtual const wchar_t* GetUIName() const override;
	virtual const wchar_t* GetUICategory() const override;
	virtual const wchar_t* GetUIDescription() const override;
	virtual void Execute(WWKey eInput) const override;
};