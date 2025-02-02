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

	static void COMPILETIMEEVAL OPTIONALINLINE AddDefaults()
	{
		FindOrAllocate(GameStrings::Radiation());
	}

	COMPILETIMEEVAL OPTIONALINLINE WarheadTypeClass* GetWarhead() const
	{
		return RadWarhead.Get(RulesClass::Instance->RadSiteWarhead);
	}

	COMPILETIMEEVAL OPTIONALINLINE bool GetWarheadDetonate() const
	{
		return this->RadWarhead_Detonate.Get(RulesExtData::Instance()->RadWarhead_Detonate);
	}

	COMPILETIMEEVAL OPTIONALINLINE bool GetHasOwner(const Nullable<bool>&nOwner) const
	{
		return nOwner.Get(this->RadHasOwner.Get(RulesExtData::Instance()->RadHasOwner));
	}

	COMPILETIMEEVAL OPTIONALINLINE bool GetHasInvoker(const Nullable<bool>& nOwner) const
	{
		return nOwner.Get(this->RadHasInvoker.Get(RulesExtData::Instance()->RadHasInvoker));
	}

	COMPILETIMEEVAL OPTIONALINLINE bool GetHasOwner() const
	{
		return (this->RadHasOwner.Get(RulesExtData::Instance()->RadHasOwner));
	}

	COMPILETIMEEVAL OPTIONALINLINE bool GetHasInvoker() const
	{
		return (this->RadHasInvoker.Get(RulesExtData::Instance()->RadHasInvoker));
	}

	COMPILETIMEEVAL OPTIONALINLINE ColorStruct GetColor() const
	{
		return this->RadSiteColor.Get(RulesClass::Instance->RadColor);
	}

	COMPILETIMEEVAL OPTIONALINLINE int GetDurationMultiple() const
	{
		return this->DurationMultiple.Get(RulesClass::Instance->RadDurationMultiple);
	}

	COMPILETIMEEVAL OPTIONALINLINE int GetApplicationDelay()  const
	{
		return this->ApplicationDelay.Get(RulesClass::Instance->RadApplicationDelay);
	}

	COMPILETIMEEVAL OPTIONALINLINE int GetBuildingApplicationDelay() const
	{
		return this->BuildingApplicationDelay.Get(RulesExtData::Instance()->RadApplicationDelay_Building.Get());
	}

	COMPILETIMEEVAL OPTIONALINLINE int GetBuildingDamageMaxCount() const
	{
		return this->BuildingDamageMaxCount.Get(RulesExtData::Instance()->RadBuildingDamageMaxCount);
	}

	COMPILETIMEEVAL OPTIONALINLINE int GetLevelMax()  const
	{
		return this->LevelMax.Get(RulesClass::Instance->RadLevelMax);
	}

	COMPILETIMEEVAL OPTIONALINLINE int GetLevelDelay() const
	{
		return this->LevelDelay.Get(RulesClass::Instance->RadLevelDelay);
	}

	COMPILETIMEEVAL OPTIONALINLINE int GetLightDelay() const
	{
		return this->LightDelay.Get(RulesClass::Instance->RadLightDelay);
	}

	COMPILETIMEEVAL OPTIONALINLINE double GetLevelFactor() const
	{
		return this->LevelFactor.Get(RulesClass::Instance->RadLevelFactor);
	}

	COMPILETIMEEVAL OPTIONALINLINE double GetLightFactor() const
	{
		return this->LightFactor.Get(RulesClass::Instance->RadLightFactor);
	}

	COMPILETIMEEVAL OPTIONALINLINE double GetTintFactor() const
	{
		return this->TintFactor.Get(RulesClass::Instance->RadTintFactor);
	}

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};