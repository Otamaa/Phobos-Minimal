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
		Debug::InitLogger(); 
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


#include <Misc/Multithread.h>

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
	{
		if (!IsInitialized)
			exit(ERROR);

		const auto time = Debug::GetCurTimeA();


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
        HANDLE process = Patch::CurrentProcess;
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

float WWMath_Sin(double radians)
{
	COMPILETIMEEVAL float radToIndex = std::bit_cast<float>(0x4522F983u);

	// Convert angle to fixed-point table index
	const long long idx64 = static_cast<long long>(radians * radToIndex);

	const bool isOdd = (idx64 & 1) != 0;               // v2
	int idx = static_cast<int>((idx64 / 2) & 0x80001FFF); // v3

	// Sign-extend the 13-bit index (because &0x80001FFF keeps only certain bits)
	if (idx < 0) {
		int t = (idx - 1) | 0xFFFFE000;
		idx = t + 1 + ((t + 1) < 0 ? 0x2000 : 0);
	}

	// Odd-bit correction (round up if necessary)
	if (isOdd && idx < 8191) {
		idx++;
	}


	return Math::FastMath_sin_Table[idx];
}

float WWMath_Cos(double radians)
{
	COMPILETIMEEVAL float radToIndex = std::bit_cast<float>(0x4522F983u);

	int64_t t = (int64_t)(radians * radToIndex);

	int bit = t & 1;
	int idx = (t >> 1) & 0x80001FFF;

	if (idx < 0) {
		idx = ((idx - 1) | 0xFFFFE000) + 1;
		idx += (idx < 0) ? 0x2800 : 0x0800; // 10240 or 2048
	} else {
		idx += 2048;
	}

	if (bit && idx < 10239)
		idx++;

	return Math::FastMath_sin_Table[idx];
}

float WWMath_sqrt(double n)
{
	union FastMathUnion {
		float f;
		unsigned int i;
	};

	// Handle zero early
	if (n == 0.0)
		return 0.0f;

	// Use absolute value, but only via their original codepath
	double v = n;
	if (n < 0.0)
		v = -n;

	// Reinterpret bits as IEEE754 float
	FastMathUnion num;
	num.f = static_cast<float>(v);

	// Extract mantissa (lower 23 bits)
	uint32_t mant = num.i & 0x7FFFFF;

	// Extract unbiased exponent
	int exp = (num.i >> 23) - 127;

	// If exponent is odd, normalize the mantissa
	if (exp & 1)
		mant |= 0x800000u;

	// Build the result float from:
	//   - new exponent: (exp >> 1) + 127
	//   - table lookup from mantissa
	uint32_t out_bits =
		(((exp >> 1) + 127) << 23) |
		Math::FastMath_sqrt_Table[mant >> 10];

	FastMathUnion out;
	out.i = out_bits;
	return out.f;
}

bool InspectMathDetailed()
{
	struct MathTest {
		const char* name;
	};

	struct DoubleMathTest : public MathTest {
		std::pair<const char*, double> first;
		std::pair<const char*, double> second;

		DoubleMathTest(const char* testName, const char* firstname , double firstRes, const char* secondname,double secondRes)
			: MathTest { testName }, first { firstname, firstRes }, second { secondname , secondRes } { }
	};

	struct FloatMathTest : public MathTest {
		std::pair<const char*, float>  first;
		std::pair<const char*, float>  second;

		FloatMathTest(const char* testName, const char* firstname, float firstRes, const char* secondname, float secondRes)
			: MathTest{ testName }, first { firstname ,firstRes }, second { secondname , secondRes } {}
	};

	struct IntMathTest : public MathTest {
		std::pair<const char*, int> first;
		std::pair<const char*, int>  second;

		IntMathTest(const char* testName, const char* firstname, int firstRes, const char* secondname, int secondRes)
			: MathTest{ testName }, first { firstname ,firstRes }, second { secondname , secondRes } {}
	};

	// Collect all test cases
	std::vector<DoubleMathTest> testsdouble;
	std::vector<FloatMathTest> testsfloat;
	std::vector<IntMathTest> testsint;

	/*testsint.emplace_back(*/
	//	"cast testing , game ftol infamous for true'ing (F2I(-5.00) == -4.00)",
	//	"dll cast",
	//	(int)(-5.00),
	//	"game ftol",
	//	Math::F2I(-5.00)
	//);
	//
	//tests.emplace_back(
	//	"expected : 0.3763770469559380854890894443664",
	//	Unsorted::LevelHeight / gcem::sqrt(float(Unsorted::LevelHeight * Unsorted::LevelHeight + Unsorted::LeptonsPerCell * Unsorted::LeptonsPerCell)),
	//	Unsorted::LevelHeight / std::sqrt(float(Unsorted::LevelHeight * Unsorted::LevelHeight + Unsorted::LeptonsPerCell * Unsorted::LeptonsPerCell))
	//);

	//tests.emplace_back(
	//	"expected : 0.9264665771223091335116047861327",
	//	Unsorted::LeptonsPerCell / gcem::sqrt(float(Unsorted::LevelHeight * Unsorted::LevelHeight + Unsorted::LeptonsPerCell * Unsorted::LeptonsPerCell)),
	//	Unsorted::LeptonsPerCell / std::sqrt(float(Unsorted::LevelHeight * Unsorted::LevelHeight + Unsorted::LeptonsPerCell * Unsorted::LeptonsPerCell))
	//);

	//tests.emplace_back(
	//	"expected : 0.3522530794922131411764879370407",
	//	Unsorted::LevelHeight / gcem::sqrt(float(2 * Unsorted::LevelHeight * Unsorted::LevelHeight + Unsorted::LeptonsPerCell * Unsorted::LeptonsPerCell)),
	//	Unsorted::LevelHeight / std::sqrt(float(2 * Unsorted::LevelHeight * Unsorted::LevelHeight + Unsorted::LeptonsPerCell * Unsorted::LeptonsPerCell))
	//);

	//tests.emplace_back(
	//	"expected : 0.8670845033654477321267395373309",
	//	Unsorted::LeptonsPerCell / gcem::sqrt(float(2 * Unsorted::LevelHeight * Unsorted::LevelHeight + Unsorted::LeptonsPerCell * Unsorted::LeptonsPerCell)),
	//	Unsorted::LeptonsPerCell / std::sqrt(float(2 * Unsorted::LevelHeight * Unsorted::LevelHeight + Unsorted::LeptonsPerCell * Unsorted::LeptonsPerCell))
	//);

	//tests.emplace_back(
	//	"expected : 0.5333964609104418418483761938761",
	//	Unsorted::CellHeight / gcem::sqrt(float(2 * Unsorted::CellHeight * Unsorted::CellHeight + Unsorted::LeptonsPerCell * Unsorted::LeptonsPerCell)),
	//	Unsorted::CellHeight / std::sqrt(float(2 * Unsorted::CellHeight * Unsorted::CellHeight + Unsorted::LeptonsPerCell * Unsorted::LeptonsPerCell))
	//);

	//tests.emplace_back(
	//	"expected : 0.6564879518897745745826168540013",
	//	Unsorted::LeptonsPerCell / gcem::sqrt(float(2 * Unsorted::CellHeight * Unsorted::CellHeight + Unsorted::LeptonsPerCell * Unsorted::LeptonsPerCell)),
	//	Unsorted::LeptonsPerCell / std::sqrt(float(2 * Unsorted::CellHeight * Unsorted::CellHeight + Unsorted::LeptonsPerCell * Unsorted::LeptonsPerCell))
	//);

	/*testsfloat.emplace_back(
		"cos(1.570748388432313)",
		WWMath_Cos(1.570748388432313),
		(float)std::cos(1.570748388432313)
	);

	testsfloat.emplace_back(
		"sin(1.570748388432313)",
		WWMath_Sin(1.570748388432313),
		(float)std::sin(1.570748388432313)
	);

	testsfloat.emplace_back(
		"cos(0.7853262558535721)",
		WWMath_Cos(0.7853262558535721),
		(float)std::cos(0.7853262558535721)
	);

	testsfloat.emplace_back(
		"sin(0.7853262558535721)",
		WWMath_Sin(0.7853262558535721),
		(float)std::sin(0.7853262558535721)
	);

	testsfloat.emplace_back(
		"1 / sqrt(5)",
		1.0 / WWMath_sqrt(5.0),
		1.0 / (float)std::sqrt(5.0)
	);
	
	testsdouble.emplace_back(
		"2 / sqrt(5)",
		2.0 / WWMath_sqrt(5.0),
		2.0 / (float)std::sqrt(5.0)
	);*/

	//constexpr double base_ = gcem::pow(256.0, 2.0);
	//testsdouble.emplace_back(
	//	"POW 1",
	//	"gcem" , base_,
	//	"std" , std::pow(256.0, 2.0)
	//);

	////testsdouble.emplace_back(
	////	"POW 2",
	////	base_,
	////	Game_pow2(256.0, 2.0)
	////);

	//constexpr float tile_diag = gcem::sqrt(base_ + base_);
	//static constexpr reference<double, 0xAC1368> TileDiagonal;
	//constexpr double MAP_TILE_DIAGONAL = std::bit_cast<double>(0x4076A09E60000000ull);
	//testsdouble.emplace_back(
	//	"Tile Diagonal1 sqrt",
	//	"gcem" , tile_diag,
	//	"std"  , std::sqrt(base_ + base_)
	//);


	//// = 1.047197551196598 (π/3, 60 degrees)
	//constexpr double PI_BY_THREE = std::bit_cast<double>(0x3FF0C15238732D65ull);
	//constexpr double tan_ = gcem::tan(Math::PI_BY_TWO_APPROX - PI_BY_THREE);
	//constexpr double floor = gcem::tan(Math::PI_BY_TWO_APPROX - PI_BY_THREE) * tile_diag * 0.5;
	//double std_floor = std::tan(Math::PI_BY_TWO_APPROX - PI_BY_THREE) * tile_diag * 0.5;
	//static constexpr reference<int, 0xAC13C8> CellHeight;
	//testsdouble.emplace_back(
	//	"Floor Height tan",
	//	 "gcem" , tan_,
	//	 "std"   , std::tan(Math::PI_BY_TWO_APPROX - PI_BY_THREE)
 //	);

	//testsdouble.emplace_back(
	//	"Floor Height overral",
	//	"gcem", floor,
	//	"std", std_floor
	//);

	// Run all tests
	int passed = 0;
	int failed = 0;

	Debug::Log("\n=== Math Validation Report ===\n\n");

	for (const auto& test : testsdouble)
	{
		if (*(uint32_t*)&test.first.second == *(uint32_t*)&test.second.second) {
			Debug::Log("[PASS] %s\n", test.name);
			passed++;
		} else {
			Debug::Log("[FAIL] %s\n", test.name);
			Debug::Log("       %s:    %.20f (0x%08X)\n",
					  test.first.first, test.first.second, *(uint32_t*)&test.first.second);
			Debug::Log("       %s:   %.20f (0x%08X)\n",
					  test.second.first, test.second.second, *(uint32_t*)&test.second);
			Debug::Log("       Diff:     %.20e\n\n",
					  test.first.second - test.second.second);
			failed++;
		}
	}

	for (const auto& test : testsfloat)
	{
		if (*(uint32_t*)&test.first.second == *(uint32_t*)&test.second.second)
		{
			Debug::Log("[PASS] %s\n", test.name);
			passed++;
		}
		else
		{
			Debug::Log("[FAIL] %s\n", test.name);
			Debug::Log("       %s:    %.20f (0x%08X)\n",
					  test.first.first, test.first.second, *(uint32_t*)&test.first.second);
			Debug::Log("       %s:   %.20f (0x%08X)\n",
					  test.second.first, test.second.second, *(uint32_t*)&test.second);
			Debug::Log("       Diff: %.20e\n\n",
					  test.first.second - test.second.second);
			failed++;
		}
	}

	for (const auto& test : testsint) {
		if (*(uint32_t*)&test.first.second == *(uint32_t*)&test.second.second) {
			Debug::Log("[PASS] %s\n", test.name);
			passed++;
		}
		else {
			Debug::Log("[FAIL] %s\n", test.name);
			Debug::Log("       %s:    %d (0x%08X)\n",
					  test.first.first, test.first.second, *(uint32_t*)&test.first.second);
			Debug::Log("       %s:   %d (0x%08X)\n",
					  test.second.first, test.second.second, *(uint32_t*)&test.second);
			Debug::Log("       Diff: %d\n\n",
					  test.first.second - test.second.second);
			failed++;
		}
	}

	Debug::Log("\n=== Summary ===\n");
	Debug::Log("Passed: %d\n", passed);
	Debug::Log("Failed: %d\n", failed);
	Debug::Log("Total:  %d\n", passed + failed);

	if (failed > 0)
	{
		Debug::FatalError(
			"\n%d math validation(s) failed!\n"
			"gcem constexpr != std runtime values\n"
			"This WILL cause multiplayer desyncs.\n",
			failed
		);
		return false;
	}

	
	struct Generate {
		double value;
		const char* name;
	};

	std::vector<Generate> gens;

	//gens.emplace_back(std::sqrt(8),"sqrt(8)");

	for (const auto& gen : gens) {
		uint64_t bits = std::bit_cast<uint64_t>(gen.value);
		Debug::Log("Generating %s :  0x%016llX\n", gen.name ,(unsigned long long)bits);
	}
	return true;
}

#include <Misc/Hooks.CRT.h>
typedef DWORD(__stdcall* FP_GetVersion)();
static COMPILETIMEEVAL referencefunc<FP_GetVersion, 0x7E1288> const Game_GetVersion {};

DWORD __stdcall GetVersion_Wrapper() {
	auto ver = Game_GetVersion.invoke()();
	CRTHooks::_set_fp_mode();
	ApplyEarlyFuncs();
	//LuaData::ApplyCoreHooks();
	Phobos::ExeRun();
	return ver;
}

bool __fastcall Parse_Command_Line(int argc, char* argv[]) {
	JMP_STD(0x52F620);
}

bool __fastcall Phobos_Parse_Command_Line(int argc, char* argv[]) {
	if (argc > 1)
	{
		Debug::LogDeferred("Parsing command line arguments...\n");
	}

	if (!Parse_Command_Line(argc, argv))
	{
		return false;
	}

	Phobos::CmdLineParse(argv, argc);

	Debug::LogDeferredFinalize();

#ifdef MATHTEST
	InspectMathDetailed();
#endif


	return true;
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
			Phobos::hInstance = hInstance;
			saved_lpReserved = lpReserved;
			CRTHooks::_set_fp_mode();

			CRTHooks::Print_FPUMode();

            if (!StartPatching()) {
                return FALSE;
            }

			DisableThreadLibraryCalls((HMODULE)hInstance);
			IsInitialized = true;
			
			MH_Initialize();

			CRTHooks::Apply();

			Patch::Apply_CALL(0x6BC08C, Phobos_Parse_Command_Line);
			Patch::Apply_CALL6(0x7CD835, GetVersion_Wrapper);
			Patch::Apply_TYPED<DWORD>(0x7B853C, { 1 });
			Patch::Apply_TYPED<char>(0x82612C + 13, { '\n' });
		}
	}
	break;
	case DLL_PROCESS_DETACH:
	{

		if (!StopPatching()) {
			return FALSE;
		}

		bool g_isProcessTerminating = (lpReserved != nullptr);

		if (g_isProcessTerminating && IsInitialized)
		{
			Multithreading::ShutdownMultitheadMode();
			Debug::DeactivateLogger();
			gJitRuntime.reset();
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

ASMJIT_PATCH(0x6BBE6A, WinMain_AllowMultipleInstances, 0x6) {
	return Phobos::Otamaa::AllowMultipleInstance ? 0x6BBED6 : 0x0;
}

ASMJIT_PATCH(0x52FE55, Scenario_Start, 0x6)
{
	auto _seed = (DWORD)Game::Seed();
	Debug::Log("Init Phobos Randomizer seed %x.\n", _seed);
	Phobos::Random::SetRandomSeed(Game::Seed());
	return 0;
}ASMJIT_PATCH_AGAIN(0x52FEB7, Scenario_Start, 0x6)

#pragma endregion
