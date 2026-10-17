#pragma once
#include "Commands.h"

class AutoLoadCommandClass : public CommandClass
{
public:
	AutoLoadCommandClass() = default;
	virtual ~AutoLoadCommandClass() = default;

public:

	//CommandClass
	virtual const char* GetName() const override;
	virtual const wchar_t* GetUIName() const override;
	virtual const wchar_t* GetUICategory() const override;
	virtual const wchar_t* GetUIDescription() const override;
	virtual void Execute(WWKey dwUnk) const override;

	static bool g_enabled;
	static int g_stagger;
	static volatile LONG g_wantLoad;
	static volatile LONG g_wantVariant;

	static int s_armed;
	static int s_countdown;
	static DWORD s_lastVariantTick;


	static void TickAll();
	static void ReadConfig();
	static void Reset();
};