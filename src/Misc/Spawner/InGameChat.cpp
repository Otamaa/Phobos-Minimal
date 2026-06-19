/**
*  yrpp-spawner
*
*  Copyright(C) 2022-present CnCNet
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.If not, see <http://www.gnu.org/licenses/>.
*/

#include "Main.h"
#include <Utilities/Macro.h>
#include <HouseClass.h>

// This corrects the processing of Unicode player names
// and prohibits incoming messages from players with whom chat is disabled

#pragma pack(push, 1)
struct GlobalPacket_NetMessage
{
	static COMPILETIMEEVAL reference<GlobalPacket_NetMessage, 0xA8D638u> const Instance {};

	int Command;
	wchar_t PlayerName[21];
	BYTE HouseIndex;
	BYTE ChatID;
	wchar_t Message[112];
	BYTE Color;
	BYTE CRC;
};
#pragma pack(pop)

struct DiplomacyChatToggleState
{
	static COMPILETIMEEVAL reference<DiplomacyChatToggleState, 0xA8D108u> const Instance {};

	byte ByHouse[8];
};

#pragma region DisableChat

static bool inline IsDisableChatEnabled()
{
	return SpawnerMain::GetGameConfigs()->DisableChat || !SpawnerMain::GetMainConfigs()->AllowChat;
}

// Continuously enforce DisableChat by resetting ChatMask every frame,
// preventing players from re-enabling chat via the alliances menu.
ASMJIT_PATCH(0x55EF38, MainLoop_AfterRender__DisableChat, 0x6)
{
	static int LastDisableChatFeedbackFrame = -1000;

	if (IsDisableChatEnabled()) {
		const int currentFrame = Unsorted::CurrentFrame;

		if (currentFrame < LastDisableChatFeedbackFrame)
			LastDisableChatFeedbackFrame = -1000; // new match started

		if (currentFrame - LastDisableChatFeedbackFrame >= 90) {
			MessageListClass::Instance->PrintMessage(L"Chat is disabled. Message not sent.");
			LastDisableChatFeedbackFrame = currentFrame;
		}

		return 0x55F056; // skip the send
	}

	return 0; // execute original: cmp edi, ebx; mov [esp+0x14], ebx
}

// After receiving a message, don't play sound if AddMessage returned NULL
// (i.e. the message was suppressed). Mirrors: hack 0x0048D97E in chat_disable.asm
ASMJIT_PATCH(0x48D97E, NetworkCallBack_NetMessage_Sound, 0x5)
{
	if(!SpawnerMain::GetMainConfigs()->AllowChat)
		return 0x48D99A;

	if (!R->EAX<void*>())
		return 0x48D99A; // skip sound

	return 0; // execute original: mov eax, [0x8871E0]
}

// In diplomacy dialog, make chat checkbox non-interactive for each player,
// matching the existing Player_MuteSWLaunches disabled-checkbox behavior.
// Replaces `mov eax, Player_MuteSWLaunches` (5 bytes at 0x657F8E).
// The subsequent `push 0; test eax, eax; jnz 0x657FC0` remains intact and
// branches to the disabled-checkbox path when EAX is non-zero.
ASMJIT_PATCH(0x657F8E, RadarClass_Diplomacy_DisableChatToggleUI, 0x5)
{
	R->EAX(IsDisableChatEnabled() ? 1 : Unsorted::MuteSWLaunches());
	return 0;
}

// The non-interactive branch (loc_657FC0) sets BM_SETCHECK(1) before disabling.
// Force it back to OFF for DisableChat so visuals match the intended locked state.
ASMJIT_PATCH(0x657FDB, RadarClass_Diplomacy_ForceDisabledChatVisualOff, 0x5)
{
	if (IsDisableChatEnabled()) {
		GET(HWND, hWnd, EBP);
		SendMessageA(hWnd, BM_SETCHECK, BST_UNCHECKED, 0);
	}

	return 0;
}

// Continuously enforce DisableChat by resetting ChatMask every frame,
// preventing re-enabling chat from the alliance menu.
ASMJIT_PATCH(0x55DDA5, MainLoop_AfterRender_DisableChat, 0x5)
{
	GET(MessageListClass*, pThis, ECX);

	pThis->Manage();

	if (IsDisableChatEnabled()) {
		for (int i = 0; i < 8; ++i)
			Game::ChatMask[i] = false;
	}

	return 0x55DDAA;
}

ASMJIT_PATCH(0x48D92B, NetworkCallBack_NetMessage_Print, 0x5)
{
	enum { SkipMessage = 0x48DAD3, PrintMessage = 0x48D937 };

	if (IsDisableChatEnabled())
		return SkipMessage;

	const int houseIndex = GlobalPacket_NetMessage::Instance->HouseIndex;

	if (houseIndex < 8 && Game::ChatMask[houseIndex]) {
		if (HouseClass* pHouse = HouseClass::Array->get_or_default(houseIndex)) {
			GlobalPacket_NetMessage::Instance->Color = (byte)pHouse->ColorSchemeIndex;
			R->ESI(pHouse->UIName);
			return PrintMessage;
		}
	}

	return SkipMessage;
}

ASMJIT_PATCH(0x48D95B, NetworkCallBack_NetMessage_SetColor, 0x6)
{
	R->EAX(R->ECX());
	return 0x48D966;
}

ASMJIT_PATCH(0x55EDD2, MessageInput_Write, 0x5)
{
	HouseClass* pHouse = HouseClass::CurrentPlayer;
	wcscpy_s(GlobalPacket_NetMessage::Instance->PlayerName, pHouse->UIName);
	GlobalPacket_NetMessage::Instance->HouseIndex = (byte)pHouse->ArrayIndex;

	return 0x55EE00;
}

ASMJIT_PATCH(0x55F0A8, MessageInput_Print, 0x5)
{
	R->EAX(GlobalPacket_NetMessage::Instance->PlayerName);
	return 0x55F0B2;
}
#pragma endregion