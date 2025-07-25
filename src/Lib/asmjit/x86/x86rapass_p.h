// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_X86_X86RAPASS_P_H_INCLUDED
#define ASMJIT_X86_X86RAPASS_P_H_INCLUDED

#include "../core/api-config.h"
#ifndef ASMJIT_NO_COMPILER

#include "../core/compiler.h"
#include "../core/racfgblock_p.h"
#include "../core/racfgbuilder_p.h"
#include "../core/rapass_p.h"
#include "../x86/x86assembler.h"
#include "../x86/x86compiler.h"
#include "../x86/x86emithelper_p.h"

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

//! \cond INTERNAL
//! \addtogroup asmjit_x86
//! \{

//! X86 register allocation pass.
//!
//! Takes care of generating function prologs and epilogs, and also performs register allocation.
class X86RAPass : public BaseRAPass {
public:
  ASMJIT_NONCOPYABLE(X86RAPass)
  using Base = BaseRAPass;

  //! \name Members
  //! \{

  EmitHelper _emitHelper;

  //! \}

  //! \name Construction & Destruction
  //! \{

  X86RAPass(BaseCompiler& cc) noexcept;
  ~X86RAPass() noexcept override;

  //! \}

  //! \name Accessors
  //! \{

  //! Returns the compiler casted to `x86::Compiler`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Compiler& cc() const noexcept { return static_cast<Compiler&>(_cb); }

  //! Returns emit helper.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG EmitHelper* emitHelper() noexcept { return &_emitHelper; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool isAvxEnabled() const noexcept { return _emitHelper.isAvxEnabled(); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool isAvx512Enabled() const noexcept { return _emitHelper.isAvx512Enabled(); }

  //! \}

  //! \name Interface
  //! \{

  void onInit() noexcept override;
  void onDone() noexcept override;

  Error buildCFG() noexcept override;

  Error rewrite() noexcept override;

  Error emitMove(RAWorkReg* wReg, uint32_t dstPhysId, uint32_t srcPhysId) noexcept override;
  Error emitSwap(RAWorkReg* aReg, uint32_t aPhysId, RAWorkReg* bReg, uint32_t bPhysId) noexcept override;

  Error emitLoad(RAWorkReg* wReg, uint32_t dstPhysId) noexcept override;
  Error emitSave(RAWorkReg* wReg, uint32_t srcPhysId) noexcept override;

  Error emitJump(const Label& label) noexcept override;
  Error emitPreCall(InvokeNode* invokeNode) noexcept override;

  //! \}
};

//! \}
//! \endcond

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_COMPILER
#endif // ASMJIT_X86_X86RAPASS_P_H_INCLUDED
