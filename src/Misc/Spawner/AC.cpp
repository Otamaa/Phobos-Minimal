#include <Misc/Hooks.Otamaa.h>
#include <Utilities/Debug.h>

#include <BombClass.h>
#include <EventClass.h>

ASMJIT_PATCH(0x4FA35D, L48XN65KU0NYTS523HY8GJEDY63KH, 0x0)
{
	GET_STACK(AbstractType, type, 0x28);
	GET(int, idx, EBP);
	GET(HouseClass*, pHouse, ECX);

	if (idx >= 0)
	{
		auto pTechnoType = ObjectTypeClass::FetchTechnoType(type, idx);
		
		if (pHouse->CanBuild(pTechnoType, false, false) == CanBuildResult::Buildable) {
			return 0x4FA36D;
		}

		Debug::Log("House ID %d tried to produce %s illegaly!\n", pHouse->ArrayIndex, pTechnoType->ID);
		R->EAX(ProdFailType::Illegal);
		return 0x4FA4A0;
	}

	Debug::Log("Heap ID %d is invalid!\n", idx);
	R->EAX(ProdFailType::Illegal);
	return 0x4FA4A0;
}

ASMJIT_PATCH(0x4C71D2, KD873HWYFB7DN36G67DUY4BGT32GD, 0x5)
{
	GET(TechnoClass*, pTechno, EAX);
	GET(EventClass*, pEvent, ESI);

	if (pTechno && pTechno->GetControllingHouse() == pEvent->HouseIndex)
		return 0x0;

	return 0x4C8109;
}ASMJIT_PATCH_AGAIN(0x4C6DB2, KD873HWYFB7DN36G67DUY4BGT32GD, 0x6)
ASMJIT_PATCH_AGAIN(0x4C6F1A, KD873HWYFB7DN36G67DUY4BGT32GD, 0x6)
ASMJIT_PATCH_AGAIN(0x4C6EDA, KD873HWYFB7DN36G67DUY4BGT32GD, 0x6)
ASMJIT_PATCH_AGAIN(0x4C7194, KD873HWYFB7DN36G67DUY4BGT32GD, 0x6)
ASMJIT_PATCH_AGAIN(0x4C76C4, KD873HWYFB7DN36G67DUY4BGT32GD, 0x6)
ASMJIT_PATCH_AGAIN(0x4C7861, KD873HWYFB7DN36G67DUY4BGT32GD, 0x6)
ASMJIT_PATCH_AGAIN(0x4C74D3, KD873HWYFB7DN36G67DUY4BGT32GD, 0x6)
ASMJIT_PATCH_AGAIN(0x4C6D55, KD873HWYFB7DN36G67DUY4BGT32GD, 0x6)
ASMJIT_PATCH_AGAIN(0x4C6CF8, KD873HWYFB7DN36G67DUY4BGT32GD, 0x6)

static int Original;

ASMJIT_PATCH(0x64A0E4, DJJ83UN78EG23K69NR47DH, 0x5)
{
	Original = R->EDX();
	return 0;
}

ASMJIT_PATCH(0x64BE75, B8DM32NIB8J4ND8DNBNTIBIE, 0x8)
{
	GET_STACK(int, expected, 0x3E);

	if (Original != expected){
		Debug::Log("Forged house ID detected. Expected: %d, Actual: %d\n", Original, expected);
		return 0x64C370;
	}

	return 0;
}

ASMJIT_PATCH(0x4C782B, B63N75N698FNE63JF87EN, 0x6)
{
	GET(TechnoClass*, pTarget, EAX);
	GET(EventClass*, pEvent, ESI);

	if (!pTarget)
		return 0x4C8109;

	auto pBomb = pTarget->AttachedBomb;
	if (!pBomb)
		return 0x4C8109;

	HouseClass* pBombOwner = pBomb->OwnerHouse;

	int houseArray = 0;
	if (SessionClass::Instance->GameMode != GameMode::Campaign)
	{
		if (pBombOwner != HouseClass::CurrentPlayer()) {
			houseArray = pBombOwner->ArrayIndex;
		} else {
			houseArray = HouseClass::CurrentPlayer->ArrayIndex;
		}
	}
	else
	{
		if (!pBombOwner->IsHumanPlayer && !pBombOwner->IsInPlayerControl) {
			houseArray = pBombOwner->ArrayIndex;
		}
		else if (HouseClass::CurrentPlayer()) {
			houseArray = HouseClass::CurrentPlayer->ArrayIndex;
		} else {
			houseArray = pBombOwner->ArrayIndex;
		}
	}

	return houseArray == pEvent->HouseIndex ? 0x0 : 0x4C8109;
}
