#pragma once

#include <string>
#include <vector>
#include <map>
#include <type_traits>

#include <GeneralStructures.h>

#include <Utilities/Debug.h>

#include <Misc/DynamicPatcher/Common/Components/ScriptComponent.h>
#include <Misc/DynamicPatcher/Common/EventSystems/EventSystem.h>

#include <Misc/DynamicPatcher/Ext/BulletType/BulletStatus.h>
#include <Misc/DynamicPatcher/Ext/TechnoType/TechnoStatus.h>

#include "StateEffectScript.h"


#define STATE_EFFECT_DEFINE(STATE_NAME) \
class STATE_NAME ## Effect : public StateEffect<STATE_NAME ## Data> \
{ \
public: \
	STATE_EFFECT_SCRIPT(STATE_NAME); \
	virtual ObjectScripts GetCurrentScriptType() override { return ObjectScripts::##STATE_NAME##; }\
	virtual StateEffectTypes GetEffecType() { return StateEffectTypes::##STATE_NAME##; }\
	virtual IStateScript* GetState(TechnoStatus* status) override \
	{ \
		IStateScript* state = nullptr; \
		if (status) \
		{ \
			status->TryGetState(StateEffectTypes::##STATE_NAME##, state); \
		} \
		return state; \
	} \
	virtual IStateScript* GetState(BulletStatus* status) override \
	{ \
		IStateScript* state = nullptr; \
		if (status) \
		{ \
			status->TryGetState(StateEffectTypes::##STATE_NAME##, state); \
		} \
		return state; \
	} \
	virtual void Clean() override { StateEffect<STATE_NAME ## Data>::Clean(); } \
}; \

// TODO Add State effects

STATE_EFFECT_DEFINE(AntiBullet);
STATE_EFFECT_DEFINE(BlackHole);
STATE_EFFECT_DEFINE(DamageReaction);
STATE_EFFECT_DEFINE(Deselect);
STATE_EFFECT_DEFINE(DestroyAnim);
STATE_EFFECT_DEFINE(DestroySelf);
STATE_EFFECT_DEFINE(DisableWeapon);
STATE_EFFECT_DEFINE(ECM);
STATE_EFFECT_DEFINE(Freeze);
STATE_EFFECT_DEFINE(GiftBox);
STATE_EFFECT_DEFINE(NoMoneyNoTalk);
STATE_EFFECT_DEFINE(OverrideWeapon);
STATE_EFFECT_DEFINE(Paintball);
STATE_EFFECT_DEFINE(Pump);
STATE_EFFECT_DEFINE(Scatter);
STATE_EFFECT_DEFINE(Teleport);
STATE_EFFECT_DEFINE(Transform);



