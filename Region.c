/* ============================================================================
 *  Region.c: Memory address space resolver.
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
#include "Region.h"

#ifdef __cplusplus
#include <cstddef>
#else
#include <stddef.h>
#endif

typedef const struct RegionInfo Region;

/* Mapped regions. */
static Region USEG =   {0, 0x0000000000000000ULL, 0x0000000080000000ULL, true};
static Region UXSEG =  {0, 0x0000000000000000ULL, 0x0000010000000000ULL, true};
static Region XKSSEG = {0, 0x4000000000000000ULL, 0x0000010000000000ULL, true};
static Region XKSEG =  {0, 0xC000000000000000ULL, 0x000000FF80000000ULL, true};
static Region SSEG =   {0, 0xFFFFFFFFC0000000ULL, 0x0000000020000000ULL, true};
static Region KSEG3 =  {0, 0xFFFFFFFFE0000000ULL, 0x0000000020000000ULL, true};

/* Unmapped regions. */
static Region KSEG0 = {
  0xFFFFFFFF80000000ULL,
  0xFFFFFFFF80000000ULL,
  0x0000000020000000ULL, true};

static Region KSEG1 = {
  0xFFFFFFFFA0000000ULL,
  0xFFFFFFFFA0000000ULL,
  0x0000000020000000ULL, false};

/* ============================================================================
 *  GetDefaultRegion: Return a pointer to any valid region.
 * ========================================================================= */
Region* GetDefaultRegion(void) {
  return &KSEG0;
}

/* ============================================================================
 *  GetRegionInfo: Given a virtual address, determines if it's mapped, cached,
 *  wired, etc. depending on the current operating mode. If the address doesn't
 *  map to something valid, wired or not, return NULL.
 * ========================================================================= */
Region* GetRegionInfo(const struct VR4300 *vr4300, uint64_t address) {
  uint32_t upper = address >> 32;
  uint32_t lower = address;
  unsigned effectiveMode;

  /* Check for useg/xuseg. */
  if (lower < 0x80000000) {
    if (upper < 0x00000100 && vr4300->cp0.regs.status.ux)
      return &UXSEG;

    return &USEG;
  }

  /* Kernel mode is used if the erl or exl bits are set. */
  effectiveMode =
    (vr4300->cp0.regs.status.erl || vr4300->cp0.regs.status.exl)
    ? 0: vr4300->cp0.regs.status.ksu;

  /* Check for upper region sseg/ksegs. */
  if (upper == 0xFFFFFFFF && effectiveMode < 2) {
    if (effectiveMode == 0) {

      /* kseg3. */
      if ((lower - 0xE0000000) < 0x20000000)
        return &KSEG3;

      /* kseg1. */
      else if ((lower - 0xA0000000) < 0x20000000) {
        return &KSEG1;
      }

      /* kseg0. */
      else if ((lower - 0x80000000) < 0x20000000) {
        return &KSEG0;
      }
    }

    /* sseg. */
    if ((lower - 0xC0000000) > 0x20000000)
      return &SSEG;

    return NULL;
  }

  /* Check for xkseg/xkphys. */
  if (effectiveMode == 0 && vr4300->cp0.regs.status.kx)  {
    if (upper == 0xC0000000 && (lower - 0x80000000) > 0)
      return &XKSEG;

    /* TODO: Check for/support xkphys addresses. */
    debug("Access in unsupported xkphys address region.");
    return NULL;
  }

  /* Check for xksseg. */
  if ((effectiveMode == 1 && vr4300->cp0.regs.status.sx) ||
    (effectiveMode == 0 && vr4300->cp0.regs.status.kx))
    if ((upper - 0x40000000) < 0x00000100)
      return &XKSSEG;

  debugarg("GetRegionInfo: Invalid address [0x%.16llx].", address);
  return NULL;
}

