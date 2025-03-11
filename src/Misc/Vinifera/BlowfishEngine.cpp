#include "BlowfishEngine.h"

#include <Utilities/Macro.h>

/*******************************************************************************
/*								O P E N  S O U R C E
/*******************************************************************************
 *
 *  @project       Phobos-minimal
 *
 *  @file          BlowfishEngine.CPP
 *
 *  @author        Joe L. Bostic (see notes below)
 *
 *  @contributors  CCHyper, Otamaa
 *
 *  @brief         This implements the Blowfish algorithm.
 *
 *  @note          This file contains heavily modified code from the source code
 *                 released by Vinifera for the C&C: Tiberian Sun engine extension
 *                 under the GPL3 license. Source:
 *                 https://github.com/Vinifera-Developers/Vinifera/blob/develop
 *
 ******************************************************************************/

//COMPILETIMEEVAL std::string encryptDecrypt(const std::string& toEncrypt , const std::string& key) {
//    std::string output = toEncrypt;
//    //char pkey[3] = {'K', 'C', 'Q'}; //Any chars will work, in an array of any size
//	//i % (sizeof(pkey) / sizeof(char))
//
//    for (int i = 0; i < toEncrypt.size(); i++)
//        output[i] = toEncrypt[i] ^ key[i % (sizeof(key.size() - 1) / sizeof(char))];
//
//    return output;
//}

DEFINE_JUMP(LJMP, 0x6BC33A, 0x6BC425);
DEFINE_JUMP(LJMP, 0x6BD6CA, 0x6BD71D);

DEFINE_HOOK(0x438300, BlowStraw_Key_replace, 0x6)
{
	GET(BlowStraw*, pThis, ECX);
	GET_STACK(void*, pKey, 0x4);
	GET_STACK(int, len, 0x8);
	pThis->BlowStraw::Key(pKey, len);
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
	pThis->BlowPipe::Key(pKey, len);
	return 0x43820D;
}

DEFINE_HOOK(0x437F50, BlowfishEngine_CTOR, 0x6)
{
	GET(BlowfishEngine*, pThis, ECX);
	R->EAX(new BlowfishEngine());
	return 0x437FBD;
}

DEFINE_HOOK(0x437FC0, BlowfishEngine_DTOR, 0x6)
{
	GET(BlowfishEngine*, pThis, ECX);
	//pThis->BlowfishEngine::~BlowfishEngine();
	return 0x437FCC;
}

//DEFINE_FUNCTION_JUMP(LJMP, 0x438300, BlowStraw::Key));
//DEFINE_FUNCTION_JUMP(LJMP, 0x438210, BlowStraw::Get));
//DEFINE_FUNCTION_JUMP(LJMP, 0x438060, BlowPipe::Flush));
//DEFINE_FUNCTION_JUMP(LJMP, 0x4380A0, BlowPipe::Put));
//DEFINE_FUNCTION_JUMP(LJMP, 0x4381D0, BlowPipe::Key));
DEFINE_FUNCTION_JUMP(LJMP, 0x437FD0, BlowfishEngine::Submit_Key);
DEFINE_FUNCTION_JUMP(LJMP, 0x438000, BlowfishEngine::Encrypt_Wrapper);
DEFINE_FUNCTION_JUMP(LJMP, 0x438030, BlowfishEngine::Decrypt_Wrapper);