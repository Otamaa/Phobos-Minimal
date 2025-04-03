#pragma once

#ifdef COMPILE_PORTED_DP_FEATURES_

#include "TextHandler.h"
#include <Drawing.h>

struct PrintTextManager
{
private:
	static Point2D fontSize;
	static std::queue<RollingText> rollingTextQueue;
public:

	static Point2D FontSize()
	{
		if (fontSize == Point2D::Empty)
		{
			std::wstring temp = L"0123456789+-*/%";
			RectangleStruct fontRect = Drawing::GetTextBox(temp.c_str(), 0, 0, 0, 0, 0);
			int x = fontRect.Width / 15;
			fontSize.X = x % 2 == 0 ? x : x + 1;
		}
		return fontSize;
	}

	static void InsertRollingText(const wchar_t* text, CoordStruct location, Point2D offset, int rollSpeed, int duration, PrintTextData data)
	{
		rollingTextQueue.emplace(text,location,offset,rollSpeed,duration,data);
	}

	static void PrintAllText()
	{
		for (size_t i = 0; i < rollingTextQueue.size(); i++)
		{
			RollingText rollingText = rollingTextQueue.front();
			rollingTextQueue.pop();
			Point2D offset;
			Point2D pos;
			RectangleStruct bound;

			if (rollingText.CanPrint(offset, pos, bound))
			{
				Point2D pos2 = pos + offset;
				PrintOutRollingText(rollingText.Text, rollingText.Data, pos2, &bound, DSurface::Temp(), false);
				rollingTextQueue.push(rollingText);
			}
		}
	}

	static void PrintOutRollingText(std::wstring text, PrintTextData data, Point2D pos, RectangleStruct* pBound, DSurface* pSurface, bool isBuilding)
	{
		if (data.UseSHP)
		{
			//std::wstring t = text;
			int zeroFrameIndex = data.ZeroFrameIndex;
			Point2D imageSize = data.ImageSize;
			int x = imageSize.X % 2 == 0 ? imageSize.X : imageSize.X + 1;
			int y = isBuilding ? x / 2 : 0;

			for(auto const& c: text)
			{
				int frameIndex = zeroFrameIndex;
				int frameOffset = 0;

				switch (c)
				{
				case '0':
					frameOffset = 0;
					break;
				case '1':
					frameOffset = 1;
					break;
				case '2':
					frameOffset = 2;
					break;
				case '3':
					frameOffset = 3;
					break;
				case '4':
					frameOffset = 4;
					break;
				case '5':
					frameOffset = 5;
					break;
				case '6':
					frameOffset = 6;
					break;
				case '7':
					frameOffset = 7;
					break;
				case '8':
					frameOffset = 8;
					break;
				case '9':
					frameOffset = 9;
					break;
				case '+ ':
					frameOffset = 10;
					break;
				case '-':
					frameOffset = 11;
					break;
				case '*':
					frameOffset = 12;
					break;
				case '/':
				case '|':
					frameOffset = 13;
					break;
				case '%':
					frameOffset = 14;
					break;
				}

				frameIndex += frameOffset;
				auto pSHP = FileSystem::PIPS_SHP();
				if (data.CustomSHP)
				{
					if(auto pCustomSHP = FileSystem::LoadSHPFile(data.SHPFileName))
						pSHP = pCustomSHP;
				}

				pSurface->DrawSHP(FileSystem::PALETTE_PAL(), pSHP, frameIndex, &pos, pBound, BlitterFlags::None, false, 0, 0, 0x3E8, 0, nullptr, 0, 0, 0);

				pos.X += x;
				pos.Y -= y;
			}
		}
		else
		{
			int textColor = data.Color.ToInit();
			int shadowColor = data.ShadowColor.ToInit();
			int x = FontSize().X;
			int y = isBuilding ? FontSize().X / 2 : 0;

			for (auto const& Item : text)
			{
				std::wstring Buff { Item };
				if (data.ShadowOffset != Point2D::Empty)
				{
					Point2D shadow = pos + data.ShadowOffset;
					pSurface->DSurfaceDrawText(Buff.c_str(), pBound, &shadow, shadowColor, 0, TextPrintType::NoShadow);
				}
				pSurface->DSurfaceDrawText(Buff.c_str(), pBound, &pos, textColor, 0, TextPrintType::NoShadow);

				pos.X += x;
				pos.Y -= y;
			}
		}
	}
};

#endif
