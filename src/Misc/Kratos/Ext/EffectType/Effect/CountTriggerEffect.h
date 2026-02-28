#pragma once

#include <string>
#include <vector>

#include <GeneralDefinitions.h>
#include <AnimClass.h>

#include "../EffectScript.h"
#include "CountTriggerData.h"


/// @brief EffectScript
/// GameObject
///		|__ AttachEffect
///				|__ AttachEffectScript#0
///						|__ EffectScript#0
///						|__ EffectScript#1
///				|__ AttachEffectScript#1
///						|__ EffectScript#0
///						|__ EffectScript#1
///						|__ EffectScript#2
class CountTriggerEffect : public EffectScript
{
public:
	EFFECT_SCRIPT(CountTrigger);

	virtual void Clean() override
	{
		EffectScript::Clean();

		_count.clear();
	}

	virtual void OnUpdate() override;
	virtual void OnWarpUpdate() override;

#pragma region Save/Load
	template <typename T>
	bool Serialize(T& stream) {
		return stream
			.Process(this->_count)
			.Success();
	};

	virtual bool Load(PhobosStreamReader& stream, bool registerForChange) override
	{
		EffectScript::Load(stream, registerForChange);
		return this->Serialize(stream);
	}
	virtual bool Save(PhobosStreamWriter& stream) const override
	{
		EffectScript::Save(stream);
		return const_cast<CountTriggerEffect*>(this)->Serialize(stream);
	}
#pragma endregion
private:
	void Watch();
	bool CanActive(double num, Point2D range);

	// 计数器触发次数
	std::map<int, int> _count{};
};
