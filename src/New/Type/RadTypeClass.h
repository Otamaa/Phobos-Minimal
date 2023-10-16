#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/TemplateDefB.h>
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

	inline WarheadTypeClass* GetWarhead() const
	{
		return RadWarhead.Get(RulesClass::Instance->RadSiteWarhead);
	}

	inline bool GetWarheadDetonate() const
	{
		return this->RadWarhead_Detonate.Get(RulesExtData::Instance()->RadWarhead_Detonate);
	}

	inline bool GetHasOwner(const Nullable<bool>&nOwner) const
	{
		return nOwner.Get(this->RadHasOwner.Get(RulesExtData::Instance()->RadHasOwner));
	}

	inline bool GetHasInvoker(const Nullable<bool>& nOwner) const
	{
		return nOwner.Get(this->RadHasInvoker.Get(RulesExtData::Instance()->RadHasInvoker));
	}

	inline bool GetHasOwner() const
	{
		return (this->RadHasOwner.Get(RulesExtData::Instance()->RadHasOwner));
	}

	inline bool GetHasInvoker() const
	{
		return (this->RadHasInvoker.Get(RulesExtData::Instance()->RadHasInvoker));
	}

	inline ColorStruct GetColor() const
	{
		return this->RadSiteColor.Get(RulesClass::Instance->RadColor);
	}

	inline int GetDurationMultiple() const
	{
		return this->DurationMultiple.Get(RulesClass::Instance->RadDurationMultiple);
	}

	inline int GetApplicationDelay()  const
	{
		return abs(this->ApplicationDelay.Get(RulesClass::Instance->RadApplicationDelay));
	}

	inline int GetBuildingApplicationDelay() const
	{
		return abs(this->BuildingApplicationDelay.Get(RulesExtData::Instance()->RadApplicationDelay_Building.Get()));
	}

	inline int GetLevelMax()  const
	{
		return this->LevelMax.Get(RulesClass::Instance->RadLevelMax);
	}

	inline int GetLevelDelay() const
	{
		return this->LevelDelay.Get(RulesClass::Instance->RadLevelDelay);
	}

	inline int GetLightDelay() const
	{
		return this->LightDelay.Get(RulesClass::Instance->RadLightDelay);
	}

	inline double GetLevelFactor() const
	{
		return this->LevelFactor.Get(RulesClass::Instance->RadLevelFactor);
	}

	inline double GetLightFactor() const
	{
		return this->LightFactor.Get(RulesClass::Instance->RadLightFactor);
	}

	inline double GetTintFactor() const
	{
		return this->TintFactor.Get(RulesClass::Instance->RadTintFactor);
	}

	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm);
	virtual void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};