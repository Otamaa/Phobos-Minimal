#pragma once

#include <EBolt.h>
#include <Utilities/TemplateDefB.h>

class WeaponTypeClass;
class TechnoClass;
class ParticleSystemTypeClass;
struct EboltExtData {
	int Color[3] {};
	bool Disable[3] { false };
	int Arcs { 8 };
	int BurstIndex { 0 };
	bool ParticleSysEnabled { true };
	ParticleSystemTypeClass* pSys { nullptr };

	OPTIONALINLINE bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return Stm
			.Process(this->Color)
			.Process(this->Disable)
			.Process(this->Arcs)
			.Process(this->BurstIndex)
			.Process(this->ParticleSysEnabled)
			.Process(this->pSys)
			.Success()
			;
	}

	OPTIONALINLINE bool Save(PhobosStreamWriter& Stm) const
	{
		return Stm
			.Process(this->Color)
			.Process(this->Disable)
			.Process(this->Arcs)
			.Process(this->BurstIndex)
			.Process(this->ParticleSysEnabled)
			.Process(this->pSys)
			.Success()
			;
	}

public:
	static void GetColors(int(&color)[3], EBolt* pBolt , Nullable<ColorStruct>& clr1, Nullable<ColorStruct>& clr2, Nullable<ColorStruct>& clr3);

	static EBolt* _CreateOneOf(WeaponTypeClass* pWeapon, TechnoClass* pFirer);
	static EBolt* _CreateOneOf(bool disable1, bool disable2, bool dosable3, bool alternateColor, int arch, int lifetime, Nullable<ColorStruct>& clr1, Nullable<ColorStruct>& clr2, Nullable<ColorStruct>& clr3);

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
	static void Clear();
public:
	static PhobosMap<EBolt*, EboltExtData> Container;
};

