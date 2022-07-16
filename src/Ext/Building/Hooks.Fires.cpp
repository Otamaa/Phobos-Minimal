#include <Ext/Building/Body.h>

#include <SpecificStructures.h>
#include <ScenarioClass.h>
#include <WarheadTypeClass.h>

#include <Ext/BuildingType/Body.h>
#include <Ext/Anim/Body.h>
#include <Utilities/Macro.h>

#ifdef REPLACE_BUILDING_ONFIRE
namespace DamageFireAnims
{
	void HandleRemove(BuildingClass* pThis)
	{
		if (auto const pExt = BuildingExt::ExtMap.Find(pThis))
		{
			for (auto& pItem : pExt->DamageFireAnims)
			{
				if (pItem != nullptr)
				{
					pItem->RemainingIterations = 0;
					pItem->DetachFromObject(pItem->OwnerObject, true);
					pItem->UnInit();
					//GameDelete(pItem);
					pItem = nullptr;
				}
			}
		}
	}

	void HandleInvalidPtr(BuildingClass* pThis, void* ptr)
	{
		if (auto const pExt = BuildingExt::ExtMap.Find(pThis))
		{
			for (auto& pItem : pExt->DamageFireAnims) {
				if ((void*)pItem == ptr) {
					pItem = nullptr;
				}
			}
		}
	}

	void __fastcall Construct(BuildingClass* pThis, void* _)
	{
		auto pType = pThis->Type;
		auto pExt = BuildingExt::ExtMap.Find(pThis);
		auto pTypeext = BuildingTypeExt::ExtMap.Find(pType);

		if (!pExt || !pTypeext)
			return;

		auto const pFire = pTypeext->DamageFireTypes.GetElements(RulesGlobal->DamageFireTypes);

		if (!pFire.empty() &&
			!(pTypeext->DamageFire_Offs.Count == 0)
			)
		{

			for (int i = 0; i < pTypeext->DamageFire_Offs.Count; ++i)
			{
				auto nFireOffs = pTypeext->DamageFire_Offs[i];
				auto nPixel = TacticalGlobal->ApplyOffsetPixel(nFireOffs);
				CoordStruct nPixCoord { nPixel.X, nPixel.Y, 0 };
				nPixCoord += pThis->GetCenterCoord();

				if (auto const pFireType = pFire[ScenarioGlobal->Random(0, pFire.size() - 1)])
				{
					if (auto pAnim = GameCreate<AnimClass>(pFireType, nPixCoord))
					{
						auto nBuildingHeight = pType->GetFoundationHeight(false);
						auto nWidth = pType->GetFoundationWidth();
						auto nAdjust = ((3 * (nFireOffs.Y - 15 * nWidth + (-15) * nBuildingHeight)) >> 1) - 10;
						pAnim->ZAdjust = nAdjust > 0 ? 0 : nAdjust; //ZAdjust always negative
						if (pAnim->Type->End > 0)
							pAnim->Animation.Value = ScenarioGlobal->Random(0, pAnim->Type->End - 1);

						pAnim->Owner = pThis->GetOwningHouse();
						pExt->DamageFireAnims.push_back(pAnim);
					}
				}
			}
		}
	}
};

DEFINE_JUMP(CALL,0x43FC92,GET_OFFSET(DamageFireAnims::Construct));
DEFINE_JUMP(LJMP,0x460388, 0x46048E); // no thankyou , we handle it ourself !
DEFINE_JUMP(LJMP,0x43BA72, 0x43BA7F); //remove memset for buildingFireAnims

#define HANDLEREMOVE_HOOKS(addr ,name, size ,ret) \
DEFINE_HOOK(addr , BuildingClass_##name##_DamageFireAnims , size ) { \
	GET(BuildingClass*, pThis, ESI);\
	DamageFireAnims::HandleRemove(pThis);\
	return ret;\
}

HANDLEREMOVE_HOOKS(0x43BDD5, DTOR, 0x6, 0x43BDF6)
HANDLEREMOVE_HOOKS(0x44AB87, Fire1, 0x6, 0x44ABAC)
HANDLEREMOVE_HOOKS(0x440076, Fire2, 0x6, 0x44009B)
HANDLEREMOVE_HOOKS(0x43FC99, Fire3, 0x6, 0x43FCBE)
HANDLEREMOVE_HOOKS(0x4458E4, Fire4, 0x6, 0x445905)

#undef HANDLEREMOVE_HOOKS

DEFINE_HOOK(0x4415F9, BuildingClass_handleRemoveFire5, 0x5)
{
	GET(BuildingClass*, pThis, ESI);
	DamageFireAnims::HandleRemove(pThis);

	R->EBX(0);
	return 0x44161C;
}

DEFINE_HOOK(0x43C2A0, BuildingClass_RemoveFire_handle, 0x5)
{
	GET(BuildingClass*, pThis, ECX);
	DamageFireAnims::HandleRemove(pThis);
	return 0x43C2C9;
}

DEFINE_HOOK(0x44EA1C, BuildingClass_DetachOrInvalidPtr_handle, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	GET(void*, ptr, EBP);
	DamageFireAnims::HandleInvalidPtr(pThis, ptr);
	return 0x44EA2F;
}
#endif

DEFINE_HOOK(0x44270B, BuildingClass_ReceiveDamge_OnFire, 0x9)
{
	GET(BuildingClass* const, pThis, ESI);
	GET_STACK(CellStruct*, pCell, 0x10);
	REF_STACK(args_ReceiveDamage const, ReceiveDamageArgs, STACK_OFFS(0x9C, -0x4));

	if (ReceiveDamageArgs.WH->Sparky)
	{
		auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pThis->Type);
		const bool Onfire = pTypeExt->HealthOnfire.Get(pThis->GetHealthStatus());
		auto const pFireType = pTypeExt->OnFireTypes.GetElements(RulesClass::Instance->OnFire);

		if (Onfire && pFireType.size() >= 3)
		{
			for (; (pCell->X != 0x7FFF || pCell->Y != 0x7FFF); ++pCell)
			{
				auto const&[nCellX , nCellY] = pThis->GetMapCoords() + *pCell;
				auto nDestCoord = CoordStruct { (nCellX * 256) + 128,(nCellY * 256) + 128,0 };
				nDestCoord.Z = MapClass::Instance->GetCellFloorHeight(nDestCoord);

				auto PlayFireAnim = [&](int nLoop = 1, int nFireTypeAt = 2)
				{
					if (auto pAnimType = pFireType.at(nFireTypeAt))
					{
						nDestCoord = MapClass::GetRandomCoordsNear(nDestCoord, 96, false);
						if (auto const pAnim = GameCreate<AnimClass>(pAnimType, nDestCoord, 0, nLoop))
						{
							pAnim->SetOwnerObject(pThis);
							const auto pKiller = ReceiveDamageArgs.Attacker;
							const auto Invoker = (pKiller) ? pKiller->Owner : ReceiveDamageArgs.SourceHouse;

							AnimExt::SetAnimOwnerHouseKind(pAnim, Invoker, pThis->Owner, pKiller, false);
						}
					}
				};

				switch (ScenarioClass::Instance->Random.RandomRanged(0, pThis->Type->GetFoundationWidth() + pThis->Type->GetFoundationHeight(false) + 5))
				{
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
					PlayFireAnim(ScenarioClass::Instance->Random(0, pFireType.size() - 1), 0);
					break;
				case 6:
				case 7:
				case 8:
					PlayFireAnim(ScenarioClass::Instance->Random(0, pFireType.size() - 1), 1);
					break;
				case 9:
					PlayFireAnim();
					break;
				default:
					break;
				}
			}
		}
	}

	return 0x4428FE;
}