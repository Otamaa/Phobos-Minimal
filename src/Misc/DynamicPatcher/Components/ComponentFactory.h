#pragma once

#include <Utilities/PhobosMap.h>

#include <string>

class Component;
class ComponentFactory
{
public:
	using ComponentCreator = Component * (*) (void);

	bool Register(const std::string& name, ComponentCreator creator) {
		return _creatorMap.insert(name, creator);
	}

	Component* Create(const std::string& name);

	void PrintCreaterInfo();


private:
	PhobosMap<std::string, ComponentCreator> _creatorMap {};

public :
	static ComponentFactory Instance;
};

static Component* CreateComponent(const std::string name) {
	return ComponentFactory::Instance.Create(name);
}

#define DECLARE_COMPONENT(CLASS_NAME, ...) \
	CLASS_NAME() : __VA_ARGS__() \
	{ \
		this->Name = ScriptName; \
	} \
	\
	virtual void FreeComponent() override \
	{ \
		Clean(); \
		Pool.push_back(this); \
	} \
	\
	static Component* Create() \
	{ \
		Component* c = nullptr; \
		if (!Pool.empty()) \
		{ \
			auto it = Pool.begin(); \
			c = *it; \
			Pool.erase(it); \
		} \
		if (!c) \
		{ \
			c = static_cast<Component*>(new CLASS_NAME()); \
		} \
		return c; \
	} \
	\
	inline static std::string ScriptName = #CLASS_NAME; \
	\
	inline static bool g_temp_##CLASS_NAME = ComponentFactory::Instance.Register(#CLASS_NAME, CLASS_NAME::Create); \
	\
	inline static std::vector<CLASS_NAME*> Pool{}; \
