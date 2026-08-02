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
	static COMPILETIMEEVAL const char* MainSection = "CrateTypes";
	static COMPILETIMEEVAL const char* ClassName = "CrateTypeClass";

public:

	Valueable<int> Weight {};
	Valueable<AnimTypeClass*> Anim {};
	Valueable<double> Argument {};
	Valueable<bool> Naval {};
	ValueableIdx<VocClass> Sound {};
	ValueableIdx<VoxClass> Eva {};
	SpeedType Speed { SpeedType::Track };

    CrateTypeClass(const char* const pTitle): Enumerable<CrateTypeClass>(pTitle) { }
	virtual ~CrateTypeClass() = default;

	static void ReadListFromINI(CCINIClass* pINI);
	static void AddDefaults();
	static void ReadFromPowerups(CCINIClass* pINI);
	static void ReadFromINIList(CCINIClass* pINI);

	void LoadFromINI(CCINIClass *pINI);
	void LoadFromStream(PhobosStreamReader &Stm);
	void SaveToStream(PhobosStreamWriter &Stm);

	void PlayAllAffects(CoordStruct loc, CoordStruct locsound, bool isPlayerControlled) const;
private:
	template <typename T>
	void Serialize(T& Stm);
};