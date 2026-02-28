#include <exception>
#include <Windows.h>

#include <GeneralStructures.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <InfantryClass.h>

#include <Misc/Kratos/Kratos.h>
#include <Utilities/Macro.h>

#include <Misc/Kratos/Extension/TechnoExt.h>
#include <Misc/Kratos/Extension/WarheadTypeExt.h>

#include <Misc/Kratos/Ext/TechnoType/TechnoStatus.h>

#ifdef _ENABLE_HOOKS

ASMJIT_PATCH(0x5194EF, InfantryClass_DrawIt_InAir_Shadow_Skip, 0x5)
{
	GET(InfantryClass*, pInf, EBP);
	if (pInf->Type->NoShadow || pInf->CloakState != CloakState::Uncloaked)
	{
		return 0x51958A;
	}
	return 0;
}

#pragma region Infantry death anims
ASMJIT_PATCH(0x5184F7, Infantry_ReceiveDamage_DeathAnim, 0x8)
{
	GET(InfantryClass*, pInf, ESI);
	if (TechnoStatus* status = GetStatus<TechnoExt, TechnoStatus>(pInf))
	{
		if (status->PlayDestroyAnims())
		{
			if (pInf->Type->NotHuman || pInf->IsInAir())
			{
				pInf->UnInit();
				return 0x5185E5;
			}
			return 0x5185F1;
		}
	}
	return 0;
}
#pragma endregion

// ASMJIT_PATCH(0x51F5C0, Infantry_Mission_Hunt_Deploy, 0x6)
// {
// 	GET(InfantryClass*, pInf, ESI);
// 	DoType doing = pInf->SequenceAnim;
// 	AbstractClass* pTarget = pInf->Target;
// 	if (!pInf->Owner->IsControlledByHuman()
// 		&& pTarget && pInf->IsCloseEnough(pTarget, pInf->SelectWeapon(pTarget))
// 		&& doing != DoType::Deploy && doing != DoType::Deployed && doing != DoType::DeployedFire && doing != DoType::DeployedIdle)
// 	{
// 		int result = pInf->UpdateDeplory();
// 		if (result != -1)
// 		{
// 			R->EAX(result);
// 			return 0x51F5BE;
// 		}
// 	}
// 	return 0;
// }

#endif