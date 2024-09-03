#include "BlowfishEngine.h"

#include <Utilities/Macro.h>

/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          BLOWFISH.CPP
 *
 *  @author        Joe L. Bostic (see notes below)
 *
 *  @contributors  CCHyper
 *
 *  @brief         This implements the Blowfish algorithm.
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
 *  @note          This file contains heavily modified code from the source code
 *                 released by Electronic Arts for the C&C Remastered Collection
 *                 under the GPL3 license. Source:
 *                 https://github.com/ElectronicArts/CnC_Remastered_Collection
 *
 ******************************************************************************/

DEFINE_JUMP(LJMP, 0x6BC33A, 0x6BC425);
DEFINE_JUMP(LJMP, 0x6BD6CA, 0x6BD71D);

DEFINE_HOOK(0x438300, BlowStraw_Key_replace, 0x6)
{
	GET(BlowStraw*, pThis, ECX);
	GET_STACK(void*, pKey, 0x4);
	GET_STACK(int, len , 0x8);
	pThis->Key(pKey, len);
	return 0x43833D;
}

DEFINE_HOOK(0x438210, BlowStraw_Get_replace, 0x5)
{
	GET(BlowStraw*, pThis, ECX);
	GET_STACK(void*, pDest, 0x4);
	GET_STACK(int, len, 0x8);
	R->EAX(pThis->BlowStraw::Get(pDest, len));
	return 0x4382F0;
}

DEFINE_HOOK(0x438060, BlowPipe_Flush_replace, 0x6)
{
	GET(BlowPipe*, pThis, ECX);
	R->EAX(pThis->BlowPipe::Flush());
	return 0x438094;
}

DEFINE_HOOK(0x4380A0, BlowPipe_Put_replace, 0x5)
{
	GET(BlowPipe*, pThis, ECX);
	GET_STACK(void*, pSource, 0x4);
	GET_STACK(int, len, 0x8);
	R->EAX(pThis->BlowPipe::Put(pSource, len));
	return 0x4381B1;
}

DEFINE_HOOK(0x4381D0, BlowPipe_Key_replace, 0x6)
{
	GET(BlowPipe*, pThis, ECX);
	GET_STACK(void*, pKey, 0x4);
	GET_STACK(int, len, 0x8);
	pThis->Key(pKey, len);
	return 0x43820D;
}

DEFINE_HOOK(0x437F50, BlowfishEngine_CTOR, 0x6)
{
	GET(BlowfishEngine*, pThis, ECX);
	pThis->BlowfishEngine::BlowfishEngine();
	R->EAX(pThis);
	return 0x437FBD;
}

DEFINE_HOOK(0x437FC0, BlowfishEngine_DTOR, 0x6)
{
	GET(BlowfishEngine*, pThis, ECX);
	pThis->BlowfishEngine::~BlowfishEngine();
	return 0x437FCC;
}

DEFINE_HOOK(0x437FD0, BlowfishEngine_Submit_Key, 0x6)
{
	GET(BlowfishEngine*, pThis, ECX);
	GET_STACK(void*, pKey, 0x4);
	GET_STACK(int, len, 0x8);
	pThis->Submit_Key(pKey, len);
	return 0x437FF5;
}

DEFINE_HOOK(0x437FD0, BlowfishEngine_Encrypt, 0x6)
{
	GET(BlowfishEngine*, pThis, ECX);
	GET_STACK(int, len, 0x4);
	GET_STACK(void*, pKey, 0x8);
	GET_STACK(void*, pChyper, 0xC);
	R->EAX(pThis->Encrypt(pKey, len , pChyper));
	return 0x437FF5;
}

DEFINE_HOOK(0x438030, BlowfishEngine_Decrypt, 0x6)
{
	GET(BlowfishEngine*, pThis, ECX);
	GET_STACK(int, len, 0x4);
	GET_STACK(void*, pKey, 0x8);
	GET_STACK(void*, pChyper, 0xC);
	R->EAX(pThis->Decrypt(pKey, len, pChyper));
	return 0x43805C;
}
