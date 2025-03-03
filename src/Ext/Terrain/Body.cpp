#include "Body.h"

#include <Ext/TerrainType/Body.h>
#include <Ext/Cell/Body.h>
#include <Ext/TechnoType/Body.h>

#include <Utilities/Macro.h>

void TerrainExtData::InvalidatePointer(AbstractClass* ptr, bool bRemoved)
{
	if (this->LighSource.get() == ptr) {
		this->LighSource.release();
	}

	if (this->AttachedAnim.get() == ptr) {
		this->AttachedAnim.release();
	}

}

bool TerrainExtData::CanMoveHere(TechnoClass* pThis, TerrainClass* pTerrain) {
	const auto pExt = TerrainTypeExtContainer::Instance.Find(pTerrain->Type);

	if (pExt->IsPassable)
		return true;

	if (pThis->WhatAmI() == UnitClass::AbsID)
	{
		if (pTerrain->Type->Crushable)
		{
			if (TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType())->CrushLevel.Get(pThis) > pExt->CrushableLevel)
			{
				return true;
			}
		}
	}

	return false;
}

void TerrainExtData::InitializeLightSource()
{
	if (!this->LighSource && this->AttachedToObject->Type)
	{
		auto const TypeData = TerrainTypeExtContainer::Instance.Find(this->AttachedToObject->Type);

		if (!TypeData->LightEnabled || !TypeData->LightIntensity.isset())
			return;

		auto const nVisibility = TypeData->LightVisibility.Get();

		if (!nVisibility)
			return;

		auto Tint = TypeData->GetLightTint();
		auto Coords = this->AttachedToObject->GetCoords();
		const auto light = GameCreate<LightSourceClass>(Coords, nVisibility, TypeData->GetLightIntensity(), Tint);
		light->Activate();
		this->LighSource.reset(light);
	}
}

void TerrainExtData::InitializeAnim()
{
	if (!AttachedAnim && this->AttachedToObject->Type)
	{
		auto const TypeData = TerrainTypeExtContainer::Instance.Find(this->AttachedToObject->Type);

		if (TypeData->AttachedAnim.empty())
			return;

		AnimTypeClass* pAnimType = nullptr;
		if (TypeData->AttachedAnim.size() == 1)
			pAnimType = TypeData->AttachedAnim[0];
		else
			pAnimType =
			TypeData->
			AttachedAnim[ScenarioClass::Instance->Random.RandomFromMax(TypeData->AttachedAnim.size() - 1)];

		if (pAnimType)
		{
			auto const Coords = this->AttachedAnim->GetCoords();

			AttachedAnim.reset(GameCreate<AnimClass>(pAnimType, Coords));
		}
	}
}

void TerrainExtData::Unlimbo(TerrainClass* pThis, CoordStruct* pCoord)
{
	if (!pThis || !pThis->Type)
		return;

	auto const TerrainExt = TerrainExtContainer::Instance.FindOrAllocate(pThis);

	//if (auto const CellExt = CellExt::ExtMap.Find<true>(Map[*pCoord]))
	//{
	//	auto const iter = std::find_if(CellExt->AttachedTerrain.begin(), CellExt->AttachedTerrain.end(),
	//		[&](auto const pCellTerrain) { return pCellTerrain == pThis; });

	//	if (iter != CellExt->AttachedTerrain.end())
	//		CellExt->AttachedTerrain.push_back(pThis);
	//}

	{
		TerrainExt->InitializeLightSource();
		TerrainExt->InitializeAnim();
	}

}

// =============================
// load / save
template <typename T>
void TerrainExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)
		.Process(this->LighSource, true)
		.Process(this->AttachedAnim, true)
		.Process(this->AttachedFireAnim, true)
		.Process(this->Adjencentcells)
		;
}

// =============================
// container
TerrainExtContainer TerrainExtContainer::Instance;

// container hooks
#include <Notifications.h>

DEFINE_JUMP(LJMP, 0x71BC31 , 0x71BC86);

DEFINE_HOOK(0x71BE74, TerrainClass_CTOR, 0x5)
{
	GET(TerrainClass*, pItem, ESI);
	TerrainExtContainer::Instance.Allocate(pItem);
	//PointerExpiredNotification::NotifyInvalidObject->Add(pItem);
	return 0;
}

DEFINE_HOOK(0x71BCA5, TerrainClass_CTOR_MoveAndAllocate, 0x5)
{
	GET(TerrainClass*, pItem, ESI);
	GET_STACK(CellStruct*, pCoord, 0x24);

	auto pExt = TerrainExtContainer::Instance.FindOrAllocate(pItem);

	if (pCoord->IsValid()) {
		//vtable may not instantiated
		if (!pItem->TerrainClass::Unlimbo(CellClass::Cell2Coord(*pCoord), static_cast<DirType>(0))) {
			pItem->ObjectClass::UnInit();
		}

		if(pItem->Type){
			GeneralUtils::AdjacentCellsInRange(pExt->Adjencentcells, (short)TerrainTypeExtContainer::Instance.Find(pItem->Type)->SpawnsTiberium_Range);
		}
	}

	return 0x0;
}

//Remove Ext later , dont do it to early otherwise some stuffs broke
DEFINE_HOOK(0x71B824, TerrainClass_DTOR, 0x5)
{
	GET(TerrainClass*, pItem, ESI);

	if(Unsorted::WTFMode() || pItem->Type)
	{
		pItem->IsAlive = true;
		if (!pItem->Limbo())
			pItem->AnnounceExpiredPointer();
	}

	if(auto pExt = TerrainExtContainer::Instance.TryFind(pItem)) {
		delete pExt;
		TerrainExtContainer::Instance.ClearExtAttribute(pItem);
		//PointerExpiredNotification::NotifyInvalidObject->Remove(pItem);
	}

	return 0x71B845;
}


#include <Misc/Hooks.Otamaa.h>

HRESULT __stdcall FakeTerrainClass::_Load(IStream* pStm)
{

	TerrainExtContainer::Instance.PrepareStream(this, pStm);
	HRESULT res = this->TerrainClass::Load(pStm);

	if (SUCCEEDED(res))
		TerrainExtContainer::Instance.LoadStatic();

	return res;
}

HRESULT __stdcall FakeTerrainClass::_Save(IStream* pStm, bool clearDirty)
{

	TerrainExtContainer::Instance.PrepareStream(this, pStm);
	HRESULT res = this->TerrainClass::Save(pStm, clearDirty);

	if (SUCCEEDED(res))
		TerrainExtContainer::Instance.SaveStatic();

	return res;
}

DEFINE_JUMP(VTABLE, 0x7F5240, MiscTools::to_DWORD(&FakeTerrainClass::_Load))
DEFINE_JUMP(VTABLE, 0x7F5244, MiscTools::to_DWORD(&FakeTerrainClass::_Save))

//DEFINE_HOOK(0x71CFD0, TerrainClass_Detach, 0x5)
//{
//	GET(TerrainClass*, pThis, ECX);
//	GET_STACK(AbstractClass*, pObj, 0x4);
//	GET_STACK(bool, bRemoved, 0x8);
//
//	pThis->ObjectClass::PointerExpired(pObj, bRemoved);
//	TerrainExtContainer::Instance.InvalidatePointerFor(pThis, pObj, bRemoved);
//
//	if (pThis->Type == pObj)
//		pThis->Type = nullptr;
//
//	return 0x71CFF7;
//}

void FakeTerrainClass::_Detach(AbstractClass* target, bool all)
{
	TerrainExtContainer::Instance.InvalidatePointerFor(this, target, all);
	this->TerrainClass::PointerExpired(target, all);
}
DEFINE_JUMP(VTABLE, 0x7F5254, MiscTools::to_DWORD(&FakeTerrainClass::_Detach));


void FakeTerrainClass::_AnimPointerExpired(AnimClass* pAnim) {

	auto pExt = this->_GetExtData();

	if (pExt->AttachedFireAnim.get() == pAnim) {
		pExt->AttachedFireAnim.release();
	}
}
DEFINE_JUMP(VTABLE ,0x7F528C, MiscTools::to_DWORD(&FakeTerrainClass::_AnimPointerExpired))