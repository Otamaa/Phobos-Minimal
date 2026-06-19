#include "Main.h"
#include <Utilities/Macro.h>

#include <MessageListClass.h>
#include <HouseClass.h>

// void* __fastcall fake_MessageListClass__Add_Message(MessageListClass* pThis,
// 													DWORD,
// 													wchar_t* Name,
// 													int ID,
// 													wchar_t* message,
// 													int color,
// 													TextPrintType PrintType,
// 													int32_t duration,
// 													bool SinglePlayer)
// {
// 	if (Name == NULL || SpawnerMain::GetMainConfigs()->AllowChat) {
// 		return pThis->AddMessage(Name, ID, message, color, PrintType, duration, SinglePlayer);
// 	}

// 	if (_wcsicmp(Name, HouseClass::CurrentPlayer->UIName) == 0) {
// 		return pThis->AddMessage(0, 0, L"Chat is disabled. Message not sent.", 4, TextPrintType(0x4096), 270, 1);
// 	}

// 	return NULL;
// }

// DEFINE_FUNCTION_JUMP(CALL, 0x48D979, fake_MessageListClass__Add_Message);
// DEFINE_FUNCTION_JUMP(CALL, 0x55F0F5, fake_MessageListClass__Add_Message);
