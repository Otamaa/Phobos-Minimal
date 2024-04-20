#include <Ext/Building/Body.h>

#include <SpecificStructures.h>
#include <ScenarioClass.h>
#include <WarheadTypeClass.h>

#include <Ext/BuildingType/Body.h>
#include <Ext/Anim/Body.h>
#include <Utilities/Macro.h>

#ifndef ENABLE_NEWHOOKS
// Brief :
// The AnimClass* memory is owned by the vector , dont delete it
// just un-init it and replace it with nullptr is enough
namespace DamageFireAnims
{
	void FORCEINLINE HandleRemoveAsExt(BuildingExtData* pExt)
	{
		if (!pExt)
			return;

		for (auto& nFires : pExt->DamageFireAnims) {
			if (nFires && nFires->Type) {
 				//GameDelete<true,false>(nFires);
				nFires->TimeToDie = true;
				nFires->UnInit();
				nFires = nullptr;
			}
		}
	}

	void FORCEINLINE HandleRemove(BuildingClass* pThis) {
		auto pExt = BuildingExtContainer::Instance.Find(pThis);
		HandleRemoveAsExt(pExt);
	}

	void FORCEINLINE HandleInvalidPtr(BuildingClass* pThis, void* ptr) {
		auto const pExt = BuildingExtContainer::Instance.Find(pThis);
		if (!pExt)
			return;

		for (auto& nFires : pExt->DamageFireAnims) {
			if (nFires == ptr) {
				nFires = nullptr;
			}
		}
	}

	void Construct(BuildingClass* pThis)
	{
		const auto pType = pThis->Type;
		const auto pExt = BuildingExtContainer::Instance.Find(pThis);
		const auto pTypeext = pExt->Type;

		HandleRemoveAsExt(pExt);

		auto const& pFire = pTypeext->DamageFireTypes.GetElements(RulesClass::Instance->DamageFireTypes);

		if (!pFire.empty() &&
			!pTypeext->DamageFire_Offs.empty())
		{
			pExt->DamageFireAnims.resize(pTypeext->DamageFire_Offs.size());

			for (int i = 0; i < (int)pTypeext->DamageFire_Offs.size(); ++i)
			{
				const auto& nFireOffs = pTypeext->DamageFire_Offs[i];
				const auto&[nPiX ,nPiY] = TacticalClass::Instance->ApplyOffsetPixel(nFireOffs);

				CoordStruct nPixCoord { nPiX, nPiY, 0 };
				nPixCoord += pThis->GetRenderCoords();

				if (const auto pFireType = pFire[pFire.size() == 1 ?
					 0 : ScenarioClass::Instance->Random.RandomFromMax(pFire.size() - 1)])
				{
					auto pAnim = GameCreate<AnimClass>(pFireType, nPixCoord);
					const auto nBuildingHeight = pType->GetFoundationHeight(false);
					const auto nWidth = pType->GetFoundationWidth();
					const auto nAdjust = ((3 * (nFireOffs.Y - 15 * nWidth + (-15) * nBuildingHeight)) >> 1) - 10;
					pAnim->ZAdjust = nAdjust > 0 ? 0 : nAdjust; //ZAdjust always negative
					if (pAnim->Type->End > 0)
						pAnim->Animation.Value = ScenarioClass::Instance->Random.RandomFromMax(pAnim->Type->End - 1);

					pAnim->Owner = pThis->GetOwningHouse();
					pExt->DamageFireAnims[i] = std::move(pAnim);
				}
			}
		}
	}
};

DEFINE_HOOK(0x43FC90, BuildingClass_CreateDamageFireAnims, 0x7)
{
	GET(BuildingClass*, pThis, ESI);
	DamageFireAnims::Construct(pThis);
	return 0x43FC97;
}

//DEFINE_JUMP(CALL, 0x43FC92, GET_OFFSET(DamageFireAnims::Construct));
DEFINE_HOOK(0x46038A , BuildingTypeClass_ReadINI_SkipDamageFireAnims, 0x6)
{
	return 0x46048E;
}
//DEFINE_JUMP(LJMP,0x43BA72, 0x43BA7F); //remove memset for buildingFireAnims
//DEFINE_JUMP(LJMP, 0x46038A, 0x46048E);

#define HANDLEREMOVE_HOOKS(addr ,reg ,name, size ,ret) \
DEFINE_HOOK(addr , BuildingClass_##name##_DamageFireAnims , size ) { \
	GET(BuildingClass*, pThis, reg);\
	DamageFireAnims::HandleRemove(pThis);\
	return ret;\
}

HANDLEREMOVE_HOOKS(0x43BDD5,ESI, DTOR, 0x6, 0x43BDF6)
HANDLEREMOVE_HOOKS(0x44AB87,EBP, Fire1, 0x6, 0x44ABAC)
HANDLEREMOVE_HOOKS(0x440076,ESI, Fire2, 0x6, 0x44009B)
HANDLEREMOVE_HOOKS(0x43FC99,ESI, Fire3, 0x6, 0x43FCBE)
HANDLEREMOVE_HOOKS(0x4458E4,ESI, Fire4, 0x6, 0x445905)

#undef HANDLEREMOVE_HOOKS

DEFINE_HOOK(0x4415F9, BuildingClass_handleRemoveFire5, 0x5)
{
	GET(BuildingClass*, pThis, ESI);
	DamageFireAnims::HandleRemove(pThis);

	R->EBX(0);
	return 0x44161C;
}

DEFINE_HOOK(0x43C2A0, BuildingClass_RemoveFire_handle, 0x8) //was 5
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

//remove it from load
//DEFINE_JUMP(LJMP, 0x454154, 0x454170);
DEFINE_HOOK(0x454154 , BuildingClass_LoadGame_DamageFireAnims , 0x6) {
	return 0x454170;
}
#endif

DEFINE_HOOK(0x44270B, BuildingClass_ReceiveDamage_OnFire, 0x9)
{
	GET(BuildingClass* const, pThis, ESI);
	GET_STACK(CellStruct*, pFoundationArray, 0x10);
	REF_STACK(args_ReceiveDamage const, args, STACK_OFFS(0x9C, -0x4));

	if (args.WH->Sparky)
	{
		auto const pTypeExt = BuildingTypeExtContainer::Instance.Find(pThis->Type);
		auto const pBldExt = BuildingExtContainer::Instance.Find(pThis);
		const bool Onfire = pTypeExt->HealthOnfire.Get(pThis->GetHealthStatus());
		auto const pFireType = pTypeExt->OnFireTypes.GetElements(RulesClass::Instance->OnFire);

		if (Onfire && pFireType.size() >= 3) {

			if(Unsorted::CurrentFrame < pBldExt->LastFlameSpawnFrame + RulesExtData::Instance()->BuildingFlameSpawnBlockFrames)
				return 0x4428FE;

			pBldExt->LastFlameSpawnFrame = Unsorted::CurrentFrame;

			for (; (pFoundationArray->X != 0x7FFF || pFoundationArray->Y != 0x7FFF); ++pFoundationArray)
			{
				auto const&[nCellX , nCellY] = pThis->InlineMapCoords() + *pFoundationArray;
				CoordStruct nDestCoord { (nCellX * 256) + 128, (nCellY * 256) + 128, 0 };
				nDestCoord.Z = MapClass::Instance->GetCellFloorHeight(nDestCoord);

				auto PlayFireAnim = [&](int nLoop = 1, int nFireTypeAt = 2)
				{
					if (auto pAnimType = pFireType[nFireTypeAt])
					{
						nDestCoord = MapClass::GetRandomCoordsNear(nDestCoord, 96, false);
						auto const pAnim = GameCreate<AnimClass>(pAnimType, nDestCoord, 0, nLoop);
						pAnim->SetOwnerObject(pThis);
						const auto pKiller = args.Attacker;
						const auto Invoker = (pKiller) ? pKiller->Owner : args.SourceHouse;
						AnimExtData::SetAnimOwnerHouseKind(pAnim, Invoker, pThis->Owner, pKiller, false);
					}
				};

				switch (ScenarioClass::Instance->Random.RandomFromMax(pThis->Type->GetFoundationWidth() + pThis->Type->GetFoundationHeight(false) + 5))
				{
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
					PlayFireAnim(ScenarioClass::Instance->Random.RandomFromMax(pFireType.size()), 0);
					break;
				case 6:
				case 7:
				case 8:
					PlayFireAnim(ScenarioClass::Instance->Random.RandomFromMax(pFireType.size()), 1);
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