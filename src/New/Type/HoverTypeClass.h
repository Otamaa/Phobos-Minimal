#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/TemplateDef.h>

class HoverTypeClass final : public Enumerable<HoverTypeClass>
{
private:

	Nullable<AnimTypeClass*> AboveWaterAnim;
	NullableIdx<VocClass> ScoldSound;
	Nullable<int> HoverHeight;
	Nullable<double> HoverBob;
	Nullable<double> HoverDampen;
	Nullable<double> HoverAcceleration;
	Nullable<double> HoverBrake;
	Nullable<double> HoverBoost;

public:

	HoverTypeClass(const char* const pTitle) : Enumerable<HoverTypeClass> { pTitle }
		, AboveWaterAnim {}
		, ScoldSound {}
		, HoverHeight {}
		, HoverBob {}
		, HoverDampen {}
		, HoverAcceleration {}
		, HoverBrake {}
		, HoverBoost()
	{ }

	COMPILETIMEEVAL OPTIONALINLINE AnimTypeClass* GetAboveWaterAnim() const {
		return this->AboveWaterAnim.Get(RulesClass::Instance->Wake);
	}

	COMPILETIMEEVAL OPTIONALINLINE int GetScoldSound() const {
		return this->ScoldSound.Get(RulesClass::Instance->ScoldSound);
	}

	COMPILETIMEEVAL OPTIONALINLINE 	int GetHeight() const {
		return this->HoverHeight.Get(RulesClass::Instance->HoverHeight);
	}

	COMPILETIMEEVAL OPTIONALINLINE double GetBob() const {
		return this->HoverBob.Get(RulesClass::Instance->HoverBob);
	}

	COMPILETIMEEVAL OPTIONALINLINE double GetDampen() const {
		return this->HoverDampen.Get(RulesClass::Instance->HoverDampen);
	}

	COMPILETIMEEVAL OPTIONALINLINE double GetAccel() const {
		return this->HoverAcceleration.Get(RulesClass::Instance->HoverAcceleration);
	}

	COMPILETIMEEVAL OPTIONALINLINE double GetBrake() const {
		return this->HoverBrake.Get(RulesClass::Instance->HoverBrake);
	}

	COMPILETIMEEVAL OPTIONALINLINE double GetBoost() const {
		return this->HoverBoost.Get(RulesClass::Instance->HoverBoost);
	}

	static void COMPILETIMEEVAL OPTIONALINLINE AddDefaults() {
		FindOrAllocate(DEFAULT_STR2);
	}

	COMPILETIMEEVAL static const HoverTypeClass* GetMyHover(int nIdx)
	{ return HoverTypeClass::FindFromIndex(nIdx); }

	void LoadFromINI(CCINIClass *pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};
