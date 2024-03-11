#include <Utilities/Debug.h>

#include "Header.h"

//use the padding ,...
DEFINE_STRONG_HOOK_AGAIN(0x4A4AC0, Debug_Log, 5)
DEFINE_STRONG_HOOK(0x4068E0, Debug_Log, 5)
{
	LEA_STACK(va_list const, args, 0x8);
	GET_STACK(const char* const, pFormat, 0x4);

	Debug::LogWithVArgs(pFormat, args);

	return 0x4A4AF9; // changed to co-op with YDE
}

DEFINE_HOOK(0x4C850B, Exception_Dialog, 5)
{
	Debug::FreeMouse();
	return 0;
}

DEFINE_HOOK(0x534f89, Game_ReloadNeutralMIX_NewLine, 5)
{
	Debug::Log("LOADED NEUTRAL.MIX\n");
	return 0x534F96;
}

DEFINE_HOOK(0x5FDDA4, IsOverlayIdxTiberium_Log, 6)
{
	GET(OverlayTypeClass*, pThis, EAX);
	Debug::Log(*reinterpret_cast<const char**>(0x833490), pThis->ID);
	return 0x5FDDC1;
}

DEFINE_HOOK(0x52E9AA, Frontend_WndProc_Checksum, 5)
{
	if (SessionClass::Instance->GameMode == GameMode::LAN || SessionClass::Instance->GameMode == GameMode::Internet)
	{
		auto nHashes = HashData::GetINIChecksums();
		Debug::Log("Rules checksum: %08X\n", nHashes.Rules);
		Debug::Log("Art checksum: %08X\n", nHashes.Art);
		Debug::Log("AI checksum: %08X\n", nHashes.AI);
	}

	return 0;
}
