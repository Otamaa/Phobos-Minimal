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

	virtual ~TunnelTypeClass() override = default;

	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm) override;
	virtual void SaveToStream(PhobosStreamWriter& Stm) override;

private:
	template <typename T>
	void Serialize(T& Stm);
};