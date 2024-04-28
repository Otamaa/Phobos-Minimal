#include "Body.h"

#include <Ext/TechnoType/Body.h>
#include <Ext/House/Body.h>
#include <Ext/WarheadType/Body.h>

AnimTypeClass* CaptureExt::GetMindcontrollAnimType(TechnoClass* pController, TechnoClass* pTarget, AnimTypeClass* pFallback)
{
	if (!pFallback)
		pFallback = RulesClass::Instance->ControlledAnimationType;

	const int wpidx = pController->SelectWeapon(pTarget);
	if (wpidx >= 0)
	{
		if (const auto pWH = pController->GetWeapon(wpidx)->WeaponType->Warhead)
			return WarheadTypeExtContainer::Instance.Find(pWH)->MindControl_Anim.Get(pFallback);
	}

	return pFallback;
}

bool CaptureExt::AllowDrawLink(TechnoTypeClass* pType)
{
	if (const auto pExt = TechnoTypeExtContainer::Instance.Find(pType))
		return pExt->Draw_MindControlLink.Get();

	return true;
}

bool CaptureExt::FreeUnit(CaptureManagerClass* pManager, TechnoClass* pTarget, bool bSilent)
{
	if (pTarget)
	{
		for (int i = pManager->ControlNodes.Count - 1; i >= 0; --i)
		{
			const auto pNode = pManager->ControlNodes[i];

			if (!pNode)
				continue;

			if (pTarget == pNode->Unit)
			{
				if (pTarget->MindControlRingAnim) {
					GameDelete<true,false>(pTarget->MindControlRingAnim);
					//pTarget->MindControlRingAnim->TimeToDie = true;
					//pTarget->MindControlRingAnim->UnInit();
					pTarget->MindControlRingAnim = nullptr;
				}

				if (!bSilent)
				{
					int nSound = pTarget->GetTechnoType()->MindClearedSound;

					if (nSound == -1)
						nSound = RulesClass::Instance->MindClearedSound;

					VocClass::PlayIndexAtPos(nSound, pTarget->GetCoords());
				}

				// Fix : Player defeated should not get this unit.
				const auto pOriginOwner = !pNode->OriginalOwner ||
					pNode->OriginalOwner->Defeated ?
					HouseExtData::FindNeutral() : pNode->OriginalOwner;

				pTarget->SetOwningHouse(pOriginOwner, !bSilent);
				pManager->DecideUnitFate(pTarget);
				pTarget->MindControlledBy = nullptr;

				if (pManager->ControlNodes.RemoveAt(i))
				{
					GameDelete<false, false>(pNode);
				}

				return true;
			}
		}
	}

	return false;
}
#include <Ext/Building/Body.h>

// new CaptureUnit function that inclued new features
bool CaptureExt::CaptureUnit(CaptureManagerClass* pManager, TechnoClass* pTarget,
	bool bRemoveFirst, bool bSilent, AnimTypeClass* pControlledAnimType)
{
	if (pManager->CanCapture(pTarget))
	{
		if (pManager->MaxControlNodes <= 0)
			return false;

		if (!pManager->InfiniteMindControl)
		{
			if (pManager->MaxControlNodes == 1 && pManager->ControlNodes.Count == 1)
				CaptureExt::FreeUnit(pManager, pManager->ControlNodes[0]->Unit , bSilent);
			else if (pManager->ControlNodes.Count == pManager->MaxControlNodes && bRemoveFirst)
				CaptureExt::FreeUnit(pManager, pManager->ControlNodes[0]->Unit , bSilent);
		}

		{
			auto pControlNode = GameCreate<ControlNode>(pTarget, pTarget->Owner, RulesClass::Instance->MindControlAttackLineFrames);
			pManager->ControlNodes.AddItem(pControlNode);
			const auto pBld = specific_cast<BuildingClass*>(pTarget);

			if (pBld) {
				BuildingExtContainer::Instance.Find(pBld)->BeignMCEd = true;
			}

			if (pTarget->SetOwningHouse(pManager->Owner->Owner, !bSilent))
			{
				pTarget->MindControlledBy = pManager->Owner;

				pManager->DecideUnitFate(pTarget);

				if (pControlledAnimType)
				{
					const auto pType = pTarget->GetTechnoType();
					CoordStruct location = pTarget->GetCoords();

					if (pBld)
						location.Z += pBld->Type->Height * Unsorted::LevelHeight;
					else
						location.Z += pType->MindControlRingOffset;

					{
						auto const pAnim = GameCreate<AnimClass>(pControlledAnimType, location);
						pTarget->MindControlRingAnim = pAnim;
						pAnim->SetOwnerObject(pTarget);

						if (pBld)
							pAnim->ZAdjust = -1024;
					}
				}

				return true;
			}
			else
			{
				if (pBld) {
					BuildingExtContainer::Instance.Find(pBld)->BeignMCEd = false;
				}
			}
		}
	}

	return false;
}

// Used For Replacing Vanilla Call function !
bool CaptureExt::CaptureUnit(CaptureManagerClass* pManager, TechnoClass* pTechno, bool bSilent)
{
	if (pTechno)
	{
		const auto Controller = pManager->Owner;
		return CaptureExt::CaptureUnit(pManager, pTechno,
		TechnoTypeExtContainer::Instance.Find(Controller->GetTechnoType())->MultiMindControl_ReleaseVictim,
		bSilent, RulesClass::Instance->ControlledAnimationType);
	}

	return false;
}

void CaptureExt::DecideUnitFate(CaptureManagerClass* pManager, FootClass* pFoot , bool Captured)
{
	// to be implemented (if needed). - secsome
}

// =============================
// load / save

//template <typename T>
//void CaptureExt::ExtData::Serialize(T& Stm) {
//
//	Stm
//		.Process(this->Initialized)
//
//		;
//
//}

// =============================
// container
//CaptureExt::ExtContainer CaptureExt::ExtMap;

// =============================
// container hooks

//DEFINE_HOOK_AGAIN(0x471998, CaptureManagerClass_CTOR, 0x6) // factory
//DEFINE_HOOK(0x471887, CaptureManagerClass_CTOR, 0x6)
//{
//	GET(CaptureManagerClass* const, pItem, ESI);
//
//	CaptureExt::ExtMap.Allocate(pItem);
//
//	return 0;
//}
//
//DEFINE_HOOK_AGAIN(0x4718ED , CaptureManagerClass_DTOR , 0x6) // Factory
//DEFINE_HOOK(0x4729EF, CaptureManagerClass_DTOR, 0x7)
//{
//	GET(CaptureManagerClass* const, pItem, ESI);
//	CaptureExt::ExtMap.Remove(pItem);
//	return 0;
//}
//
//DEFINE_HOOK_AGAIN(0x4728E0, CaptureManagerClass_SaveLoad_Prefix, 0x5)
//DEFINE_HOOK(0x472720, CaptureManagerClass_SaveLoad_Prefix, 0x8)
//{
//	GET_STACK(CaptureManagerClass*, pThis, 0x4);
//	GET_STACK(IStream*, pStm, 0x8);
//	CaptureExt::ExtMap.PrepareStream(pThis, pStm);
//	return 0;
//}
//
//DEFINE_HOOK(0x4728CA, CaptureManagerClass_Load_Suffix, 0x7)
//{
//	GET(HRESULT, nRes, EAX);
//
//	if(SUCCEEDED(nRes))
//		CaptureExt::ExtMap.LoadStatic();
//
//	return 0;
//}
//
//DEFINE_HOOK(0x472958, CaptureManagerClass_Save_Suffix, 0x7)
//{
//	GET(HRESULT, nRes, EAX);
//
//	if (SUCCEEDED(nRes))
//		CaptureExt::ExtMap.SaveStatic();
//
//	return 0;
//}