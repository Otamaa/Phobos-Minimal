#include "Body.h"

#include <Misc/Ares/Hooks/Header.h>

#include <Ext/TechnoType/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/Scenario/Body.h>
#include <Ext/HouseType/Body.h>
#include <Ext/Side/Body.h>

#include <Misc/Ares/CSF.h>

#include <TechnoTypeClass.h>
#include <TextDrawing.h>

/*
ASMJIT_PATCH(0x6A9747, StripClass_Draw_GetCameo, 6)
{
	GET(int, CameoIndex, ECX);

	auto& Item = MouseClassExt::TabCameos[MouseClass::Instance->ActiveTabIndex][CameoIndex];

	auto ptr = reinterpret_cast<byte*>(&Item);
	ptr -= 0x58;
	R->EAX<byte*>(ptr);
	R->Stack<byte*>(0x30, ptr);

	R->ECX(Item.ItemType);

	return (Item.ItemType == AbstractType::Special)
		? 0x6A9936
		: 0x6A9761
		;
}

ASMJIT_PATCH(0x6A95C8, StripClass_Draw_Status, 8)
{
	GET(int, CameoIndex, EAX);

	DWORD val = (DWORD)MouseClassExt::TabCameos[MouseClass::Instance->ActiveTabIndex][CameoIndex].Status;
	R->EDX<DWORD*>(&val);

	return 0x6A95D3;
}

ASMJIT_PATCH(0x6A9866, StripClass_Draw_Status_1, 8)
{
	GET(int, CameoIndex, ECX);

	return (MouseClassExt::TabCameos
		[MouseClass::Instance->ActiveTabIndex]
		[CameoIndex].Status == BuildState::Building)
		? 0x6A9874
		: 0x6A98CF
		;
}

ASMJIT_PATCH(0x6A9886, StripClass_Draw_Status_2, 8)
{
	GET(int, CameoIndex, EAX);

	auto ptr = reinterpret_cast<byte*>(
		&MouseClassExt::TabCameos
		[MouseClass::Instance->ActiveTabIndex]
		[CameoIndex]
	);

	ptr += 0x10;
	R->EDI<byte*>(ptr);

	auto dwPtr = reinterpret_cast<DWORD*>(ptr);
	R->EAX<DWORD>(*dwPtr);

	return 0x6A9893;
}

ASMJIT_PATCH(0x6A9EBA, StripClass_Draw_Status_3, 8)
{
	GET(int, CameoIndex, EAX);

	return (MouseClassExt::TabCameos
		[MouseClass::Instance->ActiveTabIndex]
		[CameoIndex].Status == BuildState::OnHold
		)
		? 0x6A9ECC
		: 0x6AA01C
		;
}

ASMJIT_PATCH(0x6A99BE, StripClass_Draw_BreakDrawLoop, 5)
{
	R->Stack8(0x12, 0);
	return 0x6AA01C;
}

ASMJIT_PATCH(0x6A9B4F, StripClass_Draw_TestFlashFrame, 6)
{
	GET(int, CameoIndex, EAX);
	GET(const bool, greyCameo, EBX);
	GET(const int, destX, ESI);
	GET(const int, destY, EBP);
	GET_STACK(const RectangleStruct, boundingRect, STACK_OFFSET(0x48C, -0x3E0));
	GET_STACK(TechnoTypeClass* const, pType, STACK_OFFSET(0x48C, -0x458));

	R->EAX(Unsorted::CurrentFrame());
	if ((MouseClassExt::TabCameos[MouseClass::Instance->ActiveTabIndex]
		[CameoIndex].FlashEndFrame > Unsorted::CurrentFrame
		))
	{
		return 0x6A9B67;
	}

	if (pType)
	{
		//DrawGreyCameoExtraCover

		Point2D position { destX + 30, destY + 24 };
		const auto pRulesExt = RulesExtData::Instance();
		const Vector3D<int>& frames = pRulesExt->Cameo_OverlayFrames.Get();

		if (greyCameo) // Only draw extras over grey cameos
		{
			auto frame = frames.Y;
			const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

			if (pTypeExt->Cameo_AlwaysExist.Get(pRulesExt->Cameo_AlwaysExist))
			{
				auto& vec = ScenarioExtData::Instance()->OwnedExistCameoTechnoTypes;

				if (vec.contains(pType))
				{
					if (const auto CameoPCX = pTypeExt->GreyCameoPCX.GetSurface())
					{
						auto drawRect = RectangleStruct { destX, destY, 60, 48 };
						PCX::Instance->BlitToSurface(&drawRect, DSurface::Sidebar, CameoPCX);
					}

					frame = frames.Z;
				}
			}

			if (frame >= 0)
			{
				ConvertClass* pConvert = FileSystem::PALETTE_PAL;
				if (pRulesExt->Cameo_OverlayPalette.GetConvert())
					pConvert = pRulesExt->Cameo_OverlayPalette.GetConvert();

				DSurface::Sidebar->DrawSHP(
					pConvert,
					pRulesExt->Cameo_OverlayShapes,
					frame,
					&position,
					&boundingRect,
					BlitterFlags(0x600),
					0, 0,
					ZGradient::Ground,
					1000, 0, 0, 0, 0, 0);
			}
		}

		if (const auto pBuildingType = cast_to<BuildingTypeClass*, false>(pType)) // Only count owned buildings
		{
			const auto pHouse = HouseClass::CurrentPlayer();
			auto count = BuildingTypeExtData::GetUpgradesAmount(pBuildingType, pHouse);

			if (count == -1)
				count = pHouse->CountOwnedAndPresent(pBuildingType);

			if (count > 0)
			{
				if (frames.X >= 0)
				{
					ConvertClass* pConvert = FileSystem::PALETTE_PAL;
					if (pRulesExt->Cameo_OverlayPalette.GetConvert())
						pConvert = pRulesExt->Cameo_OverlayPalette.GetConvert();

					DSurface::Sidebar->DrawSHP(
						pConvert,
						pRulesExt->Cameo_OverlayShapes,
						frames.X,
						&position,
						&boundingRect,
						BlitterFlags(0x600),
						0, 0,
						ZGradient::Ground,
						1000, 0, 0, 0, 0, 0);
				}

				if (Phobos::Config::ShowBuildingStatistics
					&& BuildingTypeExtContainer::Instance.Find(pBuildingType)->Cameo_ShouldCount.Get(pBuildingType->BuildCat != BuildCat::Combat || (pBuildingType->BuildLimit != INT_MAX))
					)
				{
					GET_STACK(RectangleStruct, surfaceRect, STACK_OFFSET(0x48C, -0x438));

					const COLORREF color = Drawing::RGB_To_Int(Drawing::TooltipColor);
					const TextPrintType printType = TextPrintType::Background | TextPrintType::Right | TextPrintType::FullShadow | TextPrintType::Point8;
					auto textPosition = Point2D { destX , destY + 1 };
					static fmt::basic_memory_buffer<wchar_t> text;
					text.clear();
					fmt::format_to(std::back_inserter(text), L"{}", count);
					text.push_back(L'\0');
					DSurface::Sidebar->DrawText_Old(text.data(), &surfaceRect, &textPosition, color, 0, (DWORD)printType);
				}
			}
		}
	}

	return 0x6A9BC5;
}

ASMJIT_PATCH(0x6A9C54, StripClass_DrawStrip_FindFactoryDehardCode, 0x6)
{
	GET(TechnoTypeClass* const, pType, ECX);
	LEA_STACK(BuildCat*, pBuildCat, STACK_OFFSET(0x490, -0x490));

	if (const auto pBuildingType = cast_to<BuildingTypeClass*>(pType))
		*pBuildCat = pBuildingType->BuildCat;

	return 0;
}

ASMJIT_PATCH(0x6A9789, StripClass_DrawStrip_NoGreyCameo, 0x6)
{
	enum { CheckWhoCanBuildMe = 0x6A9799, SetDarken = 0x6A97FB };

	GET(TechnoTypeClass* const, pType, EBX);
	GET_STACK(bool, clicked, STACK_OFFSET(0x48C, -0x475));

	if (!RulesExtData::Instance()->ExpandBuildingQueue)
	{
		if (pType->WhatAmI() == AbstractType::BuildingType && clicked)
			return SetDarken;
	}
	else if (const auto pBuildingType = cast_to<BuildingTypeClass*>(pType))
	{
		if (const auto pFactory = HouseClass::CurrentPlayer->GetPrimaryFactory(AbstractType::BuildingType, pType->Naval, pBuildingType->BuildCat))
		{
			if (const auto pProduct = cast_to<BuildingClass*>(pFactory->Object))
			{
				if (pFactory->IsDone() && pProduct->Type != pType && ((pProduct->Type->BuildCat != BuildCat::Combat) ^ (pBuildingType->BuildCat == BuildCat::Combat)))
					return SetDarken;
			}
		}
	}

	return CheckWhoCanBuildMe;
}

// #896002: darken SW cameo if player can't afford it
ASMJIT_PATCH(0x6A99B7, StripClass_Draw_SuperDarken, 5)
{
	GET(int, idxSW, EDI);
	R->BL(SWTypeExtData::DrawDarken(HouseClass::CurrentPlayer->Supers.Items[idxSW]));
	return 0;
}

ConvertClass* SWConvert = nullptr;
BSurface* CameoPCXSurface = nullptr;

ASMJIT_PATCH(0x6A9948, StripClass_Draw_SuperWeapon, 6)
{
	GET(SuperWeaponTypeClass*, pSuper, EAX);

	if (auto pManager = SWTypeExtContainer::Instance.Find(pSuper)->SidebarPalette.GetConvert())
		SWConvert = pManager;

	return 0x0;
}

ASMJIT_PATCH(0x6A9A2A, StripClass_Draw_Main, 6)
{
	GET_STACK(TechnoTypeClass*, pTechno, 0x6C);

	ConvertClass* pResult = nullptr;
	if (pTechno)
	{
		if (auto pPal = TechnoTypeExtContainer::Instance.TryFind(pTechno)->CameoPal.GetConvert())
		{
			pResult = pPal;
		}
	}
	else
		pResult = SWConvert;

	R->EDX(pResult ? pResult : FileSystem::CAMEO_PAL());
	return 0x6A9A30;
}

ASMJIT_PATCH(0x6A9952, StripClass_Draw_SuperWeapon_PCX, 6)
{
	GET(SuperWeaponTypeClass*, pSuper, EAX);
	CameoPCXSurface = SWTypeExtContainer::Instance.Find(pSuper)->SidebarPCX.GetSurface();
	return 0x0;
}

ASMJIT_PATCH(0x6A980A, StripClass_Draw_TechnoType_PCX, 8)
{
	GET(TechnoTypeClass*, pType, EBX);

	CameoPCXSurface = TechnoTypeExt_ExtData::GetPCXSurface(pType, HouseClass::CurrentPlayer);

	return 0;
}

ASMJIT_PATCH(0x6A99F3, StripClass_Draw_SkipSHPForPCX, 6)
{
	if (CameoPCXSurface)
		return 0x6A9A43;

	GET_STACK(SHPStruct const*, pCameo, STACK_OFFS(0x48C, 0x444));

	if (pCameo)
	{
		auto pCameoRef = pCameo->AsReference();
		char pFilename[0x20];
		strcpy_s(pFilename, RulesExtData::Instance()->MissingCameo.data());
		_strlwr_s(pFilename);

		if (!_stricmp(pCameoRef->Filename, GameStrings::XXICON_SHP())
			&& (strstr(pFilename, ".pcx")))
		{
			BSurface* pCXSurf = nullptr;

			if (PCX::Instance->LoadFile(pFilename))
				pCXSurf = PCX::Instance->GetSurface(pFilename);

			if (pCXSurf)
			{
				GET(int, destX, ESI);
				GET(int, destY, EBP);

				RectangleStruct bounds { destX, destY, 60, 48 };
				PCX::Instance->BlitToSurface(&bounds, DSurface::Sidebar, pCXSurf);

				return 0x6A9A43; //skip drawing shp cameo
			}
		}
	}

	return 0;
}

ASMJIT_PATCH(0x6A9A43, StripClass_Draw_DrawPCX, 6)
{
	if (CameoPCXSurface)
	{
		GET(int, TLX, ESI);
		GET(int, TLY, EBP);
		RectangleStruct bounds { TLX, TLY, 60, 48 };
		PCX::Instance->BlitToSurface(&bounds, DSurface::Sidebar, CameoPCXSurface, Drawing::ColorStructToWordRGB(Drawing::DefaultColors[6]));
		CameoPCXSurface = nullptr;
	}

	return 0;
}

ASMJIT_PATCH(0x6a9822, StripClass_Draw_Power, 5)
{
	GET(FactoryClass*, pFactory, ECX);

	bool IsDone = pFactory->IsDone();

	if (IsDone)
	{
		if (auto pBuilding = cast_to<BuildingClass*, false>(pFactory->Object))
		{
			IsDone = pBuilding->FindFactory(true, true) != nullptr;
		}
	}

	R->EAX(IsDone);
	return 0x6A9827;
}

ASMJIT_PATCH(0x6a96d9, StripClass_Draw_Strip, 7)
{
	GET(StripClass*, pThis, EDI);
	GET(int, idx_first, ECX);
	GET(int, idx_Second, EDX);
	R->EAX(&SidebarClass::SelectButtonCombined[idx_Second + 2 * idx_first]);
	return pThis->IsScrolling ? 0x6A9703 : 0x6A9714;
}

ASMJIT_PATCH(0x6AA0CA, StripClass_Draw_DrawObserverBackground, 6)
{
	enum { DrawSHP = 0x6AA0ED, DontDraw = 0x6AA159 };

	GET(HouseTypeClass*, pCountry, EAX);

	const auto pData = HouseTypeExtContainer::Instance.Find(pCountry);

	if (pData->ObserverBackgroundSHP)
	{
		R->EAX<SHPStruct*>(pData->ObserverBackgroundSHP);
		return DrawSHP;
	}
	else if (auto PCXSurface = pData->ObserverBackground.GetSurface())
	{
		GET(int, TLX, EDI);
		GET(int, TLY, EBX);
		RectangleStruct bounds = { TLX, TLY, ObserverBackgroundWidth, ObserverBackgroundHeight };
		PCX::Instance->BlitToSurface(&bounds, DSurface::Sidebar, PCXSurface, Drawing::ColorStructToWordRGB(Drawing::DefaultColors[6]));
	}

	return DontDraw;
}

ASMJIT_PATCH(0x6AA164, StripClass_Draw_DrawObserverFlag, 6)
{
	enum { IDontKnowYou = 0x6AA16D, DrawSHP = 0x6AA1DB, DontDraw = 0x6AA2CE };

	GET(HouseTypeClass*, pCountry, EAX);

	const auto idx = pCountry->ParentIdx;

	//special cases
	if (idx == -2)
	{
		R->EAX(idx);
		return 0x6AA1CD;
	}

	if (idx == -3)
	{
		R->EAX(idx);
		return 0x6AA17D;
	}

	const auto pData = HouseTypeExtContainer::Instance.Find(pCountry);

	if (pData->ObserverFlagSHP)
	{
		R->ESI<SHPStruct*>(pData->ObserverFlagSHP);
		R->EAX<int>(pData->ObserverFlagYuriPAL ? 9 : 0);
		return DrawSHP;
	}
	else if (auto PCXSurface = pData->ObserverFlag.GetSurface())
	{
		GET(int, TLX, EDI);
		GET(int, TLY, EBX);
		RectangleStruct bounds = { TLX + ObserverFlagPCXX , TLY + ObserverFlagPCXY,
				ObserverFlagPCXWidth, ObserverFlagPCXHeight
		};

		PCX::Instance->BlitToSurface(&bounds, DSurface::Sidebar, PCXSurface, Drawing::ColorStructToWordRGB(Drawing::DefaultColors[6]));
	}

	return DontDraw;
}*/

static COMPILETIMEEVAL int ObserverBackgroundWidth = 121;
static COMPILETIMEEVAL int ObserverBackgroundHeight = 96;

static COMPILETIMEEVAL int ObserverFlagPCXX = 70;
static COMPILETIMEEVAL int ObserverFlagPCXY = 70;
static COMPILETIMEEVAL int ObserverFlagPCXWidth = 45;
static COMPILETIMEEVAL int ObserverFlagPCXHeight = 21;

class FakeStripClass : public StripClass
{
public:
	void __Draw_It(bool forceRedraw);
};

static COMPILETIMEEVAL reference<bool, 0xB0B518> const SidebarBlitRequested_FullRedraw {};
static COMPILETIMEEVAL reference<int, 0xB0B4F8> const Sidebar_B0B4F8 {};
static COMPILETIMEEVAL reference<int, 0x884CF8> const Sidebar_884CF8 {};
static COMPILETIMEEVAL reference<wchar_t, 0xB078B0, 64>  const Sidebar_UIName {};
static COMPILETIMEEVAL constant_ptr<int, 0x884C34> const Sidebar_Rank_ {};
static COMPILETIMEEVAL constant_ptr<int, 0x884BB4> const Sidebar_Kills_ {};
static COMPILETIMEEVAL constant_ptr<int, 0x884BD4> const Sidebar_Units_ {};
static COMPILETIMEEVAL constant_ptr<int, 0x884BF4> const Sidebar_Credits_ {};
static COMPILETIMEEVAL constant_ptr<HouseClass*, 0x884B94> const Sidebar_Houses_ {};
static COMPILETIMEEVAL constant_ptr<ColorScheme*, 0x884C14> const Sidebar_Converts_ {};

void __thiscall FakeStripClass::__Draw_It(bool forceRedraw)
{
	// Early exit checks
	// 006A954D: mov al, [esi+1Ch] - check AllowedToDraw
	if (!this->AllowedToDraw)
		return;

	// 006A9558: mov al, [esi+3Ch] - check NeedsRedraw
	// 006A955F: mov al, byte ptr [esp+480h+arg_0] - check forceRedraw
	if (!this->NeedsRedraw && !forceRedraw)
		return;

	// 006A9571: mov byte ptr [esi+3Ch], 0
	this->NeedsRedraw = false;

	// 006A957D: mov SidebarBlitRequested_FullRedraw, 1
	SidebarBlitRequested_FullRedraw = true;
	DSurface* SidebarSurface = DSurface::Sidebar();

	// Setup clip rect from sidebar surface
	// 006A9584-006A95AF: Setup argC rect and get surface dimensions
	RectangleStruct clipRect {
		.X = 0,
		.Y = 0,
		.Width = SidebarSurface->Get_Width(),
		.Height = SidebarSurface->Get_Height()
	};

	// 006A95A3: mov ebx, HouseClass *PlayerPtr
	// 006A95A9: mov ebp, AnotherPlayerPtr2
	HouseClass* pPlayer = HouseClass::CurrentPlayer();
	HouseClass* pAnotherPlayer = HouseClass::Observer();
	StripClass* pStrip = this;

	// 006A95B3-006A95BE: Check if PlayerPtr == AnotherPlayerPtr2 for starting index
	int buildableIdx = 0;
	bool hasActiveProduction = false;

	if (pPlayer == pAnotherPlayer)
	{
		buildableIdx = this->BuildableCount;
	}

	// ==========================================================================
	// HOOK: 0x6A95C8 - StripClass_Draw_Status (8 bytes)
	// Replaces status pointer calculation for extended cameo support
	// Original: lea ecx, [eax+eax*2+6] / lea edx, [eax+ecx*4+2] / lea edx, [esi+edx*4]
	// Hook reads from MouseClassExt::TabCameos[MouseClass::Instance->ActiveTabIndex] instead
	// ==========================================================================

	// 006A95C1-006A95E9: Loop to check if any buildable has active production
	int totalBuildables = this->BuildableCount;
	while (buildableIdx < totalBuildables)
	{
		// Hook: Reads from MouseClassExt::TabCameos[ActiveTabIndex][buildableIdx].Status
		BuildType* pBuildable = &MouseClassExt::TabCameos[MouseClass::Instance->ActiveTabIndex][buildableIdx];
		if (pBuildable->Status == BuildState::Building || pBuildable->Status == BuildState::OnHold)
		{
			hasActiveProduction = true;
			break;
		}
		++buildableIdx;
	}

	// 006A95EE-006A9607: Calculate tab offset based on Scen->SideBarTypeseems
	const int tabOffset = ScenarioClass::Instance->PlayerSideIndex ? 18 : 26;

	// 006A9607-006A9644: Calculate visible rows
	// Formula: ((SidebarBounds.Height - dword_B0B4F8 - tabOffset + SidebarBounds.Y - 7) / 50) + IsScrolling
	int visibleRows = (DSurface::SidebarBounds->Height - Sidebar_B0B4F8 - tabOffset + DSurface::SidebarBounds->Y - 7) / 50;
	visibleRows += (this->IsScrolling != 0) ? 1 : 0;
	int maxRows = visibleRows;

	// 006A9646-006A964E: Branch based on player mode
	// ========================================================================
	// NORMAL MODE: Draw buildable items (PlayerPtr != AnotherPlayerPtr2)
	// ========================================================================
	if (pPlayer != pAnotherPlayer && maxRows > 0) {
		// 006A965C: Main row loop
		for (int rowIndex = 0; rowIndex < maxRows; ++rowIndex)
		{
			// 006A9664: Column loop (0 to 1)
			for (int column = 0; column < 2; ++column)
			{
				// 006A9664-006A96C9: Initialize per-item state
				bool shouldRedraw = forceRedraw;
				bool shouldDarken = false;
				bool isInProduction = false;
				bool isCompleted = false;
				bool isMouseOver = false;
				bool hasQueuedCount = false;

				int progressFrame = 0;
				int topIndex = pStrip->TopRowIndex;
				//int stripTabIndex = pStrip->TabIndex;
				int buildableIndex = column + 2 * (topIndex + rowIndex);
				int buildableCount = pStrip->BuildableCount;

				// Calculate screen position
				int stripX = pStrip->Location.X;
				int screenX = stripX + column * SidebarClass::ObjectWidth - DSurface::SidebarBounds->X;
				int screenY = rowIndex * SidebarClass::ObjectHeight + pStrip->Location.Y + 1;
				int originalScreenY = screenY;

				// Item data pointers
				SHPStruct* cameoShape = NULL;
				TechnoTypeClass* pFactoryType = NULL;
				TechnoTypeClass* pTechnoType = NULL;
				const wchar_t* statusText = NULL;
				const wchar_t* pUIName = NULL;
				BSurface* CameoPCXSurface = nullptr;
				ConvertClass* CameoConvert = nullptr;
				AbstractTypeClass* pBuildableItem = nullptr;

				auto pSideExt = SideExtContainer::Instance.Find(SideClass::Array->Items[pPlayer->SideIndex]);
				
				SHPStruct* _GCLOCK_Shape = FileSystem::GCLOCK2_SHP();
				ConvertClass* _GCLOCK_Convert = FileSystem::CAMEO_PAL();
				BlitterFlags _GCLOCK_Trans = BlitterFlags::TransLucent50;

				if (pSideExt->GClock_Shape)
					_GCLOCK_Shape = pSideExt->GClock_Shape;

				if (auto pConvert = pSideExt->GClock_Palette.GetConvert())
					_GCLOCK_Convert = pConvert;

				if (pSideExt->GClock_Transculency->GetIntValue() > 0)
					_GCLOCK_Trans = pSideExt->GClock_Transculency->GetBlitterFlags();

				// ==========================================================================
				// HOOK: 0x6A96D9 - StripClass_Draw_Strip (7 bytes)
				// Simplifies SelectButton lookup and handles scrolling check
				// Original: Complex calculation with Rate * 15 * 4
				// Hook: R->EAX(&SidebarClass::SelectButtonCombined[column + 2 * rowIndex])
				//       Returns 0x6A9703 if scrolling, 0x6A9714 otherwise
				// ==========================================================================

				// 006A96C6-006A9701: Get SelectClass button
				SelectClass* pSelectButton = &SidebarClass::SelectButtonCombined[rowIndex * 2 + column];

				// 006A9701-006A9710: Adjust position if scrolling
				if (pStrip->IsScrolling)
				{
					screenY = screenY + pStrip->Slid - SidebarClass::ObjectHeight;
					originalScreenY = screenY;
				}

				// 006A9714-006A9727: Process buildable item if within bounds
				if (buildableIndex < buildableCount)
				{
					// 006A972D-006A9742: Check tooltip state
					isMouseOver = (pSelectButton->__MouseOver && !ScenarioClass::IsUserInputLocked());

					// ==========================================================================
					// HOOK: 0x6A9747 - StripClass_Draw_GetCameo (6 bytes)
					// Replaces buildable lookup with MouseClassExt::TabCameos access
					// Original: lea edx, [ecx+ecx*2] / lea eax, [ecx+edx*4] / mov ecx, [edi+eax*4+5Ch]
					// Hook: 
					//   auto& Item = MouseClassExt::TabCameos[MouseClass::Instance->ActiveTabIndex][CameoIndex];
					//   Returns 0x6A9936 for SuperWeapon (AbstractType::Special)
					//   Returns 0x6A9761 for Techno
					// ==========================================================================

					// 006A9747-006A975B: Get buildable info
					BuildType* pBuildable = &MouseClassExt::TabCameos[MouseClass::Instance->ActiveTabIndex][buildableIndex];
					AbstractType buildableType = pBuildable->ItemType;
					// --------------------------------------------------------
					// SuperWeapon handling (AbstractType::Special == 0x1F)
					// 006A9936-006A99BC
					// --------------------------------------------------------

					if (buildableType == AbstractType::Special)
					{
						int superIndex = pBuildable->ItemIndex;
						SuperClass* pSuper = pPlayer->Supers.Items[superIndex];
						SuperWeaponTypeClass* pSuperType = pSuper->Type;
						pBuildableItem = pSuperType;
						pUIName = pSuperType->UIName;

						auto pSWTypeExt = SWTypeExtContainer::Instance.Find(pSuperType);

						// ==========================================================================
						// HOOK: 0x6A9948 - StripClass_Draw_SuperWeapon (6 bytes)
						// Stores custom palette from SWTypeExtContainer for later use
						// Hook: if (auto pManager = SWTypeExtContainer::Instance.Find(pSuper)->SidebarPalette.GetConvert())
						//           SWConvert = pManager;
						// ==========================================================================
						if (auto pManager = pSWTypeExt->SidebarPalette.GetConvert())
							CameoConvert = pManager;
						// ==========================================================================
						// HOOK: 0x6A9952 - StripClass_Draw_SuperWeapon_PCX (6 bytes)
						// Stores PCX surface from SWTypeExtContainer for later drawing
						// Hook: CameoPCXSurface = SWTypeExtContainer::Instance.Find(pSuper)->SidebarPCX.GetSurface();
						// ==========================================================================
						CameoPCXSurface = pSWTypeExt->SidebarPCX.GetSurface();

						// 006A9948-006A995C: Get cameo shape
						cameoShape = pSuperType->SidebarImage;

						if (pSWTypeExt->GClock_Shape)
							_GCLOCK_Shape = pSWTypeExt->GClock_Shape;

						if (auto pConvert = pSWTypeExt->GClock_Palette.GetConvert())
							_GCLOCK_Convert = pConvert;

						if (pSWTypeExt->GClock_Transculency->GetIntValue() > 0)
							_GCLOCK_Trans = pSWTypeExt->GClock_Transculency->GetBlitterFlags();

						isInProduction = true;

						// 006A9966-006A99B1: Get super weapon state
						isCompleted = !pSuper->ShouldDrawProgress();
						statusText = pSuper->NameReadiness();
						progressFrame = pSuper->GetCameoChargeState();

						// ==========================================================================
						// HOOK: 0x6A99B7 - StripClass_Draw_SuperDarken (5 bytes)
						// Darkens SW cameo if player can't afford it
						// Original: xor bl, bl (shouldDarken = false)
						// Hook: R->BL(SWTypeExtData::DrawDarken(HouseClass::CurrentPlayer->Supers.Items[idxSW]));
						// ==========================================================================
						shouldDarken = SWTypeExtData::DrawDarken(pSuper);
						shouldRedraw = true;
					}
					// --------------------------------------------------------
					// Techno handling
					// 006A9761-006A98D3
					// --------------------------------------------------------
					else
					{
						// 006A9761-006A976B: Fetch techno type
						pTechnoType = TechnoTypeClass::FetchTechnoType(buildableType, pBuildable->ItemIndex);

						if (pTechnoType)
						{
							pUIName = pTechnoType->UIName;
							bool shouldDisable = false;

							// ==========================================================================
							// HOOK: 0x6A9789 - StripClass_DrawStrip_NoGreyCameo (6 bytes)
							// Handles ExpandBuildingQueue feature
							// Original: cmp eax, 7 / jnz (check if BuildingType with active production)
							// Hook: Additional checks for BuildCat and factory state when
							//       RulesExtData::Instance()->ExpandBuildingQueue is enabled
							// ==========================================================================

							if (!RulesExtData::Instance()->ExpandBuildingQueue)
							{
								if (pTechnoType->WhatAmI() == AbstractType::BuildingType && hasActiveProduction)
								{
									shouldDisable = true;
								}
							}
							else if (const auto pBuildingType = cast_to<BuildingTypeClass*, false>(pTechnoType))
							{
								if (const auto pFactory = HouseClass::CurrentPlayer->GetPrimaryFactory(AbstractType::BuildingType, pTechnoType->Naval, pBuildingType->BuildCat))
								{
									if (const auto pProduct = cast_to<BuildingClass*>(pFactory->Object))
									{
										if (pFactory->IsDone() && pProduct->Type != pTechnoType && ((pProduct->Type->BuildCat != BuildCat::Combat) ^ (pBuildingType->BuildCat == BuildCat::Combat)))
											shouldDisable = true;
									}
								}
							}

							// 006A978C-006A97B2: Check if cameo should be disabled
							if (!pTechnoType->FindFactory(true, true, true, pPlayer))
							{
								shouldDisable = true;
							}
							else
							{
								// 006A97B4-006A97F9: Check Can_Build and ShouldDisableCameo
								pFactoryType = TechnoTypeClass::FetchTechnoType(pBuildable->ItemType, pBuildable->ItemIndex);

								if (pPlayer->CanBuild(pFactoryType, false, false) == CanBuildResult::TemporarilyUnbuildable || HouseExtData::ShouldDisableCameo(pPlayer, pFactoryType, true))
									shouldDisable = true;
							}

							// ==========================================================================
							// HOOK: 0x6A980A - StripClass_Draw_TechnoType_PCX (8 bytes)
							// Gets PCX surface for techno type cameo
							// Hook: CameoPCXSurface = TechnoTypeExt_ExtData::GetPCXSurface(pType, HouseClass::CurrentPlayer);
							// ==========================================================================
							auto pTechnoTypeExt = TechnoTypeExtContainer::Instance.Find(pTechnoType);

							if (pTechnoTypeExt->GClock_Shape)
								_GCLOCK_Shape = pTechnoTypeExt->GClock_Shape;

							if (auto pConvert = pTechnoTypeExt->GClock_Palette.GetConvert())
								_GCLOCK_Convert = pConvert;

							if (pTechnoTypeExt->GClock_Transculency->GetIntValue() > 0)
								_GCLOCK_Trans = pTechnoTypeExt->GClock_Transculency->GetBlitterFlags();

							CameoPCXSurface = TechnoTypeExt_ExtData::GetPCXSurface(pTechnoType, pPlayer);

							// 006A9800-006A980E: Get cameo data
							cameoShape = pTechnoType->GetCameo();
							if (auto pConv = TechnoTypeExtContainer::Instance.Find(pTechnoType)->CameoPal.GetConvert())
								CameoConvert = pConv;

							FactoryClass* pFactory = pBuildable->CurrentFactory;

							// ------------------------------------------------
							// Factory exists - 006A981D-006A9877
							// ------------------------------------------------
							if (pFactory)
							{
								isInProduction = true;

								// ==========================================================================
								// HOOK: 0x6A9822 - StripClass_Draw_Power (5 bytes)
								// Checks if factory is done AND has power (for buildings)
								// Original: call Has_Completed
								// Hook: Also checks pBuilding->FindFactory(true, true) for power requirement
								// ==========================================================================

								// 006A9822-006A982D: Check if completed
								isCompleted = pFactory->IsDone();

								if (isCompleted)
								{
									if (auto pBuilding = cast_to<BuildingClass*, false>(pFactory->Object))
									{
										isCompleted = pBuilding->FindFactory(true, true) != nullptr;
									}
								}

								if (isCompleted)
									statusText = CSFLoader::FetchStringManager("TXT_READY", NULL, NULL, 0);
								else
									shouldRedraw = true;

								// 006A9850-006A9877: Calculate progress
								int completion = pFactory->GetProgress();

								// ==========================================================================
								// HOOK: 0x6A9866 - StripClass_Draw_Status_1 (8 bytes)
								// Checks status from MouseClassExt::TabCameos
								// Hook: Returns 0x6A9874 if Building, 0x6A98CF otherwise
								// ==========================================================================

								if (pBuildable->Status == BuildState::Building)
								{
									int storedProgress = pBuildable->Progress.Stage;
									if (storedProgress > completion)
										completion = (storedProgress + completion) / 2;
								}
								progressFrame = completion;
								shouldDarken = false;
							}
							// ------------------------------------------------
							// No factory - check status directly
							// 006A9879-006A98CF
							// ------------------------------------------------
							else
							{
								shouldDarken = shouldDisable;
								isInProduction = false;

								// ==========================================================================
								// HOOK: 0x6A9886 - StripClass_Draw_Status_2 (8 bytes)
								// Gets status from MouseClassExt::TabCameos
								// Hook: Calculates pointer to extended cameo data + 0x10 (Status offset)
								// ==========================================================================

								// 006A9891-006A9931: Switch on buildable status
								switch (pBuildable->Status)
								{
								case BuildState::Normal:
									if (!forceRedraw)
										continue;
									break;

								case BuildState::Building:
								{
									shouldRedraw = true;
									isInProduction = true;
									int completion = 0;
									int storedProgress = pBuildable->Progress.Stage;
									if (storedProgress > completion)
										completion = (storedProgress + completion) / 2;
									progressFrame = completion;
									break;
								}

								case BuildState::OnHold:
								{
									isInProduction = true;
									isCompleted = false;
									shouldDarken = true;
									int completion = 0;
									if (pBuildable->Status == BuildState::Building)
									{
										int storedProgress = pBuildable->Progress.Stage;
										if (storedProgress > 0)
											completion = storedProgress / 2;
									}
									progressFrame = completion;
									break;
								}

								case BuildState::Ready:
									isInProduction = true;
									isCompleted = true;
									shouldDarken = false;
									break;
								}
							}
						}

						pBuildableItem = pTechnoType;
					}
				} else { // dont proceed further if nothing is found
					isInProduction = false;
					continue;
				}

				// ==========================================================================
				// HOOK: 0x6A99BE - StripClass_Draw_BreakDrawLoop (5 bytes)
				// Sets isInProduction = false and jumps to drawing section
				// ==========================================================================

				// ============================================================
				// DRAWING SECTION - 006A99C3-006AA01C
				// ============================================================

				RectangleStruct boundingRect = { 0, 0, screenX + 60, screenY + 48 };

				if (!shouldRedraw)
					continue;

				// ==========================================================================
				// HOOK: 0x6A99F3 - StripClass_Draw_SkipSHPForPCX (6 bytes)
				// Skips SHP drawing if PCX surface exists, also handles MissingCameo
				// ==========================================================================

				if (CameoPCXSurface)
				{
					RectangleStruct bounds { screenX, screenY, 60, 48 };
					PCX::Instance->BlitToSurface(&bounds, DSurface::Sidebar, CameoPCXSurface, Drawing::DefaultColors[6].ToInit());
					CameoPCXSurface = nullptr;
				}
				else
					// 006A99F3-006A9A3E: Draw cameo shape
					if (cameoShape)
					{
						// ==========================================================================
						// HOOK: 0x6A9A2A - StripClass_Draw_Main (6 bytes)
						// Gets custom palette for cameo drawing
						// Original: mov edx, ConvertClass * CameoDrawer
						// Hook: Uses TechnoTypeExtContainer palette or SWConvert if available
						// ==========================================================================
						bool drawSHP = true;
						auto pCameoRef = cameoShape->AsReference();
						char pFilename[0x20];
						strcpy_s(pFilename, RulesExtData::Instance()->MissingCameo.data());
						_strlwr_s(pFilename);

						if (!_stricmp(pCameoRef->Filename, GameStrings::XXICON_SHP())
							&& (strstr(pFilename, ".pcx")))
						{
							BSurface* pCXSurf = nullptr;

							if (PCX::Instance->LoadFile(pFilename))
								pCXSurf = PCX::Instance->GetSurface(pFilename);

							if (pCXSurf)
							{
								RectangleStruct bounds { screenX, screenY, 60, 48 };
								PCX::Instance->BlitToSurface(&bounds, DSurface::Sidebar, pCXSurf);

								drawSHP = false;
							}
						}

						if (drawSHP)
						{
							Point2D cameoPos = { screenX, screenY };
							CC_Draw_Shape(SidebarSurface, CameoConvert ? CameoConvert : FileSystem::CAMEO_PAL(), cameoShape, 0, &cameoPos,
									  &boundingRect, BlitterFlags::bf_400, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
						}
					}

				// ==========================================================================
				// HOOK: 0x6A9A43 - StripClass_Draw_DrawPCX (6 bytes)
				// Draws PCX cameo if CameoPCXSurface was set by earlier hooks
				// ==========================================================================
				const int tooltipColor = Drawing::TooltipColor->ToInit();
				const int textColor = tooltipColor;

				// 006A9A43-006A9AFB: Draw tooltip highlight rectangle
				if (isMouseOver)
				{
					RectangleStruct highlightRect = { screenX, screenY, 60, 48 };
					SidebarSurface->Draw_Rect(boundingRect, highlightRect, textColor);
				}

				// 006A9AFB-006A9B4B: Draw darken overlay
				if (shouldDarken)
				{
					Point2D darkenPos = { screenX, screenY };
					CC_Draw_Shape(SidebarSurface, FileSystem::CAMEO_PAL(), FileSystem::DARKEN_SHP(), 0, &darkenPos,
								  &boundingRect, BlitterFlags::bf_400 | BlitterFlags::Darken, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
				}

				// ==========================================================================
				// HOOK: 0x6A9B4F - StripClass_Draw_TestFlashFrame (6 bytes)
				// Major hook: Flash check, grey cameo overlay, building count, statistics
				// ==========================================================================
				if (pBuildableItem)
				{
					//DrawGreyCameoExtraCover

					if(pTechnoType){
						Point2D position { screenX + 30, screenY + 24 };
						const auto pRulesExt = RulesExtData::Instance();
						const Vector3D<int>& frames = pRulesExt->Cameo_OverlayFrames.Get();

						if (shouldDarken) // Only draw extras over grey cameos
						{
							auto frame = frames.Y;
							const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pTechnoType);

							if (pTypeExt->Cameo_AlwaysExist.Get(pRulesExt->Cameo_AlwaysExist))
							{
								auto& vec = ScenarioExtData::Instance()->OwnedExistCameoTechnoTypes;

								if (vec.contains(pTechnoType))
								{
									if (const auto CameoPCX = pTypeExt->GreyCameoPCX.GetSurface())
									{
										auto drawRect = RectangleStruct { screenX, screenY, 60, 48 };
										PCX::Instance->BlitToSurface(&drawRect, DSurface::Sidebar, CameoPCX);
									}

									frame = frames.Z;
								}
							}

							if (frame >= 0)
							{
								ConvertClass* pConvert = FileSystem::PALETTE_PAL;
								if (pRulesExt->Cameo_OverlayPalette.GetConvert())
									pConvert = pRulesExt->Cameo_OverlayPalette.GetConvert();

								DSurface::Sidebar->DrawSHP(
									pConvert,
									pRulesExt->Cameo_OverlayShapes,
									frame,
									&position,
									&boundingRect,
									BlitterFlags(0x600),
									0, 0,
									ZGradient::Ground,
									1000, 0, 0, 0, 0, 0);
							}
						}

						//TODO : check bounding rect
						if (const auto pBuildingType = cast_to<BuildingTypeClass*, false>(pTechnoType)) // Only count owned buildings
						{
							const auto pHouse = HouseClass::CurrentPlayer();
							auto count = BuildingTypeExtData::GetUpgradesAmount(pBuildingType, pHouse);

							if (count == -1)
								count = pHouse->CountOwnedAndPresent(pBuildingType);

							if (count > 0)
							{
								if (frames.X >= 0)
								{
									ConvertClass* pConvert = FileSystem::PALETTE_PAL;
									if (pRulesExt->Cameo_OverlayPalette.GetConvert())
										pConvert = pRulesExt->Cameo_OverlayPalette.GetConvert();

									DSurface::Sidebar->DrawSHP(
										pConvert,
										pRulesExt->Cameo_OverlayShapes,
										frames.X,
										&position,
										&boundingRect,
										BlitterFlags(0x600),
										0, 0,
										ZGradient::Ground,
										1000, 0, 0, 0, 0, 0);
								}

								if (Phobos::Config::ShowBuildingStatistics
									&& BuildingTypeExtContainer::Instance.Find(pBuildingType)->Cameo_ShouldCount.Get(pBuildingType->BuildCat != BuildCat::Combat || (pBuildingType->BuildLimit != INT_MAX))
									)
								{

									const TextPrintType printType = TextPrintType::Background | TextPrintType::Right | TextPrintType::FullShadow | TextPrintType::Point8;
									auto textPosition = Point2D { screenX , screenY + 1 };
									static fmt::basic_memory_buffer<wchar_t> text;
									text.clear();
									fmt::format_to(std::back_inserter(text), L"{}", count);
									text.push_back(L'\0');
									DSurface::Sidebar->DrawText_Old(text.data(), &boundingRect, &textPosition, textColor, 0, (DWORD)printType);
								}
							}
						}
					
					}

					// 006A9B4B-006A9BC5: Draw flashing effect

					BuildType* pCurrentBuildable = &MouseClassExt::TabCameos[MouseClass::Instance->ActiveTabIndex][buildableIndex];
					if (pCurrentBuildable->FlashEndFrame > Unsorted::CurrentFrame) {
						if ((Unsorted::CurrentFrame & 0xF) > 8)
						{
							Point2D flashPos = { screenX, screenY };
							CC_Draw_Shape(SidebarSurface, FileSystem::CAMEO_PAL(), FileSystem::DARKEN_SHP(), 0, &flashPos,
										  &boundingRect, BlitterFlags::bf_400 | BlitterFlags::TransLucent50, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
						}
					}

					// 006A9BC5-006A9BF1: Draw UI name
					if (pUIName) {
						Point2D namePos = { screenX, screenY + 36 };
						TextDrawing::Draw_Text_On_Sidebar(pUIName, &namePos, &clipRect, 60);
					}

					// ==========================================================================
					// HOOK: 0x6A9C54 - StripClass_DrawStrip_FindFactoryDehardCode (6 bytes)
					// Gets BuildCat from BuildingType for factory lookup
					// ==========================================================================

					// 006A9BF1-006A9D88: Draw queued count

					if (pTechnoType)
					{
						bool isNaval = pTechnoType->Naval;
						AbstractType rtti = pTechnoType->WhatAmI();

						BuildCat cat = BuildCat::DontCare;
						if (const auto pBuildingType = cast_to<BuildingTypeClass*, false>(pTechnoType))
							cat = pBuildingType->BuildCat;

						if (FactoryClass* pHouseFactory = pPlayer->GetPrimaryFactory(rtti, isNaval, cat))
						{
							int queuedCount = pHouseFactory->CountTotal(pTechnoType);
							bool shouldDrawCount = (queuedCount > 1);

							if (!shouldDrawCount && queuedCount > 0)
							{
								TechnoClass* pFactoryObject = pHouseFactory->Object;
								if (!pFactoryObject)
								{
									shouldDrawCount = true;
								}
								else
								{
									TechnoTypeClass* pFactoryObjType = pFactoryObject->GetTechnoType();
									if (pFactoryObjType && pFactoryObjType != pTechnoType)
										shouldDrawCount = true;
								}
							}

							if (shouldDrawCount)
							{
								wchar_t countBuffer[8];
								swprintf(countBuffer, L"%d", queuedCount);

								int countX = screenX + 60;
								int countY = screenY + 1;
								Point2D countPos = { countX, countY };
								RectangleStruct countBgRect;
								Drawing::GetTextDimensions(&countBgRect, countBuffer, countPos, TextPrintType::Right | TextPrintType::FullShadow | TextPrintType::Point8, 2, 1);
								LoadProgressManager::FillRectWithColor(countBgRect, SidebarSurface, 0, 0xAF);
								TextDrawing::Fancy_Text_Print_Wide_NoFormat(countBuffer, SidebarSurface, &clipRect, &countPos,
													  textColor, 0, TextPrintType::Right | TextPrintType::FullShadow | TextPrintType::Point8);
								hasQueuedCount = true;
							}
						}
					}

					// 006A9D88-006AA01C: Draw status text and progress clock
					if (isInProduction)
					{
						// Draw status text ("Ready", etc.)
						if (statusText)
						{
							int statusX = screenX + 30;
							int statusY = screenY + 1;
							Point2D statusP { statusX, statusY };
							RectangleStruct statusBgRect;
							Drawing::GetTextDimensions(&statusBgRect, statusText, statusP, TextPrintType::Center | TextPrintType::FullShadow | TextPrintType::Point8, 2, 1);

							LoadProgressManager::FillRectWithColor(statusBgRect, SidebarSurface, 0, 0xAF);

							Point2D statusPos = { statusX, statusY };
							TextDrawing::Fancy_Text_Print_Wide_NoFormat(statusText, SidebarSurface, &clipRect, &statusPos,
												  textColor, 0, TextPrintType::Center | TextPrintType::FullShadow | TextPrintType::Point8);
						}

						// Draw progress clock if not completed
						if (!isCompleted)
						{
							Point2D clockPos = { screenX, screenY };
							CC_Draw_Shape(SidebarSurface, _GCLOCK_Convert, _GCLOCK_Shape,
										  progressFrame + 1, &clockPos, &boundingRect,
										  BlitterFlags::bf_400 | _GCLOCK_Trans, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);

							// ==========================================================================
							// HOOK: 0x6A9EBA - StripClass_Draw_Status_3 (8 bytes)
							// Checks status from MouseClassExt::TabCameos for HOLD text
							// ==========================================================================

							// Check if should draw "HOLD" text
							bool shouldDrawHold = false;
							FactoryClass* pCurrentFactory = pCurrentBuildable->CurrentFactory;

							if (pCurrentFactory && (!pCurrentFactory->Production.Timer.Rate || pCurrentFactory->IsSuspended))
								shouldDrawHold = true;

							if (pCurrentBuildable->Status == BuildState::OnHold)
								shouldDrawHold = true;

							if (shouldDrawHold)
							{
								const wchar_t* holdText = CSFLoader::FetchStringManager("TXT_HOLD", NULL, NULL, 0);

								if (hasQueuedCount)
								{
									// Draw at top-left (queued count shown)
									int holdX = screenX + 2;
									int holdY = originalScreenY + 1;
									Point2D holdPos = { holdX, holdY };
									RectangleStruct holdBgRect;
									Drawing::GetTextDimensions(&holdBgRect, holdText, holdPos, TextPrintType::FullShadow | TextPrintType::Point8, 2, 1);
									LoadProgressManager::FillRectWithColor(holdBgRect, SidebarSurface, 0, 0xAF);
									TextDrawing::Fancy_Text_Print_Wide_NoFormat(holdText, SidebarSurface, &clipRect, &holdPos,
														  textColor, 0, TextPrintType::FullShadow | TextPrintType::Point8);
								}
								else
								{
									// Draw centered (no queued count)
									int holdX = screenX + 30;
									int holdY = originalScreenY + 1;
									Point2D holdPos = { holdX, holdY };
									RectangleStruct holdBgRect;
									Drawing::GetTextDimensions(&holdBgRect, holdText, holdPos, TextPrintType::Center | TextPrintType::FullShadow | TextPrintType::Point8, 2, 1);
									LoadProgressManager::FillRectWithColor(holdBgRect, SidebarSurface, 0, 0xAF);
									TextDrawing::Fancy_Text_Print_Wide_NoFormat(holdText, SidebarSurface, &clipRect, &holdPos,
														  textColor, 0, TextPrintType::Center | TextPrintType::FullShadow | TextPrintType::Point8);
								}
							}
						}
					}
				} // end column loop
			} // end row loop
		}
	}

	// ========================================================================
	// OBSERVER MODE: Draw house info panels
	// 006AA05B-006AA595
	// Only entered if PlayerPtr == AnotherPlayerPtr2
	// ========================================================================
	if (pPlayer == pAnotherPlayer && maxRows > 0) {
		int observerRows = maxRows / 2;

		for (int rowIndex = 0; rowIndex < observerRows; ++rowIndex)
		{
			int houseIndex = pStrip->TopRowIndex + rowIndex;
			int panelX = pStrip->Location.X - DSurface::SidebarBounds->X;
			int panelY = pStrip->Location.Y + 2 * rowIndex * SidebarClass::ObjectHeight + 1;

			// Bounds check
			if (houseIndex >= Sidebar_884CF8)
				break;

			HouseClass* pObservedHouse = Sidebar_Houses_[houseIndex];

			if (!pObservedHouse)
				break;

			HouseTypeClass* pHouseType = pObservedHouse->Type;

			// --------------------------------------------------------------------
			// Draw faction shape (Allied/Soviet/Yuri)
			// --------------------------------------------------------------------
			if (pHouseType)
			{

				const auto pData = HouseTypeExtContainer::Instance.Find(pHouseType);

				if (pData->ObserverBackgroundSHP)
				{

					RectangleStruct factionBounds = { 0, 0, panelX + pData->ObserverBackgroundSHP->Width, panelY + pData->ObserverBackgroundSHP->Height };
					Point2D factionPos = { panelX, panelY };
					CC_Draw_Shape(SidebarSurface, FileSystem::ObserverDrawer(), pData->ObserverBackgroundSHP, 0, &factionPos,
								  &factionBounds, BlitterFlags::bf_400, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);

				}
				else if (auto PCXSurface = pData->ObserverBackground.GetSurface())
				{
					RectangleStruct bounds = { panelX, panelY, ObserverBackgroundWidth, ObserverBackgroundHeight };
					PCX::Instance->BlitToSurface(&bounds, DSurface::Sidebar, PCXSurface, Drawing::DefaultColors[6].ToInit());
				}
			}

			// --------------------------------------------------------------------
			// Draw country shape
			// --------------------------------------------------------------------
			if (pHouseType)
			{
				const auto idx = pHouseType->ParentIdx;
				SHPStruct* pCountryShape = NULL;
				int countryIndex = 0;

				//special cases
				if (idx == -2)
				{
					pCountryShape = FileSystem::RandomSideShape_SHP;
				}
				else if (idx == -3)
				{
					pCountryShape = FileSystem::ObserverSideShape_SHP;
				}
				else
				{

					const auto pData = HouseTypeExtContainer::Instance.Find(pHouseType);

					if (pData->ObserverFlagSHP)
					{
						pCountryShape = pData->ObserverFlagSHP;
						countryIndex = pData->ObserverFlagYuriPAL ? 9 : 0;
					}
					else if (auto PCXSurface = pData->ObserverFlag.GetSurface())
					{
						RectangleStruct bounds = { panelX + ObserverFlagPCXX , panelY + ObserverFlagPCXY,
								ObserverFlagPCXWidth, ObserverFlagPCXHeight
						};

						PCX::Instance->BlitToSurface(&bounds, DSurface::Sidebar, PCXSurface, Drawing::DefaultColors[6].ToInit());
					}
				}

				if (pCountryShape)
				{
					int countryShapeX = panelX + 70;
					int countryShapeY = panelY + 70;

					RectangleStruct countryBounds = { 0, 0, pCountryShape->Width + countryShapeX, pCountryShape->Height + countryShapeY };
					Point2D countryPos = { countryShapeX, countryShapeY };

					// Yuri uses different drawer
					if (countryIndex == 9)
					{
						CC_Draw_Shape(SidebarSurface, FileSystem::YuriObserverDrawer(), pCountryShape, 0, &countryPos,
									  &countryBounds, BlitterFlags::bf_400, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
					}
					else
					{
						CC_Draw_Shape(SidebarSurface, FileSystem::ObserverDrawer(), pCountryShape, 0, &countryPos,
									  &countryBounds, BlitterFlags::bf_400, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
					}
				}
			}

			// --------------------------------------------------------------------
			// Draw stats text
			// --------------------------------------------------------------------
			wcscpy(Sidebar_UIName(), pObservedHouse->UIName);

			constexpr int TextPadding = 17;
			wchar_t rankStr[16], killsStr[16], unitsStr[16], creditsStr[16];

			_itow(Sidebar_Kills_[houseIndex], killsStr, 10);
			_itow(Sidebar_Units_[houseIndex], unitsStr, 10);
			_itow(Sidebar_Credits_[houseIndex], creditsStr, 10);

			ColorScheme* pColorScheme = Sidebar_Converts_[houseIndex];

			// Adjust for scrolling
			int adjustedPanelY = panelY;
			if (pStrip->IsScrolling)
				adjustedPanelY += pStrip->Slid - SidebarClass::ObjectHeight;

			Point2D textPos = { panelX + 8, adjustedPanelY + 4 };
			wchar_t textBuffer[64];

			// Draw house name
			TextDrawing::Fancy_Text_Print_Wide_NoFormat(Sidebar_UIName(), SidebarSurface, &clipRect, &textPos,
								  pColorScheme, 0, TextPrintType::FullShadow | TextPrintType::Point8);

			// Draw rank (internet only)
			if (SessionClass::Instance->GameMode == GameMode::Internet)
			{
				textPos.Y += TextPadding;
				_itow(Sidebar_Rank_[houseIndex], rankStr, 10);

				if (Sidebar_Rank_[houseIndex] >= 1)
				{
					swprintf(textBuffer, CSFLoader::FetchStringManager("GUI:ObsRank", NULL, NULL, 0), rankStr);
				}
				else
				{
					wcscpy(textBuffer, CSFLoader::FetchStringManager("TXT_UNRANKED", NULL, NULL, 0));
				}

				TextDrawing::Fancy_Text_Print_Wide_NoFormat(textBuffer, SidebarSurface, &clipRect, &textPos,
									  pColorScheme, 0, TextPrintType::FullShadow | TextPrintType::Point8);
			}

			// Draw kills
			textPos.Y += TextPadding;
			swprintf(textBuffer, CSFLoader::FetchStringManager("GUI:ObsKills", NULL, NULL, 0), killsStr);
			TextDrawing::Fancy_Text_Print_Wide_NoFormat(textBuffer, SidebarSurface, &clipRect, &textPos,
								  pColorScheme, 0, TextPrintType::FullShadow | TextPrintType::Point8);

			// Draw units
			textPos.Y += TextPadding;
			swprintf(textBuffer, CSFLoader::FetchStringManager("GUI:ObsUnits", NULL, NULL, 0), unitsStr);
			TextDrawing::Fancy_Text_Print_Wide_NoFormat(textBuffer, SidebarSurface, &clipRect, &textPos,
								  pColorScheme, 0, TextPrintType::FullShadow | TextPrintType::Point8);

			// Draw credits
			textPos.Y += TextPadding;
			swprintf(textBuffer, CSFLoader::FetchStringManager("GUI:ObsCredits", NULL, NULL, 0), creditsStr);
			TextDrawing::Fancy_Text_Print_Wide_NoFormat(textBuffer, SidebarSurface, &clipRect, &textPos,
								  pColorScheme, 0, TextPrintType::FullShadow | TextPrintType::Point8);
		}
	}

	// 006AA59B-006AA5A5: Store Slid to __LastSlid
	pStrip->__LastSlid = pStrip->Slid;
}

DEFINE_FUNCTION_JUMP(CALL, 0x6A6FDF, FakeStripClass::__Draw_It)