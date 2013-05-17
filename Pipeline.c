/* ============================================================================
 *  Pipeline.c: Processor pipeline.
 *
 *  VR4300SIM: NEC VR43xx Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#include "Common.h"
#include "CPU.h"
#include "DCStage.h"
#include "EXStage.h"
#include "Fault.h"
#include "ICStage.h"
#include "Pipeline.h"
#include "Region.h"
#include "RFStage.h"
#include "WBStage.h"

#ifdef __cplusplus
#include <cstring>
#else
#include <string.h>
#endif

static void CheckForPendingInterrupts(struct VR4300 *);
static void IncrementCycleCounters(struct VR4300 *);

#ifdef DO_FASTFORWARD
static void FastForward(struct VR4300 *);
#endif

/* ============================================================================
 *  Pipeline exceptions that require us to kill instructions are uncommon.
 *  Therefore, to prevent littering the pipeline with conditionals, we simply
 *  use functions that only execute specific pipeline stages and invoke them
 *  as necessary.
 *
 *  The last "stage" will actually invoke a fault handler, instead of cycling
 *  the pipeline. The stages spend handing the fault count towards the delay
 *  cycles.
 * ========================================================================= */
static void CycleVR4300_StartRF(struct VR4300 *);
static void CycleVR4300_StartEX(struct VR4300 *);
static void CycleVR4300_StartDC(struct VR4300 *);
typedef void (*ShortPipelineFunction)(struct VR4300 *);

#ifdef DO_FASTFORWARD
static const ShortPipelineFunction CycleVR4300Short[5] = {
#else
static const ShortPipelineFunction CycleVR4300Short[4] = {
#endif
  CycleVR4300_StartRF,
  CycleVR4300_StartEX,
  CycleVR4300_StartDC,
  HandleFaults,
#ifdef DO_FASTFORWARD
  FastForward,
#endif
};

/* ============================================================================
 *  Checks for pending interrupts and queues them up if present.
 * ========================================================================= */
static void
CheckForPendingInterrupts(struct VR4300 *vr4300) {
  if (!vr4300->cp0.canRaiseInterrupt)
    return;

  /* There are are interrupts pending, and the mask matches... */
  if (vr4300->cp0.regs.cause.ip & vr4300->cp0.regs.status.im) {
    const struct VR4300Opcode *opcode = &vr4300->pipeline.rfexLatch.opcode;

    /* Queue the exception up, prepare to kill stages. */
    QueueFault(&vr4300->pipeline.faultManager, VR4300_FAULT_INTR,
      vr4300->pipeline.icrfLatch.pc, opcode->flags, 0 /* No Data */,
      VR4300_PIPELINE_STAGE_IC);
  }
}

/* ============================================================================
 *  CycleVR4300: Advances the state of the processor pipeline one PCycle.
 * ========================================================================= */
void
CycleVR4300(struct VR4300 *vr4300) {
  if (vr4300->pipeline.stalls > 0)
    vr4300->pipeline.stalls--;

  /* If any faults were raised, handle and bail. */
  else if (vr4300->pipeline.faultManager.killStage >= 0)
    CycleVR4300Short[vr4300->pipeline.faultManager.killStage](vr4300);

  else {
    VR4300WBStage(vr4300);
    VR4300DCStage(vr4300);
    VR4300EXStage(vr4300);
    VR4300RFStage(vr4300);
    VR4300ICStage(vr4300);

    /* Only check on a full cycle. */
    /* TODO: What about end of stalls? */
    CheckForPendingInterrupts(vr4300);
  }

  IncrementCycleCounters(vr4300);
}

/* ============================================================================
 *  CycleVR4300_StartRF: Advances the state of the processor pipeline.
 *  We intentionally skip the IC stage; faulting instruction is in RF.
 * ========================================================================= */
static void
CycleVR4300_StartRF(struct VR4300 *vr4300) {
  vr4300->pipeline.faultManager.killStage++;

  VR4300WBStage(vr4300);
  VR4300DCStage(vr4300);
  VR4300EXStage(vr4300);
  VR4300RFStage(vr4300);
}

/* ============================================================================
 *  CycleVR4300_StartEX: Advances the state of the processor pipeline.
 *  We intentionally skip the IC/RF stage; faulting instruction is in EX.
 * ========================================================================= */
static void
CycleVR4300_StartEX(struct VR4300 *vr4300) {
  vr4300->pipeline.faultManager.killStage++;

  VR4300WBStage(vr4300);
  VR4300DCStage(vr4300);
  VR4300EXStage(vr4300);
}

/* ============================================================================
 *  CycleVR4300_StartDC: Advances the state of the processor pipeline.
 *  We intentionally skip the IC/RF/EX stage; faulting instruction is in DC.
 * ========================================================================= */
static void
CycleVR4300_StartDC(struct VR4300 *vr4300) {
  vr4300->pipeline.faultManager.killStage++;

  VR4300WBStage(vr4300);
  VR4300DCStage(vr4300);
}

/* ============================================================================
 *  Fast forwards the CPU when it's just busy-looping.
 * ========================================================================= */
#ifdef DO_FASTFORWARD
static void
FastForward(struct VR4300 *vr4300) {
  /* Only check on a full cycle. */
  /* TODO: What about end of stalls? */
  CheckForPendingInterrupts(vr4300);
}
#endif

/* ============================================================================
 *  Bump counters, check for timer interrupts, etc. before we leave.
 * ========================================================================= */
static void
IncrementCycleCounters(struct VR4300 *vr4300) {
  vr4300->pipeline.cycles++;

  /* Increment the count register; timer interrupt unlikely. */
  vr4300->cp0.regs.count += (vr4300->pipeline.cycles & 0x01);
  if (unlikely(vr4300->cp0.regs.count == vr4300->cp0.regs.compare))
    vr4300->cp0.regs.cause.ip |= 0x80;
}

/* ============================================================================
 *  VR4300InitPipeline: Initializes the pipeline.
 * ========================================================================= */
void
VR4300InitPipeline(struct VR4300Pipeline *pipeline) {
  memset(pipeline, 0, sizeof(*pipeline));

  /* Validate the cached regions. */
  pipeline->icrfLatch.region = GetDefaultRegion();
  pipeline->dcwbLatch.region = GetDefaultRegion();

  InitFaultManager(&pipeline->faultManager);
}

