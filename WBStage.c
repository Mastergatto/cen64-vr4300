/* ============================================================================
 *  WBStage.c: Writeback stage.
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
#include "Pipeline.h"
#include "WBStage.h"

/* ============================================================================
 *  VR4300WBStage: Writes results back to the register file.
 * ========================================================================= */
void
VR4300WBStage(struct VR4300 *vr4300) {
  const struct VR4300DCWBLatch *dcwbLatch = &vr4300->pipeline.dcwbLatch;
  vr4300->regs[dcwbLatch->result.dest] = dcwbLatch->result.data;
}

