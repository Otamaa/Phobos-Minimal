#include "Body.h"

#include <TerrainTypeClass.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/Macro.h>

#include <TerrainClass.h>
#include <TacticalClass.h>
#include <ParticleTypeClass.h>

#include <Helpers/Macro.h>

#include <AnimClass.h>

void TerrainTypeExtData::Initialize()
{
	this->AttachedAnim.reserve(1);
	this->CrushableLevel = this->This()->Crushable ? 10 : 0;
}

int TerrainTypeExtData::GetTiberiumGrowthStage()
{
	return GeneralUtils::GetRangedRandomOrSingleValue(this->SpawnsTiberium_GrowthStage);
}

int TerrainTypeExtData::GetCellsPerAnim()
{
	return GeneralUtils::GetRangedRandomOrSingleValue(this->SpawnsTiberium_CellsPerAnim);
}

bool TerrainTypeExtData::LoadFromINI(CCINIClass* pINI, bool parseFailAddr)
{
	if (!this->ObjectTypeExtData::LoadFromINI(pINI, parseFailAddr))
		return false;

	auto pThis = this->This();
	const char* pSection = pThis->ID;

	if (parseFailAddr)
		return false;

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

	this->CrushableLevel.Read(exINI, pSection, "CrushableLevel");

#pragma region Otamaa
	this->LightEnabled.Read(exINI, pSection, "LightEnabled");
	this->LightVisibility.Read(exINI, pSection, GameStrings::LightVisibility());
	this->LightIntensity.Read(exINI, pSection, GameStrings::LightIntensity());
	this->LightRedTint.Read(exINI, pSection, GameStrings::LightRedTint());
	this->LightGreenTint.Read(exINI, pSection, GameStrings::LightGreenTint());
	this->LightBlueTint.Read(exINI, pSection, GameStrings::LightBlueTint());

	this->AttachedAnim.Read(exINI, pSection, "AttachedAnims",true);
	this->Warhead.Read(exINI, pSection, "SpawnsTiberium.ExplodeWarhead");
	this->Damage.Read(exINI, pSection, "SpawnsTiberium.ExplodeDamage");
	this->AreaDamage.Read(exINI, pSection, "SpawnsTiberium.ExplodeDealAreaDamage");

	this->CustomPalette.Read(exINI, pSection, "CustomPalette");
#pragma endregion

	this->Bounty.Read(exINI, pSection, "Bounty");
	this->HasDamagedFrames.Read(exINI, pSection, "HasDamagedFrames");
	this->HasCrumblingFrames.Read(exINI, pSection, "HasCrumblingFrames");
	this->CrumblingSound.Read(exINI, pSection, "CrumblingSound");
	this->AnimationLength.Read(exINI, pSection, "AnimationLength");
	this->TreeFires.Read(exINI, pSection, "TreeFire");
	this->SpawnsTiberium_Particle.Read(exINI, pSection, "SpawnsTiberium.Particle");

	return true;
}

void TerrainTypeExtData::PlayDestroyEffects(CoordStruct coords)
{
	VocClass::SafeImmedietelyPlayAt(this->DestroySound, &coords);

	if (auto pAnimType = this->DestroyAnim)
		GameCreate<AnimClass>(pAnimType.Get(), coords);
}

// =============================
// load / save

template <typename T>
void TerrainTypeExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->CustomPalette)
		.Process(this->SpawnsTiberium_Type)
		.Process(this->SpawnsTiberium_Range)
		.Process(this->SpawnsTiberium_GrowthStage)
		.Process(this->SpawnsTiberium_CellsPerAnim)
		.Process(this->DestroyAnim)
		.Process(this->DestroySound)

		.Process(this->MinimapColor)
		.Process(this->IsPassable)
		.Process(this->CanBeBuiltOn)
		.Process(this->CrushableLevel)

		.Process(this->LightEnabled)
		.Process(this->LightVisibility)
		.Process(this->LightIntensity)
		.Process(this->LightRedTint)
		.Process(this->LightGreenTint)
		.Process(this->LightBlueTint)
		.Process(this->AttachedAnim)
		.Process(this->Warhead)
		.Process(this->Damage)
		.Process(this->AreaDamage)
		.Process(this->Bounty)

		.Process(this->HasDamagedFrames)
		.Process(this->HasCrumblingFrames)
		.Process(this->CrumblingSound)
		.Process(this->AnimationLength)

		.Process(this->TreeFires)
		.Process(this->SpawnsTiberium_Particle)
		;

}

void TerrainTypeExtData::Remove(TerrainClass* pTerrain)
{
	if (!pTerrain)
		return;

	RectangleStruct rect {};
	pTerrain->GetRenderDimensions(&rect);
	TacticalClass::Instance->RegisterDirtyArea(rect, false);
	pTerrain->Disappear(true);
	pTerrain->UnInit();
}

// =============================
// container
TerrainTypeExtContainer TerrainTypeExtContainer::Instance;
std::vector<TerrainTypeExtData*> Container<TerrainTypeExtData>::Array;

// =============================
// container hooks

ASMJIT_PATCH(0x71DBC0, TerrainTypeClass_CTOR, 0x7)
{
	GET(TerrainTypeClass*, pItem, ESI);
	TerrainTypeExtContainer::Instance.Allocate(pItem);
	pItem->RadarInvisible = false;
	return 0;
}

ASMJIT_PATCH(0x71E3A5, TerrainTypeClass_SDDTOR, 0x6)
{
	GET(TerrainTypeClass*, pItem, ESI);

	TerrainTypeExtContainer::Instance.Remove(pItem);

	return 0;
}

bool FakeTerrainTypeClass::_ReadFromINI(CCINIClass* pINI)
{
	bool status = this->TerrainTypeClass::LoadFromINI(pINI);
	TerrainTypeExtContainer::Instance.LoadFromINI(this, pINI, !status);
	return status;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F54BC, FakeTerrainTypeClass::_ReadFromINI)

HRESULT __stdcall FakeTerrainTypeClass::_Load(IStream* pStm)
{
	auto hr = this->TerrainTypeClass::Load(pStm);

	if (SUCCEEDED(hr))
	{
		hr = TerrainTypeExtContainer::Instance.ReadDataFromTheByteStream(this,
			TerrainTypeExtContainer::Instance.AllocateNoInit(this), pStm);
	}

	return hr;
}

HRESULT __stdcall FakeTerrainTypeClass::_Save(IStream* pStm, BOOL clearDirty)
{
	auto hr = this->TerrainTypeClass::Save(pStm, clearDirty);

	if (SUCCEEDED(hr))
	{
		hr = TerrainTypeExtContainer::Instance.WriteDataToTheByteStream(this, pStm);
	}

	return hr;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F546C, FakeTerrainTypeClass::_Load)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5470, FakeTerrainTypeClass::_Save)