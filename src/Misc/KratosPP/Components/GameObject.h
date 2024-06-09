#pragma once
#include "Component.h"

class GameObject : public Component
{
public:
	bool ExtChanged = false;

	GameObject() : Component()
	{
		this->Name = ComponentName(GameObject);
	}

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
