#include "Phobos.Entity.h"

entt::registry PhobosEntity::Registry;

void PhobosEntity::OnStartup()
{

}

void PhobosEntity::OnExit()
{
	Registry.clear();
}

void PhobosEntity::OnClear()
{
	Registry.clear();
}
