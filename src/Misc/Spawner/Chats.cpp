#include "Main.h"
#include <Utilities/Macro.h>

#include <MessageListClass.h>
#include <HouseClass.h>

DEFINE_HOOK(0x55EF38, MassageClass_DisableChat_1, 0x6)
{
	if (!SpawnerMain::GetMainConfigs()->AllowChat)
		return 0x55F056;

	R->Stack(0x14, 0x0);
	return R->EDI<int>() > 0 ? 0x55EF48 : 0x55F056;
}

DEFINE_HOOK(0x48D97E, MassageClass_DisableChat_2, 0x5) {
	return !SpawnerMain::GetMainConfigs()->AllowChat ? 0x48D99A : 0x0;
}

void* __fastcall fake_MessageListClass__Add_Message(MessageListClass* pThis,
													DWORD,
													wchar_t* Name,
													int ID,
													wchar_t* message,
													int color,
													TextPrintType PrintType,
													int32_t duration,
													bool SinglePlayer)
{
	if (Name == NULL || !SpawnerMain::GetMainConfigs()->AllowChat) {
		return pThis->AddMessage(Name, ID, message, color, PrintType, duration, SinglePlayer);
	}

	if (_wcsicmp(Name, HouseClass::CurrentPlayer->UIName) == 0) {
		return pThis->AddMessage(0, 0, L"Chat is disabled. Message not sent.", 4, TextPrintType(0x4096), 270, 1);
	}

	return NULL;
}

DEFINE_JUMP(CALL, 0x48D979, GET_OFFSET(fake_MessageListClass__Add_Message));
DEFINE_JUMP(CALL, 0x55F0F5, GET_OFFSET(fake_MessageListClass__Add_Message));
