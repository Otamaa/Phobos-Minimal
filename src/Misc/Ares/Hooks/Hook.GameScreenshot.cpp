#include "Header.h"

#include <CommandClass.h>

#include <Utilities/Macro.h>

//this is still 0.A code , need check the new one ,..
void __fastcall ScreenCaptureCommandClass_Process(CommandClass* pThis , DWORD)
{
	RECT Viewport = {};
	if (Imports::GetWindowRect.invoke()(Game::hWnd, &Viewport))
	{
		POINT TL = { Viewport.left, Viewport.top }, BR = { Viewport.right, Viewport.bottom };
		if (Imports::ClientToScreen.invoke()(Game::hWnd, &TL) && Imports::ClientToScreen.invoke()(Game::hWnd, &BR))
		{
			RectangleStruct ClipRect = { TL.x, TL.y, Viewport.right + 1, Viewport.bottom + 1 };

			DSurface* Surface = DSurface::Primary;

			int width = Surface->Get_Width();
			int height = Surface->Get_Height();

			size_t arrayLen = width * height;

			if (width < ClipRect.Width)
			{
				ClipRect.Width = width;
			}

			if (height < ClipRect.Height)
			{
				ClipRect.Height = height;
			}

			WWMouseClass::Instance->HideCursor();

			if (WORD* buffer = reinterpret_cast<WORD*>(Surface->Lock(0, 0)))
			{
				//char fName[0x80];
				const std::string fName = "SCRN." + Debug::GetCurTimeA() + ".BMP";
				CCFileClass ScreenShot { fName.c_str()};
				ScreenShot.Open(FileAccessMode::Write);

#pragma pack(push, 1)
				struct bmpfile_full_header
				{
					unsigned char magic[2];
					DWORD filesz;
					WORD creator1;
					WORD creator2;
					DWORD bmp_offset;
					DWORD header_sz;
					DWORD width;
					DWORD height;
					WORD nplanes;
					WORD bitspp;
					DWORD compress_type;
					DWORD bmp_bytesz;
					DWORD hres;
					DWORD vres;
					DWORD ncolors;
					DWORD nimpcolors;
					DWORD R; //
					DWORD G; //
					DWORD B; //
				} h {};
#pragma pack(pop)

				h.magic[0] = 'B';
				h.magic[1] = 'M';

				h.creator1 = h.creator2 = 0;

				h.header_sz = 40;
				h.width = width;
				h.height = -height; // magic! no need to reverse rows this way
				h.nplanes = 1;
				h.bitspp = 16;
				h.compress_type = BI_BITFIELDS;
				h.bmp_bytesz = arrayLen * 2;
				h.hres = 4000;
				h.vres = 4000;
				h.ncolors = h.nimpcolors = 0;

				h.R = 0xF800;
				h.G = 0x07E0;
				h.B = 0x001F; // look familiar?

				h.bmp_offset = sizeof(h);
				h.filesz = h.bmp_offset + h.bmp_bytesz;

				ScreenShot.WriteBytes(&h, sizeof(h));
				std::vector<WORD> _pixelData(arrayLen);
				WORD* pixels = _pixelData.data();
				int pitch = Surface->VideoSurfaceDescription->lPitch;
				for (int r = 0; r < height; ++r)
				{
					memcpy(pixels, reinterpret_cast<void*>(buffer), width * 2);
					pixels += width;
					buffer += pitch / 2; // /2 because buffer is a WORD * and pitch is in bytes
				}

				ScreenShot.WriteBytes(_pixelData.data(), arrayLen * 2);
				ScreenShot.Close();
				Debug::LogInfo("Wrote screenshot to file {}", fName);
				Surface->Unlock();
			}

			WWMouseClass::Instance->ShowCursor();
		}
	}
}
DEFINE_FUNCTION_JUMP(VTABLE, 0x7EBF24 , ScreenCaptureCommandClass_Process);
