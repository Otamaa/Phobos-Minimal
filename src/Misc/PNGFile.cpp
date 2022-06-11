/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          FILEPNG.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Function for writing PNG files from a graphic surface.
 *
 *  @license       Vinifera is free software: you can redistribute it and/or
 *                 modify it under the terms of the GNU General Public License
 *                 as published by the Free Software Foundation, either version
 *                 3 of the License, or (at your option) any later version.
 *
 *                 Vinifera is distributed in the hope that it will be
 *                 useful, but WITHOUT ANY WARRANTY; without even the implied
 *                 warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *                 PURPOSE. See the GNU General Public License for more details.
 *
 *                 You should have received a copy of the GNU General Public
 *                 License along with this program.
 *                 If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/
#include "PNGFile.h"
#include <Base/Always.h>
#include <cstdlib>
#include <Drawing.h>
#include <CCFileClass.h>
#include <Phobos.CRT.h>
#include <Utilities/Debug.h>
#include <Utilities/GeneralUtils.h>
#include <Lib/LodePNG/LodePNG.h>

template <typename T,typename FileClass>
static bool F_Read(FileClass pWho, T& obj, int size = sizeof(T)) {
	return pWho->ReadBytes(&obj, size) == size;
}

template <typename T, typename FileClass>
static bool F_Write(FileClass pWho,T& obj, int size = sizeof(T)) {
	return pWho->WriteBytes(&obj, size) == size;
}

bool PNGFile::Write(FileClass*name, Surface &pic, const BytePalette *palette, bool greyscale)
{
    int pic_width = pic.Get_Width();
    int pic_height = pic.Get_Height();

    /**
     *  Copy graphic data from the surface to the buffer.
     */
    unsigned short *buffer = (unsigned short *)malloc(pic_height * (pic_width * sizeof(unsigned short)));
    if (!buffer) {
        return false;
    }

    memcpy(buffer, (unsigned short *)pic.Lock(), pic_height * (pic_width * sizeof(unsigned short)));
    pic.Unlock();

    unsigned short *bufferptr = buffer;

    /**
     *  Convert the pixel data from 16bit to 24bit.
     */
    unsigned char *image = (unsigned char *)malloc(pic_height * pic_width * 3);

    struct rgb {
        unsigned char r, g, b;
    };
    rgb *imageptr = (rgb *)image;

#pragma warning( push )
#pragma warning (disable : 4244)
    for (int i = 0; i < (pic_width * pic_height); ++i) {
        unsigned short value = *(bufferptr++);
        unsigned char r = (value & 0xF800) >> 11; // Extract the 5 R bits
        unsigned char g = (value & 0x07E0) >> 5;  // Extract the 6 G bits
        unsigned char b = (value & 0x001F);       // Extract the 5 B bits
        imageptr[i].r = (r * 255) / 31;
        imageptr[i].g = (g * 255) / 63;
        imageptr[i].b = (b * 255) / 31;
    }
#pragma warning( pop )

    /**
     *  Encode the graphic data to png data to be written to the file.
     */
    unsigned char *png = nullptr;
    size_t pngsize = 0;
    int error = lodepng_encode_memory(&png, &pngsize, (unsigned char *)image, pic_width, pic_height, LCT_RGB, 8);

    /**
     *  Now write data to the file.
     */
    name->Open(FileAccessMode::Write);
	name->WriteBytes(png, pngsize);
    name->Close();

    /**
     *  Cleanup buffers.
     */
	free(png);
	free(image);
	free(buffer);

    /**
     *  Handle any errors.
     */
    if (error) {
        Debug::Log("lodepng_encode error %u: %s\n", error, lodepng_error_text(error));
        return false;
    }

    return true;
}

//0x6BA140
BSurface * PNGFile::Read(FileClass*name, unsigned char *palette, void *buff, long size)
{
    if(name == nullptr) {
		Debug::Log("PNGFile() - Invalid FileClass ptr ! \n");
		return nullptr;
	}

	auto const pName = name->GetFileName();

    if (!PhobosCRT::stristr(pName, ".png")) {
        //Debug::Log("PNGFile() - Invalid file[%s] !\n", pName);
        return nullptr;
	} else {
		Debug::Log("PNGFile() - Reading file[%s] !\n", pName);
	}

	if (!name->Exists()) {
		Debug::Log("PNGFile() - Reading file[%s]!, Doesnt Exist !\n", pName);
		return nullptr;
	}

    bool file_opened = false;
    if (!name->HasHandle()) {
        name->Open(FileAccessMode::Read);
        file_opened = true;
    }

	size_t png_buffersize = name->GetFileSize();
	LodePNGState state;
	unsigned char* png_image = nullptr;     // Output png image.
	unsigned int png_width;
	unsigned int png_height;
	BSurface* pic = nullptr;

	// Raw png loaded from file.
	unsigned char* png_buffer = (unsigned char *)malloc(sizeof(char) * png_buffersize);

    if (!png_buffer) {
		Debug::Log("PNGFile() - Failed to allocate PNG buffer!\n");
        return nullptr;
    }
	size_t nReadedBytes = name->ReadBytes(png_buffer, png_buffersize);
    if (nReadedBytes != png_buffersize) {
		Debug::Log("PNGFile() - Failed to read PNG file!\n");
		free(png_buffer);
        return nullptr;
    }

    lodepng_state_init(&state);
    state.info_raw.colortype = LCT_RGB;
    state.info_raw.bitdepth = 8;
    state.decoder.color_convert = false;

    unsigned error = lodepng_decode(&png_image, &png_width, &png_height, &state, png_buffer, png_buffersize);

	if (!png_image || error) {

		Debug::Log("PNGFile() - Error [%u] %s\n", error, lodepng_error_text(error));
        lodepng_state_cleanup(&state);
		free(png_buffer);
		free(png_image);

        return nullptr;
    }

    /**
     *  We only support standard 8bit PNG RGB, report error otherwise.
     */
    if (state.info_raw.bitdepth == 16
     || state.info_raw.colortype == LCT_GREY
     || state.info_raw.colortype == LCT_PALETTE
     || state.info_raw.colortype == LCT_GREY_ALPHA
     || state.info_raw.colortype == LCT_RGBA) {

		Debug::Log("PNGFile() - Unsupported PNG format type!\n");
        lodepng_state_cleanup(&state);
		free(png_buffer);
		free(png_image);

        return nullptr;
    }

	Debug::Log("PNGFile() - bitdepth: %d, colortype: %d.\n",
        state.info_raw.bitdepth, state.info_raw.colortype);

    if (buff) {
		MemoryBuffer b(buff, size);
		pic = GameCreate<BSurface>(png_width, png_height, 2, b);
    } else {
        pic = GameCreate<BSurface>(png_width, png_height, 2,nullptr);
    }

	if (!pic) {
		Debug::Log("PNGFile() - Failed To Create PNG File ! \n");
		free(png_buffer);
		free(png_image);
		lodepng_state_cleanup(&state);
		return nullptr;
	}

#pragma warning( push )
#pragma warning (disable : 4244)
    for (int y = 0; y < pic->Height; ++y) {

        unsigned short *buffptr = (unsigned short *)pic->Lock(0, y);
        for (int x = 0; x < pic->Width; ++x) {

            int r = *png_image++; // & 0xFF;
            int g = *png_image++; // & 0xFF;
            int b = *png_image++; // & 0xFF;

            *buffptr++ = DSurface::RGBA_To_Pixel(r, g, b);
        }

        pic->Unlock();
    }
#pragma warning( pop )

	free(png_buffer);
	lodepng_state_cleanup(&state);
    if (file_opened) {
        name->Close();
    }

    return pic;
}

BSurface * PNGFile::Read(FileClass*name, const MemoryBuffer&buff, BytePalette *palette)
{
    return PNGFile::Read(name, (unsigned char *)palette, buff.Get_Buffer(), buff.Get_Size());
}

DEFINE_HOOK(0x630310, Read_PCX_File_InterceptPNG, 0x7)
{
	enum
	{
		Replace = 0x63033B,
		Continue = 0x0
	};

	GET(CCFileClass* const, pFile, ECX);
	GET(unsigned char* const, pPal, EDX);
	GET_STACK(void* const, pBuffer, 0x4);
	GET_STACK(long const, nBuffersize, 0x8);

	if (auto const pSurface = PNGFile::Read(pFile, pPal, pBuffer, nBuffersize))
	{
		R->EAX(pSurface);
		return Replace;
	}

	return Continue;
}
