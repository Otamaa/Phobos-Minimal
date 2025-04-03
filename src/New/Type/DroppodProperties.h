#pragma once

#include <Utilities/TemplateDefB.h>
#include <Utilities/SavegameDef.h>

struct DroppodProperties
{
	Valueable<SHPStruct*> Droppod_PodImage_Infantry {};
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

	void Read(INI_EX& iniEx, const char* pSection);

	void Initialize();

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{

		return Stm
			.Process(this->Droppod_PodImage_Infantry)
			.Process(this->Droppod_Puff)
			.Process(this->Droppod_Angle)
			.Process(this->Droppod_Speed)
			.Process(this->Droppod_Weapon)
			.Process(this->Droppod_GroundPodAnim)
			.Process(this->Droppod_Trailer)
			.Process(this->Droppod_Trailer_Attached)
			.Process(this->Droppod_Trailer_SpawnDelay)
			.Process(this->Droppod_AtmosphereEntry)
			.Success();
	}

	bool Save(PhobosStreamWriter& Stm) const
	{
		return Stm
			.Process(this->Droppod_PodImage_Infantry)
			.Process(this->Droppod_Puff)
			.Process(this->Droppod_Angle)
			.Process(this->Droppod_Speed)
			.Process(this->Droppod_Weapon)
			.Process(this->Droppod_GroundPodAnim)
			.Process(this->Droppod_Trailer)
			.Process(this->Droppod_Trailer_Attached)
			.Process(this->Droppod_Trailer_SpawnDelay)
			.Process(this->Droppod_AtmosphereEntry)
			.Success();
	}
};

struct NullableDroppodProperties
{
	Nullable<SHPStruct*> Droppod_PodImage_Infantry {};
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

	void Read(INI_EX& iniEx, const char* pSection);

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{

		return Stm
			.Process(this->Droppod_PodImage_Infantry)
			.Process(this->Droppod_Puff)
			.Process(this->Droppod_Angle)
			.Process(this->Droppod_Speed)
			.Process(this->Droppod_Weapon)
			.Process(this->Droppod_GroundPodAnim)
			.Process(this->Droppod_Trailer)
			.Process(this->Droppod_Trailer_Attached)
			.Process(this->Droppod_Trailer_SpawnDelay)
			.Process(this->Droppod_AtmosphereEntry)
			.Success();
	}

	bool Save(PhobosStreamWriter& Stm) const
	{
		return Stm
			.Process(this->Droppod_PodImage_Infantry)
			.Process(this->Droppod_Puff)
			.Process(this->Droppod_Angle)
			.Process(this->Droppod_Speed)
			.Process(this->Droppod_Weapon)
			.Process(this->Droppod_GroundPodAnim)
			.Process(this->Droppod_Trailer)
			.Process(this->Droppod_Trailer_Attached)
			.Process(this->Droppod_Trailer_SpawnDelay)
			.Process(this->Droppod_AtmosphereEntry)
			.Success();
	}
};
