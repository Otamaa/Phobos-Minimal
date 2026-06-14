#include "Phobos.Hookers.h"

#include <Utilities/Debug.h>

#include <minhook/MinHook.h>
#include <Zydis/Zydis.h>

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
std::map<unsigned int, HooksData> PhobosHookers::Hooks;

namespace AsmjitRebuild
{
	// Resolve relative operands in an encoder request to absolute addresses,
	// using Zydis's own calc-absolute helper against the instruction's
	// original runtime address (srcAddr).
	inline void ResolveRelativeOperands(
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

	// bytes:        pointer to the original overwritten bytes
	// size:         number of overwritten bytes (org_vec.size() after any
	//               CALL/JMP-rel32-at-offset-0 special case has been removed
	//               -- or pass the WHOLE org_vec and remove that special case
	//               entirely, see migration notes below)
	// originalAddr: the address these bytes were read from (hook addr)
	// newAddr:      the address these bytes will be placed at in the
	//               trampoline (base + p_code offset)
	//
	// Returns the relocated byte sequence to embed in place of `bytes`.
	inline std::vector<uint8_t> RebuildInstructions(
		uint8_t const* bytes, size_t size, uint32_t originalAddr, uint32_t newAddr)
	{
		ZydisDecoder decoder;
		ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_COMPAT_32, ZYDIS_STACK_WIDTH_32);

		struct InstructionInfo
		{
			size_t srcOffset;       // offset into original bytes
			ZyanU8 srcLength;       // original instruction length
			bool intraPrologue;     // relative branch targets within this block
			size_t targetSrcOffset; // source offset of branch target (intra-prologue only)
			size_t outputSize;      // size in the output buffer
			size_t outputOffset;    // offset within the output buffer
			std::optional<ZydisEncoderRequest> encoderReq; // cached request (relative instrs only)
		};

		std::vector<InstructionInfo> infos;
		size_t tailOffset = size; // offset of undecoded tail, if any

		// ---- Pass 1: decode + classify ----
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
						"RebuildInstructions: Failed to decode instruction at 0x%08X, "
						"copying remaining %u bytes verbatim. Hook at 0x%08X may have "
						"a malformed overwritten-bytes region.\n",
						static_cast<unsigned int>(srcAddr),
						static_cast<unsigned int>(size - offset),
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
					// Zydis classifies them as COND_BR, so exclude by mnemonic.
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

					// Find immediate operand and resolve absolute target.
					for (ZyanU8 i = 0; i < instruction.operand_count_visible; ++i)
					{
						if (operands[i].type == ZYDIS_OPERAND_TYPE_IMMEDIATE)
						{
							ZyanU64 absAddr;
							if (ZYAN_SUCCESS(ZydisCalcAbsoluteAddress(
								&instruction, &operands[i], srcAddr, &absAddr)))
							{
								// Check if target falls within this block.
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
											"RebuildInstructions: Relative instruction "
											"at 0x%08X has an intra-block target but no "
											"near encoding (rel8-only mnemonic). Hook at "
											"0x%08X may not work correctly.\n",
											static_cast<unsigned int>(srcAddr),
											originalAddr);
									}
								}
							}
							break;
						}
					}

					// Build + cache encoder request for pass 2.
					ZydisEncoderRequest req;
					if (!ZYAN_FAILED(ZydisEncoderDecodedInstructionToEncoderRequest(
						&instruction, operands,
						instruction.operand_count_visible, &req)))
					{
						ResolveRelativeOperands(req, instruction, operands, srcAddr);

						if (hasNearForm)
						{
							// Force near encoding so output size is deterministic.
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

		// ---- Pass 2: emit relocated instructions ----
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
					"RebuildInstructions: Failed to re-encode instruction at 0x%08X, "
					"copying %u bytes verbatim. Hook at 0x%08X may not work correctly.\n",
					static_cast<unsigned int>(srcAddr), info.srcLength, originalAddr);

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

	if (!trampoline.trampoline_address)
	{
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
		Debug::Log("Hook %x seems to be conflicted with other hooks! disassembly: %s \n", (void*)hookAddress, disassemblyResult.c_str());
	}
	if (beforeAddress)
	{
		Debug::Log("Hook %x seems to be conflicted with other hooks! see assembly before address\n", (void*)hookAddress);
	}
}

// Emits: 64 A3 14 00 00 00  ->  MOV FS:[0x14], EAX  (moffs32 form, fixed encoding)
inline void EmitMovFs14Eax(asmjit::x86::Assembler& assembly)
{
	assembly.db(0x64); // FS segment prefix
	assembly.db(0xA3); // MOV moffs32, EAX
	assembly.dd(0x14); // displacement (4 bytes, written as imm32)
}

// Emits: 64 83 3D 14 00 00 00 00  ->  CMP DWORD PTR FS:[0x14], 0
inline void EmitCmpFs14Zero(asmjit::x86::Assembler& assembly)
{
	assembly.db(0x64); // FS segment prefix
	assembly.db(0x83); // CMP r/m32, imm8 (opcode group)
	assembly.db(0x3D); // ModRM: mod=00, reg=111(/7 = CMP), rm=101 (disp32, no base)
	assembly.dd(0x14); // disp32 = 0x14
	assembly.db(0x00); // imm8 = 0
}

// Emits: 64 FF 25 14 00 00 00  ->  JMP DWORD PTR FS:[0x14]
inline void EmitJmpFs14Indirect(asmjit::x86::Assembler& assembly)
{
	assembly.db(0x64); // FS segment prefix
	assembly.db(0xFF); // opcode group FF
	assembly.db(0x25); // ModRM: mod=00, reg=100(/4 = JMP), rm=101 (disp32, no base)
	assembly.dd(0x14); // disp32 = 0x14
}

// Emits PUSHFD + POPAD-replica restoring all 7 GP regs in PUSHAD-reversal
// order, with EAX/ESP slot-swap so ESP is corrected without corrupting
// the stack before all POPs complete.
inline void EmitPopfdPopadReplica(asmjit::x86::Assembler& assembly)
{
	assembly.popfd();
	assembly.pop(asmjit::x86::edi);
	assembly.pop(asmjit::x86::esi);
	assembly.pop(asmjit::x86::ebp);
	assembly.pop(asmjit::x86::ebx);   // EBX = temp holder for new ESP

	// MOV EAX, [ESP + 0xC]  (restore EAX, last in PUSHAD order)
	assembly.mov(asmjit::x86::eax, asmjit::x86::ptr(asmjit::x86::esp, 0xC));
	// MOV [ESP + 0xC], EBX  (place new ESP value there)
	assembly.mov(asmjit::x86::ptr(asmjit::x86::esp, 0xC), asmjit::x86::ebx);

	assembly.pop(asmjit::x86::ebx);
	assembly.pop(asmjit::x86::edx);
	assembly.pop(asmjit::x86::ecx);
	assembly.pop(asmjit::x86::esp);   // restore ESP last
}

// Full dispatcher trampoline:
//   PUSHAD; PUSHFD
//   PUSH HookAddress
//   PUSH ESP                ; REGISTERS*
//   CALL ProcAddress
//   ADD ESP, 8
//   MOV  FS:[0x14], EAX
//   CMP  DWORD PTR FS:[0x14], 0
//   JE   proceed
//   <popfd/popad replica>
//   JMP  DWORD PTR FS:[0x14]
//   <falls through here if JE taken -- caller binds `proceed` next>
//
// NOTE: does NOT bind `proceed`. Caller binds it exactly once, then emits
// EmitPopfdPopadReplica() again + the overwritten bytes + jmp back.
inline void EmitDispatchTrampoline(
	asmjit::x86::Assembler& assembly,
	asmjit::Label proceed,
	const asmjit::Imm& hookAddress,
	const asmjit::Imm& procAddress)
{
	assembly.pushad();
	assembly.pushfd();

	assembly.push(hookAddress);
	assembly.push(asmjit::x86::esp);   // REGISTERS* (final arg)

	assembly.call(procAddress);

	assembly.add(asmjit::x86::esp, 8);

	EmitMovFs14Eax(assembly);     // MOV FS:[0x14], EAX
	EmitCmpFs14Zero(assembly);    // CMP DWORD PTR FS:[0x14], 0
	assembly.je(proceed);

	// jmp_to_address:
	EmitPopfdPopadReplica(assembly);
	EmitJmpFs14Indirect(assembly); // JMP DWORD PTR FS:[0x14]

	// fall-through (JE taken) lands here -- caller binds `proceed`.
}

bool PhobosHookers::InstallSingleHook(unsigned int addr, const HookEntry& sm_vec0, std::vector<uint8_t>& org_vec)
{
	CheckHookConflict(addr, sm_vec0.size);

	size_t hook_size = sm_vec0.size;
	DWORD hookSize = MaxImpl(hook_size, 5u);

	// ---- Setup trampoline BEFORE creating hook code ----
	if (!SetupTrampoline(addr, hookSize))
	{
		Debug::Log("Failed to setup trampoline for hook at 0x%x, skipping!\n", addr);
		return false;
	}

	FunctionTrampoline& trampoline = g_trampolines[addr];

	asmjit::CodeHolder code;
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

	// ---- Dispatcher trampoline (PUSHAD/PUSHFD .. CALL .. FS:[0x14] check) ----
	EmitDispatchTrampoline(
		assembly,
		l_origin,
		asmjit::imm(static_cast<uint32_t>(addr)),
		asmjit::imm(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(sm_vec0.func))));

	if (gJitErrorHandler.hadError)
	{
		Debug::Log("Aborting hook 0x%x: error during dispatcher emission\n", addr);
		return false;
	}

	// ---- proceed: ---- (bound exactly ONCE)
	assembly.bind(l_origin);
	EmitPopfdPopadReplica(assembly);

	if (gJitErrorHandler.hadError)
	{
		Debug::Log("Aborting hook 0x%x: error during proceed-path emission\n", addr);
		return false;
	}

	// ---- copy overwritten original bytes from trampoline backup ----
	org_vec.resize(hookSize);
	memcpy(org_vec.data(), trampoline.original_bytes.data(), hookSize);

	// Fix relative jump or call if the first overwritten instruction is
	// CALL/JMP rel32 (its target must be recomputed for the new location).
	//if (org_vec[0] == Assembly::CALL || org_vec[0] == Assembly::JMP)
	//{
	//	DWORD dest = addr + 5 + *reinterpret_cast<DWORD*>(org_vec.data() + 1);
	//	switch (org_vec[0])
	//	{
	//	case Assembly::JMP:
	//		assembly.jmp(dest);
	//		org_vec.erase(org_vec.begin(), org_vec.begin() + 5);
	//		Debug::Log("hook at 0x%x is placed at JMP fixing the relative addr !\n", addr);
	//		break;
	//	case Assembly::CALL:
	//		assembly.call(dest);
	//		org_vec.erase(org_vec.begin(), org_vec.begin() + 5);
	//		Debug::Log("hook at 0x%x is placed at CALL fixing the relative addr !\n", addr);
	//		break;
	//	}
	//
	//	if (gJitErrorHandler.hadError)
	//	{
	//		Debug::Log("Aborting hook 0x%x: error during CALL/JMP rel32 fixup\n", addr);
	//		return false;
	//	}
	//}

	  // ---- rebuild overwritten bytes, fixing any relative branches ----
	{
		// newAddr = where these bytes will live once relocated to `addr`.
		// assembly.offset() returns the current write position within the
		// code buffer's .text section (bytes emitted so far).
		uint32_t const newAddr = addr + static_cast<uint32_t>(assembly.offset());

		auto rebuilt = AsmjitRebuild::RebuildInstructions(
			org_vec.data(), org_vec.size(), addr, newAddr);

		if (rebuilt.size() != org_vec.size())
		{
			Debug::Log(
				"hook at 0x%x: overwritten bytes rebuilt from %u to %u bytes "
				"(relative branch re-encoded to near form)\n",
				addr, static_cast<unsigned int>(org_vec.size()),
				static_cast<unsigned int>(rebuilt.size()));
		}

		org_vec = std::move(rebuilt);
	}

	assembly.embed(org_vec.data(), org_vec.size());
	assembly.jmp(addr + hookSize);

	if (gJitErrorHandler.hadError)
	{
		Debug::Log("Aborting hook 0x%x: error during overwritten-bytes/jmp-back emission\n", addr);
		return false;
	}

	// ---- finalize: add to runtime, then relocate a second pass to addr ----
	const void* fn {};
	asmjit::Error addErr = gJitRuntime->add(&fn, &code);
	if (addErr != asmjit::kErrorOk)
	{
		Debug::Log("AsmJit ERROR %u (%s) while adding hook at 0x%x to JitRuntime\n",
			static_cast<unsigned int>(addErr), asmjit::DebugUtils::error_as_string(addErr), addr);
		return false;
	}

	code.reset();
	code.init(gJitRuntime->environment(), gJitRuntime->cpu_features());
	code.set_error_handler(&gJitErrorHandler);
	code.attach(&assembly);

	assembly.jmp(fn);

	if (gJitErrorHandler.hadError)
	{
		Debug::Log("Aborting hook 0x%x: error during final jmp(fn) emission\n", addr);
		return false;
	}

	code.flatten();
	code.resolve_cross_section_fixups();

	asmjit::Error relocErr = code.relocate_to_base(addr);
	if (relocErr != asmjit::kErrorOk)
	{
		Debug::Log("AsmJit ERROR %u (%s) while relocating hook at 0x%x to base\n",
			static_cast<unsigned int>(relocErr), asmjit::DebugUtils::error_as_string(relocErr), addr);
		return false;
	}

	// ---- write into the target process image ----
	void* hookAddress = reinterpret_cast<void*>(addr);

	DWORD protect_flag {};
	DWORD protect_flagb {};

	if (!VirtualProtect(hookAddress, hookSize, PAGE_EXECUTE_READWRITE, &protect_flag))
	{
		Debug::Log("VirtualProtect (unprotect) failed for hook at 0x%x (GetLastError=%u)\n", addr, GetLastError());
		return false;
	}

	code.copy_flattened_data(hookAddress, hookSize);

	if (!VirtualProtect(hookAddress, hookSize, protect_flag, &protect_flagb))
	{
		Debug::Log("VirtualProtect (restore) failed for hook at 0x%x (GetLastError=%u)\n", addr, GetLastError());
		// Memory was already written; not fatal to the hook itself, but flag it.
	}

	FlushInstructionCache(Game::hInstance(), hookAddress, hookSize);

	Debug::Log("Hook installed at 0x%x (size: %d bytes)\n", addr, hookSize);
	return true;
}

void PhobosHookers::ApplyasmjitPatch()
{
	std::vector<unsigned int> failedHooks;

	for (auto& [addr, data] : Hooks) {
		auto& [sm_vec, org_vec] = data;

		if (sm_vec.empty()) {
			Debug::Log("hook at 0x%x is empty !\n", addr);
			continue;
		}

		if (sm_vec.size() > 1) {
			Debug::Log("hook at 0x%x , has %d functions registered ! only the first will be installed.\n",
				addr, sm_vec.size());
		}

		if (!InstallSingleHook(addr, sm_vec[0], org_vec)) {
			failedHooks.push_back(addr);
		}

		if (sm_vec.size() > 1) {
			Debug::Log("remaining hook function(s) at 0x%x ignored (multi-hook chaining not implemented).\n", addr);
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
		Hooks[begin->hookAddr].summary.emplace_back(begin->hookFunc, begin->hookSize);
	}

	ApplyasmjitPatch();
}
