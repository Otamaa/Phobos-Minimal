#pragma once

#include <Utilities/TemplateDefB.h>

class PhobosStreamReader;
class PhobosStreamWriter;

struct DroppodProperties
{
	Valueable<SHPCaches*> Droppod_PodImage_Infantry {};
	Valueable<AnimTypeClass*> Droppod_Puff {};
	Valueable<double> Droppod_Angle {};
	Valueable<int> Droppod_Speed {};
	Valueable<int> Droppod_Height {};
	Valueable<WeaponTypeClass*> Droppod_Weapon {};
	ValueableVector<AnimTypeClass*> Droppod_GroundPodAnim {};

	Valueable<AnimTypeClass*> Droppod_Trailer {};
	Valueable<bool> Droppod_Trailer_Attached { false };
	Valueable<int> Droppod_Trailer_SpawnDelay { 6 };
	Valueable<AnimTypeClass*> Droppod_AtmosphereEntry {};

public:

	void Read(INI_EX& iniEx, const char* pSection);
	void Initialize();
	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;
};

struct NullableDroppodProperties
{
	Nullable<SHPCaches*> Droppod_PodImage_Infantry {};
	Nullable<AnimTypeClass*> Droppod_Puff {};
	Nullable<double> Droppod_Angle {};
	Nullable<int> Droppod_Speed {};
	Nullable<int> Droppod_Height {};
	Nullable<WeaponTypeClass*> Droppod_Weapon {};
	NullableVector<AnimTypeClass*> Droppod_GroundPodAnim {};

	Nullable<AnimTypeClass*> Droppod_Trailer {};
	Nullable<bool> Droppod_Trailer_Attached { };
	Nullable<int> Droppod_Trailer_SpawnDelay {  };
	Nullable<AnimTypeClass*> Droppod_AtmosphereEntry {};

public:

	void Read(INI_EX& iniEx, const char* pSection);
	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;
};