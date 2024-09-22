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

	Valueable<int> Weight { 0 };
	Valueable<AnimTypeClass*> Anim { nullptr };
	Valueable<double> Argument { 0.0 };
	Valueable<bool> Naval { false };
	ValueableIdx<VocClass> Sound { -1 };

    CrateTypeClass(const char* const pTitle): Enumerable<CrateTypeClass>(pTitle)
		, Weight { }
		, Anim { nullptr }
		, Argument { 0.0 }
		, Naval { false }
		, Sound {}
	{ }

	static void ReadListFromINI(CCINIClass* pINI);
	static void constexpr inline AddDefaults(){
		for (auto crate : Powerups::Effects){
			FindOrAllocate(crate);
		}
	}

	static void ReadFromINIList(CCINIClass* pINI);

	void LoadFromINI(CCINIClass *pINI);
	void LoadFromStream(PhobosStreamReader &Stm);
	void SaveToStream(PhobosStreamWriter &Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};