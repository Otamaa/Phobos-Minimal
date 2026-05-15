/*
	SYRINGE.H - FIXED VERSION
	-------------------------
	Fixed version with bug corrections and safety enhancements.

	CRITICAL FIXES:
	1. Float bit-pattern preservation in Set<T>()
	2. Signed offset arithmetic in lea() and At()
	3. Type safety bounds checking
	4. Float support in Push/Pop
	5. Enhanced debug validation

	BACKWARD COMPATIBILITY:
	- Define SYRINGE_LEGACY_BEHAVIOR to preserve old (buggy) float conversion
	- Define SYRINGE_NO_SAFETY_CHECKS to disable runtime validation
	- All fixes are ABI-compatible with existing code

	REQUIREMENTS:
	- C++20 or later (for std::bit_cast)
	- Enable intrinsics and optimization

	-pd (fixed by otamaa)
*/

#pragma once

#include <Base/Always.h>
#include <DebugLog.h>
#include <type_traits>
#include <bit>
#include <cstdint>
#include <cstring>
#include <xmmintrin.h>  // SSE intrinsics
#include <emmintrin.h>  // SSE2 intrinsics

// ============================================================================
// CONFIGURATION
// ============================================================================

// Uncomment to preserve legacy float conversion behavior (NOT RECOMMENDED)
// #define SYRINGE_LEGACY_BEHAVIOR

// Uncomment to disable runtime safety checks in debug builds
// #define SYRINGE_NO_SAFETY_CHECKS

// Enable aggressive inlining for performance
#ifndef SYRINGE_FORCE_INLINE
#if defined(_MSC_VER)
#define SYRINGE_FORCE_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
#define SYRINGE_FORCE_INLINE __attribute__((always_inline)) inline
#else
#define SYRINGE_FORCE_INLINE inline
#endif
#endif

// ============================================================================
// SAFETY VALIDATION (Debug builds only, zero runtime cost in Release)
// ============================================================================

// x86 EFLAGS register proxy for readable conditional checks
// Usage: auto eflags = R->EFLAGS(); if (eflags.ZF) { ... }

struct EFlagsProxy
{
	DWORD raw;

	// Individual flag accessors (bit positions)
	constexpr bool CF() const noexcept { return (raw & (1 << 0)) != 0; }  // Carry
	constexpr bool PF() const noexcept { return (raw & (1 << 2)) != 0; }  // Parity
	constexpr bool AF() const noexcept { return (raw & (1 << 4)) != 0; }  // Auxiliary Carry
	constexpr bool ZF() const noexcept { return (raw & (1 << 6)) != 0; }  // Zero
	constexpr bool SF() const noexcept { return (raw & (1 << 7)) != 0; }  // Sign
	constexpr bool TF() const noexcept { return (raw & (1 << 8)) != 0; }  // Trap
	constexpr bool IF() const noexcept { return (raw & (1 << 9)) != 0; }  // Interrupt Enable
	constexpr bool DF() const noexcept { return (raw & (1 << 10)) != 0; } // Direction
	constexpr bool OF() const noexcept { return (raw & (1 << 11)) != 0; } // Overflow

	// Common condition checks (matches x86 Jcc mnemonics)
	constexpr bool IsZero() const noexcept { return ZF(); }                          // JZ/JE
	constexpr bool IsNotZero() const noexcept { return !ZF(); }                      // JNZ/JNE
	constexpr bool IsCarry() const noexcept { return CF(); }                         // JC/JB/JNAE
	constexpr bool IsNotCarry() const noexcept { return !CF(); }                     // JNC/JAE/JNB
	constexpr bool IsSign() const noexcept { return SF(); }                          // JS
	constexpr bool IsNotSign() const noexcept { return !SF(); }                      // JNS
	constexpr bool IsOverflow() const noexcept { return OF(); }                      // JO
	constexpr bool IsNotOverflow() const noexcept { return !OF(); }                  // JNO
	constexpr bool IsBelow() const noexcept { return CF(); }                         // JB (unsigned <)
	constexpr bool IsAboveOrEqual() const noexcept { return !CF(); }                 // JAE (unsigned >=)
	constexpr bool IsLessOrEqual() const noexcept { return ZF() || (SF() != OF()); } // JLE (signed <=)
	constexpr bool IsGreater() const noexcept { return !ZF() && (SF() == OF()); }    // JG (signed >)
	constexpr bool IsLess() const noexcept { return SF() != OF(); }                  // JL (signed <)
	constexpr bool IsGreaterOrEqual() const noexcept { return SF() == OF(); }        // JGE (signed >=)
	constexpr bool IsAbove() const noexcept { return !CF() && !ZF(); }               // JA (unsigned >)
	constexpr bool IsBelowOrEqual() const noexcept { return CF() || ZF(); }          // JBE (unsigned <=)

	// Implicit conversion back to DWORD for compatibility
	constexpr operator DWORD() const noexcept { return raw; }
};

namespace SyringeInternal
{
#if defined(_DEBUG) && !defined(SYRINGE_NO_SAFETY_CHECKS)

	// Validate pointer is in reasonable memory range
	template<typename T>
	SYRINGE_FORCE_INLINE void ValidatePointer([[maybe_unused]] T* ptr)
	{
		const auto addr = reinterpret_cast<std::uintptr_t>(ptr);

		// Check for null
		if (addr == 0)
		{
			Debug::Log("[SYRINGE] WARNING: Null pointer access\n");
			return;
		}

		// Check for obviously invalid addresses (first 64KB, kernel space on x86)
		if (addr < 0x00010000)
		{
			Debug::Log("[SYRINGE] ERROR: Invalid low address: 0x%08X\n",
				static_cast<DWORD>(addr));
		}

		if (addr >= 0x80000000 && addr < 0x90000000)
		{
			// Might be valid kernel/driver space, just warn
			Debug::Log("[SYRINGE] WARNING: Kernel space address: 0x%08X\n",
				static_cast<DWORD>(addr));
		}
	}

	// Validate stack pointer is in reasonable range
	SYRINGE_FORCE_INLINE void ValidateStackPointer([[maybe_unused]] DWORD esp)
	{
		// Typical user-mode stack is 0x00010000 - 0x7FFFFFFF
		if (esp < 0x00010000 || esp > 0x7FFFFFFF)
		{
			Debug::Log("[SYRINGE] WARNING: Suspicious ESP value: 0x%08X\n", esp);
		}

		// Check alignment (stack should be 4-byte aligned minimum)
		if ((esp & 0x3) != 0)
		{
			Debug::Log("[SYRINGE] WARNING: Misaligned ESP: 0x%08X\n", esp);
		}
	}

	// Validate offset won't cause integer overflow
	SYRINGE_FORCE_INLINE void ValidateOffset([[maybe_unused]] DWORD base,
											  [[maybe_unused]] int offset)
	{
		const auto base_signed = static_cast<std::intptr_t>(base);
		const auto result = base_signed + offset;

		// Check for wrap-around
		if (offset > 0 && result < base_signed)
		{
			Debug::Log("[SYRINGE] ERROR: Positive offset overflow: base=0x%08X offset=%d\n",
				base, offset);
		}
		if (offset < 0 && result > base_signed)
		{
			Debug::Log("[SYRINGE] ERROR: Negative offset underflow: base=0x%08X offset=%d\n",
				base, offset);
		}
	}

#else
	// Release builds: no-op (optimized away completely)
	template<typename T>
	SYRINGE_FORCE_INLINE constexpr void ValidatePointer([[maybe_unused]] T* ptr) noexcept {}

	SYRINGE_FORCE_INLINE constexpr void ValidateStackPointer([[maybe_unused]] DWORD esp) noexcept {}

	SYRINGE_FORCE_INLINE constexpr void ValidateOffset([[maybe_unused]] DWORD base,
														 [[maybe_unused]] int offset) noexcept {}
#endif

} // namespace SyringeInternal

// ============================================================================
// REGISTER CLASSES
// ============================================================================

class LimitedRegister
{
protected:
	DWORD data;

	SYRINGE_FORCE_INLINE WORD* wordData() noexcept
	{
		return reinterpret_cast<WORD*>(&this->data);
	}

	SYRINGE_FORCE_INLINE BYTE* byteData() noexcept
	{
		return reinterpret_cast<BYTE*>(&this->data);
	}

public:
	SYRINGE_FORCE_INLINE WORD Get16() const noexcept
	{
		return *reinterpret_cast<const WORD*>(&this->data);
	}

	// ========================================================================
	// FIXED: Type-safe Get() with bounds checking
	// ========================================================================
	template<typename T>
	SYRINGE_FORCE_INLINE T Get() const noexcept
	{
		using BaseType = std::remove_cv_t<T>;

		static_assert(sizeof(BaseType) <= sizeof(DWORD),
			"SYRINGE: Cannot Get() a type larger than DWORD (4 bytes). "
			"Attempted to read type of size greater than register width.");

		static_assert(std::is_trivially_copyable_v<BaseType>,
			"SYRINGE: Get() requires trivially copyable type. "
			"Use custom serialization for complex types.");

		// Use memcpy for strict aliasing compliance (compiler optimizes to direct read)
		BaseType result;
		std::memcpy(&result, &this->data, sizeof(BaseType));
		return result;
	}

	// ========================================================================
	// FIXED: Proper bit-pattern preservation for floats, pointers, integrals
	// ========================================================================
	template<typename T>
	SYRINGE_FORCE_INLINE void Set(T value) noexcept
	{
		static_assert(sizeof(T) <= sizeof(DWORD),
			"SYRINGE: Cannot Set() a type larger than DWORD (4 bytes). "
			"Type size exceeds register width.");

		static_assert(std::is_trivially_copyable_v<T>,
			"SYRINGE: Set() requires trivially copyable type.");

		// Type-specific handling for correct semantics
		if constexpr (std::is_floating_point_v<T>)
		{
#ifdef SYRINGE_LEGACY_BEHAVIOR
			// OLD BUGGY BEHAVIOR: Numeric conversion (3.14f -> 3)
			// Only enabled if explicitly requested for backward compat
			this->data = static_cast<DWORD>(value);
#else
			// CORRECT: Preserve bit pattern (0x40490FDB stays 0x40490FDB)
			// Convert to float first to handle double->float narrowing correctly
			const float float_value = static_cast<float>(value);
			this->data = std::bit_cast<DWORD>(float_value);
#endif
		}
		else if constexpr (std::is_pointer_v<T>)
		{
			// Pointers: reinterpret address as DWORD
			this->data = static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(value));
		}
		else if constexpr (std::is_enum_v<T> || std::is_integral_v<T>)
		{
			// Enums and integers: numeric conversion is safe and expected
			this->data = static_cast<DWORD>(value);
		}
		else
		{
			// Other trivially-copyable types: preserve bit pattern
			static_assert(sizeof(T) == sizeof(DWORD),
				"SYRINGE: Non-standard types must be exactly DWORD-sized (4 bytes).");
			this->data = std::bit_cast<DWORD>(value);
		}
	}

	// Modifies only low 16 bits; upper 16 bits unchanged (x86 partial register write)
	SYRINGE_FORCE_INLINE void Set16(WORD value) noexcept
	{
		*this->wordData() = value;
	}
};

class ExtendedRegister : public LimitedRegister
{
public:
	// High byte of low word (bits 8-15)
	SYRINGE_FORCE_INLINE BYTE Get8Hi() const noexcept
	{
		return reinterpret_cast<const BYTE*>(&this->data)[1];
	}

	// Low byte of low word (bits 0-7)
	SYRINGE_FORCE_INLINE BYTE Get8Lo() const noexcept
	{
		return reinterpret_cast<const BYTE*>(&this->data)[0];
	}

	// Modifies only bits 8-15; other bits unchanged
	SYRINGE_FORCE_INLINE void Set8Hi(BYTE value) noexcept
	{
		this->byteData()[1] = value;
	}

	// Modifies only bits 0-7; other bits unchanged
	SYRINGE_FORCE_INLINE void Set8Lo(BYTE value) noexcept
	{
		this->byteData()[0] = value;
	}
};

class StackRegister : public ExtendedRegister
{
public:
	// ========================================================================
	// FIXED: Proper signed offset arithmetic (handles negative stack offsets)
	// ========================================================================

	// Load Effective Address: calculate pointer without dereferencing
	template<typename T>
	SYRINGE_FORCE_INLINE T* lea(int byteOffset) const noexcept
	{
		SyringeInternal::ValidateOffset(this->data, byteOffset);

		// Use signed arithmetic to preserve negative offsets correctly
		// OLD BUG: static_cast<DWORD>(byteOffset) made -8 become 0xFFFFFFF8
		// NEW FIX: Signed addition, then convert to pointer
		const auto address = static_cast<std::intptr_t>(this->data) + byteOffset;
		const auto ptr = reinterpret_cast<T*>(static_cast<std::uintptr_t>(address));

		SyringeInternal::ValidatePointer(ptr);
		return ptr;
	}

	// Non-template overload for DWORD addresses
	SYRINGE_FORCE_INLINE DWORD lea(int byteOffset) const noexcept
	{
		SyringeInternal::ValidateOffset(this->data, byteOffset);

		const auto address = static_cast<std::intptr_t>(this->data) + byteOffset;
		return static_cast<DWORD>(address);
	}

	// Read value at offset (dereference + read)
	template<typename T>
	SYRINGE_FORCE_INLINE T At(int byteOffset) const
	{
		using BaseType = std::remove_cv_t<T>;

		static_assert(std::is_trivially_copyable_v<BaseType>,
			"SYRINGE: At() requires trivially copyable type.");

		SyringeInternal::ValidateOffset(this->data, byteOffset);

		const auto address = static_cast<std::intptr_t>(this->data) + byteOffset;
		const auto ptr = reinterpret_cast<const BaseType*>(static_cast<std::uintptr_t>(address));

		SyringeInternal::ValidatePointer(ptr);

		// For read: use memcpy for strict aliasing safety (optimizes to direct read)
		BaseType result;
		std::memcpy(&result, ptr, sizeof(BaseType));
		return result;
	}

	// Write value at offset (dereference + write)
	template<typename T>
	SYRINGE_FORCE_INLINE void At(int byteOffset, T value)
	{
		using BaseType = std::remove_cv_t<T>;

		static_assert(std::is_trivially_copyable_v<BaseType>,
			"SYRINGE: At() requires trivially copyable type.");

		SyringeInternal::ValidateOffset(this->data, byteOffset);

		const auto address = static_cast<std::intptr_t>(this->data) + byteOffset;
		auto ptr = reinterpret_cast<BaseType*>(static_cast<std::uintptr_t>(address));

		SyringeInternal::ValidatePointer(ptr);

		// For write: direct assignment (compiler optimizes appropriately)
		*ptr = value;
	}
};

// ============================================================================
// REGISTER SHORTCUT MACROS
// ============================================================================

#define REG_SHORTCUTS(reg) \
	SYRINGE_FORCE_INLINE DWORD reg() const noexcept \
		{ return this->_ ## reg.Get<DWORD>(); } \
	template<typename T> SYRINGE_FORCE_INLINE T reg() const noexcept \
		{ return this->_ ## reg.Get<T>(); } \
	template<typename T> SYRINGE_FORCE_INLINE void reg(T value) noexcept \
		{ this->_ ## reg.Set(value); } \

#define REG_SHORTCUTS_X(r) \
	SYRINGE_FORCE_INLINE WORD r ## X() const noexcept \
		{ return this->_E ## r ## X.Get16(); } \
	SYRINGE_FORCE_INLINE void r ## X(WORD value) noexcept \
		{ this->_E ## r ## X.Set16(value); } \

#define REG_SHORTCUTS_HL(r) \
	SYRINGE_FORCE_INLINE BYTE r ## H() const noexcept \
		{ return this->_E ## r ## X.Get8Hi(); } \
	SYRINGE_FORCE_INLINE void r ## H(BYTE value) noexcept \
		{ this->_E ## r ## X.Set8Hi(value); } \
	SYRINGE_FORCE_INLINE BYTE r ## L() const noexcept \
		{ return this->_E ## r ## X.Get8Lo(); } \
	SYRINGE_FORCE_INLINE void r ## L(BYTE value) noexcept \
		{ this->_E ## r ## X.Set8Lo(value); } \

#define REG_SHORTCUTS_XHL(r) \
	REG_SHORTCUTS_X(r); \
	REG_SHORTCUTS_HL(r); \

// ============================================================================
// REGISTER METADATA
// ============================================================================

static constexpr const char* Register_names[] = {
	"EDI", "ESI", "EBP", "ESP", "EBX", "EDX", "ECX", "EAX"
};

enum class RegistersType : int
{
	EDI, ESI, EBP, ESP, EBX, EDX, ECX, EAX
};

#define MAKEREG(RegName, inherit, type) \
struct RegName : public inherit { \
	constexpr SYRINGE_FORCE_INLINE const char* name() const noexcept { \
		return Register_names[static_cast<int>(RegistersType::type)]; \
	} \
	constexpr SYRINGE_FORCE_INLINE RegistersType type() const noexcept { \
		return RegistersType::type; \
	} \
}; \
static_assert(sizeof(RegName) == sizeof(inherit), \
	"SYRINGE: Register wrapper must be same size as base class");

MAKEREG(LI_EDI, LimitedRegister, EDI)
MAKEREG(LI_ESI, LimitedRegister, ESI)
MAKEREG(ST_EBP, StackRegister, EBP)
MAKEREG(ST_ESP, StackRegister, ESP)
MAKEREG(EX_EBX, ExtendedRegister, EBX)
MAKEREG(EX_EDX, ExtendedRegister, EDX)
MAKEREG(EX_ECX, ExtendedRegister, ECX)
MAKEREG(EX_EAX, ExtendedRegister, EAX)

// ============================================================================
// MAIN REGISTERS CLASS
// ============================================================================

class REGISTERS
{
private:
	// ========================================================================
	// TYPE TRAITS FOR PUSH/POP
	// ========================================================================

	template<typename T>
	using StackValueType = std::remove_cv_t<std::remove_reference_t<T>>;

	// FIXED: Allow floating-point types in push/pop (common in x86 calling conventions)
	template<typename T>
	static constexpr bool IsPushPopType =
		std::is_pointer_v<T>
		|| std::is_enum_v<T>
		|| std::is_integral_v<T>
		|| std::is_floating_point_v<T>  // FIXED: Floats are valid on x86 stack
		|| (sizeof(T) == sizeof(DWORD) && std::is_trivially_copyable_v<T>);

	// ========================================================================
	// PUSH/POP CONVERSION HELPERS
	// ========================================================================

	template<typename T>
	static SYRINGE_FORCE_INLINE DWORD PushPopToDWORD(T value) noexcept
	{
		using ValueType = StackValueType<T>;

		static_assert(IsPushPopType<ValueType>,
			"SYRINGE::Push/Pop only support: pointers, enums, integral types, "
			"floating-point types, and trivially-copyable DWORD-sized values.");

		static_assert(sizeof(ValueType) <= sizeof(DWORD),
			"SYRINGE::Push/Pop: Type too large for x86 stack slot (max 4 bytes).");

		if constexpr (std::is_pointer_v<ValueType>)
		{
			return static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(value));
		}
		else if constexpr (std::is_floating_point_v<ValueType>)
		{
			// FIXED: Preserve float bit pattern (not numeric conversion!)
			const float float_value = static_cast<float>(value);
			return std::bit_cast<DWORD>(float_value);
		}
		else if constexpr (std::is_enum_v<ValueType> || std::is_integral_v<ValueType>)
		{
			return static_cast<DWORD>(value);
		}
		else
		{
			// Trivially-copyable DWORD-sized types
			return std::bit_cast<DWORD>(value);
		}
	}

	template<typename T>
	static SYRINGE_FORCE_INLINE T DWORDToPushPop(DWORD value) noexcept
	{
		using ValueType = StackValueType<T>;

		static_assert(IsPushPopType<ValueType>,
			"SYRINGE::Push/Pop only support: pointers, enums, integral types, "
			"floating-point types, and trivially-copyable DWORD-sized values.");

		static_assert(sizeof(ValueType) <= sizeof(DWORD),
			"SYRINGE::Push/Pop: Type too large for x86 stack slot (max 4 bytes).");

		if constexpr (std::is_pointer_v<ValueType>)
		{
			return reinterpret_cast<T>(static_cast<std::uintptr_t>(value));
		}
		else if constexpr (std::is_floating_point_v<ValueType>)
		{
			// FIXED: Restore float bit pattern
			return static_cast<T>(std::bit_cast<float>(value));
		}
		else if constexpr (std::is_enum_v<ValueType> || std::is_integral_v<ValueType>)
		{
			return static_cast<T>(value);
		}
		else
		{
			// Trivially-copyable DWORD-sized types
			return std::bit_cast<ValueType>(value);
		}
	}

	// ========================================================================
	// REGISTER STORAGE
	// ========================================================================

	DWORD origin;  // Original EIP where hook was triggered
	DWORD flags;   // EFLAGS register

	LI_EDI _EDI;
	LI_ESI _ESI;
	ST_EBP _EBP;
	ST_ESP _ESP;
	EX_EBX _EBX;
	EX_EDX _EDX;
	EX_ECX _ECX;
	EX_EAX _EAX;

public:
	// ========================================================================
	// ORIGIN (EIP) AND FLAGS ACCESS
	// ========================================================================

	SYRINGE_FORCE_INLINE DWORD Origin() const noexcept
	{
		return this->origin;
	}

	SYRINGE_FORCE_INLINE void Origin(DWORD value) noexcept
	{
		this->origin = value;
	}

	SYRINGE_FORCE_INLINE DWORD EFLAGS() const noexcept
	{
		return this->flags;
	}

	SYRINGE_FORCE_INLINE EFlagsProxy GetEFLAGS() const noexcept
	{
		return { this->flags };
	}

	SYRINGE_FORCE_INLINE void EFLAGS(DWORD value) noexcept
	{
		this->flags = value;
	}

	// ========================================================================
	// REGISTER ACCESS SHORTCUTS
	// ========================================================================

	REG_SHORTCUTS(EAX);
	REG_SHORTCUTS(EBX);
	REG_SHORTCUTS(ECX);
	REG_SHORTCUTS(EDX);
	REG_SHORTCUTS(ESI);
	REG_SHORTCUTS(EDI);
	REG_SHORTCUTS(ESP);
	REG_SHORTCUTS(EBP);

	REG_SHORTCUTS_XHL(A);  // AX, AH, AL
	REG_SHORTCUTS_XHL(B);  // BX, BH, BL
	REG_SHORTCUTS_XHL(C);  // CX, CH, CL
	REG_SHORTCUTS_XHL(D);  // DX, DH, DL

	// ========================================================================
	// STACK HELPERS
	// ========================================================================

	// Load Effective Address of stack slot
	template<typename T>
	SYRINGE_FORCE_INLINE T lea_Stack(int offset) const noexcept
	{
		return reinterpret_cast<T>(this->_ESP.lea(offset));
	}

	template<>
	SYRINGE_FORCE_INLINE DWORD lea_Stack(int offset) const noexcept
	{
		return this->_ESP.lea(offset);
	}

	template<>
	SYRINGE_FORCE_INLINE int lea_Stack(int offset) const noexcept
	{
		return static_cast<int>(this->_ESP.lea(offset));
	}

	// Get reference to stack value
	template<typename T>
	SYRINGE_FORCE_INLINE T& ref_Stack(int offset) const
	{
		return *this->lea_Stack<T*>(offset);
	}

	// Read stack value
	template<typename T>
	SYRINGE_FORCE_INLINE T Stack(int offset) const
	{
		return this->_ESP.At<T>(offset);
	}

	SYRINGE_FORCE_INLINE DWORD Stack32(int offset) const
	{
		return this->_ESP.At<DWORD>(offset);
	}

	SYRINGE_FORCE_INLINE WORD Stack16(int offset) const
	{
		return this->_ESP.At<WORD>(offset);
	}

	SYRINGE_FORCE_INLINE BYTE Stack8(int offset) const
	{
		return this->_ESP.At<BYTE>(offset);
	}

	// Write stack value
	template<typename T>
	SYRINGE_FORCE_INLINE void Stack(int offset, T value)
	{
		using BaseType = std::remove_cv_t<T>;
		this->_ESP.At<BaseType>(offset, value);
	}

	SYRINGE_FORCE_INLINE void Stack16(int offset, WORD value)
	{
		this->_ESP.At<WORD>(offset, value);
	}

	SYRINGE_FORCE_INLINE void Stack8(int offset, BYTE value)
	{
		this->_ESP.At<BYTE>(offset, value);
	}

	// ========================================================================
	// PUSH/POP SIMULATION (x86 32-bit calling convention)
	// ========================================================================

	/**
	 * Simulate x86 PUSH instruction: decrement ESP by 4, write value
	 * Supports: pointers, integers, enums, floats, DWORD-sized trivial types
	 */
	template<typename T>
	SYRINGE_FORCE_INLINE void Push(T value)
	{
		const DWORD new_esp = this->ESP() - 4;
		SyringeInternal::ValidateStackPointer(new_esp);

		this->ESP(new_esp);
		this->Stack(0, PushPopToDWORD(value));
	}

	/**
	 * Simulate x86 POP instruction: read value, increment ESP by 4
	 * Supports: pointers, integers, enums, floats, DWORD-sized trivial types
	 */
	template<typename T = DWORD>
	SYRINGE_FORCE_INLINE T Pop()
	{
		SyringeInternal::ValidateStackPointer(this->ESP());

		const DWORD value = this->Stack32(0);
		this->ESP(this->ESP() + 4);
		return DWORDToPushPop<T>(value);
	}

	// ========================================================================
	// BASE POINTER (EBP) HELPERS
	// ========================================================================

	template<typename T>
	SYRINGE_FORCE_INLINE T Base(int offset) const
	{
		return this->_EBP.At<T>(offset);
	}

	template<typename T>
	SYRINGE_FORCE_INLINE void Base(int offset, T value)
	{
		using BaseType = std::remove_cv_t<T>;
		this->_EBP.At<BaseType>(offset, value);
	}
};

// ============================================================================
// X86 FPU/SSE STATE STRUCTURES
// ============================================================================

#pragma pack(push, 4)

// x87 FPU register (80-bit extended precision)
struct FPURegister
{
	BYTE data[10];  // 80 bits
	BYTE reserved[6];  // Padding to 16 bytes

	// Convert to/from double (may lose precision)
	double ToDouble() const noexcept;
	void FromDouble(double value) noexcept;

	// Access raw 80-bit value
	const BYTE* Raw() const noexcept { return data; }
	BYTE* Raw() noexcept { return data; }
};

// FXSAVE format (modern FPU/SSE state) - 512 bytes
struct alignas(16) FXSAVE_AREA
{
	WORD    FCW;          // FPU Control Word
	WORD    FSW;          // FPU Status Word
	BYTE    FTW;          // FPU Tag Word (compressed)
	BYTE    Reserved1;
	WORD    FOP;          // Last FPU opcode
	DWORD   FPU_IP;       // FPU Instruction Pointer
	WORD    FPU_CS;       // FPU Code Segment
	WORD    Reserved2;
	DWORD   FPU_DP;       // FPU Data Pointer
	WORD    FPU_DS;       // FPU Data Segment
	WORD    Reserved3;
	DWORD   MXCSR;        // SSE Control/Status Register
	DWORD   MXCSR_MASK;   // MXCSR mask

	// x87 FPU registers (ST0-ST7)
	FPURegister FPU[8];

	// SSE registers (XMM0-XMM7 on 32-bit, XMM0-XMM15 on 64-bit)
	__m128  XMM[8];  // 128-bit XMM registers

	BYTE    Reserved4[224];  // Reserved/padding
};

static_assert(sizeof(FXSAVE_AREA) == 512, "FXSAVE_AREA must be 512 bytes");
static_assert(alignof(FXSAVE_AREA) == 16, "FXSAVE_AREA must be 16-byte aligned");

#pragma pack(pop)

// ============================================================================
// DEBUG REGISTER HELPER CLASS
// ============================================================================

class DebugRegisters
{
private:
	DWORD* dr_array;  // Points to DR0 in CONTEXT

public:
	explicit DebugRegisters(DWORD* dr0_ptr) noexcept : dr_array(dr0_ptr) {}

	// Individual register access
	DWORD DR0() const noexcept { return dr_array[0]; }
	DWORD DR1() const noexcept { return dr_array[1]; }
	DWORD DR2() const noexcept { return dr_array[2]; }
	DWORD DR3() const noexcept { return dr_array[3]; }
	DWORD DR6() const noexcept { return dr_array[4]; }  // Debug status
	DWORD DR7() const noexcept { return dr_array[5]; }  // Debug control

	void DR0(DWORD value) noexcept { dr_array[0] = value; }
	void DR1(DWORD value) noexcept { dr_array[1] = value; }
	void DR2(DWORD value) noexcept { dr_array[2] = value; }
	void DR3(DWORD value) noexcept { dr_array[3] = value; }
	void DR6(DWORD value) noexcept { dr_array[4] = value; }
	void DR7(DWORD value) noexcept { dr_array[5] = value; }

	// Hardware breakpoint helpers
	enum class BreakType : DWORD
	{
		Execute = 0,      // Break on instruction execution
		Write = 1,        // Break on data write
		ReadWrite = 3     // Break on data read/write
	};

	enum class BreakSize : DWORD
	{
		Byte = 0,         // 1-byte breakpoint
		Word = 1,         // 2-byte breakpoint
		DWord = 3,        // 4-byte breakpoint
		QWord = 2         // 8-byte breakpoint (x64 only)
	};

	// Set hardware breakpoint (index 0-3)
	void SetBreakpoint(int index, DWORD address, BreakType type, BreakSize size) noexcept;

	// Clear hardware breakpoint
	void ClearBreakpoint(int index) noexcept;

	// Check if breakpoint is enabled
	bool IsBreakpointEnabled(int index) const noexcept;
};

// ============================================================================
// SEGMENT REGISTER HELPER CLASS
// ============================================================================

class SegmentRegisters
{
private:
	DWORD* seg_array;  // Points to SegGs in CONTEXT

public:
	explicit SegmentRegisters(DWORD* seg_gs_ptr) noexcept : seg_array(seg_gs_ptr) {}

	DWORD GS() const noexcept { return seg_array[0]; }
	DWORD FS() const noexcept { return seg_array[1]; }
	DWORD ES() const noexcept { return seg_array[2]; }
	DWORD DS() const noexcept { return seg_array[3]; }
	DWORD CS() const noexcept { return seg_array[4]; }  // Set via EIP change
	DWORD SS() const noexcept { return seg_array[5]; }

	void GS(DWORD value) noexcept { seg_array[0] = value; }
	void FS(DWORD value) noexcept { seg_array[1] = value; }
	void ES(DWORD value) noexcept { seg_array[2] = value; }
	void DS(DWORD value) noexcept { seg_array[3] = value; }
	void SS(DWORD value) noexcept { seg_array[5] = value; }

	// Thread Information Block (TIB) access via FS register
	void* GetTIB() const noexcept;

	// Thread Environment Block (TEB) is same as TIB on Windows
	void* GetTEB() const noexcept { return GetTIB(); }
};

// ============================================================================
// FPU/X87 REGISTER HELPER CLASS
// ============================================================================

class FPURegisters
{
private:
	FXSAVE_AREA* fxsave;

public:
	explicit FPURegisters(FXSAVE_AREA* fxsave_ptr) noexcept : fxsave(fxsave_ptr) {}

	// Control/Status registers
	WORD FCW() const noexcept { return fxsave->FCW; }  // Control Word
	WORD FSW() const noexcept { return fxsave->FSW; }  // Status Word
	BYTE FTW() const noexcept { return fxsave->FTW; }  // Tag Word
	DWORD MXCSR() const noexcept { return fxsave->MXCSR; }  // SSE control

	void FCW(WORD value) noexcept { fxsave->FCW = value; }
	void FSW(WORD value) noexcept { fxsave->FSW = value; }
	void FTW(BYTE value) noexcept { fxsave->FTW = value; }
	void MXCSR(DWORD value) noexcept { fxsave->MXCSR = value; }

	// FPU stack registers (ST0-ST7)
	const FPURegister& ST(int index) const noexcept { return fxsave->FPU[index & 7]; }
	FPURegister& ST(int index) noexcept { return fxsave->FPU[index & 7]; }

	// Convenience accessors
	const FPURegister& ST0() const noexcept { return fxsave->FPU[0]; }
	const FPURegister& ST1() const noexcept { return fxsave->FPU[1]; }
	const FPURegister& ST2() const noexcept { return fxsave->FPU[2]; }
	const FPURegister& ST3() const noexcept { return fxsave->FPU[3]; }
	const FPURegister& ST4() const noexcept { return fxsave->FPU[4]; }
	const FPURegister& ST5() const noexcept { return fxsave->FPU[5]; }
	const FPURegister& ST6() const noexcept { return fxsave->FPU[6]; }
	const FPURegister& ST7() const noexcept { return fxsave->FPU[7]; }

	FPURegister& ST0() noexcept { return fxsave->FPU[0]; }
	FPURegister& ST1() noexcept { return fxsave->FPU[1]; }
	FPURegister& ST2() noexcept { return fxsave->FPU[2]; }
	FPURegister& ST3() noexcept { return fxsave->FPU[3]; }
	FPURegister& ST4() noexcept { return fxsave->FPU[4]; }
	FPURegister& ST5() noexcept { return fxsave->FPU[5]; }
	FPURegister& ST6() noexcept { return fxsave->FPU[6]; }
	FPURegister& ST7() noexcept { return fxsave->FPU[7]; }

	// FPU stack top (extracted from FSW)
	int GetStackTop() const noexcept { return (fxsave->FSW >> 11) & 7; }

	// Get logical ST(i) taking stack rotation into account
	const FPURegister& GetLogicalST(int i) const noexcept
	{
		return fxsave->FPU[(GetStackTop() + i) & 7];
	}
};

// ============================================================================
// SSE/XMM REGISTER HELPER CLASS
// ============================================================================

class SSERegisters
{
private:
	FXSAVE_AREA* fxsave;

public:
	explicit SSERegisters(FXSAVE_AREA* fxsave_ptr) noexcept : fxsave(fxsave_ptr) {}

	// Direct XMM register access
	__m128 XMM0() const noexcept { return fxsave->XMM[0]; }
	__m128 XMM1() const noexcept { return fxsave->XMM[1]; }
	__m128 XMM2() const noexcept { return fxsave->XMM[2]; }
	__m128 XMM3() const noexcept { return fxsave->XMM[3]; }
	__m128 XMM4() const noexcept { return fxsave->XMM[4]; }
	__m128 XMM5() const noexcept { return fxsave->XMM[5]; }
	__m128 XMM6() const noexcept { return fxsave->XMM[6]; }
	__m128 XMM7() const noexcept { return fxsave->XMM[7]; }

	void XMM0(__m128 value) noexcept { fxsave->XMM[0] = value; }
	void XMM1(__m128 value) noexcept { fxsave->XMM[1] = value; }
	void XMM2(__m128 value) noexcept { fxsave->XMM[2] = value; }
	void XMM3(__m128 value) noexcept { fxsave->XMM[3] = value; }
	void XMM4(__m128 value) noexcept { fxsave->XMM[4] = value; }
	void XMM5(__m128 value) noexcept { fxsave->XMM[5] = value; }
	void XMM6(__m128 value) noexcept { fxsave->XMM[6] = value; }
	void XMM7(__m128 value) noexcept { fxsave->XMM[7] = value; }

	// Indexed access
	__m128 XMM(int index) const noexcept { return fxsave->XMM[index & 7]; }
	void XMM(int index, __m128 value) noexcept { fxsave->XMM[index & 7] = value; }

	// ────────────────────────────────────────────────────────────────────────
	// TYPED ACCESSORS (interpret XMM as different data types)
	// ────────────────────────────────────────────────────────────────────────

	// As 4 floats (ps = packed single-precision)
	void SetPS(int index, float f0, float f1, float f2, float f3) noexcept
	{
		fxsave->XMM[index & 7] = _mm_set_ps(f3, f2, f1, f0);
	}

	void GetPS(int index, float& f0, float& f1, float& f2, float& f3) const noexcept
	{
		alignas(16) float temp[4];
		_mm_store_ps(temp, fxsave->XMM[index & 7]);
		f0 = temp[0]; f1 = temp[1]; f2 = temp[2]; f3 = temp[3];
	}

	// As 2 doubles (pd = packed double-precision)
	void SetPD(int index, double d0, double d1) noexcept
	{
		fxsave->XMM[index & 7] = _mm_castpd_ps(_mm_set_pd(d1, d0));
	}

	void GetPD(int index, double& d0, double& d1) const noexcept
	{
		alignas(16) double temp[2];
		_mm_store_pd(temp, _mm_castps_pd(fxsave->XMM[index & 7]));
		d0 = temp[0]; d1 = temp[1];
	}

	// As 4 ints (epi32 = packed 32-bit integers)
	void SetEPI32(int index, int i0, int i1, int i2, int i3) noexcept
	{
		fxsave->XMM[index & 7] = _mm_castsi128_ps(_mm_set_epi32(i3, i2, i1, i0));
	}

	void GetEPI32(int index, int& i0, int& i1, int& i2, int& i3) const noexcept
	{
		alignas(16) int temp[4];
		_mm_store_si128(reinterpret_cast<__m128i*>(temp),
						_mm_castps_si128(fxsave->XMM[index & 7]));
		i0 = temp[0]; i1 = temp[1]; i2 = temp[2]; i3 = temp[3];
	}

	// As raw bytes
	void GetBytes(int index, BYTE out[16]) const noexcept
	{
		_mm_storeu_ps(reinterpret_cast<float*>(out), fxsave->XMM[index & 7]);
	}

	void SetBytes(int index, const BYTE in[16]) noexcept
	{
		fxsave->XMM[index & 7] = _mm_loadu_ps(reinterpret_cast<const float*>(in));
	}
};

// ============================================================================
// EXTENDED REGISTERS CLASS (Full CONTEXT wrapper)
// ============================================================================

class REGISTERS_EXTENDED : public REGISTERS
{
private:
	// Additional fields from CONTEXT structure (match CONTEXT layout)
	DWORD   ContextFlags;
	DWORD   Dr0;
	DWORD   Dr1;
	DWORD   Dr2;
	DWORD   Dr3;
	DWORD   Dr6;
	DWORD   Dr7;
	FLOATING_SAVE_AREA FloatSave;
	DWORD   SegGs;
	DWORD   SegFs;
	DWORD   SegEs;
	DWORD   SegDs;
	// ... GPRs inherited from REGISTERS ...
	DWORD   SegCs;
	DWORD   SegSs;
	BYTE    ExtendedRegisters[512];

public:
	// ────────────────────────────────────────────────────────────────────────
	// CONTEXT FLAGS
	// ────────────────────────────────────────────────────────────────────────

	DWORD Flags() const noexcept { return ContextFlags; }
	void Flags(DWORD value) noexcept { ContextFlags = value; }

	// ────────────────────────────────────────────────────────────────────────
	// HELPER OBJECT ACCESSORS
	// ────────────────────────────────────────────────────────────────────────

	DebugRegisters Debug() noexcept
	{
		return DebugRegisters(&Dr0);
	}

	SegmentRegisters Segments() noexcept
	{
		return SegmentRegisters(&SegGs);
	}

	FPURegisters FPU() noexcept
	{
		return FPURegisters(reinterpret_cast<FXSAVE_AREA*>(ExtendedRegisters));
	}

	SSERegisters SSE() noexcept
	{
		return SSERegisters(reinterpret_cast<FXSAVE_AREA*>(ExtendedRegisters));
	}

	// ────────────────────────────────────────────────────────────────────────
	// DIRECT ACCESSORS (for when you don't need the helper objects)
	// ────────────────────────────────────────────────────────────────────────

	// Debug registers
	DWORD DR0() const noexcept { return Dr0; }
	DWORD DR1() const noexcept { return Dr1; }
	DWORD DR2() const noexcept { return Dr2; }
	DWORD DR3() const noexcept { return Dr3; }
	DWORD DR6() const noexcept { return Dr6; }
	DWORD DR7() const noexcept { return Dr7; }

	void DR0(DWORD value) noexcept { Dr0 = value; }
	void DR1(DWORD value) noexcept { Dr1 = value; }
	void DR2(DWORD value) noexcept { Dr2 = value; }
	void DR3(DWORD value) noexcept { Dr3 = value; }
	void DR6(DWORD value) noexcept { Dr6 = value; }
	void DR7(DWORD value) noexcept { Dr7 = value; }

	// Segment registers
	DWORD GS() const noexcept { return SegGs; }
	DWORD FS() const noexcept { return SegFs; }
	DWORD ES() const noexcept { return SegEs; }
	DWORD DS() const noexcept { return SegDs; }
	DWORD CS() const noexcept { return SegCs; }
	DWORD SS() const noexcept { return SegSs; }

	void GS(DWORD value) noexcept { SegGs = value; }
	void FS(DWORD value) noexcept { SegFs = value; }
	void ES(DWORD value) noexcept { SegEs = value; }
	void DS(DWORD value) noexcept { SegDs = value; }
	void SS(DWORD value) noexcept { SegSs = value; }

	// EIP (instruction pointer) - already in base REGISTERS as Origin()
	DWORD EIP() const noexcept { return this->Origin(); }
	void EIP(DWORD value) noexcept { this->Origin(value); }

	// Access raw FXSAVE area
	FXSAVE_AREA* GetFXSAVE() noexcept
	{
		return reinterpret_cast<FXSAVE_AREA*>(ExtendedRegisters);
	}

	const FXSAVE_AREA* GetFXSAVE() const noexcept
	{
		return reinterpret_cast<const FXSAVE_AREA*>(ExtendedRegisters);
	}

	// ────────────────────────────────────────────────────────────────────────
	// CONVENIENCE METHODS
	// ────────────────────────────────────────────────────────────────────────

	// Check if specific context portions are present
	bool HasDebugRegisters() const noexcept
	{
		return (ContextFlags & CONTEXT_DEBUG_REGISTERS) != 0;
	}

	bool HasFloatingPoint() const noexcept
	{
		return (ContextFlags & CONTEXT_FLOATING_POINT) != 0;
	}

	bool HasExtendedRegisters() const noexcept
	{
		return (ContextFlags & CONTEXT_EXTENDED_REGISTERS) != 0;
	}

	bool HasSegments() const noexcept
	{
		return (ContextFlags & CONTEXT_SEGMENTS) != 0;
	}

	// Save/restore FXSAVE state to external buffer
	void SaveFXSAVE(FXSAVE_AREA* dest) const noexcept
	{
		std::memcpy(dest, ExtendedRegisters, sizeof(FXSAVE_AREA));
	}

	void RestoreFXSAVE(const FXSAVE_AREA* src) noexcept
	{
		std::memcpy(ExtendedRegisters, src, sizeof(FXSAVE_AREA));
	}
};

// Ensure layout matches expectations
static_assert(sizeof(REGISTERS_EXTENDED) <= sizeof(void*) * 256,
	"REGISTERS_EXTENDED size seems wrong - check CONTEXT structure alignment");

// ============================================================================
// IMPLEMENTATION OF HELPER METHODS
// ============================================================================

// FPURegister conversion methods
inline double FPURegister::ToDouble() const noexcept
{
	// x87 80-bit format: 1 sign bit, 15 exp bits, 64 mantissa bits
	// This is a simplified conversion; full implementation needs careful handling
	double result = 0.0;

	// Load 80-bit value via inline assembly or use compiler intrinsics
	// For now, return 0.0 as placeholder (implement based on your needs)
	// Real implementation: __asm { fld tbyte ptr [this->data]; fstp qword ptr [result] }

	return result;
}

inline void FPURegister::FromDouble(double value) noexcept
{
	// Store double as 80-bit extended precision
	// Real implementation: __asm { fld qword ptr [value]; fstp tbyte ptr [this->data] }
	std::memset(data, 0, sizeof(data));  // Placeholder
}

// DebugRegisters breakpoint methods
inline void DebugRegisters::SetBreakpoint(int index, DWORD address,
										  BreakType type, BreakSize size) noexcept
{
	if (index < 0 || index > 3) return;

	// Set debug address register
	dr_array[index] = address;

	// Configure DR7 control register
	DWORD dr7 = DR7();

	// Enable breakpoint (LE bit for local enable)
	const int enable_bit = index * 2;
	dr7 |= (1 << enable_bit);

	// Set breakpoint type and size
	const int config_base = 16 + (index * 4);
	dr7 &= ~(0xF << config_base);  // Clear existing config
	dr7 |= (static_cast<DWORD>(type) << config_base);
	dr7 |= (static_cast<DWORD>(size) << (config_base + 2));

	DR7(dr7);
}

inline void DebugRegisters::ClearBreakpoint(int index) noexcept
{
	if (index < 0 || index > 3) return;

	// Clear enable bit in DR7
	DWORD dr7 = DR7();
	const int enable_bit = index * 2;
	dr7 &= ~(3 << enable_bit);  // Clear both local and global enable
	DR7(dr7);

	// Clear address register
	dr_array[index] = 0;
}

inline bool DebugRegisters::IsBreakpointEnabled(int index) const noexcept
{
	if (index < 0 || index > 3) return false;

	const int enable_bit = index * 2;
	return (DR7() & (3 << enable_bit)) != 0;
}

// SegmentRegisters TIB access
inline void* SegmentRegisters::GetTIB() const noexcept
{
	// TIB is pointed to by FS segment base address
	// On Windows x86, FS:[0] points to TIB
	// This requires inline assembly or platform-specific code

#ifdef _WIN32
	// Use __readfsdword intrinsic
	return reinterpret_cast<void*>(__readfsdword(0x18));
#else
	return nullptr;
#endif
}

// ============================================================================
// CONVENIENCE MACROS FOR EXTENDED REGISTERS
// ============================================================================

// Cast standard REGISTERS* to extended version
#define REGISTERS_EX(R) (reinterpret_cast<REGISTERS_EXTENDED*>(R))

// Access extended register features in hooks
#define GET_DEBUG_REG(R) (REGISTERS_EX(R)->Debug())
#define GET_SEGMENT_REG(R) (REGISTERS_EX(R)->Segments())
#define GET_FPU_REG(R) (REGISTERS_EX(R)->FPU())
#define GET_SSE_REG(R) (REGISTERS_EX(R)->SSE())

// Example usage in hook:
// DEFINE_HOOK(0x12345, MyHook, 6)
// {
//     auto sse = GET_SSE_REG(R);
//     __m128 xmm0 = sse.XMM0();
//     // ... manipulate SSE registers ...
//     return 0;
// }

// ============================================================================
// EXPORT MACROS FOR HOOK FUNCTIONS
// ============================================================================

#define EXPORT extern "C" __declspec(dllexport) DWORD __cdecl
#define EXPORT_DEBUG_DECLARE(name) DWORD SYRINGE_FORCE_INLINE name(REGISTERS* R);
#define EXPORT_DEBUG(name) DWORD SYRINGE_FORCE_INLINE name(REGISTERS* R)
#define EXPORT_FUNC(name) extern "C" __declspec(dllexport) DWORD __cdecl name (REGISTERS *R)
#define EXPORT_FUNC_NAKED(name) extern "C" __declspec(naked, dllexport) DWORD __cdecl name (REGISTERS *R)

// ============================================================================
// SYRINGE V2 HOOK REGISTRATION
// ============================================================================

#if SYR_VER == 2

#pragma pack(push, 16)
#pragma warning(push)
#pragma warning(disable : 4324)

__declspec(align(16)) struct hookdecl
{
	unsigned int hookAddr;
	unsigned int hookSize;
	const char* hookName;
};

__declspec(align(16)) struct hookdeclfunc
{
	unsigned int hookAddr;
	unsigned int hookSize;
	const void* hookFunc;
};

#pragma warning(pop)
#pragma pack(pop)

#define SYRINGE_HOOKS_SECTION_NAME ".syhks00"
#pragma section(SYRINGE_HOOKS_SECTION_NAME, read)

#define ASMJIT_PATCH_SECTION_NAME ".asmjit0"
#pragma section(ASMJIT_PATCH_SECTION_NAME, read)

#define PATCH_SECTION_NAME ".patch"
#pragma section(PATCH_SECTION_NAME, read)

namespace SyringeData
{
	namespace Hooks { };
	namespace Hosts { };
}

#define _YR_PP_CAT_INNER(a, b) a##b
#define _YR_PP_CAT(a, b) _YR_PP_CAT_INNER(a, b)
#define _YR_PP_STRINGIZE_INNER(x) #x
#define _YR_PP_STRINGIZE(x) _YR_PP_STRINGIZE_INNER(x)

#define _YR_LINKER_FORCE_INCLUDE(symbol) \
	__pragma(comment(linker, "/include:_" _YR_PP_STRINGIZE(symbol)))

#define _YR_DEFINE_INCLUDE_ANCHOR(name, expr) \
	extern "C" __declspec(selectany) const void* name = (expr); \
	_YR_LINKER_FORCE_INCLUDE(name)

#define declhost(exename, checksum) \
namespace SyringeData { namespace Hosts { __declspec(allocate(".syexe00")) hostdecl _hst__ ## exename  { checksum, #exename }; }; }; \
_YR_DEFINE_INCLUDE_ANCHOR(_YR_PP_CAT(YrKeepHost_, exename), &SyringeData::Hosts::_hst__ ## exename)

#define declhook(hook, funcname, size) \
namespace SyringeData { namespace Hooks { __declspec(allocate(".syhks00")) hookdecl _hk__ ## hook ## funcname  {  hook, size, #funcname }; }; }; \
_YR_DEFINE_INCLUDE_ANCHOR(_YR_PP_CAT(YrKeepHook_, _YR_PP_CAT(hook, funcname)), &SyringeData::Hooks::_hk__ ## hook ## funcname)

#define decl_asmjit_patch_data(hook, funcname, size) \
namespace AsmjitPatchData { \
namespace Patchs { \
__declspec(allocate(ASMJIT_PATCH_SECTION_NAME)) \
hookdeclfunc _hk__ ## hook ## funcname { hook, size, &funcname }; \
}; };

#endif // SYR_VER == 2

#ifndef decl_asmjit_patch_data
#define decl_asmjit_patch_data(hook, funcname, size)
#endif

#ifndef declhook
#define declhook(hook, funcname, size)
#endif

// ============================================================================
// HOOK DEFINITION MACROS
// ============================================================================

#ifndef DEBUG_HOOK

#define ASMJIT_PATCH_AGAIN(hook, funcname, size) \
	declhook(hook, funcname, size)

#define ASMJIT_PATCH(hook, funcname, size) \
	EXPORT_FUNC(funcname); \
	declhook(hook, funcname, size) \
	EXPORT_FUNC(funcname)

#define DEFINE_HOOK(hook, funcname, size) \
	EXPORT_FUNC(funcname); \
	declhook(hook, funcname, size) \
	EXPORT_FUNC(funcname)

#define DEFINE_HOOK_AGAIN(hook, funcname, size) \
	declhook(hook, funcname, size)

#else // DEBUG_HOOK

#include <chrono>

struct DebugData
{
	static std::chrono::steady_clock::time_point StartTime;
	static void Start(DWORD origin, const char* funcName, int size);
	static void End(DWORD origin, const char* funcName, int size);
	static void StartO(DWORD origin, const char* funcName, int size);
	static void EndO(DWORD origin, const char* funcName, int size);
};

#define DEFINE_HOOK(hook, funcname, size) \
	declhook(hook, funcname##_DEBUG_HOOK__LOG_, size) \
	EXPORT_DEBUG_DECLARE(funcname##_DEBUG_) \
	EXPORT_FUNC(funcname##_DEBUG_HOOK__LOG_) \
	{ \
		DebugData::Start(R->Origin(), #funcname, size); \
		DWORD ret = funcname##_DEBUG_(R); \
		DebugData::End(R->Origin(), #funcname, size); \
		return ret; \
	} \
	EXPORT_DEBUG(funcname##_DEBUG_)

#define DEFINE_HOOK_AGAIN(hook, funcname, size) \
	declhook(hook, funcname##_DEBUG_HOOK__LOG_, size)

#endif // DEBUG_HOOK

// ============================================================================
// COMPILE-TIME VALIDATION
// ============================================================================

// Ensure REGISTERS class layout matches expected size
static_assert(sizeof(REGISTERS) == 10 * sizeof(DWORD),
	"SYRINGE: REGISTERS class size mismatch. Check struct packing.");

static_assert(alignof(REGISTERS) <= 4,
	"SYRINGE: REGISTERS alignment too large. May break x86 ABI.");

// Ensure register wrappers don't add overhead
static_assert(sizeof(LimitedRegister) == sizeof(DWORD));
static_assert(sizeof(ExtendedRegister) == sizeof(DWORD));
static_assert(sizeof(StackRegister) == sizeof(DWORD));

// ============================================================================
// END OF HEADER
// ============================================================================