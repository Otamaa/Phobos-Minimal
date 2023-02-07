#include "Body.h"

#include <TerrainTypeClass.h>
#include <Utilities/GeneralUtils.h>

TerrainTypeExt::ExtContainer TerrainTypeExt::ExtMap;

int TerrainTypeExt::ExtData::GetTiberiumGrowthStage()
{
	return GeneralUtils::GetRangedRandomOrSingleValue(this->SpawnsTiberium_GrowthStage.Get());
}

int TerrainTypeExt::ExtData::GetCellsPerAnim()
{
	return GeneralUtils::GetRangedRandomOrSingleValue(this->SpawnsTiberium_CellsPerAnim.Get());
}

void TerrainTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->Get();
	const char* pSection = pThis->ID;

	if (!pINI->GetSection(pSection))
		return;

	INI_EX exINI(pINI);
	this->SpawnsTiberium_Type.Read(exINI, pSection, "SpawnsTiberium.Type");
	this->SpawnsTiberium_Range.Read(exINI, pSection, "SpawnsTiberium.Range");
	this->SpawnsTiberium_GrowthStage.Read(exINI, pSection, "SpawnsTiberium.GrowthStage");
	this->SpawnsTiberium_CellsPerAnim.Read(exINI, pSection, "SpawnsTiberium.CellsPerAnim");

	this->DestroyAnim.Read(exINI, pSection, GameStrings::DestroyAnim());
	this->DestroySound.Read(exINI, pSection, "DestroySound");

	this->MinimapColor.Read(exINI, pSection, "MinimapColor");

	this->IsPassable.Read(exINI, pSection, "IsPassable");
	this->CanBeBuiltOn.Read(exINI, pSection, "CanBeBuiltOn");

#pragma region Otamaa
	this->LightVisibility.Read(exINI, pSection, GameStrings::LightVisibility());
	this->LightIntensity.Read(exINI, pSection, GameStrings::LightIntensity());
	this->LightRedTint.Read(exINI, pSection, GameStrings::LightRedTint());
	this->LightGreenTint.Read(exINI, pSection, GameStrings::LightGreenTint());
	this->LightBlueTint.Read(exINI, pSection, GameStrings::LightBlueTint());

	this->AttachedAnim.Read(exINI, pSection, "AttachedAnims",true);
	this->Warhead.Read(exINI, pSection, "SpawnsTiberium.ExplodeWarhead");
	this->Damage.Read(exINI, pSection, "SpawnsTiberium.ExplodeDamage");
	this->AreaDamage.Read(exINI, pSection, "SpawnsTiberium.ExplodeDealAreaDamage");

	this->CustomPalette.Read(exINI.GetINI(), pSection, "CustomPalette");
#pragma endregion
}

// =============================
// load / save

template <typename T>
void TerrainTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->CustomPalette)
		.Process(this->SpawnsTiberium_Type)
		.Process(this->SpawnsTiberium_Range)
		.Process(this->SpawnsTiberium_GrowthStage)
		.Process(this->SpawnsTiberium_CellsPerAnim)
		.Process(this->DestroyAnim)
		.Process(this->DestroySound)

		.Process(this->LightVisibility)
		.Process(this->LightIntensity)
		.Process(this->LightRedTint)
		.Process(this->LightGreenTint)
		.Process(this->LightBlueTint)
		.Process(this->AttachedAnim)
		;

}

void TerrainTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	TExtension<TerrainTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void TerrainTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	TExtension<TerrainTypeClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool TerrainTypeExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool TerrainTypeExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

void TerrainTypeExt::Remove(TerrainClass* pTerrain)
{
	if (!pTerrain)
		return;

	RectangleStruct rect {};
	rect = *pTerrain->GetRenderDimensions(&rect);
	TacticalClass::Instance->RegisterDirtyArea(rect, false);
	pTerrain->Disappear(true);
	pTerrain->UnInit();
}

// =============================
// container

TerrainTypeExt::ExtContainer::ExtContainer() : TExtensionContainer("TerrainTypeClass") { }
TerrainTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x71DBC0, TerrainTypeClass_CTOR, 0x7)
{
	GET(TerrainTypeClass*, pItem, ESI);
#ifndef ENABLE_NEWHOOKS
	TerrainTypeExt::ExtMap.JustAllocate(pItem, pItem, "Trying To Allocate from nullptr !");
#else
	TerrainTypeExt::ExtMap.FindOrAllocate(pItem);
#endif
	pItem->RadarInvisible = false;
	return 0;
}

DEFINE_HOOK(0x71E3B5, TerrainTypeClass_SDDTOR, 0x7)
{
	GET(TerrainTypeClass*, pItem, ESI);

	TerrainTypeExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x71E1D0, TerrainTypeClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x71E240, TerrainTypeClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(TerrainTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	TerrainTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x71E233, TerrainTypeClass_Load_Suffix, 0x6)
{
	TerrainTypeExt::ExtMap.LoadStatic();

	return 0;
}

DEFINE_HOOK(0x71E258, TerrainTypeClass_Save_Suffix, 0x5)
{
	TerrainTypeExt::ExtMap.SaveStatic();

	return 0;
}

DEFINE_HOOK(0x71E0B4, TerrainTypeClass_LoadFromINI_ReturnFalse, 0xA)
{
	GET(TerrainTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, STACK_OFFS(0x20C, -0x4));

	TerrainTypeExt::ExtMap.LoadFromINI(pItem, pINI);

	return 0;
}

DEFINE_HOOK(0x71E0A6, TerrainTypeClass_LoadFromINI, 0x5)
{
	GET(TerrainTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, STACK_OFFS(0x210, -0x4));

	TerrainTypeExt::ExtMap.LoadFromINI(pItem, pINI);

	return 0;
}

