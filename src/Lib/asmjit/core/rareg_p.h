// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_RAREG_P_H_INCLUDED
#define ASMJIT_CORE_RAREG_P_H_INCLUDED

#include "../core/api-config.h"
#ifndef ASMJIT_NO_COMPILER

#include "../core/radefs_p.h"
#include "../core/zonevector.h"

ASMJIT_BEGIN_NAMESPACE

//! \cond INTERNAL
//! \addtogroup asmjit_ra
//! \{

//! Flags used by \ref RAWorkReg.
enum class RAWorkRegFlags : uint32_t {
  //! No flags.
  kNone = 0,

  //! This register has already been allocated.
  kAllocated = 0x00000001u,

  //! The register is used across multiple basic blocks.
  kMultiBlockUse = 0x00000008u,

  //! Stack slot has to be allocated.
  kStackUsed = 0x00000010u,
  //! Stack allocation is preferred.
  kStackPreferred = 0x00000020u,
  //! Marked for stack argument reassignment.
  kStackArgToStack = 0x00000040u,

  //! Set when this register is used as a LEAD consecutive register at least once.
  kLeadConsecutive = 0x00001000u,
  //! Used to mark consecutive registers during processing.
  kProcessedConsecutive = 0x00002000u,

  //! Used during liveness analysis to mark registers that were visited (to mark RATiedFlags::kLast).
  kSingleBlockVisitedFlag = 0x40000000u,
  //! Used during liveness analysis to mark registers that only live in a single basic block as alive.
  kSingleBlockLiveFlag = 0x80000000u
};
ASMJIT_DEFINE_ENUM_FLAGS(RAWorkRegFlags)

//! Work register provides additional data of \ref VirtReg that is used by register allocator.
//!
//! In general when a virtual register is found by register allocator it maps it to \ref RAWorkReg
//! and then only works with it. The reason for such mapping is that users can create many virtual
//! registers, which are not used inside a register allocation scope (which is currently always a
//! function). So register allocator basically scans the function for virtual registers and maps
//! them into WorkRegs, which receive a temporary ID (workId), which starts from zero. This WorkId
//! is then used in bit-arrays and other mappings.
class RAWorkReg {
public:
  ASMJIT_NONCOPYABLE(RAWorkReg)

  //! \name Constants
  //! \{

  static inline constexpr uint32_t kNoArgIndex = 0xFFu;

  //! \}

  //! \name Members
  //! \{

  //! RAPass-specific register identifier used during liveness analysis and register allocation.
  RAWorkId _workId = kBadWorkId;
  //! Copy of virtual register id used by \ref VirtReg.
  uint32_t _vRegId = 0;

  //! Permanent association with \ref VirtReg.
  VirtReg* _virtReg = nullptr;
  //! Temporary association with \ref RATiedReg.
  RATiedReg* _tiedReg = nullptr;
  //! Stack slot associated with the register.
  RAStackSlot* _stackSlot = nullptr;

  //! Copy of a signature used by \ref VirtReg.
  OperandSignature _signature {};
  //! RAPass specific flags used during analysis and allocation.
  RAWorkRegFlags _flags = RAWorkRegFlags::kNone;

  //! The identifier of a basic block this register lives in.
  //!
  //! If this register is used by multiple basic blocks, the id would always be `kBadBlock`.
  RABlockId _singleBasicBlockId = kBadBlockId;

  //! Constains all USE ids collected from all instructions.
  //!
  //! If this mask is non-zero and not a power of two, it means that the register is used multiple times in
  //! instructions where it requires to have a different use ID. This means that in general it's not possible
  //! to keep this register in a single home.
  RegMask _useIdMask = 0;
  //! Preferred mask of registers (if non-zero) to allocate this register to.
  //!
  //! If this mask is zero it means that either there is no intersection of preferred registers collected from all
  //! TiedRegs or there is no preference at all (the register can be allocated to any register all the time).
  RegMask _preferredMask = 0xFFFFFFFFu;
  //! Consecutive mask, which was collected from all instructions where this register was used as a lead consecutive
  //! register.
  RegMask _consecutiveMask = 0xFFFFFFFFu;
  //! IDs of all physical registers that are clobbered during the lifetime of this WorkReg.
  //!
  //! This mask should be updated by `RAPass::buildLiveness()`, because it's global and should
  //! be updated after unreachable code has been removed.
  RegMask _clobberSurvivalMask = 0;
  //! IDs of all physical registers this WorkReg has been allocated to.
  RegMask _allocatedMask = 0;

  //! A byte-mask where each bit represents one valid byte of the register.
  uint64_t _regByteMask = 0;

  //! Argument index (or `kNoArgIndex` if none).
  uint8_t _argIndex = kNoArgIndex;
  //! Argument value index in the pack (0 by default).
  uint8_t _argValueIndex = 0;
  //! Global home register ID (if any, assigned by RA).
  uint8_t _homeRegId = Reg::kIdBad;
  //! Global hint register ID (provided by RA or user).
  uint8_t _hintRegId = Reg::kIdBad;

  //! Live spans of the `VirtReg`.
  RALiveSpans _liveSpans {};
  //! Live statistics.
  RALiveStats _liveStats {};

  //! All nodes that read/write this VirtReg/WorkReg.
  ZoneVector<BaseNode*> _refs {};
  //! All nodes that write to this VirtReg/WorkReg.
  ZoneVector<BaseNode*> _writes {};

  //! Contains work IDs of all immediate consecutive registers of this register.
  //!
  //! \note This bit array only contains immediate consecutives. This means that if this is a register that is
  //! followed by 3 more registers, then it would still have only a single immediate. The rest registers would
  //! have immediate consecutive registers as well, except the last one.
  ZoneBitVector _immediateConsecutives {};

  //! \}

  //! \name Construction & Destruction
  //! \{

  ASMJIT_INLINE_NODEBUG RAWorkReg(VirtReg* vReg, OperandSignature signature, RAWorkId workId) noexcept
    : _workId(workId),
      _vRegId(vReg->id()),
      _virtReg(vReg),
      _signature(signature),
      _hintRegId(uint8_t(vReg->homeIdHint())) {}

  //! \}

  //! \name Accessors
  //! \{

  [[nodiscard]]
  ASMJIT_INLINE RAWorkId workId() const noexcept {
    ASMJIT_ASSERT(_workId != kBadWorkId);
    return _workId;
  }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t vRegId() const noexcept { return _vRegId; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG TypeId typeId() const noexcept { return _virtReg->typeId(); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RAWorkRegFlags flags() const noexcept { return _flags; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool hasFlag(RAWorkRegFlags flag) const noexcept { return Support::test(_flags, flag); }

  ASMJIT_INLINE_NODEBUG void addFlags(RAWorkRegFlags flags) noexcept { _flags |= flags; }
  ASMJIT_INLINE_NODEBUG void xorFlags(RAWorkRegFlags flags) noexcept { _flags ^= flags; }
  ASMJIT_INLINE_NODEBUG void clearFlags(RAWorkRegFlags flags) noexcept { _flags &= ~flags; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool isAllocated() const noexcept { return hasFlag(RAWorkRegFlags::kAllocated); }

  ASMJIT_INLINE_NODEBUG void markAllocated() noexcept { addFlags(RAWorkRegFlags::kAllocated); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool isWithinSingleBasicBlock() const noexcept { return !hasFlag(RAWorkRegFlags::kMultiBlockUse); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RABlockId singleBasicBlockId() const noexcept { return _singleBasicBlockId; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool isLeadConsecutive() const noexcept { return hasFlag(RAWorkRegFlags::kLeadConsecutive); }

  ASMJIT_INLINE_NODEBUG void markLeadConsecutive() noexcept { addFlags(RAWorkRegFlags::kLeadConsecutive); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool isProcessedConsecutive() const noexcept { return hasFlag(RAWorkRegFlags::kProcessedConsecutive); }

  ASMJIT_INLINE_NODEBUG void markProcessedConsecutive() noexcept { addFlags(RAWorkRegFlags::kProcessedConsecutive); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool isStackUsed() const noexcept { return hasFlag(RAWorkRegFlags::kStackUsed); }

  ASMJIT_INLINE_NODEBUG void markStackUsed() noexcept { addFlags(RAWorkRegFlags::kStackUsed); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool isStackPreferred() const noexcept { return hasFlag(RAWorkRegFlags::kStackPreferred); }

  ASMJIT_INLINE_NODEBUG void markStackPreferred() noexcept { addFlags(RAWorkRegFlags::kStackPreferred); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG OperandSignature signature() const noexcept { return _signature; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RegType type() const noexcept { return _signature.regType(); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RegGroup group() const noexcept { return _signature.regGroup(); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG VirtReg* virtReg() const noexcept { return _virtReg; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool hasTiedReg() const noexcept { return _tiedReg != nullptr; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RATiedReg* tiedReg() const noexcept { return _tiedReg; }

  ASMJIT_INLINE_NODEBUG void setTiedReg(RATiedReg* tiedReg) noexcept { _tiedReg = tiedReg; }

  ASMJIT_INLINE_NODEBUG void resetTiedReg() noexcept { _tiedReg = nullptr; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool hasStackSlot() const noexcept { return _stackSlot != nullptr; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RAStackSlot* stackSlot() const noexcept { return _stackSlot; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RALiveSpans& liveSpans() noexcept { return _liveSpans; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const RALiveSpans& liveSpans() const noexcept { return _liveSpans; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RALiveStats& liveStats() noexcept { return _liveStats; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const RALiveStats& liveStats() const noexcept { return _liveStats; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool hasArgIndex() const noexcept { return _argIndex != kNoArgIndex; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t argIndex() const noexcept { return _argIndex; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t argValueIndex() const noexcept { return _argValueIndex; }

  inline void setArgIndex(uint32_t argIndex, uint32_t valueIndex) noexcept {
    _argIndex = uint8_t(argIndex);
    _argValueIndex = uint8_t(valueIndex);
  }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool hasHomeRegId() const noexcept { return _homeRegId != Reg::kIdBad; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t homeRegId() const noexcept { return _homeRegId; }

  ASMJIT_INLINE_NODEBUG void setHomeRegId(uint32_t physId) noexcept { _homeRegId = uint8_t(physId); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool hasHintRegId() const noexcept { return _hintRegId != Reg::kIdBad; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t hintRegId() const noexcept { return _hintRegId; }

  ASMJIT_INLINE_NODEBUG void setHintRegId(uint32_t physId) noexcept { _hintRegId = uint8_t(physId); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RegMask useIdMask() const noexcept { return _useIdMask; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool hasUseIdMask() const noexcept { return _useIdMask != 0u; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool hasMultipleUseIds() const noexcept { return Support::has_at_least_2_bits_set(_useIdMask); }

  ASMJIT_INLINE_NODEBUG void addUseIdMask(RegMask mask) noexcept { _useIdMask |= mask; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RegMask preferredMask() const noexcept { return _preferredMask; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool hasPreferredMask() const noexcept { return _preferredMask != 0xFFFFFFFFu; }

  ASMJIT_INLINE_NODEBUG void restrictPreferredMask(RegMask mask) noexcept { _preferredMask &= mask; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RegMask consecutiveMask() const noexcept { return _consecutiveMask; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool hasConsecutiveMask() const noexcept { return _consecutiveMask != 0xFFFFFFFFu; }

  ASMJIT_INLINE_NODEBUG void restrictConsecutiveMask(RegMask mask) noexcept { _consecutiveMask &= mask; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RegMask clobberSurvivalMask() const noexcept { return _clobberSurvivalMask; }

  ASMJIT_INLINE_NODEBUG void addClobberSurvivalMask(RegMask mask) noexcept { _clobberSurvivalMask |= mask; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RegMask allocatedMask() const noexcept { return _allocatedMask; }

  ASMJIT_INLINE_NODEBUG void addAllocatedMask(RegMask mask) noexcept { _allocatedMask |= mask; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint64_t regByteMask() const noexcept { return _regByteMask; }

  ASMJIT_INLINE_NODEBUG void setRegByteMask(uint64_t mask) noexcept { _regByteMask = mask; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool hasImmediateConsecutives() const noexcept { return !_immediateConsecutives.empty(); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const ZoneBitVector& immediateConsecutives() const noexcept { return _immediateConsecutives; }

  [[nodiscard]]
  inline Error addImmediateConsecutive(ZoneAllocator* allocator, RAWorkId workId) noexcept {
    if (_immediateConsecutives.size() <= uint32_t(workId)) {
      ASMJIT_PROPAGATE(_immediateConsecutives.resize(allocator, uint32_t(workId) + 1u));
    }

    _immediateConsecutives.setBit(workId, true);
    return kErrorOk;
  }

  //! \}
};

//! \}
//! \endcond

ASMJIT_END_NAMESPACE

#endif // !ASMJIT_NO_COMPILER
#endif // ASMJIT_CORE_RAREG_P_H_INCLUDED
