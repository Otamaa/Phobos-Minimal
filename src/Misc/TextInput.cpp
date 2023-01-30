#include <Phobos.h>
#include <Utilities\Macro.h>

#include <CRT.h>
#include <Unsorted.h>

//DEFINE_HOOK(0x55E484, MessageInput_AllowSkirmish, 0x9)
//{
//	return Phobos::Otamaa::IsAdmin ? 0x55E48D : 0x0;
//}

//wchar_t* IMEBuffer = reinterpret_cast<wchar_t*>(0xB730EC);

UINT NOINLINE GetCurentCodepage()
{
	char szLCData[6 + 1];
	WORD lang = LOWORD(GetKeyboardLayout(NULL));
	LCID locale = MAKELCID(lang, SORT_DEFAULT);
	GetLocaleInfoA(locale, LOCALE_IDEFAULTANSICODEPAGE, szLCData, _countof(szLCData));

	return CRT::atoi(szLCData);
}

wchar_t NOINLINE LocalizeCaracter(char character)
{
	wchar_t result {};
	UINT codepage = GetCurentCodepage();
	MultiByteToWideChar(codepage, MB_USEGLYPHCHARS, &character, 1, &result, 1);
	return result;
}

DEFINE_HOOK(0x5D46C7, MessageListClass_Input, 0x5)
{
	if (!Game::IMEBuffer[0])
		R->EBX<wchar_t>(LocalizeCaracter(R->EBX<char>()));

	return 0;
}

DEFINE_HOOK(0x61510E, WWUI_NewEditCtrl, 0x7)
{
	R->EDI<wchar_t>(LocalizeCaracter(R->EDI<char>()));
	return 0;
}

//DEFINE_JUMP(LJMP,0x7CC2AC, GET_OFFSET(mbstowcs));

// It is required to add Imm32.lib to AdditionalDependencies
//DEFINE_HOOK(0x777F15, IMEUpdateCompositionString, 0x7)
//{
//	Game::IMECompositionString[0] = 0;
//	ImmGetCompositionStringW(Game::IMEContext, GCS_COMPSTR, Game::IMECompositionString, 256);
//
//	return 0;
//}

