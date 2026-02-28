#pragma once

#include <string>
#include <vector>

#include <GeneralStructures.h>

#include <Misc/Kratos/Ext/EffectType/Effect/EffectData.h>
#include <Misc/Kratos/Ext/Helper/CastEx.h>


class DisableWeaponData : public EffectData
{
public:
	EFFECT_DATA(DisableWeapon);

	bool Disable = false;
	bool DisableWithTarget = false;

	std::vector<LandType> OnLandTypes{};

	DisableWeaponData() : EffectData()
	{
		AffectWho = AffectWho::ALL;
	}

	virtual void Read(INIBufferReader* reader) override
	{
		EffectData::Read(reader);
		Read(reader, "Weapon.");
	}

	virtual void Read(INIBufferReader* reader, std::string title) override
	{
		EffectData::Read(reader, title);

		Disable = reader->Get(title + "Disable", false);
		OnLandTypes = reader->GetList(title + "DisableOnLands", OnLandTypes);
		DisableWithTarget = reader->Get(title + "DisableWithTarget", false);

		Enable = Disable;
	}

#pragma region save/load
	template <typename T>
	bool Serialize(T& stream)
	{
		return stream
			.Process(this->Disable)
			.Process(this->OnLandTypes)
			.Process(this->DisableWithTarget)
			.Success();
	};

	virtual bool Load(PhobosStreamReader& stream, bool registerForChange) override
	{
		EffectData::Load(stream, registerForChange);
		return this->Serialize(stream);
	}
	virtual bool Save(PhobosStreamWriter& stream) const override
	{
		EffectData::Save(stream);
		return const_cast<DisableWeaponData*>(this)->Serialize(stream);
	}
#pragma endregion
};
