#pragma once
#include <Base/Always.h>

class RegionClass
{
public:
	RegionClass() { Threat = 0; }
	~RegionClass() { }

	int operator != (RegionClass const& region) { return memcmp(this, &region, sizeof(RegionClass)); }
	int operator == (RegionClass const& region) { return !(memcmp(this, &region, sizeof(RegionClass))); }
	int operator > (RegionClass const& region) { return memcmp(this, &region, sizeof(RegionClass)) > 0; }
	int operator < (RegionClass const& region) { return memcmp(this, &region, sizeof(RegionClass)) < 0; }

	void Reset_Threat() { Threat = 0; }
	void Adjust_Threat(int threat, int neg) { if (neg) Threat -= threat; else Threat += threat; }
	int Threat_Value() const { return Threat; }

protected:
	long Threat;
};
