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

#include <MessageBoxLogging.h>

#pragma region defines
HANDLE Phobos::hInstance;
char Phobos::readBuffer[readLength] {};
wchar_t Phobos::wideBuffer[readLength] {};
const char Phobos::readDelims[4] { "," };
const char Phobos::readDefval[4] { "" };
std::string Phobos::AppIconPath {};
DrawDamageMode Phobos::Debug_DisplayDamageNumbers { DrawDamageMode::disabled };
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
const wchar_t* Phobos::UI::Tech_Label { L"" };
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
int Phobos::Config::MessageDisplayInCenter_BoardOpacity { 40 };
int Phobos::Config::MessageDisplayInCenter_LabelsCount { 4 };
int Phobos::Config::MessageDisplayInCenter_RecordsCount { 12 };
bool Phobos::Config::ShowBuildingStatistics { false };
bool Phobos::Config::ApplyShadeCountFi { true };
bool Phobos::Config::SaveVariablesOnScenarioEnd { false };
bool Phobos::Config::MultiThreadSinglePlayer { false };
bool Phobos::Config::UseImprovedPathfindingBlockageHandling { false };
bool Phobos::Config::HideLightFlashEffects { false };
bool Phobos::Config::HideLaserTrailEffects { false };
bool Phobos::Config::HideShakeEffects { false };
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
bool Phobos::Config::AllowSwitchNoMoveCommand = false;
bool Phobos::Config::AllowDistributionCommand = false;
bool Phobos::Config::AllowDistributionCommand_SpreadMode = true;
bool Phobos::Config::AllowDistributionCommand_SpreadModeScroll = true;
bool Phobos::Config::AllowDistributionCommand_FilterMode = true;
bool Phobos::Config::AllowDistributionCommand_AffectsAllies = true;
bool Phobos::Config::AllowDistributionCommand_AffectsEnemies = true;
bool Phobos::Config::ApplyNoMoveCommand = true;
int Phobos::Config::DistributionSpreadMode = 2;
int Phobos::Config::DistributionFilterMode = 2;

int Phobos::Config::SuperWeaponSidebar_RequiredSignificance { 0 };
bool Phobos::Config::SuperWeaponSidebarCommands { false };
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

bool Phobos::SaveGlobals(PhobosStreamWriter& stm)
{
	return stm
		.Process(Phobos::Config::ArtImageSwap)
		.Process(Phobos::Config::ShowTechnoNamesIsActive)
		.Process(Phobos::Misc::CustomGS)
		.Process(Phobos::Config::ApplyShadeCountFix)
		.Process(Phobos::Otamaa::CompatibilityMode)
		.Process(Phobos::Config::UnitPowerDrain)
		.Success();
}

bool Phobos::LoadGlobals(PhobosStreamReader& stm)
{
	return stm
		.Process(Phobos::Config::ArtImageSwap)
		.Process(Phobos::Config::ShowTechnoNamesIsActive)
		.Process(Phobos::Misc::CustomGS)
		.Process(Phobos::Config::ApplyShadeCountFix)
		.Process(Phobos::Otamaa::CompatibilityMode)
		.Process(Phobos::Config::UnitPowerDrain)
		.Success();
}

struct GraphicsRuntimeAPI
{
	enum class Type
	{
		UNK, DX, DXGI, OGL, VK
	};

	GraphicsRuntimeAPI(const std::vector<dllData>& dlls)
		: name { "Unknown" }, type { Type::UNK }
	{
		for (auto& dll : dlls)
		{
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
			else if (IS_SAME_STR_("vulkan-1.dll", dll.ModuleName.c_str()))
			{
				name = "Vulkan";
				type = Type::VK;
				break;
			}
		}
	}

	~GraphicsRuntimeAPI() = default;

	FORCEDINLINE COMPILETIMEEVAL const char* GetName() const
	{
		return name.c_str();
	}

	FORCEDINLINE COMPILETIMEEVAL Type GetType()
	{
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
	void handle_error(asmjit::Error err, const char* message, asmjit::BaseEmitter* origin) override
	{
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
struct HookSummary
{
	const void* func;
	size_t size;
};

struct HooksData
{
	std::vector<HookSummary> summary {};
	std::vector<uint8_t> originalOpcode {};
};

// remove the comment if you want to run the dll with patched gamemd
//#define NO_SYRINGE

bool IsRunningInAppContainer()
{
	static bool s_checked = false;
	static bool s_isAppContainer = false;

	if (!s_checked)
	{
		HANDLE hToken;
		if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
		{
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

		for (auto& module : Patch::ModuleDatas)
		{
			if (IS_SAME_STR_(module.ModuleName.c_str(), "kernel32.dll"))
			{
				SetProcessMitigationPolicyFunc pSetProcessMitigationPolicy =
					(SetProcessMitigationPolicyFunc)GetProcAddress(module.Handle, "SetProcessMitigationPolicy");

				if (pSetProcessMitigationPolicy)
				{
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

struct FunctionTrampoline
{
	void* original_address;
	void* trampoline_address;
	std::vector<uint8_t> original_bytes;
	size_t hook_size;
};

// Global trampoline storage (maps hook address -> trampoline)
std::unordered_map<unsigned int, FunctionTrampoline> g_trampolines;

// Setup trampoline - call this BEFORE writing hook
bool SetupTrampoline(unsigned int target_address, size_t hook_size)
{
	FunctionTrampoline& trampoline = g_trampolines[target_address];

	trampoline.original_address = reinterpret_cast<void*>(target_address);
	trampoline.hook_size = hook_size;

	// Read original bytes from target
	trampoline.original_bytes.resize(hook_size);
	memcpy(trampoline.original_bytes.data(), trampoline.original_address, hook_size);

	// Allocate executable memory for trampoline
	trampoline.trampoline_address = VirtualAlloc(
		nullptr,
		hook_size + 5, // original bytes + jmp instruction
		MEM_COMMIT | MEM_RESERVE,
		PAGE_EXECUTE_READWRITE
	);

	if (!trampoline.trampoline_address)
	{
		Debug::LogDeferred("Failed to allocate trampoline for hook at 0x%x\n", target_address);
		g_trampolines.erase(target_address);
		return false;
	}

	// Write original bytes to trampoline
	memcpy(trampoline.trampoline_address,
		   trampoline.original_bytes.data(),
		   hook_size);

	// Add jump back to original function (after hooked bytes)
	uint8_t* trampoline_end = reinterpret_cast<uint8_t*>(trampoline.trampoline_address) + hook_size;
	uintptr_t return_address = target_address + hook_size;
	uintptr_t jump_offset = return_address - (reinterpret_cast<uintptr_t>(trampoline_end) + 5);

	trampoline_end[0] = 0xE9; // JMP opcode
	memcpy(trampoline_end + 1, &jump_offset, 4);

	Debug::LogDeferred("Trampoline created for hook at 0x%x -> 0x%p\n",
					  target_address, trampoline.trampoline_address);

	return true;
}

// Cleanup function - call when shutting down
void CleanupTrampolines()
{
	for (auto& pair : g_trampolines) {
		if (pair.second.trampoline_address) {
			VirtualFree(pair.second.trampoline_address, 0, MEM_RELEASE);
		}
	}

	g_trampolines.clear();
	Debug::LogDeferred("All trampolines cleaned up\n");
}

// Modified patch function with trampoline support
void ApplyasmjitPatch()
{
	for (auto& [addr, data] : Hooks)
	{
		auto& [sm_vec, org_vec] = data;

		if (sm_vec.empty())
		{
			Debug::LogDeferred("hook at 0x%x is empty !\n", addr);
			continue;
		}

		CheckHookConflict(addr, sm_vec[0].size);

		if (sm_vec.size() > 1)
		{
			Debug::LogDeferred("hook at 0x%x , has %d functions registered !\n", addr, sm_vec.size());
		}

		size_t hook_size = sm_vec[0].size;
		DWORD hookSize = MaxImpl(hook_size, 5u);

#ifndef allowrecursive
		// Setup trampoline BEFORE creating hook code
		if (!SetupTrampoline(addr, hookSize))
		{
			Debug::LogDeferred("Failed to setup trampoline for hook at 0x%x, skipping!\n", addr);
			continue;
		}

		FunctionTrampoline& trampoline = g_trampolines[addr];
#endif

		asmjit::CodeHolder code;
		code.init(gJitRuntime->environment(), gJitRuntime->cpu_features());
		code.set_error_handler(&gJitErrorHandler);
		asmjit::x86::Assembler assembly(&code);
		asmjit::Label l_origin = assembly.new_label();

		if (sm_vec.size() == 1)
		{
#ifndef allowrecursive
			// Original non-recursive version
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

			// POPAD replica
			assembly.popad();

			assembly.cmp(asmjit::x86::dword_ptr(asmjit::x86::esp, -0x2C), 0);
			assembly.jz(l_origin);
			assembly.jmp(asmjit::x86::ptr(asmjit::x86::esp, -0x2C));
#else
			// Recursive-safe version with trampoline
			assembly.pushad();
			assembly.pushfd();

			// Push hook address
			assembly.push(asmjit::imm(static_cast<uint32_t>(addr)));

			// Push stack pointer (pointer to parameters)
			assembly.push(asmjit::x86::esp);

			// Call handler function
			assembly.call(asmjit::imm(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(sm_vec[0].func))));

			// Clean up stack (2 pushes = 8 bytes)
			assembly.add(asmjit::x86::esp, 8);

			// Store return value in TLS (temporary storage through register restoration)
			assembly.mov(asmjit::x86::Mem(asmjit::x86::fs, 0x14, 4), asmjit::x86::eax);

			// Restore flags
			assembly.popfd();

			// POPAD replica
			assembly.pop(asmjit::x86::edi);
			assembly.pop(asmjit::x86::esi);
			assembly.pop(asmjit::x86::ebp);
			assembly.pop(asmjit::x86::ebx);
			assembly.mov(asmjit::x86::eax, asmjit::x86::ptr(asmjit::x86::esp, 0x0C));
			assembly.mov(asmjit::x86::ptr(asmjit::x86::esp, 0x0C), asmjit::x86::ebx);
			assembly.pop(asmjit::x86::ebx);
			assembly.pop(asmjit::x86::edx);
			assembly.pop(asmjit::x86::ecx);
			assembly.pop(asmjit::x86::esp);

			// Check if handler returned a redirect address
			assembly.cmp(asmjit::x86::Mem(asmjit::x86::fs, 0x14, 4), asmjit::imm(0));
			assembly.je(l_origin);

			// If non-zero, jump to returned address
			assembly.jmp(asmjit::x86::Mem(asmjit::x86::fs, 0x14, sizeof(void*)));
#endif
		}
		else
		{
			Debug::LogDeferred("remaining hook at 0x%x is ignored !\n", addr);
		}

		assembly.bind(l_origin);
		void* hookAddress = reinterpret_cast<void*>(addr);

#ifdef allowrecursive
		// Original version: copy bytes from original address
		org_vec.resize(hookSize);
		memcpy(org_vec.data(), hookAddress, hookSize);
#else
		// Recursive version: use bytes from trampoline backup
		org_vec.resize(hookSize);
		memcpy(org_vec.data(), trampoline.original_bytes.data(), hookSize);
#endif

		// Fix relative jump or call
		if (org_vec[0] == Assembly::CALL || org_vec[0] == Assembly::JMP)
		{
			DWORD dest = addr + 5 + *reinterpret_cast<DWORD*>(org_vec.data() + 1);
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
		code.init(gJitRuntime->environment(), gJitRuntime->cpu_features());
		code.set_error_handler(&gJitErrorHandler);
		code.attach(&assembly);
		assembly.jmp(fn);
		code.flatten();
		code.resolve_cross_section_fixups();
		code.relocate_to_base(addr);

		DWORD protect_flag {};
		DWORD protect_flagb {};
		VirtualProtect(hookAddress, hookSize, PAGE_EXECUTE_READWRITE, &protect_flag);
		code.copy_flattened_data(hookAddress, hookSize);
		VirtualProtect(hookAddress, hookSize, protect_flag, &protect_flagb);
		FlushInstructionCache(Game::hInstance, hookAddress, hookSize);

		Debug::LogDeferred("Hook installed at 0x%x (size: %d bytes)\n", addr, hookSize);
	}
}

void Initasmjit()
{
	gJitRuntime = std::make_unique<asmjit::JitRuntime>();

	void* buffer {};
	int len = Patch::GetSection(Phobos::hInstance, ASMJIT_PATCH_SECTION_NAME, &buffer);

	hookdeclfunc* end = (hookdeclfunc*)((DWORD)buffer + len);
	Debug::LogDeferred("Applying %d asmjit hooks.\n", std::distance((hookdeclfunc*)buffer, end));

	for (hookdeclfunc* begin = (hookdeclfunc*)buffer; begin < end; begin++)
	{
		Hooks[begin->hookAddr].summary.emplace_back(begin->hookFunc, begin->hookSize);
	}

	ApplyasmjitPatch();
}

void ShutdownAsmjit()
{
	CleanupTrampolines();
	gJitRuntime.reset();
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
			if (Parser<int>::Parse(pArg + 0xAu, &result) && result < 0)
			{
				result = 0;
			}

			processAffinityMask = result;
		}
		else
		{
			const std::string cur = pArg;
			if (cur.starts_with("-ExceptionHandler="))
			{

				const size_t delim = cur.find("=");
				const std::string value = cur.substr(delim + 1, cur.size() - delim - 1);

				if (!value.empty())
				{
					Parser<bool>::Parse(value.data(), &dontSetExceptionHandler);
				}
			}
		}

		SpawnerMain::CmdLineParse(pArg);
	}

	if (Debug::LogEnabled)
	{
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
	}
	else
	{
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
		if (pINI->ReadString(GameStrings::General(), GameStrings::Name, "", Phobos::readBuffer) <= 0)
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

	for (auto& module : Patch::ModuleDatas)
	{
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
				if (vi2.dwBuildNumber < 21996)
				{

					if (!bHaveVerFromKernel32) // we failed above; let's hope this would be useful
						aVer += std::to_string(vi2.dwMajorVersion) + "." + std::to_string(vi2.dwMinorVersion);

					aVer += (" ");

					if (vi2.szCSDVersion[0])
						aVer += (PhobosCRT::WideStringToString(vi2.szCSDVersion) + " ");

				}
				else
				{
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

	for (auto& dlls : Patch::ModuleDatas)
	{
		Debug::LogDeferred("Module [(%d) %s: Base address = %x]\n", i++, dlls.ModuleName.c_str(), dlls.BaseAddr);

		if (IS_SAME_STR_(dlls.ModuleName.c_str(), "cncnet5.dll"))
		{
			Debug::FatalErrorAndExit("This dll dont need cncnet5.dll to run!, please remove first");
		}
		else if (IS_SAME_STR_(dlls.ModuleName.c_str(), ARES_DLL_S))
		{
			Debug::FatalErrorAndExit("This dll dont need Ares.dll to run!, please remove first");
		}
		else if (IS_SAME_STR_(dlls.ModuleName.c_str(), PHOBOS_DLL_S))
		{
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

	if (!Phobos::Otamaa::ExeTerminated)
	{
		Phobos::Otamaa::ExeTerminated = true;

		//for (auto& datas : Patch::ModuleDatas) {
		//	for (size_t i = 0; i < Patch::ModuleDatas.size(); ++i) {
		//		if (i != 0 || (datas.Handle && datas.Handle != INVALID_HANDLE_VALUE)) {
		//			CloseHandle(datas.Handle);
		//		}
		//	}
		//}

		for (auto& handle : Handles::Array) {
			handle->detachptr();
		}
		Handles::Array.clear();
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

	for (auto& module : Patch::ModuleDatas)
	{
		if (IS_SAME_STR_(module.ModuleName.c_str(), "ntdll.dll"))
		{
			ntdll = module.Handle;
			break;
		}
	}

	if (ntdll != NULL)
	{

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

static void __cdecl PatchExit(int uExitCode)
{
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

static CriticalSection critSec3, critSec4;
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

	static char* _strtok(char* str, const char* delim)
	{
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
	}
	else
	{
		Debug::LogDeferred("Failed to change protection for Characteristics.\n");
	}
}

NOINLINE bool IsGamemdExe(HMODULE curProc)
{

	constexpr static const wchar_t* gameExecutables[] = {
		L"gamemd.exe",      // Yuri's Revenge
		L"gamepp.exe",      // Possible variant
	};

	constexpr static size_t executableCount = sizeof(gameExecutables) / sizeof(gameExecutables[0]);


	wchar_t filename[MAX_PATH];
	GetModuleFileNameW(curProc, filename, MAX_PATH);

	// Get just the filename without path
	const wchar_t* execName = wcsrchr(filename, L'\\');
	if (execName)
	{
		execName++; // Skip the backslash
	}
	else
	{
		execName = filename; // No path separator found
	}

	std::wstring path(filename);
	std::ranges::transform(path, path.begin(), ::towlower);

	for (size_t i = 0; i < executableCount; ++i) {
		if (path.find(gameExecutables[i]) != std::wstring::npos) {
			return true;
		}
	}

	return false;
}

LPVOID saved_lpReserved;
bool IsInitialized = false;

NOINLINE void ApplyEarlyFuncs()
{

	//make sure is correct executable loading the dll
	//if(IsGamemdExe((HMODULE)Patch::CurrentProcess))
	{


		//std::thread([]() {
		//	EnableLargeAddressAwareFlag(Patch::CurrentProcess);
		//}).detach();
		if (!IsInitialized)
			exit(ERROR);

		MH_Initialize();
		//Imports::SetCapture = SetCapture_Hook;
		//Imports::SetCursorPos = SetCursorPos_Hook;

		//auto mod = GetModuleHandleW(L"ntdll.dll");

		//if (auto target = (FnRtl)GetProcAddress(mod, "RtlGetAppContainerNamedObjectPath")) {
		//	MH_CreateHook(target, &HookedRtlGetAppContainerNamedObjectPath, (void**)&RealFn);
		//	MH_EnableHook(target);
		//};

		TheMemoryPoolCriticalSection = &critSec4;
		TheDmaCriticalSection = &critSec3;

		Mem::preMainInitMemoryManager();

		const auto time = Debug::GetCurTimeA();
		Patch::Apply_TYPED<DWORD>(0x7B853C, { 1 });
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

		len = Patch::GetSection(Phobos::hInstance, SYRINGE_HOOKS_SECTION_NAME, &buffer);
		Debug::LogDeferred("Applying %d Syringe hook(s).\n", std::distance((hookdecl*)buffer, (hookdecl*)((DWORD)buffer + len)));

		Initasmjit();

		Phobos::ExecuteLua();

		char buf[1024] {};

		if (GetEnvironmentVariable("__COMPAT_LAYER", buf, sizeof(buf)))
		{
			Debug::LogDeferred("Compatibility modes detected : %s .\n", buf);
		}
	}
}

#include <Misc/CustomMemoryManager.h>

void InitializeCustomMemorySystem()
{
	Debug::LogDeferred("Initializing Custom Memory System...\n");

	// Run signature safety check
	// if (!CustomMemoryManager::RunSignatureSafetyCheck())
	// {
	// 	Debug::LogDeferred("WARNING: Potential signature conflicts detected!\n");
	// 	Debug::LogDeferred("Consider using CustomMemoryManager::RegenerateSignatures() if issues occur.\n");
	// }

	/*
		Debug::LogDeferred("Custom Memory System initialization complete!\n");
	*/


}

static bool startPatching = false;
static DWORD OriginalCodeProtect = 0;
static DWORD OriginalDataProtect = 0;

struct ImageSectionInfo {
    LPVOID BaseOfCode;
    LPVOID BaseOfData;
    SIZE_T SizeOfCode;
    SIZE_T SizeOfData;
};

class MapViewOfFileClass {
 public:
    explicit MapViewOfFileClass(const wchar_t *fileName):
    File(INVALID_HANDLE_VALUE),
    FileMapping(NULL),
    FileBase(NULL),
    DosHeader(NULL),
    NTHeader(NULL),
    OptionalHeader(NULL),
    SectionHeaders(NULL)
{
    File = CreateFileW(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		if (File != INVALID_HANDLE_VALUE) {
			FileMapping = CreateFileMapping(File, NULL, PAGE_READONLY, 0, 0, NULL);

			if (FileMapping != NULL) {
				FileBase = MapViewOfFile(FileMapping, FILE_MAP_READ, 0, 0, 0);

				if (FileBase != NULL) {
					DosHeader = (PIMAGE_DOS_HEADER)FileBase;

					if (DosHeader->e_magic == IMAGE_DOS_SIGNATURE) {
						NTHeader = (PIMAGE_NT_HEADERS)((uint8_t *)DosHeader + DosHeader->e_lfanew);

						if (NTHeader->Signature == IMAGE_NT_SIGNATURE) {
							OptionalHeader = (PIMAGE_OPTIONAL_HEADER)&NTHeader->OptionalHeader;
							SectionHeaders = IMAGE_FIRST_SECTION(NTHeader);
						}
					}
				}
			}
		}
	}

    ~MapViewOfFileClass(){
		if (FileBase != NULL)
			UnmapViewOfFile(FileBase);
		if (FileMapping != NULL)
			CloseHandle(FileMapping);
		if (File != INVALID_HANDLE_VALUE)
			CloseHandle(File);
	}

    LPVOID GetMapViewOfFile() const { return FileBase; }
    PIMAGE_DOS_HEADER GetDosHeader() const { return DosHeader; }
    PIMAGE_NT_HEADERS GetNtHeader() const { return NTHeader; }
    PIMAGE_OPTIONAL_HEADER GetOptionalHeader() const { return OptionalHeader; }
    PIMAGE_SECTION_HEADER GetSectionHeaders() const { return SectionHeaders; }
    WORD GetSectionHeaderCount() const { return NTHeader ? NTHeader->FileHeader.NumberOfSections : 0; }

private:
    HANDLE File;
    HANDLE FileMapping;
	LPVOID FileBase;
    PIMAGE_DOS_HEADER DosHeader;
    PIMAGE_NT_HEADERS NTHeader;
    PIMAGE_OPTIONAL_HEADER OptionalHeader;
    PIMAGE_SECTION_HEADER SectionHeaders;
};

bool GetModuleSectionInfo(ImageSectionInfo &info)
{
    wchar_t fileName[MAX_PATH] = { 0 };

    if (GetModuleFileNameW(NULL, fileName, std::size(fileName)) != 0) {
        MapViewOfFileClass mapView(fileName);
        PIMAGE_OPTIONAL_HEADER OptionalHeader = mapView.GetOptionalHeader();

        if (OptionalHeader != NULL) {
            info.BaseOfCode = LPVOID(OptionalHeader->ImageBase + OptionalHeader->BaseOfCode);
            info.BaseOfData = LPVOID(OptionalHeader->ImageBase + OptionalHeader->BaseOfData);
            info.SizeOfCode = SIZE_T(OptionalHeader->SizeOfCode);
            info.SizeOfData = SIZE_T(OptionalHeader->SizeOfInitializedData + OptionalHeader->SizeOfUninitializedData);

            return true;
        }
    }
    return false;
}

bool StartPatching() {
	if(startPatching){
		return true;
	}

	bool success = false;
    ImageSectionInfo info;

	if (GetModuleSectionInfo(info)) {
        success = true;
        HANDLE process = GetCurrentProcess();
        if (VirtualProtectEx(process, info.BaseOfCode, info.SizeOfCode, PAGE_EXECUTE_READWRITE, &OriginalCodeProtect) == FALSE) {
            success = false;
        }
        if (VirtualProtectEx(process, info.BaseOfData, info.SizeOfData, PAGE_EXECUTE_READWRITE, &OriginalDataProtect) == FALSE) {
            success = false;
        }
    }

	startPatching = success;

    return success;
}

bool StopPatching()
{
    bool success = false;
    DWORD old_protect;
    ImageSectionInfo info;

    if (GetModuleSectionInfo(info)) {
        success = true;
        HANDLE process = Patch::CurrentProcess;
        if (VirtualProtectEx(process, info.BaseOfCode, info.SizeOfCode, OriginalCodeProtect, &old_protect) == FALSE) {
            success = false;
        }
        if (VirtualProtectEx(process, info.BaseOfData, info.SizeOfData, OriginalDataProtect, &old_protect) == FALSE) {
            success = false;
        }
    }

    startPatching = false;

    return success;
}

#include <LaserDrawClass.h>

#define HAVE_MREMAP     0
#define MORECORE_CONTIGUOUS 0
#define DLMALLOC_EXPORT /* if you want explicit exports - but we'll avoid exporting malloc/free */
#define USE_DL_PREFIX 1

#define dlmalloc  mydlmalloc
#define dlfree    mydlfree
#define dlrealloc mydlrealloc
#define dlcalloc  mydlcalloc

#include <ExtraHeaders/DougLeaMalloc.c>

template <typename T>
struct DlAllocator
{
	using value_type = T;
	DlAllocator() noexcept { }
	template <class U> DlAllocator(const DlAllocator<U>&) noexcept { }
	T* allocate(std::size_t n)
	{
		if (n == 0) return nullptr;
		size_t bytes = n * sizeof(T);
		void* p = mydlmalloc(bytes);
		if (!p) throw std::bad_alloc();
		return static_cast<T*>(p);
	}
	void deallocate(T* p, std::size_t n) noexcept
	{
		(void)n;
		if (p) mydlfree(p);
	}
	template <class U> bool operator==(DlAllocator<U> const&) const noexcept { return true; }
	template <class U> bool operator!=(DlAllocator<U> const&) const noexcept { return false; }
};

struct AllocInfo
{
	void* real_ptr;    // pointer returned by mydlmalloc (base)
	size_t requested;  // original requested size
	size_t sizeEntry;  // rounded sizeEntry used for footer/header
};

static inline void* key_empty() { return nullptr; }
static inline void* key_tomb() { return (void*)1; }

struct SimpleMapEntry
{
	void* key;      // user pointer returned to game (user-visible)
	AllocInfo val;
};

struct SimpleMap
{
	SimpleMapEntry* table;
	size_t capacity; // power of two
	size_t count;
};

static SimpleMap* g_simple_map = nullptr;
static SRWLOCK g_map_lock = SRWLOCK_INIT;
static const size_t HEADER_SIZE = 4; // sbh header bytes written before user pointer

// ---------- helper: compute SBH sizeEntry (matches decompiled) ----------
static inline size_t compute_sizeEntry(size_t requested)
{
	return (requested + 23) & ~(size_t)0xF;
}

// ---------- create SBH-style block backed by dlmalloc ----------
static void* create_sbh_block(size_t requested)
{
	if (requested == 0) requested = 1;
	size_t sizeEntry = compute_sizeEntry(requested);
	size_t allocNeeded = sizeEntry + HEADER_SIZE + 8;
	void* real = mydlmalloc(allocNeeded);
	if (!real) return nullptr;
	uint32_t stored = static_cast<uint32_t>(sizeEntry + 1);
	*(uint32_t*)real = stored;
	void* user = (void*)((char*)real + HEADER_SIZE);
	uint32_t* footer = (uint32_t*)((char*)real + (sizeEntry - 4));
	*footer = stored;
	return user;
}

static size_t simple_hash_ptr(void* p)
{
	uintptr_t x = (uintptr_t)p;
	x = x ^ (x >> 16);
	return (size_t)x;
}

static bool simple_map_init(size_t initial_pow2 = 1024)
{
	if (g_simple_map) return true;
	if (initial_pow2 < 16) initial_pow2 = 16;
	void* m = mydlmalloc(sizeof(SimpleMap));
	if (!m) return false;
	g_simple_map = (SimpleMap*)m;
	g_simple_map->capacity = initial_pow2;
	g_simple_map->count = 0;
	size_t bytes = g_simple_map->capacity * sizeof(SimpleMapEntry);
	g_simple_map->table = (SimpleMapEntry*)mydlmalloc(bytes);
	if (!g_simple_map->table)
	{
		mydlfree(g_simple_map);
		g_simple_map = nullptr;
		return false;
	}
	for (size_t i = 0; i < g_simple_map->capacity; ++i)
	{
		g_simple_map->table[i].key = key_empty();
	}
	return true;
}

static void simple_map_destroy()
{
	if (!g_simple_map) return;
	mydlfree(g_simple_map->table);
	mydlfree(g_simple_map);
	g_simple_map = nullptr;
}

static bool simple_map_resize(size_t newcap)
{
	if (!g_simple_map) return false;
	SimpleMapEntry* newtab = (SimpleMapEntry*)mydlmalloc(newcap * sizeof(SimpleMapEntry));
	if (!newtab) return false;
	for (size_t i = 0; i < newcap; ++i) newtab[i].key = key_empty();
	for (size_t i = 0; i < g_simple_map->capacity; ++i)
	{
		void* k = g_simple_map->table[i].key;
		if (k != key_empty() && k != key_tomb())
		{
			size_t h = simple_hash_ptr(k) & (newcap - 1);
			while (newtab[h].key != key_empty()) h = (h + 1) & (newcap - 1);
			newtab[h] = g_simple_map->table[i];
		}
	}
	mydlfree(g_simple_map->table);
	g_simple_map->table = newtab;
	g_simple_map->capacity = newcap;
	return true;
}

static bool simple_map_insert(void* k, const AllocInfo& v)
{
	if (!g_simple_map)
	{
		if (!simple_map_init(1024)) return false;
	}

	if ((g_simple_map->count + 1) * 10 >= g_simple_map->capacity * 6)
	{
		if (!simple_map_resize(g_simple_map->capacity * 2)) return false;
	}
	size_t idx = simple_hash_ptr(k) & (g_simple_map->capacity - 1);
	while (true)
	{
		void* cur = g_simple_map->table[idx].key;
		if (cur == key_empty() || cur == key_tomb())
		{
			g_simple_map->table[idx].key = k;
			g_simple_map->table[idx].val = v;
			g_simple_map->count++;
			return true;
		}
		if (cur == k)
		{
			g_simple_map->table[idx].val = v;
			return true;
		}
		idx = (idx + 1) & (g_simple_map->capacity - 1);
	}

	return false;
}

static AllocInfo* simple_map_find(void* k)
{
	if (!g_simple_map) return nullptr;
	size_t idx = simple_hash_ptr(k) & (g_simple_map->capacity - 1);
	size_t start = idx;
	while (true)
	{
		void* cur = g_simple_map->table[idx].key;
		if (cur == key_empty()) return nullptr;
		if (cur == k) return &g_simple_map->table[idx].val;
		idx = (idx + 1) & (g_simple_map->capacity - 1);
		if (idx == start) return nullptr;
	}
}

static bool simple_map_remove(void* k)
{
	if (!g_simple_map) return false;
	size_t idx = simple_hash_ptr(k) & (g_simple_map->capacity - 1);
	size_t start = idx;
	while (true)
	{
		void* cur = g_simple_map->table[idx].key;
		if (cur == key_empty()) return false;
		if (cur == k)
		{
			g_simple_map->table[idx].key = key_tomb();
			if (g_simple_map->count) g_simple_map->count--;
			return true;
		}
		idx = (idx + 1) & (g_simple_map->capacity - 1);
		if (idx == start) return false;
	}
}

// ---------- wrappers (no calls to original CRT or game functions) ----------
extern "C" void InitDlWrapper()
{
	simple_map_init(1024);
}

extern "C" void ShutdownDlWrapper()
{
	simple_map_destroy();
}

extern "C" void* __cdecl game_malloc_wrapper(size_t size)
{
	if (!g_simple_map) InitDlWrapper();
	void* user = create_sbh_block(size);
	if (!user) return nullptr;
	AllocInfo info;
	info.real_ptr = (void*)((char*)user - HEADER_SIZE);
	info.requested = size;
	info.sizeEntry = compute_sizeEntry(size);

	AcquireSRWLockExclusive(&g_map_lock);
	simple_map_insert(user, info);
	ReleaseSRWLockExclusive(&g_map_lock);
	return user;
}

extern "C" void __cdecl game_free_wrapper(void* user_ptr)
{
	if (!user_ptr) return;
	AcquireSRWLockExclusive(&g_map_lock);
	AllocInfo* it = simple_map_find(user_ptr);
	if (it)
	{
		void* real = it->real_ptr;
		simple_map_remove(user_ptr);
		ReleaseSRWLockExclusive(&g_map_lock);
		mydlfree(real);
		return;
	}
	ReleaseSRWLockExclusive(&g_map_lock);
}

extern "C" void* __cdecl game_realloc_wrapper(void* user_ptr, size_t newsize)
{

	if (!g_simple_map) InitDlWrapper();
	if (!user_ptr)
	{
		return game_malloc_wrapper(newsize);
	}
	AcquireSRWLockExclusive(&g_map_lock);
	AllocInfo* it = simple_map_find(user_ptr);
	if (it)
	{
		AllocInfo info = *it;
		ReleaseSRWLockExclusive(&g_map_lock);
		void* new_user = create_sbh_block(newsize);
		if (!new_user) return nullptr;
		size_t tocopy = info.requested < newsize ? info.requested : newsize;
		memcpy(new_user, user_ptr, tocopy);
		game_free_wrapper(user_ptr);
		return new_user;
	}
	ReleaseSRWLockExclusive(&g_map_lock);
	void* fresh = game_malloc_wrapper(newsize);
	return fresh;
}

extern "C" size_t __cdecl game_msize_wrapper(void* user_ptr)
{
	if (!user_ptr) return 0;
	if (!g_simple_map) return 0;
	AcquireSRWLockShared(&g_map_lock);
	AllocInfo* it = simple_map_find(user_ptr);
	if (it)
	{
		size_t req = it->requested;
		ReleaseSRWLockShared(&g_map_lock);
		return req;
	}
	ReleaseSRWLockShared(&g_map_lock);
	return 0;
}

extern "C" void* __cdecl game_calloc_wrapper(size_t num, size_t size)
{

	size_t total = num * size;
	void* user = game_malloc_wrapper(total ? total : 1);
	if (user)
	{
		// use memset
		memset(user, 0, total ? total : 1);
	}
	return user;
}

static bool allocHookInit = false;
using PVFV = void(__cdecl*)(void);
using initterm_t = void(__cdecl*)(PVFV*, PVFV*);

// Real initterm function address (unchanged)
constexpr uintptr_t REAL_INITTERM_ADDR = 0x007CBED3;

// Caller sites you provided (addresses that call REAL_INITTERM_ADDR)
constexpr uintptr_t INITTERM_CALLERS[] = {
	0x007CBDC4,
	0x007CBDD3,
	0x007CBE8B,
	0x007CBE9C
};
constexpr size_t INITTERM_CALLER_COUNT = sizeof(INITTERM_CALLERS) / sizeof(INITTERM_CALLERS[0]);

// flag flipped by wrapper after real initterm returns
static volatile LONG g_crt_ready = 0;

// The wrapper that callers will be patched to call instead of calling REAL_INITTERM_ADDR directly.
// Must match signature: void __cdecl _initterm(PVFV* first, PVFV* last)
extern "C" void __cdecl initterm_caller_wrapper(PVFV* first, PVFV* last)
{
	// Call the real initterm directly (we didn't overwrite the real function)
	initterm_t real = reinterpret_cast<initterm_t>(REAL_INITTERM_ADDR);
	// call it
	real(first, last);

	// mark ready
	InterlockedExchange(&g_crt_ready, 1);
}

// Helper: patch a near CALL (E8 rel32) at callSiteAddr to call newTarget
static bool PatchNearCall(uintptr_t callSiteAddr, void* newTarget)
{
	BYTE* p = reinterpret_cast<BYTE*>(callSiteAddr);
	// We expect a near CALL opcode 0xE8
	if (p[0] != 0xE8)
	{
		// Not a direct near call; skip (or handle other patterns manually)
		return false;
	}

	DWORD oldProtect;
	if (!VirtualProtect(p, 5, PAGE_EXECUTE_READWRITE, &oldProtect)) return false;

	// compute relative offset: target - (callSite + 5)
	intptr_t rel = (intptr_t)newTarget - (intptr_t)(callSiteAddr + 5);
	*(int32_t*)(p + 1) = (int32_t)rel;

	// restore protection and flush icache
	VirtualProtect(p, 5, oldProtect, &oldProtect);
	FlushInstructionCache(GetCurrentProcess(), (LPCVOID)callSiteAddr, 5);
	return true;
}

// Installer: patch all known callers to call our wrapper
static void InstallInittermCallerPatches()
{
	for (size_t i = 0; i < INITTERM_CALLER_COUNT; ++i)
	{
		uintptr_t site = INITTERM_CALLERS[i];
		PatchNearCall(site, (void*)&initterm_caller_wrapper);
	}
}

extern "C" void WaitAndInstallAllocHooks()
{	
	// 1) Install caller patches (redirect callers -> our wrapper)
	InstallInittermCallerPatches();

	// 2) Wait for wrapper to flip the flag (timeout safety)
	const DWORD timeout_ms = 30000;
	DWORD start = GetTickCount();
	while (InterlockedCompareExchange(&g_crt_ready, 0, 0) == 0)
	{
		if ((DWORD)(GetTickCount() - start) > timeout_ms)
		{
			MessageBoxA(NULL, "Timeout waiting for CRT ready flag", "dlwrap", MB_OK | MB_ICONERROR);
			return;
		}
		Sleep(5);
	}

	if (!allocHookInit)
	{
		//  /**
		// *  C memory functions.
		// */
		Hook_Function(0x7C93E8, &game_free_wrapper);
		Hook_Function(0x7D0F45, &game_realloc_wrapper);
		Hook_Function(0x7D3374, &game_calloc_wrapper);
		Hook_Function(0x7C9430, &game_malloc_wrapper);
		Hook_Function(0x7D107D, &game_msize_wrapper);

		/**
		 *  C++ new and delete.
		 */

		allocHookInit = true;
	}

	/**
	 *  Standard functions.
	 */
	 //Hook_Function(0x7D5408, &strdup);
	 //Hook_Function(0x7C9CC2, &std::strtok);
}

DWORD WINAPI InitThreadProc(LPVOID)
{
	// DO NOT call any hooking code here for this test.
	// Minimal allocator test:
	//if (!g_simple_map) InitDlWrapper(); // or simple_map_init()

	//early stuffs that can be installed without breaking game
	Hook_Function(0x7C8E17, &std::malloc);
	Hook_Function(0x7C8B3D, &std::free);

	//WaitAndInstallAllocHooks();
	
	return 0;
}

BOOL APIENTRY DllMain(HANDLE hInstance, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		if (IsGamemdExe(nullptr))
		{
			Patch::CurrentProcess = GetCurrentProcess();

            if (!StartPatching()) {
                return FALSE;
            }

			DisableThreadLibraryCalls((HMODULE)hInstance);

			//Patch_Jump(0x6BBFC9, &_set_fp_mode);

			//HANDLE h = CreateThread(nullptr, 0, InitThreadProc, nullptr, 0, nullptr);
			//if (h) CloseHandle(h);

			//InitHeapPatches(HeapCreate(0, 1 << 20, 0));

			Phobos::hInstance = hInstance;
			saved_lpReserved = lpReserved;
			IsInitialized = true;

#if defined(NO_SYRINGE)
			ApplyEarlyFuncs();
			//LuaData::ApplyCoreHooks();
			Phobos::ExeRun();
#endif
		}

	}
	break;
	case DLL_PROCESS_DETACH:
	{

		if (!StopPatching()) {
			return FALSE;
		}

		ShutdownDlWrapper();

		bool g_isProcessTerminating = (lpReserved != nullptr);

		if (g_isProcessTerminating && IsInitialized)
		{
			Multithreading::ShutdownMultitheadMode();
			Debug::DeactivateLogger();
			gJitRuntime.reset();
			//Mem::shutdownMemoryManager();
			MH_Uninitialize();
		}
	}
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

	if (SessionClass::IsSingleplayer() && !ScenarioClass::ScenarioSaved())
	{
		ScenarioClass::ScenarioSaved = true;
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

ASMJIT_PATCH(0x6BBE6A, WinMain_AllowMultipleInstances, 0x6)
{
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
DEFINE_HOOK(0x7CD810, Game_ExeRun, 0x9)
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
