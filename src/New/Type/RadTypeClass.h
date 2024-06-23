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
	Nullable<int> BuildingDamageMaxCount;

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
		, BuildingDamageMaxCount {}
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

	constexpr inline WarheadTypeClass* GetWarhead() const
	{
		return RadWarhead.Get(RulesClass::Instance->RadSiteWarhead);
	}

	constexpr inline bool GetWarheadDetonate() const
	{
		return this->RadWarhead_Detonate.Get(RulesExtData::Instance()->RadWarhead_Detonate);
	}

	constexpr inline bool GetHasOwner(const Nullable<bool>&nOwner) const
	{
		return nOwner.Get(this->RadHasOwner.Get(RulesExtData::Instance()->RadHasOwner));
	}

	constexpr inline bool GetHasInvoker(const Nullable<bool>& nOwner) const
	{
		return nOwner.Get(this->RadHasInvoker.Get(RulesExtData::Instance()->RadHasInvoker));
	}

	constexpr inline bool GetHasOwner() const
	{
		return (this->RadHasOwner.Get(RulesExtData::Instance()->RadHasOwner));
	}

	constexpr inline bool GetHasInvoker() const
	{
		return (this->RadHasInvoker.Get(RulesExtData::Instance()->RadHasInvoker));
	}

	constexpr inline ColorStruct GetColor() const
	{
		return this->RadSiteColor.Get(RulesClass::Instance->RadColor);
	}

	constexpr inline int GetDurationMultiple() const
	{
		return this->DurationMultiple.Get(RulesClass::Instance->RadDurationMultiple);
	}

	constexpr inline int GetApplicationDelay()  const
	{
		return this->ApplicationDelay.Get(RulesClass::Instance->RadApplicationDelay);
	}

	constexpr inline int GetBuildingApplicationDelay() const
	{
		return this->BuildingApplicationDelay.Get(RulesExtData::Instance()->RadApplicationDelay_Building.Get());
	}

	constexpr inline int GetBuildingDamageMaxCount() const
	{
		return this->BuildingDamageMaxCount.Get(RulesExtData::Instance()->RadBuildingDamageMaxCount);
	}

	constexpr inline int GetLevelMax()  const
	{
		return this->LevelMax.Get(RulesClass::Instance->RadLevelMax);
	}

	constexpr inline int GetLevelDelay() const
	{
		return this->LevelDelay.Get(RulesClass::Instance->RadLevelDelay);
	}

	constexpr inline int GetLightDelay() const
	{
		return this->LightDelay.Get(RulesClass::Instance->RadLightDelay);
	}

	constexpr inline double GetLevelFactor() const
	{
		return this->LevelFactor.Get(RulesClass::Instance->RadLevelFactor);
	}

	constexpr inline double GetLightFactor() const
	{
		return this->LightFactor.Get(RulesClass::Instance->RadLightFactor);
	}

	constexpr inline double GetTintFactor() const
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