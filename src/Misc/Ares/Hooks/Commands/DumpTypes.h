#pragma once

#include <Commands/Commands.h>

class DumperTypesCommandClass : public PhobosCommandClass
{
public:

	virtual const char* GetName() const override;
	virtual const wchar_t* GetUIName() const override;
	virtual const wchar_t* GetUICategory() const override;
	virtual const wchar_t* GetUIDescription() const override;

	template <typename T>
	void LogType(const char* pSection) const;

	virtual void Execute(WWKey dwUnk) const override;
};
