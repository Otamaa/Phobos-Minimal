#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>

class TunnelTypeClass final : public Enumerable<TunnelTypeClass>
{
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