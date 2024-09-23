#pragma once

#include <Base/Always.h>
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
	/// <summary>
/// Gets how many counts of same cumulative AttachEffect type instance techno has active on it.
/// </summary>
/// <param name="pAttachEffectType">AttachEffect type.</param>
/// <param name="ignoreSameSource">Ignore AttachEffects that come from set invoker and source.</param>
/// <param name="pInvoker">Invoker Techno used for same source check.</param>
/// <param name="pSource">Source AbstractClass instance used for same source check.</param>
/// <returns>Number of active cumulative AttachEffect type instances on the techno. 0 if the AttachEffect type is not cumulative.</returns>
	static int GetAttachedEffectCumulativeCount(TechnoClass* pTechno, PhobosAttachEffectTypeClass* pAttachEffectType, bool ignoreSameSource = false, TechnoClass* pInvoker = nullptr, AbstractClass* pSource = nullptr);
	static void UpdateCumulativeAttachEffects(TechnoClass* pTechno, PhobosAttachEffectTypeClass* pAttachEffectType, PhobosAttachEffectClass* pRemoved);

	// Updates state of all AttachEffects on techno.
	static void UpdateAttachEffects(TechnoClass* pTechno);

	/// <summary>
/// Gets whether or not techno has listed AttachEffect types active on it
/// </summary>
/// <param name="attachEffectTypes">Attacheffect types.</param>
/// <param name="requireAll">Whether or not to require all listed types to be present or if only one will satisfy the check.</param>
/// <param name="ignoreSameSource">Ignore AttachEffects that come from set invoker and source.</param>
/// <param name="pInvoker">Invoker Techno used for same source check.</param>
/// <param name="pSource">Source AbstractClass instance used for same source check.</param>
/// <returns>True if techno has active AttachEffects that satisfy the source, false if not.</returns>
	static bool HasAttachedEffects(TechnoClass* pTechno, std::vector<PhobosAttachEffectTypeClass*>& attachEffectTypes, bool requireAll, bool ignoreSameSource,
		TechnoClass* pInvoker, AbstractClass* pSource, std::vector<int> const* minCounts, std::vector<int> const* maxCounts);

	static void UpdateSelfOwnedAttachEffects(TechnoClass* pTechno , TechnoTypeClass* pNewType);

	static void ApplyRevengeWeapon(TechnoClass* pThis , TechnoClass* pSource , WarheadTypeClass* pWH);
	static void ApplyExpireWeapon(std::vector<WeaponTypeClass*>& expireWeapons, std::set<PhobosAttachEffectTypeClass*>& cumulativeTypes, TechnoClass* pThis);
	static void ApplyReflectDamage(TechnoClass* pThis, int* pDamage, TechnoClass* pAttacker, HouseClass* pAttacker_House, WarheadTypeClass* pWH);
};