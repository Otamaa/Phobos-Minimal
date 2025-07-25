// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_RAINST_P_H_INCLUDED
#define ASMJIT_CORE_RAINST_P_H_INCLUDED

#include "../core/api-config.h"
#ifndef ASMJIT_NO_COMPILER

#include "../core/compilerdefs.h"
#include "../core/radefs_p.h"
#include "../core/rareg_p.h"
#include "../core/support_p.h"
#include "../core/zone.h"

ASMJIT_BEGIN_NAMESPACE

//! \cond INTERNAL
//! \addtogroup asmjit_ra
//! \{

//! Register allocator's data associated with each `InstNode`.
class RAInst {
public:
  ASMJIT_NONCOPYABLE(RAInst)

  //! \name Members
  //! \{

  //! Instruction RW flags.
  InstRWFlags _instRWFlags;
  //! Aggregated RATiedFlags from all operands & instruction specific flags.
  RATiedFlags _flags;
  //! Total count of RATiedReg's.
  uint32_t _tiedTotal;
  //! Index of RATiedReg's per register group.
  RARegIndex _tiedIndex;
  //! Count of RATiedReg's per register group.
  RARegCount _tiedCount;
  //! Number of live, and thus interfering VirtReg's at this point.
  RALiveCount _liveCount;
  //! Fixed physical registers used.
  RARegMask _usedRegs;
  //! Clobbered registers (by a function call).
  RARegMask _clobberedRegs;
  //! Tied registers.
  RATiedReg _tiedRegs[1];

  //! \}

  //! \name Construction & Destruction
  //! \{

  inline RAInst(InstRWFlags instRWFlags, RATiedFlags tiedFlags, uint32_t tiedTotal, const RARegMask& clobberedRegs) noexcept {
    _instRWFlags = instRWFlags;
    _flags = tiedFlags;
    _tiedTotal = tiedTotal;
    _tiedIndex.reset();
    _tiedCount.reset();
    _liveCount.reset();
    _usedRegs.reset();
    _clobberedRegs = clobberedRegs;
  }

  //! \}

  //! \name Accessors
  //! \{

  //! Returns instruction RW flags.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG InstRWFlags instRWFlags() const noexcept { return _instRWFlags; };

  //! Tests whether the given `flag` is present in instruction RW flags.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool hasInstRWFlag(InstRWFlags flag) const noexcept { return Support::test(_instRWFlags, flag); }

  //! Adds `flags` to instruction RW flags.
  ASMJIT_INLINE_NODEBUG void addInstRWFlags(InstRWFlags flags) noexcept { _instRWFlags |= flags; }

  //! Returns the instruction flags.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RATiedFlags flags() const noexcept { return _flags; }

  //! Tests whether the instruction has flag `flag`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool hasFlag(RATiedFlags flag) const noexcept { return Support::test(_flags, flag); }

  //! Replaces the existing instruction flags with `flags`.
  ASMJIT_INLINE_NODEBUG void setFlags(RATiedFlags flags) noexcept { _flags = flags; }

  //! Adds instruction `flags` to this RAInst.
  ASMJIT_INLINE_NODEBUG void addFlags(RATiedFlags flags) noexcept { _flags |= flags; }

  //! Clears instruction `flags` from  this RAInst.
  ASMJIT_INLINE_NODEBUG void clearFlags(RATiedFlags flags) noexcept { _flags &= ~flags; }

  //! Tests whether one operand of this instruction has been patched from Reg to Mem.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool isRegToMemPatched() const noexcept { return hasFlag(RATiedFlags::kInst_RegToMemPatched); }

  //! Tests whether this instruction can be transformed to another instruction if necessary.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool isTransformable() const noexcept { return hasFlag(RATiedFlags::kInst_IsTransformable); }

  //! Returns the associated block with this RAInst.
  // [[nodiscard]]
  // ASMJIT_INLINE_NODEBUG RABlock* block() const noexcept { return _block; }

  //! Returns tied registers (all).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RATiedReg* tiedRegs() const noexcept { return const_cast<RATiedReg*>(_tiedRegs); }

  //! Returns tied registers for a given `group`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RATiedReg* tiedRegs(RegGroup group) const noexcept { return const_cast<RATiedReg*>(_tiedRegs) + _tiedIndex.get(group); }

  //! Returns count of all tied registers.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t tiedCount() const noexcept { return _tiedTotal; }

  //! Returns count of tied registers of a given `group`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t tiedCount(RegGroup group) const noexcept { return _tiedCount[group]; }

  //! Returns `RATiedReg` at the given `index`.
  [[nodiscard]]
  inline RATiedReg* tiedAt(size_t index) const noexcept {
    ASMJIT_ASSERT(index < _tiedTotal);
    return tiedRegs() + index;
  }

  //! Returns `RATiedReg` at the given `index` of the given register `group`.
  [[nodiscard]]
  inline RATiedReg* tiedOf(RegGroup group, size_t index) const noexcept {
    ASMJIT_ASSERT(index < _tiedCount.get(group));
    return tiedRegs(group) + index;
  }

  [[nodiscard]]
  inline const RATiedReg* tiedRegForWorkReg(RegGroup group, RAWorkReg* wReg) const noexcept {
    const RATiedReg* array = tiedRegs(group);
    size_t count = tiedCount(group);

    for (size_t i = 0; i < count; i++) {
      const RATiedReg* tiedReg = &array[i];
      if (tiedReg->workReg() == wReg) {
        return tiedReg;
      }
    }

    return nullptr;
  }

  inline void setTiedAt(size_t index, RATiedReg& tied) noexcept {
    ASMJIT_ASSERT(index < _tiedTotal);
    _tiedRegs[index] = tied;
  }

  //! \name Static Functions
  //! \{

  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG size_t sizeOf(uint32_t tiedRegCount) noexcept {
    return Zone::alignedSizeOf<RAInst>() - sizeof(RATiedReg) + tiedRegCount * sizeof(RATiedReg);
  }

  //! \}
};

//! A helper class that is used to build an array of RATiedReg items that are then copied to `RAInst`.
class RAInstBuilder {
public:
  ASMJIT_NONCOPYABLE(RAInstBuilder)

  //! \name Members
  //! \{

  //! Basic block id.
  RABlockId _basicBlockId;
  //! Instruction RW flags.
  InstRWFlags _instRWFlags;

  //! Flags combined from all RATiedReg's.
  RATiedFlags _aggregatedFlags;
  //! Flags that will be cleared before storing the aggregated flags to `RAInst`.
  RATiedFlags _forbiddenFlags;
  RARegCount _count;
  RARegsStats _stats;

  RARegMask _used;
  RARegMask _clobbered;

  //! Current tied register in `_tiedRegs`.
  RATiedReg* _cur;
  //! Array of temporary tied registers.
  RATiedReg _tiedRegs[128];

  //! \}

  //! \name Construction & Destruction
  //! \{

  ASMJIT_INLINE_NODEBUG explicit RAInstBuilder(RABlockId blockId = kBadBlockId) noexcept { reset(blockId); }

  ASMJIT_INLINE_NODEBUG void init(RABlockId blockId) noexcept { reset(blockId); }
  ASMJIT_INLINE_NODEBUG void reset(RABlockId blockId) noexcept {
    _basicBlockId = blockId;
    _instRWFlags = InstRWFlags::kNone;
    _aggregatedFlags = RATiedFlags::kNone;
    _forbiddenFlags = RATiedFlags::kNone;
    _count.reset();
    _stats.reset();
    _used.reset();
    _clobbered.reset();
    _cur = _tiedRegs;
  }

  //! \}

  //! \name Accessors
  //! \{

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG InstRWFlags instRWFlags() const noexcept { return _instRWFlags; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool hasInstRWFlag(InstRWFlags flag) const noexcept { return Support::test(_instRWFlags, flag); }

  ASMJIT_INLINE_NODEBUG void addInstRWFlags(InstRWFlags flags) noexcept { _instRWFlags |= flags; }

  ASMJIT_INLINE_NODEBUG void clearInstRWFlags(InstRWFlags flags) noexcept { _instRWFlags &= ~flags; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RATiedFlags aggregatedFlags() const noexcept { return _aggregatedFlags; }

  ASMJIT_INLINE_NODEBUG void addAggregatedFlags(RATiedFlags flags) noexcept { _aggregatedFlags |= flags; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RATiedFlags forbiddenFlags() const noexcept { return _forbiddenFlags; }

  ASMJIT_INLINE_NODEBUG void addForbiddenFlags(RATiedFlags flags) noexcept { _forbiddenFlags |= flags; }

  //! Returns the number of tied registers added to the builder.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t tiedRegCount() const noexcept { return uint32_t((size_t)(_cur - _tiedRegs)); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RATiedReg* begin() noexcept { return _tiedRegs; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RATiedReg* end() noexcept { return _cur; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const RATiedReg* begin() const noexcept { return _tiedRegs; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const RATiedReg* end() const noexcept { return _cur; }

  //! Returns `RATiedReg` at the given `index`.
  [[nodiscard]]
  inline RATiedReg* operator[](size_t index) noexcept {
    ASMJIT_ASSERT(index < tiedRegCount());
    return &_tiedRegs[index];
  }

  //! Returns `RATiedReg` at the given `index`. (const).
  [[nodiscard]]
  inline const RATiedReg* operator[](size_t index) const noexcept {
    ASMJIT_ASSERT(index < tiedRegCount());
    return &_tiedRegs[index];
  }

  //! \}

  //! \name Utilities
  //! \{

  [[nodiscard]]
  Error add(
    RAWorkReg* workReg,
    RATiedFlags flags,
    RegMask useRegMask, uint32_t useId, uint32_t useRewriteMask,
    RegMask outRegMask, uint32_t outId, uint32_t outRewriteMask,
    uint32_t rmSize = 0,
    RAWorkReg* consecutiveParent = nullptr
  ) noexcept {
    RegGroup group = workReg->group();
    RATiedReg* tiedReg = workReg->tiedReg();

    if (useId != Reg::kIdBad) {
      _stats.makeFixed(group);
      _used[group] |= Support::bitMask<RegMask>(useId);
      flags |= RATiedFlags::kUseFixed;
    }

    if (outId != Reg::kIdBad) {
      _clobbered[group] |= Support::bitMask<RegMask>(outId);
      flags |= RATiedFlags::kOutFixed;
    }

    _aggregatedFlags |= flags;
    _stats.makeUsed(group);

    if (!tiedReg) {
      // Would happen when the builder is not reset properly after each instruction - so catch that!
      ASMJIT_ASSERT(tiedRegCount() < ASMJIT_ARRAY_SIZE(_tiedRegs));

      tiedReg = _cur++;
      tiedReg->init(workReg, flags, useRegMask, useId, useRewriteMask, outRegMask, outId, outRewriteMask, rmSize, consecutiveParent);
      workReg->setTiedReg(tiedReg);

      _count.add(group);
      return kErrorOk;
    }
    else {
      if (consecutiveParent != tiedReg->consecutiveParent()) {
        if (tiedReg->hasConsecutiveParent()) {
          return DebugUtils::errored(kErrorInvalidState);
        }
        tiedReg->_consecutiveParent = consecutiveParent;
      }

      if (useId != Reg::kIdBad) {
        if (ASMJIT_UNLIKELY(tiedReg->hasUseId())) {
          return DebugUtils::errored(kErrorOverlappedRegs);
        }
        tiedReg->setUseId(useId);
      }

      if (outId != Reg::kIdBad) {
        if (ASMJIT_UNLIKELY(tiedReg->hasOutId())) {
          return DebugUtils::errored(kErrorOverlappedRegs);
        }
        tiedReg->setOutId(outId);
      }

      tiedReg->addRefCount();
      tiedReg->addFlags(flags);
      tiedReg->_useRegMask &= useRegMask;
      tiedReg->_useRewriteMask |= useRewriteMask;
      tiedReg->_outRegMask &= outRegMask;
      tiedReg->_outRewriteMask |= outRewriteMask;
      tiedReg->_rmSize = uint8_t(Support::max<uint32_t>(tiedReg->rmSize(), rmSize));
      return kErrorOk;
    }
  }

  [[nodiscard]]
  Error addCallArg(RAWorkReg* workReg, uint32_t useId) noexcept {
    ASMJIT_ASSERT(useId != Reg::kIdBad);

    RATiedFlags flags = RATiedFlags::kUse | RATiedFlags::kRead | RATiedFlags::kUseFixed;
    RegGroup group = workReg->group();
    RegMask allocable = Support::bitMask<RegMask>(useId);

    _aggregatedFlags |= flags;
    _used[group] |= allocable;
    _stats.makeFixed(group);
    _stats.makeUsed(group);

    RATiedReg* tiedReg = workReg->tiedReg();
    if (!tiedReg) {
      // Could happen when the builder is not reset properly after each instruction.
      ASMJIT_ASSERT(tiedRegCount() < ASMJIT_ARRAY_SIZE(_tiedRegs));

      tiedReg = _cur++;
      tiedReg->init(workReg, flags, allocable, useId, 0, allocable, Reg::kIdBad, 0);
      workReg->setTiedReg(tiedReg);

      _count.add(group);
      return kErrorOk;
    }
    else {
      if (tiedReg->hasUseId()) {
        flags |= RATiedFlags::kDuplicate;
        tiedReg->_useRegMask |= allocable;
      }
      else {
        tiedReg->setUseId(useId);
        tiedReg->_useRegMask &= allocable;
      }

      tiedReg->addRefCount();
      tiedReg->addFlags(flags);
      return kErrorOk;
    }
  }

  [[nodiscard]]
  Error addCallRet(RAWorkReg* workReg, uint32_t outId) noexcept {
    ASMJIT_ASSERT(outId != Reg::kIdBad);

    RATiedFlags flags = RATiedFlags::kOut | RATiedFlags::kWrite | RATiedFlags::kOutFixed;
    RegGroup group = workReg->group();
    RegMask outRegs = Support::bitMask<RegMask>(outId);

    _aggregatedFlags |= flags;
    _used[group] |= outRegs;
    _stats.makeFixed(group);
    _stats.makeUsed(group);

    RATiedReg* tiedReg = workReg->tiedReg();
    if (!tiedReg) {
      // Could happen when the builder is not reset properly after each instruction.
      ASMJIT_ASSERT(tiedRegCount() < ASMJIT_ARRAY_SIZE(_tiedRegs));

      tiedReg = _cur++;
      tiedReg->init(workReg, flags, Support::bit_ones<RegMask>, Reg::kIdBad, 0, outRegs, outId, 0);
      workReg->setTiedReg(tiedReg);

      _count.add(group);
      return kErrorOk;
    }
    else {
      if (tiedReg->hasOutId()) {
        return DebugUtils::errored(kErrorOverlappedRegs);
      }

      tiedReg->addRefCount();
      tiedReg->addFlags(flags);
      tiedReg->setOutId(outId);
      return kErrorOk;
    }
  }

  //! \}
};

//! \}
//! \endcond

ASMJIT_END_NAMESPACE

#endif // !ASMJIT_NO_COMPILER
#endif // ASMJIT_CORE_RAINST_P_H_INCLUDED
