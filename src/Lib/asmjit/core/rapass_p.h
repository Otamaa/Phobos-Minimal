// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_RAPASS_P_H_INCLUDED
#define ASMJIT_CORE_RAPASS_P_H_INCLUDED

#include "../core/api-config.h"
#ifndef ASMJIT_NO_COMPILER

#include "../core/compiler.h"
#include "../core/emithelper_p.h"
#include "../core/raassignment_p.h"
#include "../core/racfgblock_p.h"
#include "../core/radefs_p.h"
#include "../core/rainst_p.h"
#include "../core/rastack_p.h"
#include "../core/support_p.h"

ASMJIT_BEGIN_NAMESPACE

//! \cond INTERNAL
//! \addtogroup asmjit_ra
//! \{

//! Register allocation pass used by `BaseCompiler`.
class BaseRAPass : public Pass {
public:
  ASMJIT_NONCOPYABLE(BaseRAPass)
  using Base = Pass;

  //! \name Constants
  //! \{

  static inline constexpr uint32_t kCallArgWeight = 80;

  //! \}

  //! \name Types
  //! \{

  using PhysToWorkMap = RAAssignment::PhysToWorkMap;
  using WorkToPhysMap = RAAssignment::WorkToPhysMap;

  //! \}

  //! \name Members
  //! \{

  //! Allocator that uses zone passed to `runOnFunction()`.
  ZoneAllocator _allocator {};
  //! Emit helper.
  BaseEmitHelper* _iEmitHelper = nullptr;

  //! Logger, disabled if null.
  Logger* _logger = nullptr;
  //! Format options, copied from Logger, or zeroed if there is no logger.
  FormatOptions _formatOptions {};
  //! Diagnostic options, copied from Emitter, or zeroed if there is no logger.
  DiagnosticOptions _diagnosticOptions {};

  //! Function being processed.
  FuncNode* _func = nullptr;
  //! Stop node.
  BaseNode* _stop = nullptr;
  //! Node that is used to insert extra code after the function body.
  BaseNode* _extraBlock = nullptr;

  //! Blocks (first block is the entry, always exists).
  ZoneVector<RABlock*> _blocks {};
  //! Function exit blocks (usually one, but can contain more).
  ZoneVector<RABlock*> _exits {};
  //! Post order view (POV).
  ZoneVector<RABlock*> _pov {};

  //! Number of instruction nodes.
  uint32_t _instructionCount = 0;
  //! Number of created blocks (internal).
  uint32_t _createdBlockCount = 0;

  //! Shared assignment blocks.
  ZoneVector<RASharedAssignment> _sharedAssignments {};

  //! Timestamp generator (incremental).
  mutable uint64_t _lastTimestamp = 0;

  //! Architecture traits.
  const ArchTraits* _archTraits = nullptr;
  //! Index to physical registers in `RAAssignment::PhysToWorkMap`.
  RARegIndex _physRegIndex = RARegIndex();
  //! Count of physical registers in `RAAssignment::PhysToWorkMap`.
  RARegCount _physRegCount = RARegCount();
  //! Total number of physical registers.
  uint32_t _physRegTotal = 0;
  //! Indexes of a possible scratch registers that can be selected if necessary.
  Support::Array<uint8_t, 2> _scratchRegIndexes {};

  //! Registers available for allocation.
  RARegMask _availableRegs = RARegMask();
  //! Registers clobbered by the function.
  RARegMask _clobberedRegs = RARegMask();

  //! Work registers (registers used by the function).
  ZoneVector<RAWorkReg*> _workRegs;
  //! Work registers per register group.
  Support::Array<ZoneVector<RAWorkReg*>, Globals::kNumVirtGroups> _workRegsOfGroup;

  //! Count of work registers that live across multiple basic blocks.
  uint32_t _multiWorkRegCount = 0u;
  //! Count of work registers, incremented before allocating _workRegs array.
  uint32_t _totalWorkRegCount = 0u;

  //! Register allocation strategy per register group.
  Support::Array<RAStrategy, Globals::kNumVirtGroups> _strategy;
  //! Global max live-count (from all blocks) per register group.
  RALiveCount _globalMaxLiveCount = RALiveCount();
  //! Global live spans per register group.
  Support::Array<RALiveSpans*, Globals::kNumVirtGroups> _globalLiveSpans {};
  //! Temporary stack slot.
  Operand _temporaryMem = Operand();

  //! Stack pointer.
  Reg _sp = Reg();
  //! Frame pointer.
  Reg _fp = Reg();
  //! Stack manager.
  RAStackAllocator _stackAllocator {};
  //! Function arguments assignment.
  FuncArgsAssignment _argsAssignment {};
  //! Some StackArgs have to be assigned to StackSlots.
  uint32_t _numStackArgsToStackSlots = 0;

  //! Maximum name-size computed from all WorkRegs.
  uint32_t _maxWorkRegNameSize = 0;
  //! Temporary string builder used to format comments.
  StringTmp<192> _tmpString;

  //! \}

  //! \name Construction & Destruction
  //! \{

  BaseRAPass(BaseCompiler& cc) noexcept;
  ~BaseRAPass() noexcept override;

  //! \}

  //! \name Accessors
  //! \{

  //! Returns the associated `BaseCompiler`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG BaseCompiler& cc() const noexcept { return static_cast<BaseCompiler&>(_cb); }

  //! Returns \ref Logger passed to \ref run().
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Logger* logger() const noexcept { return _logger; }

  //! Returns either a valid logger if the given `option` is set and logging is enabled, or nullptr.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Logger* getLoggerIf(DiagnosticOptions option) const noexcept { return Support::test(_diagnosticOptions, option) ? _logger : nullptr; }

  //! Returns whether the diagnostic `option` is enabled.
  //!
  //! \note Returns false if there is no logger (as diagnostics without logging make no sense).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool hasDiagnosticOption(DiagnosticOptions option) const noexcept { return Support::test(_diagnosticOptions, option); }

  //! Returns \ref Zone passed to \ref run().
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Zone* zone() const noexcept { return _allocator.zone(); }

  //! Returns \ref ZoneAllocator used by the register allocator.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG ZoneAllocator* allocator() const noexcept { return const_cast<ZoneAllocator*>(&_allocator); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<RASharedAssignment> sharedAssignments() const { return _sharedAssignments.as_span(); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t sharedAssignmentCount() const noexcept { return _sharedAssignments.size(); }

  //! Returns the current function node.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG FuncNode* func() const noexcept { return _func; }

  //! Returns the stop of the current function.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG BaseNode* stop() const noexcept { return _stop; }

  //! Returns an extra block used by the current function being processed.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG BaseNode* extraBlock() const noexcept { return _extraBlock; }

  //! Sets an extra block, see `extraBlock()`.
  ASMJIT_INLINE_NODEBUG void setExtraBlock(BaseNode* node) noexcept { _extraBlock = node; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t endPosition() const noexcept { return _instructionCount * 2u; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const RARegMask& availableRegs() const noexcept { return _availableRegs; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const RARegMask& clobberedRegs() const noexcept { return _clobberedRegs; }

  //! \}

  //! \name Utilities
  //! \{

  inline void makeUnavailable(RegGroup group, uint32_t regId) noexcept {
    _availableRegs[group] &= ~Support::bitMask<RegMask>(regId);
  }

  inline void makeUnavailable(const RARegMask::RegMasks& regs) noexcept {
    _availableRegs.clear(regs);
  }

  //! \}

  //! \name Run
  //! \{

  Error run(Zone* zone, Logger* logger) override;

  //! Runs the register allocator for the given `func`.
  Error runOnFunction(Zone* zone, FuncNode* func, bool last) noexcept;

  //! Performs all allocation steps sequentially, called by `runOnFunction()`.
  Error onPerformAllSteps() noexcept;

  //! \}

  //! \name Events
  //! \{

  //! Called by \ref runOnFunction() before the register allocation to initialize
  //! architecture-specific data and constraints.
  virtual void onInit() noexcept;

  //! Called by \ref runOnFunction(` after register allocation to clean everything
  //! up. Called even if the register allocation failed.
  virtual void onDone() noexcept;

  //! \}

  //! \name CFG - Basic-Block Management
  //! \{

  //! Returns the function's entry block.
  [[nodiscard]]
  inline RABlock* entryBlock() noexcept {
    ASMJIT_ASSERT(!_blocks.empty());
    return _blocks[0];
  }

  //! \overload
  [[nodiscard]]
  inline const RABlock* entryBlock() const noexcept {
    ASMJIT_ASSERT(!_blocks.empty());
    return _blocks[0];
  }

  //! Returns all basic blocks of this function.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<RABlock*> blocks() noexcept { return _blocks.as_span(); }

  //! Returns the count of basic blocks (returns size of `_blocks` array).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t blockCount() const noexcept { return _blocks.size(); }

  //! Returns the count of reachable basic blocks (returns size of `_pov` array).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t reachableBlockCount() const noexcept { return _pov.size(); }

  //! Tests whether the CFG has dangling blocks - these were created by `newBlock()`, but not added to CFG through
  //! `addBlocks()`. If `true` is returned and the  CFG is constructed it means that something is missing and it's
  //! incomplete.
  //!
  //! \note This is only used to check if the number of created blocks matches the number of added blocks.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool hasDanglingBlocks() const noexcept { return _createdBlockCount != blockCount(); }

  //! Gest a next timestamp to be used to mark CFG blocks.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RABlockTimestamp nextTimestamp() const noexcept { return RABlockTimestamp(++_lastTimestamp); }

  //! Creates a new `RABlock` instance.
  //!
  //! \note New blocks don't have ID assigned until they are added to the block array by calling `addBlock()`.
  [[nodiscard]]
  RABlock* newBlock(BaseNode* initialNode = nullptr) noexcept;

  //! Tries to find a neighboring LabelNode (without going through code) that is already connected with `RABlock`.
  //! If no label is found then a new RABlock is created and assigned to all possible labels in a backward direction.
  [[nodiscard]]
  RABlock* newBlockOrExistingAt(LabelNode* cbLabel, BaseNode** stoppedAt = nullptr) noexcept;

  //! Adds the given `block` to the block list and assign it a unique block id.
  [[nodiscard]]
  Error addBlock(RABlock* block) noexcept;

  [[nodiscard]]
  inline Error addExitBlock(RABlock* block) noexcept {
    block->addFlags(RABlockFlags::kIsFuncExit);
    return _exits.append(allocator(), block);
  }

  [[nodiscard]]
  ASMJIT_INLINE RAInst* newRAInst(InstRWFlags instRWFlags, RATiedFlags flags, uint32_t tiedRegCount, const RARegMask& clobberedRegs) noexcept {
    void* p = zone()->alloc(RAInst::sizeOf(tiedRegCount));
    if (ASMJIT_UNLIKELY(!p)) {
      return nullptr;
    }
    return new(Support::PlacementNew{p}) RAInst(instRWFlags, flags, tiedRegCount, clobberedRegs);
  }

  [[nodiscard]]
  ASMJIT_INLINE Error assignRAInst(BaseNode* node, RABlock* block, RAInstBuilder& ib) noexcept {
    RABlockId blockId = block->blockId();
    uint32_t tiedRegCount = ib.tiedRegCount();
    RAInst* raInst = newRAInst(ib.instRWFlags(), ib.aggregatedFlags(), tiedRegCount, ib._clobbered);

    if (ASMJIT_UNLIKELY(!raInst)) {
      return DebugUtils::errored(kErrorOutOfMemory);
    }

    RARegIndex index;
    RATiedFlags flagsFilter = ~ib.forbiddenFlags();

    index.buildIndexes(ib._count);
    raInst->_tiedIndex = index;
    raInst->_tiedCount = ib._count;

    for (uint32_t i = 0; i < tiedRegCount; i++) {
      RATiedReg* tiedReg = ib[i];
      RAWorkReg* workReg = tiedReg->workReg();

      RegGroup group = workReg->group();
      workReg->resetTiedReg();

      if (workReg->_singleBasicBlockId != blockId) {
        if (Support::bool_or(workReg->_singleBasicBlockId != kBadBlockId, !tiedReg->isWriteOnly())) {
          workReg->addFlags(RAWorkRegFlags::kMultiBlockUse);
        }

        workReg->_singleBasicBlockId = blockId;
        tiedReg->addFlags(RATiedFlags::kFirst);
      }

      if (tiedReg->hasUseId()) {
        block->addFlags(RABlockFlags::kHasFixedRegs);
        raInst->_usedRegs[group] |= Support::bitMask<RegMask>(tiedReg->useId());
      }

      if (tiedReg->hasOutId()) {
        block->addFlags(RABlockFlags::kHasFixedRegs);
      }

      RATiedReg& dst = raInst->_tiedRegs[index[group]++];
      dst = *tiedReg;
      dst._flags &= flagsFilter;

      if (!tiedReg->isDuplicate()) {
        dst._useRegMask &= ~ib._used[group];
      }
    }

    node->setPassData<RAInst>(raInst);
    return kErrorOk;
  }

  //! \}

  //! \name CFG - Build CFG
  //! \{

  //! Traverse the whole function and do the following:
  //!
  //!   1. Construct CFG (represented by `RABlock`) by populating `_blocks` and `_exits`. Blocks describe the control
  //!      flow of the function and contain some additional information that is used by the register allocator.
  //!
  //!   2. Remove unreachable code immediately. This is not strictly necessary for BaseCompiler itself as the register
  //!      allocator cannot reach such nodes, but keeping instructions that use virtual registers would fail during
  //!      instruction encoding phase (Assembler).
  //!
  //!   3. `RAInst` is created for each `InstNode` or compatible. It contains information that is essential for further
  //!      analysis and register allocation.
  //!
  //! Use `RACFGBuilderT` template that provides the necessary boilerplate.
  [[nodiscard]]
  virtual Error buildCFG() noexcept;

  //! Called after the CFG is built.
  [[nodiscard]]
  Error initSharedAssignments(Span<uint32_t> sharedAssignmentsMap) noexcept;

  //! \}

  //! \name CFG - Views Order
  //! \{

  //! Constructs CFG views (only POV at the moment).
  [[nodiscard]]
  Error buildCFGViews() noexcept;

  //! \}

  //! \name CFG - Dominators
  //! \{

  // Terminology:
  //   - A node `X` dominates a node `Z` if any path from the entry point to `Z` has to go through `X`.
  //   - A node `Z` post-dominates a node `X` if any path from `X` to the end of the graph has to go through `Z`.

  //! Constructs a dominator-tree from CFG.
  [[nodiscard]]
  Error buildCFGDominators() noexcept;

  [[nodiscard]]
  bool _strictlyDominates(const RABlock* a, const RABlock* b) const noexcept;

  [[nodiscard]]
  const RABlock* _nearestCommonDominator(const RABlock* a, const RABlock* b) const noexcept;

  //! Tests whether the basic block `a` dominates `b` - non-strict, returns true when `a == b`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool dominates(const RABlock* a, const RABlock* b) const noexcept { return a == b ? true : _strictlyDominates(a, b); }

  //! Tests whether the basic block `a` dominates `b` - strict dominance check, returns false when `a == b`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool strictlyDominates(const RABlock* a, const RABlock* b) const noexcept { return a == b ? false : _strictlyDominates(a, b); }

  //! Returns a nearest common dominator of `a` and `b`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RABlock* nearestCommonDominator(RABlock* a, RABlock* b) const noexcept { return const_cast<RABlock*>(_nearestCommonDominator(a, b)); }

  //! Returns a nearest common dominator of `a` and `b` (const).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const RABlock* nearestCommonDominator(const RABlock* a, const RABlock* b) const noexcept { return _nearestCommonDominator(a, b); }

  //! \}

  //! \name CFG - Utilities
  //! \{

  [[nodiscard]]
  Error removeUnreachableCode() noexcept;

  //! Returns `node` or some node after that is ideal for beginning a new block. This function is mostly used after
  //! a conditional or unconditional jump to select the successor node. In some cases the next node could be a label,
  //! which means it could have assigned some block already.
  [[nodiscard]]
  BaseNode* findSuccessorStartingAt(BaseNode* node) noexcept;

  //! Returns `true` of the `node` can flow to `target` without reaching code nor data. It's used to eliminate jumps
  //! to labels that are next right to them.
  [[nodiscard]]
  bool isNextTo(BaseNode* node, BaseNode* target) noexcept;

  //! \}

  //! \name Virtual Register Management
  //! \{

  //! Returns a native size of the general-purpose register of the target architecture.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t registerSize() const noexcept { return _sp.size(); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RAWorkReg* workRegById(RAWorkId workId) const noexcept { return _workRegs[uint32_t(workId)]; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG ZoneVector<RAWorkReg*>& workRegs() noexcept { return _workRegs; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG ZoneVector<RAWorkReg*>& workRegs(RegGroup group) noexcept { return _workRegsOfGroup[group]; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const ZoneVector<RAWorkReg*>& workRegs() const noexcept { return _workRegs; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const ZoneVector<RAWorkReg*>& workRegs(RegGroup group) const noexcept { return _workRegsOfGroup[group]; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t multiWorkRegCount() const noexcept { return _multiWorkRegCount; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t workRegCount() const noexcept { return _totalWorkRegCount; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t workRegCount(RegGroup group) const noexcept { return _workRegsOfGroup[group].size(); }

  inline void _buildPhysIndex() noexcept {
    _physRegIndex.buildIndexes(_physRegCount);
    _physRegTotal = uint32_t(_physRegIndex[RegGroup::kMaxVirt]) +
                    uint32_t(_physRegCount[RegGroup::kMaxVirt]) ;
  }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t physRegIndex(RegGroup group) const noexcept { return _physRegIndex[group]; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t physRegTotal() const noexcept { return _physRegTotal; }

  [[nodiscard]]
  Error _asWorkReg(RAWorkReg** out, VirtReg* vReg) noexcept;

  //! Creates `RAWorkReg` data for the given `vReg`. The function does nothing
  //! if `vReg` already contains link to `RAWorkReg`. Called by `constructBlocks()`.
  [[nodiscard]]
  ASMJIT_INLINE Error asWorkReg(RAWorkReg** out, VirtReg* vReg) noexcept {
    RAWorkReg* wReg = vReg->workReg();
    if (ASMJIT_LIKELY(wReg)) {
      *out = wReg;
      return kErrorOk;
    }
    else {
      return _asWorkReg(out, vReg);
    }
  }

  [[nodiscard]]
  ASMJIT_INLINE Error virtIndexAsWorkReg(RAWorkReg** out, uint32_t vIndex) noexcept {
    Span<VirtReg*> virtRegs = cc().virtRegs();
    if (ASMJIT_UNLIKELY(vIndex >= virtRegs.size()))
      return DebugUtils::errored(kErrorInvalidVirtId);
    return asWorkReg(out, virtRegs[vIndex]);
  }

  [[nodiscard]]
  RAStackSlot* _createStackSlot(RAWorkReg* workReg) noexcept;

  [[nodiscard]]
  inline RAStackSlot* getOrCreateStackSlot(RAWorkReg* workReg) noexcept {
    RAStackSlot* slot = workReg->stackSlot();
    return slot ? slot : _createStackSlot(workReg);
  }

  [[nodiscard]]
  inline BaseMem workRegAsMem(RAWorkReg* workReg) noexcept {
    (void)getOrCreateStackSlot(workReg);
    return BaseMem(OperandSignature::fromOpType(OperandType::kMem) |
                   OperandSignature::fromMemBaseType(_sp.regType()) |
                   OperandSignature::fromBits(OperandSignature::kMemRegHomeFlag),
                   workReg->vRegId(), 0, 0);
  }

  [[nodiscard]]
  WorkToPhysMap* newWorkToPhysMap() noexcept;

  [[nodiscard]]
  PhysToWorkMap* newPhysToWorkMap() noexcept;

  [[nodiscard]]
  inline PhysToWorkMap* clonePhysToWorkMap(const PhysToWorkMap* map) noexcept {
    return static_cast<PhysToWorkMap*>(zone()->dup(map, PhysToWorkMap::sizeOf(_physRegTotal)));
  }

  [[nodiscard]]
  Error buildRegIds() noexcept;

  //! \name Liveness Analysis & Statistics
  //! \{

  //! 1. Calculates GEN/KILL/IN/OUT of each block.
  //! 2. Calculates live spans and basic statistics of each work register.
  [[nodiscard]]
  Error buildLiveness() noexcept;

  //! Assigns argIndex to WorkRegs. Must be called after the liveness analysis
  //! finishes as it checks whether the argument is live upon entry.
  [[nodiscard]]
  Error assignArgIndexToWorkRegs() noexcept;

  //! \}

  //! \name Register Allocation - Global
  //! \{

  //! Runs a global register allocator.
  [[nodiscard]]
  Error runGlobalAllocator() noexcept;

  //! Initializes data structures used for global live spans.
  [[nodiscard]]
  Error initGlobalLiveSpans() noexcept;

  [[nodiscard]]
  Error binPack(RegGroup group) noexcept;

  //! \}

  //! \name Register Allocation - Local
  //! \{

  //! Runs a local register allocator.
  [[nodiscard]]
  Error runLocalAllocator() noexcept;

  [[nodiscard]]
  Error setBlockEntryAssignment(RABlock* block, const RABlock* fromBlock, const RAAssignment& fromAssignment) noexcept;

  [[nodiscard]]
  Error setSharedAssignment(uint32_t sharedAssignmentId, const RAAssignment& fromAssignment) noexcept;

  //! Called after the RA assignment has been assigned to a block.
  //!
  //! This cannot change the assignment, but can examine it.
  [[nodiscard]]
  Error blockEntryAssigned(const PhysToWorkMap* physToWorkMap) noexcept;

  //! \}

  //! \name Register Allocation Utilities
  //! \{

  [[nodiscard]]
  Error useTemporaryMem(BaseMem& out, uint32_t size, uint32_t alignment) noexcept;

  //! \}

  //! \name Function Prolog & Epilog
  //! \{

  [[nodiscard]]
  virtual Error updateStackFrame() noexcept;

  [[nodiscard]]
  Error _markStackArgsToKeep() noexcept;

  [[nodiscard]]
  Error _updateStackArgs() noexcept;

  [[nodiscard]]
  Error insertPrologEpilog() noexcept;

  //! \}

  //! \name Instruction Rewriter
  //! \{

  [[nodiscard]]
  virtual Error rewrite() noexcept;

  //! \}

#ifndef ASMJIT_NO_LOGGING
  //! \name Logging
  //! \{

  Error annotateCode() noexcept;
  Error _dumpBlockIds(String& sb, Span<RABlock*> blocks) noexcept;
  Error _dumpBlockLiveness(String& sb, const RABlock* block) noexcept;
  Error _dumpLiveSpans(String& sb) noexcept;

  //! \}
#endif

  //! \name Emit
  //! \{

  [[nodiscard]]
  virtual Error emitMove(RAWorkReg* wReg, uint32_t dstPhysId, uint32_t srcPhysId) noexcept;

  [[nodiscard]]
  virtual Error emitSwap(RAWorkReg* aReg, uint32_t aPhysId, RAWorkReg* bReg, uint32_t bPhysId) noexcept;

  [[nodiscard]]
  virtual Error emitLoad(RAWorkReg* wReg, uint32_t dstPhysId) noexcept;

  [[nodiscard]]
  virtual Error emitSave(RAWorkReg* wReg, uint32_t srcPhysId) noexcept;

  [[nodiscard]]
  virtual Error emitJump(const Label& label) noexcept;

  [[nodiscard]]
  virtual Error emitPreCall(InvokeNode* invokeNode) noexcept;

  //! \}
};

// Late implementation of RABlock member functions:
inline ZoneAllocator* RABlock::allocator() const noexcept { return _ra->allocator(); }

inline RegMask RABlock::entryScratchGpRegs() const noexcept {
  RegMask regs = _entryScratchGpRegs;
  if (hasSharedAssignmentId()) {
    regs = _ra->_sharedAssignments[_sharedAssignmentId].entryScratchGpRegs();
  }
  return regs;
}

//! \}
//! \endcond

ASMJIT_END_NAMESPACE

#endif // !ASMJIT_NO_COMPILER
#endif // ASMJIT_CORE_RAPASS_P_H_INCLUDED
