/* ============================================================================
 *  CPU.c: VR4300 Processor (VR4300).
 *
 *  VR4300SIM: NEC VR43xx Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#include "Address.h"
#include "Common.h"
#include "CP0.h"
#include "CP1.h"
#include "CPU.h"
#include "Fault.h"
#include "TLBTree.h"

#ifdef __cplusplus
#include <cassert>
#include <cstdlib>
#include <cstring>
#else
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#endif

#define MI_EBUS_TEST_MODE 0x0080
#define MI_INIT_MODE      0x0100
#define MI_RDRAM_REG_MODE 0x0200

#define MI_INTR_SP 0x01
#define MI_INTR_SI 0x02
#define MI_INTR_AI 0x04
#define MI_INTR_VI 0x08
#define MI_INTR_PI 0x10
#define MI_INTR_DP 0x20

static void CheckForRCPInterrupts(struct VR4300 *);
static void InitVR4300(struct VR4300 *);

/* ============================================================================
 *  Mnemonics table.
 * ========================================================================= */
#ifndef NDEBUG
const char *MIRegisterMnemonics[NUM_MI_REGISTERS] = {
#define X(reg) #reg,
#include "Registers.md"
#undef X
};
#endif

/* ============================================================================
 *  CheckForRCPInterrupts: Checks for pending RCP interrupts.
 * ========================================================================= */
static void
CheckForRCPInterrupts(struct VR4300 *vr4300) {
  if (vr4300->miregs[MI_INTR_REG] & vr4300->miregs[MI_INTR_MASK_REG])
    vr4300->cp0.regs.cause.ip |= 0x04;
  else
    vr4300->cp0.regs.cause.ip &= ~0x04;
}

/* ============================================================================
 *  ConnectVR4300ToBus: Connects a VR4300 instance to a Bus instance.
 * ========================================================================= */
void
ConnectVR4300ToBus(struct VR4300 *vr4300, struct BusController *bus) {
  vr4300->bus = bus;
}

/* ============================================================================
 *  CreateVR4300: Creates and initializes a VR4300 instance.
 * ========================================================================= */
struct VR4300 *
CreateVR4300(void) {
  struct VR4300 *vr4300;

  if ((vr4300 = (struct VR4300*) malloc(sizeof(struct VR4300))) == NULL) {
    debug("Failed to allocate memory.");
    return NULL;
  }
  
  InitVR4300(vr4300);
  return vr4300;
}


/* ============================================================================
 *  DestroyVR4300: Releases any resources allocated for a VR4300 instance.
 *========================================================================== */
void
DestroyVR4300(struct VR4300 *vr4300) {
  free(vr4300);
}

/* ============================================================================
 *  InitVR4300: Initializes the VR4300.
 * ========================================================================= */
static void
InitVR4300(struct VR4300 *vr4300) {
  debug("Initializing CPU.");
	memset(vr4300, 0, sizeof(*vr4300));

  vr4300->bus = NULL;
  VR4300InitCP0(&vr4300->cp0);
  VR4300InitCP1(&vr4300->cp1);
  VR4300InitTLB(&vr4300->tlb);
  VR4300InitPipeline(&vr4300->pipeline);

  /* MESS uses this version, so we will too? */
  vr4300->miregs[MI_VERSION_REG] = 0x01010101;
}

/* ============================================================================
 *  MIRegRead: Read from MI registers.
 * ========================================================================= */
int
MIRegRead(void *_vr4300, uint32_t address, void *_data) {
	struct VR4300 *vr4300 = (struct VR4300*) _vr4300;
	uint32_t *data = (uint32_t*) _data;

  address -= MI_REGS_BASE_ADDRESS;
  enum MIRegister reg = (enum MIRegister) (address / 4);

  debugarg("MIRegRead: Reading from register [%s].", MIRegisterMnemonics[reg]);
  *data = vr4300->miregs[reg];

  return 0;
}

/* ============================================================================
 *  MIRegWrite: Write to MI registers.
 * ========================================================================= */
int
MIRegWrite(void *_vr4300, uint32_t address, void *_data) {
	struct VR4300 *vr4300 = (struct VR4300*) _vr4300;
	uint32_t *data = (uint32_t*) _data, result;

  address -= MI_REGS_BASE_ADDRESS;
  enum MIRegister reg = (enum MIRegister) (address / 4);

  debugarg("MIRegWrite: Writing to register [%s].", MIRegisterMnemonics[reg]);

  /* Change mode settings? */
  if (reg == MI_INIT_MODE_REG) {
    result = *data & 0x7F;

    if (*data & 0x0080)
      result &= ~MI_INIT_MODE;
    else if (*data & 0x0100)
      result |= MI_INIT_MODE;

    if (*data & 0x0200)
      result &= ~MI_EBUS_TEST_MODE;
    else if (*data & 0x0400)
      result |= MI_EBUS_TEST_MODE;

    if (*data & 0x0800)
      vr4300->miregs[MI_INTR_REG] &= ~MI_INTR_DP;

    if (*data & 0x1000)
      result &= ~MI_RDRAM_REG_MODE;
    else if (*data & 0x2000)
      result |= MI_RDRAM_REG_MODE;

    vr4300->miregs[MI_INIT_MODE_REG] = result;
  }

  /* Change interrupt mask? */
  else if (reg == MI_INTR_MASK_REG) {
    if (*data & 0x0001)
      vr4300->miregs[MI_INTR_MASK_REG] &= ~MI_INTR_SP;
    else if (*data & 0x0002)
      vr4300->miregs[MI_INTR_MASK_REG] |= MI_INTR_SP;

    if (*data & 0x0004)
      vr4300->miregs[MI_INTR_MASK_REG] &= ~MI_INTR_SI;
    else if (*data & 0x0008)
      vr4300->miregs[MI_INTR_MASK_REG] |= MI_INTR_SI;

    if (*data & 0x0010)
      vr4300->miregs[MI_INTR_MASK_REG] &= ~MI_INTR_AI;
    else if (*data & 0x0020)
      vr4300->miregs[MI_INTR_MASK_REG] |= MI_INTR_AI;

    if (*data & 0x0040)
      vr4300->miregs[MI_INTR_MASK_REG] &= ~MI_INTR_VI;
    else if (*data & 0x0080)
      vr4300->miregs[MI_INTR_MASK_REG] |= MI_INTR_VI;

    if (*data & 0x0100)
      vr4300->miregs[MI_INTR_MASK_REG] &= ~MI_INTR_PI;
    else if (*data & 0x0200)
      vr4300->miregs[MI_INTR_MASK_REG] |= MI_INTR_PI;

    if (*data & 0x0400)
      vr4300->miregs[MI_INTR_MASK_REG] &= ~MI_INTR_DP;
    else if (*data & 0x0800)
      vr4300->miregs[MI_INTR_MASK_REG] |= MI_INTR_DP;

    CheckForRCPInterrupts(vr4300);
  }

  return 0;
}

/* ============================================================================
 *  VR4300ClearRCPInterrupt: Clears a bit in MI_INTR_REG.
 * ========================================================================= */
void
VR4300ClearRCPInterrupt(struct VR4300 *vr4300, unsigned mask) {
  vr4300->miregs[MI_INTR_REG] &= ~mask;
  CheckForRCPInterrupts(vr4300);
}

/* ============================================================================
 *  VR4300RaiseRCPInterrupt: Sets a bit in MI_INTR_REG.
 * ========================================================================= */
void
VR4300RaiseRCPInterrupt(struct VR4300 *vr4300, unsigned mask) {
  vr4300->miregs[MI_INTR_REG] |= mask;
  CheckForRCPInterrupts(vr4300);
}

