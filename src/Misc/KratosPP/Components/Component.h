#pragma once

#include <Misc/KratosPP/Interfaces/IComponent.h>
#include <Misc/KratosPP/Interfaces/IExtData.h>

#define ComponentName(CLASS_NAME) #CLASS_NAME

class Component : public IComponent
{
public:

	void SetExtData(IExtData* extData)
	{
		this->_extData = extData;
		for (Component* c : this->_children)
		{
			c->SetExtData(extData);
		}
	}

	void EnsureAwaked()
	{
		if (!this->_awaked)
		{
			this->Awake();
			_awaked = true;
			this->ForeachChild([](Component* c)
 {
	 c->EnsureAwaked();
			});

			this->ClearDisableComponent();
		}
	}

	void EnsureDestroy()
	{
		this->_disable = true;
		this->Destroy();
		for (auto it = this->_children.begin(); it != this->_children.end();)
		{
			(*it)->EnsureDestroy();
			it = this->_children.erase(it);
		}

		this->_parent = nullptr;

		delete this;
	}


	bool AlreadyAwake() const
	{
		return this->_awaked;
	}

	void Disable()
	{
		_disable = true;
	}

	bool IsEnable() const
	{
		return !_disable;
	}
	void AddComponent(Component* component, int index = -1)
	{
		component->SetExtData(_extData);
		component->_parent = this;
		if (index < 0 || index >= (int)this->_children.size())
		{
			// vector::push_back 和 vector::emplace_back 会调用析构
			// list::emplace_back 不会
			this->_children.emplace_back(component);
		}
		else
		{
			auto it = this->_children.begin();
			if (index > 0)
			{
				std::advance(it, index);
			}

			this->_children.insert(it, component);
		}
	}

	void RemoveComponent(Component* component, bool disable = true)
	{
		auto it = std::remove_if(this->_children.begin(), this->_children.end(), [component, disable](Component* c)
		{
			if (component == c)
			{
				c->_parent = nullptr;
				if (disable)
				{
					c->Disable();
				}

				return true;
			}
			return false;
		});

		_children.erase(it);
	}

	void ClearDisableComponent()
	{
		auto it = std::remove_if(this->_children.begin(), this->_children.end(), [](Component* c)
		{
			if (c->_disable)
			{
				c->_parent = nullptr;
				c->EnsureDestroy();
				return true;
			}

			return false;
		});

		_children.erase(it);
	}

	void RestoreComponent();

	void AttachToComponent(Component* component)
	{
		if (this->_parent == component)
		{
			return;
		}

		this->DetachFromParent();
		component->AddComponent(this);
	}

	void DetachFromParent(bool disable = true)
	{
		if (this->_parent)
		{
			this->_parent->RemoveComponent(this, disable);
			this->_parent = nullptr;
		}
	}

	virtual void OnUpdate() override
	{
		IComponent::OnUpdate();
	}

	virtual void Activate()
	{
		this->_active = true;
	}

	virtual void Deactivate()
	{
		this->_active = false;
	}

	virtual bool IsActive()
	{
		return _active;
	}


	Component* AddComponent(const std::string& name, int index = -1);

	Component* FindOrAllocate(const std::string& name)
	{
		Component* c = this->GetComponentByName(name);

		if (!c)
		{
			c = AddComponent(name);
			c->EnsureAwaked();
		}

		return c;
	}

	template <typename TScript>
	TScript* FindOrAttach()
	{
		return static_cast<TScript*>(FindOrAllocate(TScript::ScriptName));
	}

#pragma region Foreach

	void Foreach(std::function<void(Component*)> action)
	{
		// 执行全部
		int level = 0;
		int maxLevel = -1;
		this->ForeachLevel(action, level, maxLevel);

		this->OnForeachEnd();
	}

	void ForeachLevel(std::function<void(Component*)> action, int& level, int& maxLevel)
	{
		// 执行自身
		if (this->IsEnable() && this->IsActive())
		{
			Component* _this = this;
			action(_this);
			int nextLevel = level + 1;
			if (maxLevel < 0 || nextLevel < maxLevel)
			{
				// 执行子模块
				for (Component* c : this->_children)
				{
					c->ForeachLevel(action, nextLevel, maxLevel);
					if (c->IsBreak())
					{
						break;
					}
				}
			}
		}
		// 清理失效的子模块
		this->ClearDisableComponent();
	}

	void ForeachChild(std::function<void(Component*)> action, bool force = false)
	{
		for (Component* c : this->_children)
		{
			action(c);
			if (!force)
			{
				if (c->IsBreak())
				{
					break;
				}
			}
		}
	}

	void Break()
	{
		_break = true;
	}

	bool IsBreak()
	{
		if (this->_break)
		{
			this->_break = false;
			return true;
		}
		return this->_break;
	}

#pragma endregion

#pragma region GetComponent

	Component* GetComponentInParentByName(const std::string& name)
	{
		Component* c = nullptr;
		// find first level
		for (Component* children : this->_children) {
			if (children->Name == name) {
				c = children;
				break;
			}
		}

		if (!c && this->_parent) {
			c = this->_parent->GetComponentInParentByName(name);
		}

		return c;
	}

	Component* GetComponentInChildrenByName(const std::string& name)
	{
		Component* c = nullptr;
		// find first level
		for (Component* children : this->_children) {
			if (children->Name == name) {
				c = children;
				break;
			}
		}

		if (!c)
		{
			for (Component* children : this->_children)
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

	Component* GetComponentByName(const std::string& name)
	{
		return this->GetComponentInChildrenByName(name);
	}

	template <typename TComponent>
	TComponent* GetComponentInParent()
	{
		TComponent* c = nullptr;
		// find first level
		for (Component* children : _children)
		{
			if (typeid(*children) == typeid(TComponent))
			{
				c = (TComponent*)children;
				break;
			}
		}
		if (!c && _parent)
		{
			c = _parent->GetComponentInParent<TComponent>();
		}
		return c;
	}

	template <typename TComponent>
	TComponent* GetComponentInChildren()
	{
		TComponent* c = nullptr;
		// find first level
		for (Component* children : this->_children)
		{
			if (typeid(*children) == typeid(TComponent))
			{
				c = (TComponent*)children;
				break;
			}
		}
		if (!c)
		{
			for (Component* children : _children)
			{
				TComponent* r = children->GetComponentInChildren<TComponent>();
				if (r)
				{
					c = r;
					break;
				}
			}
		}
		return c;
	}

	template <typename TComponent>
	TComponent* GetComponent()
	{
		return this->GetComponentInChildren<TComponent>();
	}

	Component* GetParent()
	{
		return this->_parent;
	}

#pragma endregion

	Component& operator=(const Component& other)
	{
		if (this != &other)
		{
			Name = other.Name;
			Tag = other.Tag;
			_awaked = other._awaked;
			_disable = other._disable;
			_active = other._active;
			_break = other._break;
		}
		return *this;
	}

#pragma region save/load

	template <typename T>
	bool Serialize(T& stream, bool isLoad)
	{
		// 储存Component的控制参数
		stream
			// 存取子组件清单
			.Process(this->_childrenNames)

			// 存取Component自身的属性
			.Process(this->Name)
			.Process(this->Tag)
			// 每次读档之后，所有的Component实例都是重新创建的，不从存档中读取，只获取事件控制
			.Process(this->_awaked)
			.Process(this->_disable)
			.Process(this->_active)

			.Process(this->_break);
		return stream.Success();
	}

	virtual bool Load(PhobosStreamReader& stream, bool registerForChange) override
	{
		bool loaded = this->Serialize(stream, true);
		// 根据子组件清单恢复
		RestoreComponent();
		// 读取每个子组件的内容
		this->ForeachChild([&stream, &registerForChange](Component* c) { c->Load(stream, registerForChange); });
		return loaded;
	}

	virtual bool Save(PhobosStreamWriter& stream) const override
	{
		Component* pThis = const_cast<Component*>(this);
		// 生成子组件清单
		pThis->_childrenNames.clear();
		for (Component* c : pThis->_children)
		{
			pThis->_childrenNames.push_back(c->Name);
		}

		bool saved = pThis->Serialize(stream, false);
		// 存入每个子组件的内容
		pThis->ForeachChild([&stream](Component* c)
 {
	 c->Save(stream);
			});
		return saved;
	}

#pragma endregion

public:

	std::string Name;
	std::string Tag;

protected:

	IExtData* _extData = nullptr;

	bool _awaked = false;
	bool _disable = false;
	bool _active = true;
	bool _break = false;

	std::vector<std::string> _childrenNames {};
	Component* _parent = nullptr;
	std::list<Component*> _children {};
};
