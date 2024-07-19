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

DEFINE_HOOK_AGAIN(0x62D810, ParticleClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x62D7A0, ParticleClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(ParticleClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	ParticleExtContainer::Instance.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x62D801, ParticleClass_Load_Suffix, 0x6)
{
	ParticleExtContainer::Instance.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x62D825, ParticleClass_Save_Suffix, 0x7)
{
	GET(const HRESULT , nRest, EAX);

	ParticleExtContainer::Instance.GetSavingObject()->byte130 = true;
	if (SUCCEEDED(nRest)) {
		ParticleExtContainer::Instance.SaveStatic();
	}

	return 0x62D82C;
}

static void __fastcall ParticleClass_Detach(ParticleClass* pThis, void* _, AbstractClass* pTarget, bool bRemove)
{
	pThis->ObjectClass::PointerExpired(pTarget ,bRemove);

	//ParticleExt::ExtMap.InvalidatePointerFor(pThis, pTarget, bRemove);
}

DEFINE_JUMP(VTABLE, 0x7EF97C, GET_OFFSET(ParticleClass_Detach))