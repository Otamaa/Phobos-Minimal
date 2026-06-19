#include "Phobos.Hookers.h"

#include <Utilities/Debug.h>

#include <minhook/MinHook.h>
#include <Zydis/Zydis.h>

#include <Utilities/Handle.h>

namespace Assembly
{
	static constexpr BYTE INIT = 0x00,
		INT3 = 0xCC,
		NOP = 0x90,
		CALL = 0xE8,
		JMP = 0xE9,
		JLE = 0x7E;
};


JitErrorHandler PhobosHookers::gJitErrorHandler;

void JitErrorHandler::handle_error(asmjit::Error err, const char* message, asmjit::BaseEmitter* /*origin*/)
{
	hadError = true;
	Debug::Log(
		"AsmJit ERROR %u (%s) while assembling hook at 0x%x: %s\n",
		static_cast<unsigned int>(err),
		asmjit::DebugUtils::error_as_string(err),
		currentHookAddr,
		message ? message : "(no message)");
}


std::unique_ptr<asmjit::JitRuntime>  PhobosHookers::gJitRuntime;
std::unordered_map<unsigned int, FunctionTrampoline> PhobosHookers::g_trampolines;
std::map<unsigned int, std::map<const void*, size_t>> PhobosHookers::Hooks;

// Resolve relative operands in an encoder request to absolute addresses.
void ResolveRelativeOperands(
	ZydisEncoderRequest& req,
	ZydisDecodedInstruction const& instruction,
	ZydisDecodedOperand const* operands,
	ZyanU64 srcAddr)
{
	for (ZyanU8 i = 0; i < req.operand_count; ++i)
	{
		if (req.operands[i].type == ZYDIS_OPERAND_TYPE_IMMEDIATE)
		{
			ZyanU64 absAddr;
			if (ZYAN_SUCCESS(ZydisCalcAbsoluteAddress(
				&instruction, &operands[i], srcAddr, &absAddr)))
			{
				req.operands[i].imm.u = absAddr;
			}
		}
		else if (req.operands[i].type == ZYDIS_OPERAND_TYPE_MEMORY)
		{
			ZyanU64 absAddr;
			if (ZYAN_SUCCESS(ZydisCalcAbsoluteAddress(
				&instruction, &operands[i], srcAddr, &absAddr)))
			{
				req.operands[i].mem.displacement =
					static_cast<ZyanI64>(absAddr);
			}
		}
	}
}

std::vector<BYTE> RebuildInstructions(
	BYTE const* bytes, size_t size, DWORD originalAddr, DWORD newAddr)
{
	ZydisDecoder decoder;
	ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_COMPAT_32, ZYDIS_STACK_WIDTH_32);

	// --- Pass 1: decode all instructions and classify relative branches ---

	struct InstructionInfo
	{
		size_t srcOffset;       // offset into original bytes
		ZyanU8 srcLength;       // original instruction length
		bool intraPrologue;     // relative branch targets within the prologue
		size_t targetSrcOffset; // source offset of branch target (intra-prologue only)
		size_t outputSize;      // size in the output buffer
		size_t outputOffset;    // offset within the output buffer
		std::optional<ZydisEncoderRequest> encoderReq; // cached encoder request (relative instrs only)
	};

	std::vector<InstructionInfo> infos;
	size_t tailOffset = size; // offset of undecoded tail, if any

	{
		size_t offset = 0;
		size_t outOff = 0;
		while (offset < size)
		{
			ZydisDecodedInstruction instruction;
			ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT];

			auto const srcAddr = static_cast<ZyanU64>(originalAddr + offset);

			if (ZYAN_FAILED(ZydisDecoderDecodeFull(
				&decoder, bytes + offset, size - offset, &instruction, operands)))
			{
				Debug::Log(
					__FUNCTION__ ": Failed to decode instruction at 0x%08X, "
					"copying remaining %u bytes verbatim. This could mean "
					"there is a faulty return 0 hook at 0x%08X.",
					static_cast<DWORD>(srcAddr), static_cast<unsigned>(size - offset),
					originalAddr);

				tailOffset = offset;
				break;
			}

			InstructionInfo info {};
			info.srcOffset = offset;
			info.srcLength = instruction.length;
			info.outputSize = instruction.length; // default fallback
			info.intraPrologue = false;
			info.targetSrcOffset = 0;

			if (instruction.attributes & ZYDIS_ATTRIB_IS_RELATIVE)
			{
				// Only Jcc, JMP, and CALL have near (rel32) forms.
				// LOOP/LOOPE/LOOPNE/JCXZ/JECXZ/JRCXZ are rel8-only but
				// Zydis classifies them as COND_BR, so we must exclude
				// them by mnemonic.
				auto const cat = instruction.meta.category;
				auto const mn = instruction.mnemonic;
				bool const hasNearForm =
					(cat == ZYDIS_CATEGORY_COND_BR
						|| cat == ZYDIS_CATEGORY_UNCOND_BR
						|| cat == ZYDIS_CATEGORY_CALL)
					&& mn != ZYDIS_MNEMONIC_LOOP
					&& mn != ZYDIS_MNEMONIC_LOOPE
					&& mn != ZYDIS_MNEMONIC_LOOPNE
					&& mn != ZYDIS_MNEMONIC_JCXZ
					&& mn != ZYDIS_MNEMONIC_JECXZ
					&& mn != ZYDIS_MNEMONIC_JRCXZ;

				// Find the immediate operand and resolve its absolute target.
				for (ZyanU8 i = 0; i < instruction.operand_count_visible; ++i)
				{
					if (operands[i].type == ZYDIS_OPERAND_TYPE_IMMEDIATE)
					{
						ZyanU64 absAddr;
						if (ZYAN_SUCCESS(ZydisCalcAbsoluteAddress(
							&instruction, &operands[i], srcAddr, &absAddr)))
						{
							// Check if target falls within the prologue.
							if (absAddr >= originalAddr
								&& absAddr < originalAddr + size)
							{
								if (hasNearForm)
								{
									info.intraPrologue = true;
									info.targetSrcOffset =
										static_cast<size_t>(absAddr - originalAddr);
								}
								else
								{
									Debug::Log(
										__FUNCTION__ ": Relative instruction "
										"at 0x%08X has an intra-prologue target "
										"but no near encoding. Hook at 0x%08X "
										"may not work correctly.",
										static_cast<DWORD>(srcAddr),
										originalAddr);
								}
							}
						}
						break;
					}
				}

				// Build and cache the encoder request for pass 2.
				ZydisEncoderRequest req;
				if (!ZYAN_FAILED(ZydisEncoderDecodedInstructionToEncoderRequest(
					&instruction, operands,
					instruction.operand_count_visible, &req)))
				{
					ResolveRelativeOperands(
						req, instruction, operands, srcAddr);

					if (hasNearForm)
					{
						// Force near encoding so output size is deterministic
						// regardless of the final destination address.
						req.branch_type = ZYDIS_BRANCH_TYPE_NEAR;
						req.branch_width = ZYDIS_BRANCH_WIDTH_32;
					}

					info.encoderReq = req;

					if (hasNearForm)
					{
						// 6 bytes for Jcc near (0F 8x rel32), 5 bytes for JMP/CALL (E9/E8 rel32).
						info.outputSize = (instruction.meta.category == ZYDIS_CATEGORY_COND_BR)
							? 6u : 5u;
					}
				}
			}

			info.outputOffset = outOff;
			outOff += info.outputSize;

			infos.push_back(info);
			offset += instruction.length;
		}
	}

	// --- Pass 2: emit relocated instructions ---

	std::vector<uint8_t> result;
	result.reserve(size * 2);

	for (size_t idx = 0; idx < infos.size(); ++idx)
	{
		auto const& info = infos[idx];
		auto const srcAddr = static_cast<ZyanU64>(originalAddr + info.srcOffset);
		auto const dstAddr = static_cast<ZyanU64>(newAddr + result.size());

		if (!info.encoderReq)
		{
			result.insert(result.end(),
				bytes + info.srcOffset,
				bytes + info.srcOffset + info.srcLength);
			continue;
		}

		auto req = *info.encoderReq;

		if (info.intraPrologue)
		{
			// Map the immediate target to its relocated output offset.
			for (ZyanU8 i = 0; i < req.operand_count; ++i)
			{
				// For jumps within the prologue, find the instruction to which
				// the jump is, and substitute its new shifted absolute address
				if (req.operands[i].type == ZYDIS_OPERAND_TYPE_IMMEDIATE)
				{
					for (size_t j = 0; j < infos.size(); ++j)
					{
						if (infos[j].srcOffset == info.targetSrcOffset)
						{
							req.operands[i].imm.u = static_cast<ZyanU64>(
								newAddr + infos[j].outputOffset);
							break;
						}
					}
					break;
				}
			}
		}

		uint8_t encoded[ZYDIS_MAX_INSTRUCTION_LENGTH];
		ZyanUSize encodedLen = sizeof(encoded);

		if (ZYAN_FAILED(ZydisEncoderEncodeInstructionAbsolute(
			&req, encoded, &encodedLen, dstAddr)))
		{
			Debug::Log(
				__FUNCTION__ ": Failed to re-encode instruction at 0x%08X, "
				"copying %u bytes verbatim. This could mean there is a "
				"faulty return 0 hook at 0x%08X.",
				static_cast<DWORD>(srcAddr), info.srcLength,
				originalAddr);

			result.insert(result.end(),
				bytes + info.srcOffset,
				bytes + info.srcOffset + info.srcLength);
		}
		else
		{
			result.insert(result.end(), encoded, encoded + encodedLen);
		}
	}

	// Append any undecoded tail bytes verbatim.
	if (tailOffset < size)
		result.insert(result.end(), bytes + tailOffset, bytes + size);

	return result;
}

void PhobosHookers::InitMinHook()
{
	MH_Initialize();
}

void PhobosHookers::CleanupTrampolines()
{
	for (auto& pair : g_trampolines)
	{
		if (pair.second.trampoline_address)
		{
			VirtualFree(pair.second.trampoline_address, 0, MEM_RELEASE);
		}
	}

	g_trampolines.clear();
	MH_Uninitialize();
	Debug::Log("All trampolines cleaned up\n");
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

bool PhobosHookers::InstallSingleHook(unsigned int addr, std::map<const void*, size_t>& sm_vec0)
{
	auto hookTo = sm_vec0.begin();

	if (hookTo->second < 5u)
		Debug::Log("Hook at 0x%x size is %d less than 5 bytes(JMP).\n", hookTo->second, addr);

	size_t hookSize = MaxImpl(hookTo->second, 5u);

	if (!SetupTrampoline(addr, hookSize)) {
		Debug::Log("Failed to setup trampoline for hook at 0x%x, skipping!\n", addr);
		return false;
	}

	FunctionTrampoline& trampoline = g_trampolines[addr];

	asmjit::CodeHolder code {};
		code.init(gJitRuntime->environment(), gJitRuntime->cpu_features());
		code.set_error_handler(&gJitErrorHandler);

		gJitErrorHandler.currentHookAddr = addr;
		gJitErrorHandler.hadError = false;
	asmjit::x86::Assembler assembly(&code);
	asmjit::Label l_origin = assembly.new_label();

	if (l_origin.id() == asmjit::Globals::kInvalidId)
	{
		Debug::Log("Failed to allocate label for hook at 0x%x (newLabel returned invalid id)\n", addr);
		return false;
	}
	// Original non-recursive version
	assembly.pushad();
	assembly.pushfd();
	assembly.push(addr);
	assembly.sub(asmjit::x86::esp, 4);
	assembly.lea(asmjit::x86::eax, asmjit::x86::ptr(asmjit::x86::esp, 4));
	assembly.push(asmjit::x86::eax);
	assembly.call(hookTo->first);
	assembly.add(asmjit::x86::esp, 0xC);
	assembly.mov(asmjit::x86::ptr(asmjit::x86::esp, -8), asmjit::x86::eax);
	assembly.popfd();

	// POPAD replica
	assembly.popad();

	assembly.cmp(asmjit::x86::dword_ptr(asmjit::x86::esp, -0x2C), 0);
	assembly.jz(l_origin);
	assembly.jmp(asmjit::x86::ptr(asmjit::x86::esp, -0x2C));

	assembly.bind(l_origin);
	void* hookAddress = reinterpret_cast<void*>(addr);

	// Recursive version: use bytes from trampoline backup
	trampoline.modified_original_bytes.resize(hookSize);
	memcpy(trampoline.modified_original_bytes.data(), trampoline.original_bytes.data(), hookSize);

	// Fix relative jump or call
	if (trampoline.modified_original_bytes[0] == Assembly::CALL || trampoline.modified_original_bytes[0] == Assembly::JMP) {
		DWORD dest = addr + 5 + *reinterpret_cast<DWORD*>(trampoline.modified_original_bytes.data() + 1);
		switch (trampoline.modified_original_bytes[0])
		{
		case Assembly::JMP: // jmp
			assembly.jmp(dest);
			trampoline.modified_original_bytes.erase(trampoline.modified_original_bytes.begin(), trampoline.modified_original_bytes.begin() + 5);
			Debug::Log("hook at 0x%x is placed at JMP fixing the relative addr !\n", addr);
			break;
		case Assembly::CALL: // call
			assembly.call(dest);
			trampoline.modified_original_bytes.erase(trampoline.modified_original_bytes.begin(), trampoline.modified_original_bytes.begin() + 5);
			Debug::Log("hook at 0x%x is placed at CALL fixing the relative addr !\n", addr);
			break;
		default: break;
		}
	}

	assembly.embed(trampoline.modified_original_bytes.data(), trampoline.modified_original_bytes.size());
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
	FlushInstructionCache(Game::hInstance(), hookAddress, hookSize);

	Debug::Log("Hook installed at 0x%x (size: %d bytes)\n", addr, hookSize);
	
	return true;
}

bool PhobosHookers::SetupTrampoline(unsigned int target_address, size_t hook_size)
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

	if (!trampoline.trampoline_address) {
		Debug::Log("Failed to allocate trampoline for hook at 0x%x (GetLastError=%u)\n",
			target_address, GetLastError());
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

	Debug::Log("Trampoline created for hook at 0x%x -> 0x%p\n",
		target_address, trampoline.trampoline_address);

	return true;
}

void PhobosHookers::ApplyasmjitPatch()
{
	std::vector<unsigned int> failedHooks;

	for (auto& [addr, data] : Hooks) {

		if (data.empty()) {
			Debug::Log("hook at 0x%x is empty !\n", addr);
			continue;
		}

		if (data.size() > 1) {
			Debug::Log("hook at 0x%x , has %d functions registered ! only the first will be installed.\n",
				addr, data.size());

			int i = 0;
			for (auto& dd_ : data) {
				Debug::Log("hook at 0x%x , %d has size of %d.\n",
					addr, ++i,dd_.second);

			}
		}

		if (!InstallSingleHook(addr, data)) {
			failedHooks.push_back(addr);
		}
	}

	if (!failedHooks.empty()) {
		Debug::Log("\n==== asmjit hook installation summary ====\n");
		Debug::Log("%d hook(s) FAILED to install:\n", static_cast<int>(failedHooks.size()));
		for (auto a : failedHooks) {
			Debug::Log("    0x%x\n", a);
		}
		Debug::Log("All other hooks installed successfully.\n");
		Debug::Log("===========================================\n\n");
	} else {
		Debug::Log("All %d asmjit hook(s) installed successfully.\n", static_cast<int>(Hooks.size()));
	}
}

void PhobosHookers::Initasmjit()
{
	gJitRuntime = std::make_unique<asmjit::JitRuntime>();

	void* buffer {};
	int len = Patch::GetSection(Phobos::hInstance, ASMJIT_PATCH_SECTION_NAME, &buffer);

	hookdeclfunc* end = (hookdeclfunc*)((DWORD)buffer + len);
	Debug::Log("Applying %d asmjit hooks.\n", std::distance((hookdeclfunc*)buffer, end));

	for (hookdeclfunc* begin = (hookdeclfunc*)buffer; begin < end; begin++) {
		Hooks[begin->hookAddr][begin->hookFunc] = begin->hookSize;
	}

	ApplyasmjitPatch();
}
