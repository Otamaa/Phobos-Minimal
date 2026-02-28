#pragma once

#include <Misc/Kratos/Ext/ObjectType/FilterData.h>

enum class AffectWho : int
{
	MASTER = 0, STAND = 1, ALL = 2,
};

template <>
inline bool Parser<AffectWho>::TryParse(const char* pValue, AffectWho* outValue)
{
	switch (toupper(static_cast<unsigned char>(*pValue)))
	{
	case 'M':
		if (outValue)
		{
			*outValue = AffectWho::MASTER;
		}
		return true;
	case 'S':
		if (outValue)
		{
			*outValue = AffectWho::STAND;
		}
		return true;
	default:
		if (outValue)
		{
			*outValue = AffectWho::ALL;
		}
		return true;
	}
}

#define EFFECT_DATA(DATA_NAME) \
	virtual std::string GetEffectScriptName() override \
	{ \
		return std::string{ #DATA_NAME } + "Effect"; \
	} \
	virtual std::string GetStateScriptName() override \
	{ \
		return std::string{ #DATA_NAME } + "State"; \
	} \

class EffectData : public FilterData
{
public:
	virtual std::string GetEffectScriptName() { return ""; };
	virtual std::string GetStateScriptName() { return ""; };

	// 触发条件
	bool Powered = false; // 建筑需要使用电力

	bool CheckHealthPrecent = false; // 需要检查血量
	double ActiveWhenHealthPercent = 1; // 血量低于这个比例再触发
	double DeactiveWhenHealthPercent = 0; // 血量高于这个比例触发

	int TriggeredTimes = -1; // 触发次数
	AffectWho AffectWho = AffectWho::MASTER;
	bool DeactiveWhenCivilian = false;

	virtual void Read(INIBufferReader* reader) override
	{
		FilterData::Read(reader);
		Read(reader, "");
	}

	virtual void Read(INIBufferReader* reader, std::string title) override
	{
		FilterData::Read(reader, title);

		Enable = reader->Get(title + "Enable", Enable);

		Powered = reader->Get(title + "Powered", Powered);
		ActiveWhenHealthPercent = reader->GetPercent(title + "ActiveWhenHealthPrecent", ActiveWhenHealthPercent); // 兼容拼写错误
		DeactiveWhenHealthPercent = reader->GetPercent(title + "DeactiveWhenHealthPrecent", DeactiveWhenHealthPercent); // 兼容拼写错误
		ActiveWhenHealthPercent = reader->GetPercent(title + "ActiveWhenHealthPercent", ActiveWhenHealthPercent); // 正确拼写
		DeactiveWhenHealthPercent = reader->GetPercent(title + "DeactiveWhenHealthPercent", DeactiveWhenHealthPercent); // 正确拼写


		CheckHealthPrecent = (ActiveWhenHealthPercent > 0.0 && ActiveWhenHealthPercent < 1.0)
			|| (DeactiveWhenHealthPercent > 0.0 && DeactiveWhenHealthPercent < 1.0);

		TriggeredTimes = reader->Get(title + "TriggeredTimes", TriggeredTimes);
		AffectWho = reader->Get(title + "AffectWho", AffectWho);
		DeactiveWhenCivilian = reader->Get(title + "DeactiveWhenCivilian", DeactiveWhenCivilian);
	}

#pragma region save/load
	template <typename T>
	bool Serialize(T& stream)
	{
		return stream
			.Process(this->Powered)

			.Process(this->CheckHealthPrecent)
			.Process(this->ActiveWhenHealthPercent)
			.Process(this->DeactiveWhenHealthPercent)

			.Process(this->TriggeredTimes)
			.Process(this->AffectWho)
			.Process(this->DeactiveWhenCivilian)
			.Success();
	};

	virtual bool Load(PhobosStreamReader& stream, bool registerForChange) override
	{
		FilterData::Load(stream, registerForChange);
		return this->Serialize(stream);
	}
	virtual bool Save(PhobosStreamWriter& stream) const override
	{
		FilterData::Save(stream);
		return const_cast<EffectData*>(this)->Serialize(stream);
	}
#pragma endregion
};
