// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Guard]
#include "../asmjit_build.h"
#if defined(ASMJIT_BUILD_X86)

// [Dependencies]
#include "../base/arch.h"
#include "../base/inst.h"

#if defined(ASMJIT_BUILD_X86)
# include "../x86/x86instimpl_p.h"
#endif

#if defined(ASMJIT_BUILD_ARM)
# include "../arm/arminstimpl_p.h"
#endif

// [Api-Begin]
#include "../asmjit_apibegin.h"

namespace asmjit {

// ============================================================================
// [asmjit::Inst - Validate]
// ============================================================================

#if !defined(ASMJIT_DISABLE_INST_API)
Error Inst::validate(uint32_t archType, const Detail& detail, const Operand_* operands, uint32_t count) noexcept {
#if defined(ASMJIT_BUILD_X86)
  if (ArchInfo::isX86Family(archType))
    return X86InstImpl::validate(archType, detail, operands, count);
#endif

#if defined(ASMJIT_BUILD_ARM)
  if (ArchInfo::isArmFamily(archType))
    return ArmInstImpl::validate(archType, detail, operands, count);
#endif

  return DebugUtils::errored(kErrorInvalidArch);
}
#endif

// ============================================================================
// [asmjit::Inst - QueryRWInfo]
// ============================================================================

#if !defined(ASMJIT_DISABLE_INST_API)
Error Inst::queryRWInfo(uint32_t archType, const Inst::Detail& detail, const Operand_* operands, uint32_t count, Inst::IRWInfo& out) noexcept {
# if defined(ASMJIT_BUILD_X86)
  if (ArchInfo::isX86Family(archType))
    return X86InstImpl::queryRWInfo(archType, detail, operands, count, out);
# endif

# if defined(ASMJIT_BUILD_ARM)
  if (ArchInfo::isArmFamily(archType))
    return ArmInstImpl::queryRWInfo(archType, detail, operands, count, out);
# endif

  return DebugUtils::errored(kErrorInvalidArch);
}
#endif

// ============================================================================
// [asmjit::Inst - QueryCpuFeatures]
// ============================================================================

#if !defined(ASMJIT_DISABLE_INST_API)
Error Inst::queryCpuFeatures(uint32_t archType, const Detail& detail, const Operand_* operands, uint32_t count, CpuFeatures& out) noexcept {
# if defined(ASMJIT_BUILD_X86)
  if (ArchInfo::isX86Family(archType))
    return X86InstImpl::queryCpuFeatures(archType, detail, operands, count, out);
# endif

# if defined(ASMJIT_BUILD_ARM)
  if (ArchInfo::isArmFamily(archType))
    return ArmInstImpl::queryCpuFeatures(archType, detail, operands, count, out);
# endif

  return DebugUtils::errored(kErrorInvalidArch);
}
#endif

} // asmjit namespace

// [Api-End]
#include "../asmjit_apiend.h"

// [Guard]
#endif // ASMJIT_BUILD_X86
