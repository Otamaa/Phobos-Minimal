// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_X86_X86EMITHELPER_P_H_INCLUDED
#define ASMJIT_X86_X86EMITHELPER_P_H_INCLUDED

#include "../core/api-config.h"

#include "../core/emithelper_p.h"
#include "../core/func.h"
#include "../x86/x86emitter.h"
#include "../x86/x86instapi_p.h"
#include "../x86/x86operand.h"

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

//! \cond INTERNAL
//! \addtogroup asmjit_x86
//! \{

[[nodiscard]]
static ASMJIT_INLINE_NODEBUG RegType vecTypeIdToRegType(TypeId typeId) noexcept {
  return uint32_t(typeId) <= uint32_t(TypeId::_kVec128End) ? RegType::kVec128 :
         uint32_t(typeId) <= uint32_t(TypeId::_kVec256End) ? RegType::kVec256 : RegType::kVec512;
}

//! Instruction identifiers for targeting SSE and AVX backends.
struct EmitHelperInstructionIds {
  //! 16-bit identifier is enough for us here as X86|X86_64 doesn't have more than 65536 instructions.
  using IId = uint16_t;

  IId _movd_movss[2];
  IId _movq_movsd[2];
  IId _movups_movaps[2];
  IId _movupd_movapd[2];
  IId _movdqu_movdqa[2];
  IId _movlps[1];
  IId _cvtss2sd_cvtsd2ss[2];
  IId _cvtps2pd_cvtpd2ps[2];

  ASMJIT_INLINE_NODEBUG InstId movd() const noexcept { return _movd_movss[0]; }
  ASMJIT_INLINE_NODEBUG InstId movss() const noexcept { return _movd_movss[1]; }
  ASMJIT_INLINE_NODEBUG InstId movd_or_movss(size_t idx) const noexcept { return _movd_movss[idx]; }

  ASMJIT_INLINE_NODEBUG InstId movq() const noexcept { return _movq_movsd[0]; }
  ASMJIT_INLINE_NODEBUG InstId movsd() const noexcept { return _movq_movsd[1]; }
  ASMJIT_INLINE_NODEBUG InstId movq_or_movsd(size_t idx) const noexcept { return _movq_movsd[idx]; }

  ASMJIT_INLINE_NODEBUG InstId movups() const noexcept { return _movups_movaps[0]; }
  ASMJIT_INLINE_NODEBUG InstId movaps() const noexcept { return _movups_movaps[1]; }
  ASMJIT_INLINE_NODEBUG InstId movups_or_movaps(size_t idx) const noexcept { return _movups_movaps[idx]; }

  ASMJIT_INLINE_NODEBUG InstId movupd() const noexcept { return _movupd_movapd[0]; }
  ASMJIT_INLINE_NODEBUG InstId movapd() const noexcept { return _movupd_movapd[1]; }
  ASMJIT_INLINE_NODEBUG InstId movupd_or_movapd(size_t idx) const noexcept { return _movupd_movapd[idx]; }

  ASMJIT_INLINE_NODEBUG InstId movdqu() const noexcept { return _movdqu_movdqa[0]; }
  ASMJIT_INLINE_NODEBUG InstId movdqa() const noexcept { return _movdqu_movdqa[1]; }
  ASMJIT_INLINE_NODEBUG InstId movdqu_or_movdqa(size_t idx) const noexcept { return _movdqu_movdqa[idx]; }

  ASMJIT_INLINE_NODEBUG InstId movlps() const noexcept { return _movlps[0]; }

  ASMJIT_INLINE_NODEBUG InstId cvtss2sd() const noexcept { return _cvtss2sd_cvtsd2ss[0]; }
  ASMJIT_INLINE_NODEBUG InstId cvtsd2ss() const noexcept { return _cvtss2sd_cvtsd2ss[1]; }

  ASMJIT_INLINE_NODEBUG InstId cvtps2pd() const noexcept { return _cvtps2pd_cvtpd2ps[0]; }
  ASMJIT_INLINE_NODEBUG InstId cvtpd2ps() const noexcept { return _cvtps2pd_cvtpd2ps[1]; }
};

//! Emit helper data for SSE at [0] and AVX/AVX-512 at [1].
extern const EmitHelperInstructionIds _emitHelperInstructionIds[2];

class EmitHelper : public BaseEmitHelper {
protected:
  bool _avxEnabled;
  bool _avx512Enabled;
  const EmitHelperInstructionIds* _ids;

public:
  ASMJIT_INLINE_NODEBUG explicit EmitHelper(BaseEmitter* emitter = nullptr, bool avxEnabled = false, bool avx512Enabled = false) noexcept
    : BaseEmitHelper(emitter) { reset(emitter, avxEnabled, avx512Enabled); }

  ASMJIT_INLINE_NODEBUG virtual ~EmitHelper() noexcept = default;

  ASMJIT_INLINE_NODEBUG bool isAvxEnabled() const noexcept { return _avxEnabled; }
  ASMJIT_INLINE_NODEBUG bool isAvx512Enabled() const noexcept { return _avx512Enabled; }
  ASMJIT_INLINE_NODEBUG const EmitHelperInstructionIds& ids() const noexcept { return *_ids; }

  ASMJIT_INLINE void reset(BaseEmitter* emitter, bool avxEnabled, bool avx512Enabled) noexcept {
    _emitter = emitter;
    _avxEnabled = avxEnabled || avx512Enabled;
    _avx512Enabled = avx512Enabled;
    _ids = &_emitHelperInstructionIds[size_t(_avxEnabled)];
  }

  Error emitRegMove(
    const Operand_& dst_,
    const Operand_& src_, TypeId typeId, const char* comment = nullptr) override;

  Error emitArgMove(
    const Reg& dst_, TypeId dstTypeId,
    const Operand_& src_, TypeId srcTypeId, const char* comment = nullptr) override;

  Error emitRegSwap(
    const Reg& a,
    const Reg& b, const char* comment = nullptr) override;

  Error emitProlog(const FuncFrame& frame);
  Error emitEpilog(const FuncFrame& frame);
};

void initEmitterFuncs(BaseEmitter* emitter) noexcept;

static ASMJIT_INLINE void updateEmitterFuncs(BaseEmitter* emitter) noexcept {
#ifndef ASMJIT_NO_VALIDATION
  emitter->_funcs.validate = emitter->is32Bit() ? InstInternal::validateX86 : InstInternal::validateX64;
#else
  DebugUtils::unused(emitter);
#endif
}

//! \}
//! \endcond

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_X86_X86EMITHELPER_P_H_INCLUDED
