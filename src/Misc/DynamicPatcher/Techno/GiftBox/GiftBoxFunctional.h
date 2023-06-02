#pragma once
#ifdef COMPILE_PORTED_DP_FEATURES
#include <GeneralDefinitions.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>

class GiftBoxData;
class WarheadTypeClass;
struct GiftBoxFunctional
{
	static void Init(TechnoExt::ExtData* pExt  , TechnoTypeExt::ExtData* pTypeExt);
	static void AI(TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt);
	static void Destroy(TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt);
	static void TakeDamage(TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt,WarheadTypeClass* pWH ,DamageState nState);
};
#endif