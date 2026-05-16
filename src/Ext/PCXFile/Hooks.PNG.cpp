#include <PCX.h>
#include <Utilities/Patch.h>
#include <Utilities/Macro.h>
#include <Utilities/Debug.h>

#include <CCFileClass.h>
#include <ScenarioClass.h>
#include <FileSystem.h>
#include <ConvertClass.h>

#include <Lib/lodepng/lodepng.h>

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
	name->Open(FileAccessMode::Write);
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

	if (!PhobosCRT::stristr(name->GetFileName(), ".png"))
	{
		Debug::Log("Read_PNG_File() - Invalid filename!\n");
		return nullptr;
	}

	if (!name->Exists()) return nullptr;

	bool file_opened = false;
	if (!name->HasHandle())
	{
		name->Open(FileAccessMode::Read);
		file_opened = true;
	}

	png_buffersize = name->GetFileSize();

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
BSurface* __fastcall Read_PCX_File_Intercept(FileClass* file, unsigned char* palette, void* buff, long size)
{
	char fnamebuffer[32];
	std::strncpy(fnamebuffer, file->GetFileName(), sizeof(fnamebuffer));

	/**
	 *  Find the location of the file extension separator.
	 */
	char* file_name = std::strchr((char*)fnamebuffer, '.');

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
	if (pngfile.Exists())
	{

		BSurface* image = Read_PNG_File(&pngfile, palette, buff, size);
		if (image)
		{
			return image;
		}
	}

	/**
	 *  Fallback to the PCX file.
	 */
	return (BSurface*)PCXImages::Read_PCX_File(file, palette, buff, size);
}


DEFINE_FUNCTION_JUMP(CALL,0x5CCBD7,  Read_PCX_File_Intercept)
DEFINE_FUNCTION_JUMP(CALL,0x5CCDEC,  Read_PCX_File_Intercept)
DEFINE_FUNCTION_JUMP(CALL,0x5CE577,  Read_PCX_File_Intercept)
DEFINE_FUNCTION_JUMP(CALL,0x5CE713,  Read_PCX_File_Intercept)
DEFINE_FUNCTION_JUMP(CALL,0x641DF4,  Read_PCX_File_Intercept)
DEFINE_FUNCTION_JUMP(CALL,0x6B9D54,  Read_PCX_File_Intercept)
DEFINE_FUNCTION_JUMP(CALL,0x7687CF,  Read_PCX_File_Intercept)
DEFINE_FUNCTION_JUMP(CALL,0x7695F7,  Read_PCX_File_Intercept)
DEFINE_FUNCTION_JUMP(CALL,0x778233,  Read_PCX_File_Intercept)
DEFINE_FUNCTION_JUMP(CALL,0x7AF9BA,  Read_PCX_File_Intercept)
DEFINE_FUNCTION_JUMP(CALL,0x7AFA0C,  Read_PCX_File_Intercept)
DEFINE_FUNCTION_JUMP(CALL,0x7AFA51,  Read_PCX_File_Intercept)
DEFINE_FUNCTION_JUMP(CALL,0x7B04C6,  Read_PCX_File_Intercept)
DEFINE_FUNCTION_JUMP(CALL,0x7B050F,  Read_PCX_File_Intercept)
DEFINE_FUNCTION_JUMP(CALL,0x7B0558,  Read_PCX_File_Intercept)


ASMJIT_PATCH(0x6B9D9C, RGB_PCX_Loader, 0x7)
{
	GET(BSurface* const, pSurf, EDI);
	return (pSurf->BytesPerPixel == 2) ? 0x6B9EE7 : 0x0;
}

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
	if (!_stricmp(pFilename, nBuffer))
	{
		IMPL_SNPRNINTF(nBuffer, sizeof(nBuffer), "ls%sobs.pcx",
			Game::ScreenWidth() != 640 ? GameStrings::_800() : GameStrings::_640());

		if (PCXImages::Instance->LoadFile(nBuffer))
			pcx = PCXImages::Instance->GetSurface(nBuffer);
	}

	if (strstr(pFilename, ".pcx") || pcx)
	{
		if (!pcx)
		{
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