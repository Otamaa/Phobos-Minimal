#include "Debug.h"
#include "Macro.h"
#include <Phobos.h>


#include <MouseClass.h>
#include <Surface.h>

#include <CRT.h>
#include <YRPPCore.h>
#include <AbstractClass.h>
#include <vector>

#include <AnimClass.h>
#include <BuildingClass.h>
#include <BulletClass.h>
#include <UnitClass.h>
#include <InfantryClass.h>
#include <AircraftClass.h>
#include <WeaponTypeClass.h>
#include <WarheadTypeClass.h>
#include <HouseClass.h>
#include <HouseTypeClass.h>

#include <filesystem>

void Debug::InitLogger() {

	if (!std::filesystem::exists(Debug::LogFilePathName.c_str())) {
		Debug::FatalError("Uneable to find %ls path !", Debug::LogFilePathName.c_str());
		Debug::LogEnabled = false;
		return;
	}

	Debug::LogFile = _wfsopen(Debug::LogFileFullPath.c_str(), L"w", _SH_DENYWR);

	if (!LogFile) {
		Debug::LogEnabled = false;
		return;
	}

	Debug::Log("Log File [%ls].\n", Debug::LogFileFullPath.c_str());
}

void Debug::DeactivateLogger()
{
	if (Debug::LogFile) {
		fclose(Debug::LogFile);
		Debug::LogFile = nullptr;
		Debug::LogEnabled = false;
	}
}

void Debug::DetachLogger()
{
	if (Debug::LogFileActive() && Debug::made) {

		//Debug::g_MainLogger->info("Closing log file on program termination");

		Debug::DeactivateLogger();

		if (std::filesystem::exists(Debug::LogFileFullPath.c_str())) {
			CopyFileW(Debug::LogFileFullPath.c_str(), Debug::LogFileMainFormattedName.c_str(), FALSE);
		}
	}
}

void Debug::PrepareLogFile()
{ 
	if (!made) {
		wchar_t path[MAX_PATH];
		GetCurrentDirectoryW(MAX_PATH, path);
		Debug::ApplicationFilePath = path;
		Debug::LogFilePathName = path;
		Debug::LogFilePathName += L"\\debug";
		std::filesystem::path logDir = std::filesystem::path(Debug::LogFilePathName);
		std::filesystem::create_directories(logDir);
		Debug::LogFileFullPath = logDir.wstring() + (Debug::LogFileMainName + Debug::LogFileExt);
		Debug::LogFileMainFormattedName = Debug::LogFilePathName + Debug::LogFileMainName + L"." + GetCurTime() + Debug::LogFileExt;

		made = 1;
	}
}

void Debug::DumpStack(REGISTERS* R, size_t len, int startAt)
{
	if (!Debug::LogFileActive()) {
		return;
	}

	fprintf_s(Debug::LogFile, "Dumping %d bytes of stack\n" , len);
	auto const end = len / 4;
	auto const* const mem = R->lea_Stack<DWORD*>(startAt);
	for (auto i = 0u; i < end; ++i)
	{

		const char* suffix = "";
		const char* Object = "";
		const uintptr_t ptr = mem[i];
		if (ptr >= 0x401000 && ptr <= 0xB79BE4)
			suffix = "GameMemory!";
		else
		{
			for (auto begin = Patch::ModuleDatas.begin() + 1; begin != Patch::ModuleDatas.end(); ++begin)
			{
				if (ptr >= begin->BaseAddr && ptr <= (begin->BaseAddr + begin->Size))
				{
					suffix = (begin->ModuleName + " Memory!").c_str();
					break;
				}
			}
		}

		if (ptr != 0u && ptr != std::numeric_limits<uintptr_t>::max() && ptr != std::numeric_limits<uintptr_t>::min())
		{
			switch (VTable::Get((mem + i)))
			{
#define DECLARE_VTABLE_STRING(x) case x::vtable: Object = #x; break;
				DECLARE_VTABLE_STRING(AnimClass)
				DECLARE_VTABLE_STRING(UnitClass)
				DECLARE_VTABLE_STRING(AircraftClass)
				DECLARE_VTABLE_STRING(InfantryClass)
				DECLARE_VTABLE_STRING(BuildingClass)
				DECLARE_VTABLE_STRING(WeaponTypeClass)
				DECLARE_VTABLE_STRING(WarheadTypeClass)
				DECLARE_VTABLE_STRING(BulletClass)
				DECLARE_VTABLE_STRING(BulletTypeClass)
				DECLARE_VTABLE_STRING(HouseClass)
				DECLARE_VTABLE_STRING(HouseTypeClass)
#undef DECLARE_VTABLE_STRING
			default:
				break;
			}
		}
		fprintf_s(Debug::LogFile, "esp+%04X = %08X %s %s\n", i * 4, mem[i], suffix , Object);
	}

	fprintf_s(Debug::LogFile, "====================Done.\n");
	Debug::Flush();
}

std::wstring Debug::PrepareSnapshotDirectory()
{
	const std::wstring buffer = Debug::LogFilePathName + L"\\snapshot-" + Debug::GetCurTime();
	if (!std::filesystem::create_directories(buffer)) {
		std::wstring msg = std::format(L"Log file failed to create snapshor dir. Error code = {}", errno);
		MessageBoxW(Game::hWnd.get(), Debug::LogFileFullPath.c_str(), msg.c_str(), MB_OK | MB_ICONEXCLAMATION);
		Phobos::Otamaa::ExeTerminated = true;
		Debug::ExitGame(1);
	}

	return buffer;
}

void Debug::LogFileRemove() {
	if (std::filesystem::exists(Debug::LogFileFullPath.c_str())) {
		DeleteFileW(Debug::LogFileFullPath.c_str());
	}
}

void Debug::FreeMouse()
{
	Game::StreamerThreadFlush();
	const auto pMouse = MouseClass::Instance();

	if (pMouse)
	{
		const auto pMouseVtable = VTable::Get(pMouse);

		if (pMouseVtable == 0x7E1964)
		{
			pMouse->UpdateCursor(MouseCursorType::Default, false);
		}
	}

	const auto pWWMouse = WWMouseClass::Instance();

	if (pWWMouse)
	{
		const auto pWWMouseVtable = VTable::Get(pWWMouse);

		if (pWWMouseVtable == 0x7F7B2C)
		{
			pWWMouse->ReleaseMouse();
		}
	}

	ShowCursor(TRUE);

	auto const BlackSurface = [](DSurface* pSurface)
		{
			if (pSurface && VTable::Get(pSurface) == DSurface::vtable && pSurface->BufferPtr)
			{
				pSurface->Fill(0);
			}
		};

	BlackSurface(DSurface::Alternate);
	BlackSurface(DSurface::Composite);
	BlackSurface(DSurface::Hidden);
	BlackSurface(DSurface::Temp);
	BlackSurface(DSurface::Primary);
	BlackSurface(DSurface::Sidebar);
	BlackSurface(DSurface::Tile);

	ShowCursor(TRUE);
}

void Debug::FatalErrorCore(bool Dump, const std::string& msg)
{
	const bool log = Debug::LogFileActive();

	if (msg.empty()) {

		if (log)
			fprintf_s(Debug::LogFile, "Fatal Error: %ls\n", DefaultFEMessage.c_str());

		Debug::FreeMouse();
		MessageBoxW(Game::hWnd, DefaultFEMessage.c_str(), L"Fatal Error - Yuri's Revenge", MB_OK | MB_ICONERROR);
	} else {

		if (log)
			fprintf_s(Debug::LogFile, "Fatal Error: %s\n", msg.c_str());

		Debug::FreeMouse();
		MessageBoxA(Game::hWnd, msg.c_str(), "Fatal Error - Yuri's Revenge", MB_OK | MB_ICONERROR);
	}

	if (Dump) {
		Debug::FullDump();
	}
}

void Debug::INIParseFailed(const char* section, const char* flag, const char* value, const char* Message)
{
	if (Phobos::Otamaa::TrackParserErrors && Debug::LogEnabled) {

		if (!Message) {
			fprintf_s(Debug::LogFile, "[Phobos] Failed to parse INI file content: [%s]%s=%s.\n", section, flag, value);
		} else {
			fprintf_s(Debug::LogFile, "[Phobos] Failed to parse INI file content: [%s]%s=%s (%s).\n", section, flag, value, Message);
		}

		Debug::RegisterParserError();
	}
}

