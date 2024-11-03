#include "ComponentFactory.h"
#include "Component.h"

#include <Utilities/Debug.h>

ComponentFactory ComponentFactory::Instance;

Component* ComponentFactory::Create(const std::string& name)
{
	auto it = _creatorMap.get_key_iterator(name);

	if (it != _creatorMap.end())
	{
		Component* c = it->second();
		c->Name = name;
		return c;
	}

	return nullptr;
}

void ComponentFactory::PrintCreaterInfo()
{
	if (!_creatorMap.empty())
	{
		Debug::Log("Component List:\n");
		for (auto& it : _creatorMap) {
			Debug::Log(" -- %s\n", it.first.c_str());
		}
	}
}