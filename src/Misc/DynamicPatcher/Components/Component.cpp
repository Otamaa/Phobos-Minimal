#include "Component.h"
#include "ComponentFactory.h"

#include <algorithm>
#include <Utilities/SavegameDef.h>

void Component::SetExtData(IExtData* extData) {
	_extData = extData;
	for (Component* c : _children) {
		c->SetExtData(extData);
	}
}

void Component::Clean() {
	_extData = nullptr;
	_awaked = false;
	_disable = false;
	_active = true;
	_break = false;
	_childrenNames.clear();
	_parent = nullptr;
	_children.clear();
}

void Component::EnsureAwaked()
{
	if (!_awaked) {
		Awake();
		_awaked = true;
		ForeachChild([](Component* c) {
				c->EnsureAwaked();
		});
		ClearDisableComponent();
	}
}

void Component::EnsureDestroy()
{
	_disable = true;
	Destroy();
	for (auto it = _children.begin(); it != _children.end();) {
		(*it)->EnsureDestroy();
		it = _children.erase(it);
	}
	_parent = nullptr;
	FreeComponent();
}

void Component::AddComponent(Component* component, int index)
{
	component->SetExtData(_extData);
	component->_parent = this;

	if (index < 0 || index >= (int)_children.size()) {
		_children.emplace_back(component);
	}
	else
	{
		auto it = _children.begin();

		if (index > 0) {
			std::advance(it, index);
		}

		_children.insert(it, component);
	}

}

Component* Component::AddComponent(const std::string& name, int index)
{
	Component* c = CreateComponent(name);
	if (c) {
		AddComponent(c, index);
	}

	return c;
}

Component* Component::FindOrAllocate(const std::string& name)
{
	Component* c = GetComponentByName(name);
	if (!c) {
		c = AddComponent(name);
		c->EnsureAwaked();
	}
	return c;
}

void Component::RemoveComponent(Component* component, bool disable)
{
	auto it = std::remove_if(_children.begin(), _children.end(), [component , disable](Component* pItem){
		if (pItem == component) {
			pItem->_parent = nullptr;
			if (disable) {
				pItem->Disable();
			}

			return true;
		}

		return false;
	});


	_children.erase(it, _children.end());
}

void Component::ClearDisableComponent()
{
	auto it = std::remove_if(_children.begin(), _children.end(), [](Component* pItem)
	{
		if (pItem->_disable) {
			pItem->_parent = nullptr;
			pItem->EnsureDestroy();
			return true;
		}

		return false;
	});

	_children.erase(it, _children.end());
}

void Component::RestoreComponent()
{
	auto it = std::remove_if(_children.begin(), _children.end(), [this](Component* pItem) {
		if (std::find(this->_childrenNames.begin(), this->_childrenNames.end(), pItem->Name) == _childrenNames.end()) {
			pItem->_parent = nullptr;
			pItem->EnsureDestroy();
			return true;
		}

		return false;
	});

	_children.erase(it, _children.end());

	std::vector<std::string> currentNames {};
	for (Component* c : _children) {
		currentNames.push_back(c->Name);
	}

	std::vector<std::string> v(_childrenNames.size());
	std::vector<std::string>::iterator end =
	std::set_difference(
			_childrenNames.begin(),
			_childrenNames.end(),
			currentNames.begin(),
			currentNames.end(),
		v.begin()
	);


	for (auto ite = v.begin(); ite != end; ite++) {
		AddComponent(CreateComponent((*ite)));
	}
}

Component* Component::GetComponentInParentByName(const std::string& name)
{
	Component* c = nullptr;

	for (Component* children : _children) {
		if (children->Name == name) {
			c = children;
			break;
		}
	}

	if (!c && _parent) {
		c = _parent->GetComponentInParentByName(name);
	}

	return c;
}

Component* Component::GetComponentInChildrenByName(const std::string& name)
{
	Component* c = nullptr;

	for (Component* children : _children) {
		if (children->Name == name) {
			c = children;
			break;
		}
	}

	if (!c) {
		for (Component* children : _children) {
			if (Component* r = children->GetComponentInChildrenByName(name)) {
				c = r;
				break;
			}
		}
	}

	return c;
}

void Component::AttachToComponent(Component* component)
{
	if (_parent == component) {
		return;
	}

	DetachFromParent();
	component->AddComponent(this);
}

void Component::DetachFromParent(bool disable)
{
	if (_parent) {
		_parent->RemoveComponent(this, disable);
		this->_parent = nullptr;
	}
}

bool Component::Load(PhobosStreamReader& stream, bool registerForChange)
{
	bool loaded = this->Serialize(stream, true);
	RestoreComponent();
	this->ForeachChild([&stream, &registerForChange](Component* c) { c->Load(stream, registerForChange); });
	return loaded;
}

bool Component::Save(PhobosStreamWriter& stream) const
{
	Component* pThis = const_cast<Component*>(this);

	pThis->_childrenNames.clear();
	for (Component* c : pThis->_children)
	{
		pThis->_childrenNames.push_back(c->Name);
	}
	bool saved = pThis->Serialize(stream, false);

	pThis->ForeachChild([&stream](Component* c)
{
	c->Save(stream);
	});
	return saved;
}