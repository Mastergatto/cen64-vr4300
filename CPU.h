/* ============================================================================
 *  CPU.h: VR4300 Processor (VR4300).
 *
 *  VR4300SIM: NEC VR43xx Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#ifndef __VR4300__CPU_H__
#define __VR4300__CPU_H__
#include "Common.h"
#include "CP0.h"
#include "CP1.h"
#include "Externs.h"
#include "Pipeline.h"
#include "TLB.h"

#define VR4300_LINK_REGISTER VR4300_REGISTER_RA

enum VR4300Register {
  VR4300_REGISTER_ZERO, VR4300_REGISTER_AT, VR4300_REGISTER_V0,
  VR4300_REGISTER_V1, VR4300_REGISTER_A0, VR4300_REGISTER_A1,
  VR4300_REGISTER_A2, VR4300_REGISTER_A3, VR4300_REGISTER_T0,
  VR4300_REGISTER_T1, VR4300_REGISTER_T2, VR4300_REGISTER_T3,
  VR4300_REGISTER_T4, VR4300_REGISTER_R5, VR4300_REGISTER_T6,
  VR4300_REGISTER_T7, VR4300_REGISTER_S0, VR4300_REGISTER_S1,
  VR4300_REGISTER_S2, VR4300_REGISTER_S3, VR4300_REGISTER_S4,
  VR4300_REGISTER_S5, VR4300_REGISTER_S6, VR4300_REGISTER_S7,
  VR4300_REGISTER_T8, VR4300_REGISTER_T9, VR4300_REGISTER_K0,
  VR4300_REGISTER_K1, VR4300_REGISTER_GP, VR4300_REGISTER_SP,
  VR4300_REGISTER_FP, VR4300_REGISTER_RA, VR4300_REGISTER_LO,
  VR4300_REGISTER_HI, NUM_VR4300_REGISTERS
};

enum MIRegister {
#define X(reg) reg,
#include "Registers.md"
#undef X
  NUM_MI_REGISTERS
};

#ifndef NDEBUG
extern const char *RIRegisterMnemonics[NUM_MI_REGISTERS];
#endif

struct VR4300 {
  uint64_t regs[NUM_VR4300_REGISTERS];
  uint32_t miregs[NUM_MI_REGISTERS];

  struct VR4300TLB tlb;
  struct BusController *bus;
  struct VR4300CP0 cp0;
  struct VR4300CP1 cp1;

  struct VR4300Pipeline pipeline;
};

struct VR4300 *CreateVR4300(void);
void DestroyVR4300(struct VR4300 *);

#endif

