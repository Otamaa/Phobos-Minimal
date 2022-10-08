#include "Body.h"
#include <Ext/TerrainType/Body.h>

TerrainExt::ExtContainer TerrainExt::ExtMap;

void TerrainExt::ExtData::InitializeConstants()
{

}

void TerrainExt::ExtData::InvalidatePointer(void* ptr, bool bRemoved)
{
	auto const abs = static_cast<AbstractClass*>(ptr)->WhatAmI();
	switch (abs)
	{
	case AbstractType::Anim:
	case AbstractType::LightSource:
	{
		AnnounceInvalidPointer(LighSource, ptr);
		AnnounceInvalidPointer(AttachedAnim, ptr);
	}
	break;
	default:
		return;
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

		if (auto light = GameCreate<LightSourceClass>(Coords, nVisibility, TypeData->GetLightIntensity(), Tint))
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
			pAnimType = TypeData->AttachedAnim.at(0);
		else
			pAnimType = TypeData->AttachedAnim.at(ScenarioGlobal->Random(0, TypeData->AttachedAnim.size() - 1));

		if (pAnimType)
		{
			auto const Coords = this->Get()->GetCoords();

			if (auto pAnim = GameCreate<AnimClass>(pAnimType, Coords))
			{
				//pAnim->SetOwnerObject(this->Get());
				AttachedAnim = pAnim;
			}
		}
	}
}

void TerrainExt::ExtData::ClearAnim()
{
	if (AttachedAnim)
	{
		CallDTOR<false>(AttachedAnim);
		AttachedAnim = nullptr;
	}
}

//called when it Dtor ed , for more optimal
void TerrainExt::ExtData::ClearLightSource()
{
	if (LighSource)
	{
		LighSource->Deactivate();
		CallDTOR<false>(LighSource);
		LighSource = nullptr;
	}
}

void TerrainExt::Unlimbo(TerrainClass* pThis)
{
	if (!pThis || !pThis->Type)
		return;

	auto const TerrainExt = TerrainExt::ExtMap.Find(pThis);

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
	Extension<TerrainClass>::Serialize(Stm);
	this->Serialize(Stm);
}

void TerrainExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<TerrainClass>::Serialize(Stm);
	this->Serialize(Stm);
}

void TerrainExt::ExtContainer::InvalidatePointer(void* ptr, bool bRemoved) { }

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
#ifndef ENABLE_NEWHOOKS
	TerrainExt::ExtMap.JustAllocate(pItem, pItem, "Trying To Allocate from nullptr !");
#else
	TerrainExt::ExtMap.FindOrAllocate(pItem);
#endif
	return 0;
}

DEFINE_HOOK(0x71BCA5, TerrainClass_CTOR_MoveAndAllocate, 0x5)
{
	GET(TerrainClass*, pItem, ESI);
	GET_STACK(CellStruct*, pCoord, 0x24);

	TerrainExt::ExtMap.JustAllocate(pItem, pItem, "Trying To Allocate from nullptr !");

	auto const nDefaultCell = CellStruct::Empty;

	if (pCoord->X != nDefaultCell.X || pCoord->Y != nDefaultCell.Y) {
		if (!pItem->Unlimbo(CellClass::Cell2Coord(*pCoord), static_cast<DirType>(0))) {
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
		pItem->Limbo();
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
DEFINE_JUMP(LJMP, 0x71CFA4, 0x71CFB2);