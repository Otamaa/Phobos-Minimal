#include "Body.h"

#include <ScenarioClass.h>
//#include <Ext/Convert/Body.h>

int IsometricTileTypeExtContainer::CurrentTileset = -1;
std::map<std::string, std::map<TintStruct, LightConvertClass*>> IsometricTileTypeExtContainer::LightConvertEntities;

LightConvertClass* IsometricTileTypeExtData::GetLightConvert(IsometricTileTypeClass* pOvrl, int r, int g, int b)
{
	if (!DSurface::Primary())
		return nullptr;

	if(pOvrl){
		auto pExt = IsometricTileTypeExtContainer::Instance.Find(pOvrl);

		if (pExt->Palette) {

			auto& entities = IsometricTileTypeExtContainer::LightConvertEntities[pExt->Palette->Name.data()];

			ScenarioClass::Instance->ScenarioLighting(&r, &g, &b);
			TintStruct tint(r, g, b);
			auto Iter = entities.find(tint);

			if (Iter != entities.end())
			{
				if (!Iter->second)
				{
					LightConvertClass* pLightConvert = GameCreate<LightConvertClass>
						(
							pExt->Palette->Palette.get(),
							&FileSystem::TEMPERAT_PAL,
							DSurface::Primary,
							r,
							g,
							b,
							!entities.empty(),
							nullptr,
							(r + g + b < 2000 ? 27 : 53)
						);

					LightConvertClass::Array->AddItem(pLightConvert);
					Iter->second = pLightConvert;
				}
			}
			else
			{
				LightConvertClass* pLightConvert = GameCreate<LightConvertClass>
					(
						pExt->Palette->Palette.get(),
						&FileSystem::TEMPERAT_PAL,
						DSurface::Primary,
						r,
						g,
						b,
						!entities.empty(),
						nullptr,
						(r + g + b < 2000 ? 27 : 53)
					);

				LightConvertClass::Array->AddItem(pLightConvert);
				entities.emplace(tint, pLightConvert);
			}

			return Iter->second;
		}
	}

	if (r == 1000 && g == 1000 && b == 1000 && LightConvertClass::Array->Count)
		return LightConvertClass::Array->Items[0];

	ScenarioClass::Instance->ScenarioLighting(&r, &g, &b);
	if (LightConvertClass::Array->Count <= 1)
	{
		LightConvertClass* pLightConvert = GameCreate<LightConvertClass>
			(
				&FileSystem::ISOx_PAL,
				&FileSystem::TEMPERAT_PAL,
				DSurface::Primary,
				r,
				g,
				b,
				LightConvertClass::Array->Count != 0,
				nullptr,
				((r + g + b) < 2000 ? 27 : 53)
			);

		LightConvertClass::Array->AddItem(pLightConvert);
		return pLightConvert;
	}
	else
	{
		for(int i = 1; i < LightConvertClass::Array->Count; ++i)  {
			if (LightConvertClass::Array->Items[i]->Color1.Red == r && 
				LightConvertClass::Array->Items[i]->Color1.Green == g && 
				LightConvertClass::Array->Items[i]->Color1.Blue == b
				)
				return LightConvertClass::Array->Items[i];
		}
	}
	
	return nullptr;
}

// =============================
// load / save

void IsometricTileTypeExtData::LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr)
{
	this->Tileset = IsometricTileTypeExtContainer::CurrentTileset;

	char pSection[16];
	IMPL_SNPRNINTF(pSection, sizeof(pSection), "TileSet%04d", IsometricTileTypeExtContainer::CurrentTileset);
	INI_EX exINI(pINI);

	this->Palette.Read(exINI, pSection, "CustomPalette");
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