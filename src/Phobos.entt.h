#pragma once
#include <Lib/entt/src/entt/entt.hpp>

class TechnoClass;
class YRentt
{
public:

	static inline std::unique_ptr<entt::registry> gEntt;

	static void BindToentt(TechnoClass* pTechno);
	static void UnbindFromentt(TechnoClass* pTechno);

	static void Destroy() { gEntt = nullptr; }
	static void Create() { gEntt = std::make_unique<entt::registry>(); }
	static void Clear() {
		gEntt->clear(); // TODO : clear only specific events
	}

};