#include <string>
#include <Utilities/Macro.h>
#include <Utilities/Debug.h>
#include <HouseClass.h>
#include <HouseTypeClass.h>

void NOINLINE WriteHouseName(HouseClass* pHouse)
{
	switch (pHouse->GetSpawnPosition())
	{
	case 0:
	{
		CRT::strncpy(pHouse->PlainName, GameStrings::PlayerAt_A(), sizeof(pHouse->PlainName));
		break;
	}
	case 1:
	{
		CRT::strncpy(pHouse->PlainName, GameStrings::PlayerAt_B(), sizeof(pHouse->PlainName));
		break;
	}
	case 2:
	{
		CRT::strncpy(pHouse->PlainName, GameStrings::PlayerAt_C(), sizeof(pHouse->PlainName));
		break;
	}
	case 3:
	{
		CRT::strncpy(pHouse->PlainName, GameStrings::PlayerAt_D(), sizeof(pHouse->PlainName));
		break;
	}
	case 4:
	{
		CRT::strncpy(pHouse->PlainName, GameStrings::PlayerAt_E(), sizeof(pHouse->PlainName));
		break;
	}
	case 5:
	{
		CRT::strncpy(pHouse->PlainName, GameStrings::PlayerAt_F(), sizeof(pHouse->PlainName));
		break;
	}
	case 6:
	{
		CRT::strncpy(pHouse->PlainName, GameStrings::PlayerAt_G(), sizeof(pHouse->PlainName));
		break;
	}
	case 7:
	{
		CRT::strncpy(pHouse->PlainName, GameStrings::PlayerAt_H(), sizeof(pHouse->PlainName));
		break;
	}
	default:
	{
		if (pHouse->IsHumanPlayer)
			CRT::strncpy(pHouse->PlainName, GameStrings::human_player(), sizeof(pHouse->PlainName));
		else
			CRT::strncpy(pHouse->PlainName, GameStrings::Computer_(), sizeof(pHouse->PlainName));

		break;
	}
	}
}

DEFINE_HOOK(0x68804A, AssignHouses_PlayerHouses, 0x5)
{
	Debug::Log("Assinging player houses' names\n");

	GET(HouseClass*, pPlayerHouse, EBP);

	WriteHouseName(pPlayerHouse);

	return 0x68808E;
}

DEFINE_HOOK(0x688210, AssignHouses_ComputerHouses, 0x5)
{
	Debug::Log("Assinging computer houses' names\n");

	GET(HouseClass*, pAiHouse, EBP);

	WriteHouseName(pAiHouse);

	return 0x688252;
}
