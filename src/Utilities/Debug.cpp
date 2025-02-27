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
		const auto logDir = PhobosCRT::WideStringToString(Debug::LogFilePathName);
		Debug::FatalError("Uneable to find %s path !", logDir.c_str());
		return;
	}

	const auto log_full = PhobosCRT::WideStringToString(Debug::LogFileFullPath);
	spdlog::init_thread_pool(8192, 120);
	Debug::file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_full.c_str());
	Debug::sink2_vector.push_back(Debug::file_sink);
	Debug::dist_file_sink = std::make_shared<spdlog::sinks::dist_sink_mt>(Debug::sink2_vector);
	Debug::file_sink->set_level(spdlog::level::trace);
	Debug::g_MainLogger = std::make_shared<spdlog::async_logger>("main", Debug::dist_file_sink, spdlog::thread_pool(), spdlog::async_overflow_policy::block);
	Debug::g_MainLogger->set_level(spdlog::level::trace);
	spdlog::register_logger(g_MainLogger);
	spdlog::set_default_logger(g_MainLogger);
	Debug::g_MainLogger->info("Log File [{}]", log_full);

}

void Debug::DeactivateLogger()
{
	Debug::g_MainLogger->flush();
	spdlog::shutdown();
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

	Debug::g_MainLogger->info("Dumping {} bytes of stack", len);
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
		Debug::g_MainLogger->info("esp+{{0:x}} = {{1:x}} {} {}", i * 4, mem[i], suffix , Object);
	}

	Debug::g_MainLogger->info("====================Done."); // flushes
}

std::wstring Debug::PrepareSnapshotDirectory()
{
	const std::wstring buffer = Debug::LogFilePathName + L"\\snapshot-" + Debug::GetCurTime();
	if (!std::filesystem::create_directories(buffer)) {
		std::wstring msg = std::format(L"Log file failed to create snapshor dir. Error code = {}", errno);
		MessageBoxW(Game::hWnd.get(), Debug::LogFileFullPath.c_str(), msg.c_str(), MB_OK | MB_ICONEXCLAMATION);
		Phobos::Otamaa::ExeTerminated = true;
		ExitProcess(1);
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
			Debug::g_MainLogger->error("Fatal Error: {}", PhobosCRT::WideStringToString(DefaultFEMessage));

		Debug::FreeMouse();
		MessageBoxW(Game::hWnd, DefaultFEMessage.c_str(), L"Fatal Error - Yuri's Revenge", MB_OK | MB_ICONERROR);
	} else {

		if (log)
			Debug::g_MainLogger->error("Fatal Error: {}", msg);

		Debug::FreeMouse();
		MessageBoxA(Game::hWnd, msg.c_str(), "Fatal Error - Yuri's Revenge", MB_OK | MB_ICONERROR);
	}

	if (Dump) {
		Debug::FullDump();
	}

	if (Debug::ExitWithException) {
		CopyFileW(Debug::LogFileFullPath.c_str(), Debug::ExitWithExceptionCopyto.c_str(), FALSE);
		Debug::ExitWithException = false;
	}

	Debug::ExitGame();
}

void Debug::INIParseFailed(const char* section, const char* flag, const char* value, const char* Message)
{
	if (Phobos::Otamaa::TrackParserErrors && Debug::LogEnabled) {

		if (!Message) {
			Debug::g_MainLogger->error("[Phobos] Failed to parse INI file content: [{}]{}={}", section, flag, value);
		} else {
			Debug::g_MainLogger->error("[Phobos] Failed to parse INI file content: [{}]{}={} ({})", section, flag, value, Message);
		}

		Debug::RegisterParserError();
	}
}

