// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_COMPILERDEFS_H_INCLUDED
#define ASMJIT_CORE_COMPILERDEFS_H_INCLUDED

#include "../core/api-config.h"
#include "../core/operand.h"
#include "../core/support.h"
#include "../core/type.h"
#include "../core/zonestring.h"

ASMJIT_BEGIN_NAMESPACE

class RAWorkReg;

//! \addtogroup asmjit_compiler
//! \{

//! Flags associated with a virtual register \ref VirtReg.
enum class VirtRegFlags : uint8_t {
  kNone = 0x00u,

  //! True if this is a fixed register, never reallocated.
  kIsFixed = 0x01u,

  //! True if the virtual register is only used as a stack area (never accessed as register). Stack area is allocated
  //! via \ref BaseCompiler::_newStack() and then architecture dependent compilers like \ref x86::Compiler::newStack().
  kIsStackArea = 0x02u,

  //! True if the virtual register has a stack slot.
  //!
  //! Stack slots are assigned by the register allocator - so initially when a \ref VirtReg is created this flag would
  //! not be set. When a virtual register is spilled, stack slot is automatically created for the register and the
  //! \ref VirtReg::_stackOffset member is updated. Stack areas will always have associated stack slot during register
  //! allocation.
  kHasStackSlot = 0x04u,

  //! Virtual register `log2(alignment)` mask (for spilling) (3 bits in flags).
  //!
  //! \note For space purposes the alignment is stored as log2(alignment). So the alignment is `1 << _alignmentLog2`.
  kAlignmentLog2Mask = 0xE0u
};
ASMJIT_DEFINE_ENUM_FLAGS(VirtRegFlags)

//! Public virtual register interface, managed by \ref BaseCompiler.
//!
//! When a virtual register is created by \ref BaseCompiler a `VirtReg` is linked with the register operand id it
//! returns. This `VirtReg` can be accessed via \ref BaseCompiler::virtRegByReg() function, which returns a pointer
//! to `VirtReg`.
//!
//! In general, `VirtReg` should be only introspected as it contains important variables that are needed and managed
//! by AsmJit, however, the `VirtReg` API can also be used to influence register allocation. For example there is
//! a \ref VirtReg::setWeight() function, which could be used to increase a weight of a virtual register (thus make
//! it hard to spill, for example). In addition, there is a \ref VirtReg::setHomeIdHint() function, which can be used
//! to do an initial assignment of a physical register of a virtual register. However, AsmJit could still override
//! the physical register assigned in some special cases.
class VirtReg {
public:
  ASMJIT_NONCOPYABLE(VirtReg)

  //! \name Constants
  //! \{

  static constexpr inline uint32_t kAlignmentLog2Mask  = uint32_t(VirtRegFlags::kAlignmentLog2Mask);
  static constexpr inline uint32_t kAlignmentLog2Shift = Support::ctz_const<kAlignmentLog2Mask>;

  static ASMJIT_INLINE_CONSTEXPR VirtRegFlags _flagsFromAlignmentLog2(uint32_t alignmentLog2) noexcept {
    return VirtRegFlags(alignmentLog2 << kAlignmentLog2Shift);
  }

  static ASMJIT_INLINE_CONSTEXPR uint32_t _alignmentLog2FromFlags(VirtRegFlags flags) noexcept {
    return uint32_t(flags) >> kAlignmentLog2Shift;
  }

  //! \}

  //! \name Members
  //! \{

  //! Virtual register id.
  uint32_t _id = 0;
  //! Virtual register size (can be smaller than a real register size if only a part of the register is used).
  uint32_t _virtSize = 0;

  //! Virtual register type.
  RegType _regType = RegType::kNone;
  //! Virtual register flags.
  VirtRegFlags _regFlags = VirtRegFlags::kNone;

  //! Virtual register weight.
  //!
  //! Weight is used for alloc/spill decisions. Higher weight means a higher priority to keep the virtual
  //! register always allocated as a physical register. The default weight is zero, which means standard
  //! weight (no weight is added to the initial priority, which is calculated based on the number of uses
  //! divided by the sum of widths of all live spans).
  uint8_t _weight = 0;

  //! Type id.
  TypeId _typeId = TypeId::kVoid;

  //! Home register hint for the register allocator (initially unassigned).
  uint8_t _homeIdHint = Reg::kIdBad;

  //! Stack offset assigned by the register allocator relative to stack pointer (can be negative as well).
  int32_t _stackOffset = 0;

  //! Virtual register name (either empty or user provided).
  ZoneString<16> _name {};

  // The following members are used exclusively by RAPass. They are initialized when the VirtReg is created to
  // null pointers and then changed during RAPass execution. RAPass sets them back to NULL before it returns.

  //! Reference to `RAWorkReg`, used during register allocation.
  RAWorkReg* _workReg = nullptr;

  //! \}

  //! \name Construction & Destruction
  //! \{

  ASMJIT_INLINE_NODEBUG VirtReg(RegType regType, VirtRegFlags regFlags, uint32_t id, uint32_t virtSize, TypeId typeId) noexcept
    : _id(id),
      _virtSize(virtSize),
      _regType(regType),
      _regFlags(regFlags),
      _typeId(typeId) {}

  //! \}

  //! \name Accessors
  //! \{

  //! Returns the virtual register id.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t id() const noexcept { return _id; }

  //! Returns a virtual register type (maps to the physical register type as well).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RegType regType() const noexcept { return _regType; }

  //! Returns a virtual register flags.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG VirtRegFlags regFlags() const noexcept { return _regFlags; }

  //! Returns the virtual register size.
  //!
  //! The virtual register size describes how many bytes the virtual register needs to store its content. It can be
  //! smaller than the physical register size, see `regSize()`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t virtSize() const noexcept { return _virtSize; }

  //! Returns the virtual register alignment required for memory operations (load/spill).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t alignment() const noexcept { return 1u << _alignmentLog2FromFlags(_regFlags); }

  //! Returns the virtual register type id.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG TypeId typeId() const noexcept { return _typeId; }

  //! Returns the virtual register weight - the register allocator can use it as explicit hint for alloc/spill
  //! decisions.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t weight() const noexcept { return _weight; }

  //! Sets the virtual register weight (0 to 255) - the register allocator can use it as explicit hint for alloc/spill
  //! decisions and initial bin-packing.
  ASMJIT_INLINE_NODEBUG void setWeight(uint32_t weight) noexcept { _weight = uint8_t(weight); }

  //! Returns whether the virtual register is always allocated to a fixed physical register (and never reallocated).
  //!
  //! \note This is only used for special purposes and it's mostly internal.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool isFixed() const noexcept { return Support::test(_regFlags, VirtRegFlags::kIsFixed); }

  //! Tests whether the virtual register is in fact a stack that only uses the virtual register id.
  //!
  //! \note It's an error if a stack is accessed as a register.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool isStackArea() const noexcept { return Support::test(_regFlags, VirtRegFlags::kIsStackArea); }

  //! Tests whether this virtual register (or stack) has assigned a stack offset.
  //!
  //! If this is a virtual register that was never allocated on stack, it would return false, otherwise if
  //! it's a virtual register that was spilled or explicitly allocated stack, the return value would be true.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool hasStackSlot() const noexcept { return Support::test(_regFlags, VirtRegFlags::kHasStackSlot); }

  //! Assigns a stack offset of this virtual register to `stackOffset` and sets `_hasStackSlot` to true.
  ASMJIT_INLINE_NODEBUG void assignStackSlot(int32_t stackOffset) noexcept {
    _regFlags |= VirtRegFlags::kHasStackSlot;
    _stackOffset = stackOffset;
  }

  //! Tests whether this virtual register has assigned a physical register as a hint to the register allocator.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool hasHomeIdHint() const noexcept { return _homeIdHint != Reg::kIdBad; }

  //! Returns a physical register hint, which will be used by the register allocator.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t homeIdHint() const noexcept { return _homeIdHint; }

  //! Assigns a physical register hint, which will be used by the register allocator.
  ASMJIT_INLINE_NODEBUG void setHomeIdHint(uint32_t homeId) noexcept { _homeIdHint = uint8_t(homeId); }
  //! Resets a physical register hint.
  ASMJIT_INLINE_NODEBUG void resetHomeIdHint() noexcept { _homeIdHint = Reg::kIdBad; }

  //! Returns a stack offset associated with a virtual register or explicit stack allocation.
  //!
  //! \note Always verify that the stack offset has been assigned by calling \ref hasStackSlot(). The return
  //! value will be zero when the stack offset was not assigned.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG int32_t stackOffset() const noexcept { return _stackOffset; }

  //! Returns the virtual register name.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const char* name() const noexcept { return _name.data(); }

  //! Returns the size of the virtual register name.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t nameSize() const noexcept { return _name.size(); }

  //! Tests whether the virtual register has an associated `RAWorkReg` at the moment.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool hasWorkReg() const noexcept { return _workReg != nullptr; }

  //! Returns an associated RAWorkReg with this virtual register (only valid during register allocation).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RAWorkReg* workReg() const noexcept { return _workReg; }

  //! Associates a RAWorkReg with this virtual register (used by register allocator).
  ASMJIT_INLINE_NODEBUG void setWorkReg(RAWorkReg* workReg) noexcept { _workReg = workReg; }

  //! Reset the RAWorkReg association (used by register allocator).
  ASMJIT_INLINE_NODEBUG void resetWorkReg() noexcept { _workReg = nullptr; }

  //! \}
};

//! \}

ASMJIT_END_NAMESPACE

#endif // ASMJIT_CORE_COMPILERDEFS_H_INCLUDED

