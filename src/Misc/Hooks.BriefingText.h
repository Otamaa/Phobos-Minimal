#pragma once
#include <Base/Always.h>
#include <BitFont.h>
#include <Surface.h>

DEFINE_REFERENCE(int, SpecialDialog, 0xA8EDA0u)

struct BriefingText
{
	static constexpr DWORD BriefingBodyCaller = 0x621144;
	static constexpr int MinBoxWidth = 400;
	static constexpr int MinBoxHeight = 100;
	static constexpr int SurfaceMargin = 8;
	static constexpr int SideMarginPercent = 80;
	static constexpr int TopMarginPercent = 80;

	// ------------------------------------------------------------------------
	//  Restores the font clip rect on scope exit (after UnLock).
	// ------------------------------------------------------------------------
	struct ClipGuard
	{
		BitFont* Font { nullptr };
		LTRBStruct Saved { };

		~ClipGuard()
		{
			if (this->Font)
				this->Font->Bounds = this->Saved;
		}

		void OpenToSurface(BitFont* font, int width, int height)
		{
			this->Font = font;
			this->Saved = font->Bounds;

			font->Bounds.Left = 0;
			font->Bounds.Top = 0;
			font->Bounds.Right = width - 1;
			font->Bounds.Bottom = height - 1;
		}
	};

	// ------------------------------------------------------------------------
	//  Helpers
	// ------------------------------------------------------------------------
	static bool IsSaneSurface(int w, int h)
	{
		return w >= 64 && w <= 8192 && h >= 64 && h <= 8192;
	}

	// Main area = everything left of the option menu; vanilla box is centred in it.
	static void ApplyDialogLayout(int boxWidth, int surfaceWidth,
								  int& posX, int& posY, int& wrapWidth)
	{
		const int mainWidth = std::min(posX * 2 + boxWidth, surfaceWidth);
		if (mainWidth <= 0)
			return;

		const int paddingX = posX * SideMarginPercent / 100;

		posX = paddingX;
		posY = posY * TopMarginPercent / 100;
		wrapWidth = mainWidth - paddingX * 2;
	}

	// ------------------------------------------------------------------------
	//  Entry point: adjusts the by-value params in place. Returns true if applied.
	// ------------------------------------------------------------------------
	static bool TryApply(DWORD caller, BitFont* font, Surface* surface, const wchar_t* text,
						 int& posX, int& posY, int& boxWidth, int& boxHeight,
						 int& fadeStart, ClipGuard& clip)
	{
		if (caller != BriefingBodyCaller)
			return false;

		if (!font || !surface || !text)
			return false;

		if (boxWidth < MinBoxWidth || boxHeight < MinBoxHeight)
			return false;

		const int surfaceWidth = surface->Get_Width();
		const int surfaceHeight = surface->Get_Height();

		if (!IsSaneSurface(surfaceWidth, surfaceHeight))
			return false;

		int wrapWidth = surfaceWidth - posX - SurfaceMargin;

		if (SpecialDialog() == 9)
			ApplyDialogLayout(boxWidth, surfaceWidth, posX, posY, wrapWidth);

		wrapWidth = std::clamp(wrapWidth, boxWidth, surfaceWidth);
		// (clamp lo=boxWidth wins, matching the original: never narrower than vanilla)

		const int maxHeight = std::max(surfaceHeight - posY, boxHeight);

		boxWidth = wrapWidth;
		boxHeight = maxHeight;
		fadeStart = 0;

		clip.OpenToSurface(font, surfaceWidth, surfaceHeight);
		return true;
	}
};