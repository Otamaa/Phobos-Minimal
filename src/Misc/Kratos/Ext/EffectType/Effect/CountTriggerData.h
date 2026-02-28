#pragma once

#include <string>
#include <vector>

#include <GeneralStructures.h>

#include <Misc/Kratos/Common/INI/INIConfig.h>

#include <Misc/Kratos/Ext/EffectType/Effect/EffectData.h>
#include <Misc/Kratos/Ext/Helper/MathEx.h>
#include "CounterData.h"

enum class CountTriggerWho : int
{
	ME = 0,
	SOURCE = 1,
	COUNTER = 2,
};

template <>
inline bool Parser<CountTriggerWho>::TryParse(const char* pValue, CountTriggerWho* outValue)
{
	switch (toupper(static_cast<unsigned char>(*pValue)))
	{
	case 'S':
		if (outValue)
		{
			*outValue = CountTriggerWho::SOURCE;
		}
		return true;
	case 'C':
		if (outValue)
		{
			*outValue = CountTriggerWho::COUNTER;
		}
		return true;
	case 'M':
		if (outValue)
		{
			*outValue = CountTriggerWho::ME;
		}
		return true;
	}
	return false;
}

class CountTriggerEntity
{
public:
	bool Enable = false;

	Point2D Range = { 0, -1 };
	int TriggeredTimes = -1; // 触发次数

	// 触发计数操作
	double Num = 0;
	CounterType NumType = CounterType::Number;
	CountTriggerWho NumFrom = CountTriggerWho::ME;
	CounterAction Action = CounterAction::ADD;
	bool ResetNum = false;
	bool RemoveCounter = false;

	// 触发AE操作
	bool Attach = false;
	std::vector<std::string> AttachEffects{};
	std::vector<double> AttachChances{};
	CountTriggerWho AttachTo = CountTriggerWho::ME;
	CountTriggerWho AttachFrom = CountTriggerWho::SOURCE;

	bool Remove = false;
	std::vector<std::string> RemoveEffects{};
	std::vector<int> RemoveEffectsLevel{};
	std::vector<std::string> RemoveEffectsWithMarks{};
	bool RemoveEffectsSkipNext = false;
	CountTriggerWho RemoveWho = CountTriggerWho::ME;

	virtual void Read(INIBufferReader* reader, std::string title)
	{
		Range = reader->Get(title + "Range", Range);
		TriggeredTimes = reader->Get(title + "TriggeredTimes", TriggeredTimes);

		// 初始数字特殊格式
		std::string numStr{ "" };
		numStr = reader->Get(title + "Num", numStr);
		if (IsNotNone(numStr))
		{
			if (std::regex_match(numStr, INIReader::Number))
			{
				int buffer = 0;
				const char* pFmt = "%d";
				if (sscanf_s(numStr.c_str(), pFmt, &buffer) == 1)
				{
					Num = buffer;
					NumType = CounterType::Number;
				}
			}
			else
			{
				const char v = *uppercase(numStr).substr(0, 1).c_str();
				switch (v)
				{
				case 'H': // HP
					Num = 0;
					NumType = CounterType::HP;
					break;
				case 'M': // MAXHP
					Num = 0;
					NumType = CounterType::MaxHP;
					break;
				}
			}
		}
		NumFrom = reader->Get(title + "NumFrom", NumFrom);

		Action = reader->Get(title + "Action", Action);
		ResetNum = reader->Get(title + "ResetNum", ResetNum);
		RemoveCounter = reader->Get(title + "RemoveCounter", RemoveCounter);

		AttachEffects = reader->GetList(title + "AttachEffects", AttachEffects);
		ClearIfGetNone(AttachEffects);
		AttachChances = reader->GetChanceList(title + "AttachChances", AttachChances);
		Attach = !AttachEffects.empty();
		AttachTo = reader->Get(title + "AttachTo", AttachTo);
		AttachFrom = reader->Get(title + "AttachFrom", AttachFrom);

		RemoveEffects = reader->GetList(title + "RemoveEffects", RemoveEffects);
		ClearIfGetNone(RemoveEffects);
		RemoveEffectsLevel = reader->GetList(title + "RemoveEffectsLevel", RemoveEffectsLevel);
		RemoveEffectsWithMarks = reader->GetList(title + "RemoveEffectsWithMarks", RemoveEffectsWithMarks);
		ClearIfGetNone(RemoveEffectsWithMarks);
		Remove = !RemoveEffects.empty() || !RemoveEffectsWithMarks.empty();
		RemoveEffectsSkipNext = reader->Get(title + "RemoveEffectsSkipNext", RemoveEffectsSkipNext);
		RemoveWho = reader->Get(title + "RemoveWho", RemoveWho);

		Enable = Num != 0 || NumType != CounterType::Number || ResetNum || RemoveCounter || Attach || Remove;
	}

#pragma region save/load
	template <typename T>
	bool Serialize(T& stream)
	{
		return stream
			.Process(this->Enable)
			.Process(this->Range)
			.Process(this->TriggeredTimes)

			.Process(this->NumType)
			.Process(this->Num)
			.Process(this->Action)
			.Process(this->ResetNum)
			.Process(this->RemoveCounter)

			.Process(this->Attach)
			.Process(this->AttachEffects)
			.Process(this->AttachChances)
			.Process(this->AttachTo)
			.Process(this->AttachFrom)

			.Process(this->Remove)
			.Process(this->RemoveEffects)
			.Process(this->RemoveEffectsLevel)
			.Process(this->RemoveEffectsWithMarks)
			.Process(this->RemoveEffectsSkipNext)
			.Process(this->RemoveWho)

			.Success();
	};

	virtual bool Load(PhobosStreamReader& stream, bool registerForChange)
	{
		return this->Serialize(stream);
	}
	virtual bool Save(PhobosStreamWriter& stream) const
	{
		return const_cast<CountTriggerEntity*>(this)->Serialize(stream);
	}
#pragma endregion
};

class CountTriggerData : public EffectData
{
public:
	EFFECT_DATA(CountTrigger);

	std::string Watch{};

	std::vector<CountTriggerEntity> Actions{}; // 触发效果列表

	virtual void Read(INIBufferReader* reader) override
	{
		Read(reader, "CountTrigger.");
	}

	virtual void Read(INIBufferReader* reader, std::string title) override
	{
		EffectData::Read(reader, title);

		Watch = reader->Get(title + "Mark", Watch);
		Watch = reader->Get(title + "Watch", Watch);

		// 读取无序号的
		CountTriggerEntity defaultEntity;
		defaultEntity.Read(reader, title);
		if (defaultEntity.Enable)
		{
			Actions.push_back(defaultEntity);
		}
		// 读取有序号的
		for (int i = 0; i < 128; i++)
		{
			CountTriggerEntity entity{};
			entity.Read(reader, "CountTrigger" + std::to_string(i) + ".");
			if (entity.Enable)
			{
				Actions.push_back(entity);
			}
		}

		Enable = IsNotNone(Watch) && !Actions.empty();
	}

#pragma region save/load
	template <typename T>
	bool Serialize(T& stream)
	{
		return stream
			.Process(this->Watch)
			.Process(this->Actions)

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
		return const_cast<CountTriggerData*>(this)->Serialize(stream);
	}
#pragma endregion
};
