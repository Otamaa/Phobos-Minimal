#include "SelectedColumnClass.h"
#include "SelectedInfoClass.h"

#include <GameOptionsClass.h>
#include <FPSCounter.h>
#include <PCX.h>
#include <VocClass.h>

#include <Ext/Side/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>

SelectedColumnClass::SelectedColumnClass(int x, int y, int width, int height)
	: GadgetClass(x, y, width, height, static_cast<GadgetFlag>(0), false)
{
	this->Disabled = !Phobos::Config::SelectedDisplay_Enable || !SelectedInfoClass::Instance.SingleSelect || !SelectedInfoClass::Instance.ObtainSelect;
}

bool SelectedColumnClass::Draw(bool forced)
{
	if (!ScenarioClass::Instance->UserInputLocked)
		SelectedInfoClass::Instance.DrawInfo();

	return true;
}

void SelectedColumnClass::OnMouseEnter()
{
	SelectedInfoClass::Instance.IsHovering = true;
	MouseClass::Instance->UpdateCursor(MouseCursorType::Default, false);
}

void SelectedColumnClass::OnMouseLeave()
{
	SelectedInfoClass::Instance.IsHovering = false;
	MouseClass::Instance->UpdateCursor(MouseCursorType::Default, false);
}

void SelectedColumnClass::DrawInfo() const
{
	const auto pSideExt = SideExtContainer::Instance.Find(SideClass::Array->Items[ScenarioClass::Instance->PlayerSideIndex]);
	auto position = Point2D { this->Rect.X, this->Rect.Y };
	auto surfaceRect = RectangleStruct { 0, 0, this->Rect.X + this->Rect.Width, this->Rect.Y + this->Rect.Height };

	if (const auto pMainSHP = pSideExt->SelectedInfo_Main.Get((SHPCaches*)SelectedInfoClass::SelectedInfo_Main)) {
		DSurface::Composite->DrawSHP(pSideExt->SelectedInfo_Palette.GetOrDefaultConvert(FileSystem::ANIM_PAL),
			pMainSHP, 0, &position, &surfaceRect, BlitterFlags::bf_400, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}

	const auto pThis = SelectedInfoClass::Instance.CurrentSelectTechno[0];
	const auto pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pTypeExt = pExt->TypeExtData;
	const auto pOwner = pThis->Owner;

	auto getDisplayType = [&]() -> ObjectTypeClass*
		{
			if (!pOwner->IsAlliedWith(HouseClass::CurrentPlayer()) && !HouseClass::IsCurrentPlayerObserver())
			{
				if (pThis->IsDisguisedAs(HouseClass::CurrentPlayer()))
				{
					if (const auto pDisguiseTypeExt = TechnoTypeExtContainer::Instance.TryFind((TechnoTypeClass*)pThis->Disguise))
					{
						if (const auto pFakeType = pDisguiseTypeExt->Fake_Of.Get())
							return pFakeType;
					}

					return pThis->Disguise;
				}

				if (const auto pFakeType = pTypeExt->Fake_Of.Get())
					return pFakeType;
			}

			return pTypeExt->This();
		};

	const auto pDisplayType = getDisplayType();
	const auto pDisplayTypeExt = TechnoTypeExtContainer::Instance.TryFind((TechnoTypeClass*)pDisplayType);

	TextPrintType printType = TextPrintType::Center | TextPrintType::Point8;
	COLORREF color = Drawing::TooltipColor->ToInit();
	position += Point2D { 126, 5 };

	if (const auto name = (pDisplayTypeExt && !pDisplayTypeExt->EnemyUIName.Get().empty() && !pOwner->IsAlliedWith(HouseClass::CurrentPlayer()))
		? pDisplayTypeExt->EnemyUIName.Get().Text : pDisplayType->UIName)
	{
		size_t length = MinImpl(wcslen(name), static_cast<size_t>(31));

		for (auto check = name; *check != L'\0'; ++check)
		{
			if (*check == L'\n')
			{
				length = static_cast<size_t>(check - name);
				break;
			}
		}

		wchar_t text[0x20] = { 0 };
		wcsncpy_s(text, name, length);
		text[length] = L'\0';

		// two ways of handling names too long for the available gap.
		// Flip this and rebuild to compare; delete the losing branch once decided.
		enum class LongNameMode { Truncate, Scroll };
		constexpr LongNameMode ActiveLongNameMode = LongNameMode::Truncate;

		// VERIFY: gap between cameo right edge (~66) and info-icon column (~179),
		// centered on the existing anchor at Rect.X + 126. Tune if it still clips
		// against the cameo/icons in-game.
		constexpr int maxWidth = 104;
		constexpr int msPerChar = 140; // wall-clock ms per character step (Scroll mode)
		constexpr int pauseMs = 900;   // hold at each end, in real ms (Scroll mode)

		const auto textBox = Drawing::GetTextBox(text, position, { 3, 2 });

		if (textBox.Width <= maxWidth)
		{
			// Fits fine - draw centered as before, drop any stale cache.
			this->NameScroll_Cache.clear();
			this->NameScroll_CumulativeWidths.clear();
			this->NameScroll_MaxOffset = 0;

			DSurface::Composite->DSurfaceDrawText(text, &surfaceRect, &position, color, 0, printType);
		}
		else
		{
			const bool textChanged = this->NameScroll_Cache != text;

			if (textChanged)
			{
				// Selection/name changed - rebuild the per-character width cache
				// once, so per-frame work below never touches GetTextBox.
				this->NameScroll_Cache = text;
				this->NameScroll_CumulativeWidths.assign(length + 1, 0);

				for (size_t i = 1; i <= length; ++i)
				{
					wchar_t prefix[0x20] = { 0 };
					wcsncpy_s(prefix, text, i);
					prefix[i] = L'\0';
					this->NameScroll_CumulativeWidths[i] = Drawing::GetTextBox(prefix, position, { 3, 2 }).Width;
				}

				this->NameScroll_MaxOffset = 0;

				for (int start = static_cast<int>(length) - 1; start >= 0; --start)
				{
					const int suffixWidth = this->NameScroll_CumulativeWidths[length] - this->NameScroll_CumulativeWidths[start];

					if (suffixWidth > maxWidth)
					{
						this->NameScroll_MaxOffset = start + 1;
						break;
					}
				}

				this->NameScroll_StartTime = SystemTimer::GetTime();
			}

			if constexpr (ActiveLongNameMode == LongNameMode::Truncate)
			{
				// SUSPECT: "..." (ASCII) rather than a Unicode ellipsis glyph -
				// legacy bitmap fonts in this engine aren't guaranteed to have
				// U+2026 mapped, ASCII dots are the safe bet.
				const auto ellipsisWidth = Drawing::GetTextBox(L"...", position, { 3, 2 }).Width;

				int keep = 0;

				while (keep < static_cast<int>(length)
					&& (this->NameScroll_CumulativeWidths[keep + 1] + ellipsisWidth) <= maxWidth)
				{
					++keep;
				}

				const std::wstring truncated = std::wstring(text, text + keep) + L"...";

				auto leftPrintType = printType;
				leftPrintType &= ~TextPrintType::Center;

				Point2D leftPosition { position.X - (maxWidth / 2), position.Y };
				DSurface::Composite->DSurfaceDrawText(truncated.c_str(), &surfaceRect, &leftPosition, color, 0, leftPrintType);
			}
			else
			{
				int charOffset = 0;

				if (this->NameScroll_MaxOffset > 0)
				{
					// wall-clock driven, NOT Unsorted::CurrentFrame() -
					// this engine's logic tick rate isn't independent of render
					// throughput (uncapped FPS runs the sim faster too, see the
					// SelectedIngameTimer/FPS block further down this file), so a
					// frame-count-based animation inherits that coupling. Real
					// elapsed ms is the only source immune to it.
					const int travelMs = this->NameScroll_MaxOffset * msPerChar;
					const int cycleLengthMs = (travelMs + pauseMs) * 2;
					const int elapsed = SystemTimer::GetTime() - this->NameScroll_StartTime;
					const int t = ((elapsed % cycleLengthMs) + cycleLengthMs) % cycleLengthMs;

					if (t < pauseMs)
						charOffset = 0;
					else if (t < pauseMs + travelMs)
						charOffset = (t - pauseMs) / msPerChar;
					else if (t < pauseMs + travelMs + pauseMs)
						charOffset = this->NameScroll_MaxOffset;
					else
						charOffset = this->NameScroll_MaxOffset - ((t - pauseMs - travelMs - pauseMs) / msPerChar);
				}

				// Grow the visible window from charOffset using the cached widths -
				// pure array lookups, no GetTextBox calls at render time.
				int endIdx = charOffset;

				while (endIdx < static_cast<int>(length)
					&& (this->NameScroll_CumulativeWidths[endIdx + 1] - this->NameScroll_CumulativeWidths[charOffset]) <= maxWidth)
				{
					++endIdx;
				}

				const std::wstring visible(text + charOffset, text + endIdx);

				// Left-align within the box; the string is already pre-trimmed to
				// fit, so containment doesn't depend on engine-side clip-rect behavior.
				auto scrollPrintType = printType;
				scrollPrintType &= ~TextPrintType::Center;

				Point2D scrollPosition { position.X - (maxWidth / 2), position.Y };
				DSurface::Composite->DSurfaceDrawText(visible.c_str(), &surfaceRect, &scrollPosition, color, 0, scrollPrintType);
			}
		}
	}

	position.Y += 18;
	{
		int value = -1, maxValue = 0;
		const auto infoType = pDisplayTypeExt ? pDisplayTypeExt->SelectedInfo_UpperType.Get() : DisplayInfoType::Shield;
		const auto infoIndex = pDisplayTypeExt ? pDisplayTypeExt->SelectedInfo_UpperIndex.Get() : 0;
		SelectedInfoClass::GetValuesForDisplay(pThis, pDisplayType, infoType, value, maxValue, infoIndex);

		const bool valid = value >= 0 && maxValue > 0;
		const auto ratio = valid ? static_cast<double>(value) / maxValue : 1.0;

		if (valid && pDisplayTypeExt)
		{
			const auto divisor = pDisplayTypeExt->SelectedInfo_UpperDivisor.Get();

			if (divisor > 1)
			{
				value = MaxImpl(value / divisor, value ? 1 : 0);
				maxValue = MaxImpl(maxValue / divisor, 1);
			}
		}

		if (!pDisplayTypeExt || pDisplayTypeExt->SelectedInfo_UpperColor.Get() == ColorStruct { 0, 0, 0 })
			color = (ratio > RulesClass::Instance->ConditionYellow) ? 0x67EC : (ratio > RulesClass::Instance->ConditionRed ? 0xFFEC : 0xF986);
		else
			color = pDisplayTypeExt->SelectedInfo_UpperColor->ToInit();

		wchar_t text[0x20] = { 0 };

		position.X += 6;
		printType &= ~TextPrintType::Center;

		if (valid)
			swprintf_s(text, L"%d", maxValue);
		else
			swprintf_s(text, L"--");

		DSurface::Composite->DSurfaceDrawText(text, &surfaceRect, &position, color, 0, printType);

		position.X -= 12;
		printType |= TextPrintType::Right;

		if (valid)
			swprintf_s(text, L"%d", value);
		else
			swprintf_s(text, L"--");

		DSurface::Composite->DSurfaceDrawText(text, &surfaceRect, &position, color, 0, printType);

		position.X += 6;
		printType &= ~TextPrintType::Right;
		printType |= TextPrintType::Center;
		DSurface::Composite->DSurfaceDrawText(L"/", &surfaceRect, &position, color, 0, printType);
	}

	position.Y += 14;
	{
		int value = -1, maxValue = 0;
		const auto infoType = pDisplayTypeExt ? pDisplayTypeExt->SelectedInfo_BelowType.Get() : DisplayInfoType::Health;
		const auto infoIndex = pDisplayTypeExt ? pDisplayTypeExt->SelectedInfo_BelowIndex.Get() : 0;
		SelectedInfoClass::GetValuesForDisplay(pThis, pDisplayType, infoType, value, maxValue, infoIndex);

		const bool valid = value >= 0 && maxValue > 0;
		const auto ratio = valid ? static_cast<double>(value) / maxValue : 1.0;

		if (valid && pDisplayTypeExt)
		{
			const auto divisor = pDisplayTypeExt->SelectedInfo_BelowDivisor.Get();

			if (divisor > 1)
			{
				value = MaxImpl(value / divisor, value ? 1 : 0);
				maxValue = MaxImpl(maxValue / divisor, 1);
			}
		}

		if (!pDisplayTypeExt || pDisplayTypeExt->SelectedInfo_BelowColor.Get() == ColorStruct { 0, 0, 0 })
			color = (ratio > RulesClass::Instance->ConditionYellow) ? 0x67EC : (ratio > RulesClass::Instance->ConditionRed ? 0xFFEC : 0xF986);
		else
			color = pDisplayTypeExt->SelectedInfo_BelowColor->ToInit();

		wchar_t text[0x20] = { 0 };

		position.X += 6;
		printType &= ~TextPrintType::Center;

		if (valid)
			swprintf_s(text, L"%d", maxValue);
		else
			swprintf_s(text, L"--");

		DSurface::Composite->DSurfaceDrawText(text, &surfaceRect, &position, color, 0, printType);

		position.X -= 12;
		printType |= TextPrintType::Right;

		if (valid)
			swprintf_s(text, L"%d", value);
		else
			swprintf_s(text, L"--");

		DSurface::Composite->DSurfaceDrawText(text, &surfaceRect, &position, color, 0, printType);

		position.X += 6;
		printType &= ~TextPrintType::Right;
		printType |= TextPrintType::Center;
		DSurface::Composite->DSurfaceDrawText(L"/", &surfaceRect, &position, color, 0, printType);
	}

	{
		int value = -1, maxValue = 0;
		const auto infoType = pDisplayTypeExt ? pDisplayTypeExt->SelectedInfo_CameoType.Get() : DisplayInfoType::Ammo;
		const auto infoIndex = pDisplayTypeExt ? pDisplayTypeExt->SelectedInfo_CameoIndex.Get() : 0;
		SelectedInfoClass::GetValuesForDisplay(pThis, pDisplayType, infoType, value, maxValue, infoIndex);

		auto drawRect = RectangleStruct { 10, position.Y + 24, static_cast<int>(180 * ((value <= -1 || maxValue <= 0) ? 1.0 : (static_cast<double>(value) / maxValue)) + 0.5), 15 };
		ColorStruct drawColor { 255, 255, 255 };
		DSurface::Composite->Fill_Rect_Trans(&drawRect, &drawColor, 25);
	}

	position += Point2D { -20, 22 };
	{
		const auto status = static_cast<int>(SelectedInfoClass::GetCurrentStatus(pThis));
		const auto text = GeneralUtils::LoadStringUnlessMissing(SelectedInfoClass::StatusEntry[status], SelectedInfoClass::Status[status]);
		DSurface::Composite->DSurfaceDrawText(text, &surfaceRect, &position, COLOR_WHITE, 0, printType);
	}

	const auto pMainCameo = SelectedInfoClass::Instance.MainCameo;

	if (!pMainCameo)
		return;

	if (const auto pCameoPCX = pDisplayTypeExt ? pDisplayTypeExt->CameoPCX.GetSurface() : nullptr)
	{
		auto drawRect = RectangleStruct { pMainCameo->Rect.X, pMainCameo->Rect.Y, pMainCameo->Rect.Width, pMainCameo->Rect.Height };
		PCXImages::Instance->BlitToSurface(&drawRect, DSurface::Composite, pCameoPCX);
	}
	else if (const auto pSHP = pDisplayType->GetCameo())
	{
		if (const auto MissingCameoPCX = SelectedInfoClass::SearchMissingCameo(pDisplayType->WhatAmI(), pSHP))
		{
			auto drawRect = RectangleStruct { pMainCameo->Rect.X, pMainCameo->Rect.Y, pMainCameo->Rect.Width, pMainCameo->Rect.Height };
			PCXImages::Instance->BlitToSurface(&drawRect, DSurface::Composite, MissingCameoPCX);
		}
		else
		{
			position = Point2D { pMainCameo->Rect.X, pMainCameo->Rect.Y };
			const auto cameoRect = RectangleStruct { 0, 0, pMainCameo->Rect.X + pMainCameo->Rect.Width, pMainCameo->Rect.Y + pMainCameo->Rect.Height };
			const auto pPal = pDisplayTypeExt ? pDisplayTypeExt->CameoPal.GetOrDefaultConvert(FileSystem::CAMEO_PAL) : FileSystem::CAMEO_PAL;
			DSurface::Composite->DrawSHP(pPal, pSHP, 0, &position, &cameoRect, BlitterFlags::bf_400, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
		}
	}

	if (pMainCameo->Hovering && pDisplayTypeExt && Phobos::Config::ToolTipDescriptions)
	{
		const auto csf = pDisplayTypeExt->UIDescription_HoveredInfo.Get((pDisplayTypeExt->UIDescription.Get()));

		if (!csf.empty())
		{
			const auto description = csf.Text;
			auto originalLocation = Point2D { pMainCameo->Rect.X, pMainCameo->Rect.Y };
			std::vector<std::wstring> lines;
			std::wstring descStr(description);
			size_t pos = 0;

			while (pos < descStr.size())
			{
				size_t next = descStr.find(L'\n', pos);
				std::wstring line;

				if (next == std::wstring::npos)
				{
					line = descStr.substr(pos);
					pos = descStr.size();
				}
				else
				{
					line = descStr.substr(pos, next - pos);
					pos = next + 1;
				}

				if (!line.empty())
					lines.push_back(line);
			}

			if (!lines.empty())
			{
				int maxWidth = 0;
				int lineHeight = 0;

				for (const auto& line : lines)
				{
					RectangleStruct rect = Drawing::GetTextBox(line.c_str(), originalLocation, { 3 ,2 });

					if (rect.Width > maxWidth)
						maxWidth = rect.Width;

					if (!lineHeight)
						lineHeight = rect.Height + 1;
				}

				const int totalHeight = lineHeight * lines.size();
				auto textLocation = originalLocation + Point2D { 4, -(totalHeight + 5) };
				textLocation.Y = MaxImpl(textLocation.Y, 2);

				RectangleStruct textRect { originalLocation.X, (textLocation.Y - 2), (maxWidth + 8), totalHeight };
				ColorStruct bgColor { 0, 0, 0 };
				DSurface::Composite->Fill_Rect_Trans(&textRect, &bgColor, 40);
				DSurface::Composite->Draw_Rect(textRect, COLOR_WHITE);

				for (size_t i = 0; i < lines.size(); ++i)
				{
					Point2D linePos = textLocation;
					linePos.Y += i * lineHeight;
					DSurface::Composite->DSurfaceDrawText(lines[i].c_str(), &linePos, COLOR_WHITE);
				}
			}
		}
	}
}

// ----------------------------------------

SelectedBottomClass::SelectedBottomClass(int x, int y, int width, int height)
	: GadgetClass(x, y, width, height, static_cast<GadgetFlag>(0), false)
{
	this->Disabled = !Phobos::Config::SelectedDisplay_Enable;
}

bool SelectedBottomClass::Draw(bool forced)
{
	return false;
}

void SelectedBottomClass::OnMouseEnter()
{
	SelectedInfoClass::Instance.IsHovering = true;
	MouseClass::Instance->UpdateCursor(MouseCursorType::Default, false);
}

void SelectedBottomClass::OnMouseLeave()
{
	SelectedInfoClass::Instance.IsHovering = false;
	MouseClass::Instance->UpdateCursor(MouseCursorType::Default, false);
}

void SelectedBottomClass::DrawInfo() const
{
	const auto pSideExt = SideExtContainer::Instance.Find(SideClass::Array->Items[ScenarioClass::Instance->PlayerSideIndex]);
	auto rect = RectangleStruct { 0, 0, this->Rect.X + this->Rect.Width, this->Rect.Y + this->Rect.Height };
	const auto pSHP = pSideExt->SelectedInfo_Bottom.Get((SHPCaches*)SelectedInfoClass::SelectedInfo_Bottom);

	if (pSHP && pSHP->GetFrameCount() >= 3)
	{
		const auto position = Point2D { this->Rect.X, this->Rect.Y };
		const auto frame = Phobos::Config::SelectedDisplay_Enable ? (SelectedInfoClass::Instance.SingleSelect ? 1 : 2) : 0;
		DSurface::Composite->DrawSHP(pSideExt->SelectedInfo_Palette.GetOrDefaultConvert(FileSystem::ANIM_PAL),
			pSHP, frame, &position, &rect, BlitterFlags::bf_400, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}

	if (!Phobos::Config::SelectedDisplay_Enable)
		return;

	TextPrintType printType = TextPrintType::Center | TextPrintType::Point8;
	auto location = Point2D { this->Rect.X + 38, this->Rect.Y + 4 };

	if (FakeRulesClass::Instance()->FPSCounter != FPSCounterMode::disabled)
	{
		const auto fps = FPSCounter::CurrentFrameRate();
		const auto gameSpeed = GameOptionsClass::Instance->GameSpeed;
		COLORREF color = 0x67EC;

		if (!gameSpeed || fps < static_cast<unsigned int>(60 / gameSpeed))
		{
			if (fps < 10)
				color = 0xF986;
			else if (fps < 20)
				color = 0xFC05;
			else if (fps < 30)
				color = 0xFCE5;
			else if (fps < 45)
				color = 0xFFEC;
			else if (fps < 60)
				color = 0x9FEC;
		}

		{
			wchar_t buffer[0x20];
			swprintf_s(buffer, L"FPS: %u", fps);
			DSurface::Composite->DSurfaceDrawText(buffer, &rect, &location, color, 0, printType);
		}

		location.X += 86;
		{
			wchar_t buffer[0x20];
			swprintf_s(buffer, L"AVG: %.2lf", FPSCounter::GetAverageFrameRate());
			DSurface::Composite->DSurfaceDrawText(buffer, &rect, &location, COLOR_WHITE, 0, printType);
		}

		location.X += 80;
	}

	{
		auto second = 0;

		if (FakeRulesClass::Instance->SelectedIngameTimer)
		{
			second = Unsorted::CurrentFrame() / 15;
		}
		else
		{
			const auto& timer = ScenarioClass::Instance->ElapsedTimer;
			auto time = timer.TimeLeft;

			if (timer.StartTime != -1)
				time += SystemTimer::GetTime() - timer.StartTime;

			second = time / 60;
		}

		const auto minute = second / 60;

		wchar_t buffer[0x20];

		if (const auto hour = minute / 60)
			swprintf_s(buffer, L"%d:%02d:%02d", hour, minute % 60, second % 60);
		else
			swprintf_s(buffer, L"%02d:%02d", minute % 60, second % 60);

		DSurface::Composite->DSurfaceDrawText(buffer, &rect, &location, COLOR_WHITE, 0, printType);
	}
}