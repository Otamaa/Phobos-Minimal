#include <exception>
#include <Windows.h>

#include <AnimTypeClass.h>
#include <GeneralDefinitions.h>
#include <SpecificStructures.h>

#include <Misc/Kratos/Kratos.h>
#include <Utilities/Macro.h>

#include <Misc/Kratos/Extension/AnimExt.h>
#include <Misc/Kratos/Extension/TechnoExt.h>

#include <Misc/Kratos/Ext/Helper/Scripts.h>

#include <Misc/Kratos/Ext/AnimType/ExpireAnimData.h>
#include <Misc/Kratos/Ext/AnimType/AnimStatus.h>
#include <Misc/Kratos/Ext/Common/CommonStatus.h>
#include <Misc/Kratos/Ext/TechnoType/TechnoStatus.h>

// ----------------
// Extension
// ----------------

// ----------------
// Component
// ----------------


// ----------------
// Feature
// ----------------
#ifdef _ENABLE_HOOKS

//later this need alot of adjusment
#pragma region AnimType Damage

// Takes over all damage from animations, including Ares
ASMJIT_PATCH(0x424513, AnimClass_Update_Explosion, 0x6)
{
	GET(AnimClass*, pThis, ESI);
	AnimStatus* status = nullptr;
	if (CombatDamage::Data()->AllowAnimDamageTakeOverByKratos && TryGetStatus<AnimExt>(pThis, status))
	{
		status->Explosion_Damage();
		return 0x42464C;
	}

	return 0;
}

// 碎片、流星敲地板触发，砸水中不触发
ASMJIT_PATCH(0x423E7B, AnimClass_Extras_Explosion, 0xA)
{
	GET(AnimClass*, pThis, ESI);

	AnimStatus* status = nullptr;
	if (CombatDamage::Data()->AllowAnimDamageTakeOverByKratos && TryGetStatus<AnimExt>(pThis, status))
	{
		status->Explosion_Damage(true, true);
		return 0x423EFD;
	}

	return 0;
}

// Take over to create Extras Anim when Meteor/Debris hit the water
// Phobos hook on 0x423CC7 and skip all game code, so it won't work with Phobos
ASMJIT_PATCH(0x423CD5, AnimClass_Extras_HitWater, 0x6)
{
	GET(AnimClass*, pThis, ESI);

	AnimStatus* status = nullptr;
	if (TryGetStatus<AnimExt>(pThis, status))
	{
		if (CombatDamage::Data()->AllowAnimDamageTakeOverByKratos && CombatDamage::Data()->AllowDamageIfDebrisHitWater)
		{
			status->Explosion_Damage(true);
		}
	}
	// 接管砸在水中的动画
	ExpireAnimData* data = INI::GetConfig<ExpireAnimData>(INI::Art, pThis->Type->ID)->Data;
	CoordStruct location = pThis->GetCoords();
	// 涟漪
	AnimTypeClass* pWake = RulesClass::Instance->Wake;
	if (IsNotNone(data->WakeAnimOnWater))
	{
		pWake = AnimTypeClass::Find(data->WakeAnimOnWater.c_str());
		if (!pWake)
		{
			Debug::Log("Warning: Anim %s try to create a unknow wake anim %s.\n", pThis->Type->ID, data->WakeAnimOnWater.c_str());
		}
	}
	if (pWake)
	{
		AnimClass* pNewAnim = GameCreate<AnimClass>(pWake, location);
		pNewAnim->Owner = pThis->Owner;
	}
	// 水花
	AnimTypeClass* pSplash = nullptr;
	if (IsNotNone(data->ExpireAnimOnWater))
	{
		pSplash = AnimTypeClass::Find(data->ExpireAnimOnWater.c_str());
		if (!pSplash)
		{
			Debug::Log("Warning: Anim %s try to create a unknow splash anim %s.\n", pThis->Type->ID, data->ExpireAnimOnWater.c_str());
		}
	}
	else
	{
		// 流星是大水花，碎片是小水花
		pSplash = pThis->Type->IsMeteor ? RulesClass::Instance->SplashList.back() : RulesClass::Instance->SplashList.front();
	}
	if (pSplash)
	{
		location.Z += 3;
		AnimClass* pNewAnim = GameCreate<AnimClass>(pSplash, location);
		pNewAnim->Owner = pThis->Owner;
	}
	return 0x423EFD;
}

#pragma endregion

#endif