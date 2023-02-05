#pragma once

#include <Utilities/SavegameDef.h>
#include <Utilities/TemplateDef.h>

class EffectBaseType
{
	virtual void Read() = 0;
	virtual bool Load(PhobosStreamReader& stm, bool RegisterForChange) = 0;
	virtual bool Save(PhobosStreamWriter& stm) = 0;
};