#pragma once

#include <EBolt.h>
#include <Utilities/TemplateDefB.h>

class WeaponTypeClass;
class TechnoClass;
class ParticleSystemTypeClass;
struct EboltExtData
{
	// ============================================================
	// 8-byte aligned: Pointer
	// ============================================================
	ParticleSystemTypeClass* pSys { nullptr };

	// ============================================================
	// 4-byte aligned: int arrays and ints
	// ============================================================
	int Color[3] {};
	int Arcs { 8 };
	int BurstIndex { 0 };

	// ============================================================
	// 1-byte aligned: bool (packed at the end)
	// ============================================================
	bool Disable[3] { false };
	bool ParticleSysEnabled { true };
	// 4 bools = 4 bytes, naturally aligned

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
	static EBolt* _CreateOneOf(bool disable1, bool disable2, bool disable3, bool alternateColor, int arch, int lifetime, Nullable<ColorStruct>& clr1, Nullable<ColorStruct>& clr2, Nullable<ColorStruct>& clr3);

	static void Clear();
public:
	static PhobosMap<EBolt*, EboltExtData> Container;
};

