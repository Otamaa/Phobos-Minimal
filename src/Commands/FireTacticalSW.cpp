#include "FireTacticalSW.h"

#include <New/SuperWeaponSidebar/SWSidebarClass.h>
#include <New/SuperWeaponSidebar/SWColumnClass.h>
#include <New/SuperWeaponSidebar/SWButtonClass.h>

#include <ScenarioClass.h>
#include <Utilities/GeneralUtils.h>

template<size_t Index>
class FireTacticalSWCommandClass : public PhobosCommandClass
{
	virtual const char* GetName() const override;
	virtual const wchar_t* GetUIName() const override;
	virtual const wchar_t* GetUICategory() const override;
	virtual const wchar_t* GetUIDescription() const override;
	virtual void Execute(WWKey eInput) const override;
};

template<size_t Index>
OPTIONALINLINE const char* FireTacticalSWCommandClass<Index>::GetName() const
{
	IMPL_SNPRNINTF(Phobos::readBuffer, Phobos::readLength, "FireTacticalSW%d", Index);
	return Phobos::readBuffer;
}

template<size_t Index>
OPTIONALINLINE const wchar_t* FireTacticalSWCommandClass<Index>::GetUIName() const
{
	const wchar_t* csfString = GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_FIRE_TACTICAL_SW_XX", L"Fire Super Weapon %d");
	_snwprintf_s(Phobos::wideBuffer, std::size(Phobos::wideBuffer), csfString, Index);
	return Phobos::wideBuffer;
}

template<size_t Index>
OPTIONALINLINE const wchar_t* FireTacticalSWCommandClass<Index>::GetUICategory() const
{
	return CATEGORY_CONTROL;
}

template<size_t Index>
OPTIONALINLINE const wchar_t* FireTacticalSWCommandClass<Index>::GetUIDescription() const
{
	const wchar_t* csfString = GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_FIRE_TACTICAL_SW_XX_DESC", L"Fires the Super Weapon at position %d in the Super Weapon sidebar.");
	_snwprintf_s(Phobos::wideBuffer, std::size(Phobos::wideBuffer), csfString, Index);
	return Phobos::wideBuffer;
}

template<size_t Index>
OPTIONALINLINE void FireTacticalSWCommandClass<Index>::Execute(WWKey eInput) const
{
	if (!SWSidebarClass::IsEnabled() || ScenarioClass::Instance->UserInputLocked)
		return;

	const auto& columns = SWSidebarClass::Global()->Columns;

	if (columns.empty())
		return;

	const auto& buttons = columns.front()->Buttons;
	size_t idx = size_t(Index - 1);

	if (idx < buttons.size())
		buttons[idx]->LaunchSuper();
}

template <typename T>
static FORCEDINLINE T* Make()
{
	T* command = GameCreate<T>();
	CommandClass::Array->push_back(command);
	return command;
};

void FireTacticalSWDispatch::Dispatch()
{

	if (Phobos::Config::SuperWeaponSidebarCommands) {
		SWSidebarClass::Commands[0] = Make<FireTacticalSWCommandClass<1>>();
		SWSidebarClass::Commands[1] = Make<FireTacticalSWCommandClass<2>>();
		SWSidebarClass::Commands[2] = Make<FireTacticalSWCommandClass<3>>();
		SWSidebarClass::Commands[3] = Make<FireTacticalSWCommandClass<4>>();
		SWSidebarClass::Commands[4] = Make<FireTacticalSWCommandClass<5>>();
		SWSidebarClass::Commands[5] = Make<FireTacticalSWCommandClass<6>>();
		SWSidebarClass::Commands[6] = Make<FireTacticalSWCommandClass<7>>();
		SWSidebarClass::Commands[7] = Make<FireTacticalSWCommandClass<8>>();
		SWSidebarClass::Commands[8] = Make<FireTacticalSWCommandClass<9>>();
		SWSidebarClass::Commands[9] = Make<FireTacticalSWCommandClass<10>>();
	}
}