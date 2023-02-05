#pragma once

#include "../../EffectBaseType.h"

class StandType : public EffectBaseType
{
	virtual void Read() override { }
	virtual bool Load(PhobosStreamReader& stm, bool RegisterForChange) override { return true; }
	virtual bool Save(PhobosStreamWriter& stm) override { return true; }

};