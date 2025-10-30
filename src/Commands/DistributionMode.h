#pragma once

#include "Commands.h"
#include <CellStruct.h>

class SwitchNoMoveCommandClass : public CommandClass
{
public:
	virtual const char* GetName() const override;
	virtual const wchar_t* GetUIName() const override;
	virtual const wchar_t* GetUICategory() const override;
	virtual const wchar_t* GetUIDescription() const override;
	virtual void Execute(WWKey eInput) const override;
};

class DistributionModeSpreadCommandClass : public CommandClass
{
public:
	virtual const char* GetName() const override;
	virtual const wchar_t* GetUIName() const override;
	virtual const wchar_t* GetUICategory() const override;
	virtual const wchar_t* GetUIDescription() const override;
	virtual void Execute(WWKey eInput) const override;
};

class DistributionModeFilterCommandClass : public CommandClass
{
public:
	virtual const char* GetName() const override;
	virtual const wchar_t* GetUIName() const override;
	virtual const wchar_t* GetUICategory() const override;
	virtual const wchar_t* GetUIDescription() const override;
	virtual void Execute(WWKey eInput) const override;
};

class ObjectClass;
class TechnoClass;
class DistributionModeHoldDownCommandClass : public CommandClass
{
public:
	static bool Enabled;
	static bool OnMessageShowed;
	static bool OffMessageShowed;
	static int ShowTime;

	virtual const char* GetName() const override;
	virtual const wchar_t* GetUIName() const override;
	virtual const wchar_t* GetUICategory() const override;
	virtual const wchar_t* GetUIDescription() const override;
	virtual bool ExtraTriggerCondition(WWKey eInput) const override;
	virtual void Execute(WWKey eInput) const override;

	static void DistributionModeOn(int idx);
	static void DistributionModeOff(int idx);
	static void DistributionSpreadModeExpand();
	static void DistributionSpreadModeReduce();

	static void ClickedWaypoint(ObjectClass* pSelect, int idxPath, signed char idxWP);
	static void ClickedTargetAction(ObjectClass* pSelect, Action action, ObjectClass* pTarget);
	static void ClickedCellAction(ObjectClass* pSelect, Action action, CellStruct* pCell, CellStruct* pSecondCell);
	static void AreaGuardAction(TechnoClass* pTechno);

	static void DrawRadialIndicator();
};