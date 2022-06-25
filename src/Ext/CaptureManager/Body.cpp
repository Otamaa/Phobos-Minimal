#include "Body.h"

#include <Ext/TechnoType/Body.h>
#include <Ext/House/Body.h>

template<> const DWORD TExtension<CaptureExt::base_type>::Canary = 0x87654121;
CaptureExt::ExtContainer CaptureExt::ExtMap;

void CaptureExt::ExtData::InitializeConstants() { }

bool CaptureExt::CanCapture(CaptureManagerClass* pManager, TechnoClass* pTarget)
{
	if (pManager->MaxControlNodes == 1)
		return pManager->CanCapture(pTarget);

	auto pTechnoTypeExt = TechnoTypeExt::GetExtData(pManager->Owner->GetTechnoType());
	if (pTechnoTypeExt && pTechnoTypeExt->MultiMindControl_ReleaseVictim)
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

		if (auto pControlNode = GameCreate<ControlNode>())
		{
			pControlNode->OriginalOwner = pTarget->Owner;
			pControlNode->Unit = pTarget;

			pManager->ControlNodes.AddItem(pControlNode);
			pControlNode->LinkDrawTimer.Start(RulesClass::Instance->MindControlAttackLineFrames);

			if (pTarget->SetOwningHouse(pManager->Owner->Owner))
			{
				pTarget->MindControlledBy = pManager->Owner;

				pManager->DecideUnitFate(pTarget);

				auto const pBld = abstract_cast<BuildingClass*>(pTarget);
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


				pTarget->QueueMission(Mission::Move, false);
				if (pManager->Owner->IsHumanControlled && pTarget->Target)
					pTarget->SetDestination(pTarget->GetCell(), true);
				{
					if (auto pTargetFoot = abstract_cast<FootClass*>(pTarget))
					{
						pTargetFoot->SetTarget(nullptr);
						pTargetFoot->QueueMission(Mission::Guard, false);
						pTargetFoot->SetDestination(pTarget->GetCell(), true);
						pTargetFoot->NextMission();
					}
				}

				return true;
			}

		}
	}

	return false;
}

bool CaptureExt::CaptureUnit(CaptureManagerClass* pManager, TechnoClass* pTechno, AnimTypeClass* pControlledAnimType)
{
	if (pTechno)
	{
		const auto pTarget = pTechno->AbsDerivateID & AbstractFlags::Techno ? pTechno : nullptr;

		bool bRemoveFirst = false;
		if (auto pTechnoTypeExt = TechnoTypeExt::GetExtData(pManager->Owner->GetTechnoType()))
			bRemoveFirst = pTechnoTypeExt->MultiMindControl_ReleaseVictim;

		return CaptureExt::CaptureUnit(pManager, pTarget, bRemoveFirst, pControlledAnimType);
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
	this->Serialize(Stm);
}

void CaptureExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
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

CaptureExt::ExtContainer::ExtContainer() : TExtensionContainer("CaptureManagerClass") { };
CaptureExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x471832, CaptureManagerClass_CTOR, 0x9)
{
	GET(CaptureManagerClass* const, pItem, ESI);
	ExtensionWrapper::GetWrapper(pItem)->CreateExtensionObject<CaptureExt::ExtData>(pItem);
	return 0;
}

DEFINE_HOOK(0x4729E1, CaptureManagerClass_DTOR, 0xD)
{
	GET(CaptureManagerClass* const, pItem, ESI);
	if (auto pExt = ExtensionWrapper::GetWrapper(pItem)->ExtensionObject)
		pExt->Uninitialize();

	ExtensionWrapper::GetWrapper(pItem)->DestoryExtensionObject();
	return 0;
}

DEFINE_HOOK_AGAIN(0x4728E0, CaptureManagerClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x472720, CaptureManagerClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(CaptureManagerClass*, pThis, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	CaptureExt::ExtMap.PrepareStream(pThis, pStm);

	return 0;
}

DEFINE_HOOK(0x4728CA, CaptureManagerClass_Load_Suffix, 0xA)
{
	CaptureExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x472958, CaptureManagerClass_Save_Suffix, 0x7)
{
	CaptureExt::ExtMap.SaveStatic();
	return 0;
}