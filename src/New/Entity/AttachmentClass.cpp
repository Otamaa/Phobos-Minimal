#include "AttachmentClass.h"

#include <DirStruct.h>
#include <BulletClass.h>
#include <BulletTypeClass.h>
#include <WarheadTypeClass.h>
#include <CaptureManagerClass.h>

#include <ObjBase.h>

#include <Ext/Techno/Body.h>
#include <New/Interfaces/AttachmentLocomotionClass.h>

HelperedVector<AttachmentClass*> AttachmentClass::Array;

AttachmentTypeClass* AttachmentClass::GetType()
{
	return AttachmentTypeClass::Array[this->Data->Type].get();
}

TechnoTypeClass* AttachmentClass::GetChildType()
{
	return (size_t)this->Data->TechnoType < TechnoTypeClass::Array->Count
		? TechnoTypeClass::Array->Items[this->Data->TechnoType]
		: nullptr;
}

CoordStruct AttachmentClass::GetChildLocation()
{
	return TechnoExtData::GetFLHAbsoluteCoords(this->Parent, this->Data->FLH, this->Data->IsOnTurret);
}

AttachmentClass::~AttachmentClass()
{
	// clean up non-owning references
	if (this->Child)
	{
		auto const& pChildExt = TechnoExtContainer::Instance.Find(Child);
		pChildExt->ParentAttachment = nullptr;
	}

	Array.remove(this);
}

void AttachmentClass::Initialize()
{
	if (this->Child)
		return;

	if (this->GetType()->RespawnAtCreation)
		this->CreateChild();
}

void AttachmentClass::CreateChild()
{
	if (auto const pChildType = this->GetChildType())
	{
		if (pChildType->WhatAmI() != AbstractType::UnitType)
			return;

		// clear the mutexes temporally
		// this is really dangerious that can cause issues
		// since Mutex is there to make stuffs go wrong or overlap eachother
		int mutex_old = std::exchange(Unsorted::ScenarioInit(), 0);
		const auto pTechno = static_cast<TechnoClass*>(pChildType->CreateObject(this->Parent->Owner));
		Unsorted::ScenarioInit = mutex_old;

		if (pTechno) {
			this->AttachChild(pTechno);
		} else {
			Debug::Log("[" __FUNCTION__ "] Failed to create child %s of parent %s!\n",
				pChildType->ID, this->Parent->GetTechnoType()->ID);
		}
	}
}

void AttachmentClass::AI()
{
	AttachmentTypeClass* pType = this->GetType();

	if (!this->Child)
	{
		if (pType->RespawnDelay == 0)
		{
			this->CreateChild();
		}
		else if (pType->RespawnDelay > 0)
		{
			if (!this->RespawnTimer.HasStarted())
			{
				this->RespawnTimer.Start(pType->RespawnDelay);
			}
			else if (this->RespawnTimer.Completed())
			{
				this->CreateChild();
				this->RespawnTimer.Stop();
			}
		}
	}

	if (this->Child)
	{
		if (this->Child->InLimbo && !this->Parent->InLimbo)
			this->Unlimbo();
		else if (!this->Child->InLimbo && this->Parent->InLimbo)
			this->Limbo();

		this->Child->SetLocation(this->GetChildLocation());

		DirStruct childDir = this->Data->IsOnTurret
			? this->Parent->SecondaryFacing.Current() : this->Parent->PrimaryFacing.Current();

		childDir.Raw += DirStruct(this->Data->RotationAdjust).Raw; // overflow = free modulo for rotation

		this->Child->PrimaryFacing.Set_Current(childDir);
		// TODO handle secondary facing in case the turret is idle

		FootClass* pParentAsFoot = flag_cast_to<FootClass*, false>(this->Parent);
		FootClass* pChildAsFoot = flag_cast_to<FootClass*, false>(this->Child);
		if (pParentAsFoot && pChildAsFoot)
		{
			pChildAsFoot->TubeIndex = pParentAsFoot->TubeIndex;
		}

		if (pType->InheritStateEffects)
		{
			this->Child->IsFallingDown = this->Parent->IsFallingDown;
			this->Child->WasFallingDown = this->Parent->WasFallingDown;
			this->Child->CloakState = this->Parent->CloakState;
			this->Child->WarpingOut = this->Parent->WarpingOut;
			this->Child->unknown_280 = this->Parent->unknown_280; // sth related to teleport
			this->Child->BeingWarpedOut = this->Parent->BeingWarpedOut;
			this->Child->Deactivated = this->Parent->Deactivated;
			this->Child->Flash(this->Parent->Flashing.DurationRemaining);

			this->Child->IronCurtainTimer = this->Parent->IronCurtainTimer;
			this->Child->IdleActionTimer = this->Parent->IdleActionTimer;
			this->Child->IronTintTimer = this->Parent->IronTintTimer;
			this->Child->CloakDelayTimer = this->Parent->CloakDelayTimer;
			this->Child->ChronoLockRemaining = this->Parent->ChronoLockRemaining;
			this->Child->Berzerk = this->Parent->Berzerk;
			this->Child->BerzerkDurationLeft = this->Parent->BerzerkDurationLeft;
			this->Child->ChronoWarpedByHouse = this->Parent->ChronoWarpedByHouse;
			this->Child->EMPLockRemaining = this->Parent->EMPLockRemaining;
			this->Child->ShouldLoseTargetNow = this->Parent->ShouldLoseTargetNow;
		}

		if (pType->InheritOwner)
			this->Child->SetOwningHouse(this->Parent->GetOwningHouse(), false);
	}
}

// Called in Kill_Cargo, handles logics for parent destruction on children
void AttachmentClass::Destroy(TechnoClass * pSource)
{
	if (this->Child)
	{
		auto const pChildExt = TechnoExtContainer::Instance.Find(this->Child);
		pChildExt->ParentAttachment = nullptr;

		auto pType = this->GetType();

		if (pType->DestructionWeapon_Child.isset())
			TechnoExtData::FireWeaponAtSelf(this->Child, pType->DestructionWeapon_Child);

		if (pType->InheritDestruction && this->Child)
			TechnoExtData::Kill(this->Child, pSource);
		else if (!this->Child->InLimbo && pType->ParentDestructionMission.isset())
			this->Child->QueueMission(pType->ParentDestructionMission.Get(), false);

		this->Child = nullptr;
	}
}

void AttachmentClass::ChildDestroyed()
{
	if (this->Child)
	{
		if (auto const pChildExt = TechnoExtContainer::Instance.Find(this->Child))
			pChildExt->ParentAttachment = nullptr;

		AttachmentTypeClass* pType = this->GetType();
		if (pType->DestructionWeapon_Parent.isset())
			TechnoExtData::FireWeaponAtSelf(this->Parent, pType->DestructionWeapon_Parent);

		this->Child = nullptr;
	}
}

void AttachmentClass::Unlimbo()
{
	if (this->Child)
	{
		CoordStruct childCoord = TechnoExtData::GetFLHAbsoluteCoords(
			this->Parent, this->Data->FLH, this->Data->IsOnTurret);

		DirStruct childDir = this->Data->IsOnTurret
			? this->Parent->SecondaryFacing.Current() : this->Parent->PrimaryFacing.Current();

		childDir.Raw += DirStruct(this->Data->RotationAdjust).Raw; // overflow = free modulo for rotation

		++Unsorted::ScenarioInit;
		this->Child->Unlimbo(childCoord, childDir.GetDir());
		--Unsorted::ScenarioInit;
	}
}

void AttachmentClass::Limbo()
{
	if (this->Child)
		this->Child->Limbo();
}

bool AttachmentClass::AttachChild(TechnoClass * pChild)
{
	if (this->Child)
		return false;

	if (pChild->WhatAmI() != AbstractType::Unit)
		return false;

	auto const pChildAsFoot = flag_cast_to<FootClass*, false>(pChild);

	{
		if (IPersistPtr pLocoPersist = pChildAsFoot->Locomotor)
		{
			CLSID locoCLSID { };
			if (SUCCEEDED(pLocoPersist->GetClassID(&locoCLSID))
				&& locoCLSID != __uuidof(AttachmentLocomotionClass))
			{
				LocomotionClass::ChangeLocomotorTo(pChildAsFoot,
					__uuidof(AttachmentLocomotionClass));
			}
		}
	}

	this->Child = pChild;

	auto pChildExt = TechnoExtContainer::Instance.Find(this->Child);
	pChildExt->ParentAttachment = this;

	// bandaid for jitterless drawing. TODO fix properly
	// this->Child->GetTechnoType()->DisableVoxelCache = true;
	// this->Child->GetTechnoType()->DisableShadowCache = true;

	AttachmentTypeClass* pType = this->GetType();

	if (pType->InheritOwner)
	{
		if (auto pController = this->Child->MindControlledBy)
			pController->CaptureManager->FreeUnit(this->Child);
	}

	return true;
}

bool AttachmentClass::DetachChild()
{
	if (this->Child)
	{
		AttachmentTypeClass* pType = this->GetType();

		if (!this->Child->InLimbo && pType->ParentDetachmentMission.isset())
			this->Child->QueueMission(pType->ParentDetachmentMission.Get(), false);

		// FIXME this won't work probably
		if (pType->InheritOwner)
			this->Child->SetOwningHouse(this->Parent->GetOriginalOwner(), false);

		// remove the attachment locomotor manually just to be safe
		if (auto const pChildAsFoot = flag_cast_to<FootClass* , false>(this->Child))
			LocomotionClass::End_Piggyback(pChildAsFoot->Locomotor);

		auto pChildExt = TechnoExtContainer::Instance.Find(this->Child);
		pChildExt->ParentAttachment = nullptr;
		this->Child = nullptr;

		return true;
	}

	return false;
}


void AttachmentClass::InvalidatePointer(void* ptr)
{
	AnnounceInvalidPointer(this->Parent, ptr);
	AnnounceInvalidPointer(this->Child, ptr);
}

#pragma region Save/Load

template <typename T>
bool AttachmentClass::Serialize(T & stm)
{
	return stm
		.Process(this->Data)
		.Process(this->Parent)
		.Process(this->Child)
		.Process(this->RespawnTimer)
		.Success();
}

bool AttachmentClass::Load(PhobosStreamReader & stm, bool RegisterForChange)
{
	return Serialize(stm);
}

bool AttachmentClass::Save(PhobosStreamWriter & stm) const
{
	return const_cast<AttachmentClass*>(this)->Serialize(stm);
}

#pragma endregion