#include "Body.h"

#include <Ext/TerrainType/Body.h>
#include <Ext/Cell/Body.h>
#include <Ext/TechnoType/Body.h>

#include <Utilities/Macro.h>

TerrainExtData::~TerrainExtData()
{
	LighSource.SetDestroyCondition(!Phobos::Otamaa::ExeTerminated);
	AttachedAnim.SetDestroyCondition(!Phobos::Otamaa::ExeTerminated);
	AttachedFireAnim.SetDestroyCondition(!Phobos::Otamaa::ExeTerminated);
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
	if (!this->LighSource && This()->Type)
	{
		auto const TypeData = TerrainTypeExtContainer::Instance.Find(This()->Type);

		if (!TypeData->LightEnabled || !TypeData->LightIntensity.isset())
			return;

		auto const nVisibility = TypeData->LightVisibility.Get();

		if (!nVisibility)
			return;

		auto Tint = TypeData->GetLightTint();
		auto Coords = This()->GetCoords();
		const auto light = GameCreate<LightSourceClass>(Coords, nVisibility, TypeData->GetLightIntensity(), Tint);
		light->Activate();
		this->LighSource.reset(light);
	}
}

void TerrainExtData::InitializeAnim()
{
	if (!AttachedAnim && This()->Type)
	{
		auto const TypeData = TerrainTypeExtContainer::Instance.Find(This()->Type);

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
		.Process(this->LighSource, true)
		.Process(this->AttachedAnim, true)
		.Process(this->AttachedFireAnim, true)
		.Process(this->Adjencentcells)
		;
}

// =============================
// container
TerrainExtContainer TerrainExtContainer::Instance;
std::vector<TerrainExtData*> Container<TerrainExtData>::Array;

bool TerrainExtContainer::LoadGlobals(PhobosStreamReader& Stm)
{
	Clear();

	size_t Count = 0;
	if (!Stm.Load(Count))
		return false;

	Array.reserve(Count);

	for (size_t i = 0; i < Count; ++i)
	{

		void* oldPtr = nullptr;

		if (!Stm.Load(oldPtr))
			return false;

		auto newPtr = new TerrainExtData(nullptr, noinit_t());
		PHOBOS_SWIZZLE_REGISTER_POINTER((long)oldPtr, newPtr, "TerrainExtData")
		ExtensionSwizzleManager::RegisterExtensionPointer(oldPtr, newPtr);
		newPtr->LoadFromStream(Stm);
		Array.push_back(newPtr);
	}

	return true;
}

bool TerrainExtContainer::SaveGlobals(PhobosStreamWriter& Stm)
{
	Stm.Save(Array.size());

	for (auto& item : Array)
	{
		// write old pointer and name, then delegate
		Stm.Save(item);
		item->SaveToStream(Stm);
	}

	return true;
}

// container hooks
#include <Notifications.h>

DEFINE_JUMP(LJMP, 0x71BC31 , 0x71BC86);

ASMJIT_PATCH(0x71BE74, TerrainClass_CTOR, 0x5)
{
	GET(TerrainClass*, pItem, ESI);
	TerrainExtContainer::Instance.Allocate(pItem);
	//PointerExpiredNotification::NotifyInvalidObject->Add(pItem);
	return 0;
}

ASMJIT_PATCH(0x71BCA5, TerrainClass_CTOR_MoveAndAllocate, 0x5)
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
ASMJIT_PATCH(0x71B824, TerrainClass_DTOR, 0x5)
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

void FakeTerrainClass::_AI()
{
	this->ObjectClass::Update();
	if (this->Type->IsAnimated) {
		if (!this->Animation.Stage) {
			auto v2 =ScenarioClass::Instance->Random.Random();

			if ((double)((int)Math::abs(v2) % 1000000) * 0.000001 < this->Type->AnimationProbability) {
				this->Animation.Stage = 0;
				this->Animation.Start(this->Type->AnimationRate);
			}
		}
	}

	if (this->Animation.Timer.GetTimeLeft() || !this->Animation.Timer.Rate)
	{
		// timer is still running or hasn't been set yet.
		this->Animation.HasChanged = false;
	}
	else
	{
		// timer expired. move one step forward.
		this->Animation.Stage += this->Animation.Step;
		this->Animation.HasChanged = true;
		this->Animation.Timer.Restart();

		//auto const pTypeExt = this->_GetTypeExtData();

		//not sure what here ,..
	}
}

//ASMJIT_PATCH(0x71CFD0, TerrainClass_Detach, 0x5)
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
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5254, FakeTerrainClass::_Detach);


void FakeTerrainClass::_AnimPointerExpired(AnimClass* pAnim) {

	auto pExt = this->_GetExtData();

	if (pExt->AttachedFireAnim.get() == pAnim) {
		pExt->AttachedFireAnim.release();
	}
}
DEFINE_FUNCTION_JUMP(VTABLE ,0x7F528C, FakeTerrainClass::_AnimPointerExpired)
