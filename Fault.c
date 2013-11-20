/* ============================================================================
 *  Fault.c: Fault and exception handler.
 *
 *  VR4300SIM: NEC VR43xx Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#include "Common.h"
#include "CP0.h"
#include "CPU.h"
#include "Fault.h"
#include "Pipeline.h"

#ifdef __cplusplus
#include <cassert>
#include <cstring>
#else
#include <assert.h>
#include <string.h>
#endif

static void CommonExceptionHandler(struct VR4300CP0 *,
  uint64_t *, uint64_t, unsigned, uint32_t);

/* ============================================================================
 *  VR4300/MIPS exception vectors.
 * ========================================================================= */
#define VR4300_GENERAL_BEV_VECTOR 0xFFFFFFFFBFC00200ULL
#define VR4300_GENERAL_VECTOR     0xFFFFFFFF80000180ULL
#define VR4300_RESET_VECTOR       0xFFFFFFFFBFC00000ULL

/* ============================================================================
 *  Mnemonic and callback tables.
 * ========================================================================= */
#ifndef NDEBUG
const char *VR4300FaultMnemonics[NUM_VR4300_FAULTS] = {
#define X(fault) #fault,
#include "Fault.md"
#undef X
};
#endif

/* ============================================================================
 *  Common exception handler for all but Cold/Soft/NMI and TLB/XTLB.
 * ========================================================================= */
static void
CommonExceptionHandler(struct VR4300CP0 *cp0, uint64_t *pc,
  uint64_t faultingPC, unsigned exeCode, uint32_t opcodeFlags) {
  cp0->regs.cause.excCode = exeCode;

  /* Setup the cause register. */
  if (cp0->regs.status.exl == 0) {
    uint64_t epc;

    if (opcodeFlags & OPCODE_INFO_BRANCH) {
      epc = faultingPC - 4;
      cp0->regs.cause.bd = 1;
    }

    else {
      epc = faultingPC;
      cp0->regs.cause.bd = 0;
    }

    cp0->regs.epc = epc;
  }

  /* Disable interrupts. */
  /* Switch to kernel mode. */
  cp0->interruptRaiseMask = 0;
  cp0->regs.status.exl = 1;

  /* Jump to the exception vector. */
  *pc = (cp0->regs.status.ds.bev)
    ? VR4300_GENERAL_BEV_VECTOR - 4
    : VR4300_GENERAL_VECTOR - 4;
}

/* ===========================================================================
 *  InitFaultManager: Initializes the fault manager.
 * ========================================================================= */
void
InitFaultManager(struct VR4300FaultManager *manager) {
  PerformHardReset(manager);
}

/* ===========================================================================
 *  PerformHardReset: Queues up a hard reset exception.
 * ========================================================================= */
void PerformHardReset(struct VR4300FaultManager *manager) {
  QueueFault(manager, VR4300_FAULT_RST, 0, 0, 0, VR4300_PIPELINE_STAGE_IC);
}

/* ===========================================================================
 *  PerformSoftReset: Queues up a soft reset exception.
 * ========================================================================= */
void PerformSoftReset(struct VR4300FaultManager *manager) {
  QueueFault(manager, VR4300_FAULT_RST, 0, 0, 1, VR4300_PIPELINE_STAGE_IC);
}

/* ===========================================================================
 *  QueueFault: Queues up a pipeline fault notice in a prioritized fashion.
 * ========================================================================= */
void
QueueFault(struct VR4300FaultManager *manager, enum VR4300PipelineFault fault,
  uint64_t faultingPC, uint32_t nextOpcodeFlags, uint32_t faultCauseData,
  enum VR4300PCUIndex pcuIndex) {
  debugarg("Queued up a fault: %s.", VR4300FaultMnemonics[fault]);

  assert(pcuIndex != VR4300_PCU_NORMAL);

  /* Higher priority ready? */
  if (fault < manager->fault)
    return;

  manager->faultingPC = faultingPC;
  manager->nextOpcodeFlags = nextOpcodeFlags;
  manager->faultCauseData = faultCauseData;

  manager->pcuIndex = pcuIndex;
  manager->fault = fault;
}

/* ============================================================================
 *  VR4300FaultBRPT: Breakpoint Exception.
 * ========================================================================= */
void
VR4300FaultBRPT(struct VR4300 *unused(vr4300)) {
  debug("Unimplemented fault: BRPT.");
}

/* ============================================================================
 *  VR4300FaultCOP: CACHE Op Interlock.
 * ========================================================================= */
void
VR4300FaultCOP(struct VR4300 *unused(vr4300)) {
  debug("Unimplemented fault: COP.");
}

/* ============================================================================
 *  VR4300FaultCP0I: CP0 Bypass Interlock.
 * ========================================================================= */
void
VR4300FaultCP0I(struct VR4300 *unused(vr4300)) {
  debug("Unimplemented fault: CP0I.");
}

/* ============================================================================
 *  VR4300FaultCPU: Coprocessor Unusable Exception.
 * ========================================================================= */
void
VR4300FaultCPU(struct VR4300 *vr4300) {
  const struct VR4300FaultManager *manager = &vr4300->pipeline.faultManager;
  struct VR4300Pipeline *pipeline = &vr4300->pipeline;
  struct VR4300CP0 *cp0 = &vr4300->cp0;

  debug("Handing fault: CPU.");

  cp0->regs.cause.ce = manager->faultCauseData;
  CommonExceptionHandler(cp0, &pipeline->icrfLatch.pc,
    manager->faultingPC, 11, manager->nextOpcodeFlags);
}

/* ============================================================================
 *  VR4300FaultDADE: Data Address Error Exception.
 * ========================================================================= */
void
VR4300FaultDADE(struct VR4300 *unused(vr4300)) {
  debug("Unimplemented fault: DADE.");
}

/* ============================================================================
 *  VR4300FaultDBE: Data Bus Error Exception.
 * ========================================================================= */
void
VR4300FaultDBE(struct VR4300 *unused(vr4300)) {
  debug("Unimplemented fault: DBE.");
}

/* ============================================================================
 *  VR4300FaultDCB: Data Cache Busy Interlock.
 * ========================================================================= */
void
VR4300FaultDCB(struct VR4300 *unused(vr4300)) {
  debug("Unimplemented fault: DCB.");
}

/* ============================================================================
 *  VR4300FaultDCM: Data Cache Miss Interlock.
 * ========================================================================= */
void
VR4300FaultDCM(struct VR4300 *unused(vr4300)) {
  debug("Unimplemented fault: DCM.");
}

/* ============================================================================
 *  VR4300FaultDTLB: Data TLB Exception (Miss/Invalid/Modification).
 * ========================================================================= */
void
VR4300FaultDTLB(struct VR4300 *unused(vr4300)) {
  debug("Unimplemented fault: DTLB.");
}

/* ============================================================================
 *  VR4300FaultFPE: Floating Point Exception.
 * ========================================================================= */
void
VR4300FaultFPE(struct VR4300 *unused(vr4300)) {
  debug("Unimplemented fault: FPE.");
}

/* ============================================================================
 *  VR4300FaultIADE: Instruction Address Error Exception.
 * ========================================================================= */
void
VR4300FaultIADE(struct VR4300 *unused(vr4300)) {
  debug("Unimplemented fault: IADE.");
}

/* ============================================================================
 *  VR4300FaultIBE: Instruction Bus Error Exception.
 * ========================================================================= */
void
VR4300FaultIBE(struct VR4300 *unused(vr4300)) {
  debug("Unimplemented fault: IBE.");
}

/* ============================================================================
 *  VR4300FaultICB: Instruction Cache Busy Interlock.
 * ========================================================================= */
void
VR4300FaultICB(struct VR4300 *unused(vr4300)) {
  debug("Unimplemented fault: ICB.");
}

/* ============================================================================
 *  VR4300FaultINTR: Interrupt Exception.
 * ========================================================================= */
void
VR4300FaultINTR(struct VR4300 *vr4300) {
  const struct VR4300FaultManager *manager = &vr4300->pipeline.faultManager;
  struct VR4300Pipeline *pipeline = &vr4300->pipeline;
  struct VR4300CP0 *cp0 = &vr4300->cp0;

  debug("Handing fault: INTR.");

  cp0->regs.cause.ce = manager->faultCauseData;
  CommonExceptionHandler(cp0, &pipeline->icrfLatch.pc,
    manager->faultingPC, 0, manager->nextOpcodeFlags);
}

/* ============================================================================
 *  VR4300FaultINV: Invalid fault.
 * ========================================================================= */
void
VR4300FaultINV(struct VR4300 *unused(vr4300)) {
  assert(0 && "Caught the 'INV' fault?");
}

/* ============================================================================
 *  VR4300FaultITM: Instruction TLB Exception (Miss E./Invalid/Miss I.).
 * ========================================================================= */
void
VR4300FaultITM(struct VR4300 *unused(vr4300)) {
  debug("Unimplemented fault: ITM.");
}

/* ============================================================================
 *  VR4300FaultLDI: Load Interlock.
 * ========================================================================= */
void
VR4300FaultLDI(struct VR4300 *unused(vr4300)) {
  debug("Unimplemented fault: LDI.");
}

/* ============================================================================
 *  VR4300FaultMCI: Multicycle Instruction Interlock.
 * ========================================================================= */
void
VR4300FaultMCI(struct VR4300 *unused(vr4300)) {
  debug("Unimplemented fault: MCI.");
}

/* ============================================================================
 *  VR4300FaultNMI: Non-Maskable Interrupt Exception.
 * ========================================================================= */
void
VR4300FaultNMI(struct VR4300 *unused(vr4300)) {
  debug("Unimplemented fault: NMI.");
}

/* ============================================================================
 *  VR4300FaultOVFL: Integer Overflow Exception.
 * ========================================================================= */
void
VR4300FaultOVFL(struct VR4300 *unused(vr4300)) {
  debug("Unimplemented fault: OVFL.");
}

/* ============================================================================
 *  VR4300FaultRST: Reset Exception.
 * ========================================================================= */
void
VR4300FaultRST(struct VR4300 *vr4300) {
  struct VR4300Pipeline *pipeline = &vr4300->pipeline;
  struct VR4300CP0 *cp0 = &vr4300->cp0;

  debug("Handling fault: RST");

  /* RST/SOFT */
  if (vr4300->pipeline.faultManager.faultCauseData) {
    debug("Unimplemented fault: RST/Soft.");
  }

  /* RST/HARD */
  else {
    cp0->regs.status.ds.ts = 0;
    cp0->regs.status.ds.sr = 0;
    cp0->regs.status.rp = 0;
    cp0->regs.config.ep = 0;

    cp0->regs.status.erl = 1;
    cp0->regs.status.ds.bev = 1;
    cp0->regs.config.be = 1;

    /* Set to upper limit. */
    cp0->regs.random = 31;

    /* Set to DivMode pins. */
    cp0->regs.config.ec = 0;
  }

  /* Change the PC to the reset exception vector. */
  pipeline->icrfLatch.pc = VR4300_RESET_VECTOR - 4;
}

/* ============================================================================
 *  VR4300FaultRSVD: Reserved Instruction Exception.
 * ========================================================================= */
void
VR4300FaultRSVD(struct VR4300 *unused(vr4300)) {
  debug("Unimplemented fault: RSVD.");
}

/* ============================================================================
 *  VR4300FaultSYSC: System Call Exception.
 * ========================================================================= */
void
VR4300FaultSYSC(struct VR4300 *unused(vr4300)) {
  debug("Unimplemented fault: SYSC.");
}

/* ============================================================================
 *  VR4300FaultTRAP: Trap Exception.
 * ========================================================================= */
void
VR4300FaultTRAP(struct VR4300 *unused(vr4300)) {
  debug("Unimplemented fault: TRAP.");
}

/* ============================================================================
 *  VR4300FaultWAT: Watch Exception.
 * ========================================================================= */
void
VR4300FaultWAT(struct VR4300 *unused(vr4300)) {
  debug("Unimplemented fault: WAT.");
}

/* ============================================================================
 *  HandleFaults: Resolves pipeline fault conditions.
 *
 *   Interlocks: Resolve by stalling the pipeline until hardware corrects.
 *   Exceptions: Resolve by aborting both the faulty and subsequent insns.
 * ========================================================================= */
typedef void (*const FaultHandler)(struct VR4300 *);

static const FaultHandler FaultHandlerTable[NUM_VR4300_FAULTS] = {
#define X(fault) &VR4300Fault##fault,
#include "Fault.md"
#undef X
};

void
HandleFaults(struct VR4300 *vr4300) {
  struct VR4300FaultManager *manager = &vr4300->pipeline.faultManager;
  struct VR4300ICRFLatch *icrfLatch = &vr4300->pipeline.icrfLatch;
  struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;
  struct VR4300DCWBLatch *dcwbLatch = &vr4300->pipeline.dcwbLatch;

  /* Trash all outputs of proceeding instructions. */
  memset(&dcwbLatch->result, 0, sizeof(dcwbLatch->result));
  memset(&exdcLatch->result, 0, sizeof(exdcLatch->result));
  VR4300InvalidateOpcode(&rfexLatch->opcode);
  icrfLatch->iwMask = 0;

  /* Resolve the fault appropriately. */
  FaultHandlerTable[manager->fault](vr4300);

  /* Reset the pipeline (to effectively flush it). */
  manager->pcuIndex = VR4300_PCU_NORMAL;
  manager->fault = VR4300_FAULT_INV;
}

