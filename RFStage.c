/* ============================================================================
 *  RFStage.c: Register fetch stage.
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
#include "Externs.h"
#include "Pipeline.h"

/* ============================================================================
 *  VR4300RFStage: Decodes instructions, reads from cache, grabs registers.
 * ========================================================================= */
void
VR4300RFStage(struct VR4300 *vr4300) {
  struct VR4300ICRFLatch *icrfLatch = &vr4300->pipeline.icrfLatch;
  struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  uint32_t address = icrfLatch->address;

  /* Always update PC. */
  icrfLatch->pc += 4;

  /* TODO: Giant hack: bypass the ICache for now. */
  uint32_t iw = BusReadWord(vr4300->bus, address) & icrfLatch->iwMask;
  icrfLatch->iwMask = ~0;

  /* Decode the instruction, save the results. */
  rfexLatch->opcode = *VR4300DecodeInstruction(iw);
  rfexLatch->iw = iw;
}

