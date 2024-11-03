#pragma once

#include "IComponent.h"
#include <string>
#include <vector>
#include <list>

class IExtData;
class PhobosStreamReader;
class PhobosStreamWriter;
class Component : public IComponent
{
public:
	std::string Name;
	std::string Tag;

public:

	void SetExtData(IExtData* extData);

	virtual void ExtChanged() { };
	virtual void Clean() override;
	virtual void Awake() { };
	virtual void Destroy() { };
	virtual void OnUpdate() { };
	virtual void OnUpdateEnd() { };
	virtual void OnWarpUpdate() { };
	virtual void OwnerIsRelease(void* ptr) { };
	virtual void OnForeachEnd() { };
	virtual void FreeComponent() { };

	virtual void Activate()
	{
		_active = true;
	}

	virtual void Deactivate()
	{
		_active = false;
	}

	virtual bool IsActive()
	{
		return _active;
	}

	bool AlreadyAwake()
	{
		return _awaked;
	}

	void Disable()
	{
		_disable = true;
	}

	bool IsEnable()
	{
		return !_disable;
	}

	void EnsureAwaked();
	void EnsureDestroy();

	void AddComponent(Component* component, int index = -1);

	Component* AddComponent(const std::string& name, int index = -1);
	Component* FindOrAllocate(const std::string& name);

	template <typename TScript>
	TScript* FindOrAttach() {
		return static_cast<TScript*>(FindOrAllocate(TScript::ScriptName));
	}

	void RemoveComponent(Component* component, bool disable = true);
	void ClearDisableComponent();
	void RestoreComponent();
	void AttachToComponent(Component* component);
	void DetachFromParent(bool disable = true);

#pragma region Foreach

	template<typename F>
	void Foreach(F action)
	{
		int level = 0;
		int maxLevel = -1;
		ForeachLevel(action, level, maxLevel);

		OnForeachEnd();
	}

	template<typename F>
	void ForeachLevel(F action, int& level, int& maxLevel)
	{
		if (IsEnable() && IsActive())
		{
			Component* _this = this;
			action(_this);
			int nextLevel = level + 1;
			if (maxLevel < 0 || nextLevel < maxLevel)
			{
				for (Component* c : _children)
				{
					c->ForeachLevel(action, nextLevel, maxLevel);
					if (c->IsBreak())
					{
						break;
					}
				}
			}
		}

		ClearDisableComponent();
	}

	template<typename F>
	void ForeachChild(F action, bool force = false)
	{
		for (Component* c : _children)
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
		if (_break)
		{
			_break = false;
			return true;
		}
		return _break;
	}

#pragma endregion

#pragma region GetComponent

	Component* GetComponentInParentByName(const std::string& name);

	Component* GetComponentInChildrenByName(const std::string& name);

	Component* GetComponentByName(const std::string& name)
	{
		return GetComponentInChildrenByName(name);
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
		for (Component* children : _children)
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
		return GetComponentInChildren<TComponent>();
	}

	Component* GetParent()
	{
		return _parent;
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
		stream
			.Process(this->_childrenNames)
			.Process(this->Name)
			.Process(this->Tag)
			.Process(this->_awaked)
			.Process(this->_disable)
			.Process(this->_active)
			.Process(this->_break);
		return stream.Success();
	}

	virtual bool Load(PhobosStreamReader& stream, bool registerForChange) override;
	virtual bool Save(PhobosStreamWriter& stream) const override;
#pragma endregion

protected:

	IExtData* _extData { nullptr };
	Component* _parent { nullptr };

	bool _awaked { false };
	bool _disable { false };
	bool _active { true };
	bool _break { false };

	std::vector<std::string> _childrenNames {};
	std::list<Component*> _children {};
};
