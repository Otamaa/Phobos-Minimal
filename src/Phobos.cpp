#include <Phobos.h>

#include <Phobos.lib.h>

#ifdef ENABLE_CLR
#include <Scripting/CLR.h>
#endif

#include <CCINIClass.h>
#include <Unsorted.h>
#include <Drawing.h>
#include <filesystem>

#include <Utilities/Macro.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/Debug.h>
#include <Utilities/Patch.h>
#include <Utilities/GameConfig.h>

#include <Misc/Patches.h>

#include <Misc/PhobosGlobal.h>

#include <Misc/Ares/Hooks/Header.h>
#include <Misc/Spawner/Main.h>

#include <Dbghelp.h>
#include <tlhelp32.h>
#include <winternl.h>
#include <cfenv>
#include <WinBase.h>
#include <CD.h>
#include <aclapi.h>
#include <GameOptionsClass.h>

#include <Phobos.Lua.h>
#include <Phobos.UI.h>
#include <Phobos.Defines.h>

#include <mutex>
#include <unordered_map>

#include <Lib/asmjit/x86.h>


const char* GetGAPIRuntime(){

	for (auto& dll : Patch::ModuleDatas) {
		if (_strnicmp(dll.ModuleName.c_str(), "d3d", 3) == 0
			|| _stricmp(dll.ModuleName.c_str(),"dxgi") == 0)
			return "DirectX";
		else if (IS_SAME_STR_("opengl32.dll", dll.ModuleName.c_str()))
			return "OpenGL";
		else if (IS_SAME_STR_("vulkan.dll", dll.ModuleName.c_str()))
			return "Vulkan";

		else "Unknown";
	}
}

std::unique_ptr<asmjit::JitRuntime> gJitRuntime;

class asmjitErrHandler : public asmjit::ErrorHandler
{
public:
	void handleError(asmjit::Error err, const char* message, asmjit::BaseEmitter* origin) override {
		Debug::LogDeferred("AsmJit ERROR: %s\n", message);
	}
};

asmjitErrHandler gJitErrorHandler;

namespace Assembly
{
	static constexpr BYTE INIT = 0x00,
		INT3 = 0xCC,
		NOP = 0x90,
		CALL = 0xE8,
		JMP = 0xE9,
		JLE = 0x7E;
};
struct HookSummary {
	const void* func;
	size_t size;
};
std::map<unsigned int, std::vector<HookSummary>> Hooks { };

void ApplyasmjitPatch() {

	for (auto& hook : Hooks) {

		if (hook.second.empty()) {
			Debug::LogDeferred("hook at 0x%x is empty !\n", hook.first);
			continue;
		}
			

		if (hook.second.size() > 1) {
			Debug::LogDeferred("hook at 0x%x , has %d functions registered !\n", hook.first, hook.second.size());
		}
		size_t hook_size = hook.second[0].size;
		asmjit::CodeHolder code;
		code.init(gJitRuntime->environment(), gJitRuntime->cpuFeatures());
		code.setErrorHandler(&gJitErrorHandler);
		asmjit::x86::Assembler assembly(&code);
		asmjit::Label l_origin = assembly.newLabel();
		DWORD hookSize = MaxImpl(hook_size, 5u);

		for (auto& hook_fn : hook.second) {
			assembly.pushad();
			assembly.pushfd();
			assembly.push(hook.first);
			assembly.sub(asmjit::x86::esp, 4);
			assembly.lea(asmjit::x86::eax, asmjit::x86::ptr(asmjit::x86::esp, 4));
			assembly.push(asmjit::x86::eax);
			assembly.call(hook_fn.func);
			assembly.add(asmjit::x86::esp, 0xC);
			assembly.mov(asmjit::x86::ptr(asmjit::x86::esp, -8), asmjit::x86::eax);
			assembly.popfd();
			assembly.popad();
			assembly.cmp(asmjit::x86::dword_ptr(asmjit::x86::esp, -0x2C), 0);
			assembly.jz(l_origin);
			assembly.jmp(asmjit::x86::ptr(asmjit::x86::esp, -0x2C));
		
		}

		assembly.bind(l_origin);
		void* hookAddress = (void*)hook.first;

		std::vector<byte> originalCode(hookSize);
		memcpy(originalCode.data(), hookAddress, hookSize);

		// fix relative jump or call
		if (originalCode[0] == Assembly::CALL || originalCode[0] == Assembly::JMP)
		{
			DWORD dest = hook.first + 5 + *(DWORD*)(originalCode.data() + 1);
			switch (originalCode[0])
			{
			case Assembly::JMP: // jmp
				assembly.jmp(dest);
				originalCode.erase(originalCode.begin(), originalCode.begin() + 5);
				Debug::LogDeferred("hook at 0x%x is placed at JMP fixing the relative addr !\n", hook.first);
				break;
			case Assembly::CALL: // call
				assembly.call(dest);
				originalCode.erase(originalCode.begin(), originalCode.begin() + 5);
				Debug::LogDeferred("hook at 0x%x is placed at CALL fixing the relative addr !\n", hook.first);

				break;
			}
		}
		assembly.embed(originalCode.data(), originalCode.size());
		assembly.jmp(hook.first + hookSize);
		const void* fn {};
		gJitRuntime->add(&fn, &code);
		code.reset();
		code.init(gJitRuntime->environment(), gJitRuntime->cpuFeatures());
		code.setErrorHandler(&gJitErrorHandler);
		code.attach(&assembly);
		assembly.jmp(fn);
		code.flatten();
		code.resolveUnresolvedLinks();
		code.relocateToBase(hook.first);

		DWORD protect_flag {};
		DWORD protect_flagb {};
		VirtualProtect(hookAddress, hookSize, PAGE_EXECUTE_READWRITE, &protect_flag);
		code.copyFlattenedData(hookAddress, hookSize);
		VirtualProtect(hookAddress, hookSize, protect_flag, &protect_flagb);
		FlushInstructionCache(Game::hInstance, hookAddress, hookSize);
	}

	Hooks.clear();
}

void Initasmjit()
{
	gJitRuntime = std::make_unique<asmjit::JitRuntime>();

	void* buffer {};
	int len = Patch::GetSection(Phobos::hInstance, ASMJIT_PATCH_SECTION_NAME, &buffer);

	hookdeclb* end = (hookdeclb*)((DWORD)buffer + len);
	Debug::LogDeferred("Applying %d asmjit hooks.\n", std::distance((hookdeclb*)buffer, end));

	for (hookdeclb* begin = (hookdeclb*)buffer; begin < end; begin++) {
		auto& hook = Hooks[begin->hookAddr];
		hook.emplace_back(begin->hookFunc, begin->hookSize);
	}

	ApplyasmjitPatch();
}

#ifdef EXPERIMENTAL_IMGUI
DEFINE_HOOK(0x5D4E66, Windows_Message_Handler_Add, 0x7)
{
	PhobosWindowClass::Callback();
	return 0x0;
}
#endif

#pragma region DEFINES

#ifdef ENABLE_TLS
DWORD TLS_Thread::dwTlsIndex_SHPDRaw_1;
DWORD TLS_Thread::dwTlsIndex_SHPDRaw_2;
#endif

#pragma endregion

#pragma region PhobosFunctions
void Phobos::CheckProcessorFeatures()
{
#if _M_IX86_FP != 2 //only SSE
	static_assert(false, "Phobos compiled using unsupported architecture.");
#endif

	const BOOL supported = IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE);
	Debug::Log("Phobos requires a CPU with SSE support. %s.\n",
		supported ? "Available" : "Not available");

	if (!supported)
	{
		//doesnot get inlined when using reference<>
		MessageBoxA(Game::hWnd.get(),
			"This version of Phobos requires a CPU with SSE"
			" support.\n\nYour CPU does not support SSE. "
			"Game will now exit.",
			"Phobos - CPU Requirements", MB_ICONERROR);

		Debug::Log("Game will now exit.\n");
		Debug::ExitGame(533u);
	}
}

void Phobos::PassiveSaveGame()
{
	GeneralUtils::PrintMessage(StringTable::LoadString(GameStrings::TXT_SAVING_GAME));
	const auto name = "Map." + Debug::GetCurTimeA() + ".sav";

	if (ScenarioClass::SaveGame(name.c_str(), Phobos::CustomGameSaveDescription.c_str()))
		GeneralUtils::PrintMessage(StringTable::LoadString(GameStrings::TXT_GAME_WAS_SAVED));
	else
		GeneralUtils::PrintMessage(StringTable::LoadString(GameStrings::TXT_ERROR_SAVING_GAME));
}

void Phobos::CmdLineParse(char** ppArgs, int nNumArgs)
{
	DWORD_PTR processAffinityMask = 1; // limit to first processor
	bool dontSetExceptionHandler = false;

	// > 1 because the exe path itself counts as an argument, too!
	std::string args {};
	for (int i = 1; i < nNumArgs; i++)
	{
		const auto pArg = ppArgs[i];
		args += " ";
		args += pArg;

		if (IS_SAME_STR_I(pArg, "-Icon"))
		{
			Phobos::AppIconPath = ppArgs[++i];
		}
		else if (IS_SAME_STR_I(pArg, "-LOG"))
		{
			Debug::LogEnabled = true;
		}
		else if (IS_SAME_STR_I(pArg, "-AI-CONTROL"))
		{
			Phobos::Otamaa::AllowAIControl = true;
		}
		else if (IS_SAME_STR_I(pArg, "-LOG-CSF"))
		{
			Phobos::Otamaa::OutputMissingStrings = true;
		}
		else if (IS_SAME_STR_I(pArg, "-LOG-AUDIO"))
		{
			Phobos::Otamaa::OutputAudioLogs = true;
		}
		else if (IS_SAME_STR_I(pArg, "-STRICT"))
		{
			Phobos::Otamaa::StrictParser = true;
		}
		else if (IS_SAME_STR_I(pArg, "-NOLOGO"))
		{
			Phobos::Otamaa::NoLogo = true;
		}
		else if (IS_SAME_STR_I(pArg, "-CD"))
		{
			Phobos::Otamaa::NoCD = true;
		}
		else if (IS_SAME_STR_I(pArg, "-Inheritance"))
		{
			Phobos::Config::UseNewInheritance = true;
		}
		else if (IS_SAME_STR_I(pArg, "-Include"))
		{
			Phobos::Config::UseNewIncludes = true;
		}
		else if (IS_SAME_STR_I(pArg, "-b=" _STR(BUILD_NUMBER)))
		{
			Phobos::Config::HideWarning = true;
		}
		else if (IS_SAME_STR_I(pArg, "-EXCEPTION") == 0)
		{
			ExceptionMode = ExceptionHandlerMode::NoRemove;
		}
		else if (!strncasecmp(pArg, "-AFFINITY:", 0xAu))
		{
			int result = 1;
			if (Parser<int>::Parse(pArg + 0xAu, &result) && result < 0) {
				result = 0;
			}

			processAffinityMask = result;
		} else {
			const std::string cur = pArg;
			if (cur.starts_with("-ExceptionHandler=")) {

				const size_t delim = cur.find("=");
				const std::string value = cur.substr(delim + 1, cur.size() - delim - 1);

				if (!value.empty()) {
					Parser<bool>::Parse(value.data(), &dontSetExceptionHandler);
				}
			}
		}

		SpawnerMain::CmdLineParse(pArg);
	}

	if (Debug::LogEnabled) {
		Debug::InitLogger(); //init the real logger

		Debug::Log("Initialized Phobos " PRODUCT_VERSION ".\n");
		Debug::Log("args %s\n", args.c_str());

		SpawnerMain::PrintInitializeLog();
	} else {
		Debug::DeactivateLogger();
		Debug::LogFileRemove();
		Debug::made = false;// reset
	}

#ifdef _Enable_these
	SID_IDENTIFIER_AUTHORITY _ID {};
	HANDLE _Token {};
	DWORD _RetLength {};
	PSID _PID {};

	if (AllocateAndInitializeSid(&_ID, 1u, 0, 0, 0, 0, 0, 0, 0, 0, &_PID))
	{
		if (OpenProcessToken(Patch::CurrentProcess, 8u, &_Token))
		{
			GetTokenInformation(_Token, TokenUser, 0, 0, &_RetLength);
			if (_RetLength <= 0x400)
			{
				HLOCAL _Alloc = LocalAlloc(0x40u, 0x400u);
				if (GetTokenInformation(_Token, TokenUser, _Alloc, 0x400u, &_RetLength))
				{
					ACL _Acl {};
					if (InitializeAcl(&_Acl, 0x400u, 2u)
					  && AddAccessDeniedAce(&_Acl, 2u, 0xFAu, _PID)
					  && AddAccessAllowedAce(&_Acl, 2u, 0x100701u, _PID))
					{
						SetSecurityInfo(Patch::CurrentProcess, SE_KERNEL_OBJECT, 0x80000004, 0, 0, &_Acl, 0);
					}
				}
			}
		}
	}

	if (_PID)
		FreeSid(_PID);
#endif

	Phobos::CheckProcessorFeatures();

	Game::DontSetExceptionHandler = dontSetExceptionHandler;
	Debug::Log("ExceptionHandler is %s .\n", dontSetExceptionHandler ? "not present" : "present");

	if (processAffinityMask)
	{
		Debug::Log("Set Process Affinity: %d (%d).\n", processAffinityMask, processAffinityMask);
		SetProcessAffinityMask(Patch::CurrentProcess, processAffinityMask);
	}

	if (!CDDriveManagerClass::Instance->NumCDDrives)
	{
		Debug::Log("No CD drives detected. Switching to NoCD mode.\n");
		Phobos::Otamaa::NoCD = true;
	}

	if (Phobos::Otamaa::NoCD)
	{
		Debug::Log("Optimizing list of CD drives for NoCD mode.\n");
		std::memset(CDDriveManagerClass::Instance->CDDriveNames, -1, 26);

		char drv[] = "a:\\";
		for (int i = 0; i < 26; ++i)
		{
			drv[0] = 'a' + (i + 2) % 26;
			if (GetDriveTypeA(drv) == DRIVE_FIXED)
			{
				CDDriveManagerClass::Instance->CDDriveNames[0] = (i + 2) % 26;
				CDDriveManagerClass::Instance->NumCDDrives = 1;
				break;
			}
		}
	}
}

void Phobos::ThrowUsageWarning(CCINIClass* pINI)
{
	//there is only few mod(s) that using this
	//just add your mod name or remove these code if you dont like it
	if (!Phobos::Otamaa::IsAdmin)
	{
		if(pINI->ReadString(GameStrings::General(), GameStrings::Name, "", Phobos::readBuffer) <= 0)
			return;

		const std::string ModNameTemp = Phobos::readBuffer;

		if (!ModNameTemp.empty())
		{
			std::size_t nFInd = ModNameTemp.find("Rise of the East");
			if (nFInd == std::string::npos)
				nFInd = ModNameTemp.find("Ion Shock");

			if (nFInd == std::string::npos)
				nFInd = ModNameTemp.find("New War");

			if (nFInd == std::string::npos)
				nFInd = ModNameTemp.find("NewWar");

			if (nFInd == std::string::npos)
			{
				MessageBoxW(NULL,
					L"This is not Official Phobos Build.\n\n"
					L"Please contact original Mod Author for Assistance !.",
					L"Warning !", MB_OK);
			}
		}
	}
}

void Phobos::DrawVersionWarning()
{
	if (!Phobos::Config::HideWarning)
	{
		const auto pSurface = DSurface::Composite();
		if (!pSurface || VTable::Get(pSurface) != DSurface::vtable)
			return;

		const auto wanted = Drawing::GetTextDimensions(Phobos::VersionDescription, Point2D::Empty, 0, 2, 0);

		RectangleStruct rect {
			pSurface->Get_Width() - wanted.Width - 10,
			0,
			wanted.Width + 10,
			wanted.Height + 10
		};

		Point2D location { rect.X + 5,5 };

		pSurface->Fill_Rect(rect, COLOR_BLACK);
		pSurface->DSurfaceDrawText(Phobos::VersionDescription, &location, COLOR_RED);
	}
}

void Phobos::InitAdminDebugMode()
{
	if (!Phobos::Otamaa::IsAdmin)
		return;

	// this thing can cause game to lockup when loading data
	//better disable it for release
	const bool Detached =
		Phobos::DetachFromDebugger();

	if (Detached)
	{
		MessageBoxW(NULL,
		L"You can now attach a debugger.\n\n"

		L"Press OK to continue YR execution.",
		L"Debugger Notice", MB_OK);
	}
	else
	{
		MessageBoxW(NULL,
		L"You can now attach a debugger.\n\n"

		L"To attach a debugger find the YR process in Process Hacker "
		L"/ Visual Studio processes window and detach debuggers from it, "
		L"then you can attach your own debugger. After this you should "
		L"terminate Syringe.exe because it won't automatically exit when YR is closed.\n\n"

		L"Press OK to continue YR execution.",
		L"Debugger Notice", MB_OK);
	}

}

#include <New/Type/TheaterTypeClass.h>
#include <New/Type/CursorTypeClass.h>

//https://opengrok.libreoffice.org/xref/core/vcl/win/app/salinst.cxx?r=c35f8114#868
static std::string GetOsVersionQuick()
{
	std::string aVer { "Windows " }; // capacity for string like "Windows 6.1 Service Pack 1 build 7601"
	HMODULE kernel = NULL;
	HMODULE ntdll = NULL;

	for (auto& module : Patch::ModuleDatas) {
		if (IS_SAME_STR_(module.ModuleName.c_str(), "kernel32.dll"))
			kernel = module.Handle;
		else if (IS_SAME_STR_(module.ModuleName.c_str(), "ntdll.dll"))
			ntdll = module.Handle;
	}
	// GetVersion(Ex) and VersionHelpers (based on VerifyVersionInfo) API are
	// subject to manifest-based behavior since Windows 8.1, so give wrong results.
	// Another approach would be to use NetWkstaGetInfo, but that has some small
	// reported delays (some milliseconds), and might get slower in domains with
	// poor network connections.
	// So go with a solution described at https://web.archive.org/web/20090228100958/http://msdn.microsoft.com/en-us/library/ms724429.aspx
	bool bHaveVerFromKernel32 = false;
	if (kernel)
	{
		wchar_t szPath[MAX_PATH] {};
		DWORD dwCount = GetModuleFileNameW(kernel, szPath, std::size(szPath));
		if (dwCount != 0 && dwCount < std::size(szPath))
		{
			dwCount = GetFileVersionInfoSizeW(szPath, nullptr);
			if (dwCount != 0)
			{
				std::unique_ptr<char[]> ver(new char[dwCount]);
				if (GetFileVersionInfoW(szPath, 0, dwCount, ver.get()) != FALSE)
				{
					void* pBlock = nullptr;
					UINT dwBlockSz = 0;
					if (VerQueryValueW(ver.get(), L"\\", &pBlock, &dwBlockSz) != FALSE && dwBlockSz >= sizeof(VS_FIXEDFILEINFO))
					{
						VS_FIXEDFILEINFO* vi1 = static_cast<VS_FIXEDFILEINFO*>(pBlock);
						aVer += (std::to_string(HIWORD(vi1->dwProductVersionMS)) + "."
									+ std::to_string(LOWORD(vi1->dwProductVersionMS)));
						bHaveVerFromKernel32 = true;
					}
				}
			}
		}
	}
	// Now use RtlGetVersion (which is not subject to deprecation for GetVersion(Ex) API)
	// to get build number and SP info
	bool bHaveVerFromRtlGetVersion = false;
	if (ntdll)
	{
		auto const RtlGetVersion_t =
			(NTSTATUS(WINAPI*)(LPOSVERSIONINFOEXW))GetProcAddress(ntdll, "RtlGetVersion");

		if (RtlGetVersion_t != NULL)
		{
			OSVERSIONINFOEXW vi2 {}; // initialize with zeroes - a better alternative to memset
			vi2.dwOSVersionInfoSize = sizeof(vi2);

			if (NULL == RtlGetVersion_t(&vi2))
			{
				if(vi2.dwBuildNumber < 21996) {

					if (!bHaveVerFromKernel32) // we failed above; let's hope this would be useful
						aVer += std::to_string(vi2.dwMajorVersion) + "." + std::to_string(vi2.dwMinorVersion);

					aVer += (" ");

					if (vi2.szCSDVersion[0])
						aVer += (PhobosCRT::WideStringToString(vi2.szCSDVersion) + " ");

				} else {
					aVer = "Windows 11 ";
				}

				aVer += ("Build " + std::to_string(vi2.dwBuildNumber));

				bHaveVerFromRtlGetVersion = true;
			}
		}
	}
	
	if (!bHaveVerFromKernel32 && !bHaveVerFromRtlGetVersion)
		aVer += "unknown";

	return aVer;
}

void Phobos::ExeRun()
{
	Phobos::Otamaa::ExeTerminated = false;
	Game::Savegame_Magic = AresGlobalData::InternalVersion;
	Game::bVideoBackBuffer = false;
	Game::bAllowVRAMSidebar = false;

	MouseCursor::GetCursor(MouseCursorType::ParaDrop).FrameRate = 4;
	MouseCursor::GetCursor(MouseCursorType::Chronosphere).FrameRate = 4;
	MouseCursor::GetCursor(MouseCursorType::IronCurtain).FrameRate = 4;
	MouseCursor::GetCursor(MouseCursorType::Detonate).FrameRate = 4;
	MouseCursor::GetCursor(MouseCursorType::Cursor_36).FrameRate = 4;
	MouseCursor::GetCursor(MouseCursorType::IvanBomb).FrameRate = 4;

	Patch::PrintAllModuleAndBaseAddr();
	Phobos::InitAdminDebugMode();

	int i = 0;

	for (auto&dlls : Patch::ModuleDatas) {
		Debug::LogDeferred("Module [(%d) %s: Base address = %x]\n", i++, dlls.ModuleName.c_str(), dlls.BaseAddr);

		if (IS_SAME_STR_(dlls.ModuleName.c_str(), "cncnet5.dll")) {
			Debug::FatalErrorAndExit("This dll dont need cncnet5.dll to run!, please remove first");
		}
		else if (IS_SAME_STR_(dlls.ModuleName.c_str(), ARES_DLL_S)) {
			Debug::FatalErrorAndExit("This dll dont need Ares.dll to run!, please remove first");
		}
		else if (IS_SAME_STR_(dlls.ModuleName.c_str(), PHOBOS_DLL_S)) {
			Phobos::Otamaa::PhobosBaseAddress = dlls.BaseAddr;
		}
		//else if (ExceptionMode != ExceptionHandlerMode::Default
		//		&& IS_SAME_STR_(dlls.ModuleName.c_str(), "kernel32.dll"))
		//{
		//	if (GetProcAddress(dlls.Handle, "AddVectoredExceptionHandler")) {
		//		pExceptionHandler = AddVectoredExceptionHandler(1, Exception::ExceptionFilter);
		//	}
		//}
	}

	Patch::WindowsVersion = std::move(GetOsVersionQuick());
	Debug::LogDeferred("Running on %s .\n", Patch::WindowsVersion.c_str());
	Debug::LogDeferred("Running on %s API.\n", GetGAPIRuntime());
	TheaterTypeClass::AddDefaults();
	CursorTypeClass::AddDefaults();
}

void Phobos::ExeTerminate()
{
	Debug::DeactivateLogger();

	if(!Phobos::Otamaa::ExeTerminated){
		Phobos::Otamaa::ExeTerminated = true;
	}
}

#pragma warning( push )
#pragma warning (disable : 4091)
#pragma warning (disable : 4245)
bool Phobos::DetachFromDebugger()
{
	DWORD ret = false;
	HMODULE ntdll = NULL;

	for (auto& module : Patch::ModuleDatas) {
		if (IS_SAME_STR_(module.ModuleName.c_str(), "ntdll.dll")){
			ntdll = module.Handle;
			break;
		}
	}

	if (ntdll != NULL) {

		auto const NtRemoveProcessDebug =
			(NTSTATUS(WINAPI*)(HANDLE, HANDLE))GetProcAddress(ntdll, "NtRemoveProcessDebug");
		auto const NtSetInformationDebugObject =
			(NTSTATUS(WINAPI*)(HANDLE, ULONG, PVOID, ULONG, PULONG))GetProcAddress(ntdll, "NtSetInformationDebugObject");
		auto const NtQueryInformationProcess =
			(NTSTATUS(WINAPI*)(HANDLE, ULONG, PVOID, ULONG, PULONG))GetProcAddress(ntdll, "NtQueryInformationProcess");
		auto const NtClose =
			(NTSTATUS(WINAPI*)(HANDLE))GetProcAddress(ntdll, "NtClose");

		HANDLE hDebug {};
		NTSTATUS status = NtQueryInformationProcess(Patch::CurrentProcess, 30, &hDebug, sizeof(HANDLE), 0);
		if (0 <= status)
		{
				ULONG killProcessOnExit = FALSE;
				status = NtSetInformationDebugObject(
					hDebug, 1, &killProcessOnExit, sizeof(ULONG), NULL);

			if (0 <= status)
			{
				const auto pid = Patch::GetDebuggerProcessId(GetProcessId(Patch::CurrentProcess));
				status = NtRemoveProcessDebug(Patch::CurrentProcess, hDebug);
				if (0 <= status)
				{
					HANDLE hDbgProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
					if (hDbgProcess != NULL)
						{
						ret = TerminateProcess(hDbgProcess, EXIT_SUCCESS);
						CloseHandle(hDbgProcess);
					}
				}
			}

			NtClose(hDebug);
		}
	}

	return ret;
}
#pragma warning( pop )


#pragma endregion
#include <Misc/Ares/Hooks/Hooks.MouseCursors.h>

static void __cdecl PatchExit(int uExitCode) {
	Phobos::ExeTerminate();
#ifdef EXPERIMENTAL_IMGUI
	PhobosWindowClass::Destroy();
#endif
	CRT::exit_returnsomething(uExitCode, 0, 0);
	Debug::DetachLogger();
}

static void __cdecl PatchExitB(int uExitCode)
{
	Phobos::ExeTerminate();
#ifdef EXPERIMENTAL_IMGUI
	PhobosWindowClass::Destroy();
#endif
	CRT::exit_returnsomething(uExitCode, 1, 0);
	Debug::DetachLogger();
}

DECLARE_PATCH(_set_fp_mode)
{
	// Call to "store_fpu_codeword"
	_asm { mov edx, 0x007C5EE4 };
	_asm { call edx };

	/**
	 *  Set the FPU mode to match the game (rounding towards zero [chop mode]).
	 */
	_set_controlfp(_RC_CHOP, _MCW_RC);

	/**
	 *  And this is required for the std c++ lib.
	 */
	fesetround(FE_TOWARDZERO);

	JMP(0x6BBFCE);
}

#include <Misc/Multithread.h>

BOOL APIENTRY DllMain(HANDLE hInstance, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		DisableThreadLibraryCalls((HMODULE)hInstance);
		Phobos::hInstance = hInstance;
		Patch::CurrentProcess = GetCurrentProcess();
		Patch::Apply_CALL(0x6BBFC9, &_set_fp_mode);

		const auto time = Debug::GetCurTimeA();

		Patch::Apply_CALL(0x6BD718, &PatchExit);
		Patch::Apply_CALL(0x6BD92F, &PatchExit);
		Patch::Apply_CALL(0x6BDAF4, &PatchExit);
		Patch::Apply_CALL(0x6BDC8C, &PatchExit);
		Patch::Apply_CALL(0x6BDD29, &PatchExit);
		Patch::Apply_CALL(0x6BE1B0, &PatchExit);
		Patch::Apply_CALL(0x6BEC51, &PatchExit);
		Patch::Apply_CALL(0x7CD8F3, &PatchExit);

		Patch::Apply_CALL(0x7CD912, &PatchExitB);
		Patch::Apply_CALL(0x7CD933, &PatchExitB);
		Patch::Apply_CALL(0x7D4AD8, &PatchExitB);
		Patch::Apply_CALL(0x7DC70C, &PatchExitB);
		Patch::Apply_CALL(0x87C2A0, &PatchExitB);

		const char* loadMode = lpReserved ? "statically" : "dynamicly";

		Debug::GenerateDefaultMessage();
		Debug::PrepareLogFile(); //prepare directory
		Debug::LogFileRemove(); //remove previous debug log file if presents

		Debug::LogDeferred("Phobos is being loaded (%s) %s.\n", time.c_str(), loadMode);
		LuaData::LuaDir = std::move(PhobosCRT::WideStringToString(Debug::ApplicationFilePath));
		LuaData::LuaDir += "\\Resources";

		void* buffer {};
		int len = Patch::GetSection(hInstance, PATCH_SECTION_NAME, &buffer);

		//msvc add padding between them so dont forget !
		struct _patch : public Patch {
			BYTE _paddings[3];
		};

		_patch* end = (_patch*)((DWORD)buffer + len);

		for (_patch* begin = (_patch*)buffer; begin < end; begin++) {
			begin->Apply();
		}

		Debug::LogDeferred("Applying %d Static Patches.\n", std::distance((_patch*)buffer,end));
		len = Patch::GetSection(hInstance, ".syhks00", &buffer);

		Initasmjit();

		//hookdecl
		Debug::LogDeferred("Total %d hooks applied.\n", std::distance((hookdecl*)buffer, (hookdecl*)((DWORD)buffer + len)));

		Phobos::ExecuteLua();

		//if(Phobos::Otamaa::ReplaceGameMemoryAllocator)
		{
			/* There is an issue with these , sometime it will crash when game DynamicVector::resize called
			  not really sure what is the real cause atm . */
			Patch::Apply_LJMP(0x7D107D, &_msize);
			Patch::Apply_LJMP(0x7D5408, &_strdup);
			Patch::Apply_LJMP(0x7C8E17, &malloc);
			Patch::Apply_LJMP(0x7C9430, &malloc);
			Patch::Apply_LJMP(0x7D3374, &calloc);
			Patch::Apply_LJMP(0x7D0F45, &realloc);
			Patch::Apply_LJMP(0x7C8B3D, &free);
			Patch::Apply_LJMP(0x7C93E8, &free);
			Patch::Apply_LJMP(0x7C9CC2, &strtok);
		}

		char buf[1024] {};

		if (GetEnvironmentVariable("__COMPAT_LAYER", buf, sizeof(buf))) {
			Debug::LogDeferred("Compatibility modes detected : %s .\n", buf);
		}
	}
	break;
	case DLL_PROCESS_DETACH :
		Multithreading::ShutdownMultitheadMode();
		Debug::DeactivateLogger();
		gJitRuntime.reset();
		break;
	}

	return TRUE;
}

#pragma region hooks
ASMJIT_PATCH(0x55DBCD, MainLoop_SaveGame, 0x6)
{
	// This happens right before LogicClass::Update()
	enum { SkipSave = 0x55DC99, InitialSave = 0x55DBE6 };

	bool& scenario_saved = *reinterpret_cast<bool*>(0xABCE08);
	if (SessionClass::IsSingleplayer() && !scenario_saved)
	{
		scenario_saved = true;
		if (Phobos::ShouldQuickSave)
		{
			Phobos::PassiveSaveGame();
			Phobos::ShouldQuickSave = false;
			Phobos::CustomGameSaveDescription.clear();
		}
		else if (Phobos::Config::SaveGameOnScenarioStart && SessionClass::IsCampaign())
		{
			Debug::Log("Saving Game [Filename : %s , UI : %s , LoadedUI : %ls]",
			ScenarioClass::Instance->FileName,
			ScenarioClass::Instance->UIName, 
			ScenarioClass::Instance->UINameLoaded
			);
			return InitialSave;
		}
	}

	return SkipSave;
}

ASMJIT_PATCH(0x6BBE6A, WinMain_AllowMultipleInstances , 0x6) {
	return Phobos::Otamaa::AllowMultipleInstance ? 0x6BBED6 : 0x0;
}

ASMJIT_PATCH(0x52FE55, Scenario_Start, 0x6)
{
	auto _seed = (DWORD)Game::Seed();
	Debug::Log("Init Phobos Randomizer seed %x.\n", _seed);
	Phobos::Random::SetRandomSeed(Game::Seed());
	return 0;
}ASMJIT_PATCH_AGAIN(0x52FEB7, Scenario_Start, 0x6)

DEFINE_HOOK(0x7CD810, Game_ExeRun, 0x9)
{
	Phobos::ExeRun();
	return 0;
}

ASMJIT_PATCH(0x52F639, _YR_CmdLineParse, 0x5)
{
	GET(char**, ppArgs, ESI);
	GET(int, nNumArgs, EDI);

	Phobos::CmdLineParse(ppArgs, nNumArgs);
	Debug::LogDeferredFinalize();

#ifdef EXPERIMENTAL_IMGUI
	PhobosWindowClass::Create();
#endif

	return 0;
}
#pragma endregion