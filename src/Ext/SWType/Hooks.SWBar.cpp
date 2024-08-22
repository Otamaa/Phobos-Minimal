#include "Body.h"

#include "SuperWeaponSidebar.h"
#include <EventClass.h>

#include <Ext/Rules/Body.h>
#include <Ext/Side/Body.h>

#include <Misc/PhobosToolTip.h>

int MaxSWPerRow = 10;
int MaxSWRow = 20;
int CurrentSWBarPage = 1;

SHPStruct* SWBarSHP_Top = nullptr;
SHPStruct* SWBarSHP_Bottom = nullptr;
SHPStruct* SWBarSHP_Right = nullptr;

ConvertClass* SWBarPalette = FileSystem::PALETTE_PAL();

DEFINE_HOOK(0x69300B, MouseClass_UpdateCursor, 0x6)
{
	const auto pCurrent = HouseClass::CurrentPlayer();
	if (pCurrent->Defeated)
		return 0;

	int superCount = 0;

	for (const auto pSuper : pCurrent->Supers) {
		auto pSWTypeExt = SWTypeExtContainer::Instance.Find(pSuper->Type);
		if (pSuper->Granted && pSWTypeExt->SW_ShowCameo)
			superCount++;
	}

	if (superCount == 0)
		return 0;

	const Point2D crdCursor = { WWMouseClass::Instance->GetX(), WWMouseClass::Instance->GetY() };
	const int cameoWidth = 60;
	const int cameoHeight = 48;
	Point2D location = { 0, (DSurface::ViewBounds->Height - std::min(superCount, MaxSWPerRow) * cameoHeight) / 2 };
	int location_Y = location.Y;
	int row = 0, line = 0;

	for (int idx = 0; idx < superCount && MaxSWPerRow - line > 0; idx++)
	{
		if (crdCursor.X > location.X && crdCursor.X < location.X + cameoWidth && crdCursor.Y > location.Y && crdCursor.Y < location.Y + cameoHeight)
		{
			R->Stack(STACK_OFFSET(0x30, -0x24), NULL);
			R->EAX(Action::None);
			return 0x69301A;
		}

		row++;

		if (row >= MaxSWPerRow - line)
		{
			row = 0;
			line++;

			if (MaxSWRow > 0 && line >= MaxSWRow)
			{
				break;
			}

			location_Y += cameoHeight / 2;
			location = { location.X + cameoWidth, location_Y };
		}
		else
		{
			location.Y += cameoHeight;
		}
	}

	return 0;
}

DEFINE_HOOK(0x6931A5, MouseClass_UpdateCursor_LeftPress, 0x6)
{
	const auto pCurrent = HouseClass::CurrentPlayer();
	if (pCurrent->Defeated)
		return 0;

	int superCount = 0;

	for (const auto pSuper : pCurrent->Supers)
	{
		auto pSWTypeExt = SWTypeExtContainer::Instance.Find(pSuper->Type);
		if (pSuper->Granted && pSWTypeExt->SW_ShowCameo)
			superCount++;
	}

	if (superCount == 0)
		return 0;

	const Point2D crdCursor = { WWMouseClass::Instance->GetX(), WWMouseClass::Instance->GetY() };
	const int cameoWidth = 60;
	const int cameoHeight = 48;
	Point2D location = { 0, (DSurface::ViewBounds->Height - std::min(superCount, MaxSWPerRow) * cameoHeight) / 2 };
	int location_Y = location.Y;
	int row = 0, line = 0;

	for (int idx = 0; idx < superCount && MaxSWPerRow - line > 0; idx++)
	{
		if (crdCursor.X > location.X && crdCursor.X < location.X + cameoWidth && crdCursor.Y > location.Y && crdCursor.Y < location.Y + cameoHeight)
		{
			R->EAX(Action::None);
			return 0x6931B4;
		}

		row++;

		if (row >= MaxSWPerRow - line)
		{
			row = 0;
			line++;

			if (MaxSWRow > 0 && line >= MaxSWRow)
			{
				break;
			}

			location_Y += cameoHeight / 2;
			location = { location.X + cameoWidth, location_Y };
		}
		else
		{
			location.Y += cameoHeight;
		}
	}

	return 0;
}

DEFINE_HOOK(0x693268, MouseClass_UpdateCursor_LeftRelease, 0x5)
{
	const auto pCurrent = HouseClass::CurrentPlayer();
	if (pCurrent->Defeated)
		return 0;

	std::vector<SuperClass*> grantedSupers;

	for (const auto pSuper : pCurrent->Supers) {
		auto pSWTypeExt = SWTypeExtContainer::Instance.Find(pSuper->Type);
		if (pSuper->Granted && pSWTypeExt->SW_ShowCameo)
			grantedSupers.emplace_back(pSuper);
	}

	if (grantedSupers.empty())
		return 0;

	std::sort(grantedSupers.begin(), grantedSupers.end(),
		[](SuperClass* a, SuperClass* b)
		{
			return BuildType::SortsBefore
			(
				AbstractType::Special,
				a->Type->ArrayIndex,
				AbstractType::Special,
				b->Type->ArrayIndex
			);
		});

	std::vector<SuperClass*> grantedSupersPage;
	if (MaxSWRow > 0 && MaxSWPerRow > 0)
	{
		size_t MaxCount = 0;
		int NowRow = MaxSWPerRow;
		for (int NowLine = 1; NowLine <= MaxSWRow && NowRow > 0; NowLine++)
		{
			MaxCount += NowRow * NowLine;
			NowRow--;
		}

		if (grantedSupers.size() <= MaxCount)
		{
			grantedSupersPage = grantedSupers;
		}
		else
		{
			int MaxPage = (grantedSupers.size() % MaxCount == 0) ? grantedSupers.size() / MaxCount : (grantedSupers.size() / MaxCount) + 1;
			if (CurrentSWBarPage > MaxPage)
			{
				CurrentSWBarPage = MaxPage;
			}

			size_t beginidx = (CurrentSWBarPage - 1) * MaxCount;

			for (size_t idx = 0; idx < grantedSupers.size(); idx++)
			{
				if (idx >= beginidx && idx < (beginidx + MaxCount))
				{
					grantedSupersPage.emplace_back(grantedSupers[idx]);
				}
			}
		}
	}
	else
	{
		grantedSupersPage = grantedSupers;
	}

	const Point2D crdCursor = { WWMouseClass::Instance->GetX(), WWMouseClass::Instance->GetY() };
	const int cameoWidth = 60;
	const int cameoHeight = 48;
	const int superCount = static_cast<int>(grantedSupersPage.size());
	Point2D location = { 0, (DSurface::ViewBounds->Height - std::min(superCount, MaxSWPerRow) * cameoHeight) / 2 };
	int location_Y = location.Y;
	int row = 0, line = 0;

	for (const auto pSuper : grantedSupersPage)
	{
		if (crdCursor.X > location.X && crdCursor.X < location.X + cameoWidth && crdCursor.Y > location.Y && crdCursor.Y < location.Y + cameoHeight)
		{
			bool useAITargeting = false;

			if (const auto pSWExt = SWTypeExtContainer::Instance.Find(pSuper->Type))
			{
				useAITargeting = pSWExt->SW_UseAITargeting;
				VocClass::PlayGlobal(RulesClass::Instance->GUIBuildSound, Panning::Center, 1.0);
				const bool manual = !pSWExt->SW_ManualFire && pSWExt->SW_AutoFire;
				const bool unstopable = pSuper->Type->UseChargeDrain && pSuper->ChargeDrainState == ChargeDrainState::Draining
					&& pSWExt->SW_Unstoppable;

				if (!pSuper->CanFire() && !manual)
				{
					VoxClass::PlayIndex(pSWExt->AttachedToObject->ImpatientVoice);
				}
				else
				{
					if (!pCurrent->CanTransactMoney(pSWExt->Money_Amount))
					{
						VoxClass::PlayIndex(pSWExt->EVA_InsufficientFunds);
						pSWExt->PrintMessage(pSWExt->Message_InsufficientFunds, pCurrent);
					}
					else if (!manual && !unstopable)
					{
						const auto swIndex = pSuper->Type->ArrayIndex;

						if (pSuper->Type->Action == Action::None || useAITargeting)
						{
							EventClass pEvent(pCurrent->ArrayIndex, EventType::SPECIAL_PLACE, swIndex, CellStruct::Empty);
							EventClass::AddEvent(&pEvent);
						}
						else
						{
							DisplayClass::Instance->CurrentBuilding = nullptr;
							DisplayClass::Instance->CurrentBuildingType = nullptr;
							DisplayClass::Instance->unknown_11AC = static_cast<DWORD>(-1);
							//DisplayClass::Instance->SetActiveFoundation(nullptr);
							MapClass::Instance->SetRepairMode(0);
							DisplayClass::Instance->SetSellMode(0);
							DisplayClass::Instance->PowerToggleMode = false;
							DisplayClass::Instance->PlanningMode = false;
							DisplayClass::Instance->PlaceBeaconMode = false;
							DisplayClass::Instance->CurrentSWTypeIndex = swIndex;
							MapClass::Instance->UnselectAll();
							VoxClass::PlayIndex(pSWExt->EVA_SelectTarget);
						}
					}
				}
			}
			if (pSuper->CanFire())
			{
				MapClass::Instance->UnselectAll();

				if (!useAITargeting)
					Unsorted::CurrentSWType = pSuper->Type->ArrayIndex;
			}

			R->Stack(STACK_OFFSET(0x28, 0x8), NULL);
			R->EAX(Action::None);
			return 0x693276;
		}

		row++;

		if (row >= MaxSWPerRow - line)
		{
			row = 0;
			line++;

			if (MaxSWRow > 0 && line >= MaxSWRow)
			{
				break;
			}

			location_Y += cameoHeight / 2;
			location = { location.X + cameoWidth, location_Y };
		}
		else
		{
			location.Y += cameoHeight;
		}
	}

	return 0;
}

DEFINE_HOOK(0x4F4583, GScreenClass_DrawOnTop_TheDarkSideOfTheMoon, 0x6)
{
	const auto pCurrent = HouseClass::CurrentPlayer();
	if (!pCurrent || pCurrent->Defeated)
		return 0;

	std::vector<SuperClass*> grantedSupers;

	for (const auto pSuper : pCurrent->Supers)
	{
		auto pSWTypeExt = SWTypeExtContainer::Instance.Find(pSuper->Type);
		if (pSuper->Granted && pSWTypeExt->SW_ShowCameo)
			grantedSupers.emplace_back(pSuper);
	}

	if (grantedSupers.empty())
		return 0;

	std::sort(grantedSupers.begin(), grantedSupers.end(),
		[](SuperClass* a, SuperClass* b)
		{
			return BuildType::SortsBefore
			(
				AbstractType::Special,
				a->Type->ArrayIndex,
				AbstractType::Special,
				b->Type->ArrayIndex
			);
		});

	std::vector<SuperClass*> grantedSupersPage;
	if (MaxSWRow > 0 && MaxSWPerRow > 0)
	{
		size_t MaxCount = 0;
		int NowRow = MaxSWPerRow;
		for (int NowLine = 1; NowLine <= MaxSWRow && NowRow > 0; NowLine++)
		{
			MaxCount += NowRow * NowLine;
			NowRow--;
		}

		if (grantedSupers.size() <= MaxCount)
		{
			grantedSupersPage = grantedSupers;
		}
		else
		{
			int MaxPage = (grantedSupers.size() % MaxCount == 0) ? grantedSupers.size() / MaxCount : (grantedSupers.size() / MaxCount) + 1;
			if (CurrentSWBarPage > MaxPage)
			{
				CurrentSWBarPage = MaxPage;
			}

			size_t beginidx = (CurrentSWBarPage - 1) * MaxCount;

			for (size_t idx = 0; idx < grantedSupers.size(); idx++)
			{
				if (idx >= beginidx && idx < (beginidx + MaxCount))
				{
					grantedSupersPage.emplace_back(grantedSupers[idx]);
				}
			}
		}
	}
	else
	{
		grantedSupersPage = grantedSupers;
	}

	DSurface* pSurface = DSurface::Composite;
	BitFont* pFont = BitFont::Instance;
	RectangleStruct bounds = DSurface::ViewBounds();
	const Point2D crdCursor = { WWMouseClass::Instance->GetX(), WWMouseClass::Instance->GetY() };
	const int cameoWidth = 60;
	const int cameoHeight = 48;
	const int superCount = static_cast<int>(grantedSupersPage.size());
	Point2D location = { 0, (bounds.Height - std::min(superCount, MaxSWPerRow) * cameoHeight) / 2 };
	int location_Y = location.Y;
	RectangleStruct destRect = { 0, location.Y, cameoWidth, cameoHeight };
	Point2D tooptipLocation = { cameoWidth, 0 };
	const COLORREF tooltipColor = Drawing::RGB_To_Int(Drawing::TooltipColor());
	SuperClass* tooltipTypeExt = nullptr;
	int row = 0, line = 0;

	for (int idx = 0; idx < superCount && MaxSWPerRow - line > 0; idx++)
	{
		int frame = (line == 0) ? 0 : 1;

		if (SWBarSHP_Top && row == 0)
		{
			const auto pSHP = SWBarSHP_Top;
			Point2D loc = { location.X, location.Y - pSHP->Height };

			pSurface->DrawSHP
			(
				SWBarPalette,
				pSHP,
				frame,
				&loc,
				&bounds,
				BlitterFlags::bf_400,
				0,
				0,
				ZGradient::Ground,
				1000,
				0,
				nullptr,
				0,
				0,
				0
			);
		}

		const auto pSuper = grantedSupersPage[idx];

		if (const auto pSWExt = SWTypeExtContainer::Instance.Find(pSuper->Type))
		{
			// support for pcx cameos
			if (auto pPCX = pSWExt->SidebarPCX.GetSurface())
			{
				PCX::Instance->BlitToSurface(&destRect, pSurface, pPCX);
			}
			else if (auto pCameo = pSuper->Type->SidebarImage) // old shp cameos, fixed palette
			{
				auto pCameoRef = pCameo->AsReference();
				char pFilename[0x20];
				strcpy_s(pFilename, RulesExtData::Instance()->MissingCameo.data());
				_strlwr_s(pFilename);

				if (!_stricmp(pCameoRef->Filename, "xxicon.shp") && strstr(pFilename, ".pcx"))
				{
					PCX::Instance->LoadFile(pFilename);
					if (auto CameoPCX = PCX::Instance->GetSurface(pFilename))
						PCX::Instance->BlitToSurface(&destRect, pSurface, CameoPCX);
				}
				else
				{
					auto pConvert = pSWExt->SidebarPalette ? pSWExt->SidebarPalette->GetConvert<PaletteManager::Mode::Default>() : FileSystem::CAMEO_PAL();
					pSurface->DrawSHP
					(
						pConvert,
						pCameo,
						0,
						&location,
						&bounds,
						BlitterFlags::bf_400,
						0,
						0,
						ZGradient::Ground,
						1000,
						0,
						nullptr,
						0,
						0,
						0
					);
				}
			}
			else
				continue;

			if (SWBarSHP_Right)
			{
				const auto pSHP = SWBarSHP_Right;

				pSurface->DrawSHP
				(
					SWBarPalette,
					pSHP,
					0,
					&location,
					&bounds,
					BlitterFlags::bf_400,
					0,
					0,
					ZGradient::Ground,
					1000,
					0,
					nullptr,
					0,
					0,
					0
				);
			}

			if (pSuper->IsCharged && !pCurrent->CanTransactMoney(pSWExt->Money_Amount))
			{
				pSurface->DrawSHP
				(
					FileSystem::SIDEBAR_PAL,
					FileSystem::DARKEN_SHP,
					0,
					&location,
					&bounds,
					BlitterFlags::Darken | BlitterFlags::bf_400,
						0,
						0,
						ZGradient::Ground,
						1000,
						0,
						nullptr,
						0,
						0,
						0
				);
			}

			if (!tooltipTypeExt && crdCursor.X > location.X && crdCursor.X < location.X + cameoWidth &&
				crdCursor.Y > location.Y && crdCursor.Y < location.Y + cameoHeight)
			{
				tooltipTypeExt = pSuper;
				tooptipLocation = { location.X + cameoWidth, location.Y };

				RectangleStruct cameoRect = { location.X, location.Y, cameoWidth, cameoHeight };
				pSurface->Draw_Rect(cameoRect, tooltipColor);
			}

			if (!pSuper->RechargeTimer.Completed())
			{
				Point2D loc = { location.X, location.Y };
				pSurface->DrawSHP
				(
					FileSystem::SIDEBAR_PAL,
					FileSystem::GCLOCK2_SHP,
					pSuper->GetCameoChargeState() + 1,
					&loc,
					&bounds,
					BlitterFlags::bf_400 | BlitterFlags::TransLucent50,
						0,
						0,
						ZGradient::Ground,
						1000,
						0,
						nullptr,
						0,
						0,
						0
				);
			}

			const auto buffer = pSuper->NameReadiness();

			if (GeneralUtils::IsValidString(buffer))
			{
				Point2D textLoc = { location.X + cameoWidth / 2, location.Y };
				TextPrintType printType = TextPrintType::FullShadow | TextPrintType::Point8 | TextPrintType::Background | TextPrintType::Center;

				pSurface->DSurfaceDrawText(buffer, &bounds, &textLoc, tooltipColor, 0, printType);
			}

			row++;

			if (row >= MaxSWPerRow - line)
			{
				if (SWBarSHP_Bottom)
				{
					const auto pSHP = SWBarSHP_Bottom;
					Point2D loc = { location.X, location.Y + cameoHeight };

					pSurface->DrawSHP
					(
						SWBarPalette,
						pSHP,
						frame,
						&loc,
						&bounds,
						BlitterFlags::bf_400,
						0,
						0,
						ZGradient::Ground,
						1000,
						0,
						nullptr,
						0,
						0,
						0
					);
				}

				row = 0;
				line++;

				if (MaxSWRow > 0 && line >= MaxSWRow)
				{
					break;
				}

				location_Y += cameoHeight / 2;
				location = { location.X + cameoWidth, location_Y };
				destRect.X = location.X;
				destRect.Y = location.Y;
			}
			else
			{
				if (row == superCount)
				{
					if (SWBarSHP_Bottom)
					{
						const auto pSHP = SWBarSHP_Bottom;
						Point2D loc = { location.X, location.Y + cameoHeight };

						pSurface->DrawSHP
						(
							SWBarPalette,
							pSHP,
							frame,
							&loc,
							&bounds,
							BlitterFlags::bf_400,
						0,
						0,
						ZGradient::Ground,
						1000,
						0,
						nullptr,
						0,
						0,
						0
						);
					}
				}

				location.Y += cameoHeight;
				destRect.Y += cameoHeight;
			}
		}
	}

#pragma region Draw Tooltip
	if (tooltipTypeExt)
	{
		PhobosToolTip::Instance.HelpText(tooltipTypeExt);
		const wchar_t* pDesc = PhobosToolTip::Instance.GetBuffer();

		if (GeneralUtils::IsValidString(pDesc))
		{
			const int maxWidth = Phobos::UI::MaxToolTipWidth > 0 ? Phobos::UI::MaxToolTipWidth : bounds.Width;
			int width = 0, height = 0;

			auto GetTextRect = [&pFont, &width, &height, maxWidth](const wchar_t* text)
				{
					int nWidth = 0, nHeight = 0;
					pFont->GetTextDimension(text, &nWidth, &nHeight, maxWidth);

					width = std::max(width, nWidth);
					height += nHeight;
				};

			GetTextRect(pDesc);

			width += 8;
			height += 5;
			tooptipLocation.Y = std::clamp(tooptipLocation.Y, 0, bounds.Height - height + cameoHeight);
			RectangleStruct fillRect = { tooptipLocation.X, tooptipLocation.Y, width, height };
			ColorStruct fillColor = { 0,0,0 };
			int fillOpacity = 100;

			if (auto const pSide = SideClass::Array->GetItemOrDefault(ScenarioClass::Instance->PlayerSideIndex))
			{
				if (auto pSideExt = SideExtContainer::Instance.Find(pSide))
				{
					fillColor = pSideExt->ToolTip_Background_Color.Get(RulesExtData::Instance()->ToolTip_Background_Color);
					fillOpacity = pSideExt->ToolTip_Background_Opacity.Get(RulesExtData::Instance()->ToolTip_Background_Opacity);
				}
			}

			pSurface->Fill_Rect_Trans(&fillRect, &fillColor, fillOpacity);
			pSurface->Draw_Rect(fillRect, tooltipColor);

			tooptipLocation += { 4, 3 };
			const int textHeight = pFont->InternalPTR->FontHeight + 4;

			auto DrawTextBox = [&tooptipLocation, textHeight, tooltipColor](const wchar_t* pText)
				{
					wchar_t buffer[0x400];
					wcscpy_s(buffer, pText);
					wchar_t* context = nullptr, delims[4] = L"\n";
					DSurface* pSurface = DSurface::Composite;
					const TextPrintType printType = TextPrintType::FullShadow | TextPrintType::Point8;

					for (auto pCur = wcstok_s(buffer, delims, &context); pCur; pCur = wcstok_s(nullptr, delims, &context))
					{
						if (!wcslen(pCur))
							continue;

						pSurface->DSurfaceDrawText
						(
							pCur,
							&DSurface::ViewBounds.get(),
							&tooptipLocation,
							tooltipColor,
							0,
							printType
						);

						tooptipLocation.Y += textHeight;
					}
				};

			DrawTextBox(pDesc);
		}
	}
#pragma endregion

	return 0;
}
