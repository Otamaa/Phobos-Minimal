#pragma once
#pragma warning( push )
#pragma warning (disable : 4244 4127 4702)

#include <Lib/asmjit/x86.h>

#pragma warning( pop )

#include <map>
#include <unordered_map>
#include <vector>
#include <memory>

#include <Base/Always.h>
class JitErrorHandler : public asmjit::ErrorHandler
{
public:
	// Set by ApplyasmjitPatch before each assembly.<instr>() sequence so
	// errors can be attributed to a specific hook address.
	unsigned int currentHookAddr = 0;
	bool hadError = false;

	void handle_error(asmjit::Error err, const char* message, asmjit::BaseEmitter* /*origin*/) override;
};

struct FunctionTrampoline
{
	void* original_address;
	void* trampoline_address;
	std::vector<uint8_t> original_bytes;
	std::vector<uint8_t> modified_original_bytes;
	size_t hook_size;
};

struct PhobosHookers
{
	static JitErrorHandler gJitErrorHandler;
	static std::unique_ptr<asmjit::JitRuntime> gJitRuntime;
	static std::unordered_map<unsigned int, FunctionTrampoline> g_trampolines;
	static std::map<unsigned int, std::map<const void*, size_t>> Hooks;

	static void InitMinHook();
	static void Initasmjit();
	static void ApplyasmjitPatch();
	static void CleanupTrampolines();
	static bool SetupTrampoline(unsigned int target_address, size_t hook_size);

	static bool InstallSingleHook(unsigned int addr, std::map<const void*, size_t>& sm_vec0);
};