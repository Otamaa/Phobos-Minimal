#pragma once

#include <BasicStructures.h>
#include <CoordStruct.h>

class BulletClass;
class WeaponTypeClass;
struct FireAtArmContainer
{
	BulletClass* Bullet;
	WeaponTypeClass* Weapon;
};

struct FireAtContainer_1
{
	DirStruct Dir_1;
	CoordStruct* CoordPointer;
	DirStruct Dir_2;
};