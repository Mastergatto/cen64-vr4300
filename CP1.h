/* ============================================================================
 *  CP1.h: VR4300 Coprocessor #1.
 *
 *  VR4300SIM: NEC VR43xx Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All Rights Reserved.
 * ========================================================================= */
#ifndef __VR4300__CP1__
#define __VR4300__CP1__
#include "Common.h"

struct VR4300CP1Control {
  int nativeCause;
  int nativeEnables;
  int nativeFlags;
  int nativeReserved;

  uint8_t cause;
  uint8_t enables;
  uint8_t flags;
  uint8_t reserved;

  uint8_t fs;
  uint8_t rm;
  uint8_t rs;
  uint8_t c;
};

union VR4300CP1Register {
  struct {
    double data;
  } d;

  struct {
    uint64_t data;
  } l;

  struct {
    float data[2];
  } s;

  struct {
    uint32_t data[2];
  } w;
};

struct VR4300CP1 {
  union VR4300CP1Register regs[32];
  struct VR4300CP1Control control;
};

void VR4300InitCP1(struct VR4300CP1 *);

#endif

