#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>
#include <Utilities/GeneralUtils.h>
#include <Ext/Rules/Body.h>

class WarheadTypeClass;

class RadTypeClass final : public Enumerable<RadTypeClass>
{
private:
	Nullable<int> DurationMultiple;
	Nullable<int> ApplicationDelay;
	Nullable<int> BuildingApplicationDelay;
	Nullable<double> LevelFactor;
	Nullable<int> LevelMax;
	Nullable<int> LevelDelay;
	Nullable<int> LightDelay;
	Nullable<WarheadTypeClass*> RadWarhead;
	Nullable<bool> RadWarhead_Detonate;
	Nullable<ColorStruct> RadSiteColor;
	Nullable<double> LightFactor;
	Nullable<double> TintFactor;
	Nullable<bool> RadHasOwner;
	Nullable<bool> RadHasInvoker;

public:

	RadTypeClass(const char* const pTitle) : Enumerable<RadTypeClass> { pTitle }
		, DurationMultiple {}
		, ApplicationDelay {}
		, BuildingApplicationDelay {}
		, LevelFactor {}
		, LevelMax {}
		, LevelDelay {}
		, LightDelay {}
		, RadWarhead {}
		, RadWarhead_Detonate {}
		, RadSiteColor {}
		, LightFactor {}
		, TintFactor {}
		, RadHasOwner {}
		, RadHasInvoker {}
	{ }

	virtual ~RadTypeClass() override = default;

	static void AddDefaults();

	WarheadTypeClass* GetWarhead()
	{
		return RadWarhead.Get(RulesGlobal->RadSiteWarhead);
	}

	bool GetWarheadDetonate() const
	{
		return this->RadWarhead_Detonate.Get(RulesExt::Global()->RadWarhead_Detonate);
	}

	bool GetHasOwner(const Nullable<bool>&nOwner) const
	{
		return nOwner.Get(this->RadHasOwner.Get(RulesExt::Global()->RadHasOwner));
	}

	bool GetHasInvoker(const Nullable<bool>& nOwner) const
	{
		return nOwner.Get(this->RadHasInvoker.Get(RulesExt::Global()->RadHasInvoker));
	}

	bool GetHasOwner() const
	{
		return (this->RadHasOwner.Get(RulesExt::Global()->RadHasOwner));
	}

	bool GetHasInvoker() const
	{
		return (this->RadHasInvoker.Get(RulesExt::Global()->RadHasInvoker));
	}

	const ColorStruct& GetColor()
	{
		return *this->RadSiteColor.GetEx(&RulesGlobal->RadColor);
	}

	int GetDurationMultiple()
	{
		return this->DurationMultiple.Get(RulesGlobal->RadDurationMultiple);
	}

	int GetApplicationDelay()
	{
		return abs(this->ApplicationDelay.Get(RulesGlobal->RadApplicationDelay));
	}

	int GetBuildingApplicationDelay()
	{
		return abs(this->BuildingApplicationDelay.Get(RulesExt::Global()->RadApplicationDelay_Building));
	}

	int GetLevelMax()
	{
		return this->LevelMax.Get(RulesGlobal->RadLevelMax);
	}

	int GetLevelDelay()
	{
		return this->LevelDelay.Get(RulesGlobal->RadLevelDelay);
	}

	int GetLightDelay()
	{
		return this->LightDelay.Get(RulesGlobal->RadLightDelay);
	}

	double GetLevelFactor()
	{
		return this->LevelFactor.Get(RulesGlobal->RadLevelFactor);
	}

	double GetLightFactor()
	{
		return this->LightFactor.Get(RulesGlobal->RadLightFactor);
	}

	double GetTintFactor()
	{
		return this->TintFactor.Get(RulesGlobal->RadTintFactor);
	}

	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm);
	virtual void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};