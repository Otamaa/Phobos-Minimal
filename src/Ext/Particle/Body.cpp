#include "Body.h"

#include "Ext/Bullet/Body.h"
#include "Ext/ParticleType/Body.h"
#include <Utilities/Macro.h>

#include <Misc/DynamicPatcher/Trails/TrailsManager.h>

std::pair<TechnoClass*, HouseClass*> ParticleExtData::GetOwnership(ParticleClass* pThis)
{
	TechnoClass* pAttacker = nullptr;
	HouseClass* pOwner = nullptr;
	BulletClass* pBullet = nullptr;

	if (auto const pSystem = pThis->ParticleSystem)
	{
		if (auto pSystemOwner = pSystem->Owner)
		{
			if (((pSystemOwner->AbstractFlags & AbstractFlags::Techno) != AbstractFlags::None))
				pAttacker = static_cast<TechnoClass*>(pSystemOwner);
			else if (pSystemOwner->WhatAmI() == BulletClass::AbsID)
			{
				pBullet = static_cast<BulletClass*>(pSystemOwner);
				pAttacker = static_cast<BulletClass*>(pSystemOwner)->Owner;
			}

		}

		if (pAttacker)
			pOwner = pAttacker->GetOwningHouse();
		else if (pBullet)
			pOwner = BulletExtContainer::Instance.Find(pBullet)->Owner;
	}

	return { pAttacker , pOwner };
}


// =============================
// load / save

template <typename T>
void ParticleExtData::Serialize(T& Stm)
{
	//Debug::Log("Processing Element From ParticleExt ! \n");

	Stm
		.Process(this->Initialized)
		.Process(this->LaserTrails)
		.Process(this->Trails)

		;
}

// =============================
// container
ParticleExtContainer ParticleExtContainer::Instance;
std::vector<ParticleExtData*> ParticleExtContainer::Pool;

// =============================
// container hooks

DEFINE_HOOK(0x62BB13, ParticleClass_CTOR, 0x5)
{
	GET(ParticleClass*, pItem, ESI);

	if (auto pExt = ParticleExtContainer::Instance.Allocate(pItem))
	{
		if (const auto pTypeExt = ParticleTypeExtContainer::Instance.TryFind(pItem->Type))
		{
			CoordStruct nFLH = CoordStruct::Empty;
			const ColorStruct nColor = pItem->GetOwningHouse() ? pItem->GetOwningHouse()->LaserColor : ColorStruct::Empty;

			if (pExt->LaserTrails.empty() && !LaserTrailTypeClass::Array.empty())
			{
				pExt->LaserTrails.reserve(pTypeExt->LaserTrail_Types.size());

				for (auto const& idxTrail : pTypeExt->LaserTrail_Types)
				{
					pExt->LaserTrails.emplace_back(LaserTrailTypeClass::Array[idxTrail].get(), nColor, nFLH);
				}
			}
		}

		TrailsManager::Construct(pItem);
	}

	return 0;
}

DEFINE_HOOK_AGAIN(0x62BCED, ParticleClass_DTOR, 0xA)
DEFINE_HOOK(0x62D9CD, ParticleClass_DTOR, 0xA)
{
	GET(ParticleClass* const, pItem, ESI);
	ParticleExtContainer::Instance.Remove(pItem);
	return 0;
}

#include <Misc/Hooks.Otamaa.h>

HRESULT __stdcall FakeParticleClass::_Load(IStream* pStm)
{

	ParticleExtContainer::Instance.PrepareStream(this, pStm);
	HRESULT res = this->ParticleClass::Load(pStm);

	if (SUCCEEDED(res))
		ParticleExtContainer::Instance.LoadStatic();

	return res;
}

HRESULT __stdcall FakeParticleClass::_Save(IStream* pStm, bool clearDirty)
{

	ParticleExtContainer::Instance.PrepareStream(this, pStm);
	HRESULT res = this->ParticleClass::Save(pStm, clearDirty);

	if (SUCCEEDED(res))
		ParticleExtContainer::Instance.SaveStatic();

	return res;
}

DEFINE_JUMP(VTABLE, 0x7EF968, MiscTools::to_DWORD(&FakeParticleClass::_Load))
DEFINE_JUMP(VTABLE, 0x7EF96C, MiscTools::to_DWORD(&FakeParticleClass::_Save))

void FakeParticleClass::_Detach(AbstractClass* pTarget, bool bRemove)
{
	this->ObjectClass::PointerExpired(pTarget ,bRemove);

	//ParticleExt::ExtMap.InvalidatePointerFor(pThis, pTarget, bRemove);
}

DEFINE_JUMP(VTABLE, 0x7EF97C, MiscTools::to_DWORD(&FakeParticleClass::_Detach))