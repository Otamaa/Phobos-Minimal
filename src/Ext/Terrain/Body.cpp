#include "Body.h"

#include <Ext/TerrainType/Body.h>
#include <Ext/Cell/Body.h>

void TerrainExt::ExtData::InvalidatePointer(AbstractClass* ptr, bool bRemoved)
{
	if (this->LighSource.get() == ptr) {
		this->LighSource.release();
	}

	if (this->AttachedAnim == ptr) {
		this->AttachedAnim.release();
	}
}

void TerrainExt::ExtData::InitializeLightSource()
{
	if (!this->LighSource && this->Get()->Type)
	{
		auto const TypeData = TerrainTypeExt::ExtMap.Find(this->Get()->Type);

		if (!TypeData->LightEnabled || !TypeData->LightIntensity.isset())
			return;

		auto const nVisibility = TypeData->LightVisibility.Get();

		if (!nVisibility)
			return;

		auto Tint = TypeData->GetLightTint();
		auto Coords = this->Get()->GetCoords();

		if (const auto light = GameCreate<LightSourceClass>(Coords, nVisibility, TypeData->GetLightIntensity(), Tint))
		{
			light->Activate();
			this->LighSource.reset(light);
		}
	}
}

void TerrainExt::ExtData::InitializeAnim()
{
	if (!AttachedAnim && this->Get()->Type)
	{
		auto const TypeData = TerrainTypeExt::ExtMap.Find(this->Get()->Type);

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
			auto const Coords = this->Get()->GetCoords();

			AttachedAnim.reset(GameCreate<AnimClass>(pAnimType, Coords));
		}
	}
}

void TerrainExt::ExtData::ClearAnim()
{
	AttachedAnim.reset(nullptr);
}

//called when it Dtor ed , for more optimal
void TerrainExt::ExtData::ClearLightSource()
{
	LighSource.reset(nullptr);
}

void TerrainExt::Unlimbo(TerrainClass* pThis, CoordStruct* pCoord)
{
	if (!pThis || !pThis->Type)
		return;

	auto const TerrainExt = TerrainExt::ExtMap.FindOrAllocate(pThis);

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

void TerrainExt::CleanUp(TerrainClass* pThis)
{
	if (!pThis)
		return;

	if(auto const TerrainExt = TerrainExt::ExtMap.Find(pThis)) {
		TerrainExt->ClearLightSource();
		TerrainExt->ClearAnim();
	}
}

// =============================
// load / save
template <typename T>
void TerrainExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)
		.Process(this->LighSource)
		.Process(this->AttachedAnim)
		;
}

// =============================
// container
TerrainExt::ExtContainer TerrainExt::ExtMap;

TerrainExt::ExtContainer::ExtContainer() : Container("TerrainClass") { }
TerrainExt::ExtContainer::~ExtContainer() = default;

// container hooks
#include <Notifications.h>

//DEFINE_SKIP_HOOK(0x71BC31 , TerrainClass_CTOR_RemoveUnlimboFunc , 0xA , 71BC86);
DEFINE_JUMP(LJMP, 0x71BC31 , 0x71BC86);

DEFINE_HOOK(0x71BE74, TerrainClass_CTOR, 0x5)
{
	GET(TerrainClass*, pItem, ESI);
	TerrainExt::ExtMap.Allocate(pItem);
	//PointerExpiredNotification::NotifyInvalidObject->Add(pItem);
	return 0;
}

DEFINE_HOOK(0x71BCA5, TerrainClass_CTOR_MoveAndAllocate, 0x5)
{
	GET(TerrainClass*, pItem, ESI);
	GET_STACK(CellStruct*, pCoord, 0x24);

	TerrainExt::ExtMap.FindOrAllocate(pItem);

	if (pCoord->IsValid()) {
		//vtable may not instantiated
		if (!pItem->TerrainClass::Unlimbo(CellClass::Cell2Coord(*pCoord), static_cast<DirType>(0))) {
			pItem->ObjectClass::UnInit();
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

	if(auto pExt = TerrainExt::ExtMap.TryFind(pItem)) {
		delete pExt;
		TerrainExt::ExtMap.ClearExtAttribute(pItem);
		//PointerExpiredNotification::NotifyInvalidObject->Remove(pItem);
	}

	return 0x71B845;
}

DEFINE_HOOK_AGAIN(0x71CDA0, TerrainClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x71CF30, TerrainClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(TerrainClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	TerrainExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x71CEAC, TerrainClass_Load_Suffix, 0x9)
{
	TerrainExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x71CF44, TerrainClass_Save_Suffix, 0x5)
{
	TerrainExt::ExtMap.SaveStatic();
	return 0;
}

//DEFINE_HOOK(0x71CFD0, TerrainClass_Detach, 0x5)
//{
//	GET(TerrainClass*, pThis, ECX);
//	GET_STACK(AbstractClass*, pObj, 0x4);
//	GET_STACK(bool, bRemoved, 0x8);
//
//	pThis->ObjectClass::PointerExpired(pObj, bRemoved);
//	TerrainExt::ExtMap.InvalidatePointerFor(pThis, pObj, bRemoved);
//
//	if (pThis->Type == pObj)
//		pThis->Type = nullptr;
//
//	return 0x71CFF7;
//}

void __fastcall TerrainClass_Detach_Wrapper(TerrainClass* pThis, DWORD, AbstractClass* target, bool all)
{
	TerrainExt::ExtMap.InvalidatePointerFor(pThis, target, all);
	pThis->TerrainClass::PointerExpired(target, all);
}
DEFINE_JUMP(VTABLE, 0x7F5254, GET_OFFSET(TerrainClass_Detach_Wrapper));
