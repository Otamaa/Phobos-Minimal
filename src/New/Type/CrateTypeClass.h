#pragma once
#include <SuperWeaponTypeClass.h>

#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>
#include <Utilities/TemplateDefB.h>
#include <Utilities/GeneralUtils.h>

class AnimTypeClass;
class VocClass;
class VoxClass;
class WeaponTypeClass;

class CrateTypeClass final : public Enumerable<CrateTypeClass>
{
public:

	Valueable<SuperWeaponTypeClass*> Super { nullptr };
	Valueable<WeaponTypeClass*> WeaponType { nullptr };
	Valueable<bool> SuperGrant { false };
	Valueable<int> Chance { 0 };
	Valueable<AnimTypeClass*> Anim { nullptr };
	Valueable<NewCrateType> Type { NewCrateType::Money };
	Valueable<bool> AllowWater { false };
	ValueableIdx<VocClass> Sound { -1 };
    ValueableIdx<VoxClass> Eva { -1 };
	ValueableVector<UnitTypeClass*> Unit { };
    Valueable<int> MoneyMin { 0 };
    Valueable<int> MoneyMax { 0 };

    CrateTypeClass(const char* const pTitle): Enumerable<CrateTypeClass>(pTitle)
    { }

	virtual ~CrateTypeClass() override = default;
	virtual void LoadFromINI(CCINIClass *pINI) override;
	virtual void LoadFromStream(PhobosStreamReader &Stm) override;
	virtual void SaveToStream(PhobosStreamWriter &Stm) override;

private:
	template <typename T>
	void Serialize(T& Stm);
};