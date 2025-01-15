#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/TemplateDefB.h>

class AnimTypeClass;
class VocClass;
class VoxClass;
class WeaponTypeClass;

class CrateTypeClass final : public Enumerable<CrateTypeClass>
{
public:

	Valueable<int> Weight;
	Valueable<AnimTypeClass*> Anim;
	Valueable<double> Argument;
	Valueable<bool> Naval;
	ValueableIdx<VocClass> Sound;
	SpeedType Speed;

    CrateTypeClass(const char* const pTitle): Enumerable<CrateTypeClass>(pTitle)
		, Weight { }
		, Anim { nullptr }
		, Argument { 0.0 }
		, Naval { false }
		, Sound { -1 }
		, Speed { SpeedType::Track }
	{ }

	static void ReadListFromINI(CCINIClass* pINI);
	static void OPTIONALINLINE AddDefaults(){
		if (Array.empty()){
			Array.reserve(Powerups::Effects.size());

			for (auto crate : Powerups::Effects){
				Debug::Log("Creating default Crate of [%s]\n" , crate);
				Allocate(crate);
			}
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