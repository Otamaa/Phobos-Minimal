#include "PNGFile.h"

#include <Surface.h>
#include <CCFileClass.h>
#include <CRT.h>

#include <Utilities/Debug.h>

#include <Lib/lodepng/lodepng.h>
#include <Lib/lodepng/lodepng_util.h>

template <typename T, typename FileClass>
static bool F_Read(FileClass pWho, T& obj, int size = sizeof(T))
{
	return pWho->ReadBytes(&obj, size) == size;
}

template <typename T, typename FileClass>
static bool F_Write(FileClass pWho, T& obj, int size = sizeof(T))
{
	return pWho->WriteBytes(&obj, size) == size;
}

bool PNGFile::Write(FileClass* name, Surface& pic, const BytePalette* palette, bool greyscale)
{
	int pic_width = pic.Get_Width();
	int pic_height = pic.Get_Height();

	/**
	 *  Copy graphic data from the surface to the buffer.
	 */
	unsigned short* buffer = (unsigned short*)CRT::malloc(pic_height * (pic_width * sizeof(unsigned short)));
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
	unsigned char* image = (unsigned char*)CRT::malloc(pic_height * pic_width * 3);

	struct rgb
	{
		unsigned char r, g, b;
	};
	rgb* imageptr = (rgb*)image;

	for (int i = 0; i < (pic_width * pic_height); ++i)
	{
		unsigned short value = *(bufferptr++);  
		imageptr[i].r = (unsigned char((value & 0xF800) >> 11) * 255) / 31;// Extract the 5 R bits
		imageptr[i].g = (unsigned char((value & 0x07E0) >> 5) * 255) / 63;// Extract the 6 G bits
		imageptr[i].b = (unsigned char((value & 0x001F)) * 255) / 31;// Extract the 5 B bits
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
	name->WriteBytes(png, pngsize);
	name->Close();

	/**
	 *  Cleanup buffers.
	 */
	CRT::free(png);
	CRT::free(image);
	CRT::free(buffer);

	/**
	 *  Handle any errors.
	 */
	if (error)
	{
		Debug::LogInfo("lodepng_encode error {}: {}", error, lodepng_error_text(error));
		return false;
	}

	return true;
}

//0x6BA140
BSurface* PNGFile::Read(FileClass* name, unsigned char* palette, void* buff, long size)
{
	if (name == nullptr)
	{
		Debug::LogInfo("Invalid FileClass ptr ! ");
		return nullptr;
	}

	auto const pName = name->GetFileName();

	if (!PhobosCRT::stristr(pName, ".png"))
	{
		//Debug::LogInfo("Read_PNG_File() - Invalid file[%s] !", pName);
		return nullptr;
	}
	else
	{
		Debug::LogInfo("Read_PNG_File() - Reading file[{}] !", pName);
	}

	if (!name->Exists())
	{
		Debug::LogInfo("Read_PNG_File() - Reading file[{}]!, Doesnt Exist !", pName);
		return nullptr;
	}

	bool file_opened = false;
	if (!name->HasHandle())
	{
		name->Open(FileAccessMode::Read);
		file_opened = true;
	}

	size_t png_buffersize = name->GetFileSize();

	// Raw png loaded from file.
	unsigned char* png_buffer = (unsigned char*)CRT::malloc(sizeof(char) * png_buffersize);

	if (!png_buffer)
	{
		Debug::LogInfo("Read_PNG_File() - Failed to allocate PNG buffer!");
		return nullptr;
	}
	size_t nReadedBytes = name->ReadBytes(png_buffer, png_buffersize);
	if (nReadedBytes != png_buffersize)
	{
		Debug::LogInfo("Read_PNG_File() - Failed to read PNG file!");
		CRT::free(png_buffer);
		return nullptr;
	}

	LodePNGState state;
	lodepng_state_init(&state);
	state.info_raw.colortype = LCT_RGB;
	state.info_raw.bitdepth = 8;
	state.decoder.color_convert = false;

	unsigned int png_width;
	unsigned int png_height;
	unsigned char* png_image = nullptr;     // Output png image.
	BSurface* pic = nullptr;

	unsigned error = lodepng_decode(&png_image, &png_width, &png_height, &state, png_buffer, png_buffersize);

	if (!png_image || error)
	{

		Debug::LogInfo("Read_PNG_File() - Error [{}] {}", error, lodepng_error_text(error));
		lodepng_state_cleanup(&state);
		CRT::free(png_buffer);
		CRT::free(png_image);

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

		Debug::LogInfo("Read_PNG_File() - Unsupported PNG format type!");
		lodepng_state_cleanup(&state);
		CRT::free(png_buffer);
		CRT::free(png_image);

		return nullptr;
	}

	Debug::LogInfo("Read_PNG_File() - bitdepth: {}, colortype: {}.",
		state.info_raw.bitdepth, (int)state.info_raw.colortype);

	if (buff)
	{
		MemoryBuffer b(buff, size);
		pic = GameCreate<BSurface>(png_width, png_height, 2, b);
	}
	else
	{
		pic = GameCreate<BSurface>(png_width, png_height, 2, nullptr);
	}

	if (!pic)
	{
		Debug::LogInfo("Failed To Create PNG File ! ");
		CRT::free(png_buffer);
		CRT::free(png_image);
		lodepng_state_cleanup(&state);
		return nullptr;
	}

	for (int y = 0; y < pic->Height; ++y)
	{

		unsigned short* buffptr = (unsigned short*)pic->Lock(0, y);
		for (int x = 0; x < pic->Width; ++x)
		{

			int r = *png_image++; // & 0xFF;
			int g = *png_image++; // & 0xFF;
			int b = *png_image++; // & 0xFF;

			*buffptr++ = DSurface::RGBA_To_Pixel(r, g, b);
		}

		pic->Unlock();
	}

	CRT::free(png_buffer);
	lodepng_state_cleanup(&state);
	if (file_opened)
	{
		name->Close();
	}

	return pic;
}

BSurface* PNGFile::Read(FileClass* name, const MemoryBuffer& buff, BytePalette* palette)
{
	return PNGFile::Read(name, (unsigned char*)palette, buff.Get_Buffer(), buff.Get_Size());
}
