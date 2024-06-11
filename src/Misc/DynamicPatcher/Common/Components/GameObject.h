#pragma once

#include <list>
#include <vector>

#include "Component.h"

#include <Common/MyDelegate.h>

using namespace Delegate;

class GameObject : public Component
{
public:
	DECLARE_COMPONENT(GameObject, Component);

	bool ExtChanged = false;

	virtual void Clean() override
	{
		Component::Clean();
	};

	GameObject* GetAwaked()
	{
		EnsureAwaked();
		return this;
	}

	virtual void OnForeachEnd() override
	{
		if (ExtChanged)
		{
			ExtChanged = false;
			for (Component* c : _children)
			{
				c->Foreach([](Component* cc) { cc->ExtChanged(); });
			}
		}
	}
};
