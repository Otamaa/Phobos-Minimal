#include "ComponentFactory.h"
#include <Utilities/SavegameDef.h>

#include "Component.h"

int ComponentFactory::Register(const std::string& name, ComponentCreator creator)
{
	_creatorMap.insert(make_pair(name, creator));
	Debug::Log("Registration Component \"%s\".\n", name.c_str());
	return 0;
}

Component* ComponentFactory::Create(const std::string& name)
{
	auto it = _creatorMap.find(name);
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
		Debug::Log("Component List: \n");
		for (auto it : _creatorMap)
		{
			std::string scriptName = it.first;
			Debug::Log(" -- %s\n", scriptName.c_str());
		}
	}
}