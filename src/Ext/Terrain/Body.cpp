#include "Body.h"
#include <Ext/TerrainType/Body.h>

TerrainExt::ExtContainer TerrainExt::ExtMap;

void TerrainExt::ExtData::InitializeConstants() {

}

void TerrainExt::ExtData::InvalidatePointer(void *ptr, bool bRemoved)
{
	if(LighSource)
		AnnounceInvalidPointer(LighSource,ptr);

	if (AttachedAnim.get() && (void*)AttachedAnim.get() == ptr)
		AttachedAnim.release();
}


void TerrainExt::ExtData::InitializeLightSource()
{
	auto const TypeData = TerrainTypeExt::ExtMap.Find(this->Get()->Type);

	if (!TypeData->LightIntensity.isset())
		return;

	if (!this->LighSource)
	{
		auto const nVisibility = TypeData->LightVisibility.Get();

		if (!nVisibility)
			return;

		auto Tint = TypeData->GetLightTint();
		auto Coords = this->Get()->GetCoords();

		if (auto light = GameCreate<LightSourceClass>(Coords, nVisibility, TypeData->GetLightIntensity(), Tint))
		{
			this->LighSource = std::move(light);
			this->LighSource->Activate();
		}
	}
}

void TerrainExt::ExtData::InitializeAnim()
{
	if (!AttachedAnim.get())
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
				pAnim->SetOwnerObject(this->Get());
				AttachedAnim.reset(pAnim);
			}
		}
	}
}

void TerrainExt::ExtData::ClearAnim()
{
	if (auto const pAnim = AttachedAnim.get())
	{
		pAnim->RemainingIterations = 0;
		pAnim->UnInit();
		AttachedAnim.release();
	}
}

//called when it Dtor ed , for more optimal
void TerrainExt::ExtData::ClearLightSource()
{
	if (auto const pLight = this->LighSource)
	{
		pLight->Deactivate();
		GameDelete(LighSource);
	}
}

void TerrainExt::Unlimbo(TerrainClass* pThis)
{
	if (!pThis)
		return;

	if (auto const TerrainExt = TerrainExt::ExtMap.Find(pThis))
	{
		TerrainExt->InitializeLightSource();
		TerrainExt->InitializeAnim();
	}

}

void TerrainExt::CleanUp(TerrainClass* pThis)
{
	if (!pThis)
		return;

	if (auto const TerrainExt = TerrainExt::ExtMap.Find(pThis))
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
DEFINE_HOOK(0x71BBF8, TerrainClass_CTOR, 0xD)
//DEFINE_HOOK(0x71BE6D, TerrainClass_CTOR, 0xC)
{
	GET(TerrainClass*, pItem, ESI);
#ifdef ENABLE_NEWHOOKS
	TerrainExt::ExtMap.JustAllocate(pItem, pItem, "Trying To Allocate from nullptr !");
#else
	TerrainExt::ExtMap.FindOrAllocate(pItem);
#endif
	return 0;
}

DEFINE_HOOK(0x71B7C9, TerrainClass_DTOR, 0xD)
{
	GET(TerrainClass*, pItem, ESI);

	TerrainExt::ExtMap.Remove(pItem);

	return 0;
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