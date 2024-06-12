#include "EffectScript.h"

#include <Misc/DynamicPatcher/Ext/Helper/Status.h>

AttachEffectScript* EffectScript::GetAE()
{
	if (!_ae && _parent->c_Type & ComponentType::AE_Effect) {
		_ae = reinterpret_cast<AttachEffectScript*>(_parent);
	}

	return _ae;
}

void EffectScript::Start()
{
	_started = true;
	OnStart();
}

void EffectScript::Pause()
{
	_pause = true;
	OnPause();
}

void EffectScript::Recover()
{
	if (!_started)
	{
		Start();
	}
	else
	{
		OnRecover();
		_pause = false;
	}
}
