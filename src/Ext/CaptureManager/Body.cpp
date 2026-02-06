#include "Body.h"

#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/House/Body.h>
#include <Ext/WarheadType/Body.h>

AnimTypeClass* CaptureExtData::GetMindcontrollAnimType(TechnoClass* pController, TechnoClass* pTarget, AnimTypeClass* pFallback)
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

bool FakeCaptureManagerClass::__FreeUnit(TechnoClass* pTarget, bool bSilent)
{
	if (pTarget) {
		for (int i = this->ControlNodes.Count - 1; i >= 0; --i) {
			const auto pNode = this->ControlNodes[i];

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
				int nSound = GET_TECHNOTYPE(pTarget)->MindClearedSound;

					if (nSound == -1)
						nSound = RulesClass::Instance->MindClearedSound;

					VocClass::SafeImmedietelyPlayAt(nSound, pTarget->GetCoords());
				}

				// Fix : Player defeated should not get this unit.
				const auto pOriginOwner = !pNode->OriginalOwner ||
					pNode->OriginalOwner->Defeated ?
					HouseExtData::FindNeutral() : pNode->OriginalOwner;

				TechnoExtContainer::Instance.Find(pTarget)->BeControlledThreatFrame = 0;

				pTarget->SetOwningHouse(pOriginOwner, !bSilent);
				this->__DecideUnitFate(pTarget, false);
				pTarget->MindControlledBy = nullptr;

				// Erase the node and delete it
				this->ControlNodes.erase_at(i);
				GameDelete<false, false>(pNode);

				return true;
			}
		}
	}

	return false;
}
bool FakeCaptureManagerClass::__FreeUnit_Wrap(TechnoClass* pTarget)
{
	return __FreeUnit(pTarget, false);
}
#include <Ext/Building/Body.h>

// new CaptureUnit function that inclued new features
bool FakeCaptureManagerClass::__CaptureUnit(TechnoClass* pTarget,
	bool bRemoveFirst, bool bSilent, AnimTypeClass* pControlledAnimType, int threatDelay)
{
	if (this->__CanCapture(pTarget))
	{
		if (this->MaxControlNodes <= 0)
			return false;

		if (!this->InfiniteMindControl)
		{
			if (this->MaxControlNodes == 1 && this->ControlNodes.Count == 1)
				this->__FreeUnit(this->ControlNodes[0]->Unit , bSilent);
			else if (this->ControlNodes.Count == this->MaxControlNodes && bRemoveFirst){
				auto pOwnerTypeExt = GET_TECHNOTYPEEXT(this->Owner);

				if (pOwnerTypeExt->MindControl_IgnoreSize) {
					if (this->ControlNodes.Count == this->MaxControlNodes)
						this->__FreeUnit(this->ControlNodes[0]->Unit, false);
				} else {
					const auto pTargetTypeExt = GET_TECHNOTYPEEXT(pTarget);

					while (this->ControlNodes.Count && pTargetTypeExt->MindControlSize > this->MaxControlNodes - this->__GetControlledTotalSize())
						this->__FreeUnit(this->ControlNodes[0]->Unit, false);
				}
			}
		}

		{
			auto pControlNode = GameCreate<ControlNode>(pTarget, pTarget->Owner, RulesClass::Instance->MindControlAttackLineFrames);
			this->ControlNodes.push_back(pControlNode);

			if (threatDelay > 0) {
				TechnoExtContainer::Instance.Find(pTarget)->BeControlledThreatFrame = Unsorted::CurrentFrame() + threatDelay;
			}

			const auto pBld = cast_to<BuildingClass*, false>(pTarget);

			//if (pBld) {
			//	BuildingExtContainer::Instance.Find(pBld)->BeignMCEd = true;
			//}

			if (pTarget->SetOwningHouse(this->Owner->Owner, !bSilent))
			{
				pTarget->MindControlledBy = this->Owner;

				this->__DecideUnitFate(pTarget, true);

				if (pControlledAnimType)
				{
					const auto pType = GET_TECHNOTYPE(pTarget);
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
				//if (pBld) {
				//	BuildingExtContainer::Instance.Find(pBld)->BeignMCEd = false;
				//}
			}
		}
	}

	return false;
}

bool FakeCaptureManagerClass::__CaptureUnit(TechnoClass* pTechno, bool bSilent, int threatDelay)
{
	if (pTechno) {
		return this->__CaptureUnit(pTechno,
		GET_TECHNOTYPEEXT(this->Owner)->MultiMindControl_ReleaseVictim,
		bSilent, RulesClass::Instance->ControlledAnimationType , threatDelay);
	}

	return false;
}

bool FakeCaptureManagerClass::__CaptureUnit_Wrap(AbstractClass* pTechno)
{
	return this->__CaptureUnit(flag_cast_to<TechnoClass*>(pTechno), false, 0);
}

#include <Ext/Team/Body.h>
#include <Ext/Temporal/Body.h>

//this used for direct patching , maybe it is better to use syringe then check where it called ?
//to decude captured bool
void FakeCaptureManagerClass::__DecideUnitFate_Wrap(TechnoClass* pTechno){
	this->__DecideUnitFate(pTechno, false);
}

void FakeCaptureManagerClass::__DecideUnitFate(TechnoClass* pTechno , bool Captured)
{
	if(!pTechno)
		return;

	auto pFoot = flag_cast_to<FootClass*,false>(pTechno);

	if(pFoot){
		if(auto pTeam = (FakeTeamClass*)pFoot->Team){
			pTeam->_Remove(pFoot , -1 , 0);
		}
	}

	if(auto pTemporal = (FakeTemporalClass*)pTechno->TemporalImUsing){
		if(auto pTemporalTarget = pTemporal->Target){
			pTemporal->ResetTemporalStateAndIdle();
		}
	}

	auto pTargetType = GET_TECHNOTYPE(pTechno);

	if(pTargetType->OpenTopped){
		pTechno->SetTargetForPassengers(nullptr);
	}

	 HouseClass* unitHouse =	pTechno->Owner;
    HouseClass* ownerHouse = this->Owner->Owner;
    bool bOwnershipChange = (unitHouse != ownerHouse);

	if (unitHouse->IsInPlayerControl) {
        return;
    }

	// Neutral techno should not do anything after getting freed/captured
	if(pTechno->Owner->IsNeutral()) {
        pTechno->Override_Mission(Mission::Sleep, nullptr, nullptr);
        return;
	}

	auto victimWhat = pTechno->WhatAmI();
	// Buildings get special treatment
	if (victimWhat == AbstractType::Building) {
		// Buildings can't really join teams or do other unit fates
		// BuildingClass::Mission_Hunt() does nothing anyway
		pTechno->QueueMission(Mission::Guard, false);
		return;
	}

	// Determine AI decision condition and action list
    int nCondition = 0;
    TypeList<int> *actionList;

    int availableMoney = ownerHouse->Available_Money();
    
    if (availableMoney >= RulesClass::Instance->AICaptureLowMoneyMark) {
        if (this->Owner->Owner->GetPowerPercentage() >= 1.0) {
            int healthRatio = (int)pTechno->GetHealthPercentage();
            
            if (healthRatio >= RulesClass::Instance->AICaptureWoundedMark) {
                nCondition = 3;
                actionList = &RulesClass::Instance->AICaptureNormal;
            } else {
                nCondition = 2;
                actionList = &RulesClass::Instance->AICaptureWounded;
            }
        } else {
            nCondition = 1;
            actionList = &RulesClass::Instance->AICaptureLowPower;
        }
    } else {
        nCondition = 0;
        actionList = &RulesClass::Instance->AICaptureLowMoney;
    }

    // Roll dice and determine action
    int roll = ScenarioClass::Instance->Random.RandomRanged(1, 100);
    int nAction = 0; // Default action
    int cumulativeWeight = 0;

    for (int i = 0; i < actionList->Count && i < 6; ++i) {
        int weight = actionList->Items[i];
        cumulativeWeight += weight;
        
        if (roll <= cumulativeWeight) {
            nAction = i;
            break;
        }
    }

	// Decision condition strings
	static COMPILETIMEEVAL const char* MCDecisionMaker[] = {
		"Low on Money",      // 0
		"Low on Power",      // 1
		"Unit is Wounded",   // 2
		"Nothing Special"    // 3
	};

	// Decision action strings
	static COMPILETIMEEVAL const char* MCDecisionAction[] = {
		"<Don't care>",       // 0
		"Add To Team",        // 1
		"Put in Grinder",     // 2
		"Put in Bio Reactor", // 3
		"Go to Hunt",         // 4
		"Do Nothing"          // 5
	};

    Debug::Log(
        "AICapture: I think (%s) so I roll a %d and decide (%s) with a %s.\n",
        MCDecisionMaker[nCondition],
        roll,
        MCDecisionAction[nAction],
		pTargetType->Name);

	auto pFootOwner = flag_cast_to<FootClass*>(this->Owner);

    if (pFootOwner) {
        if (TeamClass* team = pFootOwner->Team) {
            int nTeamDecision = team->Type->MindControlDecision;
            
            if (nTeamDecision) {

				if (nTeamDecision > 5)
					nTeamDecision = ScenarioClass::Instance->Random.RandomRanged(1, 5);
                
				Debug::Log("AICapture: Capturer's Team overrides with (%s).\n", MCDecisionAction[nTeamDecision]);

                nAction = nTeamDecision;
            }
        }
    }

    // Execute action
    switch (nAction) {
        case 1: // Add to team
            if (bOwnershipChange) {
                // Can't add to team if ownership changed
                break;
            }
            
            if (pFootOwner) {
                if (FakeTeamClass* team = (FakeTeamClass*)pFootOwner->Team) {
                    if (team->_Add(pFoot)) {
                        return;
                    }
                }
            }
            break;

        case 2: // Grind
            if (pTechno->EnterGrinder()) {
                return;
            }
            break;

        case 3: // Absorb
            if (pTechno->EnterBioReactor()) {
                return;
            }
            break;

        case 5: // Do nothing
            return;

        default: {
			break;
		}
    }

	// Fallback: assign hunt mission
	Debug::Log("AICapture: Naw, let's put him in hunt.\n");
	pTechno->QueueMission(Mission::Hunt, false);
}

bool FakeCaptureManagerClass::__Should_Draw_Link()
{
	// ------------------------------------------------------------
	// Check if owner is selected
	// ------------------------------------------------------------
	if (this->Owner->IsSelected)
		return true;

	// ------------------------------------------------------------
	// Check if owner's transport is selected
	// ------------------------------------------------------------
	TechnoClass* pTransport = this->Owner->Transporter;

	if (pTransport && pTransport->IsSelected)
		return true;

	// ------------------------------------------------------------
	// 	
	const auto pExt = GET_TECHNOTYPEEXT(this->Owner);

	if (HouseClass::Observer != HouseClass::CurrentPlayer 
		&& !EnumFunctions::CanTargetHouse(pExt->Draw_MindControlLink, this->Owner->Owner, HouseClass::CurrentPlayer))
		return false;

	// ------------------------------------------------------------
	// ------------------------------------------------------------
	// Check each controlled unit
	// ------------------------------------------------------------
	for (int i = this->ControlNodes.Count - 1; i >= 0; --i)
	{
		ControlNode* pNode = this->ControlNodes.Items[i];

		// Controlled unit is selected
		if (pNode->Unit->IsSelected)
			return true;

		// Attack line timer is active
		if (pNode->LinkDrawTimer.HasTimeLeft())
			return true;
	}

	return false;
}

int FakeCaptureManagerClass::__GetControlledCount()
{
	const auto pOwnerTypeExt = GET_TECHNOTYPEEXT(this->Owner);

	if (!pOwnerTypeExt->MindControl_IgnoreSize) {
		return this->__GetControlledTotalSize();
	}

	return this->ControlNodes.Count;
}

int FakeCaptureManagerClass::__GetControlledTotalSize() {
	int totalSize = 0;

	for (const auto& pNode : this->ControlNodes) {
		if (pNode) {
			if (const auto pTechno = pNode->Unit) {
				totalSize += GET_TECHNOTYPEEXT(pTechno)->MindControlSize;
			}
		}
	}

	return totalSize;
}

bool FakeCaptureManagerClass::__IsOverloading(bool* isIt)
{
	if (!this->InfiniteMindControl || this->__GetControlledCount() < this->MaxControlNodes) {
		return false;
	}

	*isIt = this->OverloadPipState > 0;
	return true;
}

bool FakeCaptureManagerClass::__CanControlMore()
{
	return this->InfiniteMindControl || this->__GetControlledCount() < this->MaxControlNodes;
}

bool FakeCaptureManagerClass::__CanCapture(TechnoClass* pTarget)
{
	// this is a complete rewrite, because it might be easier to change
	// this in a central place than spread all over the source code.
	TechnoClass* pCapturer = this->Owner;

	// target exists and doesn't belong to capturing player
	if (!pTarget || pTarget->Owner == pCapturer->Owner)
	{
		return false;
	}
	auto pTypeExt = GET_TECHNOTYPEEXT(pCapturer);
	// generally not capturable
	if (TechnoExtData::IsPsionicsImmune(pTarget))
	{
		return false;
	}

	// disallow capturing bunkered units
	if (pTarget->BunkerLinkedItem)
	{
		return false;
	}

	// TODO: extend this for mind-control priorities
	if (pTarget->IsMindControlled() || pTarget->MindControlledByHouse)
	{
		return false;
	}

	// free slot? (move on if infinite or single slot which will be freed if used)
	if (!this->InfiniteMindControl && this->MaxControlNodes != 1) {
		if (pTypeExt->MindControl_IgnoreSize) {
			if (this->ControlNodes.Count >= this->MaxControlNodes && !pTypeExt->MultiMindControl_ReleaseVictim) {
				return false;
			}
		} else {
			const int totalSize = this->__GetControlledTotalSize();
			const int available = pTypeExt->MultiMindControl_ReleaseVictim ? this->MaxControlNodes : this->MaxControlNodes - totalSize;

			if (GET_TECHNOTYPEEXT(pTarget)->MindControlSize > available)
				return false;
		}
	}

	// currently disallowed
	auto mission = pTarget->CurrentMission;
	if (pTarget->IsIronCurtained() || mission == Mission::Selling || mission == Mission::Construction) {
		return false;
	}

	// driver killed. has no mind.
	if (TechnoExtContainer::Instance.Find(pTarget)->Is_DriverKilled) {
		return false;
	}

	// passed all tests
	return true;
}

void FakeCaptureManagerClass::__DrawControlLinks()
{
	bool ownerIsSelected = this->Owner->IsSelected;

	for (int i = this->ControlNodes.Count - 1; i >= 0; --i)
	{
		ControlNode* pNode = this->ControlNodes.Items[i];
		TechnoClass* pControlled = pNode->Unit;

		/* #367 - do we need to draw a link to this victim */
		if (FootClass* pFoot = flag_cast_to<FootClass*, false>(this->Owner)) {
			if (pFoot->ParasiteImUsing && pFoot->InLimbo) {
				continue;
			}
		}
	
		// --------------------------------------------------------
		// Calculate highlight state
		// --------------------------------------------------------
		bool shouldHighlight = pNode->LinkDrawTimer.HasTimeLeft();

		if (pControlled->IsSelected)
			shouldHighlight = true;

		// --------------------------------------------------------
		// Skip if conditions not met
		// --------------------------------------------------------
		if (!this->Owner || !pControlled || (!ownerIsSelected && !shouldHighlight))
			continue;

		auto pType = GET_TECHNOTYPE(pControlled);
		// --------------------------------------------------------
		// Draw the mind control link
		// --------------------------------------------------------
		ColorStruct laserColor = this->Owner->Owner->LaserColor;
		CoordStruct target = pControlled->Location;
				    target.Z += pType->LeptonMindControlOffset;

		CoordStruct sourcePos;
		this->Owner->GetFLH(&sourcePos, -1 - (i % 5), CoordStruct::Empty);

		Drawing::DrawLinesTo(sourcePos, target, laserColor);
	}
}

bool FakeCaptureManagerClass::__SetOwnerToCivilian(TechnoClass* pTarget){
	HouseClass* pCivilian = HouseExtData::FindFirstCivilianHouse();
	if(!pCivilian)
		return false;

	for (int i = this->ControlNodes.Count - 1; i >= 0; --i){
		ControlNode* pNode = this->ControlNodes.Items[i];
		TechnoClass* pControlled = pNode->Unit;

		if(pControlled == pTarget) {
			pNode->OriginalOwner = pCivilian;
		}
	}

	return true;
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

//ASMJIT_PATCH_AGAIN(0x471998, CaptureManagerClass_CTOR, 0x6) // factory
//ASMJIT_PATCH(0x471887, CaptureManagerClass_CTOR, 0x6)
//{
//	GET(CaptureManagerClass* const, pItem, ESI);
//
//	CaptureExt::ExtMap.Allocate(pItem);
//
//	return 0;
//}
//
//ASMJIT_PATCH_AGAIN(0x4718ED , CaptureManagerClass_DTOR , 0x6) // Factory
//ASMJIT_PATCH(0x4729EF, CaptureManagerClass_DTOR, 0x7)
//{
//	GET(CaptureManagerClass* const, pItem, ESI);
//	CaptureExt::ExtMap.Remove(pItem);
//	return 0;
//}
//
//ASMJIT_PATCH_AGAIN(0x4728E0, CaptureManagerClass_SaveLoad_Prefix, 0x5)
//ASMJIT_PATCH(0x472720, CaptureManagerClass_SaveLoad_Prefix, 0x8)
//{
//	GET_STACK(CaptureManagerClass*, pThis, 0x4);
//	GET_STACK(IStream*, pStm, 0x8);
//	CaptureExt::ExtMap.PrepareStream(pThis, pStm);
//	return 0;
//}
//
//ASMJIT_PATCH(0x4728CA, CaptureManagerClass_Load_Suffix, 0x7)
//{
//	GET(HRESULT, nRes, EAX);
//
//	if(SUCCEEDED(nRes))
//		CaptureExt::ExtMap.LoadStatic();
//
//	return 0;
//}
//
//ASMJIT_PATCH(0x472958, CaptureManagerClass_Save_Suffix, 0x7)
//{
//	GET(HRESULT, nRes, EAX);
//
//	if (SUCCEEDED(nRes))
//		CaptureExt::ExtMap.SaveStatic();
//
//	return 0;
//}