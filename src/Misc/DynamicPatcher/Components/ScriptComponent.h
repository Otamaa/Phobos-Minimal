#pragma once

#include "Component.h"

class GameObject;
class ScriptComponent : public Component
{
public:
	virtual GameObject* GetGameObject() = 0;
	__declspec(property(get = GetGameObject)) GameObject* _gameObject;
};