#pragma once
#include <GeneralDefinitions.h>

class TechnoExtData;
class TechnoTypeExtData;
class GiftBoxData;
class WarheadTypeClass;
struct GiftBoxFunctional
{
	static void Init(TechnoExtData* pExt  , TechnoTypeExtData* pTypeExt);
	static void AI(TechnoExtData* pExt, TechnoTypeExtData* pTypeExt);
	static void Destroy(TechnoExtData* pExt, TechnoTypeExtData* pTypeExt);
	static void TakeDamage(TechnoExtData* pExt, TechnoTypeExtData* pTypeExt,WarheadTypeClass* pWH ,DamageState nState);
};
