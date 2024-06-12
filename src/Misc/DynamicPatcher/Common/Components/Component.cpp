#include "Component.h"

void Component::SetExtData(IExtData* extData)
{
	_extData = extData;
	for (Component* c : _children)
	{
		c->SetExtData(extData);
	}
}

void Component::OnUpdate()
{
	IComponent::OnUpdate();
}

void Component::EnsureAwaked()
{
	if (!_awaked)
	{
		Awake();
		// 在Awake中可能出现移除自身的操作，_awaked标记也用于控制Remove时是否延迟删除
		_awaked = true;
		ForeachChild([](Component* c) {
			c->EnsureAwaked();
		});
		// 销毁失效的子模块
		ClearDisableComponent();
	}
}

void Component::EnsureDestroy()
{
	_disable = true;
	Destroy();
	for (auto it = _children.begin(); it != _children.end();)
	{
		(*it)->EnsureDestroy();
		it = _children.erase(it);
	}
	_parent = nullptr;

	// 释放资源
	// GameDelete(this);
	// delete this;
	FreeComponent(); // 返回缓冲池
}

void Component::AddComponent(Component* component, int index)
{
	// 将要加入的组件的子组件的extData全部更换
	component->SetExtData(_extData);
	component->_parent = this;
	if (index < 0 || index >= (int)_children.size())
	{
		// vector::push_back 和 vector::emplace_back 会调用析构
		// list::emplace_back 不会
		_children.emplace_back(component);
	}
	else
	{
		// 插入指定位置
		auto it = _children.begin();
		if (index > 0)
		{
			std::advance(it, index);
		}
		_children.insert(it, component);
	}
}

Component* Component::AddComponent(const std::string& name, int index)
{
	Component* c = CreateComponent(name);
	if (c)
	{
		AddComponent(c, index);
	}
	return c;
}

Component* Component::FindOrAllocate(const std::string& name)
{
	Component* c = GetComponentByName(name);
	if (!c)
	{
		// 添加新的Component
		c = AddComponent(name);
		// 激活新的Component
		c->EnsureAwaked();
	}
	return c;
}

void Component::RemoveComponent(Component* component, bool disable)
{
	auto it = std::find(_children.begin(), _children.end(), component);

	if (it != _children.end())
	{
		Component* c = *it;
		c->_parent = nullptr;
		if (disable)
		{
			c->Disable();
		}
		// 从_children清单中删除
		_children.erase(it);
	}
}

void Component::ClearDisableComponent()
{
	for (auto it = _children.begin(); it != _children.end();)
	{
		Component* c = *it;
		if (c->_disable)
		{
			c->_parent = nullptr;
			it = _children.erase(it);
			c->EnsureDestroy();
		}
		else
		{
			it++;
		}
	}
}

void Component::RestoreComponent()
{
	// 去除名单中不存在的组件
	for (auto it = _children.begin(); it != _children.end();)
	{
		Component* c = *it;
		if (std::find(_childrenNames.begin(), _childrenNames.end(), c->Name) == _childrenNames.end())
		{
			c->_parent = nullptr;
			it = _children.erase(it);
			c->EnsureDestroy();
		}
		else
		{
			it++;
		}
	}
	// 添加列表中不存在的组件
	std::vector<std::string> currentNames{};
	for (Component* c : _children)
	{
		currentNames.push_back(c->Name);
	}

	// 取差集
	std::vector<std::string> v;
	v.resize(_childrenNames.size());
	std::vector<std::string>::iterator end = set_difference(_childrenNames.begin(), _childrenNames.end(), currentNames.begin(), currentNames.end(), v.begin());

	for (auto ite = v.begin(); ite != end; ite++)
	{
		Component* c = CreateComponent((*ite));
		AddComponent(c);
	}
}

Component* Component::GetComponentInParentByName(const std::string& name)
{
	Component* c = nullptr;
	// find first level
	for (Component* children : _children)
	{
		if (children->Name == name)
		{
			c = children;
			break;
		}
	}
	if (!c && _parent)
	{
		c = _parent->GetComponentInParentByName(name);
	}
	return c;
}

Component* Component::GetComponentInChildrenByName(const std::string& name)
{
	Component* c = nullptr;
	// find first level
	for (Component* children : _children)
	{
		if (children->Name == name)
		{
			c = children;
			break;
		}
	}
	if (!c)
	{
		for (Component* children : _children)
		{
			Component* r = children->GetComponentInChildrenByName(name);
			if (r)
			{
				c = r;
				break;
			}
		}
	}
	return c;
}

Component* Component::GetComponentByName(const std::string& name)
{
	return GetComponentInChildrenByName(name);
}

Component* Component::GetParent()
{
	return _parent;
}

void Component::AttachToComponent(Component* component)
{
	if (_parent == component)
	{
		return;
	}
	DetachFromParent();

	component->AddComponent(this);
}

void Component::DetachFromParent(bool disable)
{
	if (_parent)
	{
		_parent->RemoveComponent(this, disable);
		this->_parent = nullptr;
	}
}

