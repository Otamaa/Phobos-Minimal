#include <Utilities/Debug.h>
#include <Utilities/Macro.h>
#include <Utilities/Patch.h>

#include <Helpers/Macro.h>

#include <SessionClass.h>

//use the padding ,...
DEFINE_HOOK_AGAIN(0x4A4AC0, Debug_Log, 1)
DEFINE_HOOK(0x4068E0, Debug_Log, 1)
{
	LEA_STACK(va_list const, args, 0x8);
	GET_STACK(const char*, fmt, 0x4);

	if (Debug::LogFileActive()) {
		vfprintf(Debug::LogFile, fmt, args);
		Debug::Flush();
	}

	return 0x4A4AF9; // changed to co-op with YDE
}

ASMJIT_PATCH(0x4C850B, Exception_Dialog, 5)
{
	Debug::FreeMouse();
	return 0;
}

ASMJIT_PATCH(0x534f89, Game_ReloadNeutralMIX_NewLine, 5)
{
	Debug::Log("LOADED NEUTRAL.MIX\n");
	return 0x534F96;
}

ASMJIT_PATCH(0x52E9AA, Frontend_WndProc_Checksum, 5)
{
	if (SessionClass::Instance->GameMode == GameMode::LAN || SessionClass::Instance->GameMode == GameMode::Internet) {
		auto nHashes = Debug::GetINIChecksums();
		Debug::LogInfo("Checksums : \n Rules {} \n Art {} \n AI {}",
			(unsigned)nHashes.Rules,
			(unsigned)nHashes.Art,
			(unsigned)nHashes.AI
		);
	}

	return 0;
}
