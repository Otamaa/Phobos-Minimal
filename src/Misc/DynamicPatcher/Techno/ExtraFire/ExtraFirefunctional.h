#pragma once
#include <Base/Always.h>
#include <CoordStruct.h>
#include <Utilities/Iterator.h>

class AbstractClass;
class TechnoClass;
struct ExtraFireData;
class WeaponTypeClass;
struct ExtraFirefunctional
{
	static void GetWeapon(TechnoClass* pThis, AbstractClass* pTarget , int nWeaponIdx);
	static Iterator<WeaponTypeClass*> HasAnyExtraFireWeapon(TechnoClass* pThis, const ExtraFireData& nExtraFireData, int nWeaponIdx , CoordStruct& selectedFLh);
};
