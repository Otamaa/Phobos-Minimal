#include "Body.h"

#include "NewSuperWeaponType/NewSWType.h"

#include <Utilities/Enum.h>

#include <Ext/TechnoType/Body.h>
#include <Ext/HouseType/Body.h>

#include <Misc/Ares/Hooks/Header.h>
#include <Utilities/Macro.h>
// #896002: darken SW cameo if player can't afford it
ASMJIT_PATCH(0x6A99B7, StripClass_Draw_SuperDarken, 5)
{
	GET(int, idxSW, EDI);
	R->BL(SWTypeExtData::DrawDarken(HouseClass::CurrentPlayer->Supers.Items[idxSW]));
	return 0;
}

// 4AC20C, 7
// translates SW click to type
ASMJIT_PATCH(0x4AC20C, DisplayClass_LeftMouseButtonUp, 7)
{
	GET_STACK(Action, nAction, 0x9C);

	if (nAction < (Action)AresNewActionType::SuperWeaponDisallowed)
	{
		// get the actual firing SW type instead of just the first type of the
		// requested action. this allows clones to work for legacy SWs (the new
		// ones use SW_*_CURSORs). we have to check that the action matches the
		// action of the found type as the no-cursor represents a different
		// action and we don't want to start a force shield even tough the UI
		// says no.
		auto pSW = SuperWeaponTypeClass::Array->GetItemOrDefault(Unsorted::CurrentSWType());
		if (pSW && (pSW->Action != nAction))
		{
			pSW = nullptr;
		}

		R->EAX(pSW);
		return pSW ? 0x4AC21C : 0x4AC294;
	}
	else if (nAction == (Action)AresNewActionType::SuperWeaponDisallowed)
	{
		R->EAX(0);
		return 0x4AC294;
	}

	R->EAX(SWTypeExtData::CurrentSWType);
	return 0x4AC21C;
}

ASMJIT_PATCH(0x653B3A, RadarClass_GetMouseAction_CustomSWAction, 7)
{
	GET_STACK(CellStruct, MapCoords, STACK_OFFS(0x54, 0x3C));
	REF_STACK(MouseEvent, flag, 0x58);

	enum
	{
		CheckOtherCases = 0x653CA3,
		DrawMiniCursor = 0x653CC0,
		NothingToDo = 0,
		DifferentEventFlags = 0x653D6F
	};

	if (Unsorted::CurrentSWType() < 0)
		return NothingToDo;

	if ((flag & (MouseEvent::RightDown | MouseEvent::RightUp)))
		return DifferentEventFlags;

	const auto nResult = SWTypeExtData::GetAction(SuperWeaponTypeClass::Array->Items[Unsorted::CurrentSWType], &MapCoords);

	if (nResult == Action::None)
		return NothingToDo; //vanilla action

	R->ESI(nResult);
	return CheckOtherCases;
}

ASMJIT_PATCH(0x6AAEDF, SidebarClass_ProcessCameoClick_SuperWeapons, 6)
{
	enum
	{
		RetImpatientClick = 0x6AAFB1,
		SendSWLauchEvent = 0x6AAF10,
		ClearDisplay = 0x6AAF46,
		ControlClassAction = 0x6AB95A
	};

	GET(int, idxSW, ESI);
	//impatient voice is implemented inside
	SWTypeExtData::LauchSuper(HouseClass::CurrentPlayer->Supers.Items[idxSW]);
	return ControlClassAction;
}

// bugfix #277 revisited: VeteranInfantry and friends don't show promoted cameos
ASMJIT_PATCH(0x712045, TechnoTypeClass_GetCameo, 5)
{
	GET(TechnoTypeClass*, pThis, ECX);

	R->EAX(TechnoTypeExt_ExtData::CameoIsElite(pThis, HouseClass::CurrentPlayer)
		? pThis->AltCameo : pThis->Cameo);
	return 0x7120C6;
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
