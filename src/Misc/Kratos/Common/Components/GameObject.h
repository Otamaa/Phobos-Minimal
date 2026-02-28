#pragma once

#include <list>
#include <vector>

#include "Component.h"

#include <Misc/Kratos/Common/MyDelegate.h>

using namespace Delegate;

class GameObject : public Component
{
public:
	DECLARE_COMPONENT(GameObject, Component);

	virtual ~GameObject() override
	{
		LOG_COMPONENT("GameObject %s destructor called.\n", thisName.c_str());
		// 不自动调用EnsureDestroy，信任外部管理
	}

	// 特别处理 GameObject 的销毁
	void EnsureDestroy()
	{

		// 调用父类的EnsureDestroy
		Component::EnsureDestroy();

		LOG_COMPONENT("=== GameObject::EnsureDestroy END ===\n");
	}

	bool ExtChanged = false;

	virtual void Clean() override
	{
		Component::Clean();
		ExtChanged = false;
	};

	GameObject* GetAwaked()
	{
		// 如果已经唤醒，则直接返回this
		if (IsAwaked()) {
			return this;
		}

		// 如果没有唤醒，则调用EnsureAwaked()来尝试唤醒并返回this
		EnsureAwaked();
		return this;
	}

	virtual void OnForeachEnd() override
	{
		// 清理所有Disabling状态的组件
		ClearDisableComponent();

		// 通知所有子组件扩展数据已更改
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
