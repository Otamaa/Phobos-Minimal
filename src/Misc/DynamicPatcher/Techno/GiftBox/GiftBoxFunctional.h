#pragma once
#ifdef COMPILE_PORTED_DP_FEATURES
#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>

class GiftBoxData;
namespace GiftBoxFunctional
{
	void Init(TechnoExt::ExtData* pExt  , TechnoTypeExt::ExtData* pTypeExt);
	void AI(TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt);
	void Destroy(TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt);
	void TakeDamage(TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt,WarheadTypeClass* pWH ,DamageState nState);
};
#endif