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
#include "../base/intutils.h"
#include "../base/logging.h"
#include "../base/stringbuilder.h"
#include "../x86/x86internal_p.h"

// [Api-Begin]
#include "../asmjit_apibegin.h"

namespace asmjit {

// ============================================================================
// [asmjit::X86Internal - CallConv]
// ============================================================================

ASMJIT_FAVOR_SIZE Error X86Internal::initCallConv(CallConv& cc, uint32_t ccId) noexcept {
  const uint32_t kGroupGp  = X86Reg::kGroupGp;
  const uint32_t kGroupVec = X86Reg::kGroupVec;
  const uint32_t kGroupMm  = X86Reg::kGroupMm;
  const uint32_t kGroupK   = X86Reg::kGroupK;

  const uint32_t kZax = X86Gp::kIdAx;
  const uint32_t kZbx = X86Gp::kIdBx;
  const uint32_t kZcx = X86Gp::kIdCx;
  const uint32_t kZdx = X86Gp::kIdDx;
  const uint32_t kZsp = X86Gp::kIdSp;
  const uint32_t kZbp = X86Gp::kIdBp;
  const uint32_t kZsi = X86Gp::kIdSi;
  const uint32_t kZdi = X86Gp::kIdDi;

  switch (ccId) {
    case CallConv::kIdX86StdCall:
      cc.setFlags(CallConv::kFlagCalleePopsStack);
      goto X86CallConv;

    case CallConv::kIdX86MsThisCall:
      cc.setFlags(CallConv::kFlagCalleePopsStack);
      cc.setPassedOrder(kGroupGp, kZcx);
      goto X86CallConv;

    case CallConv::kIdX86MsFastCall:
    case CallConv::kIdX86GccFastCall:
      cc.setFlags(CallConv::kFlagCalleePopsStack);
      cc.setPassedOrder(kGroupGp, kZcx, kZdx);
      goto X86CallConv;

    case CallConv::kIdX86GccRegParm1:
      cc.setPassedOrder(kGroupGp, kZax);
      goto X86CallConv;

    case CallConv::kIdX86GccRegParm2:
      cc.setPassedOrder(kGroupGp, kZax, kZdx);
      goto X86CallConv;

    case CallConv::kIdX86GccRegParm3:
      cc.setPassedOrder(kGroupGp, kZax, kZdx, kZcx);
      goto X86CallConv;

    case CallConv::kIdX86CDecl:
X86CallConv:
      cc.setNaturalStackAlignment(4);
      cc.setArchType(ArchInfo::kTypeX86);
      cc.setPreservedRegs(kGroupGp, IntUtils::mask(kZbx, kZsp, kZbp, kZsi, kZdi));
      break;

    case CallConv::kIdX86Win64:
      cc.setArchType(ArchInfo::kTypeX64);
      cc.setStrategy(CallConv::kStrategyWin64);
      cc.setFlags(CallConv::kFlagPassFloatsByVec | CallConv::kFlagIndirectVecArgs);
      cc.setNaturalStackAlignment(16);
      cc.setSpillZoneSize(32);
      cc.setPassedOrder(kGroupGp, kZcx, kZdx, 8, 9);
      cc.setPassedOrder(kGroupVec, 0, 1, 2, 3);
      cc.setPreservedRegs(kGroupGp, IntUtils::mask(kZbx, kZsp, kZbp, kZsi, kZdi, 12, 13, 14, 15));
      cc.setPreservedRegs(kGroupVec, IntUtils::mask(6, 7, 8, 9, 10, 11, 12, 13, 14, 15));
      break;

    case CallConv::kIdX86SysV64:
      cc.setArchType(ArchInfo::kTypeX64);
      cc.setFlags(CallConv::kFlagPassFloatsByVec);
      cc.setNaturalStackAlignment(16);
      cc.setRedZoneSize(128);
      cc.setPassedOrder(kGroupGp, kZdi, kZsi, kZdx, kZcx, 8, 9);
      cc.setPassedOrder(kGroupVec, 0, 1, 2, 3, 4, 5, 6, 7);
      cc.setPreservedRegs(kGroupGp, IntUtils::mask(kZbx, kZsp, kZbp, 12, 13, 14, 15));
      break;

    case CallConv::kIdX86LightCall2:
    case CallConv::kIdX86LightCall3:
    case CallConv::kIdX86LightCall4: {
      uint32_t n = (ccId - CallConv::kIdX86LightCall2) + 2;

      cc.setArchType(ArchInfo::kTypeX86);
      cc.setFlags(CallConv::kFlagPassFloatsByVec);
      cc.setNaturalStackAlignment(16);
      cc.setPassedOrder(kGroupGp, kZax, kZdx, kZcx, kZsi, kZdi);
      cc.setPassedOrder(kGroupMm, 0, 1, 2, 3, 4, 5, 6, 7);
      cc.setPassedOrder(kGroupVec, 0, 1, 2, 3, 4, 5, 6, 7);

      cc.setPreservedRegs(kGroupGp , IntUtils::bits(8));
      cc.setPreservedRegs(kGroupVec, IntUtils::bits(8) & ~IntUtils::bits(n));
      cc.setPreservedRegs(kGroupMm , IntUtils::bits(8));
      cc.setPreservedRegs(kGroupK  , IntUtils::bits(8));
      break;
    }

    case CallConv::kIdX64LightCall2:
    case CallConv::kIdX64LightCall3:
    case CallConv::kIdX64LightCall4: {
      uint32_t n = (ccId - CallConv::kIdX64LightCall2) + 2;

      cc.setArchType(ArchInfo::kTypeX64);
      cc.setFlags(CallConv::kFlagPassFloatsByVec);
      cc.setNaturalStackAlignment(16);
      cc.setPassedOrder(kGroupGp, kZax, kZdx, kZcx, kZsi, kZdi);
      cc.setPassedOrder(kGroupMm, 0, 1, 2, 3, 4, 5, 6, 7);
      cc.setPassedOrder(kGroupVec, 0, 1, 2, 3, 4, 5, 6, 7);

      cc.setPreservedRegs(kGroupGp , IntUtils::bits(16));
      cc.setPreservedRegs(kGroupVec,~IntUtils::bits(n));
      cc.setPreservedRegs(kGroupMm , IntUtils::bits(8));
      cc.setPreservedRegs(kGroupK  , IntUtils::bits(8));
      break;
    }

    default:
      return DebugUtils::errored(kErrorInvalidArgument);
  }

  cc.setId(ccId);
  return kErrorOk;
}

// ============================================================================
// [asmjit::X86Internal - Helpers]
// ============================================================================

static ASMJIT_INLINE uint32_t x86GetXmmMovInst(const FuncFrame& frame) {
  bool avx = frame.isAvxEnabled();
  bool aligned = frame.hasAlignedVecSR();

  return aligned ? (avx ? X86Inst::kIdVmovaps : X86Inst::kIdMovaps)
                 : (avx ? X86Inst::kIdVmovups : X86Inst::kIdMovups);
}

static ASMJIT_INLINE uint32_t x86VecTypeIdToRegType(uint32_t typeId) noexcept {
  return typeId <= TypeId::_kVec128End ? X86Reg::kRegXmm :
         typeId <= TypeId::_kVec256End ? X86Reg::kRegYmm : X86Reg::kRegZmm;
}

// ============================================================================
// [asmjit::X86Internal - FuncDetail]
// ============================================================================

ASMJIT_FAVOR_SIZE Error X86Internal::initFuncDetail(FuncDetail& func, const FuncSignature& sign, uint32_t gpSize) noexcept {
  ASMJIT_UNUSED(sign);

  const CallConv& cc = func.getCallConv();
  uint32_t archType = cc.getArchType();
  uint32_t stackOffset = cc._spillZoneSize;

  uint32_t i;
  uint32_t argCount = func.getArgCount();

  if (func.getRetCount() != 0) {
    uint32_t typeId = func._rets[0].getTypeId();
    switch (typeId) {
      case TypeId::kI64:
      case TypeId::kU64: {
        if (archType == ArchInfo::kTypeX86) {
          // Convert a 64-bit return value to two 32-bit return values.
          func._retCount = 2;
          typeId -= 2;

          // 64-bit value is returned in EDX:EAX on X86.
          func._rets[0].initReg(X86Gp::kRegGpd, X86Gp::kIdAx, typeId);
          func._rets[1].initReg(X86Gp::kRegGpd, X86Gp::kIdDx, typeId);
          break;
        }
        else {
          func._rets[0].initReg(X86Gp::kRegGpq, X86Gp::kIdAx, typeId);
        }
        break;
      }

      case TypeId::kI8:
      case TypeId::kI16:
      case TypeId::kI32: {
        func._rets[0].initReg(X86Gp::kRegGpd, X86Gp::kIdAx, TypeId::kI32);
        break;
      }

      case TypeId::kU8:
      case TypeId::kU16:
      case TypeId::kU32: {
        func._rets[0].initReg(X86Gp::kRegGpd, X86Gp::kIdAx, TypeId::kU32);
        break;
      }

      case TypeId::kF32:
      case TypeId::kF64: {
        uint32_t regType = (archType == ArchInfo::kTypeX86) ? X86Reg::kRegFp : X86Reg::kRegXmm;
        func._rets[0].initReg(regType, 0, typeId);
        break;
      }

      case TypeId::kF80: {
        // 80-bit floats are always returned by FP0.
        func._rets[0].initReg(X86Reg::kRegFp, 0, typeId);
        break;
      }

      case TypeId::kMmx32:
      case TypeId::kMmx64: {
        // MM registers are returned through XMM or GPQ (Win64).
        uint32_t regType = X86Reg::kRegMm;
        if (archType != ArchInfo::kTypeX86)
          regType = cc.getStrategy() == CallConv::kStrategyDefault ? X86Reg::kRegXmm : X86Reg::kRegGpq;

        func._rets[0].initReg(regType, 0, typeId);
        break;
      }

      default: {
        func._rets[0].initReg(x86VecTypeIdToRegType(typeId), 0, typeId);
        break;
      }
    }
  }

  if (cc.getStrategy() == CallConv::kStrategyDefault) {
    uint32_t gpzPos = 0;
    uint32_t vecPos = 0;

    for (i = 0; i < argCount; i++) {
      FuncValue& arg = func._args[i];
      uint32_t typeId = arg.getTypeId();

      if (TypeId::isInt(typeId)) {
        uint32_t regId = gpzPos < CallConv::kMaxRegArgsPerGroup ? cc._passedOrder[X86Reg::kGroupGp].id[gpzPos] : uint8_t(Reg::kIdBad);
        if (regId != Reg::kIdBad) {
          uint32_t regType = (typeId <= TypeId::kU32) ? X86Reg::kRegGpd : X86Reg::kRegGpq;
          arg.assignRegData(regType, regId);
          func.addUsedRegs(X86Reg::kGroupGp, IntUtils::mask(regId));
          gpzPos++;
        }
        else {
          uint32_t size = std::max<uint32_t>(TypeId::sizeOf(typeId), gpSize);
          arg.assignStackOffset(int32_t(stackOffset));
          stackOffset += size;
        }
        continue;
      }

      if (TypeId::isFloat(typeId) || TypeId::isVec(typeId)) {
        uint32_t regId = vecPos < CallConv::kMaxRegArgsPerGroup ? cc._passedOrder[X86Reg::kGroupVec].id[vecPos] : uint8_t(Reg::kIdBad);

        // If this is a float, but `floatByVec` is false, we have to pass by stack.
        if (TypeId::isFloat(typeId) && !cc.hasFlag(CallConv::kFlagPassFloatsByVec))
          regId = Reg::kIdBad;

        if (regId != Reg::kIdBad) {
          arg.initTypeId(typeId);
          arg.assignRegData(x86VecTypeIdToRegType(typeId), regId);
          func.addUsedRegs(X86Reg::kGroupVec, IntUtils::mask(regId));
          vecPos++;
        }
        else {
          uint32_t size = TypeId::sizeOf(typeId);
          arg.assignStackOffset(int32_t(stackOffset));
          stackOffset += size;
        }
        continue;
      }
    }
  }

  if (cc.getStrategy() == CallConv::kStrategyWin64) {
    for (i = 0; i < argCount; i++) {
      FuncValue& arg = func._args[i];

      uint32_t typeId = arg.getTypeId();
      uint32_t size = TypeId::sizeOf(typeId);

      if (TypeId::isInt(typeId) || TypeId::isMmx(typeId)) {
        uint32_t regId = i < CallConv::kMaxRegArgsPerGroup ? cc._passedOrder[X86Reg::kGroupGp].id[i] : uint8_t(Reg::kIdBad);
        if (regId != Reg::kIdBad) {
          uint32_t regType = (size <= 4 && !TypeId::isMmx(typeId)) ? X86Reg::kRegGpd : X86Reg::kRegGpq;
          arg.assignRegData(regType, regId);
          func.addUsedRegs(X86Reg::kGroupGp, IntUtils::mask(regId));
        }
        else {
          arg.assignStackOffset(int32_t(stackOffset));
          stackOffset += gpSize;
        }
        continue;
      }

      if (TypeId::isFloat(typeId) || TypeId::isVec(typeId)) {
        uint32_t regId = i < CallConv::kMaxRegArgsPerGroup ? cc._passedOrder[X86Reg::kGroupVec].id[i] : uint8_t(Reg::kIdBad);
        if (regId != Reg::kIdBad && (TypeId::isFloat(typeId) || cc.hasFlag(CallConv::kFlagVectorCall))) {
          uint32_t regType = x86VecTypeIdToRegType(typeId);
          uint32_t regId = cc._passedOrder[X86Reg::kGroupVec].id[i];

          arg.assignRegData(regType, regId);
          func.addUsedRegs(X86Reg::kGroupVec, IntUtils::mask(regId));
        }
        else {
          arg.assignStackOffset(int32_t(stackOffset));
          stackOffset += 8; // Always 8 bytes (float/double).
        }
        continue;
      }
    }
  }

  func._argStackSize = stackOffset;
  return kErrorOk;
}

// ============================================================================
// [asmjit::X86FuncArgsContext]
// ============================================================================

static RegInfo x86GetRegForMemToMemMove(uint32_t archType, uint32_t dstTypeId, uint32_t srcTypeId) noexcept {
  uint32_t dstSize = TypeId::sizeOf(dstTypeId);
  uint32_t srcSize = TypeId::sizeOf(srcTypeId);
  uint32_t maxSize = std::max<uint32_t>(dstSize, srcSize);
  uint32_t gpSize = archType == ArchInfo::kTypeX86 ? 4 : 8;

  uint32_t signature = 0;
  if (maxSize <= gpSize || (TypeId::isInt(dstTypeId) && TypeId::isInt(srcTypeId)))
    signature = maxSize <= 4 ? X86Reg::signatureOfT<X86Reg::kRegGpd>()
                             : X86Reg::signatureOfT<X86Reg::kRegGpq>();
  else if (maxSize <= 16)
    signature = X86Reg::signatureOfT<X86Reg::kRegXmm>();
  else if (maxSize <= 32)
    signature = X86Reg::signatureOfT<X86Reg::kRegYmm>();
  else if (maxSize <= 64)
    signature = X86Reg::signatureOfT<X86Reg::kRegZmm>();

  return RegInfo { signature };
}

// Used by both `argsToFuncFrame()` and `emitArgsAssignment()`.
class X86FuncArgsContext {
public:
  enum VarId : uint32_t {
    kVarIdNone = 0xFF
  };

  //! Contains information about a single argument or SA register that may need shuffling.
  struct Var {
    inline void init(const FuncValue& cur_, const FuncValue& out_) noexcept {
      cur = cur_;
      out = out_;
    }

    //! Reset the value to its unassigned state.
    inline void reset() noexcept {
      cur.reset();
      out.reset();
    }

    inline bool isDone() const noexcept { return cur.isDone(); }
    inline void markDone() noexcept { cur.addFlags(FuncValue::kFlagIsDone); }

    FuncValue cur;
    FuncValue out;
  };

  struct WorkData {
    inline void reset() noexcept {
      _archRegs = 0;
      _workRegs = 0;
      _usedRegs = 0;
      _assignedRegs = 0;
      _dstRegs = 0;
      _dstShuf = 0;
      _numSwaps = 0;
      _numStackArgs = 0;
      std::memset(_reserved, 0, sizeof(_reserved));
      std::memset(_physToVarId, kVarIdNone, 32);
    }

    inline bool isAssigned(uint32_t regId) const noexcept {
      ASMJIT_ASSERT(regId < 32);
      return (_assignedRegs & IntUtils::mask(regId)) != 0;
    }

    inline void assign(uint32_t varId, uint32_t regId) noexcept {
      ASMJIT_ASSERT(!isAssigned(regId));
      ASMJIT_ASSERT(_physToVarId[regId] == kVarIdNone);

      _physToVarId[regId] = uint8_t(varId);
      _assignedRegs ^= IntUtils::mask(regId);
    }

    inline void reassign(uint32_t varId, uint32_t newId, uint32_t oldId) noexcept {
      ASMJIT_ASSERT( isAssigned(oldId));
      ASMJIT_ASSERT(!isAssigned(newId));
      ASMJIT_ASSERT(_physToVarId[oldId] == varId);
      ASMJIT_ASSERT(_physToVarId[newId] == kVarIdNone);

      _physToVarId[oldId] = uint8_t(kVarIdNone);
      _physToVarId[newId] = uint8_t(varId);
      _assignedRegs ^= IntUtils::mask(newId) ^ IntUtils::mask(oldId);
    }

    inline void swap(uint32_t aVarId, uint32_t aRegId, uint32_t bVarId, uint32_t bRegId) noexcept {
      ASMJIT_ASSERT(isAssigned(aRegId));
      ASMJIT_ASSERT(isAssigned(bRegId));
      ASMJIT_ASSERT(_physToVarId[aRegId] == aVarId);
      ASMJIT_ASSERT(_physToVarId[bRegId] == bVarId);

      _physToVarId[aRegId] = uint8_t(bVarId);
      _physToVarId[bRegId] = uint8_t(aVarId);
    }

    inline void unassign(uint32_t varId, uint32_t regId) noexcept {
      ASMJIT_ASSERT(isAssigned(regId));
      ASMJIT_ASSERT(_physToVarId[regId] == varId);

      _physToVarId[regId] = uint8_t(kVarIdNone);
      _assignedRegs ^= IntUtils::mask(regId);
    }

    inline uint32_t getArchRegs() const noexcept { return _archRegs; }
    inline uint32_t getWorkRegs() const noexcept { return _workRegs; }
    inline uint32_t getUsedRegs() const noexcept { return _usedRegs; }
    inline uint32_t getAssignedRegs() const noexcept { return _assignedRegs; }
    inline uint32_t getDstRegs() const noexcept { return _dstRegs; }
    inline uint32_t getAvailableRegs() const noexcept { return _workRegs & ~_assignedRegs; }

    uint32_t _archRegs;                  //!< All allocable registers provided by the architecture.
    uint32_t _workRegs;                  //!< All registers that can be used by the shuffler.
    uint32_t _usedRegs;                  //!< Registers used by the shuffler (all).
    uint32_t _assignedRegs;              //!< Assigned registers.
    uint32_t _dstRegs;                   //!< Destination registers assigned to arguments or SA.
    uint32_t _dstShuf;                   //!< Destination registers that require shuffling.
    uint8_t _numSwaps;                   //!< Number of register swaps.
    uint8_t _numStackArgs;               //!< Number of stack loads.
    uint8_t _reserved[6];                //!< Reserved (only used as padding).
    uint8_t _physToVarId[32];            //!< Physical ID to variable ID mapping.
  };

  X86FuncArgsContext() noexcept;

  inline uint32_t getArchType() const noexcept { return _archType; }
  inline uint32_t getVarCount() const noexcept { return _varCount; }

  inline Var& getVar(uint32_t varId) noexcept { return _vars[varId]; }
  inline const Var& getVar(uint32_t varId) const noexcept { return _vars[varId]; }
  inline uint32_t indexOf(const Var* var) const noexcept { return uint32_t((size_t)(var - _vars)); }

  Error initWorkData(const FuncFrame& frame, const FuncArgsAssignment& args) noexcept;
  Error markScratchRegs(FuncFrame& frame) noexcept;
  Error markDstRegsDirty(FuncFrame& frame) noexcept;
  Error markStackArgsReg(FuncFrame& frame) noexcept;

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  uint8_t _archType;
  bool _hasStackSrc;                     //!< Has arguments passed via stack (SRC).
  bool _hasPreservedFP;                  //!< Has preserved frame-pointer (FP).
  uint8_t _stackDstMask;                 //!< Has arguments assigned to stack (DST).
  uint8_t _regSwapsMask;                 //!< Register swap groups (bit-mask).
  uint8_t _saVarId;
  uint32_t _varCount;
  WorkData _workData[Reg::kGroupVirt];
  Var _vars[kFuncArgCountLoHi + 1];
};

X86FuncArgsContext::X86FuncArgsContext() noexcept {
  _archType = ArchInfo::kTypeNone;
  _varCount = 0;
  _hasStackSrc = false;
  _hasPreservedFP = false;
  _stackDstMask = 0;
  _regSwapsMask = 0;
  _saVarId = kVarIdNone;

  for (uint32_t group = 0; group < Reg::kGroupVirt; group++)
    _workData[group].reset();
}

ASMJIT_FAVOR_SIZE Error X86FuncArgsContext::initWorkData(const FuncFrame& frame, const FuncArgsAssignment& args) noexcept {
  // The code has to be updated if this changes.
  ASMJIT_ASSERT(Reg::kGroupVirt == 4);

  uint32_t i;
  const FuncDetail& func = *args.getFuncDetail();

  // Initialize ArchType.
  uint32_t archType = func.getCallConv().getArchType();
  uint32_t archRegCount = (archType == ArchInfo::kTypeX86) ? 8 : 16;

  _archType = uint8_t(archType);

  // Initialize `_archRegs`.
  _workData[X86Reg::kGroupGp ]._archRegs = IntUtils::bits(archRegCount) & ~IntUtils::mask(X86Gp::kIdSp);
  _workData[X86Reg::kGroupVec]._archRegs = IntUtils::bits(archRegCount);
  _workData[X86Reg::kGroupMm ]._archRegs = IntUtils::bits(8);
  _workData[X86Reg::kGroupK  ]._archRegs = IntUtils::bits(8);

  if (frame.hasPreservedFP())
    _workData[X86Reg::kGroupGp]._archRegs &= ~IntUtils::mask(X86Gp::kIdBp);

  // Extract information from all function arguments/assignments and build Var[] array.
  uint32_t varId = 0;
  for (i = 0; i < kFuncArgCountLoHi; i++) {
    const FuncValue& dst_ = args.getArg(i);
    if (!dst_.isAssigned()) continue;

    const FuncValue& src_ = func.getArg(i);
    if (ASMJIT_UNLIKELY(!src_.isAssigned()))
      return DebugUtils::errored(kErrorInvalidState);

    Var& var = _vars[varId];
    var.init(src_, dst_);

    FuncValue& src = var.cur;
    FuncValue& dst = var.out;

    uint32_t dstGroup = 0xFFFFFFFFU;
    uint32_t dstId = Reg::kIdBad;
    WorkData* dstWd = nullptr;

    if (dst.isReg()) {
      uint32_t dstType = dst.getRegType();
      if (ASMJIT_UNLIKELY(dstType >= X86Reg::kRegCount))
        return DebugUtils::errored(kErrorInvalidRegType);

      // Copy TypeId from source if the destination doesn't have it. The RA
      // used by CodeCompiler would never leave TypeId undefined, but users
      // of FuncAPI can just assign phys regs without specifying the type.
      if (!dst.hasTypeId())
        dst.setTypeId(X86Reg::typeIdOf(dst.getRegType()));

      dstGroup = X86Reg::groupOf(dstType);
      if (ASMJIT_UNLIKELY(dstGroup >= Reg::kGroupVirt))
        return DebugUtils::errored(kErrorInvalidRegGroup);

      dstWd = &_workData[dstGroup];
      dstId = dst.getRegId();
      if (ASMJIT_UNLIKELY(dstId >= 32 || !(dstWd->getArchRegs() & IntUtils::mask(dstId))))
        return DebugUtils::errored(kErrorInvalidPhysId);

      uint32_t dstMask = IntUtils::mask(dstId);
      if (ASMJIT_UNLIKELY(dstWd->getDstRegs() & dstMask))
        return DebugUtils::errored(kErrorOverlappedRegs);

      dstWd->_dstRegs |= dstMask;
      dstWd->_dstShuf |= dstMask;
      dstWd->_usedRegs |= dstMask;
    }
    else {
      if (!dst.hasTypeId())
        dst.setTypeId(src.getTypeId());

      RegInfo regInfo = x86GetRegForMemToMemMove(archType, dst.getTypeId(), src.getTypeId());
      if (ASMJIT_UNLIKELY(!regInfo.isValid()))
        return DebugUtils::errored(kErrorInvalidState);
      _stackDstMask = uint8_t(_stackDstMask | IntUtils::mask(regInfo.getGroup()));
    }

    if (src.isReg()) {
      uint32_t srcId = src.getRegId();
      uint32_t srcGroup = X86Reg::groupOf(src.getRegType());

      if (dstGroup == srcGroup) {
        dstWd->assign(varId, srcId);

        // The best case, register is allocated where it is expected to be.
        if (dstId == srcId)
          var.markDone();
      }
      else {
        if (ASMJIT_UNLIKELY(srcGroup >= Reg::kGroupVirt))
          return DebugUtils::errored(kErrorInvalidState);

        WorkData& srcData = _workData[srcGroup];
        srcData.assign(varId, srcId);
      }
    }
    else {
      if (dstWd)
        dstWd->_numStackArgs++;
      _hasStackSrc = true;
    }

    varId++;
  }

  // Initialize WorkData::workRegs.
  for (i = 0; i < Reg::kGroupVirt; i++)
    _workData[i]._workRegs = (_workData[i].getArchRegs() & (frame.getDirtyRegs(i) | ~frame.getPreservedRegs(i))) | _workData[i].getDstRegs() | _workData[i].getAssignedRegs();

  // Create a variable that represents `SARegId` if necessary.
  bool saRegRequired = _hasStackSrc && frame.hasDynamicAlignment() && !frame.hasPreservedFP();

  WorkData& gpRegs = _workData[Reg::kGroupGp];
  uint32_t saCurRegId = frame.getSARegId();
  uint32_t saOutRegId = args.getSARegId();

  if (saCurRegId != Reg::kIdBad) {
    // Check if the provided `SARegId` doesn't collide with input registers.
    if (ASMJIT_UNLIKELY(gpRegs.isAssigned(saCurRegId)))
      return DebugUtils::errored(kErrorOverlappedRegs);
  }

  if (saOutRegId != Reg::kIdBad) {
    // Check if the provided `SARegId` doesn't collide with argument assignments.
    if (ASMJIT_UNLIKELY((gpRegs.getDstRegs() & IntUtils::mask(saOutRegId)) != 0))
      return DebugUtils::errored(kErrorOverlappedRegs);
    saRegRequired = true;
  }

  if (saRegRequired) {
    uint32_t ptrTypeId = (archType == ArchInfo::kTypeX86) ? TypeId::kU32 : TypeId::kU64;
    uint32_t ptrRegType = (archType == ArchInfo::kTypeX86) ? Reg::kRegGp32 : Reg::kRegGp64;

    _saVarId = uint8_t(varId);
    _hasPreservedFP = frame.hasPreservedFP();

    Var& var = _vars[varId];
    var.reset();

    if (saCurRegId == Reg::kIdBad) {
      if (saOutRegId != Reg::kIdBad && !gpRegs.isAssigned(saOutRegId)) {
        saCurRegId = saOutRegId;
      }
      else {
        uint32_t availableRegs = gpRegs.getAvailableRegs();
        if (!availableRegs)
          availableRegs = gpRegs.getArchRegs() & ~gpRegs.getWorkRegs();

        if (ASMJIT_UNLIKELY(!availableRegs))
          return DebugUtils::errored(kErrorNoMorePhysRegs);

        saCurRegId = IntUtils::ctz(availableRegs);
      }
    }

    var.cur.initReg(ptrRegType, saCurRegId, ptrTypeId);
    gpRegs.assign(varId, saCurRegId);
    gpRegs._workRegs |= IntUtils::mask(saCurRegId);

    if (saOutRegId != Reg::kIdBad) {
      var.out.initReg(ptrRegType, saOutRegId, ptrTypeId);
      gpRegs._dstRegs |= IntUtils::mask(saOutRegId);
      gpRegs._workRegs |= IntUtils::mask(saOutRegId);
    }
    else {
      var.markDone();
    }

    varId++;
  }

  _varCount = varId;

  // Detect register swaps.
  for (varId = 0; varId < _varCount; varId++) {
    Var& var = _vars[varId];
    if (var.cur.isReg() && var.out.isReg()) {
      uint32_t srcId = var.cur.getRegId();
      uint32_t dstId = var.out.getRegId();

      uint32_t group = X86Reg::groupOf(var.cur.getRegType());
      if (group != X86Reg::groupOf(var.out.getRegType()))
        continue;

      WorkData& wd = _workData[group];
      if (wd.isAssigned(dstId)) {
        Var& other = _vars[wd._physToVarId[dstId]];
        if (X86Reg::groupOf(other.out.getRegType()) == group && other.out.getRegId() == srcId) {
          wd._numSwaps++;
          _regSwapsMask |= IntUtils::mask(group);
        }
      }
    }
  }

  return kErrorOk;
}

ASMJIT_FAVOR_SIZE Error X86FuncArgsContext::markDstRegsDirty(FuncFrame& frame) noexcept {
  for (uint32_t i = 0; i < Reg::kGroupVirt; i++) {
    WorkData& wd = _workData[i];
    uint32_t regs = wd.getUsedRegs() | wd._dstShuf;

    wd._workRegs |= regs;
    frame.addDirtyRegs(i, regs);
  }

  return kErrorOk;
}

ASMJIT_FAVOR_SIZE Error X86FuncArgsContext::markScratchRegs(FuncFrame& frame) noexcept {
  uint32_t groupMask = 0;

  // Handle stack to stack moves.
  groupMask |= _stackDstMask;

  // Handle register swaps.
  groupMask |= _regSwapsMask & ~IntUtils::mask(Reg::kGroupGp);

  if (!groupMask)
    return kErrorOk;

  // selects one dirty register per affected group that can be used as a scratch register.
  for (uint32_t group = 0; group < Reg::kGroupVirt; group++) {
    if (groupMask & (1U << group)) {
      WorkData& wd = _workData[group];

      // Initially, pick some clobbered or dirty register.
      uint32_t workRegs = wd.getWorkRegs();
      uint32_t regs = workRegs & ~(wd.getUsedRegs() | wd._dstShuf);

      // If that didn't work out pick some register which is not in 'used'.
      if (!regs) regs = workRegs & ~wd.getUsedRegs();

      // If that didn't work out pick any other register that is allocable.
      // This last resort case will, however, result in marking one more
      // register dirty.
      if (!regs) regs = wd.getArchRegs() & ~workRegs;

      // If that didn't work out we will have to use XORs instead of MOVs.
      if (!regs) continue;

      uint32_t regMask = IntUtils::blsi(regs);
      wd._workRegs |= regMask;
      frame.addDirtyRegs(group, regMask);
    }
  }

  return kErrorOk;
}

ASMJIT_FAVOR_SIZE Error X86FuncArgsContext::markStackArgsReg(FuncFrame& frame) noexcept {
  // TODO: Validate, improve...
  if (_saVarId != kVarIdNone) {
    const Var& var = _vars[_saVarId];
    frame.setSARegId(var.cur.getRegId());
  }
  else if (frame.hasPreservedFP()) {
    // Always EBP|RBP if the frame-pointer isn't omitted.
    frame.setSARegId(X86Gp::kIdBp);
  }

  return kErrorOk;
}

// ============================================================================
// [asmjit::X86Internal - FrameLayout]
// ============================================================================

ASMJIT_FAVOR_SIZE Error X86Internal::initFuncFrame(FuncFrame& frame, const FuncDetail& func) noexcept {
  uint32_t archType = func.getCallConv().getArchType();

  // Initializing FuncFrame means making a copy of some properties of `func`.
  // Properties like `_localStackSize` will be set by the user before the frame
  // is finalized.
  frame.reset();

  frame._archType = uint8_t(archType);
  frame._spRegId = X86Gp::kIdSp;
  frame._saRegId = X86Gp::kIdBad;

  uint32_t naturalStackAlignment = func.getCallConv().getNaturalStackAlignment();
  uint32_t minimumDynamicAlignment = std::max<uint32_t>(naturalStackAlignment, 16);

  if (minimumDynamicAlignment == naturalStackAlignment)
    minimumDynamicAlignment <<= 1;

  frame._naturalStackAlignment = uint8_t(naturalStackAlignment);
  frame._minimumDynamicAlignment = uint8_t(minimumDynamicAlignment);
  frame._redZoneSize = uint8_t(func.getRedZoneSize());
  frame._spillZoneSize = uint8_t(func.getSpillZoneSize());
  frame._finalStackAlignment = uint8_t(frame._naturalStackAlignment);

  if (func.hasFlag(CallConv::kFlagCalleePopsStack)) {
    frame._calleeStackCleanup = uint16_t(func.getArgStackSize());
  }

  // Initial masks of dirty and preserved registers.
  for (uint32_t group = 0; group < Reg::kGroupVirt; group++) {
    frame._dirtyRegs[group] = func.getUsedRegs(group);
    frame._preservedRegs[group] = func.getPreservedRegs(group);
  }

  // Exclude ESP/RSP - this register is never included in saved GP regs.
  frame._preservedRegs[Reg::kGroupGp] &= ~IntUtils::mask(X86Gp::kIdSp);

  return kErrorOk;
}

ASMJIT_FAVOR_SIZE Error X86Internal::finalizeFuncFrame(FuncFrame& frame) noexcept {
  uint32_t gpSize = frame.getArchType() == ArchInfo::kTypeX86 ? 4 : 8;

  // The final stack alignment must be updated accordingly to call and local stack alignments.
  uint32_t stackAlignment = frame._finalStackAlignment;
  ASMJIT_ASSERT(stackAlignment == std::max(frame._naturalStackAlignment,
                                  std::max(frame._callStackAlignment,
                                           frame._localStackAlignment)));

  // TODO: Must be configurable.
  uint32_t vecSize = 16;

  bool hasFP = frame.hasPreservedFP();
  bool hasDA = frame.hasDynamicAlignment();

  // Include EBP|RBP if the function preserves the frame-pointer.
  if (hasFP)
    frame._dirtyRegs[X86Reg::kGroupGp] |= IntUtils::mask(X86Gp::kIdBp);

  // These two are identical if the function doesn't align its stack dynamically.
  uint32_t saRegId = frame.getSARegId();
  if (saRegId == Reg::kIdBad)
    saRegId = X86Gp::kIdSp;

  // Fix stack arguments base-register from ESP|RSP to EBP|RBP in case it was
  // not picked before and the function performs dynamic stack alignment.
  if (hasDA && saRegId == X86Gp::kIdSp)
    saRegId = X86Gp::kIdBp;

  // Mark as dirty any register but ESP|RSP if used as SA pointer.
  if (saRegId != X86Gp::kIdSp)
    frame._dirtyRegs[X86Reg::kGroupGp] |= IntUtils::mask(saRegId);

  frame._spRegId = uint8_t(X86Gp::kIdSp);
  frame._saRegId = uint8_t(saRegId);

  // Setup stack size used to save preserved registers.
  frame._gpSaveSize    = uint16_t(IntUtils::popcnt(frame.getSavedRegs(X86Reg::kGroupGp )) * gpSize);
  frame._nonGpSaveSize = uint16_t(IntUtils::popcnt(frame.getSavedRegs(X86Reg::kGroupVec)) * vecSize +
                                  IntUtils::popcnt(frame.getSavedRegs(X86Reg::kGroupMm )) * 8 +
                                  IntUtils::popcnt(frame.getSavedRegs(X86Reg::kGroupK  )) * 8);

  uint32_t v = 0;                             // The beginning of the stack frame relative to SP after prolog.
  v += frame.getCallStackSize();              // Count 'callStackSize'    <- This is used to call functions.
  v  = IntUtils::alignUp(v, stackAlignment);  // Align to function's stack alignment.

  frame._localStackOffset = v;                // Store 'localStackOffset' <- Function's local stack starts here..
  v += frame.getLocalStackSize();             // Count 'localStackSize'   <- Function's local stack ends here.

  // If the function's stack must be aligned, calculate the alignment necessary
  // to store vector registers, and set `FuncFrame::kAttrAlignedVecSR` to inform
  // PEI that it can use instructions to perform aligned stores/loads.
  if (stackAlignment >= vecSize && frame._nonGpSaveSize) {
    frame.addAttributes(FuncFrame::kAttrAlignedVecSR);
    v = IntUtils::alignUp(v, vecSize);        // Align '_nonGpSaveOffset'.
  }

  frame._nonGpSaveOffset = v;                 // Store '_nonGpSaveOffset' <- Non-GP Save/Restore starts here.
  v += frame._nonGpSaveSize;                  // Count '_nonGpSaveSize'   <- Non-GP Save/Restore ends here.

  // Calculate if dynamic alignment (DA) slot (stored as offset relative to SP) is required and its offset.
  if (hasDA && !hasFP) {
    frame._daOffset = v;                      // Store 'daOffset'         <- DA pointer would be stored here.
    v += gpSize;                              // Count 'daOffset'.
  }
  else {
    frame._daOffset = FuncFrame::kTagInvalidOffset;
  }

  // The return address should be stored after GP save/restore regs. It has
  // the same size as `gpSize` (basically the native register/pointer size).
  // We don't adjust it now as `v` now contains the exact size that the
  // function requires to adjust (call frame + stack frame, vec stack size).
  // The stack (if we consider this size) is misaligned now, as it's always
  // aligned before the function call - when `call()` is executed it pushes
  // the current EIP|RIP onto the stack, and misaligns it by 12 or 8 bytes
  // (depending on the architecture). So count number of bytes needed to align
  // it up to the function's CallFrame (the beginning).
  if (v || frame.hasFuncCalls())
    v += IntUtils::alignDiff(v + frame.getGpSaveSize() + gpSize, stackAlignment);

  frame._gpSaveOffset = v;                    // Store 'gpSaveOffset'     <- Function's GP Save/Restore starts here.
  frame._stackAdjustment = v;                 // Store 'stackAdjustment'  <- SA used by 'add zsp, SA' and 'sub zsp, SA'.

  v += frame._gpSaveSize;                     // Count 'gpSaveSize'       <- Function's GP Save/Restore ends here.
  v += gpSize;                                // Count 'ReturnAddress'    <- As CALL pushes onto stack.

  // If the function performs dynamic stack alignment then the stack-adjustment must be aligned.
  if (hasDA)
    frame._stackAdjustment = IntUtils::alignUp(frame._stackAdjustment, stackAlignment);

  uint32_t saInvOff = FuncFrame::kTagInvalidOffset;
  uint32_t saTmpOff = gpSize + frame._gpSaveSize;

  // Calculate where the function arguments start relative to SP.
  frame._saOffsetFromSP = hasDA ? saInvOff : v;

  // Calculate where the function arguments start relative to FP or user-provided register.
  frame._saOffsetFromSA = hasFP ? gpSize * 2  // Return address + frame pointer.
                                : saTmpOff;   // Return address + all saved GP regs.

  return kErrorOk;
}

// ============================================================================
// [asmjit::X86Internal - ArgsToFrameInfo]
// ============================================================================

ASMJIT_FAVOR_SIZE Error X86Internal::argsToFuncFrame(const FuncArgsAssignment& args, FuncFrame& frame) noexcept {
  X86FuncArgsContext ctx;
  ASMJIT_PROPAGATE(ctx.initWorkData(frame, args));
  ASMJIT_PROPAGATE(ctx.markDstRegsDirty(frame));
  ASMJIT_PROPAGATE(ctx.markScratchRegs(frame));
  ASMJIT_PROPAGATE(ctx.markStackArgsReg(frame));
  return kErrorOk;
}

// ============================================================================
// [asmjit::X86Internal - Emit Helpers]
// ============================================================================

ASMJIT_FAVOR_SIZE Error X86Internal::emitRegMove(X86Emitter* emitter,
  const Operand_& dst_,
  const Operand_& src_, uint32_t typeId, bool avxEnabled, const char* comment) {

  // Invalid or abstract TypeIds are not allowed.
  ASMJIT_ASSERT(TypeId::isValid(typeId) && !TypeId::isAbstract(typeId));

  Operand dst(dst_);
  Operand src(src_);

  uint32_t instId = Inst::kIdNone;
  uint32_t memFlags = 0;

  enum MemFlags : uint32_t {
    kDstMem = 0x1,
    kSrcMem = 0x2
  };

  // Detect memory operands and patch them to have the same size as the register.
  // CodeCompiler always sets memory size of allocs and spills, so it shouldn't
  // be really necessary, however, after this function was separated from Compiler
  // it's better to make sure that the size is always specified, as we can use
  // 'movzx' and 'movsx' that rely on it.
  if (dst.isMem()) { memFlags |= kDstMem; dst.as<X86Mem>().setSize(src.getSize()); }
  if (src.isMem()) { memFlags |= kSrcMem; src.as<X86Mem>().setSize(dst.getSize()); }

  switch (typeId) {
    case TypeId::kI8:
    case TypeId::kU8:
    case TypeId::kI16:
    case TypeId::kU16:
      // Special case - 'movzx' load.
      if (memFlags & kSrcMem) {
        instId = X86Inst::kIdMovzx;
        dst.setSignature(X86Reg::signatureOfT<X86Reg::kRegGpd>());
      }
      else if (!memFlags) {
        // Change both destination and source registers to GPD (safer, no dependencies).
        dst.setSignature(X86Reg::signatureOfT<X86Reg::kRegGpd>());
        src.setSignature(X86Reg::signatureOfT<X86Reg::kRegGpd>());
      }
      ASMJIT_FALLTHROUGH;

    case TypeId::kI32:
    case TypeId::kU32:
    case TypeId::kI64:
    case TypeId::kU64:
      instId = X86Inst::kIdMov;
      break;

    case TypeId::kMmx32:
      instId = X86Inst::kIdMovd;
      if (memFlags) break;
      ASMJIT_FALLTHROUGH;

    case TypeId::kMmx64 : instId = X86Inst::kIdMovq ; break;
    case TypeId::kMask8 : instId = X86Inst::kIdKmovb; break;
    case TypeId::kMask16: instId = X86Inst::kIdKmovw; break;
    case TypeId::kMask32: instId = X86Inst::kIdKmovd; break;
    case TypeId::kMask64: instId = X86Inst::kIdKmovq; break;

    default: {
      uint32_t elementTypeId = TypeId::elementOf(typeId);
      if (TypeId::isVec32(typeId) && memFlags) {
        if (elementTypeId == TypeId::kF32)
          instId = avxEnabled ? X86Inst::kIdVmovss : X86Inst::kIdMovss;
        else
          instId = avxEnabled ? X86Inst::kIdVmovd : X86Inst::kIdMovd;
        break;
      }

      if (TypeId::isVec64(typeId) && memFlags) {
        if (elementTypeId == TypeId::kF64)
          instId = avxEnabled ? X86Inst::kIdVmovsd : X86Inst::kIdMovsd;
        else
          instId = avxEnabled ? X86Inst::kIdVmovq : X86Inst::kIdMovq;
        break;
      }

      if (elementTypeId == TypeId::kF32)
        instId = avxEnabled ? X86Inst::kIdVmovaps : X86Inst::kIdMovaps;
      else if (elementTypeId == TypeId::kF64)
        instId = avxEnabled ? X86Inst::kIdVmovapd : X86Inst::kIdMovapd;
      else if (typeId <= TypeId::_kVec256End)
        instId = avxEnabled ? X86Inst::kIdVmovdqa : X86Inst::kIdMovdqa;
      else if (elementTypeId <= TypeId::kU32)
        instId = X86Inst::kIdVmovdqa32;
      else
        instId = X86Inst::kIdVmovdqa64;
      break;
    }
  }

  if (!instId)
    return DebugUtils::errored(kErrorInvalidState);

  emitter->setInlineComment(comment);
  return emitter->emit(instId, dst, src);
}

ASMJIT_FAVOR_SIZE Error X86Internal::emitArgMove(X86Emitter* emitter,
  const X86Reg& dst_, uint32_t dstTypeId,
  const Operand_& src_, uint32_t srcTypeId, bool avxEnabled, const char* comment) {

  // Deduce optional `dstTypeId`, which may be `TypeId::kVoid` in some cases.
  if (!dstTypeId) dstTypeId = x86OpData.archRegs.regTypeToTypeId[dst_.getType()];

  // Invalid or abstract TypeIds are not allowed.
  ASMJIT_ASSERT(TypeId::isValid(dstTypeId) && !TypeId::isAbstract(dstTypeId));
  ASMJIT_ASSERT(TypeId::isValid(srcTypeId) && !TypeId::isAbstract(srcTypeId));

  X86Reg dst(dst_);
  Operand src(src_);

  uint32_t dstSize = TypeId::sizeOf(dstTypeId);
  uint32_t srcSize = TypeId::sizeOf(srcTypeId);

  uint32_t instId = Inst::kIdNone;

  // Not a real loop, just 'break' is nicer than 'goto'.
  for (;;) {
    if (TypeId::isInt(dstTypeId)) {
      if (TypeId::isInt(srcTypeId)) {
        instId = X86Inst::kIdMovsx;
        uint32_t typeOp = (dstTypeId << 8) | srcTypeId;

        // Sign extend by using 'movsx'.
        if (typeOp == ((TypeId::kI16 << 8) | TypeId::kI8 ) ||
            typeOp == ((TypeId::kI32 << 8) | TypeId::kI8 ) ||
            typeOp == ((TypeId::kI32 << 8) | TypeId::kI16) ||
            typeOp == ((TypeId::kI64 << 8) | TypeId::kI8 ) ||
            typeOp == ((TypeId::kI64 << 8) | TypeId::kI16)) break;

        // Sign extend by using 'movsxd'.
        instId = X86Inst::kIdMovsxd;
        if (typeOp == ((TypeId::kI64 << 8) | TypeId::kI32)) break;
      }

      if (TypeId::isInt(srcTypeId) || src_.isMem()) {
        // Zero extend by using 'movzx' or 'mov'.
        if (dstSize <= 4 && srcSize < 4) {
          instId = X86Inst::kIdMovzx;
          dst.setSignature(X86Reg::signatureOfT<X86Reg::kRegGpd>());
        }
        else {
          // We should have caught all possibilities where `srcSize` is less
          // than 4, so we don't have to worry about 'movzx' anymore. Minimum
          // size is enough to determine if we want 32-bit or 64-bit move.
          instId = X86Inst::kIdMov;
          srcSize = std::min(srcSize, dstSize);

          dst.setSignature(srcSize == 4 ? X86Reg::signatureOfT<X86Reg::kRegGpd>()
                                        : X86Reg::signatureOfT<X86Reg::kRegGpq>());
          if (src.isReg()) src.setSignature(dst.getSignature());
        }
        break;
      }

      // NOTE: The previous branch caught all memory sources, from here it's
      // always register to register conversion, so catch the remaining cases.
      srcSize = std::min(srcSize, dstSize);

      if (TypeId::isMmx(srcTypeId)) {
        // 64-bit move.
        instId = X86Inst::kIdMovq;
        if (srcSize == 8) break;

        // 32-bit move.
        instId = X86Inst::kIdMovd;
        dst.setSignature(X86Reg::signatureOfT<X86Reg::kRegGpd>());
        break;
      }

      if (TypeId::isMask(srcTypeId)) {
        instId = X86Inst::kmovIdFromSize(srcSize);
        dst.setSignature(srcSize <= 4 ? X86Reg::signatureOfT<X86Reg::kRegGpd>()
                                      : X86Reg::signatureOfT<X86Reg::kRegGpq>());
        break;
      }

      if (TypeId::isVec(srcTypeId)) {
        // 64-bit move.
        instId = avxEnabled ? X86Inst::kIdVmovq : X86Inst::kIdMovq;
        if (srcSize == 8) break;

        // 32-bit move.
        instId = avxEnabled ? X86Inst::kIdVmovd : X86Inst::kIdMovd;
        dst.setSignature(X86Reg::signatureOfT<X86Reg::kRegGpd>());
        break;
      }
    }

    if (TypeId::isMmx(dstTypeId)) {
      instId = X86Inst::kIdMovq;
      srcSize = std::min(srcSize, dstSize);

      if (TypeId::isInt(srcTypeId) || src.isMem()) {
        // 64-bit move.
        if (srcSize == 8) break;

        // 32-bit move.
        instId = X86Inst::kIdMovd;
        if (src.isReg()) src.setSignature(X86Reg::signatureOfT<X86Reg::kRegGpd>());
        break;
      }

      if (TypeId::isMmx(srcTypeId)) break;

      // This will hurt if `avxEnabled`.
      instId = X86Inst::kIdMovdq2q;
      if (TypeId::isVec(srcTypeId)) break;
    }

    if (TypeId::isMask(dstTypeId)) {
      srcSize = std::min(srcSize, dstSize);

      if (TypeId::isInt(srcTypeId) || TypeId::isMask(srcTypeId) || src.isMem()) {
        instId = X86Inst::kmovIdFromSize(srcSize);
        if (X86Reg::isGp(src) && srcSize <= 4) src.setSignature(X86Reg::signatureOfT<X86Reg::kRegGpd>());
        break;
      }
    }

    if (TypeId::isVec(dstTypeId)) {
      // By default set destination to XMM, will be set to YMM|ZMM if needed.
      dst.setSignature(X86Reg::signatureOfT<X86Reg::kRegXmm>());

      // This will hurt if `avxEnabled`.
      if (X86Reg::isMm(src)) {
        // 64-bit move.
        instId = X86Inst::kIdMovq2dq;
        break;
      }

      // Argument conversion.
      uint32_t dstElement = TypeId::elementOf(dstTypeId);
      uint32_t srcElement = TypeId::elementOf(srcTypeId);

      if (dstElement == TypeId::kF32 && srcElement == TypeId::kF64) {
        srcSize = std::min(dstSize * 2, srcSize);
        dstSize = srcSize / 2;

        if (srcSize <= 8)
          instId = avxEnabled ? X86Inst::kIdVcvtss2sd : X86Inst::kIdCvtss2sd;
        else
          instId = avxEnabled ? X86Inst::kIdVcvtps2pd : X86Inst::kIdCvtps2pd;

        if (dstSize == 32)
          dst.setSignature(X86Reg::signatureOfT<X86Reg::kRegYmm>());
        if (src.isReg())
          src.setSignature(X86Reg::signatureOfVecBySize(srcSize));
        break;
      }

      if (dstElement == TypeId::kF64 && srcElement == TypeId::kF32) {
        srcSize = std::min(dstSize, srcSize * 2) / 2;
        dstSize = srcSize * 2;

        if (srcSize <= 4)
          instId = avxEnabled ? X86Inst::kIdVcvtsd2ss : X86Inst::kIdCvtsd2ss;
        else
          instId = avxEnabled ? X86Inst::kIdVcvtpd2ps : X86Inst::kIdCvtpd2ps;

        dst.setSignature(X86Reg::signatureOfVecBySize(dstSize));
        if (src.isReg() && srcSize >= 32)
          src.setSignature(X86Reg::signatureOfT<X86Reg::kRegYmm>());
        break;
      }

      srcSize = std::min(srcSize, dstSize);
      if (X86Reg::isGp(src) || src.isMem()) {
        // 32-bit move.
        if (srcSize <= 4) {
          instId = avxEnabled ? X86Inst::kIdVmovd : X86Inst::kIdMovd;
          if (src.isReg()) src.setSignature(X86Reg::signatureOfT<X86Reg::kRegGpd>());
          break;
        }

        // 64-bit move.
        if (srcSize == 8) {
          instId = avxEnabled ? X86Inst::kIdVmovq : X86Inst::kIdMovq;
          break;
        }
      }

      if (X86Reg::isVec(src) || src.isMem()) {
        instId = avxEnabled ? X86Inst::kIdVmovaps : X86Inst::kIdMovaps;
        uint32_t sign = X86Reg::signatureOfVecBySize(srcSize);

        dst.setSignature(sign);
        if (src.isReg()) src.setSignature(sign);
        break;
      }
    }

    return DebugUtils::errored(kErrorInvalidState);
  }

  if (src.isMem())
    src.as<X86Mem>().setSize(srcSize);

  emitter->setInlineComment(comment);
  return emitter->emit(instId, dst, src);
}

// ============================================================================
// [asmjit::X86Internal - Emit Prolog & Epilog]
// ============================================================================

static ASMJIT_INLINE void X86Internal_setupSaveRestoreInfo(uint32_t group, const FuncFrame& frame, X86Reg& xReg, uint32_t& xInst, uint32_t& xSize) noexcept {
  switch (group) {
    case X86Reg::kGroupVec:
      xReg = x86::xmm(0);
      xInst = x86GetXmmMovInst(frame);
      xSize = xReg.getSize();
      break;
    case X86Reg::kGroupMm:
      xReg = x86::mm(0);
      xInst = X86Inst::kIdMovq;
      xSize = xReg.getSize();
      break;
    case X86Reg::kGroupK:
      xReg = x86::k(0);
      xInst = X86Inst::kIdKmovq;
      xSize = xReg.getSize();
      break;
    default:
      ASMJIT_NOT_REACHED();
  }
}

ASMJIT_FAVOR_SIZE Error X86Internal::emitProlog(X86Emitter* emitter, const FuncFrame& frame) {
  uint32_t gpSaved = frame.getSavedRegs(X86Reg::kGroupGp);

  X86Gp zsp = emitter->zsp();   // ESP|RSP register.
  X86Gp zbp = emitter->zsp();   // EBP|RBP register.
  zbp.setId(X86Gp::kIdBp);

  X86Gp gpReg = emitter->zsp(); // General purpose register (temporary).
  X86Gp saReg = emitter->zsp(); // Stack-arguments base pointer.

  // Emit: 'push zbp'
  //       'mov  zbp, zsp'.
  if (frame.hasPreservedFP()) {
    gpSaved &= ~IntUtils::mask(X86Gp::kIdBp);
    ASMJIT_PROPAGATE(emitter->push(zbp));
    ASMJIT_PROPAGATE(emitter->mov(zbp, zsp));
  }

  // Emit: 'push gp' sequence.
  {
    IntUtils::BitWordIterator<uint32_t> it(gpSaved);
    while (it.hasNext()) {
      gpReg.setId(it.next());
      ASMJIT_PROPAGATE(emitter->push(gpReg));
    }
  }

  // Emit: 'mov saReg, zsp'.
  uint32_t saRegId = frame.getSARegId();
  if (saRegId != Reg::kIdBad && saRegId != X86Gp::kIdSp) {
    saReg.setId(saRegId);
    if (frame.hasPreservedFP()) {
      if (saRegId != X86Gp::kIdBp)
        ASMJIT_PROPAGATE(emitter->mov(saReg, zbp));
    }
    else {
      ASMJIT_PROPAGATE(emitter->mov(saReg, zsp));
    }
  }

  // Emit: 'and zsp, StackAlignment'.
  if (frame.hasDynamicAlignment()) {
    ASMJIT_PROPAGATE(emitter->and_(zsp, -int32_t(frame.getFinalStackAlignment())));
  }

  // Emit: 'sub zsp, StackAdjustment'.
  if (frame.hasStackAdjustment()) {
    ASMJIT_PROPAGATE(emitter->sub(zsp, frame.getStackAdjustment()));
  }

  // Emit: 'mov [zsp + DAOffset], saReg'.
  if (frame.hasDynamicAlignment() && frame.hasDAOffset()) {
    X86Mem saMem = x86::ptr(zsp, int32_t(frame.getDAOffset()));
    ASMJIT_PROPAGATE(emitter->mov(saMem, saReg));
  }

  // Emit 'movxxx [zsp + X], {[x|y|z]mm, k}'.
  {
    X86Reg xReg;
    X86Mem xBase = x86::ptr(zsp, int32_t(frame.getNonGpSaveOffset()));

    uint32_t xInst;
    uint32_t xSize;

    for (uint32_t group = 1; group < Reg::kGroupVirt; group++) {
      IntUtils::BitWordIterator<uint32_t> it(frame.getSavedRegs(group));
      if (it.hasNext()) {
        X86Internal_setupSaveRestoreInfo(group, frame, xReg, xInst, xSize);
        do {
          xReg.setId(it.next());
          ASMJIT_PROPAGATE(emitter->emit(xInst, xBase, xReg));
          xBase.addOffsetLo32(int32_t(xSize));
        } while (it.hasNext());
      }
    }
  }

  return kErrorOk;
}

ASMJIT_FAVOR_SIZE Error X86Internal::emitEpilog(X86Emitter* emitter, const FuncFrame& frame) {
  uint32_t i;
  uint32_t regId;

  uint32_t gpSize = emitter->getGpSize();
  uint32_t gpSaved = frame.getSavedRegs(X86Reg::kGroupGp);

  X86Gp zsp = emitter->zsp();   // ESP|RSP register.
  X86Gp zbp = emitter->zsp();   // EBP|RBP register.
  zbp.setId(X86Gp::kIdBp);

  X86Gp gpReg = emitter->zsp(); // General purpose register (temporary).

  // Don't emit 'pop zbp' in the pop sequence, this case is handled separately.
  if (frame.hasPreservedFP())
    gpSaved &= ~IntUtils::mask(X86Gp::kIdBp);

  // Emit 'movxxx {[x|y|z]mm, k}, [zsp + X]'.
  {
    X86Reg xReg;
    X86Mem xBase = x86::ptr(zsp, int32_t(frame.getNonGpSaveOffset()));

    uint32_t xInst;
    uint32_t xSize;

    for (uint32_t group = 1; group < Reg::kGroupVirt; group++) {
      IntUtils::BitWordIterator<uint32_t> it(frame.getSavedRegs(group));
      if (it.hasNext()) {
        X86Internal_setupSaveRestoreInfo(group, frame, xReg, xInst, xSize);
        do {
          xReg.setId(it.next());
          ASMJIT_PROPAGATE(emitter->emit(xInst, xReg, xBase));
          xBase.addOffsetLo32(int32_t(xSize));
        } while (it.hasNext());
      }
    }
  }

  // Emit 'emms' and/or 'vzeroupper'.
  if (frame.hasMmxCleanup()) ASMJIT_PROPAGATE(emitter->emms());
  if (frame.hasAvxCleanup()) ASMJIT_PROPAGATE(emitter->vzeroupper());

  if (frame.hasPreservedFP()) {
    // Emit 'mov zsp, zbp' or 'lea zsp, [zbp - x]'
    int32_t count = int32_t(frame.getGpSaveSize() - gpSize);
    if (!count)
      ASMJIT_PROPAGATE(emitter->mov(zsp, zbp));
    else
      ASMJIT_PROPAGATE(emitter->lea(zsp, x86::ptr(zbp, -count)));
  }
  else {
    if (frame.hasDynamicAlignment() && frame.hasDAOffset()) {
      // Emit 'mov zsp, [zsp + DsaSlot]'.
      X86Mem saMem = x86::ptr(zsp, int32_t(frame.getDAOffset()));
      ASMJIT_PROPAGATE(emitter->mov(zsp, saMem));
    }
    else if (frame.hasStackAdjustment()) {
      // Emit 'add zsp, StackAdjustment'.
      ASMJIT_PROPAGATE(emitter->add(zsp, int32_t(frame.getStackAdjustment())));
    }
  }

  // Emit 'pop gp' sequence.
  if (gpSaved) {
    i = gpSaved;
    regId = 16;

    do {
      regId--;
      if (i & 0x8000) {
        gpReg.setId(regId);
        ASMJIT_PROPAGATE(emitter->pop(gpReg));
      }
      i <<= 1;
    } while (regId != 0);
  }

  // Emit 'pop zbp'.
  if (frame.hasPreservedFP())
    ASMJIT_PROPAGATE(emitter->pop(zbp));

  // Emit 'ret' or 'ret x'.
  if (frame.hasCalleeStackCleanup())
    ASMJIT_PROPAGATE(emitter->emit(X86Inst::kIdRet, int(frame.getCalleeStackCleanup())));
  else
    ASMJIT_PROPAGATE(emitter->emit(X86Inst::kIdRet));

  return kErrorOk;
}

// ============================================================================
// [asmjit::X86Internal - Emit Arguments Assignment]
// ============================================================================

static void dumpFuncValue(StringBuilder& sb, uint32_t archType, const FuncValue& value) noexcept {
  Logging::formatTypeId(sb, value.getTypeId());
  sb.appendChar('@');
  if (value.isReg()) {
    Logging::formatRegister(sb, 0, nullptr, archType, value.getRegType(), value.getRegId());
  }
  else if (value.isStack()) {
    sb.appendFormat("[%d]", value.getStackOffset());
  }
  else {
    sb.appendString("<none>");
  }
}

static void dumpAssignment(StringBuilder& sb, const X86FuncArgsContext& ctx) noexcept {
  typedef X86FuncArgsContext::Var Var;

  uint32_t archType = ctx.getArchType();
  uint32_t varCount = ctx.getVarCount();

  for (uint32_t i = 0; i < varCount; i++) {
    const Var& var = ctx.getVar(i);
    const FuncValue& dst = var.out;
    const FuncValue& cur = var.cur;

    sb.appendFormat("Var%u: ", i);
    dumpFuncValue(sb, archType, dst);
    sb.appendString(" <- ");
    dumpFuncValue(sb, archType, cur);

    if (var.isDone())
      sb.appendString(" {Done}");

    sb.appendChar('\n');
  }
}

ASMJIT_FAVOR_SIZE Error X86Internal::emitArgsAssignment(X86Emitter* emitter, const FuncFrame& frame, const FuncArgsAssignment& args) {
  typedef X86FuncArgsContext::Var Var;
  typedef X86FuncArgsContext::WorkData WorkData;

  enum WorkFlags : uint32_t {
    kWorkNone      = 0x00,
    kWorkDidSome   = 0x01,
    kWorkPending   = 0x02,
    kWorkPostponed = 0x04
  };

  X86FuncArgsContext ctx;
  ASMJIT_PROPAGATE(ctx.initWorkData(frame, args));

  /*
  {
    StringBuilder sb;
    dumpAssignment(sb, ctx);
    std::printf("%s\n", sb.getData());
  }
  */

  uint32_t archType = ctx.getArchType();
  uint32_t varCount = ctx._varCount;
  WorkData* workData = ctx._workData;

  // Use AVX if it's enabled.
  bool avxEnabled = frame.isAvxEnabled();

  uint32_t saVarId = ctx._saVarId;
  uint32_t saRegId = X86Gp::kIdSp;

  if (frame.hasDynamicAlignment()) {
    if (frame.hasPreservedFP())
      saRegId = X86Gp::kIdBp;
    else
      saRegId = saVarId < varCount ? ctx._vars[saVarId].cur.getRegId() : frame.getSARegId();
  }

  // --------------------------------------------------------------------------
  // Register to stack and stack to stack moves must be first as now we have
  // the biggest chance of having as many as possible unassigned registers.
  // --------------------------------------------------------------------------

  if (ctx._stackDstMask) {
    // Base address of all arguments passed by stack.
    X86Mem baseArgPtr = x86::ptr(emitter->gpz(saRegId), int32_t(frame.getSAOffset(saRegId)));
    X86Mem baseStackPtr = x86::ptr(emitter->gpz(X86Gp::kIdSp), int32_t(0));

    for (uint32_t varId = 0; varId < varCount; varId++) {
      Var& var = ctx._vars[varId];
      if (!var.out.isStack()) continue;

      ASMJIT_ASSERT(var.cur.isReg() || var.cur.isStack());
      X86Reg reg;

      if (var.cur.isReg()) {
        WorkData& wd = workData[X86Reg::groupOf(var.cur.getRegType())];
        uint32_t rId = var.cur.getRegId();

        reg.setSignatureAndId(X86Reg::signatureOf(var.cur.getRegType()), rId);
        wd.unassign(varId, rId);
      }
      else {
        // Stack to reg move - tricky since we move stack to stack we can decide which
        // register to use. In general we follow the rule that IntToInt moves will use
        // GP regs with possibility to sign or zero extend, and all other moves will
        // either use GP or VEC regs depending on the size of the move.
        RegInfo rInfo = x86GetRegForMemToMemMove(archType, var.out.getTypeId(), var.cur.getTypeId());
        if (ASMJIT_UNLIKELY(!rInfo.isValid()))
          return DebugUtils::errored(kErrorInvalidState);

        WorkData& wd = workData[rInfo.getGroup()];
        uint32_t availableRegs = wd.getAvailableRegs();
        if (ASMJIT_UNLIKELY(!availableRegs))
          return DebugUtils::errored(kErrorInvalidState);

        uint32_t rId = IntUtils::ctz(availableRegs);
        reg.setSignatureAndId(rInfo.getSignature(), rId);

        ASMJIT_PROPAGATE(
          emitArgMove(emitter,
                      reg,
                      var.out.getTypeId(),
                      baseArgPtr.cloneAdjusted(var.cur.getStackOffset()),
                      var.cur.getTypeId(),
                      avxEnabled));
      }

      // Reg to stack move.
      ASMJIT_PROPAGATE(
        emitRegMove(emitter, baseStackPtr.cloneAdjusted(var.out.getStackOffset()), reg, var.cur.getTypeId(), avxEnabled));

      var.markDone();
    }
  }

  // --------------------------------------------------------------------------
  // Shuffle all registers that are currently assigned accordingly to the assignment.
  // --------------------------------------------------------------------------

  uint32_t workFlags = kWorkNone;
  for (;;) {
    for (uint32_t varId = 0; varId < varCount; varId++) {
      Var& var = ctx._vars[varId];
      if (var.isDone() || !var.cur.isReg()) continue;

      uint32_t curType = var.cur.getRegType();
      uint32_t outType = var.out.getRegType();

      uint32_t curGroup = X86Reg::groupOf(curType);
      uint32_t outGroup = X86Reg::groupOf(outType);

      uint32_t curId = var.cur.getRegId();
      uint32_t outId = var.out.getRegId();

      if (curGroup != outGroup) {
        ASMJIT_ASSERT(!"IMPLEMENTED");

        // Requires a conversion between two register groups.
        if (workData[outGroup]._numSwaps) {
          // TODO: Postponed
          workFlags |= kWorkPending;
        }
        else {
          // TODO:
          workFlags |= kWorkPending;
        }
      }
      else {
        WorkData& wd = workData[outGroup];
        if (!wd.isAssigned(outId)) {
EmitMove:
          ASMJIT_PROPAGATE(
            emitArgMove(emitter,
              X86Reg::fromTypeAndId(outType, outId), var.out.getTypeId(),
              X86Reg::fromTypeAndId(curType, curId), var.cur.getTypeId(), avxEnabled));

          wd.reassign(varId, outId, curId);
          var.cur.initReg(outType, outId, var.out.getTypeId());

          if (outId == var.out.getRegId())
            var.markDone();
          workFlags |= kWorkDidSome | kWorkPending;
        }
        else {
          uint32_t altId = wd._physToVarId[outId];
          Var& altVar = ctx._vars[altId];

          if (!altVar.out.isInitialized() || (altVar.out.isReg() && altVar.out.getRegId() == curId)) {
            // Swap operation is possible only between two GP registers.
            if (curGroup == X86Reg::kGroupGp) {
              uint32_t highestType = std::max(var.cur.getRegType(), altVar.cur.getRegType());
              uint32_t signature = highestType == X86Reg::kRegGpq ? X86Reg::signatureOfT<X86Reg::kRegGpq>()
                                                                  : X86Reg::signatureOfT<X86Reg::kRegGpd>();

              ASMJIT_PROPAGATE(emitter->emit(X86Inst::kIdXchg, X86Reg(signature, outId), X86Reg(signature, curId)));
              wd.swap(varId, curId, altId, outId);
              var.cur.setRegId(outId);
              var.markDone();
              altVar.cur.setRegId(curId);

              if (altVar.out.isInitialized())
                altVar.markDone();
              workFlags |= kWorkDidSome;
            }
            else {
              // If there is a scratch register it can be used to perform the swap.
              uint32_t availableRegs = wd.getAvailableRegs();
              if (availableRegs) {
                uint32_t inOutRegs = wd.getDstRegs();
                if (availableRegs & ~inOutRegs)
                  availableRegs &= ~inOutRegs;
                outId = IntUtils::ctz(availableRegs);
                goto EmitMove;
              }
              else {
                workFlags |= kWorkPending;
              }
            }
          }
          else {
            workFlags |= kWorkPending;
          }
        }
      }
    }

    if (!(workFlags & kWorkPending))
      break;

    // If we did nothing twice it means that something is really broken.
    if ((workFlags & (kWorkDidSome | kWorkPostponed)) == kWorkPostponed)
      return DebugUtils::errored(kErrorInvalidState);

    workFlags = (workFlags & kWorkDidSome) ? kWorkNone : kWorkPostponed;
  }

  // --------------------------------------------------------------------------
  // Load arguments passed by stack into registers. This is pretty simple and
  // it never requires multiple iterations like the previous phase.
  // --------------------------------------------------------------------------

  if (ctx._hasStackSrc) {
    uint32_t iterCount = 1;
    if (frame.hasDynamicAlignment() && !frame.hasPreservedFP())
      saRegId = saVarId < varCount ? ctx._vars[saVarId].cur.getRegId() : frame.getSARegId();

    // Base address of all arguments passed by stack.
    X86Mem baseArgPtr = x86::ptr(emitter->gpz(saRegId), int32_t(frame.getSAOffset(saRegId)));

    for (uint32_t iter = 0; iter < iterCount; iter++) {
      for (uint32_t varId = 0; varId < varCount; varId++) {
        Var& var = ctx._vars[varId];
        if (var.isDone()) continue;

        if (var.cur.isStack()) {
          ASMJIT_ASSERT(var.out.isReg());

          uint32_t outId = var.out.getRegId();
          uint32_t outType = var.out.getRegType();

          uint32_t group = X86Reg::groupOf(outType);
          WorkData& wd = ctx._workData[group];

          if (outId == saRegId && group == Reg::kGroupGp) {
            // This register will be processed last as we still need `saRegId`.
            if (iterCount == 1) {
              iterCount++;
              continue;
            }
            wd.unassign(wd._physToVarId[outId], outId);
          }

          X86Reg dstReg = X86Reg::fromTypeAndId(outType, outId);
          X86Mem srcMem = baseArgPtr.cloneAdjusted(var.cur.getStackOffset());

          ASMJIT_PROPAGATE(
            emitArgMove(emitter,
              dstReg, var.out.getTypeId(),
              srcMem, var.cur.getTypeId(), avxEnabled));

          wd.assign(varId, outId);
          var.cur.initReg(outType, outId, var.cur.getTypeId(), FuncValue::kFlagIsDone);
        }
      }
    }
  }

  return kErrorOk;
}

} // asmjit namespace

// [Api-End]
#include "../asmjit_apiend.h"

// [Guard]
#endif // ASMJIT_BUILD_X86
