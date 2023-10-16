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

	virtual ~HoverTypeClass() override = default;

	inline AnimTypeClass* GetAboveWaterAnim() const {
		return this->AboveWaterAnim.Get(RulesClass::Instance->Wake);
	}

	inline int GetScoldSound() const {
		return this->ScoldSound.Get(RulesClass::Instance->ScoldSound);
	}

	inline 	int GetHeight() const {
		return this->HoverHeight.Get(RulesClass::Instance->HoverHeight);
	}

	inline double GetBob() const {
		return this->HoverBob.Get(RulesClass::Instance->HoverBob);
	}

	inline double GetDampen() const {
		return this->HoverDampen.Get(RulesClass::Instance->HoverDampen);
	}

	inline double GetAccel() const {
		return this->HoverAcceleration.Get(RulesClass::Instance->HoverAcceleration);
	}

	inline double GetBrake() const {
		return this->HoverBrake.Get(RulesClass::Instance->HoverBrake);
	}

	inline double GetBoost() const {
		return this->HoverBoost.Get(RulesClass::Instance->HoverBoost);
	}

	inline static void AddDefaults() {
		FindOrAllocate(DEFAULT_STR2);
	}

	static const HoverTypeClass* GetMyHover(int nIdx)
	{ return HoverTypeClass::FindFromIndex(nIdx); }

	virtual void LoadFromINI(CCINIClass *pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm) override;
	virtual void SaveToStream(PhobosStreamWriter& Stm) override;

private:
	template <typename T>
	void Serialize(T& Stm);
};