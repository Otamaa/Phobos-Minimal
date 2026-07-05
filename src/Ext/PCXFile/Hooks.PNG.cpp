#include <PCX.h>
#include <Utilities/Patch.h>
#include <Utilities/Macro.h>
#include <Utilities/Debug.h>

#include <CCFileClass.h>
#include <ScenarioClass.h>
#include <FileSystem.h>
#include <ConvertClass.h>

#include <Drawing.h>

#include <Lib/lodepng/lodepng.h>

#pragma pack(push, 1)
struct PCX_HEADER
{
	uint8_t  Id;               // 0x0A = PCX
	uint8_t  Version;          // 5 = PCX 3.0
	uint8_t  Encoding;         // 1 = RLE
	uint8_t  BitsPixelPlane;   // bits per pixel per plane (8)
	uint16_t Xmin, Ymin;
	uint16_t Xmax, Ymax;
	uint16_t HRes, VRes;
	uint8_t  ColorMap[48];
	uint8_t  Reserved;
	uint8_t  NumberOfPlanes;
	uint16_t BytesLinePlane;
	uint16_t PaletteType;
	uint8_t  Filler[58];       // pad to 128 bytes
};
#pragma pack(pop)

static_assert(sizeof(PCX_HEADER) == 128, "PCX_HEADER must be 128 bytes");

static constexpr int PCX_POOL_SIZE = 2048;

inline void RefillPool(FileClass* file, char* pool, char*& file_ptr) {
	file->Read(pool, PCX_POOL_SIZE);
	file_ptr = pool;
}

inline char ConsumeByte(FileClass* file, char* pool, const char* poolEnd, char*& file_ptr) {
	char byte = *file_ptr++;
	if (file_ptr >= poolEnd)
		RefillPool(file, pool, file_ptr);
	return byte;
}

BSurface* _Read_PCX_File(FileClass* file, unsigned char* palette, char* buffer, int buf_size)
{
	if (!file->IsAvaible())
		return nullptr;

	file->Open1(FileAccessMode::Read);

	// -----------------------------------------------------------------------
	// Header
	// -----------------------------------------------------------------------
	PCX_HEADER header;
	file->Read(&header, sizeof(PCX_HEADER));

	if (header.Id != 10 || header.Version != 5 || header.BitsPixelPlane != 8)
		return nullptr;

	const unsigned int planesize = header.NumberOfPlanes * header.BytesLinePlane;
	const unsigned int width = static_cast<unsigned int>(header.Xmax - header.Xmin + 1);
	int                height = header.Ymax - header.Ymin + 1;
	// 1 plane -> 8-bit paletted (1 byte/px), 3 planes -> 16-bit RGB (2 bytes/px)
	const int          bytesPerPix = (header.NumberOfPlanes != 1) ? 2 : 1;

	// -----------------------------------------------------------------------
	// Surface allocation
	// -----------------------------------------------------------------------
	BSurface* pic = nullptr;

	if (buffer)
	{
		// Clamp height to what the caller-supplied buffer can hold.
		const int maxHeight = (buf_size / static_cast<int>(width)) - 1;
		if (maxHeight < height)
			height = maxHeight;

		MemoryBuffer v65(buffer, buf_size);
		pic = GameCreate<BSurface>(width, height, bytesPerPix , v65);
	}
	else {
		pic = GameCreate<BSurface>(width, height, bytesPerPix);
	}

	if (!pic)
		return nullptr;

	// -----------------------------------------------------------------------
	// Lock and decode
	// -----------------------------------------------------------------------
	char* cbuf = static_cast<char*>(pic->Lock());

	if (cbuf) {
		char        pool[PCX_POOL_SIZE];
		char* file_ptr = pool;
		const char* poolEnd = pool + PCX_POOL_SIZE;

		file->Read(pool, PCX_POOL_SIZE);

		// -------------------------------------------------------------------
		// Path A: 3-plane RGB -> 16-bit surface
		// -------------------------------------------------------------------
		if (header.NumberOfPlanes == 3) {

			BYTE* linebuffer = static_cast<BYTE*>(YRMemory::Allocate(planesize));
			if (!linebuffer)
			{
				pic->Unlock();
				GameDelete(pic);
				return nullptr;
			}

			auto* sbuf = reinterpret_cast<int16_t*>(cbuf);

			for (int y = 0; y < height; ++y) {
				// Decode one full scanline (all 3 planes) into linebuffer.
				unsigned int x_1 = 0;
				while (x_1 < planesize) {
					const char rle = ConsumeByte(file, pool, poolEnd, file_ptr);

					if ((rle & 0xC0) == 0xC0) {
						const unsigned int run = static_cast<unsigned char>(rle & 0x3F);
						const char         fill = ConsumeByte(file, pool, poolEnd, file_ptr);
						for (unsigned int r = 0; r < run; ++r)
							linebuffer[x_1++] = fill;
					} else {
						linebuffer[x_1++] = rle;
					}
				}

				// Pack planar R/G/B rows into 16-bit RGB pixels.
				const BYTE* rPlane = &linebuffer[0];
				const BYTE* gPlane = &linebuffer[header.BytesLinePlane];
				const BYTE* bPlane = &linebuffer[2 * header.BytesLinePlane];

				for (unsigned int x = 0; x < width; ++x)
				{
					const int r = rPlane[x];
					const int g = gPlane[x];
					const int b = bPlane[x] >> Drawing::BlueShiftRight();
					*sbuf++ = static_cast<int16_t>(
						(r >> Drawing::RedShiftRight() << Drawing::RedShiftLeft()) |
						(g >> Drawing::GreenShiftRight() << Drawing::GreenShiftLeft()) |
						(b << Drawing::BlueShiftLeft()));
				}
			}

			operator delete(linebuffer);
			pic->Unlock();
		}
		// -------------------------------------------------------------------
		// Path B: 1-plane paletted -> 8-bit surface
		// -------------------------------------------------------------------
		else
		{
			if (header.BytesLinePlane == static_cast<int>(width))
			{
				// -------------------------------------------------------------
				// B1: BytesLinePlane == width — no per-row padding, flat decode
				// -------------------------------------------------------------
				const unsigned int buffersize = width * static_cast<unsigned int>(height);
				unsigned int index = 0;

				while (index < buffersize)
				{
					const char          pcx_byte = *file_ptr;
					const bool          atBoundary = (file_ptr + 1 >= poolEnd);
					++file_ptr;

					if (atBoundary)
						RefillPool(file, pool, file_ptr);

					if ((pcx_byte & 0xC0) == 0xC0)
					{
						// RLE run
						const unsigned char runcount = static_cast<unsigned char>(pcx_byte & 0x3F);
						const char          color = *file_ptr;
						++file_ptr;
						if (file_ptr >= poolEnd)
							RefillPool(file, pool, file_ptr);

						// Original used memset32 (4-byte aligned) + trailing memset;
						// plain memset is correct and equivalent for byte fills.
						memset(&cbuf[index], static_cast<unsigned char>(color), runcount);
						index += runcount;
					}
					else
					{
						cbuf[index++] = pcx_byte;
					}
				}

				pic->Unlock();
			}
			else
			{
				// -------------------------------------------------------------
				// B2: BytesLinePlane != width — row has trailing pad bytes
				// Decode row by row, writing only `width` bytes per row.
				// -------------------------------------------------------------
				char* row = cbuf;
				unsigned int  i = 0;
				unsigned char runcount_1 = 0; // persists after loop for padding drain

				for (int heightleft = height; heightleft > 0; --heightleft)
				{
					i = 0;
					while (i < static_cast<unsigned int>(header.BytesLinePlane))
					{
						const char rle_1 = *file_ptr;
						const bool atBoundary = (file_ptr + 1 >= poolEnd);
						runcount_1 = static_cast<unsigned char>(*file_ptr);
						++file_ptr;

						if (atBoundary)
							RefillPool(file, pool, file_ptr);

						if ((rle_1 & 0xC0) == 0xC0)
						{
							runcount_1 = static_cast<unsigned char>(rle_1 & 0x3F);
							const char color = *file_ptr;
							++file_ptr;
							if (file_ptr >= poolEnd)
								RefillPool(file, pool, file_ptr);

							for (unsigned int ran = 0; ran < runcount_1; ++ran)
							{
								if (i + ran < width)
									row[i + ran] = color;
							}
							i += runcount_1;
						}
						else if (i < width)
						{
							row[i++] = rle_1;
						}
						else
						{
							++i; // consume padding byte, no output
						}
					}

					row += width;
				}

				// -------------------------------------------------------------
				// Post-loop: drain up to one extra padding byte.
				// SUSPECT: exact condition preserved verbatim from assembly;
				//          purpose is aligning the stream past any trailing pad.
				// -------------------------------------------------------------
				if (i == width)
				{
					runcount_1 = static_cast<unsigned char>(*file_ptr);
					++file_ptr;
					if (file_ptr >= poolEnd)
						RefillPool(file, pool, file_ptr);
				}

				if ((runcount_1 & 0xC0) == 0xC0 && file_ptr + 1 >= poolEnd)
					RefillPool(file, pool, file_ptr);

				pic->Unlock();
			}
		}
	}
	// If Lock() returned null the original falls through to palette + close,
	// returning the (locked-but-blank) surface. Preserved intentionally.

	// -----------------------------------------------------------------------
	// Palette — PCX stores 768-byte VGA palette at end of file (planes==1 only)
	// -----------------------------------------------------------------------
	if (palette && header.NumberOfPlanes == 1)
	{
		file->Seek(-768, FileSeekMode::End);
		file->Read(palette, 768);
	}

	file->Close();
	return pic;
}

/**
 *  Writes the contents of a graphic surface as PNG to a file instance.
 *
 *  @author: CCHyper
 */
bool Write_PNG_File(FileClass* name, Surface& pic, const BytePalette* palette, bool greyscale)
{
	int pic_width = pic.Get_Width();
	int pic_height = pic.Get_Height();

	/**
	 *  Copy graphic data from the surface to the buffer.
	 */
	unsigned short* buffer = (unsigned short*)std::malloc(pic_height * (pic_width * sizeof(unsigned short)));
	if (!buffer)
	{
		return false;
	}

	std::memcpy(buffer, (unsigned short*)pic.Lock(), pic_height * (pic_width * sizeof(unsigned short)));
	pic.Unlock();

	unsigned short* bufferptr = buffer;

	/**
	 *  Convert the pixel data from 16bit to 24bit.
	 */
	unsigned char* image = (unsigned char*)std::malloc(pic_height * pic_width * 3);

	ColorStruct* imageptr = (ColorStruct*)image;

	for (int i = 0; i < (pic_width * pic_height); ++i)
	{
		unsigned short value = *(bufferptr++);
		unsigned char r = (value & 0xF800) >> 11; // Extract the 5 R bits
		unsigned char g = (value & 0x07E0) >> 5;  // Extract the 6 G bits
		unsigned char b = (value & 0x001F);       // Extract the 5 B bits
		imageptr[i].R = (r * 255) / 31;
		imageptr[i].G = (g * 255) / 63;
		imageptr[i].B = (b * 255) / 31;
	}

	/**
	 *  Encode the graphic data to png data to be written to the file.
	 */
	unsigned char* png = nullptr;
	size_t pngsize = 0;
	int error = lodepng_encode_memory(&png, &pngsize, (unsigned char*)image, pic_width, pic_height, LCT_RGB, 8);

	/**
	 *  Now write data to the file.
	 */
	name->Open1(FileAccessMode::Write);
	name->Write(png, pngsize);
	name->Close();

	/**
	 *  Cleanup buffers.
	 */
	std::free(png);
	std::free(image);
	std::free(buffer);

	/**
	 *  Handle any errors.
	 */
	if (error)
	{
		Debug::Log("lodepng_encode error %u: %s\n", error, lodepng_error_text(error));
		return false;
	}

	return true;
}


/**
 *  Read the contents of a PNG file into a graphic surface.
 *
 *  @author: CCHyper
 */
BSurface* Read_PNG_File(FileClass* name, unsigned char* palette, void* buff, long size)
{
	assert(name != nullptr);

	LodePNGState state;
	BSurface* pic = nullptr;

	unsigned char* png_image = nullptr;     // Output png image.
	unsigned int png_width;
	unsigned int png_height;

	unsigned char* png_buffer = nullptr;    // Raw png loaded from file.
	size_t png_buffersize;

	if (!PhobosCRT::stristr(name->FileName(), ".png"))
	{
		Debug::Log("Read_PNG_File() - Invalid filename!\n");
		return nullptr;
	}

	if (!name->IsAvaible()) return nullptr;

	bool file_opened = false;
	if (!name->IsOpen())
	{
		name->Open1(FileAccessMode::Read);
		file_opened = true;
	}

	png_buffersize = name->Size();

	png_buffer = (unsigned char*)std::malloc(png_buffersize);
	if (!png_buffer)
	{
		Debug::Log("Read_PNG_File() - Failed to allocate PNG buffer!\n");
		return nullptr;
	}

	if (!name->Read(png_buffer, png_buffersize))
	{
		Debug::Log("Read_PNG_File() - Failed to read PNG file!\n");

		//delete [] png_buffer;
		std::free(png_buffer);

		return nullptr;
	}

	lodepng_state_init(&state);

	state.info_raw.colortype = LCT_RGB;
	state.info_raw.bitdepth = 8;
	state.decoder.color_convert = false;

	/**
	 *  Decode the PNG data.
	 */
	unsigned error = lodepng_decode(&png_image, &png_width, &png_height, &state, png_buffer, png_buffersize);
	if (!png_image || error)
	{
		Debug::Log("Read_PNG_File() - Failed to decode PNG data!\n");

		lodepng_state_cleanup(&state);

		//delete [] png_buffer;
		std::free(png_buffer);
		std::free(png_image);

		return nullptr;
	}

	/**
	 *  We only support standard 8bit PNG RGB, report error otherwise.
	 */
	if (state.info_raw.bitdepth == 16
	 || state.info_raw.colortype == LCT_GREY
	 || state.info_raw.colortype == LCT_PALETTE
	 || state.info_raw.colortype == LCT_GREY_ALPHA
	 || state.info_raw.colortype == LCT_RGBA)
	{

		Debug::Log("Read_PNG_File() - Unsupported PNG format type!\n");

		lodepng_state_cleanup(&state);

		//delete [] png_buffer;
		std::free(png_buffer);
		std::free(png_image);

		return nullptr;
	}


#ifndef NDEBUG
	Debug::Log("Read_PNG_File() - bitdepth: %d, colortype: %d.\n",
		state.info_raw.bitdepth, state.info_raw.colortype);
#endif

	if (buff)
	{
		MemoryBuffer b(buff, size);
		pic = new BSurface(png_width, png_height, 2, &b);
	}
	else
	{
		pic = new BSurface(png_width, png_height, 2, nullptr);
	}
	assert(pic != nullptr);

	//size_t buffersize = lodepng_get_raw_size(png_width, png_height, &state.info_raw);
	//assert(buffersize == (png_width * png_height));

	/**
	 *  Copy the decoded PNG data into the image surface.
	 */
	for (int y = 0; y < pic->Get_Height(); ++y)
	{

		unsigned short* buffptr = (unsigned short*)pic->Lock(0, y);
		for (int x = 0; x < pic->Get_Width(); ++x)
		{

			int r = *png_image++; // & 0xFF;
			int g = *png_image++; // & 0xFF;
			int b = *png_image++; // & 0xFF;

			*buffptr++ = DSurface::Build_Hicolor_Pixel_BRG(r, g, b);
		}

		pic->Unlock();
	}

	std::free(png_buffer);

	lodepng_state_cleanup(&state);

	if (file_opened)
	{
		name->Close();
	}

	return pic;
}


/**
 *  Read the contents of a PNG file into a graphic surface.
 *
 *  @author: CCHyper
 */
BSurface* Read_PNG_File(FileClass* name, const MemoryBuffer& buff, BytePalette* palette)
{
	return Read_PNG_File(name, (unsigned char*)palette, buff.Get_Buffer(), buff.Get_Size());
}


/**
 *
 *  Add support for PNG files as an alternative to PCX images.
 *
 *  This intercept allows us to check the filename of the input file and see
 *  if a PNG for alternative exists for it, if so, load that instead of the
 *  PCX image file.
 *
 *  @author: CCHyper
 */
BSurface* __fastcall Read_PCX_File_Intercept(FileClass* file, unsigned char* palette, char* buff, long size)
{
	char fnamebuffer[32];
	std::strncpy(fnamebuffer, file->FileName(), sizeof(fnamebuffer));

	/**
	 *  Find the location of the file extension separator.
	 */
	char* file_name = std::strchr((char*)fnamebuffer, '.');

	if (!file_name) { //invalid name
		return nullptr;
	}
	/**
	 *  Insert a null-char where the "." was. This will give us the actual
	 *  file name without the extension, allowing us to rebuild it.
	 */
	*file_name = '\0';

	const char* upper_filename = _strupr((char*)fnamebuffer);

	char png_buffer[32 - 4];
	std::snprintf(png_buffer, sizeof(png_buffer), "%s.PNG", upper_filename);

	/**
	 *  Search for the PNG file, and load it if found.
	 */
	CCFileClass pngfile(png_buffer);

	if (pngfile.IsAvaible()) {

		BSurface* image = Read_PNG_File(&pngfile, palette, buff, size);
		if (image)
		{
			return image;
		}
	}

	/**
	 *  Fallback to the PCX file.
	 */
	return _Read_PCX_File(file, palette, buff, size);
}


DEFINE_FUNCTION_JUMP(LJMP,0x630310,  Read_PCX_File_Intercept)
DEFINE_FUNCTION_JUMP(CALL,0x5CCBD7,  Read_PCX_File_Intercept)
DEFINE_FUNCTION_JUMP(CALL,0x5CCDEC,  Read_PCX_File_Intercept)
DEFINE_FUNCTION_JUMP(CALL,0x5CE577,  Read_PCX_File_Intercept)
DEFINE_FUNCTION_JUMP(CALL,0x5CE713,  Read_PCX_File_Intercept)
DEFINE_FUNCTION_JUMP(CALL,0x641DF4,  Read_PCX_File_Intercept)
DEFINE_FUNCTION_JUMP(CALL,0x7687CF,  Read_PCX_File_Intercept)
DEFINE_FUNCTION_JUMP(CALL,0x7695F7,  Read_PCX_File_Intercept)
DEFINE_FUNCTION_JUMP(CALL,0x778233,  Read_PCX_File_Intercept)
DEFINE_FUNCTION_JUMP(CALL,0x7AF9BA,  Read_PCX_File_Intercept)
DEFINE_FUNCTION_JUMP(CALL,0x7AFA0C,  Read_PCX_File_Intercept)
DEFINE_FUNCTION_JUMP(CALL,0x7AFA51,  Read_PCX_File_Intercept)
DEFINE_FUNCTION_JUMP(CALL,0x7B04C6,  Read_PCX_File_Intercept)
DEFINE_FUNCTION_JUMP(CALL,0x7B050F,  Read_PCX_File_Intercept)
DEFINE_FUNCTION_JUMP(CALL,0x7B0558,  Read_PCX_File_Intercept)


ASMJIT_PATCH(0x5535D0, LoadProgressMgr_Draw_PCXLoadingScreen, 0x6)
{
	LEA_STACK(char*, name, 0x84);

	char pFilename[0x20];
	strcpy_s(pFilename, name);
	_strlwr_s(pFilename);

	BSurface* pcx = nullptr;
	char nBuffer[0x40];

	IMPL_SNPRNINTF(nBuffer, sizeof(nBuffer), GameStrings::LSSOBS_SHP(),
		Game::ScreenWidth() != 640 ? GameStrings::_800() : GameStrings::_640());

	if (!_stricmp(pFilename, nBuffer)) {
		IMPL_SNPRNINTF(nBuffer, sizeof(nBuffer), "ls%sobs.pcx",
			Game::ScreenWidth() != 640 ? GameStrings::_800() : GameStrings::_640());

		if (PCXImages::Instance->LoadFile(nBuffer))
			pcx = PCXImages::Instance->GetSurface(nBuffer);
	}

	if (strstr(pFilename, ".pcx") || pcx) {
		if (!pcx) {
			PCXImages::Instance->LoadFile(pFilename);
			pcx = PCXImages::Instance->GetSurface(pFilename);
		}

		if (pcx)
		{
			GET_BASE(DSurface*, pSurf, 0x60);
			RectangleStruct pSurfBounds { 0, 0, pSurf->Width, pSurf->Height };
			RectangleStruct pcxBounds { 0, 0, pcx->Width, pcx->Height };
			RectangleStruct destClip { (pSurf->Width - pcx->Width) / 2, (pSurf->Height - pcx->Height) / 2, pcx->Width, pcx->Height };

			pSurf->Copy_From(pSurfBounds, destClip, pcx, pcxBounds, pcxBounds, true, true);
		}

		return 0x553603;
	}

	return 0;
}

ASMJIT_PATCH(0x552FCB, LoadProgressMgr_Draw_PCXLoadingScreen_Campaign, 0x6)
{
	char filename[0x40];
	strcpy_s(filename, ScenarioClass::Instance->LS800BkgdName);
	_strlwr_s(filename);

	if (strstr(filename, ".pcx"))
	{
		BSurface* pPCX = nullptr;

		if (PCXImages::Instance->LoadFile(filename))
			pPCX = PCXImages::Instance->GetSurface(filename);

		if (pPCX)
		{
			GET_BASE(DSurface*, pSurface, 0x60);

			RectangleStruct pSurfBounds { 0, 0, pSurface->Width, pSurface->Height };
			RectangleStruct pcxBounds { 0, 0, pPCX->Width, pPCX->Height };
			RectangleStruct destClip { (pSurface->Width - pPCX->Width) / 2, (pSurface->Height - pPCX->Height) / 2, pPCX->Width, pPCX->Height };

			pSurface->Copy_From(pSurfBounds, destClip, pPCX, pcxBounds, pcxBounds, true, true);
		}

		return 0x552FFF;
	}

	return 0;
}

struct PCXEntryNew {
	BSurface* Surface;
	uint8_t   Palette[768];
};

//completely replace the weird linked link approach of the original game code
//to this more modern map easier to mantain and less code need to be written overral
//severing some path of original code that was unused anymore and focus only for the
//main function as PCX image file loader (additionally supporting PNG)
std::unordered_map<std::string, PCXEntryNew> g_PCXEntries;

class NOVTABLE FakePCXImages : public PCXImages {
public:

	bool _Add(char* name, int bpp, bool apply_palette) {
		if (!name)
			return false;

		// Zero VGA palette (matches original do/while zeroing loop).
		uint8_t palette[768];
		memset(palette, 0, sizeof(palette));

		// ------------------------------------------------------------------
		// Open and decode PCX
		// ------------------------------------------------------------------
		CCFileClass file(name);
		BSurface* src = Read_PCX_File_Intercept(&file, palette, nullptr, 0);

		if (!src) {
			return false;
		}

		// ------------------------------------------------------------------
		// bpp == 2: convert 8-bit src + VGA palette into a new 16-bit surface
		// ------------------------------------------------------------------
		BSurface* dest = src;

		if (src->BytesPerPixel != 2 && bpp == 2) {
			// Build 256-entry 16-bit LUT from VGA palette.
			int16_t lut[256];
			for (int idx = 0; idx < 256; ++idx)
			{
				const uint8_t r = palette[idx * 3 + 0];
				const uint8_t g = palette[idx * 3 + 1];
				const uint8_t b = palette[idx * 3 + 2];
				lut[idx] = static_cast<int16_t>(
					((r >> Drawing::RedShiftRight()) << Drawing::RedShiftLeft()) |
					((g >> Drawing::GreenShiftRight()) << Drawing::GreenShiftLeft()) |
					((b >> Drawing::BlueShiftRight()) << Drawing::BlueShiftLeft()));
			}

			RectangleStruct srcRect = src->Get_Rect();
			const int w = srcRect.Width;
			const int h = srcRect.Height;

			dest = GameCreate<BSurface>(w,h, 2);

			int16_t* dstPx = static_cast<int16_t*>(dest->Lock());
			const uint8_t* srcPx = static_cast<const uint8_t*>(src->Lock());

			for (int px = 0, count = w * h; px < count; ++px)
				dstPx[px] = lut[srcPx[px]];

			src->Unlock();
			dest->Unlock();
			GameDelete(src);
		}

		// ------------------------------------------------------------------
		// apply_palette: remap pixel indices to red-channel value in-place.
		// palette[idx * 3] == red channel byte.
		// Only valid for bpp==1.
		// ------------------------------------------------------------------
		if (apply_palette && bpp == 1)
		{
			RectangleStruct destRect = dest->Get_Rect();
			uint8_t* px = static_cast<uint8_t*>(dest->Lock());

			for (int i = 0, count = destRect.Width * destRect.Height; i < count; ++i)
				px[i] = palette[static_cast<uint8_t>(px[i]) * 3];

			dest->Unlock();
		}

		// ------------------------------------------------------------------
		// Build the cache entry and insert (replaces existing key if present).
		// operator[] on unordered_map handles both insert and replace cleanly.
		// ------------------------------------------------------------------
		std::string key(name);
		// Lowercase in-place — mirrors original Wstring::To_Lower call.
		for (char& c : key)
			c = static_cast<char>(tolower(static_cast<unsigned char>(c)));

		// If a previous entry exists for this key, destroy its surface first.
		auto it = g_PCXEntries.find(key);

		if (it != g_PCXEntries.end()) {

			if(it->second.Surface)
				GameDelete(it->second.Surface);

			it->second.Surface = dest;

			if (bpp == 1)
				memcpy(it->second.Palette, palette, sizeof(PCXEntryNew::Palette));

		} else {
			auto& entry = g_PCXEntries[key];

			entry.Surface = dest;

			if (bpp == 1)
				memcpy(entry.Palette, palette, sizeof(PCXEntryNew::Palette));
		}

		return true;
	}

	BSurface* GetSurface(const char* pFileName, BytePalette* pPalette = nullptr)
	{
		if (!pFileName)
			return nullptr;

		auto it = g_PCXEntries.find(pFileName);

		if (it != g_PCXEntries.end() && it->second.Surface) {

			//if(pPalette)
			//	std::memcpy(it->second.Palette, pPalette, sizeof(BytePalette));

			return it->second.Surface;
		}

		return nullptr;
	}
};

 DEFINE_FUNCTION_JUMP(LJMP, 0x6B9D00, FakePCXImages::_Add);
 DEFINE_FUNCTION_JUMP(LJMP, 0x6BA140, FakePCXImages::GetSurface);
