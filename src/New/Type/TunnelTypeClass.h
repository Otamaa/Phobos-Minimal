#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/TemplateDefB.h>

class TunnelTypeClass final : public Enumerable<TunnelTypeClass>
{
public:
	static COMPILETIMEEVAL const char* MainSection = "TunnelTypes";
	static COMPILETIMEEVAL const char* ClassName = "TunnelTypeClass";

public:

	Valueable<int> Passengers;
	Valueable<double> MaxSize;
	TunnelTypeClass(const char* const pTitle) : Enumerable<TunnelTypeClass>(pTitle)
	   , Passengers { 0 }
	   , MaxSize { 0.0 }
	{ }

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};