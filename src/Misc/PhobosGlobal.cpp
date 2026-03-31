#include "PhobosGlobal.h"

#include <AbstractClass.h>

PhobosGlobal PhobosGlobal::GlobalObject;

#include <Utilities/GameConfig.h>

#include <VersionHelpers.h>

void PhobosGlobal::Clear()
{
	auto pInstance = PhobosGlobal::Instance();
	pInstance->DetonateDamageArea = true;

	pInstance->TempFoundationData1.clear();
	pInstance->TempFoundationData2.clear();
	pInstance->TempCoveredCellsData.clear();
	pInstance->ColorDatas.reset();
	pInstance->PathfindTechno.Clear();
	pInstance->CurCopyArray.clear();
	pInstance->LandTypeParseCounter = 0;
	pInstance->ObjectLinkedAlphas.clear();
	pInstance->ShpCompression1Buffer.clear();
	pInstance->TriggerCounts.clear();
	pInstance->LastAnimName.clear();
	pInstance->PipDatas.clear();
}

void PhobosGlobal::PointerGotInvalid(AbstractClass* ptr, bool removed)
{
	auto pInstance = PhobosGlobal::Instance();
	pInstance->PathfindTechno.InvalidatePointer(ptr , removed);
	for (auto& copyArr : pInstance->CurCopyArray) {
		copyArr.second.Invalidate(ptr, removed);
	}
}

bool PhobosGlobal::SaveGlobals(PhobosStreamWriter& stm) { return PhobosGlobal::Instance()->Serialize(stm); }
bool PhobosGlobal::LoadGlobals(PhobosStreamReader& stm)
{

	if (PhobosGlobal::Instance()->Serialize(stm))
	{
		if (Unsorted::CursorSize())
		{
			Unsorted::CursorSize = PhobosGlobal::Instance()->TempFoundationData1.data();
		}

		if (Unsorted::CursorSizeSecond())
		{
			Unsorted::CursorSizeSecond = PhobosGlobal::Instance()->TempFoundationData2.data();
		}
		return true;
	}

	return false;
}

void PhobosGlobal::LoadGlobalsConfig()
{
	GameConfig ares_ini { "Ares.ini" };
	ares_ini.OpenINIAction([](CCINIClass* pINI)
 {
	 if (pINI->ReadString("Graphics.Advanced", "DirectX.Force", Phobos::readDefval, Phobos::readBuffer))
	 {
		 if (IS_SAME_STR_(Phobos::readBuffer, "hardware"))
		 {
			 Phobos::Config::GFX_DX_Force = 0x01l; //HW
		 }
		 else if (IS_SAME_STR_(Phobos::readBuffer, "emulation"))
		 {
			 Phobos::Config::GFX_DX_Force = 0x02l; //EM
		 }
	 }
	});

	if (IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_VISTA), LOBYTE(_WIN32_WINNT_VISTA), 0))
	{
		Phobos::Config::GFX_DX_Force = 0;
	}
}