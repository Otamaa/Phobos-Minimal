#include "Body.h"

#include <ASMMacros.h>
#include <Phobos.h>

class TechnoClass;
class TechnoTypeClass;
class SuperWeaponTypeClass;

DWORD AresData::HandleConvert::CallableAddress = 0x0;
DWORD AresData::HandleConvert::Offset = 0x44130;

DWORD AresData::ContainerMap_Find::CallableAddress = 0x0;
DWORD AresData::ContainerMap_Find::Offset = 0x58900;
DWORD AresData::ContainerMapData::SWTypeContainerOffSet = 0xC2C50;
DWORD AresData::ContainerMapData::SWTypeContainer = 0x0;

void AresData::Init()
{
	HandleConvert::CallableAddress = Phobos::AresBaseAddress + HandleConvert::Offset;
	ContainerMap_Find::CallableAddress = Phobos::AresBaseAddress + ContainerMap_Find::Offset;
	ContainerMapData::SWTypeContainer = Phobos::AresBaseAddress + ContainerMapData::SWTypeContainerOffSet;
}

void __stdcall AresData::HandleConvert::Exec(TechnoClass*, TechnoTypeClass*) {
	JMP_STD(HandleConvert::CallableAddress);
}

//template<typename T>
//void* __stdcall AresData::ContainerMap_Find::Exec(DWORD, T) {
//	JMP_STD(ContainerMap_Find::CallableAddress);
//
//}

//template<>
void* __fastcall AresData::ContainerMap_Find::Exec(DWORD* c, SuperWeaponTypeClass* p) {
	EPILOG_THISCALL;
	_asm {mov ecx, c}
	PUSH_IMM(p);
	JMP(ContainerMap_Find::CallableAddress);
}

uint32_t const Ares::BaseAddress = reinterpret_cast<uint32_t>(GetModuleHandleW(ARES_DLL));
