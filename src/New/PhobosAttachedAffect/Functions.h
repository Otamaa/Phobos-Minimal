#pragma once

#include <vector>
#include <set>

class PhobosAttachEffectTypeClass;
class AbstractClass;
class TechnoClass;
class TechnoTypeClass;
class WarheadTypeClass;
class WeaponTypeClass;
class HouseClass;
class PhobosAttachEffectClass;
struct PhobosAEFunctions
{
	static int GetAttachedEffectCumulativeCount(TechnoClass* pTechno, PhobosAttachEffectTypeClass* pAttachEffectType, bool ignoreSameSource = false, TechnoClass* pInvoker = nullptr, AbstractClass* pSource = nullptr, bool requireAnims = false);
	static void UpdateCumulativeAttachEffects(TechnoClass* pTarget, PhobosAttachEffectTypeClass* pAttachEffectType, bool createAnim);

	static void UpdateAttachEffects(TechnoClass* pTechno);
	static void UpdateAEAnimDrawingLogic(TechnoClass* pTechno);

	static bool HasAttachedEffects(TechnoClass* pTechno, std::vector<PhobosAttachEffectTypeClass*>& attachEffectTypes, bool requireAll, bool ignoreSameSource,
		TechnoClass* pInvoker, AbstractClass* pSource, std::vector<int> const* minCounts, std::vector<int> const* maxCounts, bool requireAnims = false);

	static void UpdateSelfOwnedAttachEffects(TechnoClass* pTechno , TechnoTypeClass* pNewType);

public:

	static void ApplyRevengeWeapon(TechnoClass* pThis , TechnoClass* pSource , WarheadTypeClass* pWH);
	static void ApplyReflectDamage(TechnoClass* pThis, int* pDamage, TechnoClass* pAttacker, HouseClass* pAttacker_House, WarheadTypeClass* pWH);
};