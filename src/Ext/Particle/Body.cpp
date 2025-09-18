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
	Stm
		.Process(this->LaserTrails)
		.Process(this->Trails)

		;
}

// =============================
// container
ParticleExtContainer ParticleExtContainer::Instance;
std::vector<ParticleExtData*> Container<ParticleExtData>::Array;

void Container<ParticleExtData>::Clear()
{
	Array.clear();
}

bool ParticleExtContainer::LoadGlobals(PhobosStreamReader& Stm)
{
	return LoadGlobalArrayData(Stm);
}

bool ParticleExtContainer::SaveGlobals(PhobosStreamWriter& Stm)
{
	return SaveGlobalArrayData(Stm);
}

// =============================
// container hooks

ASMJIT_PATCH(0x62BB13, ParticleClass_CTOR, 0x5)
{
	GET(ParticleClass*, pItem, ESI);

	if (pItem->Type) {
		auto pExt = ParticleExtContainer::Instance.Allocate(pItem);
		const auto pTypeExt = ParticleTypeExtContainer::Instance.Find(pItem->Type);
		CoordStruct nFLH = CoordStruct::Empty;
		const ColorStruct nColor = pItem->GetOwningHouse() ? pItem->GetOwningHouse()->LaserColor : ColorStruct::Empty;

		if (pExt->LaserTrails.empty() && !LaserTrailTypeClass::Array.empty())
		{
			pExt->LaserTrails.reserve(pTypeExt->LaserTrail_Types.size());

			for (auto const& idxTrail : pTypeExt->LaserTrail_Types)
			{
				pExt->LaserTrails.emplace_back(
					std::move(std::make_unique<LaserTrailClass>(
						LaserTrailTypeClass::Array[idxTrail].get(), nColor, nFLH)));
			}
		}

		TrailsManager::Construct(pItem);
	}

	return 0;
}

ASMJIT_PATCH(0x62D9CD, ParticleClass_DTOR, 0xA)
{
	GET(ParticleClass* const, pItem, ESI);
	ParticleExtContainer::Instance.Remove(pItem);
	return 0;
}ASMJIT_PATCH_AGAIN(0x62BCED, ParticleClass_DTOR, 0xA)

void FakeParticleClass::_Detach(AbstractClass* pTarget, bool bRemove)
{
	this->ObjectClass::PointerExpired(pTarget ,bRemove);

	//ParticleExt::ExtMap.InvalidatePointerFor(pThis, pTarget, bRemove);
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7EF97C, FakeParticleClass::_Detach)

HRESULT __stdcall FakeParticleClass::_Load(IStream* pStm)
{
	HRESULT hr = this->ParticleClass::Load(pStm);
	if (SUCCEEDED(hr))
		hr = ParticleExtContainer::Instance.LoadKey(this, pStm);

	return hr;
}

HRESULT __stdcall FakeParticleClass::_Save(IStream* pStm, BOOL clearDirty)
{
	HRESULT hr = this->ParticleClass::Save(pStm, clearDirty);
	if (SUCCEEDED(hr))
		hr = ParticleExtContainer::Instance.SaveKey(this, pStm);

	return hr;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7EF968, FakeParticleClass::_Load)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7EF96C, FakeParticleClass::_Save)