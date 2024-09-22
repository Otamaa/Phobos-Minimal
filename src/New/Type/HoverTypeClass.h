#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/GeneralUtils.h>

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

	constexpr inline AnimTypeClass* GetAboveWaterAnim() const {
		return this->AboveWaterAnim.Get(RulesClass::Instance->Wake);
	}

	constexpr inline int GetScoldSound() const {
		return this->ScoldSound.Get(RulesClass::Instance->ScoldSound);
	}

	constexpr inline 	int GetHeight() const {
		return this->HoverHeight.Get(RulesClass::Instance->HoverHeight);
	}

	constexpr inline double GetBob() const {
		return this->HoverBob.Get(RulesClass::Instance->HoverBob);
	}

	constexpr inline double GetDampen() const {
		return this->HoverDampen.Get(RulesClass::Instance->HoverDampen);
	}

	constexpr inline double GetAccel() const {
		return this->HoverAcceleration.Get(RulesClass::Instance->HoverAcceleration);
	}

	constexpr inline double GetBrake() const {
		return this->HoverBrake.Get(RulesClass::Instance->HoverBrake);
	}

	constexpr inline double GetBoost() const {
		return this->HoverBoost.Get(RulesClass::Instance->HoverBoost);
	}

	static void constexpr inline AddDefaults() {
		FindOrAllocate(DEFAULT_STR2);
	}

	constexpr static const HoverTypeClass* GetMyHover(int nIdx)
	{ return HoverTypeClass::FindFromIndex(nIdx); }

	void LoadFromINI(CCINIClass *pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};