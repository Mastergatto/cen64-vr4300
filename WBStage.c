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

/* ============================================================================
 *  VR4300WBStage: Writes results back to the register file.
 * ========================================================================= */
void
VR4300WBStage(const struct VR4300DCWBLatch *dcwbLatch, uint64_t regs[]) {
  regs[dcwbLatch->result.dest] = dcwbLatch->result.data;

  /* Fix ROMs that write to $r0. */
  regs[VR4300_REGISTER_ZERO] = 0;
}

