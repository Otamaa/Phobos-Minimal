// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include "../core/api-build_p.h"
#ifndef ASMJIT_NO_COMPILER

#include "../core/formatter_p.h"
#include "../core/ralocal_p.h"
#include "../core/rapass_p.h"
#include "../core/support_p.h"
#include "../core/type.h"
#include "../core/zonevector.h"

ASMJIT_BEGIN_NAMESPACE

// RABlock - Control Flow
// ======================

Error RABlock::appendSuccessor(RABlock* successor) noexcept {
  RABlock* predecessor = this;

  if (predecessor->hasSuccessor(successor)) {
    return kErrorOk;
  }

  ASMJIT_PROPAGATE(successor->_predecessors.willGrow(allocator()));
  ASMJIT_PROPAGATE(predecessor->_successors.willGrow(allocator()));

  predecessor->_successors.appendUnsafe(successor);
  successor->_predecessors.appendUnsafe(predecessor);

  return kErrorOk;
}

Error RABlock::prependSuccessor(RABlock* successor) noexcept {
  RABlock* predecessor = this;

  if (predecessor->hasSuccessor(successor)) {
    return kErrorOk;
  }

  ASMJIT_PROPAGATE(successor->_predecessors.willGrow(allocator()));
  ASMJIT_PROPAGATE(predecessor->_successors.willGrow(allocator()));

  predecessor->_successors.prependUnsafe(successor);
  successor->_predecessors.prependUnsafe(predecessor);

  return kErrorOk;
}

// BaseRAPass - Construction & Destruction
// =======================================

BaseRAPass::BaseRAPass(BaseCompiler& cc) noexcept : Pass(cc, "RAPass") {}
BaseRAPass::~BaseRAPass() noexcept {}

static void BaseRAPass_resetVirtRegData(BaseRAPass* self) noexcept {
  for (RAWorkReg* wReg : self->_workRegs) {
    VirtReg* vReg = wReg->virtReg();

    // Update the information regarding the stack of the virtual register.
    if (wReg->hasStackSlot()) {
      RAStackSlot* slot = wReg->stackSlot();
      vReg->assignStackSlot(slot->offset());
    }

    // Reset work reg association so it cannot be used by accident (RAWorkReg data will be destroyed).
    vReg->_workReg = nullptr;
  }
}

// BaseRAPass - Run Prepare & Cleanup
// ==================================

#ifndef ASMJIT_NO_LOGGING
static ASMJIT_INLINE void RAPass_prepareLogging(BaseRAPass& pass, Logger* logger) noexcept {
  DiagnosticOptions diag = pass._cb.diagnosticOptions();

  pass._logger = logger;

  if (logger) {
    pass._formatOptions = logger->options();
    pass._diagnosticOptions = diag;
  }
  else {
    pass._formatOptions.reset();
    pass._diagnosticOptions = diag & ~(DiagnosticOptions::kRADebugCFG |
                                       DiagnosticOptions::kRADebugUnreachable);
  }
}

static ASMJIT_INLINE void RAPass_cleanupLogging(BaseRAPass& pass) noexcept {
  pass._logger = nullptr;
  pass._formatOptions.reset();
  pass._diagnosticOptions = DiagnosticOptions::kNone;
}
#else
static ASMJIT_INLINE void RAPass_prepareLogging(BaseRAPass&, Logger*) noexcept {}
static ASMJIT_INLINE void RAPass_cleanupLogging(BaseRAPass&) noexcept {}
#endif

static void RAPass_prepareForFunction(BaseRAPass* pass, FuncDetail* funcDetail) noexcept {
  pass->_argsAssignment.reset(funcDetail);
  pass->_stackAllocator.reset(pass->_allocator.zone(), &pass->_allocator);
}

static void RAPass_cleanupAfterFunction(BaseRAPass* pass) noexcept {
  pass->_blocks.reset();
  pass->_exits.reset();
  pass->_pov.reset();
  pass->_instructionCount = 0;
  pass->_createdBlockCount = 0;

  pass->_sharedAssignments.reset();
  pass->_lastTimestamp = 0;

  pass->_archTraits = nullptr;
  pass->_physRegIndex.reset();
  pass->_physRegCount.reset();
  pass->_physRegTotal = 0;
  pass->_scratchRegIndexes.fill(Reg::kIdBad);

  pass->_availableRegs.reset();
  pass->_clobberedRegs.reset();

  pass->_workRegs.reset();
  pass->_workRegsOfGroup.forEach([](ZoneVector<RAWorkReg*>& regs) { regs.reset(); });
  pass->_multiWorkRegCount = 0u;
  pass->_totalWorkRegCount = 0u;

  pass->_strategy.forEach([](RAStrategy& strategy) { strategy.reset(); });
  pass->_globalLiveSpans.fill(nullptr);
  pass->_globalMaxLiveCount.reset();
  pass->_temporaryMem.reset();

  pass->_stackAllocator.reset(nullptr, nullptr);
  pass->_argsAssignment.reset(nullptr);
  pass->_numStackArgsToStackSlots = 0;
  pass->_maxWorkRegNameSize = 0;
}

// BaseRAPass - Run & RunOnFunction
// ================================

Error BaseRAPass::run(Zone* zone, Logger* logger) {
  // Find the first function node by skipping all nodes that are not of `NodeType::kFunc` type.
  // If there is no function in the whole code, we would just return early and not setup anything.
  BaseNode* node = cc().firstNode();
  for (;;) {
    if (!node) {
      // The code has no function.
      return kErrorOk;
    }

    if (node->type() == NodeType::kFunc) {
      break;
    }

    node = node->next();
  }

  Error err = kErrorOk;
  FuncNode* func = node->as<FuncNode>();

  RAPass_prepareLogging(*this, logger);
  do {
    // Try to find a second function in the code in order to know whether this function is last. Generally,
    // there are two use-cases we want to optimize for: The first is generating a function at a time and the
    // second is generating multiple functions at a time. In the first case we know we can do a little bit
    // cheaper cleanup at the end as we know we won't be running the register allocator again in this run().
    node = func->endNode();

    FuncNode* nextFunc = nullptr;
    while (node) {
      if (node->type() == NodeType::kFunc) {
        nextFunc = node->as<FuncNode>();
        break;
      }
      node = node->next();
    }

    err = runOnFunction(zone, func, nextFunc != nullptr);
    if (err != kErrorOk) {
      break;
    }

    func = nextFunc;
  } while (func);
  RAPass_cleanupLogging(*this);

  if (ASMJIT_UNLIKELY(err)) {
    return _cb.reportError(err);
  }

  return err;
}

Error BaseRAPass::runOnFunction(Zone* zone, FuncNode* func, [[maybe_unused]] bool last) noexcept {
  // Initialize all core structures to use `zone` and `func`.
  BaseNode* end = func->endNode();
  _func = func;
  _stop = end->next();
  _extraBlock = end;

  _allocator.reset(zone);
  RAPass_prepareForFunction(this, &_func->_funcDetail);

  // Initialize architecture-specific members.
  onInit();

  // Perform all allocation steps required.
  Error err = onPerformAllSteps();

  // Must be called regardless of the allocation status.
  onDone();

  // Reset possible connections introduced by the register allocator.
  BaseRAPass_resetVirtRegData(this);

  // Reset all core structures and everything that depends on the passed `Zone`.
  RAPass_cleanupAfterFunction(this);
  _allocator.reset(nullptr);

  _func = nullptr;
  _stop = nullptr;
  _extraBlock = nullptr;

  // Reset `Zone` as nothing should persist between `runOnFunction()` calls.
  zone->reset();

  // We alter the compiler cursor, because it doesn't make sense to reference it after the compilation - some
  // nodes may disappear and the old cursor can go out anyway.
  cc().setCursor(cc().lastNode());

  return err;
}

// BaseRAPass - Perform All Steps
// ==============================

Error BaseRAPass::onPerformAllSteps() noexcept {
  ASMJIT_PROPAGATE(buildCFG());
  ASMJIT_PROPAGATE(buildCFGViews());
  ASMJIT_PROPAGATE(removeUnreachableCode());
  ASMJIT_PROPAGATE(buildCFGDominators());
  ASMJIT_PROPAGATE(buildRegIds());
  ASMJIT_PROPAGATE(buildLiveness());
  ASMJIT_PROPAGATE(assignArgIndexToWorkRegs());

#ifndef ASMJIT_NO_LOGGING
  if (hasDiagnosticOption(DiagnosticOptions::kRAAnnotate)) {
    ASMJIT_PROPAGATE(annotateCode());
  }
#endif

  ASMJIT_PROPAGATE(runGlobalAllocator());
  ASMJIT_PROPAGATE(runLocalAllocator());

  ASMJIT_PROPAGATE(updateStackFrame());
  ASMJIT_PROPAGATE(insertPrologEpilog());

  ASMJIT_PROPAGATE(rewrite());

  return kErrorOk;
}

// BaseRAPass - Events
// ===================

void BaseRAPass::onInit() noexcept {}
void BaseRAPass::onDone() noexcept {}

// BaseRAPass - CFG - Basic Block Management
// =========================================

RABlock* BaseRAPass::newBlock(BaseNode* initialNode) noexcept {
  RABlock* block = zone()->newT<RABlock>(this);

  if (ASMJIT_UNLIKELY(!block)) {
    return nullptr;
  }

  block->setFirst(initialNode);
  block->setLast(initialNode);

  // Ignore return values here as we don't care if it was successful or not - this is a pre-allocation only
  // to make the default allocated block close to the block itself. In general, it's very common to have at
  // least 1 predecessor and successor, and in case of branches it's either 2 successors or predecessors in
  // case two basic blocks merge.
  (void)block->_predecessors.reserve_fit(allocator(), 2u);
  (void)block->_successors.reserve_fit(allocator(), 2u);

  _createdBlockCount++;
  return block;
}

RABlock* BaseRAPass::newBlockOrExistingAt(LabelNode* labelNode, BaseNode** stoppedAt) noexcept {
  if (labelNode->hasPassData()) {
    return labelNode->passData<RABlock>();
  }

  FuncNode* func = this->func();
  BaseNode* node = labelNode->prev();
  RABlock* block = nullptr;

  // Try to find some label, but terminate the loop on any code. We try hard to coalesce code that contains two
  // consecutive labels or a combination of non-code nodes between 2 or more labels.
  //
  // Possible cases that would share the same basic block:
  //
  //   1. Two or more consecutive labels:
  //     Label1:
  //     Label2:
  //
  //   2. Two or more labels separated by non-code nodes:
  //     Label1:
  //     ; Some comment...
  //     .align 16
  //     Label2:
  size_t nPendingLabels = 0;

  while (node) {
    if (node->type() == NodeType::kLabel) {
      // Function has a different NodeType, just make sure this was not messed up as we must never associate
      // BasicBlock with a `func` itself.
      ASMJIT_ASSERT(node != func);

      block = node->passData<RABlock>();
      if (block) {
        // Exit node has always a block associated with it. If we went here it means that `labelNode` passed
        // here is after the end of the function and cannot be merged with the function exit block.
        if (node == func->exitNode()) {
          block = nullptr;
        }
        break;
      }

      nPendingLabels++;
    }
    else if (node->type() == NodeType::kAlign) {
      // Align node is fine.
    }
    else {
      break;
    }

    node = node->prev();
  }

  if (stoppedAt)
    *stoppedAt = node;

  if (!block) {
    block = newBlock();
    if (ASMJIT_UNLIKELY(!block)) {
      return nullptr;
    }
  }

  labelNode->setPassData<RABlock>(block);
  node = labelNode;

  while (nPendingLabels) {
    node = node->prev();
    for (;;) {
      if (node->type() == NodeType::kLabel) {
        node->setPassData<RABlock>(block);
        nPendingLabels--;
        break;
      }

      node = node->prev();
      ASMJIT_ASSERT(node != nullptr);
    }
  }

  if (!block->first()) {
    block->setFirst(node);
    block->setLast(labelNode);
  }

  return block;
}

Error BaseRAPass::addBlock(RABlock* block) noexcept {
  ASMJIT_PROPAGATE(_blocks.willGrow(allocator()));

  block->_blockId = RABlockId(blockCount());
  _blocks.appendUnsafe(block);
  return kErrorOk;
}

// BaseRAPass - CFG - Build
// ========================

// [[pure virtual]]
Error BaseRAPass::buildCFG() noexcept {
  return DebugUtils::errored(kErrorInvalidState);
}

Error BaseRAPass::initSharedAssignments(Span<uint32_t> sharedAssignmentsMap) noexcept {
  if (sharedAssignmentsMap.is_empty()) {
    return kErrorOk;
  }

  uint32_t count = 0;
  for (RABlock* block : _blocks) {
    if (block->hasSharedAssignmentId()) {
      uint32_t sharedAssignmentId = sharedAssignmentsMap[block->sharedAssignmentId()];
      block->setSharedAssignmentId(sharedAssignmentId);
      count = Support::max(count, sharedAssignmentId + 1);
    }
  }

  ASMJIT_PROPAGATE(_sharedAssignments.resize_fit(allocator(), count));

  // Aggregate all entry scratch GP regs from blocks of the same assignment to the assignment itself. It will then be
  // used instead of RABlock's own scratch regs mask, as shared assignments have precedence.
  for (RABlock* block : _blocks) {
    if (block->hasJumpTable()) {
      Span<RABlock*> successors = block->successors();
      if (!successors.is_empty()) {
        RABlock* firstSuccessor = successors[0];
        // NOTE: Shared assignments connect all possible successors so we only need the first to propagate exit scratch
        // GP registers.
        if (firstSuccessor->hasSharedAssignmentId()) {
          RASharedAssignment& sa = _sharedAssignments[firstSuccessor->sharedAssignmentId()];
          sa.addEntryScratchGpRegs(block->exitScratchGpRegs());
        }
        else {
          // This is only allowed if there is a single successor - in that case shared assignment is not necessary.
          ASMJIT_ASSERT(successors.size() == 1u);
        }
      }
    }
    if (block->hasSharedAssignmentId()) {
      RASharedAssignment& sa = _sharedAssignments[block->sharedAssignmentId()];
      sa.addEntryScratchGpRegs(block->_entryScratchGpRegs);
    }
  }

  return kErrorOk;
}

// BaseRAPass - CFG - Views Order
// ==============================

// Stack specific to building a post-order-view. It reuses the POV vector in a way that stacked items
// are added from the end, which would never collide with items already added to the POV vector as
// they are added from the beginning (and the number of stacked items cannot exceed the number of
// blocks). Additionally, it needs one vector of uint32_t to store the index of successors where it
// ended before it was pushed on the stack.
class RAPOVBuilderStack {
protected:
  RABlock** _blockPtr;
  uint32_t* _indexPtr;
  uint32_t* _indexBegin;

public:
  ASMJIT_INLINE RAPOVBuilderStack(RABlock** povStack, uint32_t* indexStack, size_t blockCount) noexcept
    : _blockPtr(povStack + blockCount),
      _indexPtr(indexStack),
      _indexBegin(indexStack) {}

  ASMJIT_INLINE void push(RABlock* block, uint32_t index) noexcept {
    *--_blockPtr = block;
    *_indexPtr++ = index;
  }

  ASMJIT_INLINE void pop(RABlock*& block, uint32_t& index) noexcept {
    ASMJIT_ASSERT(_indexPtr != _indexBegin);

    block = *_blockPtr++;
    index = *--_indexPtr;
  }

  ASMJIT_INLINE_NODEBUG bool isEmpty() const noexcept { return _indexPtr == _indexBegin; }
};

Error BaseRAPass::buildCFGViews() noexcept {
#ifndef ASMJIT_NO_LOGGING
  Logger* logger = getLoggerIf(DiagnosticOptions::kRADebugCFG);
  ASMJIT_RA_LOG_FORMAT("[BuildCFGViews]\n");
#endif // !ASMJIT_NO_LOGGING

  size_t count = blockCount();
  if (ASMJIT_UNLIKELY(!count)) {
    return kErrorOk;
  }

  ZoneVector<uint32_t> indexes;

  ASMJIT_PROPAGATE(_pov.reserve_fit(allocator(), count));
  ASMJIT_PROPAGATE(indexes.reserve_fit(allocator(), count));

  RABlock** povData = _pov.data();
  size_t povIndex = 0u;

  RABlock* curBlock = _blocks[0];
  uint32_t curIndex = 0u;

  RAPOVBuilderStack stack(povData, indexes.data(), count);

  // This loop uses reachable bit in a RABlock to mark visited blocks.
  curBlock->makeReachable();
  for (;;) {
    while (curIndex < curBlock->successors().size()) {
      RABlock* child = curBlock->successors()[curIndex++];
      if (!child->isReachable()) {
        // Mark the block as reachable to prevent visiting the same block again.
        child->makeReachable();

        // Add the curBlock block to the stack, we will get back to it later.
        stack.push(curBlock, curIndex);

        // Visit the first successor.
        curBlock = child;
        curIndex = 0u;
      }
    }

    curBlock->_povIndex = uint32_t(povIndex);
    povData[povIndex++] = curBlock;

    if (stack.isEmpty()) {
      break;
    }

    stack.pop(curBlock, curIndex);
  }

  _pov._setSize(povIndex);
  indexes.release(allocator());

  ASMJIT_RA_LOG_COMPLEX({
    StringTmp<1024> sb;
    for (RABlock* block : blocks()) {
      sb.clear();
      if (block->hasSuccessors()) {
        sb.appendFormat("  #%u -> {", block->blockId());
        _dumpBlockIds(sb, block->successors());
        sb.append("}\n");
      }
      else {
        sb.appendFormat("  #%u -> {Exit}\n", block->blockId());
      }
      logger->log(sb);
    }
  });

  return kErrorOk;
}

// BaseRAPass - CFG - Dominators
// =============================

static ASMJIT_INLINE RABlock* intersectBlocks(RABlock* b1, RABlock* b2) noexcept {
  while (b1 != b2) {
    while (b2->povIndex() > b1->povIndex()) b1 = b1->iDom();
    while (b1->povIndex() > b2->povIndex()) b2 = b2->iDom();
  }
  return b1;
}

// Based on "A Simple, Fast Dominance Algorithm".
Error BaseRAPass::buildCFGDominators() noexcept {
#ifndef ASMJIT_NO_LOGGING
  Logger* logger = getLoggerIf(DiagnosticOptions::kRADebugCFG);
  ASMJIT_RA_LOG_FORMAT("[BuildCFGDominators]\n");
#endif // !ASMJIT_NO_LOGGING

  if (_blocks.empty()) {
    return kErrorOk;
  }

  RABlock* entryBlock = this->entryBlock();
  entryBlock->_idom = entryBlock;

  bool changed = true;

#ifndef ASMJIT_NO_LOGGING
  uint32_t numIters = 0;
#endif // !ASMJIT_NO_LOGGING

  while (changed) {
    changed = false;

#ifndef ASMJIT_NO_LOGGING
    numIters++;
#endif // !ASMJIT_NO_LOGGING

    for (RABlock* block : _pov.iterate_reverse()) {
      if (block == entryBlock) {
        continue;
      }

      RABlock* iDom = nullptr;
      Span<RABlock*> predecessors = block->predecessors();

      for (RABlock* p : predecessors.iterate_reverse()) {
        if (!p->iDom()) {
          continue;
        }
        iDom = !iDom ? p : intersectBlocks(iDom, p);
      }

      if (block->iDom() != iDom) {
        ASMJIT_ASSUME(iDom != nullptr);
        ASMJIT_RA_LOG_FORMAT("  IDom of #%u -> #%u\n", block->blockId(), iDom->blockId());
        block->_idom = iDom;
        changed = true;
      }
    }
  }

  ASMJIT_RA_LOG_FORMAT("  Done (%u iterations)\n", numIters);
  return kErrorOk;
}

bool BaseRAPass::_strictlyDominates(const RABlock* a, const RABlock* b) const noexcept {
  ASMJIT_ASSERT(a != nullptr); // There must be at least one block if this function is
  ASMJIT_ASSERT(b != nullptr); // called, as both `a` and `b` must be valid blocks.
  ASMJIT_ASSERT(a != b);       // Checked by `dominates()` and `strictlyDominates()`.

  // Nothing strictly dominates the entry block.
  const RABlock* entryBlock = this->entryBlock();
  if (a == entryBlock) {
    return false;
  }

  const RABlock* iDom = b->iDom();
  while (iDom != a && iDom != entryBlock) {
    iDom = iDom->iDom();
  }

  return iDom != entryBlock;
}

const RABlock* BaseRAPass::_nearestCommonDominator(const RABlock* a, const RABlock* b) const noexcept {
  ASMJIT_ASSERT(a != nullptr); // There must be at least one block if this function is
  ASMJIT_ASSERT(b != nullptr); // called, as both `a` and `b` must be valid blocks.
  ASMJIT_ASSERT(a != b);       // Checked by `dominates()` and `properlyDominates()`.

  if (a == b) {
    return a;
  }

  // If `a` strictly dominates `b` then `a` is the nearest common dominator.
  if (_strictlyDominates(a, b)) {
    return a;
  }

  // If `b` strictly dominates `a` then `b` is the nearest common dominator.
  if (_strictlyDominates(b, a)) {
    return b;
  }

  const RABlock* entryBlock = this->entryBlock();
  RABlockTimestamp timestamp = nextTimestamp();

  // Mark all A's dominators.
  const RABlock* block = a->iDom();
  while (block != entryBlock) {
    block->setTimestamp(timestamp);
    block = block->iDom();
  }

  // Check all B's dominators against marked dominators of A.
  block = b->iDom();
  while (block != entryBlock) {
    if (block->hasTimestamp(timestamp)) {
      return block;
    }
    block = block->iDom();
  }

  return entryBlock;
}

// BaseRAPass - CFG - Utilities
// ============================

Error BaseRAPass::removeUnreachableCode() noexcept {
  size_t numAllBlocks = blockCount();
  size_t numReachableBlocks = reachableBlockCount();

  // All reachable -> nothing to do.
  if (numAllBlocks == numReachableBlocks) {
    return kErrorOk;
  }

#ifndef ASMJIT_NO_LOGGING
  Logger* logger = getLoggerIf(DiagnosticOptions::kRADebugUnreachable);
  String& sb = _tmpString;
  ASMJIT_RA_LOG_FORMAT("[RemoveUnreachableCode - detected %zu of %zu unreachable blocks]\n", numAllBlocks - numReachableBlocks, numAllBlocks);
#endif

  for (RABlock* block : _blocks.iterate()) {
    if (block->isReachable()) {
      continue;
    }

    ASMJIT_RA_LOG_FORMAT("  Removing code from unreachable block {%u}\n", uint32_t(block->blockId()));
    BaseNode* first = block->first();
    BaseNode* last = block->last();

    BaseNode* beforeFirst = first->prev();
    BaseNode* afterLast = last->next();

    BaseNode* node = first;
    while (node != afterLast) {
      BaseNode* next = node->next();

      if (node->isCode() || node->isRemovable()) {
#ifndef ASMJIT_NO_LOGGING
        if (logger) {
          sb.clear();
          Formatter::formatNode(sb, _formatOptions, &_cb, node);
          logger->logf("    %s\n", sb.data());
        }
#endif
        cc().removeNode(node);
      }
      node = next;
    }

    if (beforeFirst->next() == afterLast) {
      block->setFirst(nullptr);
      block->setLast(nullptr);
    }
    else {
      block->setFirst(beforeFirst->next());
      block->setLast(afterLast->prev());
    }
  }

  return kErrorOk;
}

BaseNode* BaseRAPass::findSuccessorStartingAt(BaseNode* node) noexcept {
  while (node && (node->isInformative() || node->hasNoEffect())) {
    node = node->next();
  }
  return node;
}

bool BaseRAPass::isNextTo(BaseNode* node, BaseNode* target) noexcept {
  for (;;) {
    node = node->next();
    if (node == target) {
      return true;
    }

    if (!node) {
      return false;
    }

    if (node->isCode() || node->isData()) {
      return false;
    }
  }
}

// BaseRAPass - Registers - VirtReg / WorkReg Mapping
// ==================================================

Error BaseRAPass::_asWorkReg(RAWorkReg** out, VirtReg* vReg) noexcept {
  // Checked by `asWorkReg()` - must be true.
  ASMJIT_ASSERT(vReg->_workReg == nullptr);

  OperandSignature signature = RegUtils::signatureOf(vReg->regType());
  RegGroup group = signature.regGroup();
  ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);

  ZoneVector<RAWorkReg*>& wRegsByGroup = workRegs(group);
  ASMJIT_PROPAGATE(wRegsByGroup.willGrow(allocator()));

  RAWorkReg* wReg = zone()->newT<RAWorkReg>(vReg, signature, kBadWorkId);
  if (ASMJIT_UNLIKELY(!wReg)) {
    return DebugUtils::errored(kErrorOutOfMemory);
  }

  vReg->setWorkReg(wReg);
  if (!vReg->isStackArea()) {
    wReg->setRegByteMask(Support::lsb_mask<uint64_t>(vReg->virtSize()));
  }

  wRegsByGroup.appendUnsafe(wReg);
  _totalWorkRegCount++;

  // Only used by RA logging.
  _maxWorkRegNameSize = Support::max(_maxWorkRegNameSize, vReg->nameSize());

  *out = wReg;
  return kErrorOk;
}

RAStackSlot* BaseRAPass::_createStackSlot(RAWorkReg* workReg) noexcept {
  VirtReg* vReg = workReg->virtReg();
  RAStackSlot* slot = _stackAllocator.newSlot(_sp.id(), vReg->virtSize(), vReg->alignment(), RAStackSlot::kFlagRegHome);

  workReg->_stackSlot = slot;
  workReg->markStackUsed();

  return slot;
}

RAAssignment::WorkToPhysMap* BaseRAPass::newWorkToPhysMap() noexcept {
  size_t count = workRegCount();
  size_t size = WorkToPhysMap::sizeOf(count);

  // If no registers are used it could be zero, in that case return a dummy map instead of NULL.
  if (ASMJIT_UNLIKELY(!size)) {
    static const RAAssignment::WorkToPhysMap nullMap = {{ 0 }};
    return const_cast<RAAssignment::WorkToPhysMap*>(&nullMap);
  }

  WorkToPhysMap* map = zone()->alloc<WorkToPhysMap>(size);
  if (ASMJIT_UNLIKELY(!map)) {
    return nullptr;
  }

  map->reset(count);
  return map;
}

RAAssignment::PhysToWorkMap* BaseRAPass::newPhysToWorkMap() noexcept {
  uint32_t count = physRegTotal();
  size_t size = PhysToWorkMap::sizeOf(count);

  PhysToWorkMap* map = zone()->alloc<PhysToWorkMap>(size);
  if (ASMJIT_UNLIKELY(!map)) {
    return nullptr;
  }

  map->reset(count);
  return map;
}

// BaseRAPass - Registers - Liveness Analysis and Statistics
// =========================================================

ASMJIT_FAVOR_SPEED Error BaseRAPass::buildRegIds() noexcept {
  uint32_t count = _totalWorkRegCount;
  ASMJIT_PROPAGATE(_workRegs.reserve_fit(allocator(), count));

  RAWorkReg** wRegData = _workRegs.data();
  _workRegs._setSize(count);

  uint32_t multiId = 0u;
  uint32_t singleId = count;

  for (uint32_t rg = 0; rg < Globals::kNumVirtGroups; rg++) {
    for (RAWorkReg* wReg : _workRegsOfGroup[rg]) {
      if (wReg->isWithinSingleBasicBlock()) {
        wReg->_workId = RAWorkId(--singleId);
        wRegData[singleId] = wReg;
      }
      else {
        wReg->_workId = RAWorkId(multiId);
        wReg->_singleBasicBlockId = kBadBlockId;
        wRegData[multiId++] = wReg;
      }
    }
  }

  ASMJIT_ASSERT(singleId == multiId);
  _multiWorkRegCount = multiId;

  return kErrorOk;
}

// BaseRAPass - Registers - Liveness Analysis and Statistics
// =========================================================

template<typename BitMutator>
static ASMJIT_INLINE void BaseRAPass_calculateInitialInOut(RABlock* block, size_t numBitWords, Support::FixedStack<RABlock*>& queue, uint32_t povIndex) noexcept {
  using BitWord = Support::BitWord;

  // Calculate LIVE-OUT based on LIVE-IN of all successors and then recalculate LIVE-IN based on LIVE-OUT and KILL bits.
  Span<RABlock*> successors = block->successors();
  if (!successors.is_empty()) {
    BitMutator bLiveIn(block->liveIn());
    BitMutator bLiveOut(block->liveOut());
    BitMutator bKill(block->kill());

    for (RABlock* successor : successors.iterate_reverse()) {
      if (Support::bool_and(successor->povIndex() > povIndex, !successor->isEnqueued())) {
        successor->addFlags(RABlockFlags::kIsEnqueued);
        queue.push(successor);
        continue;
      }

      BitMutator bSuccIn(successor->liveIn());
      for (uint32_t bw = 0; bw < numBitWords; bw++) {
        BitWord sc = bSuccIn.bitWord(bw);

        BitWord in = bLiveIn.bitWord(bw) | sc;
        BitWord out = bLiveOut.bitWord(bw) | sc;

        bLiveIn.setBitWord(bw, in & ~bKill.bitWord(bw));
        bLiveOut.setBitWord(bw, out);
      }

      bLiveIn.commit(block->liveIn());
      bLiveOut.commit(block->liveOut());
    }
  }
}

// Calculate LIVE-IN and LIVE-OUT of the given `block` and a single `successor` that has changed its LIVE-IN bits.
template<typename BitMutator>
static ASMJIT_INLINE Support::BitWord BaseRAPass_recalculateInOut(RABlock* block, size_t numBitWords, RABlock* successor) noexcept {
  using BitWord = Support::BitWord;

  BitMutator bLiveIn(block->liveIn());
  BitMutator bLiveOut(block->liveOut());
  BitMutator bKill(block->kill());

  BitMutator bSuccIn(successor->liveIn());
  BitWord changed = 0u;

  for (size_t i = 0; i < numBitWords; i++) {
    BitWord succIn = bSuccIn.bitWord(i);

    BitWord in = bLiveIn.bitWord(i);
    BitWord out = bLiveOut.bitWord(i);
    BitWord kill = bKill.bitWord(i);

    BitWord newIn = (in | succIn) & ~kill;
    BitWord newOut = (out | succIn);

    bLiveIn.setBitWord(i, newIn);
    bLiveOut.setBitWord(i, newOut);

    changed |= in ^ newIn;
  }

  bLiveIn.commit(block->liveIn());
  bLiveOut.commit(block->liveOut());

  return changed;
}

template<typename BitMutator>
static ASMJIT_INLINE Error BaseRAPass_calculateInOutKill(
  BaseRAPass* pass,
  uint32_t* nUsesPerWorkReg,
  uint32_t* nOutsPerWorkReg,
  uint32_t* nInstsPerBlock,
  uint32_t& numVisitsOut
) noexcept {
  Span<RABlock*> pov = pass->_pov.as_span();

  size_t multiWorkRegCount = pass->multiWorkRegCount();
  size_t multiWorkRegCountAsBitWords = BitOps::size_in_words<BitWord>(multiWorkRegCount);

  constexpr RAWorkRegFlags kLiveFlag = RAWorkRegFlags::kSingleBlockLiveFlag;
  constexpr RAWorkRegFlags kVisitedFlag = RAWorkRegFlags::kSingleBlockVisitedFlag;

  // Calculate GEN and KILL and then initial LIVE-IN and LIVE-OUT bits.
  //
  // GEN is mapped to LIVE-IN, because it's not needed after LIVE-IN is calculated,
  // which is essentially `LIVE-IN = GEN & ~KILL` - so once we know GEN and KILL for
  // each block, calculating LIVE-IN is trivial.
  for (RABlock* block : pov.iterate()) {
    ASMJIT_PROPAGATE(block->alloc_live_bits(multiWorkRegCount));

    BaseNode* node = block->last();
    BaseNode* stop = block->first();

    BitMutator liveIn(block->liveIn()); // LIVE-IN which maps to GEN as well.
    BitMutator kill(block->kill());     // KILL only.

    RABlockId blockId = block->blockId();
    uint32_t nInsts = 0;

    for (;;) {
      if (node->isInst()) {
        InstNode* inst = node->as<InstNode>();
        RAInst* raInst = inst->passData<RAInst>();
        ASMJIT_ASSERT(raInst != nullptr);

        RATiedReg* tiedRegs = raInst->tiedRegs();
        uint32_t count = raInst->tiedCount();

        for (uint32_t j = 0; j < count; j++) {
          RATiedReg* tiedReg = &tiedRegs[j];
          RAWorkReg* workReg = tiedReg->workReg();

          RAWorkId workId = workReg->workId();

          // Update `nUses` and `nOuts`.
          nUsesPerWorkReg[uint32_t(workId)] += 1u;
          nOutsPerWorkReg[uint32_t(workId)] += uint32_t(tiedReg->isWrite());

          bool isKill = tiedReg->isWriteOnly();
          RATiedFlags tiedFlags = tiedReg->flags();

          // Mark as:
          //   KILL - if this VirtReg is killed afterwards.
          //   LAST - if this VirtReg is last in this basic block.
          if (workReg->isWithinSingleBasicBlock()) {
            bool wasKill = !Support::test(workReg->flags(), kLiveFlag);
            bool wasLast = !Support::test(workReg->flags(), kVisitedFlag);

            tiedFlags |= Support::bool_as_flag<RATiedFlags::kKill>(wasKill);
            tiedFlags |= Support::bool_as_flag<RATiedFlags::kLast>(wasLast);

            workReg->addFlags(kVisitedFlag);
            workReg->xorFlags(Support::bool_as_flag<kLiveFlag>(uint32_t(isKill ^ wasKill)));
          }
          else {
            bool wasKill = kill.bitAt(workId);
            bool wasLast = !liveIn.bitAt(workId);

            tiedFlags |= Support::bool_as_flag<RATiedFlags::kKill>(wasKill);
            tiedFlags |= Support::bool_as_flag<RATiedFlags::kLast>(wasLast);

            // KILL if the register is write only, otherwise GEN.
            liveIn.addBit(workId, !isKill);
            kill.xorBit(workId, bool(isKill ^ wasKill));
          }

          tiedReg->_flags = tiedFlags;

          if (tiedReg->isLeadConsecutive()) {
            workReg->markLeadConsecutive();
          }

          if (tiedReg->hasConsecutiveParent()) {
            RAWorkReg* consecutiveParentReg = tiedReg->consecutiveParent();
            ASMJIT_PROPAGATE(consecutiveParentReg->addImmediateConsecutive(pass->allocator(), workId));
          }
        }

        nInsts++;
      }

      if (node == stop) {
        break;
      }

      node = node->prev();
      ASMJIT_ASSERT(node != nullptr);
    }

    nInstsPerBlock[uint32_t(blockId)] = nInsts;

    // Calculate initial LIVE-IN from GEN - LIVE-IN = GEN & ~KILL.
    liveIn.clearBits(kill);

    liveIn.commit(block->liveIn());
    kill.commit(block->kill());
  }

  // Calculate initial values of LIVE-OUT and update LIVE-IN accordingly to LIVE-OUT.
  //
  // This step requires a queue, however, we only add a node's successors to the queue, which post-order-index
  // is greater than the post-order-index of the block being processed. This makes the next pass much faster to
  // converge.
  uint32_t numVisits = 0u;
  if (multiWorkRegCountAsBitWords > 0u) {
    ZoneVector<RABlock*> queueStorage;
    ASMJIT_PROPAGATE(queueStorage.reserve_fit(pass->allocator(), pov.size()));
    Support::FixedStack<RABlock*> queue(queueStorage.data(), pov.size());

    for (size_t povIndex = 0u; povIndex < pov.size(); povIndex++) {
      RABlock* block = pov[povIndex];
      BaseRAPass_calculateInitialInOut<BitMutator>(block, multiWorkRegCountAsBitWords, queue, uint32_t(povIndex));
    }

    // Iteratively keep recalculating LIVE-IN and LIVE-OUT once there are no more changes to the bits. This is
    // needed as there may be cycles in the CFG, which have to be propagated. This algorithm essentially uses a
    // work queue where nodes that change are pushed to propagate the changes to all predecessor nodes.
    while (!queue.isEmpty()) {
      numVisits++;

      RABlock* block = queue.pop();
      block->clearFlags(RABlockFlags::kIsEnqueued);

      for (RABlock* predecessor : block->predecessors()) {
        Support::BitWord changed = BaseRAPass_recalculateInOut<BitMutator>(predecessor, multiWorkRegCountAsBitWords, block);
        if (Support::bool_and(changed, !predecessor->isEnqueued())) {
          predecessor->addFlags(RABlockFlags::kIsEnqueued);
          queue.push(predecessor);
        }
      }
    }

    queueStorage.release(pass->allocator());
  }

  numVisitsOut = numVisits;
  return kErrorOk;
}

ASMJIT_FAVOR_SPEED Error BaseRAPass::buildLiveness() noexcept {
#ifndef ASMJIT_NO_LOGGING
  Logger* logger = getLoggerIf(DiagnosticOptions::kRADebugLiveness);
#endif

  ASMJIT_RA_LOG_FORMAT("[BuildLiveness]\n");

  size_t numAllBlocks = blockCount();
  size_t numWorkRegs = workRegCount();
  size_t multiWorkRegCount = _multiWorkRegCount;

  if (!numWorkRegs) {
    ASMJIT_RA_LOG_FORMAT("  Done (no virtual registers)\n");
    return kErrorOk;
  }

  ZoneVector<uint32_t> nUsesPerWorkReg; // Number of USEs of each RAWorkReg.
  ZoneVector<uint32_t> nOutsPerWorkReg; // Number of OUTs of each RAWorkReg.
  ZoneVector<uint32_t> nInstsPerBlock;  // Number of instructions of each RABlock.

  ASMJIT_PROPAGATE(nUsesPerWorkReg.resize_fit(allocator(), numWorkRegs));
  ASMJIT_PROPAGATE(nOutsPerWorkReg.resize_fit(allocator(), numWorkRegs));
  ASMJIT_PROPAGATE(nInstsPerBlock.resize_fit(allocator(), numAllBlocks));

  // Calculate GEN/KILL and then IN/OUT of Each Block
  // ------------------------------------------------

  {
    uint32_t numVisits = 0;

    if (multiWorkRegCount > 0u && multiWorkRegCount <= Support::bit_size_of<BitWord>) {
      // If the number of work registers as a mask fits into a single BitWord use a separate code-path that optimizes
      // for such case. This makes faster generating smaller code that doesn't have many virtual registers in use.
      ASMJIT_PROPAGATE(
        BaseRAPass_calculateInOutKill<Support::BitWordMutator>(
          this, nUsesPerWorkReg.data(), nOutsPerWorkReg.data(), nInstsPerBlock.data(), numVisits
        )
      );
    }
    else {
      ASMJIT_PROPAGATE(
        BaseRAPass_calculateInOutKill<Support::BitVectorMutator>(
          this, nUsesPerWorkReg.data(), nOutsPerWorkReg.data(), nInstsPerBlock.data(), numVisits
        )
      );
    }

    ASMJIT_RA_LOG_COMPLEX({
      String& sb = _tmpString;
      logger->logf("  LiveIn/Out Done (%u visits)\n", numVisits);

      for (uint32_t i = 0; i < numAllBlocks; i++) {
        RABlock* block = _blocks[i];

        ASMJIT_PROPAGATE(sb.assignFormat("  {#%u}\n", block->blockId()));
        ASMJIT_PROPAGATE(_dumpBlockLiveness(sb, block));

        logger->log(sb);
      }
    });
  }

  // Reserve the space in each `RAWorkReg` for references
  // ----------------------------------------------------

  for (uint32_t i = 0; i < numWorkRegs; i++) {
    RAWorkReg* workReg = workRegById(RAWorkId(i));
    ASMJIT_PROPAGATE(workReg->_refs.reserve_fit(allocator(), nUsesPerWorkReg[i]));
    ASMJIT_PROPAGATE(workReg->_writes.reserve_fit(allocator(), nOutsPerWorkReg[i]));
  }

  // These are not needed anymore, so release the memory now so other allocations can reuse it.
  nUsesPerWorkReg.release(allocator());
  nOutsPerWorkReg.release(allocator());

  // Assign block and instruction positions, build LiveCount and LiveSpans
  // ---------------------------------------------------------------------

  // This is a starting position, reserving [0, 1] for function arguments.
  uint32_t position = 2;

  for (uint32_t i = 0; i < numAllBlocks; i++) {
    RABlock* block = _blocks[i];
    if (!block->isReachable()) {
      continue;
    }

    BaseNode* node = block->first();
    BaseNode* stop = block->last();

    Span<const BitWord> liveOut = block->liveOut();

    uint32_t endPosition = position + nInstsPerBlock[i] * 2u;
    block->setFirstPosition(NodePosition(position));
    block->setEndPosition(NodePosition(endPosition));

    RALiveCount curLiveCount;
    RALiveCount maxLiveCount;

    // Process LIVE-IN.
    Support::BitVectorIterator<BitWord> it(block->liveIn());
    while (it.hasNext()) {
      RAWorkReg* workReg = _workRegs[uint32_t(it.next())];
      curLiveCount[workReg->group()]++;
      ASMJIT_PROPAGATE(workReg->liveSpans().openAt(allocator(), NodePosition(position), NodePosition(endPosition)));
    }

    for (;;) {
      if (node->isInst()) {
        InstNode* inst = node->as<InstNode>();
        RAInst* raInst = inst->passData<RAInst>();

        // Impossible - each processed instruction node must have an associated RAInst.
        ASMJIT_ASSERT(raInst != nullptr);

        RATiedReg* tiedRegs = raInst->tiedRegs();
        uint32_t count = raInst->tiedCount();

        inst->setPosition(NodePosition(position));
        raInst->_liveCount = curLiveCount;

        for (uint32_t j = 0; j < count; j++) {
          RATiedReg* tiedReg = &tiedRegs[j];
          RAWorkReg* workReg = tiedReg->workReg();

          RAWorkId workId = workReg->workId();

          // Create refs and writes.
          workReg->_refs.appendUnsafe(node);
          if (tiedReg->isWrite()) {
            workReg->_writes.appendUnsafe(node);
          }

          // We couldn't calculate this in previous steps, but since we know all LIVE-OUT at this point it becomes
          // trivial. If this is the last instruction that uses this `workReg` and it's not LIVE-OUT then it is a
          // KILL here.
          if (tiedReg->isLast() && (size_t(workId) >= multiWorkRegCount || !BitOps::bit_at(liveOut, workId))) {
            tiedReg->addFlags(RATiedFlags::kKill);
          }

          RALiveSpans& liveSpans = workReg->liveSpans();
          bool wasOpen;

          ASMJIT_PROPAGATE(liveSpans.openAt(
            allocator(), NodePosition(position + !tiedReg->isRead()), NodePosition(endPosition), wasOpen));

          RegGroup group = workReg->group();
          if (!wasOpen) {
            curLiveCount[group]++;
            raInst->_liveCount[group]++;
          }

          if (tiedReg->isKill()) {
            liveSpans.closeAt(NodePosition(position + !tiedReg->isRead() + 1u));
            curLiveCount[group]--;
          }

          // Update `RAWorkReg::useIdMask` and `RAWorkReg::hintRegId`.
          if (tiedReg->hasUseId()) {
            uint32_t useId = tiedReg->useId();
            workReg->addUseIdMask(Support::bitMask<RegMask>(useId));
            if (!workReg->hasHintRegId() && !Support::bit_test(raInst->_clobberedRegs[group], useId)) {
              workReg->setHintRegId(useId);
            }
          }

          if (tiedReg->useRegMask()) {
            workReg->restrictPreferredMask(tiedReg->useRegMask());
            if (workReg->isLeadConsecutive()) {
              workReg->restrictConsecutiveMask(tiedReg->useRegMask());
            }
          }

          if (tiedReg->outRegMask()) {
            workReg->restrictPreferredMask(tiedReg->outRegMask());
            if (workReg->isLeadConsecutive()) {
              workReg->restrictConsecutiveMask(tiedReg->outRegMask());
            }
          }

          // Update `RAWorkReg::clobberedSurvivalMask`.
          if (raInst->_clobberedRegs[group] && !tiedReg->isOutOrKill()) {
            workReg->addClobberSurvivalMask(raInst->_clobberedRegs[group]);
          }
        }

        if (node->isInvoke()) {
          func()->frame().updateCallStackAlignment(node->as<InvokeNode>()->detail().naturalStackAlignment());
        }

        position += 2;
        maxLiveCount.op<Support::Max>(raInst->_liveCount);
      }

      if (node == stop) {
        break;
      }

      node = node->next();
      ASMJIT_ASSERT(node != nullptr);
    }

    block->_maxLiveCount = maxLiveCount;
    _globalMaxLiveCount.op<Support::Max>(maxLiveCount);
    ASMJIT_ASSERT(NodePosition(position) == block->endPosition());
  }

  // Calculate WorkReg statistics
  // ----------------------------

  for (uint32_t i = 0; i < numWorkRegs; i++) {
    RAWorkReg* workReg = _workRegs[i];

    RALiveSpans& spans = workReg->liveSpans();
    uint32_t width = spans.width();
    float freq = width ? float(double(workReg->_refs.size()) / double(width)) : float(0);

    RALiveStats& stats = workReg->liveStats();
    stats._width = width;
    stats._freq = freq;
    stats._priority = freq + float(int(workReg->virtReg()->weight())) * 0.01f;
  }

  ASMJIT_RA_LOG_COMPLEX({
    String& sb = _tmpString;
    sb.clear();
    _dumpLiveSpans(sb);
    logger->log(sb);
  });

  nInstsPerBlock.release(allocator());
  return kErrorOk;
}

Error BaseRAPass::assignArgIndexToWorkRegs() noexcept {
  Span<const BitWord> liveIn = entryBlock()->liveIn();

  uint32_t argCount = func()->argCount();
  uint32_t multiWorkRegCount = _multiWorkRegCount;

  for (uint32_t argIndex = 0; argIndex < argCount; argIndex++) {
    for (uint32_t valueIndex = 0; valueIndex < Globals::kMaxValuePack; valueIndex++) {
      // Unassigned argument.
      const RegOnly& regArg = func()->argPack(argIndex)[valueIndex];
      if (!regArg.isReg() || !cc().isVirtIdValid(regArg.id())) {
        continue;
      }

      VirtReg* virtReg = cc().virtRegById(regArg.id());
      if (!virtReg) {
        continue;
      }

      // Unreferenced argument.
      RAWorkReg* workReg = virtReg->workReg();
      if (!workReg) {
        continue;
      }

      // Overwritten argument.
      RAWorkId workId = workReg->workId();
      if (uint32_t(workId) >= multiWorkRegCount || !BitOps::bit_at(liveIn, workId)) {
        continue;
      }

      workReg->setArgIndex(argIndex, valueIndex);
      const FuncValue& arg = func()->detail().arg(argIndex, valueIndex);

      if (arg.isReg() && RegUtils::groupOf(arg.regType()) == workReg->group()) {
        workReg->setHintRegId(arg.regId());
      }
    }
  }

  return kErrorOk;
}

// BaseRAPass - Allocation - Global
// ================================

#ifndef ASMJIT_NO_LOGGING
static void RAPass_dumpSpans(String& sb, uint32_t index, const RALiveSpans& liveSpans) noexcept {
  sb.appendFormat("  %02u: ", index);

  for (uint32_t i = 0; i < liveSpans.size(); i++) {
    const RALiveSpan& liveSpan = liveSpans[i];
    if (i) {
      sb.append(", ");
    }
    sb.appendFormat("[%u:%u]", liveSpan.a, liveSpan.b);
  }

  sb.append('\n');
}
#endif

Error BaseRAPass::runGlobalAllocator() noexcept {
  ASMJIT_PROPAGATE(initGlobalLiveSpans());

  for (RegGroup group : Support::enumerate(RegGroup::kMaxVirt)) {
    ASMJIT_PROPAGATE(binPack(group));
  }

  return kErrorOk;
}

ASMJIT_FAVOR_SPEED Error BaseRAPass::initGlobalLiveSpans() noexcept {
  for (RegGroup group : Support::enumerate(RegGroup::kMaxVirt)) {
    size_t physCount = _physRegCount[group];
    RALiveSpans* liveSpans = nullptr;

    if (physCount) {
      liveSpans = allocator()->zone()->alloc<RALiveSpans>(physCount * sizeof(RALiveSpans));
      if (ASMJIT_UNLIKELY(!liveSpans)) {
        return DebugUtils::errored(kErrorOutOfMemory);
      }

      for (size_t physId = 0; physId < physCount; physId++) {
        new(Support::PlacementNew{&liveSpans[physId]}) RALiveSpans();
      }
    }

    _globalLiveSpans[group] = liveSpans;
  }

  return kErrorOk;
}

struct RAConsecutiveReg {
  RAWorkReg* workReg;
  RAWorkReg* parentReg;
};

ASMJIT_FAVOR_SPEED Error BaseRAPass::binPack(RegGroup group) noexcept {
  if (workRegCount(group) == 0)
    return kErrorOk;

#ifndef ASMJIT_NO_LOGGING
  Logger* logger = getLoggerIf(DiagnosticOptions::kRADebugAssignment);
  String& sb = _tmpString;

  ASMJIT_RA_LOG_FORMAT("[BinPack] Available=%u (0x%08X) Count=%u RegGroup=%u\n",
    Support::popcnt(_availableRegs[group]),
    _availableRegs[group],
    workRegCount(group),
    uint32_t(group));
#endif

  uint32_t physCount = _physRegCount[group];

  ZoneVector<RAWorkReg*> workRegs;
  ZoneVector<RAConsecutiveReg> consecutiveRegs;
  RALiveSpans tmpSpans;

  ZoneVector<RAWorkReg*>& groupRegs = this->workRegs(group);
  ASMJIT_PROPAGATE(workRegs.reserve_fit(allocator(), groupRegs.size()));

  workRegs.assignUnsafe(groupRegs);
  workRegs.sort([](const RAWorkReg* a, const RAWorkReg* b) noexcept {
    return b->liveStats().priority() - a->liveStats().priority();
  });

  size_t numWorkRegs = workRegs.size();
  RegMask availableRegs = _availableRegs[group];
  RegMask preservedRegs = func()->frame().preservedRegs(group);

  // First try to pack everything that provides register-id hint as these are most likely function arguments and fixed
  // (pre-colored) virtual registers.
  if (!workRegs.empty()) {
    uint32_t dstIndex = 0;

    for (uint32_t index = 0; index < numWorkRegs; index++) {
      RAWorkReg* workReg = workRegs[index];

      if (workReg->isLeadConsecutive()) {
        ASMJIT_PROPAGATE(consecutiveRegs.append(allocator(), RAConsecutiveReg{workReg, nullptr}));
        workReg->markProcessedConsecutive();
      }

      if (workReg->hasHintRegId()) {
        uint32_t physId = workReg->hintRegId();
        if (Support::bit_test(availableRegs, physId)) {
          RALiveSpans& live = _globalLiveSpans[group][physId];
          Error err = tmpSpans.nonOverlappingUnionOf(allocator(), live, workReg->liveSpans());

          if (err == kErrorOk) {
            live.swap(tmpSpans);
            workReg->setHomeRegId(physId);
            workReg->markAllocated();
            continue;
          }

          if (err != 0xFFFFFFFFu) {
            return err;
          }
        }
      }

      workRegs[dstIndex++] = workReg;
    }

    workRegs._setSize(dstIndex);
    numWorkRegs = dstIndex;
  }

  // Allocate consecutive registers - both leads and all consecutives. This is important and prioritized over the rest,
  // because once a lead is allocated we really need to allocate its consecutives, otherwise we may bin pack other
  // registers into their places, which would result in wrong hints to the local allocator, and then into many moves
  // or spills.
  if (!consecutiveRegs.empty()) {
    // This loop appends all other consecutive registers into `consecutiveRegs` array. Leads are at the beginning,
    // non-leads follow.
    for (size_t i = 0;;) {
      size_t stop = consecutiveRegs.size();
      if (i == stop) {
        break;
      }

      while (i < stop) {
        RAWorkReg* workReg = consecutiveRegs[i].workReg;
        if (workReg->hasImmediateConsecutives()) {
          ZoneBitVector::ForEachBitSet it(workReg->immediateConsecutives());
          while (it.hasNext()) {
            RAWorkId consecutiveWorkId = RAWorkId(it.next());
            RAWorkReg* consecutiveReg = workRegById(consecutiveWorkId);
            if (!consecutiveReg->isProcessedConsecutive()) {
              ASMJIT_PROPAGATE(consecutiveRegs.append(allocator(), RAConsecutiveReg{consecutiveReg, workReg}));
              consecutiveReg->markProcessedConsecutive();
            }
          }
        }
        i++;
      }
    }

    size_t numConsecutiveRegs = consecutiveRegs.size();
    for (size_t i = 0; i < numConsecutiveRegs; i++) {
      RAWorkReg* workReg = consecutiveRegs[i].workReg;
      if (workReg->isAllocated()) {
        continue;
      }

      RAWorkReg* parentReg = consecutiveRegs[i].parentReg;
      RegMask physRegs = 0;

      if (!parentReg) {
        physRegs = availableRegs & workReg->preferredMask();
        if (!physRegs) {
          physRegs = availableRegs & workReg->consecutiveMask();

          // NOTE: This should never be true as it would mean we would never allocate this virtual register
          // (not here, and not later when local register allocator processes RATiedReg sets).
          if (ASMJIT_UNLIKELY(!physRegs)) {
            return DebugUtils::errored(kErrorConsecutiveRegsAllocation);
          }
        }
      }
      else if (parentReg->hasHomeRegId()) {
        uint32_t consecutiveId = parentReg->homeRegId() + 1;

        // NOTE: We don't support wrapping. If this goes beyond all allocable registers there is something wrong.
        if (consecutiveId > 31 || !Support::bit_test(availableRegs, consecutiveId)) {
          return DebugUtils::errored(kErrorConsecutiveRegsAllocation);
        }

        workReg->setHintRegId(consecutiveId);
        physRegs = Support::bitMask<uint32_t>(consecutiveId);
      }

      while (physRegs) {
        uint32_t physId = Support::bit_size_of<RegMask> - 1 - Support::clz(physRegs);

        RALiveSpans& live = _globalLiveSpans[group][physId];
        Error err = tmpSpans.nonOverlappingUnionOf(allocator(), live, workReg->liveSpans());

        if (err == kErrorOk) {
          workReg->setHomeRegId(physId);
          workReg->markAllocated();
          live.swap(tmpSpans);
          break;
        }

        if (ASMJIT_UNLIKELY(err != 0xFFFFFFFFu)) {
          return err;
        }

        physRegs ^= Support::bitMask<RegMask>(physId);
      }
    }
  }

  // Try to pack the rest.
  if (!workRegs.empty()) {
    size_t dstIndex = 0;

    for (size_t index = 0; index < numWorkRegs; index++) {
      RAWorkReg* workReg = workRegs[index];
      if (workReg->isAllocated()) {
        continue;
      }

      RegMask remainingPhysRegs = availableRegs;
      if (remainingPhysRegs & workReg->preferredMask()) {
        remainingPhysRegs &= workReg->preferredMask();
      }

      RegMask physRegs = remainingPhysRegs & ~preservedRegs;
      remainingPhysRegs &= preservedRegs;

      for (;;) {
        if (!physRegs) {
          if (!remainingPhysRegs) {
            break;
          }
          physRegs = remainingPhysRegs;
          remainingPhysRegs = 0;
        }

        uint32_t physId = Support::ctz(physRegs);

        if (workReg->clobberSurvivalMask()) {
          RegMask preferredMask = (physRegs | remainingPhysRegs) & workReg->clobberSurvivalMask();
          if (preferredMask) {
            if (preferredMask & ~remainingPhysRegs) {
              preferredMask &= ~remainingPhysRegs;
            }
            physId = Support::ctz(preferredMask);
          }
        }

        RALiveSpans& live = _globalLiveSpans[group][physId];
        Error err = tmpSpans.nonOverlappingUnionOf(allocator(), live, workReg->liveSpans());

        if (err == kErrorOk) {
          workReg->setHomeRegId(physId);
          workReg->markAllocated();
          live.swap(tmpSpans);
          break;
        }

        if (ASMJIT_UNLIKELY(err != 0xFFFFFFFFu)) {
          return err;
        }

        physRegs &= ~Support::bitMask<RegMask>(physId);
        remainingPhysRegs &= ~Support::bitMask<RegMask>(physId);
      }

      // Keep it in `workRegs` if it was not allocated.
      if (!physRegs) {
        workRegs[dstIndex++] = workReg;
      }
    }

    workRegs._setSize(dstIndex);
    numWorkRegs = dstIndex;
  }

  ASMJIT_RA_LOG_COMPLEX({
    for (uint32_t physId = 0; physId < physCount; physId++) {
      RALiveSpans& live = _globalLiveSpans[group][physId];
      if (live.empty()) {
        continue;
      }

      sb.clear();
      RAPass_dumpSpans(sb, physId, live);
      logger->log(sb);
    }
  });

  // Maybe unused if logging is disabled.
  DebugUtils::unused(physCount);

  if (workRegs.empty()) {
    ASMJIT_RA_LOG_FORMAT("  Completed.\n");
  }
  else {
    _strategy[group].setType(RAStrategyType::kComplex);
    for (RAWorkReg* workReg : workRegs) {
      workReg->markStackPreferred();
    }

    ASMJIT_RA_LOG_COMPLEX({
      size_t count = workRegs.size();
      sb.clear();
      sb.appendFormat("  Unassigned (%zu): ", count);
      for (uint32_t i = 0; i < numWorkRegs; i++) {
        RAWorkReg* workReg = workRegs[i];
        if (i) {
          sb.append(", ");
        }
        Formatter::formatVirtRegName(sb, workReg->virtReg());
      }
      sb.append('\n');
      logger->log(sb);
    });
  }

  return kErrorOk;
}

// BaseRAPass - Allocation - Local
// ===============================

Error BaseRAPass::runLocalAllocator() noexcept {
  RALocalAllocator lra(*this);
  ASMJIT_PROPAGATE(lra.init());

  if (!blockCount()) {
    return kErrorOk;
  }

  // The allocation is done when this reaches zero.
  size_t blocksRemaining = reachableBlockCount();

  // Current block.
  uint32_t blockId = 0;
  RABlock* block = _blocks[blockId];

  // The first block (entry) must always be reachable.
  ASMJIT_ASSERT(block->isReachable());

  // Assign function arguments for the initial block. The `lra` is valid now.
  ASMJIT_PROPAGATE(lra.makeInitialAssignment());
  ASMJIT_PROPAGATE(setBlockEntryAssignment(block, block, lra._curAssignment));

  // The loop starts from the first block and iterates blocks in order, however, the algorithm also allows to jump to
  // any other block when finished if it's a jump target. In-order iteration just makes sure that all blocks are visited.
  for (;;) {
    BaseNode* first = block->first();
    BaseNode* last = block->last();
    BaseNode* terminator = block->hasTerminator() ? last : nullptr;

    BaseNode* beforeFirst = first->prev();
    BaseNode* afterLast = last->next();

    bool unconditionalJump = false;
    RABlock* consecutive = block->hasSuccessors() ? block->successors()[0] : nullptr;

    lra.setBlock(block);
    block->makeAllocated();

    BaseNode* node = first;
    while (node != afterLast) {
      BaseNode* next = node->next();
      if (node->isInst()) {
        InstNode* inst = node->as<InstNode>();

        if (ASMJIT_UNLIKELY(inst == terminator)) {
          Span<RABlock*> successors = block->successors();
          if (block->hasConsecutive()) {
            ASMJIT_PROPAGATE(lra.allocBranch(inst, successors.last(), successors.first()));

            node = next;
            continue;
          }
          else if (successors.size() > 1) {
            RABlock* cont = block->hasConsecutive() ? successors.first() : nullptr;
            ASMJIT_PROPAGATE(lra.allocJumpTable(inst, successors, cont));

            node = next;
            continue;
          }
          else {
            // Otherwise this is an unconditional jump, special handling isn't required.
            unconditionalJump = true;
          }
        }

        ASMJIT_PROPAGATE(lra.allocInst(inst));
        if (inst->type() == NodeType::kInvoke) {
          ASMJIT_PROPAGATE(emitPreCall(inst->as<InvokeNode>()));
        }
        else {
          ASMJIT_PROPAGATE(lra.spillAfterAllocation(inst));
        }
      }
      node = next;
    }

    if (consecutive) {
      BaseNode* prev = afterLast ? afterLast->prev() : cc().lastNode();
      cc().setCursor(unconditionalJump ? prev->prev() : prev);

      if (consecutive->hasEntryAssignment()) {
        ASMJIT_PROPAGATE(lra.switchToAssignment(consecutive->entryPhysToWorkMap(), consecutive->liveIn(), consecutive->isAllocated(), false));
      }
      else {
        ASMJIT_PROPAGATE(lra.spillRegsBeforeEntry(consecutive));
        ASMJIT_PROPAGATE(setBlockEntryAssignment(consecutive, block, lra._curAssignment));
        lra._curAssignment.copyFrom(consecutive->entryPhysToWorkMap());
      }
    }

    // Important as the local allocator can insert instructions before
    // and after any instruction within the basic block.
    block->setFirst(beforeFirst->next());
    block->setLast(afterLast ? afterLast->prev() : cc().lastNode());

    if (--blocksRemaining == 0) {
      break;
    }

    // Switch to the next consecutive block, if any.
    if (consecutive) {
      block = consecutive;
      if (!block->isAllocated()) {
        continue;
      }
    }

    // Get the next block.
    for (;;) {
      if (++blockId >= blockCount()) {
        blockId = 0;
      }

      block = _blocks[blockId];
      if (!block->isReachable() || block->isAllocated() || !block->hasEntryAssignment()) {
        continue;
      }

      break;
    }

    // If we switched to another block we have to update the local allocator.
    ASMJIT_PROPAGATE(lra.replaceAssignment(block->entryPhysToWorkMap()));
  }

  _clobberedRegs.op<Support::Or>(lra._clobberedRegs);
  return kErrorOk;
}

Error BaseRAPass::setBlockEntryAssignment(RABlock* block, const RABlock* fromBlock, const RAAssignment& fromAssignment) noexcept {
  if (block->hasSharedAssignmentId()) {
    uint32_t sharedAssignmentId = block->sharedAssignmentId();

    // Shouldn't happen. Entry assignment of a block that has a shared-state will assign to all blocks
    // with the same sharedAssignmentId. It's a bug if the shared state has been already assigned.
    if (!_sharedAssignments[sharedAssignmentId].empty()) {
      return DebugUtils::errored(kErrorInvalidState);
    }

    return setSharedAssignment(sharedAssignmentId, fromAssignment);
  }

  PhysToWorkMap* physToWorkMap = clonePhysToWorkMap(fromAssignment.physToWorkMap());
  if (ASMJIT_UNLIKELY(!physToWorkMap)) {
    return DebugUtils::errored(kErrorOutOfMemory);
  }

  block->setEntryAssignment(physToWorkMap);

  // True if this is the first (entry) block, nothing to do in this case.
  if (block == fromBlock) {
    // Entry block should never have a shared state.
    if (block->hasSharedAssignmentId()) {
      return DebugUtils::errored(kErrorInvalidState);
    }

    return kErrorOk;
  }

  Span<const BitWord> liveOut = fromBlock->liveOut();
  Span<const BitWord> liveIn = block->liveIn();

  // It's possible that `fromBlock` has LIVE-OUT regs that `block` doesn't
  // have in LIVE-IN, these have to be unassigned.
  {
    Support::BitVectorOpIterator<BitWord, Support::AndNot> it(liveOut, liveIn);
    while (it.hasNext()) {
      RAWorkId workId = RAWorkId(it.next());
      RAWorkReg* workReg = workRegById(workId);

      RegGroup group = workReg->group();
      uint32_t physId = fromAssignment.workToPhysId(group, workId);

      if (physId != RAAssignment::kPhysNone) {
        physToWorkMap->unassign(group, physId, _physRegIndex.get(group) + physId);
      }
    }
  }

  return blockEntryAssigned(physToWorkMap);
}

Error BaseRAPass::setSharedAssignment(uint32_t sharedAssignmentId, const RAAssignment& fromAssignment) noexcept {
  ASMJIT_ASSERT(_sharedAssignments[sharedAssignmentId].empty());

  PhysToWorkMap* physToWorkMap = clonePhysToWorkMap(fromAssignment.physToWorkMap());
  if (ASMJIT_UNLIKELY(!physToWorkMap)) {
    return DebugUtils::errored(kErrorOutOfMemory);
  }

  _sharedAssignments[sharedAssignmentId].assignPhysToWorkMap(physToWorkMap);
  ASMJIT_PROPAGATE(_sharedAssignments[sharedAssignmentId]._liveIn.resize(allocator(), multiWorkRegCount()));

  Span<BitWord> sharedLiveIn = _sharedAssignments[sharedAssignmentId]._liveIn.as_span();
  Support::Array<uint32_t, Globals::kNumVirtGroups> sharedAssigned {};

  for (RABlock* block : blocks()) {
    if (block->sharedAssignmentId() == sharedAssignmentId) {
      ASMJIT_ASSERT(!block->hasEntryAssignment());

      PhysToWorkMap* entryPhysToWorkMap = clonePhysToWorkMap(fromAssignment.physToWorkMap());
      if (ASMJIT_UNLIKELY(!entryPhysToWorkMap)) {
        return DebugUtils::errored(kErrorOutOfMemory);
      }

      block->setEntryAssignment(entryPhysToWorkMap);

      Span<const BitWord> liveIn = block->liveIn();
      BitOps::or_(sharedLiveIn, sharedLiveIn, liveIn);

      for (RegGroup group : Support::enumerate(RegGroup::kMaxVirt)) {
        sharedAssigned[group] |= entryPhysToWorkMap->assigned[group];

        uint32_t physBaseIndex = _physRegIndex.get(group);
        Support::BitWordIterator<RegMask> it(entryPhysToWorkMap->assigned[group]);

        while (it.hasNext()) {
          uint32_t physId = it.next();
          RAWorkId workId = entryPhysToWorkMap->workIds[physBaseIndex + physId];

          // Should not happen as a register that only lives in a single basic block should not appear in the map.
          ASMJIT_ASSERT(uint32_t(workId) < _multiWorkRegCount);

          if (!BitOps::bit_at(liveIn, workId)) {
            entryPhysToWorkMap->unassign(group, physId, physBaseIndex + physId);
          }
        }
      }
    }
  }

  for (RegGroup group : Support::enumerate(RegGroup::kMaxVirt)) {
    uint32_t physBaseIndex = _physRegIndex.get(group);
    Support::BitWordIterator<RegMask> it(_availableRegs[group] & ~sharedAssigned[group]);

    while (it.hasNext()) {
      uint32_t physId = it.next();
      if (Support::bit_test(physToWorkMap->assigned[group], physId)) {
        physToWorkMap->unassign(group, physId, physBaseIndex + physId);
      }
    }
  }

  return blockEntryAssigned(physToWorkMap);
}

Error BaseRAPass::blockEntryAssigned(const PhysToWorkMap* physToWorkMap) noexcept {
  // Complex allocation strategy requires to record register assignments upon block entry (or per shared state).
  for (RegGroup group : Support::enumerate(RegGroup::kMaxVirt)) {
    if (!_strategy[group].isComplex()) {
      continue;
    }

    uint32_t physBaseIndex = _physRegIndex[group];
    Support::BitWordIterator<RegMask> it(physToWorkMap->assigned[group]);

    while (it.hasNext()) {
      uint32_t physId = it.next();
      RAWorkId workId = physToWorkMap->workIds[physBaseIndex + physId];

      RAWorkReg* workReg = workRegById(workId);
      workReg->addAllocatedMask(Support::bitMask<RegMask>(physId));
    }
  }

  return kErrorOk;
}

// BaseRAPass - Allocation - Utilities
// ===================================

Error BaseRAPass::useTemporaryMem(BaseMem& out, uint32_t size, uint32_t alignment) noexcept {
  ASMJIT_ASSERT(alignment <= 64);

  if (_temporaryMem.isNone()) {
    ASMJIT_PROPAGATE(cc()._newStack(&_temporaryMem.as<BaseMem>(), size, alignment));
  }
  else {
    ASMJIT_ASSERT(_temporaryMem.as<BaseMem>().isRegHome());

    uint32_t vRegId = _temporaryMem.as<BaseMem>().baseId();
    VirtReg* vReg = cc().virtRegById(vRegId);

    cc().setStackSize(vRegId, Support::max(vReg->virtSize(), size),
                               Support::max(vReg->alignment(), alignment));
  }

  out = _temporaryMem.as<BaseMem>();
  return kErrorOk;
}

// BaseRAPass - Allocation - Prolog & Epilog
// =========================================

Error BaseRAPass::updateStackFrame() noexcept {
  // Update some StackFrame information that we updated during allocation. The only information we don't have at the
  // moment is final local stack size, which is calculated last.
  FuncFrame& frame = func()->frame();
  for (RegGroup group : Support::enumerate(RegGroup::kMaxVirt)) {
    frame.addDirtyRegs(group, _clobberedRegs[group]);
  }
  frame.setLocalStackAlignment(_stackAllocator.alignment());

  // If there are stack arguments that are not assigned to registers upon entry and the function doesn't require
  // dynamic stack alignment we keep these arguments where they are. This will also mark all stack slots that match
  // these arguments as allocated.
  if (_numStackArgsToStackSlots) {
    ASMJIT_PROPAGATE(_markStackArgsToKeep());
  }

  // Calculate offsets of all stack slots and update StackSize to reflect the calculated local stack size.
  ASMJIT_PROPAGATE(_stackAllocator.calculateStackFrame());
  frame.setLocalStackSize(_stackAllocator.stackSize());

  // Update the stack frame based on `_argsAssignment` and finalize it. Finalization means to apply final calculation
  // to the stack layout.
  ASMJIT_PROPAGATE(_argsAssignment.updateFuncFrame(frame));
  ASMJIT_PROPAGATE(frame.finalize());

  // StackAllocator allocates all stots starting from [0], adjust them when necessary.
  if (frame.localStackOffset() != 0) {
    ASMJIT_PROPAGATE(_stackAllocator.adjustSlotOffsets(int32_t(frame.localStackOffset())));
  }

  // Again, if there are stack arguments allocated in function's stack we have to handle them. This handles all cases
  // (either regular or dynamic stack alignment).
  if (_numStackArgsToStackSlots) {
    ASMJIT_PROPAGATE(_updateStackArgs());
  }

  return kErrorOk;
}

Error BaseRAPass::_markStackArgsToKeep() noexcept {
  FuncFrame& frame = func()->frame();
  bool hasSAReg = frame.hasPreservedFP() || !frame.hasDynamicAlignment();

  ZoneVector<RAWorkReg*>& workRegs = _workRegs;
  size_t numWorkRegs = workRegCount();

  for (size_t workId = 0; workId < numWorkRegs; workId++) {
    RAWorkReg* workReg = workRegs[workId];
    if (workReg->hasFlag(RAWorkRegFlags::kStackArgToStack)) {
      ASMJIT_ASSERT(workReg->hasArgIndex());
      const FuncValue& srcArg = _func->detail().arg(workReg->argIndex());

      // If the register doesn't have stack slot then we failed. It doesn't make much sense as it was marked as
      // `kFlagStackArgToStack`, which requires the WorkReg was live-in upon function entry.
      RAStackSlot* slot = workReg->stackSlot();
      if (ASMJIT_UNLIKELY(!slot)) {
        return DebugUtils::errored(kErrorInvalidState);
      }

      if (hasSAReg && srcArg.isStack() && !srcArg.isIndirect()) {
        uint32_t typeSize = TypeUtils::sizeOf(srcArg.typeId());
        if (typeSize == slot->size()) {
          slot->addFlags(RAStackSlot::kFlagStackArg);
          continue;
        }
      }

      // NOTE: Update StackOffset here so when `_argsAssignment.updateFuncFrame()` is called it will take into
      // consideration moving to stack slots. Without this we may miss some scratch registers later.
      FuncValue& dstArg = _argsAssignment.arg(workReg->argIndex(), workReg->argValueIndex());
      dstArg.assignStackOffset(0);
    }
  }

  return kErrorOk;
}

Error BaseRAPass::_updateStackArgs() noexcept {
  FuncFrame& frame = func()->frame();

  ZoneVector<RAWorkReg*>& workRegs = _workRegs;
  size_t numWorkRegs = workRegCount();

  for (size_t workId = 0; workId < numWorkRegs; workId++) {
    RAWorkReg* workReg = workRegs[workId];
    if (workReg->hasFlag(RAWorkRegFlags::kStackArgToStack)) {
      ASMJIT_ASSERT(workReg->hasArgIndex());
      RAStackSlot* slot = workReg->stackSlot();

      if (ASMJIT_UNLIKELY(!slot)) {
        return DebugUtils::errored(kErrorInvalidState);
      }

      if (slot->isStackArg()) {
        const FuncValue& srcArg = _func->detail().arg(workReg->argIndex());
        if (frame.hasPreservedFP()) {
          slot->setBaseRegId(_fp.id());
          slot->setOffset(int32_t(frame.saOffsetFromSA()) + srcArg.stackOffset());
        }
        else {
          slot->setOffset(int32_t(frame.saOffsetFromSP()) + srcArg.stackOffset());
        }
      }
      else {
        FuncValue& dstArg = _argsAssignment.arg(workReg->argIndex(), workReg->argValueIndex());
        dstArg.setStackOffset(slot->offset());
      }
    }
  }

  return kErrorOk;
}

Error BaseRAPass::insertPrologEpilog() noexcept {
  FuncFrame& frame = _func->frame();

  cc().setCursor(func());
  ASMJIT_PROPAGATE(cc().emitProlog(frame));
  ASMJIT_PROPAGATE(_iEmitHelper->emitArgsAssignment(frame, _argsAssignment));

  cc().setCursor(func()->exitNode());
  ASMJIT_PROPAGATE(cc().emitEpilog(frame));

  return kErrorOk;
}

// BaseRAPass - Rewriter
// =====================

// [[pure virtual]]
Error BaseRAPass::rewrite() noexcept {
  return DebugUtils::errored(kErrorInvalidState);
}

// BaseRAPass - Emit
// =================

// [[pure virtual]]
Error BaseRAPass::emitMove(RAWorkReg* wReg, uint32_t dstPhysId, uint32_t srcPhysId) noexcept {
  DebugUtils::unused(wReg, dstPhysId, srcPhysId);
  return DebugUtils::errored(kErrorInvalidState);
}

// [[pure virtual]]
Error BaseRAPass::emitSwap(RAWorkReg* aReg, uint32_t aPhysId, RAWorkReg* bReg, uint32_t bPhysId) noexcept {
  DebugUtils::unused(aReg, aPhysId, bReg, bPhysId);
  return DebugUtils::errored(kErrorInvalidState);
}

// [[pure virtual]]
Error BaseRAPass::emitLoad(RAWorkReg* wReg, uint32_t dstPhysId) noexcept {
  DebugUtils::unused(wReg, dstPhysId);
  return DebugUtils::errored(kErrorInvalidState);
}

// [[pure virtual]]
Error BaseRAPass::emitSave(RAWorkReg* wReg, uint32_t srcPhysId) noexcept {
  DebugUtils::unused(wReg, srcPhysId);
  return DebugUtils::errored(kErrorInvalidState);
}

// [[pure virtual]]
Error BaseRAPass::emitJump(const Label& label) noexcept {
  DebugUtils::unused(label);
  return DebugUtils::errored(kErrorInvalidState);
}

Error BaseRAPass::emitPreCall(InvokeNode* invokeNode) noexcept {
  DebugUtils::unused(invokeNode);
  return DebugUtils::errored(kErrorOk);
}

// BaseRAPass - Logging
// ====================

#ifndef ASMJIT_NO_LOGGING
static void RAPass_formatLiveness(BaseRAPass* pass, String& sb, const RAInst* raInst) noexcept {
  DebugUtils::unused(pass);

  const RATiedReg* tiedRegs = raInst->tiedRegs();
  uint32_t tiedCount = raInst->tiedCount();

  for (uint32_t i = 0; i < tiedCount; i++) {
    const RATiedReg& tiedReg = tiedRegs[i];

    if (i != 0) {
      sb.append(' ');
    }

    Formatter::formatVirtRegName(sb, tiedReg.workReg()->virtReg());
    sb.append('{');
    sb.append(tiedReg.isReadWrite() ? 'X' :
              tiedReg.isRead()      ? 'R' :
              tiedReg.isWrite()     ? 'W' : '?');

    if (tiedReg.isLeadConsecutive()) {
      sb.appendFormat("|Lead[%u]", tiedReg.consecutiveData() + 1u);
    }

    if (tiedReg.hasUseId()) {
      sb.appendFormat("|Use=%u", tiedReg.useId());
    }
    else if (tiedReg.isUse()) {
      sb.append("|Use");
    }

    if (tiedReg.isUseConsecutive() && !tiedReg.isLeadConsecutive()) {
      sb.appendFormat("+%u", tiedReg.consecutiveData());
    }

    if (tiedReg.hasOutId()) {
      sb.appendFormat("|Out=%u", tiedReg.outId());
    }
    else if (tiedReg.isOut()) {
      sb.append("|Out");
    }

    if (tiedReg.isOutConsecutive() && !tiedReg.isLeadConsecutive()) {
      sb.appendFormat("+%u", tiedReg.consecutiveData());
    }

    if (tiedReg.isFirst()) {
      sb.append("|First");
    }

    if (tiedReg.isLast()) {
      sb.append("|Last");
    }

    if (tiedReg.isKill()) {
      sb.append("|Kill");
    }

    sb.append("}");
  }
}

ASMJIT_FAVOR_SIZE Error BaseRAPass::annotateCode() noexcept {
  StringTmp<1024> sb;

  for (const RABlock* block : _blocks) {
    BaseNode* node = block->first();
    if (!node) {
      continue;
    }

    BaseNode* last = block->last();
    for (;;) {
      sb.clear();
      Formatter::formatNode(sb, _formatOptions, &_cb, node);

      if (hasDiagnosticOption(DiagnosticOptions::kRADebugLiveness) && node->isInst() && node->hasPassData()) {
        const RAInst* raInst = node->passData<RAInst>();
        if (raInst->tiedCount() > 0) {
          sb.padEnd(40);
          sb.append(" | ");
          RAPass_formatLiveness(this, sb, raInst);
        }
      }

      node->setInlineComment(static_cast<char*>(cc()._codeZone.dup(sb.data(), sb.size(), true)));
      if (node == last) {
        break;
      }
      node = node->next();
    }
  }

  return kErrorOk;
}

ASMJIT_FAVOR_SIZE Error BaseRAPass::_dumpBlockIds(String& sb, Span<RABlock*> blocks) noexcept {
  for (size_t i = 0; i < blocks.size(); i++) {
    const RABlock* block = blocks[i];
    ASMJIT_PROPAGATE(sb.appendFormat(!i ? "#%u" : ", #%u", uint32_t(block->blockId())));
  }
  return kErrorOk;
}

ASMJIT_FAVOR_SIZE Error BaseRAPass::_dumpBlockLiveness(String& sb, const RABlock* block) noexcept {
  for (uint32_t liveType = 0; liveType < RABlock::kLiveCount; liveType++) {
    const char* bitsName = liveType == RABlock::kLiveIn  ? "IN  " :
                           liveType == RABlock::kLiveOut ? "OUT " : "KILL";

    Support::BitVectorIterator<BitWord> it(block->liveBits(liveType));
    if (it.hasNext()) {
      bool first = true;

      sb.appendFormat("    %s [", bitsName);
      do {
        const RAWorkReg* wReg = workRegById(RAWorkId(it.next()));
        if (!first) {
          sb.append(", ");
        }

        Formatter::formatVirtRegName(sb, wReg->virtReg());
        first = false;
      } while (it.hasNext());
      sb.append("]\n");
    }
  }

  return kErrorOk;
}

ASMJIT_FAVOR_SIZE Error BaseRAPass::_dumpLiveSpans(String& sb) noexcept {
  size_t maxSize = _maxWorkRegNameSize;

  for (RAWorkReg* workReg : _workRegs.iterate()) {
    RALiveStats& stats = workReg->liveStats();

    sb.append("  ");
    size_t oldSize = sb.size();

    Formatter::formatVirtRegName(sb, workReg->virtReg());
    sb.padEnd(oldSize + maxSize);

    sb.appendFormat(" {raId=%-5u virtId=%-5u width=%-5u freq=%0.5f priority=%0.5f ",
      uint32_t(workReg->workId()),
      workReg->vRegId(),
      stats.width(), stats.freq(), stats.priority());

    if (workReg->isWithinSingleBasicBlock()) {
      sb.appendFormat("bb=#%-4u", uint32_t(workReg->singleBasicBlockId()));
    }
    else {
      sb.append("bb=<...>");
    }

    sb.appendFormat("}: ");

    RALiveSpans& liveSpans = workReg->liveSpans();
    for (uint32_t x = 0; x < liveSpans.size(); x++) {
      const RALiveSpan& liveSpan = liveSpans[x];
      if (x) {
        sb.append(", ");
      }
      sb.appendFormat("[%u:%u]", liveSpan.a, liveSpan.b);
    }

    sb.append('\n');
  }

  return kErrorOk;
}
#endif

ASMJIT_END_NAMESPACE

#endif // !ASMJIT_NO_COMPILER
