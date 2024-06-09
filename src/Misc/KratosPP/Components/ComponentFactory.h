#pragma once

#include <map>
#include <functional>
#include <string>

class Component;
class ComponentFactory
{
public:

	static ComponentFactory& GetInstance()
	{
		static ComponentFactory instance;
		return instance;
	}

	using ComponentCreator = std::function<Component* (void)>;

	int Register(const std::string& name, ComponentCreator creator);
	Component* Create(const std::string& name);
	void PrintCreaterInfo();


private:
	ComponentFactory() = default;
	~ComponentFactory() = default;

	ComponentFactory(const ComponentFactory&) = delete;

	std::map<std::string, ComponentCreator> _creatorMap {};
};

static Component* CreateComponent(const std::string name)
{
	return ComponentFactory::GetInstance().Create(name);
}