#include "Component.h"
#include "ComponentFactory.h"

void Component::RestoreComponent()
{
	auto it = std::remove_if(this->_children.begin(), this->_children.end(), [this](Component* c)
	{
	if (std::find(this->_childrenNames.begin(), this->_childrenNames.end(), c->Name) == this->_childrenNames.end())
	{
		c->_parent = nullptr;
		c->EnsureDestroy();
		return true;
	}

	return false;
	});
	this->_children.erase(it);

	// 添加列表中不存在的组件
	std::vector<std::string> currentNames(this->_children.size());
	auto begin = _children.begin();
	for (size_t i = 0; i < currentNames.size(); ++i, ++begin)
	{
		currentNames[i] = (*begin)->Name;
	}

	// 取差集
	std::vector<std::string> v(_childrenNames.size());
	std::vector<std::string>::iterator end = set_difference(_childrenNames.begin(), _childrenNames.end(), currentNames.begin(), currentNames.end(), v.begin());

	for (auto ite = v.begin(); ite != end; ite++)
	{
		this->AddComponent(CreateComponent((*ite)));
	}
}

Component* Component::AddComponent(const std::string& name, int index)
{
	Component* c = CreateComponent(name);

	if (c)
	{
		this->AddComponent(c, index);
	}

	return c;
}