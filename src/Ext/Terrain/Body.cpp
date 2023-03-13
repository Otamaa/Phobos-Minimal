#include "Body.h"

#include <Ext/TerrainType/Body.h>
#include <Ext/Cell/Body.h>

TerrainExt::ExtContainer TerrainExt::ExtMap;

void TerrainExt::ExtData::InitializeConstants()
{

}

void TerrainExt::ExtData::InvalidatePointer(void* ptr, bool bRemoved)
{
	if (this->InvalidateIgnorable(ptr))
		return;

	if (this->LighSource == ptr) {
		this->LighSource = nullptr;
	}

	if (this->AttachedAnim == ptr) {
		this->AttachedAnim = nullptr;
	}
}

void TerrainExt::ExtData::InitializeLightSource()
{
	if (!this->LighSource && this->Get()->Type)
	{
		auto const TypeData = TerrainTypeExt::ExtMap.Find(this->Get()->Type);

		if (!TypeData->LightIntensity.isset())
			return;


		auto const nVisibility = TypeData->LightVisibility.Get();

		if (!nVisibility)
			return;

		auto Tint = TypeData->GetLightTint();
		auto Coords = this->Get()->GetCoords();

		if (const auto light = GameCreate<LightSourceClass>(Coords, nVisibility, TypeData->GetLightIntensity(), Tint))
		{
			this->LighSource = light;
			this->LighSource->Activate();
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

			if (const auto pAnim = GameCreate<AnimClass>(pAnimType, Coords)) {
				AttachedAnim = pAnim;
			}
		}
	}
}

void TerrainExt::ExtData::ClearAnim()
{
	if (AttachedAnim) {

		if (AttachedAnim->Type)
		{
			AttachedAnim->TimeToDie = true;
			AttachedAnim->UnInit();
		}

		AttachedAnim = nullptr;;
	}
}

//called when it Dtor ed , for more optimal
void TerrainExt::ExtData::ClearLightSource()
{
	if (LighSource)
	{
		GameDelete<true, false>(LighSource);
		LighSource = nullptr;
	}
}

void TerrainExt::Unlimbo(TerrainClass* pThis, CoordStruct* pCoord)
{
	if (!pThis || !pThis->Type)
		return;

	auto const TerrainExt = TerrainExt::ExtMap.Find(pThis);

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
	if (!pThis || !pThis->Type)
		return;

	auto const TerrainExt = TerrainExt::ExtMap.Find(pThis);

	{
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
		.Process(this->LighSource)
		.Process(this->AttachedAnim)
		;
}

void TerrainExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<TerrainClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void TerrainExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<TerrainClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool TerrainExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool TerrainExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

TerrainExt::ExtContainer::ExtContainer() : Container("TerrainClass") { }
TerrainExt::ExtContainer::~ExtContainer() = default;

// container hooks

DEFINE_JUMP(LJMP, 0x71BC31, 0x71BC86);

DEFINE_HOOK(0x71BE74, TerrainClass_CTOR, 0x5)
{
	GET(TerrainClass*, pItem, ESI);
	TerrainExt::ExtMap.Allocate(pItem);
	return 0;
}

DEFINE_HOOK(0x71BCA5, TerrainClass_CTOR_MoveAndAllocate, 0x5)
{
	GET(TerrainClass*, pItem, ESI);
	GET_STACK(CellStruct*, pCoord, 0x24);

	TerrainExt::ExtMap.FindOrAllocate(pItem);

	if (*pCoord != CellStruct::Empty) {
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

	TerrainExt::ExtMap.Remove(pItem);

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

DEFINE_HOOK(0x71CFE3, TerrainClass_Detach, 0x6)
{
	GET(TerrainClass*, pThis, ESI);
	GET(void*, pObj, EDI);
	GET_STACK(bool, bRemoved, STACK_OFFS(0x8, -0x8));

	if (auto pExt = TerrainExt::ExtMap.Find(pThis))
		pExt->InvalidatePointer(pObj, bRemoved);

	return pThis->Type == pObj ? 0x71CFEB : 0x71CFF5;
}

// Skip D0 CRC here
//DEFINE_JUMP(LJMP, 0x71CFA4, 0x71CFB2);