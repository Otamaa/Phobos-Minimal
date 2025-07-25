// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_RACONSTRAINTS_P_H_INCLUDED
#define ASMJIT_CORE_RACONSTRAINTS_P_H_INCLUDED

#include "../core/api-config.h"
#include "../core/archtraits.h"
#include "../core/operand.h"
#include "../core/support.h"

ASMJIT_BEGIN_NAMESPACE

//! \cond INTERNAL
//! \addtogroup asmjit_ra
//! \{

//! Provides architecture constraints used by register allocator.
class RAConstraints {
public:
  //! \name Members
  //! \{

  Support::Array<RegMask, Globals::kNumVirtGroups> _availableRegs {};

  //! \}

  [[nodiscard]]
  ASMJIT_NOINLINE Error init(Arch arch) noexcept {
    switch (arch) {
      case Arch::kX86:
      case Arch::kX64: {
        uint32_t registerCount = arch == Arch::kX86 ? 8 : 16;
        _availableRegs[RegGroup::kGp] = Support::lsb_mask<RegMask>(registerCount) & ~Support::bitMask<RegMask>(4u);
        _availableRegs[RegGroup::kVec] = Support::lsb_mask<RegMask>(registerCount);
        _availableRegs[RegGroup::kMask] = Support::lsb_mask<RegMask>(8);
        _availableRegs[RegGroup::kX86_MM] = Support::lsb_mask<RegMask>(8);
        return kErrorOk;
      }

      case Arch::kAArch64: {
        _availableRegs[RegGroup::kGp] = 0xFFFFFFFFu & ~Support::bitMask<RegMask>(18, 31u);
        _availableRegs[RegGroup::kVec] = 0xFFFFFFFFu;
        _availableRegs[RegGroup::kMask] = 0;
        _availableRegs[RegGroup::kExtra] = 0;
        return kErrorOk;
      }

      default:
        return DebugUtils::errored(kErrorInvalidArch);
    }
  }

  [[nodiscard]]
  inline RegMask availableRegs(RegGroup group) const noexcept { return _availableRegs[group]; }
};

//! \}
//! \endcond

ASMJIT_END_NAMESPACE

#endif // ASMJIT_CORE_RACONSTRAINTS_P_H_INCLUDED
