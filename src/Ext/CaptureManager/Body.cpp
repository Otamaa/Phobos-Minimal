#include "Body.h"

#include <Ext/TechnoType/Body.h>
#include <Ext/House/Body.h>

CaptureExt::ExtContainer CaptureExt::ExtMap;

void CaptureExt::ExtData::InitializeConstants() { }

bool CaptureExt::AllowDrawLink(TechnoTypeClass* pType)
{
	if (const auto pExt = TechnoTypeExt::ExtMap.Find(pType))
		return pExt->Draw_MindControlLink.Get();

	return true;
}

bool CaptureExt::CanCapture(CaptureManagerClass* pManager, TechnoClass* pTarget)
{
	if (pManager->MaxControlNodes == 1)
		return pManager->CanCapture(pTarget);

	if (TechnoTypeExt::ExtMap.Find(pManager->Owner->GetTechnoType())->MultiMindControl_ReleaseVictim)
	{
		// I hate Ares' completely rewritten things - secsome
		pManager->MaxControlNodes += 1;
		bool result = pManager->CanCapture(pTarget);
		pManager->MaxControlNodes -= 1;
		return result;
	}

	return pManager->CanCapture(pTarget);
}

bool CaptureExt::FreeUnit(CaptureManagerClass* pManager, TechnoClass* pTarget, bool bSilent)
{
	if (pTarget)
	{
		for (int i = pManager->ControlNodes.Count - 1; i >= 0; --i)
		{
			const auto pNode = pManager->ControlNodes[i];
			if (pTarget == pNode->Unit)
			{
				if (pTarget->MindControlRingAnim)
				{
					pTarget->MindControlRingAnim->UnInit();
					pTarget->MindControlRingAnim = nullptr;
				}

				if (!bSilent)
				{
					int nSound = pTarget->GetTechnoType()->MindClearedSound;

					if (nSound == -1)
						nSound = RulesClass::Instance->MindClearedSound;
					if (nSound != -1)
						VocClass::PlayIndexAtPos(nSound, pTarget->GetCoords());
				}

				// Fix : Player defeated should not get this unit.
				auto const pOriginOwner = pNode->OriginalOwner->Defeated ?
					HouseExt::FindNeutral() : pNode->OriginalOwner;

				pTarget->SetOwningHouse(pOriginOwner);
				pManager->DecideUnitFate(pTarget);
				pTarget->MindControlledBy = nullptr;

				if (pNode)
					GameDelete(pNode);

				pManager->ControlNodes.RemoveItem(i);

				return true;
			}
		}
	}

	return false;
}

bool CaptureExt::CaptureUnit(CaptureManagerClass* pManager, TechnoClass* pTarget,
	bool bRemoveFirst, AnimTypeClass* pControlledAnimType)
{
	if (CaptureExt::CanCapture(pManager, pTarget))
	{
		if (pManager->MaxControlNodes <= 0)
			return false;

		if (!pManager->InfiniteMindControl)
		{
			if (pManager->MaxControlNodes == 1 && pManager->ControlNodes.Count == 1)
				CaptureExt::FreeUnit(pManager, pManager->ControlNodes[0]->Unit);
			else if (pManager->ControlNodes.Count == pManager->MaxControlNodes)
				if (bRemoveFirst)
					CaptureExt::FreeUnit(pManager, pManager->ControlNodes[0]->Unit);
		}

		if (auto pControlNode = GameCreate<ControlNode>(pTarget, pTarget->Owner, RulesClass::Instance->MindControlAttackLineFrames))
		{
			pManager->ControlNodes.AddItem(pControlNode);

			if (pTarget->SetOwningHouse(pManager->Owner->Owner))
			{
				pTarget->MindControlledBy = pManager->Owner;

				pManager->DecideUnitFate(pTarget);

				auto const pWhat = pTarget->WhatAmI();
				auto const pBld = pWhat == AbstractType::Building ? static_cast<BuildingClass*>(pTarget) : nullptr;
				auto const pType = pTarget->GetTechnoType();
				CoordStruct location = pTarget->GetCoords();

				if (pBld)
					location.Z += pBld->Type->Height * Unsorted::LevelHeight;
				else
					location.Z += pType->MindControlRingOffset;

				if (auto const pAnimType = pControlledAnimType)
				{
					if (auto const pAnim = GameCreate<AnimClass>(pAnimType, location))
					{
						pTarget->MindControlRingAnim = pAnim;
						pAnim->SetOwnerObject(pTarget);

						if (pBld)
							pAnim->ZAdjust = -1024;
					}
				}

//#ifdef ENABLE_NEWHOOKS
				//if ((pWhat == AbstractType::Unit || pWhat == AbstractType::Infantry || pWhat == AbstractType::Aircraft) && pTarget->Target && pManager->Owner->Owner->IsControlledByHuman()) {
				//	pTarget->SetTarget(nullptr);
				//	pTarget->Override_Mission(Mission::Guard, nullptr,nullptr);
				//	pTarget->SetDestination(pTarget->GetCell(), true);
				//}
//#endif
				return true;
			}
		}
	}

	return false;
}

bool CaptureExt::CaptureUnit(CaptureManagerClass* pManager, TechnoClass* pTechno, AnimTypeClass* pControlledAnimType)
{
	if (pTechno) {
		return CaptureExt::CaptureUnit(pManager, pTechno, TechnoTypeExt::ExtMap.Find(pManager->Owner->GetTechnoType())->MultiMindControl_ReleaseVictim, pControlledAnimType);
	}

	return false;
}

void CaptureExt::DecideUnitFate(CaptureManagerClass* pManager, FootClass* pFoot)
{
	// to be implemented (if needed). - secsome
}

// =============================
// load / save

template <typename T>
void CaptureExt::ExtData::Serialize(T& Stm) { }

void CaptureExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<CaptureManagerClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void CaptureExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<CaptureManagerClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool CaptureExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool CaptureExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}
// =============================
// container

CaptureExt::ExtContainer::ExtContainer() : Container("CaptureManagerClass") { };
CaptureExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks


//DEFINE_HOOK(0x471887, CaptureManagerClass_CTOR, 0x6)
//{
//	GET(CaptureManagerClass* const, pItem, ESI);
//
//	CaptureExt::ExtMap.Allocate(pItem);
//
//	return 0;
//}
//
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