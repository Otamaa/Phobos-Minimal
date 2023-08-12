/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          FILEPNG.H
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
#pragma once

class FileClass;
class Surface;
class BSurface;
class MemoryBuffer;
struct BytePalette;

namespace PNGFile
{
	bool Write(FileClass* name, Surface& pic, const BytePalette* palette, bool greyscale = false);
	BSurface* Read(FileClass* name, unsigned char* palette = nullptr, void* buff = nullptr, long size = 0);
	BSurface* Read(FileClass* name, const MemoryBuffer& buff, BytePalette* palette = nullptr);
};