#pragma once

#include "ScriptComponent.h"
#include <Misc/DynamicPatcher/Extension/AnimExt.h>
#include <AnimClass.h>

class AnimScript : public ScriptComponent, public IAnimScript
{
public:
	SCRIPT_COMPONENT(AnimScript, AnimClass, AnimExt, pAnim);

	virtual void Clean() override { ScriptComponent::Clean(); }
};

