#include <Phobos.h>

#include <Phobos.lib.h>

#ifdef ENABLE_CLR
#include <Scripting/CLR.h>
#endif

#include <CCINIClass.h>
#include <Unsorted.h>
#include <Drawing.h>

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


#include <Utilities/SafeLogger.h>

#include <MessageBoxLogging.h>

#pragma region defines
HANDLE Phobos::hInstance;
char Phobos::readBuffer[readLength] {};
wchar_t Phobos::wideBuffer[readLength] {};
const char Phobos::readDelims[4] { "," };
const char Phobos::readDefval[4] { "" };
std::string Phobos::AppIconPath {};
bool Phobos::Debug_DisplayDamageNumbers { false };
const wchar_t* Phobos::VersionDescription { L"Phobos Otamaa Unofficial development build #" _STR(BUILD_NUMBER) L". Please test before shipping." };
bool Phobos::ShouldQuickSave { false };
std::wstring Phobos::CustomGameSaveDescription {};
PVOID Phobos::pExceptionHandler { nullptr };
ExceptionHandlerMode Phobos::ExceptionMode { ExceptionHandlerMode::Default };

bool Phobos::HasCNCnet { false };

std::mt19937 Phobos::Random::_engine;

bool Phobos::UI::DisableEmptySpawnPositions { false };
bool Phobos::UI::ExtendedToolTips { false };
int Phobos::UI::MaxToolTipWidth { 0 };
bool Phobos::UI::ShowHarvesterCounter { false };
double Phobos::UI::HarvesterCounter_ConditionYellow { 0.99 };
double Phobos::UI::HarvesterCounter_ConditionRed { 0.5 };
bool Phobos::UI::ShowProducingProgress { false };
bool Phobos::UI::ShowPowerDelta { false };
double Phobos::UI::PowerDelta_ConditionYellow { 0.75 };
double Phobos::UI::PowerDelta_ConditionRed { 1.0 };
bool Phobos::UI::CenterPauseMenuBackground { false };
bool Phobos::UI::WeedsCounter_Show { false };
bool Phobos::UI::UnlimitedColor { false };
bool Phobos::UI::AnchoredToolTips { false };

bool Phobos::UI::SuperWeaponSidebar { false };
int Phobos::UI::SuperWeaponSidebar_Interval { 0 };
int Phobos::UI::SuperWeaponSidebar_LeftOffset { 0 };
int Phobos::UI::SuperWeaponSidebar_CameoHeight { 48 };
int Phobos::UI::SuperWeaponSidebar_Max { 0 };
int Phobos::UI::SuperWeaponSidebar_MaxColumns { INT32_MAX };
bool Phobos::UI::SuperWeaponSidebar_Pyramid = true;

const wchar_t* Phobos::UI::CostLabel { L"" };
const wchar_t* Phobos::UI::PowerLabel { L"" };
const wchar_t* Phobos::UI::PowerBlackoutLabel { L"" };
const wchar_t* Phobos::UI::TimeLabel { L"" };
const wchar_t* Phobos::UI::HarvesterLabel { L"" };
const wchar_t* Phobos::UI::PercentLabel { L"" };

const wchar_t* Phobos::UI::BuidingRadarJammedLabel { L"" };
const wchar_t* Phobos::UI::BuidingFakeLabel { L"" };
const wchar_t* Phobos::UI::ShowBriefingResumeButtonLabel { L"" };
char Phobos::UI::ShowBriefingResumeButtonStatusLabel[0x20] { "" };

const wchar_t* Phobos::UI::Power_Label { L"" };
const wchar_t* Phobos::UI::Drain_Label { L"" };
const wchar_t* Phobos::UI::Storage_Label { L"" };
const wchar_t* Phobos::UI::Radar_Label { L"" };
const wchar_t* Phobos::UI::Spysat_Label { L"" };

const wchar_t* Phobos::UI::SWShotsFormat { L"" };

const wchar_t* Phobos::UI::BattlePoints_Label { L"" };
const wchar_t* Phobos::UI::BattlePointsSidebar_Label { L"" };
bool Phobos::UI::BattlePointsSidebar_Label_InvertPosition {};
bool Phobos::UI::BattlePointsSidebar_AlwaysShow { false };

bool Phobos::Config::HideWarning { false };
bool Phobos::Config::ToolTipDescriptions { true };
bool Phobos::Config::ToolTipBlur { false };
bool Phobos::Config::PrioritySelectionFiltering { true };
bool Phobos::Config::DevelopmentCommands { true };
bool Phobos::Config::ArtImageSwap { false };
bool Phobos::Config::EnableBuildingPlacementPreview { false };
bool Phobos::Config::EnableSelectBox { false };
bool Phobos::Config::TogglePowerInsteadOfRepair { false };
bool Phobos::Config::ShowTechnoNamesIsActive { false };
bool Phobos::Config::RealTimeTimers { false };
bool Phobos::Config::RealTimeTimers_Adaptive { false };
int Phobos::Config::CampaignDefaultGameSpeed { 2 };
bool Phobos::Config::DigitalDisplay_Enable { false };
bool Phobos::Config::MessageDisplayInCenter { false };
bool Phobos::Config::MessageApplyHoverState { false };
bool Phobos::Config::ShowBuildingStatistics { false };
bool Phobos::Config::ApplyShadeCountFi { true };
bool Phobos::Config::SaveVariablesOnScenarioEnd { false };
bool Phobos::Config::MultiThreadSinglePlayer { false };
bool Phobos::Config::UseImprovedPathfindingBlockageHandling { false };
bool Phobos::Config::HideLightFlashEffects { false };
bool Phobos::Config::DebugFatalerrorGenerateDump { false };
bool Phobos::Config::SaveGameOnScenarioStart { true };
bool Phobos::Config::ShowPowerDelta { true };
bool Phobos::Config::ShowHarvesterCounter { true };
bool Phobos::Config::ShowWeedsCounter { false };
bool Phobos::Config::UseNewInheritance { false };
bool Phobos::Config::UseNewIncludes { false };
bool Phobos::Config::ApplyShadeCountFix { true };
bool Phobos::Config::ShowFlashOnSelecting { true };
bool Phobos::Config::AutoBuilding_Enable { false };
bool Phobos::Config::ScrollSidebarStripInTactical { true };
bool Phobos::Config::ScrollSidebarStripWhenHoldKey { true };

bool Phobos::Config::UnitPowerDrain { false };
int Phobos::Config::SuperWeaponSidebar_RequiredSignificance { 0 };

bool Phobos::Misc::CustomGS { false };
int Phobos::Misc::CustomGS_ChangeInterval[7] { -1, -1, -1, -1, -1, -1, -1 };
int Phobos::Misc::CustomGS_ChangeDelay[7] { 0, 1, 2, 3, 4, 5, 6 };
int Phobos::Misc::CustomGS_DefaultDelay[7] { 0, 1, 2, 3, 4, 5, 6 };

bool Phobos::Otamaa::DisableCustomRadSite { false };
bool Phobos::Otamaa::IsAdmin { false };
bool Phobos::Otamaa::ShowHealthPercentEnabled { false };
bool Phobos::Otamaa::ExeTerminated { false };
bool Phobos::Otamaa::DoingLoadGame { false };
bool Phobos::Otamaa::AllowAIControl { false };
bool Phobos::Otamaa::OutputMissingStrings { false };
bool Phobos::Otamaa::OutputAudioLogs { false };
bool Phobos::Otamaa::StrictParser { false };
bool Phobos::Otamaa::ParserErrorDetected { false };
bool Phobos::Otamaa::TrackParserErrors { false };
bool Phobos::Otamaa::NoLogo { false };
bool Phobos::Otamaa::NoCD { false };
bool Phobos::Otamaa::CompatibilityMode { false };
bool Phobos::Otamaa::ReplaceGameMemoryAllocator { false };
bool Phobos::Otamaa::AllowMultipleInstance { false };
DWORD Phobos::Otamaa::PhobosBaseAddress { false };

#pragma endregion

struct GraphicsRuntimeAPI
{
	enum class Type {
		UNK , DX , DXGI, OGL , VK
	};

	GraphicsRuntimeAPI(const std::vector<dllData>& dlls)
		: name { "Unknown" }, type { Type::UNK }
	{
		for (auto& dll : dlls) {
			if (_strnicmp(dll.ModuleName.c_str(), "d3d", 3) == 0
				|| IS_SAME_STR_(dll.ModuleName.c_str(), "dxgi.dll")
				|| IS_SAME_STR_(dll.ModuleName.c_str(), "ddraw.dll")
				)
			{
				name = "DirectX";
				type = Type::DX;
				break;
			}
			else if (IS_SAME_STR_("opengl32.dll", dll.ModuleName.c_str()))
			{
				name = "OpenGL";
				type = Type::OGL;
				break;
			}
			else if (IS_SAME_STR_("vulkan-1.dll", dll.ModuleName.c_str())){
				name = "Vulkan";
				type = Type::VK;
				break;
			}
		}
	}

	~GraphicsRuntimeAPI() = default;

	FORCEDINLINE COMPILETIMEEVAL const char* GetName() const {
		return name.c_str();
	}

	FORCEDINLINE COMPILETIMEEVAL Type GetType() {
		return type;
	}

private:
	std::string name;
	Type type;
};

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

struct HooksData {
	std::vector<HookSummary> summary {};
	std::vector<uint8_t> originalOpcode {};
};

// remove the comment if you want to run the dll with patched gamemd
//#define NO_SYRINGE

bool IsRunningInAppContainer() {
	static bool s_checked = false;
	static bool s_isAppContainer = false;

	if (!s_checked) {
		HANDLE hToken;
		if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
			DWORD dwLength = 0;
			GetTokenInformation(hToken, TokenAppContainerSid, nullptr, 0, &dwLength);
			s_isAppContainer = (GetLastError() != ERROR_NOT_FOUND);
			CloseHandle(hToken);
		}
		s_checked = true;
	}

	return s_isAppContainer;
}

void OptimizeProcessForSecurity()
{
	if (IsRunningInAppContainer())
	{
		Debug::LogDeferred("App Container detected. Optimizing object creation.\n");

		// Set process mitigations only if available (Windows 8+)
		typedef BOOL(WINAPI* SetProcessMitigationPolicyFunc)(PROCESS_MITIGATION_POLICY, PVOID, SIZE_T);


#ifdef _Enable_these
		static bool s_checked = false;
		static bool s_isAppContainer = false;
		if (!s_checked)
		{
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

						s_checked = true;
					}
				}
			}

			if (_PID)
				FreeSid(_PID);
		}
#endif

		for (auto& module : Patch::ModuleDatas) {
			if (IS_SAME_STR_(module.ModuleName.c_str(), "kernel32.dll")) {
				SetProcessMitigationPolicyFunc pSetProcessMitigationPolicy =
					(SetProcessMitigationPolicyFunc)GetProcAddress(module.Handle, "SetProcessMitigationPolicy");

				if (pSetProcessMitigationPolicy) {
					// Use simplified mitigation
					pSetProcessMitigationPolicy((PROCESS_MITIGATION_POLICY)1, nullptr, 0);
				}
			}
		}

		// Reduce security descriptor checks
		SetThreadToken(nullptr, nullptr);
	}
}

std::map<unsigned int, HooksData> Hooks { };
#include <Zydis/Zydis.h>

static void CheckHookConflict(unsigned int addr, size_t size)
{
	const size_t frontOffset = 4;
	byte* hookAddress = (byte*)addr;
	bool maybeConflicted = false;
	bool beforeAddress = false;
	// check hook race
	for (int offset = -frontOffset; offset < (int)size; offset++)
	{
		byte cur = hookAddress[offset];
		switch (cur)
		{
		case Assembly::CALL:
		case Assembly::JMP:
			if (offset == 0)
			{
				offset += 5 - 1;
				continue;
			}
			else if (offset < 0 && cur != 0xE9)
			{
				continue;
			}
			maybeConflicted = true;
			break;
		}
		if (maybeConflicted)
		{
			if (offset < 0)
			{
				beforeAddress = true;
			}
			break;
		}
	}

	std::string disassemblyResult;
	if (maybeConflicted)
	{
		// Initialize decoder context
		ZydisDecoder decoder;
		ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_COMPAT_32, ZYDIS_STACK_WIDTH_32);

		// Initialize formatter. Only required when you actually plan to do instruction
		// formatting ("disassembling"), like we do here
		ZydisFormatter formatter;
		ZydisFormatterInit(&formatter, ZYDIS_FORMATTER_STYLE_INTEL);

		// Loop over the instructions in our buffer.
		// The runtime-address (instruction pointer) is chosen arbitrary here in order to better
		// visualize relative addressing
		ZyanU64 runtime_address = addr;
		ZyanUSize offset = 0;
		const ZyanUSize length = size + 4;
		ZydisDecodedInstruction instruction;
		ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT];
		while (ZYAN_SUCCESS(ZydisDecoderDecodeFull(&decoder, (void*)(addr + offset), length - offset, &instruction, operands)))
		{
			// Format & print the binary instruction structure to human-readable format
			char buffer[256];
			ZydisFormatterFormatInstruction(&formatter, &instruction, operands,
				instruction.operand_count_visible, buffer, sizeof(buffer), runtime_address, ZYAN_NULL);
			disassemblyResult += fmt::format("\n-- 0x{:08x}  {}", runtime_address, buffer);

			offset += instruction.length;
			runtime_address += instruction.length;
		}
	}
	if (disassemblyResult.contains("jmp") || disassemblyResult.contains("call"))
	{
		Debug::LogDeferred("Hook %x seems to be conflicted with other hooks! disassembly: %s \n", (void*)hookAddress, disassemblyResult.c_str());
	}
	if (beforeAddress)
	{
		Debug::LogDeferred("Hook %x seems to be conflicted with other hooks! see assembly before address\n", (void*)hookAddress);
	}
}

std::string PrintAssembly(const void* code, size_t codeSize, uintptr_t runtimeAddress = 0)
{
	ZydisDecoder decoder;
	ZydisFormatter formatter;

	ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LEGACY_32, ZYDIS_STACK_WIDTH_32);
	ZydisFormatterInit(&formatter, ZYDIS_FORMATTER_STYLE_INTEL);
	ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT];
	std::string disassemblyResult;

	size_t offset = 0;
	while (offset < codeSize)
	{
		ZydisDecodedInstruction instruction;
		while (ZYAN_SUCCESS(ZydisDecoderDecodeFull(&decoder, reinterpret_cast<const uint8_t*>(code) + offset, codeSize - offset, &instruction, operands)))
		{
			// Format & print the binary instruction structure to human-readable format
			char buffer[256];
			ZydisFormatterFormatInstruction(&formatter, &instruction, operands,
				instruction.operand_count_visible, buffer, sizeof(buffer), runtimeAddress + offset, ZYAN_NULL);
			disassemblyResult += fmt::format("0x{} : {}\n", unsigned(runtimeAddress + offset), buffer);

			offset += instruction.length;
		}
	}

	return disassemblyResult;
}

void ApplyasmjitPatch() {

	for (auto&[addr , data] : Hooks)
	{
		auto& [sm_vec, org_vec] = data;

		if (sm_vec.empty()) {
			Debug::LogDeferred("hook at 0x%x is empty !\n", addr);
			continue;
		}

		CheckHookConflict(addr, sm_vec[0].size);

		if (sm_vec.size() > 1) {
			Debug::LogDeferred("hook at 0x%x , has %d functions registered !\n", addr, sm_vec.size());
		}

		size_t hook_size = sm_vec[0].size;
		asmjit::CodeHolder code;
		code.init(gJitRuntime->environment(), gJitRuntime->cpuFeatures());
		code.setErrorHandler(&gJitErrorHandler);
		asmjit::x86::Assembler assembly(&code);
		asmjit::Label l_origin = assembly.newLabel();
		DWORD hookSize = MaxImpl(hook_size, 5u);

		if (sm_vec.size() == 1)
		{
			assembly.pushad();
			assembly.pushfd();
			assembly.push(addr);
			assembly.sub(asmjit::x86::esp, 4);
			assembly.lea(asmjit::x86::eax, asmjit::x86::ptr(asmjit::x86::esp, 4));
			assembly.push(asmjit::x86::eax);
			assembly.call(sm_vec[0].func);
			assembly.add(asmjit::x86::esp, 0xC);
			assembly.mov(asmjit::x86::ptr(asmjit::x86::esp, -8), asmjit::x86::eax);
			assembly.popfd();
			assembly.popad();
			assembly.cmp(asmjit::x86::dword_ptr(asmjit::x86::esp, -0x2C), 0);
			assembly.jz(l_origin);
			assembly.jmp(asmjit::x86::ptr(asmjit::x86::esp, -0x2C));
		} else {
			Debug::LogDeferred("remaining hook at 0x%x is ignored !\n", addr);
		}

		assembly.bind(l_origin);
		void* hookAddress = (void*)addr;

		org_vec.resize(hookSize);
		memcpy(org_vec.data(), hookAddress, hookSize);

		// fix relative jump or call
		if (org_vec[0] == Assembly::CALL || org_vec[0] == Assembly::JMP)
		{
			DWORD dest = addr + 5 + *(DWORD*)(org_vec.data() + 1);
			switch (org_vec[0])
			{
			case Assembly::JMP: // jmp
				assembly.jmp(dest);
				org_vec.erase(org_vec.begin(), org_vec.begin() + 5);
				Debug::LogDeferred("hook at 0x%x is placed at JMP fixing the relative addr !\n", addr);
				break;
			case Assembly::CALL: // call
				assembly.call(dest);
				org_vec.erase(org_vec.begin(), org_vec.begin() + 5);
				Debug::LogDeferred("hook at 0x%x is placed at CALL fixing the relative addr !\n", addr);

				break;
			}
		}
		assembly.embed(org_vec.data(), org_vec.size());
		assembly.jmp(addr + hookSize);
		const void* fn {};
		gJitRuntime->add(&fn, &code);
		code.reset();
		code.init(gJitRuntime->environment(), gJitRuntime->cpuFeatures());
		code.setErrorHandler(&gJitErrorHandler);
		code.attach(&assembly);
		assembly.jmp(fn);
		code.flatten();
		code.resolveCrossSectionFixups();
		code.relocateToBase(addr);

		DWORD protect_flag {};
		DWORD protect_flagb {};
		VirtualProtect(hookAddress, hookSize, PAGE_EXECUTE_READWRITE, &protect_flag);
		code.copyFlattenedData(hookAddress, hookSize);
		VirtualProtect(hookAddress, hookSize, protect_flag, &protect_flagb);
		FlushInstructionCache(Game::hInstance, hookAddress, hookSize);
	}
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
		hook.summary.emplace_back(begin->hookFunc, begin->hookSize);
	}

	ApplyasmjitPatch();
}

#ifdef EXPERIMENTAL_IMGUI
ASMJIT_PATCH(0x5D4E66, Windows_Message_Handler_Add, 0x7)
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
	GeneralUtils::PrintMessage(StringTable::FetchString(GameStrings::TXT_SAVING_GAME));
	const auto name = "Map." + Debug::GetCurTimeA() + ".sav";

	if (ScenarioClass::SaveGame(name.c_str(), Phobos::CustomGameSaveDescription.c_str()))
		GeneralUtils::PrintMessage(StringTable::FetchString(GameStrings::TXT_GAME_WAS_SAVED));
	else
		GeneralUtils::PrintMessage(StringTable::FetchString(GameStrings::TXT_ERROR_SAVING_GAME));
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
		//auto& logger = SafeLogger::GetInstance();
		//LogConfig config;
		//config.enabled = true;
		//config.console_output = true;
		//config.log_filename = "testings.log";
		//logger.SetConfig(config);
		//logger.Initialize();

		//LOG_INFO("DLL injection successful, logging enabled via command line");
		//LOG_INFO("Initialized Phobos " PRODUCT_VERSION ".");
		//LOG_INFO("args {}", args);

		Debug::Log("DLL injection successful, logging enabled via command line.\n");
		Debug::Log("Initialized Phobos " PRODUCT_VERSION ".\n");
		Debug::Log("args %s\n", args.c_str());

		SpawnerMain::PrintInitializeLog();
	} else {
		Debug::DeactivateLogger();
		Debug::LogFileRemove();
		Debug::made = false;// reset
	}

	Phobos::CheckProcessorFeatures();
	// Optimize for app container environments
	OptimizeProcessForSecurity();

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
		__stosd(reinterpret_cast<unsigned long*>(CDDriveManagerClass::Instance->CDDriveNames.data()), 0xFFFFFFFF, CDDriveManagerClass::Instance->CDDriveNames.size());

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
		EMIT_MSGBOXW(
		L"You can now attach a debugger.\n\n"
		L"Press OK to continue YR execution.",
		L"Debugger Notice");
	}
	else
	{
		EMIT_MSGBOXW(
		L"You can now attach a debugger.\n\n"
		L"To attach a debugger find the YR process in Process Hacker "
		L"/ Visual Studio processes window and detach debuggers from it, "
		L"then you can attach your own debugger. After this you should "
		L"terminate Syringe.exe because it won't automatically exit when YR is closed.\n\n"
		L"Press OK to continue YR execution.",
		L"Debugger Notice");
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
	Game::Savegame_Magic = Phobos::GetVersionNumber();
	Game::bVideoBackBuffer = false;
	Game::bAllowVRAMSidebar = false;

	MouseCursor::GetCursor(MouseCursorType::ParaDrop).FrameRate = 4;
	MouseCursor::GetCursor(MouseCursorType::Chronosphere).FrameRate = 4;
	MouseCursor::GetCursor(MouseCursorType::IronCurtain).FrameRate = 4;
	MouseCursor::GetCursor(MouseCursorType::Detonate).FrameRate = 4;
	MouseCursor::GetCursor(MouseCursorType::Cursor_36).FrameRate = 4;
	MouseCursor::GetCursor(MouseCursorType::IvanBomb).FrameRate = 4;

	Patch::PrintAllModuleAndBaseAddr();

#if !defined(NO_SYRINGE)
	Phobos::InitAdminDebugMode();
#endif

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
	GraphicsRuntimeAPI gRuntimeAPI(Patch::ModuleDatas);
	Debug::LogDeferred("Running on %s API.\n", gRuntimeAPI.GetName());
	TheaterTypeClass::AddDefaults();
	CursorTypeClass::AddDefaults();
}

void Phobos::ExeTerminate()
{
	Debug::DeactivateLogger();

	if(!Phobos::Otamaa::ExeTerminated){
		Phobos::Otamaa::ExeTerminated = true;

		//for (auto& datas : Patch::ModuleDatas) {
		//	for (size_t i = 0; i < Patch::ModuleDatas.size(); ++i) {
		//		if (i != 0 || (datas.Handle && datas.Handle != INVALID_HANDLE_VALUE)) {
		//			CloseHandle(datas.Handle);
		//		}
		//	}
		//}

		Patch::ModuleDatas.clear();
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
#include <ExtraHeaders/MemoryPool.h>

static CriticalSection critSec3 , critSec4;
#ifdef _ReplaceAlloc
struct GameMemoryReplacer
{


	static char* _strtok_r(char* str, const char* delim, char** saveptr)
	{
		char* token;

		if (str == nullptr)
			str = *saveptr;

		// Skip leading delimiters
		str += std::strspn(str, delim);
		if (*str == '\0')
		{
			*saveptr = str;
			return nullptr;
		}

		token = str;
		// Find end of token
		str = std::strpbrk(token, delim);
		if (str == nullptr)
		{
			// This token finishes the string
			*saveptr = token + std::strlen(token);
		}
		else
		{
			*str = '\0';
			*saveptr = str + 1;
		}
		return token;
	}

	static char* _strtok(char* str, const char* delim) {
		static char* saveptr = nullptr;
		return _strtok_r(str, delim, &saveptr);
	}
};
#endif

HMODULE GetCurrentModule()
{
	MEMORY_BASIC_INFORMATION mbi;
	VirtualQuery((void*)GetCurrentModule, &mbi, sizeof(mbi));
	return static_cast<HMODULE>(mbi.AllocationBase);
}

void LogCallStack(int skipFrames = 0, int maxFrames = 16)
{
	void* stack[32];
	unsigned short frames = CaptureStackBackTrace(skipFrames + 1, maxFrames, stack, nullptr);

	HANDLE hProcess = GetCurrentProcess();
	static bool symInit = false;
	if (!symInit)
	{
		SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);
		symInit = SymInitialize(hProcess, nullptr, TRUE);
	}

	char out[512];
	SYMBOL_INFO* sym = (SYMBOL_INFO*)_alloca(sizeof(SYMBOL_INFO) + 256);
	sym->MaxNameLen = 255;
	sym->SizeOfStruct = sizeof(SYMBOL_INFO);

	HMODULE base = GetCurrentModule();

	for (unsigned short i = 0; i < frames; ++i)
	{
		DWORD64 addr = (DWORD64)(stack[i]);
		if (SymFromAddr(hProcess, addr, nullptr, sym))
		{
			snprintf(out, sizeof(out), "[%02d] %s - 0x%p\n", i, sym->Name, (void*)addr);
		}
		else
		{
			uintptr_t offset = (uintptr_t)addr - (uintptr_t)base;
			snprintf(out, sizeof(out), "[%02d] RetAddr: 0x%p (offset: 0x%X)\n", i, (void*)addr, (unsigned)offset);
		}
		OutputDebugStringA(out);
	}
}

using FnRtl = NTSTATUS(WINAPI*)(HANDLE, HANDLE, ULONG, PUNICODE_STRING);
FnRtl RealFn = nullptr;

NTSTATUS WINAPI HookedRtlGetAppContainerNamedObjectPath(
	HANDLE token,
	HANDLE directoryHandle,
	ULONG length,
	PUNICODE_STRING path)
{
	//OutputDebugStringA("[Phobos] RtlGetAppContainerNamedObjectPath was called!\n");
	//LogCallStack(); // Capture caller
	return RealFn(token, directoryHandle, length, path);
}

#include <minhook/MinHook.h>

NOINLINE void EnableLargeAddressAwareFlag(HANDLE curProc)
{
	BYTE* base = (BYTE*)curProc; // base of gamemd.exe
	DWORD peOffset = *(DWORD*)(base + 0x3C);
	WORD* characteristics = (WORD*)(base + peOffset + 0x18);

	DWORD oldProtect;
	if (VirtualProtect(characteristics, sizeof(WORD), PAGE_EXECUTE_READWRITE, &oldProtect))
	{
		*characteristics |= 0x20; // IMAGE_FILE_LARGE_ADDRESS_AWARE
		VirtualProtect(characteristics, sizeof(WORD), oldProtect, &oldProtect);
		Debug::LogDeferred("LARGEADDRESSAWARE flag set via injector.\n");
	} else {
		Debug::LogDeferred("Failed to change protection for Characteristics.\n");
	}
}

NOINLINE bool IsGamemdExe(HMODULE curProc) {
	wchar_t filename[MAX_PATH];
	GetModuleFileNameW(curProc, filename, MAX_PATH);
	const std::wstring path(filename);

	//add more variants;

	if (path.find(L"gamemd.exe") != std::wstring::npos)
		return true;

	if (path.find(L"gamepp") != std::wstring::npos)
		return true;


	return false;
}

LPVOID saved_lpReserved;

NOINLINE void ApplyEarlyFuncs() {

	//make sure is correct executable loading the dll
	//if(IsGamemdExe((HMODULE)Patch::CurrentProcess))
	{

		//std::thread([]() {
		//	EnableLargeAddressAwareFlag(Patch::CurrentProcess);
		//}).detach();

		MH_Initialize();

		auto mod = GetModuleHandleW(L"ntdll.dll");

		if (auto target = (FnRtl)GetProcAddress(mod, "RtlGetAppContainerNamedObjectPath")) {
			MH_CreateHook(target, &HookedRtlGetAppContainerNamedObjectPath, (void**)&RealFn);
			MH_EnableHook(target);
		};

		TheMemoryPoolCriticalSection = &critSec4;
		TheDmaCriticalSection = &critSec3;

		Mem::preMainInitMemoryManager();

		//Patch::Apply_CALL(0x6BBFC9, &_set_fp_mode);

		const auto time = Debug::GetCurTimeA();
		Patch::Apply_TYPED<DWORD>(0x7B853C, {1});
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

		Patch::Apply_TYPED<char>(0x82612C + 13, { '\n' });

		const char* loadMode = saved_lpReserved ? "statically" : "dynamicly";

		Debug::GenerateDefaultMessage();
		Debug::PrepareLogFile(); //prepare directory
		Debug::LogFileRemove(); //remove previous debug log file if presents

		Debug::LogDeferred("Phobos is being loaded (%s) %s.\n", time.c_str(), loadMode);
		LuaData::LuaDir = std::move(PhobosCRT::WideStringToString(Debug::ApplicationFilePath));
		LuaData::LuaDir += "\\Resources";

		void* buffer {};
		int len = Patch::GetSection(Phobos::hInstance, PATCH_SECTION_NAME, &buffer);

		//msvc add padding between them so dont forget !
		struct _patch : public Patch
		{
			BYTE _paddings[3];
		};

		_patch* end = (_patch*)((DWORD)buffer + len);

		for (_patch* begin = (_patch*)buffer; begin < end; begin++)
		{
			begin->Apply();
		}

		Debug::LogDeferred("Applying %d Static Patche(s).\n", std::distance((_patch*)buffer, end));
		len = Patch::GetSection(Phobos::hInstance, ".syhks00", &buffer);

		Initasmjit();

		//hookdecl
		Debug::LogDeferred("Applying %d Syringe hook(s).\n", std::distance((hookdecl*)buffer, (hookdecl*)((DWORD)buffer + len)));

		Phobos::ExecuteLua();

		char buf[1024] {};

		if (GetEnvironmentVariable("__COMPAT_LAYER", buf, sizeof(buf)))
		{
			Debug::LogDeferred("Compatibility modes detected : %s .\n", buf);
		}
	}
}

BOOL APIENTRY DllMain(HANDLE hInstance, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		DisableThreadLibraryCalls((HMODULE)hInstance);
		Patch::CurrentProcess = GetCurrentProcess();
		Phobos::hInstance = hInstance;
		saved_lpReserved = lpReserved;

#if defined(NO_SYRINGE)
		ApplyEarlyFuncs();
		//LuaData::ApplyCoreHooks();
		Phobos::ExeRun();
#endif

	}
	break;
	case DLL_PROCESS_DETACH :
		Multithreading::ShutdownMultitheadMode();
		Debug::DeactivateLogger();
		gJitRuntime.reset();
		Mem::shutdownMemoryManager();
		MH_Uninitialize();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		return FALSE;
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

//syringe wont inject the dll unless it got atleast one hook
//so i keep this
#if !defined(NO_SYRINGE)
declhook(0x7CD810, Game_ExeRun, 0x9)
extern "C" __declspec(dllexport) DWORD __cdecl Game_ExeRun(REGISTERS* R)
{

	ApplyEarlyFuncs();
	//LuaData::ApplyCoreHooks();
	Phobos::ExeRun();
	return 0;
}
#endif

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
