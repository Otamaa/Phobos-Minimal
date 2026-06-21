#include "Debug.h"
#include "Macro.h"

#include <Phobos.h>

#include <MouseClass.h>
#include <Surface.h>

#include <CRT.h>
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

#include <CCINIClass.h>
#include <SessionClass.h>
#include <ScenarioClass.h>

#include <filesystem>
#pragma region declarations

std::ofstream Debug::LogFile;
bool Debug::LogFileOpen {};

bool Debug::LogEnabled {};
std::wstring Debug::ApplicationFilePath {};
std::wstring Debug::DefaultFEMessage {};
std::wstring Debug::LogFilePathName {};
std::wstring Debug::LogFileMainName { L"debug" };
std::wstring Debug::LogFileMainFormattedName {};
std::wstring Debug::LogFileExt { L".log" };
std::wstring Debug::LogFileFullPath {};
std::wstring Debug::CrashDumpFileName { L"extcrashdump.dmp" };
std::string Debug::SyncFileFormat { "SYNC%01d.TXT" };
std::string Debug::SyncFileFormat2 { "SYNC%01d_%03d.TXT" };
char Debug::LogMessageBuffer[0x1000] {};
char Debug::DefferedVectorBuffer[0x1000] {};
std::vector<std::string> Debug::DefferedVector {};
bool Debug::made {};

#pragma endregion

#pragma region _MainFunc
#include <CriticalSection.h>

Debug::Result Debug::GetINIChecksums()
{
	Result nBuffer;
	if (SessionClass::Instance->GameMode != GameMode::LAN)
	{
		nBuffer = { CCINIClass::RulesHash() , CCINIClass::ArtHash() ,  CCINIClass::AIHash() };
	}
	else
	{
		nBuffer = { CCINIClass::RulesHash_Internet() , CCINIClass::ArtHash_Internet() ,  CCINIClass::AIHash_Internet() };
	}

	if (!nBuffer.Rules)
		nBuffer.Rules = ScenarioClass::GetRulesUniqueID();

	if (!nBuffer.Art)
		nBuffer.Art = ScenarioClass::GetArtUniqueID();

	if (!nBuffer.AI)
		nBuffer.AI = ScenarioClass::GetAIUniqueID();

	return nBuffer;
}

void Debug::ApplyHooks() {
	Debug::GenerateDefaultMessage();
	Debug::PrepareLogFile(); //prepare directory
	Debug::LogFileRemove(); //remove previous debug log file if presents
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
		std::error_code ec;
		std::filesystem::create_directories(logDir, ec);

		if (ec)
		{
			Debug::FatalErrorAndExit("Failedtocreate dir %ls reason %s!\n", Debug::LogFilePathName.c_str(), ec.message().c_str());
			return;
		}

		Debug::LogFileFullPath = logDir.wstring() + L"\\" + (Debug::LogFileMainName + Debug::LogFileExt);
		Debug::LogFileMainFormattedName = Debug::LogFilePathName + L"\\" + Debug::LogFileMainName + L"." + GetCurTime() + Debug::LogFileExt;

		made = 1;
	}
}

void Debug::Log_Raw(DebugType type, const char* file, const char* function, int line, std::string_view message)
{
	static SimpleCriticalSectionClass DebugMutex;
	ScopedCriticalSectionClass mutex(&DebugMutex);

	char buffer[4096];
	char filebuff[4096];
	bool write_to_file = false;

	/**
	 *  Copy the incoming message into a null-terminated stack buffer so the
	 *  existing snprintf / ofstream plumbing below can be reused unchanged.
	 *  Truncation matches the historical 4 KB limit of Vinifera_Printf.
	 */
	const size_t len = std::min(message.size(), sizeof(buffer) - 1);
	std::memcpy(buffer, message.data(), len);
	buffer[len] = '\0';

	/**
	 *  Strip path from "file".
	 */
	if (file != nullptr) {
		file = (std::strrchr(file, '\\') ? std::strrchr(file, '\\') + 1 : file);
	}

	switch (type) {
	case DebugType::GAME:
	case DebugType::NORMAL:
	case DebugType::INFO:
	{
		IMPL_SNPRNINTF(filebuff, sizeof(filebuff), "[INFO] %s", buffer);

		write_to_file = true;

		break;
	}

	case DebugType::WARN:
	{
		IMPL_SNPRNINTF(filebuff, sizeof(filebuff), "[WARNING] %s", buffer);

		write_to_file = true;

		break;
	}

	case DebugType::ERR:
	{
		IMPL_SNPRNINTF(filebuff, sizeof(filebuff), "[ERROR] %s", buffer);

		write_to_file = true;

		break;
	}

	case DebugType::FATAL:
	{
		IMPL_SNPRNINTF(filebuff, sizeof(filebuff), "[FATAL] %s", buffer);

		write_to_file = true;

		break;
	}

	case DebugType::TRACE:
	{
		IMPL_SNPRNINTF(filebuff,
			sizeof(filebuff),
			"[TRACE] File: %s\n"
			"  Func: %s\n"
			"  Line: %d\n"
			"  Msg:  %s"
			"\n"
			,
			file, function, line, buffer);

		break;
	}

	default: break;
	};

	/**
	 *  Write the log file if flagged to do so.
	 */
	if (write_to_file && LogEnabled) {

		if (!LogFileOpen) {
			LogFile.open(Debug::LogFileFullPath, std::ios::app | std::ios::binary);
			LogFileOpen = true;
		}

		/**
		 *  Write the buffer to the log file.
		 */
		LogFile << filebuff;

		if (LogFileOpen) {
			LogFile.close();
			LogFileOpen = false;
		}
	}
}

void Debug::InitLogger() {

	if (!std::filesystem::exists(Debug::LogFilePathName.c_str())) {
		Debug::FatalError("Uneable to find %ls path !", Debug::LogFilePathName.c_str());
		Debug::LogEnabled = false;
		return;
	}

	Debug::Log("Log File [%ls].\n", Debug::LogFileFullPath.c_str());
}

void Debug::DeactivateLogger() {
	if (LogFileOpen) {
		LogFile.close();
		LogFileOpen = false;
		LogEnabled = false;
	}
}

void Debug::DetachLogger() {
	if (Debug::LogEnabled && Debug::made) {

		//Debug::g_MainLogger->info("Closing log file on program termination");

		Debug::DeactivateLogger();

		if (std::filesystem::exists(Debug::LogFileFullPath.c_str())) {
			CopyFileW(Debug::LogFileFullPath.c_str(), Debug::LogFileMainFormattedName.c_str(), FALSE);
		}
	}
}

std::wstring Debug::PrepareSnapshotDirectory() {
	const std::wstring buffer = Debug::LogFilePathName + L"\\snapshot-" + Debug::GetCurTime();
	std::error_code ec;
	std::filesystem::create_directories(buffer, ec);

	if (ec) {
		std::wstring msg = fmt::format(L"Log file failed to create snapshor dir {} .\n Error code = {}",
			Debug::LogFileFullPath, PhobosCRT::StringToWideString(ec.message()));

		MessageBoxW(Game::hWnd.get(), msg.c_str(), L"Error!", MB_OK | MB_ICONEXCLAMATION);
		Phobos::ExeTerminate();
		exit(errno);
	}

	return buffer;
}

void Debug::LogFileRemove() {
	if (std::filesystem::exists(Debug::LogFileFullPath.c_str())) {
		DeleteFileW(Debug::LogFileFullPath.c_str());
	}
}

void Debug::FreeMouse() {
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
	const bool log = Debug::LogEnabled;

	if (msg.empty())
	{

		if (log && Debug::LogEnabled)
		{
			char tracebuff[4096];
			IMPL_SNPRNINTF(tracebuff,
				sizeof(tracebuff),
				"[FATAL]  %ls", DefaultFEMessage.c_str());

			if (!LogFileOpen)
			{
				LogFile.open(Debug::LogFileFullPath, std::ios::app | std::ios::binary);
				LogFileOpen = true;
			}

			/**
			 *  Write the buffer to the log file.
			 */
			LogFile << tracebuff;

			if (LogFileOpen)
			{
				LogFile.close();
				LogFileOpen = false;
			}
		}

		Debug::FreeMouse();
		MessageBoxW(Game::hWnd, DefaultFEMessage.c_str(), L"Fatal Error - Yuri's Revenge", MB_OK | MB_ICONERROR);
	}
	else
	{

		if (log && Debug::LogEnabled)
		{
			char tracebuff[4096];
			IMPL_SNPRNINTF(tracebuff,
				sizeof(tracebuff),
				"[FATAL]  %s", msg.c_str());

			if (!LogFileOpen)
			{
				LogFile.open(Debug::LogFileFullPath, std::ios::app | std::ios::binary);
				LogFileOpen = true;
			}

			/**
			 *  Write the buffer to the log file.
			 */
			LogFile << tracebuff;

			if (LogFileOpen)
			{
				LogFile.close();
				LogFileOpen = false;
			}
		}

		Debug::FreeMouse();
		MessageBoxA(Game::hWnd, msg.c_str(), "Fatal Error - Yuri's Revenge", MB_OK | MB_ICONERROR);
	}

	if (Dump)
	{
		Debug::FullDump();
	}
}

#pragma endregion

void Debug::INIParseFailed(const char* section, const char* flag, const char* value, const char* Message)
{
	if (Phobos::Otamaa::TrackParserErrors && Debug::LogEnabled) {

		std::string formatted;
		if (!Message) {
			formatted = fmt::format("[Phobos] Failed to parse INI file content: [{}]{}={}.\n", section, flag, value);
		} else {
			formatted = fmt::format("[Phobos] Failed to parse INI file content: [{}]{}={} ({}).\n", section, flag, value, Message);
		}
		Debug::Log_Raw(DebugType::WARN, nullptr, nullptr, -1, formatted);
		Debug::RegisterParserError();
	}
}

void Debug::Log(const char* pFormat, ...)
{
	if (Debug::LogEnabled)
	{
		char buffer[4096];
		va_list args;
		va_start(args, pFormat);
		std::vsnprintf(buffer, sizeof(buffer), pFormat, args);
		va_end(args);

		Log_Raw(DebugType::GAME, nullptr, nullptr, -1, buffer);
	}
}

//this will be used to replace game debug prints
void __cdecl Debug::CLog(const char* pFormat, ...)
{
	if (Debug::LogEnabled)
	{
		char buffer[4096];
		va_list args;
		va_start(args, pFormat);
		std::vsnprintf(buffer, sizeof(buffer), pFormat, args);
		va_end(args);

		Log_Raw(DebugType::GAME, nullptr, nullptr, -1, buffer);
	}
}


void Debug::LogDeferred(const char* pFormat, ...)
{
	va_list args;
	va_start(args, pFormat);
	vsprintf_s(DefferedVectorBuffer, sizeof(DefferedVectorBuffer), pFormat, args);
	Debug::DefferedVector.emplace_back(DefferedVectorBuffer);
	va_end(args);
}

void Debug::LogDeferredFinalize()
{
	if (Debug::LogEnabled) {
		for (auto& __log : Debug::DefferedVector) {
			if (!__log.empty()) {
				Log_Raw(DebugType::GAME, nullptr, nullptr, -1, __log);
			}
		}
	}

	Debug::DefferedVector.clear();
}

void Debug::RegisterParserError()
{
	if (Phobos::Otamaa::TrackParserErrors)
	{
		Phobos::Otamaa::ParserErrorDetected = true;
	}
}