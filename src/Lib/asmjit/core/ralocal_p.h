// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_RALOCAL_P_H_INCLUDED
#define ASMJIT_CORE_RALOCAL_P_H_INCLUDED

#include "../core/api-config.h"
#ifndef ASMJIT_NO_COMPILER

#include "../core/raassignment_p.h"
#include "../core/radefs_p.h"
#include "../core/rainst_p.h"
#include "../core/rapass_p.h"
#include "../core/support.h"

ASMJIT_BEGIN_NAMESPACE

//! \cond INTERNAL
//! \addtogroup asmjit_ra
//! \{

//! Local register allocator.
class RALocalAllocator {
public:
  ASMJIT_NONCOPYABLE(RALocalAllocator)

  using PhysToWorkMap = RAAssignment::PhysToWorkMap;
  using WorkToPhysMap = RAAssignment::WorkToPhysMap;

  //! Link to `BaseRAPass`.
  BaseRAPass& _pass;
  //! Link to `BaseCompiler`.
  BaseCompiler& _cc;

  //! Architecture traits.
  const ArchTraits* _archTraits {};
  //! Registers available to the allocator.
  RARegMask _availableRegs {};
  //! Registers clobbered by the allocator.
  RARegMask _clobberedRegs {};
  //! Registers that must be preserved by the function (clobbering means saving & restoring in function prolog & epilog).
  RARegMask _funcPreservedRegs {};

  //! Register assignment (current).
  RAAssignment _curAssignment {};
  //! Register assignment used temporarily during assignment switches.
  RAAssignment _tmpAssignment {};

  //! Link to the current `RABlock`.
  RABlock* _block {};
  //! InstNode.
  InstNode* _node {};
  //! RA instruction.
  RAInst* _raInst {};

  //! Count of all TiedReg's.
  uint32_t _tiedTotal {};
  //! TiedReg's total counter.
  RARegCount _tiedCount {};

  //! Temporary workToPhysMap that can be used freely by the allocator.
  WorkToPhysMap* _tmpWorkToPhysMap {};

  //! \name Construction & Destruction
  //! \{

  inline explicit RALocalAllocator(BaseRAPass& pass) noexcept
    : _pass(pass),
      _cc(pass.cc()),
      _archTraits(pass._archTraits),
      _availableRegs(pass._availableRegs) {
    _funcPreservedRegs.init(pass.func()->frame().preservedRegs());
  }

  Error init() noexcept;

  //! \}

  //! \name Accessors
  //! \{

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RAWorkReg* workRegById(RAWorkId workId) const noexcept { return _pass.workRegById(workId); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG PhysToWorkMap* physToWorkMap() const noexcept { return _curAssignment.physToWorkMap(); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG WorkToPhysMap* workToPhysMap() const noexcept { return _curAssignment.workToPhysMap(); }

  //! Returns the currently processed block.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RABlock* block() const noexcept { return _block; }

  //! Sets the currently processed block.
  ASMJIT_INLINE_NODEBUG void setBlock(RABlock* block) noexcept { _block = block; }

  //! Returns the currently processed `InstNode`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG InstNode* node() const noexcept { return _node; }

  //! Returns the currently processed `RAInst`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RAInst* raInst() const noexcept { return _raInst; }

  //! Returns all tied regs as `RATiedReg` array.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RATiedReg* tiedRegs() const noexcept { return _raInst->tiedRegs(); }

  //! Returns tied registers grouped by the given `group`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RATiedReg* tiedRegs(RegGroup group) const noexcept { return _raInst->tiedRegs(group); }

  //! Returns count of all TiedRegs used by the instruction.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t tiedCount() const noexcept { return _tiedTotal; }

  //! Returns count of TiedRegs used by the given register `group`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t tiedCount(RegGroup group) const noexcept { return _tiedCount.get(group); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool isGroupUsed(RegGroup group) const noexcept { return _tiedCount[group] != 0; }

  //! \}

  //! \name Assignment
  //! \{

  [[nodiscard]]
  Error makeInitialAssignment() noexcept;

  [[nodiscard]]
  Error replaceAssignment(const PhysToWorkMap* physToWorkMap) noexcept;

  //! Switch to the given assignment by reassigning all register and emitting code that reassigns them.
  //! This is always used to switch to a previously stored assignment.
  //!
  //! If `tryMode` is true then the final assignment doesn't have to be exactly same as specified by `dstPhysToWorkMap`
  //! and `dstWorkToPhysMap`. This mode is only used before conditional jumps that already have assignment to generate
  //! a code sequence that is always executed regardless of the flow.
  [[nodiscard]]
  Error switchToAssignment(PhysToWorkMap* dstPhysToWorkMap, Span<const BitWord> liveIn, bool dstReadOnly, bool tryMode) noexcept;

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Error spillRegsBeforeEntry(RABlock* block) noexcept {
    return spillScratchGpRegsBeforeEntry(block->entryScratchGpRegs());
  }

  [[nodiscard]]
  Error spillScratchGpRegsBeforeEntry(uint32_t scratchRegs) noexcept;

  //! \}

  //! \name Allocation
  //! \{

  [[nodiscard]]
  Error allocInst(InstNode* node) noexcept;

  [[nodiscard]]
  Error spillAfterAllocation(InstNode* node) noexcept;

  [[nodiscard]]
  Error allocBranch(InstNode* node, RABlock* target, RABlock* cont) noexcept;

  [[nodiscard]]
  Error allocJumpTable(InstNode* node, Span<RABlock*> targets, RABlock* cont) noexcept;

  //! \}

  //! \name Decision Making
  //! \{

  enum CostModel : uint32_t {
    kCostOfFrequency = 1048576,
    kCostOfDirtyFlag = kCostOfFrequency / 4
  };

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t costByFrequency(float freq) const noexcept {
    return uint32_t(int32_t(freq * float(kCostOfFrequency)));
  }

  [[nodiscard]]
  ASMJIT_INLINE uint32_t calculateSpillCost(RegGroup group, RAWorkReg* wReg, uint32_t assignedId) const noexcept {
    uint32_t cost = costByFrequency(wReg->liveStats().freq());

    if (_curAssignment.isPhysDirty(group, assignedId))
      cost += kCostOfDirtyFlag;

    return cost;
  }

  [[nodiscard]]
  ASMJIT_INLINE uint32_t pickBestSuitableRegister(RegGroup group, RegMask allocableRegs) const noexcept {
    // These are registers must be preserved by the function itself.
    RegMask preservedRegs = _funcPreservedRegs[group];

    // Reduce the set by removing preserved registers when possible.
    if (allocableRegs & ~preservedRegs) {
      allocableRegs &= ~preservedRegs;
    }

    return Support::ctz(allocableRegs);
  }

  //! Decides on register assignment.
  [[nodiscard]]
  uint32_t decideOnAssignment(RegGroup group, RAWorkReg* wReg, uint32_t assignedId, RegMask allocableRegs) const noexcept;

  //! Decides on whether to MOVE or SPILL the given WorkReg, because it's allocated in a physical register that have
  //! to be used by another WorkReg.
  //!
  //! The function must return either `RAAssignment::kPhysNone`, which means that the WorkReg of `workId` should be
  //! spilled, or a valid physical register ID, which means that the register should be moved to that physical register
  //! instead.
  [[nodiscard]]
  uint32_t decideOnReassignment(RegGroup group, RAWorkReg* wReg, uint32_t assignedId, RegMask allocableRegs, RAInst* raInst) const noexcept;

  //! Decides on best spill given a register mask `spillableRegs`
  [[nodiscard]]
  uint32_t decideOnSpillFor(RegGroup group, RAWorkReg* wReg, RegMask spillableRegs, RAWorkId* spillWorkId) const noexcept;

  //! \}

  //! \name Emit
  //! \{

  //! Assigns a register, the content of it is undefined at this point.
  [[nodiscard]]
  ASMJIT_INLINE Error _assignReg(RegGroup rg, RAWorkId wId, uint32_t physId, bool dirty) noexcept {
    _curAssignment.assign(rg, wId, physId, dirty);
    return kErrorOk;
  }

  ASMJIT_INLINE void _unassignReg(RegGroup rg, RAWorkId wId, uint32_t physId) noexcept {
    _curAssignment.unassign(rg, wId, physId);
  }

  //! Emits a load from [VirtReg/WorkReg]'s spill slot to a physical register
  //! and makes it assigned and clean.
  [[nodiscard]]
  ASMJIT_INLINE Error onLoadReg(RegGroup rg, RAWorkReg* wReg, RAWorkId wId, uint32_t physId) noexcept {
    _curAssignment.assign(rg, wId, physId, RAAssignment::kClean);
    return _pass.emitLoad(wReg, physId);
  }

  //! Emits a save a physical register to a [VirtReg/WorkReg]'s spill slot,
  //! keeps it assigned, and makes it clean.
  [[nodiscard]]
  ASMJIT_INLINE Error onSaveReg(RegGroup rg, RAWorkReg* wReg, RAWorkId wId, uint32_t physId) noexcept {
    ASMJIT_ASSERT(_curAssignment.workToPhysId(rg, wId) == physId);
    ASMJIT_ASSERT(_curAssignment.physToWorkId(rg, physId) == wId);

    _curAssignment.makeClean(rg, wId, physId);
    return _pass.emitSave(wReg, physId);
  }

  //! Emits a move between a destination and source register, and fixes the
  //! register assignment.
  [[nodiscard]]
  ASMJIT_INLINE Error onMoveReg(RegGroup rg, RAWorkReg* wReg, RAWorkId wId, uint32_t dstPhysId, uint32_t srcPhysId) noexcept {
    if (dstPhysId == srcPhysId) {
      return kErrorOk;
    }

    _curAssignment.reassign(rg, wId, dstPhysId, srcPhysId);
    return _pass.emitMove(wReg, dstPhysId, srcPhysId);
  }

  //! Spills a variable/register, saves the content to the memory-home if modified.
  [[nodiscard]]
  ASMJIT_INLINE Error onSpillReg(RegGroup rg, RAWorkReg* wReg, RAWorkId wId, uint32_t physId) noexcept {
    if (_curAssignment.isPhysDirty(rg, physId)) {
      ASMJIT_PROPAGATE(onSaveReg(rg, wReg, wId, physId));
    }
    _unassignReg(rg, wId, physId);
    return kErrorOk;
  }

  //! Emits a swap between two physical registers and fixes their assignment.
  //!
  //! \note Target must support this operation otherwise this would ASSERT.
  [[nodiscard]]
  ASMJIT_INLINE Error onSwapReg(RegGroup rg, RAWorkReg* aReg, RAWorkId aWorkId, uint32_t aPhysId, RAWorkReg* bReg, RAWorkId bWorkId, uint32_t bPhysId) noexcept {
    _curAssignment.swap(rg, aWorkId, aPhysId, bWorkId, bPhysId);
    return _pass.emitSwap(aReg, aPhysId, bReg, bPhysId);
  }

  //! \}
};

//! \}
//! \endcond

ASMJIT_END_NAMESPACE

#endif // !ASMJIT_NO_COMPILER
#endif // ASMJIT_CORE_RALOCAL_P_H_INCLUDED
