#include "Body.h"

#include <ScenarioClass.h>
//#include <Ext/Convert/Body.h>

int IsometricTileTypeExtContainer::CurrentTileset = -1;
std::map<std::string, std::map<TintStruct, LightConvertClass*>> IsometricTileTypeExtContainer::LightConvertEntities;

LightConvertClass* IsometricTileTypeExtData::GetLightConvert(IsometricTileTypeClass* pOvrl, int r, int g, int b)
{
	if (!DSurface::Primary())
		return nullptr;

	const char* ConverName = "ISO_X.PAL";
	BytePalette* pISOPal = &FileSystem::ISOx_PAL;

	if(pOvrl) {
		auto pExt = IsometricTileTypeExtContainer::Instance.Find(pOvrl);
		if(pExt->Palette) {
			ConverName = pExt->Palette->Name.c_str();
			pISOPal = pExt->Palette->Palette.get();
		}
	}

	auto& entities = IsometricTileTypeExtContainer::LightConvertEntities[ConverName];

	ScenarioClass::Instance->ScenarioLighting(&r, &g, &b);
	TintStruct tint(r, g, b);
	auto Iter = entities.find(tint);

	if (Iter != entities.end())
	{
		if (!Iter->second)
		{
			LightConvertClass* pLightConvert = GameCreate<LightConvertClass>
				(
					pISOPal,
					&FileSystem::TEMPERAT_PAL,
					DSurface::Primary,
					r,
					g,
					b,
					false,
					nullptr,
					(r + g + b < 2000 ? 27 : 53)
				);

			LightConvertClass::Array->AddItem(pLightConvert);
			Iter->second = pLightConvert;
		}

		return Iter->second;
	}
	else
	{
		LightConvertClass* pLightConvert = GameCreate<LightConvertClass>
			(
				pISOPal,
				&FileSystem::TEMPERAT_PAL,
				DSurface::Primary,
				r,
				g,
				b,
				false,
				nullptr,
				(r + g + b < 2000 ? 27 : 53)
			);

		LightConvertClass::Array->AddItem(pLightConvert);
		entities[tint] = pLightConvert;
		return pLightConvert;
	}
}

// =============================
// load / save

void IsometricTileTypeExtData::LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr)
{
	this->Tileset = IsometricTileTypeExtContainer::CurrentTileset;

	INI_EX exINI(pINI);

	this->Palette.Read(exINI, std::format("Tileset{:04}" ,IsometricTileTypeExtContainer::CurrentTileset).c_str(), "CustomPalette");
}

template <typename T>
void IsometricTileTypeExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)
		.Process(this->Tileset)
		.Process(this->Palette)
		;
}

// =============================
// container

IsometricTileTypeExtContainer IsometricTileTypeExtContainer::Instance;

// =============================
// container hooks

DEFINE_HOOK(0x5449F2, IsometricTileTypeClass_CTOR, 0x5)
{
	GET(IsometricTileTypeClass*, pItem, EBP);

	IsometricTileTypeExtContainer::Instance.Allocate(pItem);

	return 0;
}

DEFINE_HOOK(0x544BC2, IsometricTileTypeClass_DTOR, 0x8)
{
	GET(IsometricTileTypeClass*, pItem, ESI);

	IsometricTileTypeExtContainer::Instance.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x549D70, IsometricTileTypeClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x549C80, IsometricTileTypeClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(IsometricTileTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	IsometricTileTypeExtContainer::Instance.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x549D5D, IsometricTileTypeClass_Load_Suffix, 0x5)
{
	IsometricTileTypeExtContainer::Instance.LoadStatic();

	return 0;
}

DEFINE_HOOK(0x549D8A, IsometricTileTypeClass_Save_Suffix, 0x6)
{
	IsometricTileTypeExtContainer::Instance.SaveStatic();

	return 0;
}

DEFINE_HOOK(0x54642E, IsometricTileTypeClass_LoadFromINI, 0x6)
{
	GET(IsometricTileTypeClass*, pItem, EBP);
	LEA_STACK(CCINIClass*, pINI, STACK_OFFS(0xA10, 0x9D8));

	IsometricTileTypeExtContainer::Instance.LoadFromINI(pItem, pINI, false);
	return 0;
}

DEFINE_HOOK(0x545FA3, IsometricTileTypeClass_LoadFromINI_SetTileSet, 0x8)
{
	IsometricTileTypeExtContainer::CurrentTileset = R->EDI();

	return 0;
}