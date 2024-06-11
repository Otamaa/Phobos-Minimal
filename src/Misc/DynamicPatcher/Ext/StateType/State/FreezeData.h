#pragma once

#include <string>
#include <vector>

#include <GeneralStructures.h>

#include <Misc/DynamicPatcher/Ext/EffectType/Effect/EffectData.h>
#include <Misc/DynamicPatcher/Ext/Helper/CastEx.h>


class FreezeData : public EffectData
{
public:
	EFFECT_DATA(Freeze);

	FreezeData() : EffectData()
	{
		AffectWho = AffectWho::ALL;
	}

	virtual void Read(INIBufferReader* reader) override
	{
		EffectData::Read(reader);
		Read(reader, "Freeze.");
	}

	virtual void Read(INIBufferReader* reader, std::string title) override
	{
		EffectData::Read(reader, title);
	}

#pragma region save/load
	template <typename T>
	bool Serialize(T& stream)
	{
		return stream
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
		return const_cast<FreezeData*>(this)->Serialize(stream);
	}
#pragma endregion
};
