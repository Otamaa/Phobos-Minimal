#include "Body.h"

#include <TerrainTypeClass.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/Macro.h>

#include <TerrainClass.h>
#include <TacticalClass.h>
#include <ParticleTypeClass.h>
#include <VoxClass.h>
#include <VocClass.h>
#include <Helpers/Macro.h>

#include <AnimClass.h>

#include <Ext/Rules/Body.h>


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
	this->SpawnsTiberium_StageFalloff.Read(exINI, pSection, "SpawnsTiberium.StageFalloff");

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

	{
		INI_EX exArtINI(CCINIClass::INI_Art());
		const char* pArtSection = pThis->ImageFile;

		if (!exArtINI.ReadString(pArtSection, "Foundation") || _stricmp(exArtINI.value(), "Custom") != 0) {
			this->CustomFoundation.clear();
		}
		else
		{
			// Never use the engine's numeric foundation index for a custom foundation.
			// The engine uses that index to restore its stock data on load and on INI rereads.
			this->CustomFoundation.clear();
			int width = 0;
			int height = 0;
			bool canCustomFoundation = true;

			if (!exArtINI.ReadInteger(pArtSection, "Foundation.X", &width) || width < 1 || width > MapSide) {
				Debug::INIParseFailed(pArtSection, "Foundation.X", exArtINI.value(), "Expected a width between 1 and 512");
				canCustomFoundation = false;
			} else if(!exArtINI.ReadInteger(pArtSection, "Foundation.Y", &height) || height < 1 || height > MapSide) {
				Debug::INIParseFailed(pArtSection, "Foundation.Y", exArtINI.value(), "Expected a height between 1 and 512");
				canCustomFoundation = false;
			}

			if (canCustomFoundation)
			{
				bool parserError = false;
				// Stored in the executable's .bss segment (zero heap allocation, zero runtime init)
				// Note: +1 gives room for FoundationEnd if all 512x512 cells are filled
				static std::array<CellStruct, (MapSide* MapSide) + 1> cells;
				int count = 0;

				for (int i = 0; i < width * height; ++i)
				{
					char key[32];
					_snprintf_s(key, sizeof(key), "Foundation.%d", i);

					if (!exArtINI.ReadString(pArtSection, key)) {
						parserError = true;
						break;
					}

					int offset[2] { };
					char trailing = 0;

					if (sscanf_s(exArtINI.value(), " %d , %d %c", &offset[0], &offset[1], &trailing, 1u) != 2
						|| offset[0] < 0 || offset[0] >= width || offset[1] < 0 || offset[1] >= height)
					{
						Debug::INIParseFailed(pArtSection, key, exArtINI.value(), "Expected an X,Y offset inside the foundation dimensions");
						parserError = true;
						break;
					}

					cells[count++] = CellStruct { static_cast<short>(offset[0]), static_cast<short>(offset[1]) };
				}

				if(!parserError) {
					auto endIt = cells.begin() + count;

					std::ranges::sort(cells.begin(), endIt, [](const CellStruct& lhs, const CellStruct& rhs) {
						return lhs.Y != rhs.Y ? lhs.Y < rhs.Y : lhs.X < rhs.X;
					});

					if (cells.empty() || cells.front() != CellStruct::Empty) {
						Debug::INIParseFailed(pArtSection, "Foundation", "Custom",
							"The footprint must include (0,0) because terrain placement and removal update the anchor cell. Falling back to 1x1.");
					
					} else {
						endIt = std::unique(cells.begin(), endIt);
						*endIt++ = FoundationEnd;
						this->CustomFoundation.assign(cells.begin(), endIt);
						pThis->FoundationData = this->CustomFoundation.data();
					}
				}
			}
		}
	}

	return true;
}

TerrainClass* TerrainTypeExtData::FindCustomFoundationTerrain(CellClass* pCell, bool onBridge)
{
	// Unlike GetTerrain, this also sees objects while the scenario is loading.
	for (auto pObject = onBridge ? pCell->AltObject : pCell->FirstObject; pObject; pObject = pObject->NextObject) {
		if (auto const pTerrain = cast_to<TerrainClass*, true>(pObject)) {
			if (TerrainTypeExtContainer::Instance.Find(pTerrain->Type)->HasCustomFoundation())
				return pTerrain;
		}
	}

	return nullptr;
}

void TerrainTypeExtData::PlayDestroyEffects(CoordStruct coords)
{
	VocClass::SafeImmedietelyPlayAt(this->DestroySound, &coords);

	if (auto pAnimType = this->DestroyAnim)
		GameCreate<AnimClass>(pAnimType.Get(), coords);
}

bool TerrainTypeExtData::IsThisPassable()
{
	return this->IsPassable.Get(this->This()->SpawnsTiberium ? FakeRulesClass::Instance->Tibtree_IsPassable : FakeRulesClass::Instance->Terrain_IsPassable);
}

bool TerrainTypeExtData::IsThisCanBeBuiltOn()
{
	return this->CanBeBuiltOn.Get(this->This()->SpawnsTiberium ? FakeRulesClass::Instance->Tibtree_CanBeBuiltOn : FakeRulesClass::Instance->Terrain_CanBeBuiltOn);
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
		.Process(this->SpawnsTiberium_StageFalloff)
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
		.Process(this->CustomFoundation)
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
//=============================

void TerrainTypeExtContainer::LoadFromINI(ext_t::base_type* key, CCINIClass* pINI, bool parseFailAddr)
{
	if (auto ptr = this->Find(key))
	{
		if (!pINI)
		{
			return;
		}

		//load anywhere other than rules
		ptr->LoadFromINI(pINI, parseFailAddr);
		//this function can be called again multiple time but without need to re-init the data
		ptr->SetInitState(InitState::Ruled);
	}

}

void TerrainTypeExtContainer::WriteToINI(ext_t::base_type* key, CCINIClass* pINI)
{

	if (auto ptr = this->TryFind(key))
	{
		if (!pINI)
		{
			return;
		}

		ptr->WriteToINI(pINI);
	}
}
// =============================
// container hooks

ASMJIT_PATCH(0x71DBC0, TerrainTypeClass_CTOR, 0x7)
{
	GET(TerrainTypeClass*, pItem, ESI);
	if (!Phobos::Otamaa::DoingLoadGame)
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

HRESULT __stdcall FakeTerrainTypeClass::__Load(IStream* pStm)
{
	HRESULT hr = this->TerrainTypeClass::Load(pStm);

	if (SUCCEEDED(hr)) {
		if (!TerrainTypeExtContainer::Instance.LoadByKey(this, pStm))
			return PHOBOS_E_EXTDATA_LOAD_FAILED;

		auto pTypeExt = TerrainTypeExtContainer::Instance.Find(this);

		// The footprint lives in the extension, so the owner must point at the loaded storage.
		if(pTypeExt->HasCustomFoundation())
			this->FoundationData = pTypeExt->CustomFoundation.data();
	}

	return hr;
}
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F546C, FakeTerrainTypeClass::__Load)

HRESULT __stdcall FakeTerrainTypeClass::__Save(IStream* pStm, BOOL fClearDirty)
{
	HRESULT hr = this->TerrainTypeClass::Save(pStm, fClearDirty);

	if (SUCCEEDED(hr)) {
		if (!TerrainTypeExtContainer::Instance.SaveByKey(this, pStm))
			return PHOBOS_E_EXTDATA_SAVE_FAILED;
	}

	return hr;
}
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5470, FakeTerrainTypeClass::__Save)