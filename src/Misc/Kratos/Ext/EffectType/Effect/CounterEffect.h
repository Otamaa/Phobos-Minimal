#pragma once

#include <string>
#include <vector>

#include <GeneralDefinitions.h>
#include <AnimClass.h>

#include "../EffectScript.h"
#include "CounterData.h"


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
class CounterEffect : public EffectScript
{
public:
	EFFECT_SCRIPT(Counter);

	virtual void Clean() override
	{
		EffectScript::Clean();
		CountNum = 0;
	}

	virtual void OnStart() override;
	virtual void OnPause() override;
	virtual void OnRecover() override;

	virtual void OnUpdate() override;
	virtual void OnWarpUpdate() override;

	virtual void OnReceiveDamageReal(int* pRealDamage, WarheadTypeClass* pWH, TechnoClass* pAttacker, HouseClass* pAttackingHouse) override;

	void ModifyCount(CounterAction action, double num);
	void ResetNum();
	void RemoveCounter();

	// 计数器数值，支持小数
	double CountNum = 0;

#pragma region Save/Load
	template <typename T>
	bool Serialize(T& stream) {
		return stream
			.Process(this->CountNum)
			.Success();
	};

	virtual bool Load(PhobosStreamReader& stream, bool registerForChange) override
	{
		EffectScript::Load(stream, registerForChange);
		bool result = this->Serialize(stream);
		// 向AE管理器注册自己
		AddSelfToManager();
		return result;
	}
	virtual bool Save(PhobosStreamWriter& stream) const override
	{
		EffectScript::Save(stream);
		return const_cast<CounterEffect*>(this)->Serialize(stream);
	}
#pragma endregion
private:
	void Watch();

	int CalculateRemainingDamage(int Damage);

	void AddSelfToManager();
	void RemoveSelfFromManager();
};
