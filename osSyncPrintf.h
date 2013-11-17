/* ============================================================================
 *  osSyncPrintf.h: osSyncPrintf stub helpers.
 *
 *  VR4300SIM: NEC VR43xx Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#include "CPU.h"
#include "DCStage.h"
#include <stdarg.h>
#include <stdio.h>

uint64_t BusGetRDRAMPointer(const struct BusController *bus);

/* Helper function to switch to native calling convention. */
static int __osSyncPrintf(const char *fmt, ...) {
  va_list ap;
  int ret;

  va_start(ap, fmt);
  ret = vfprintf(stdout, fmt, ap);
  va_end(ap);

  return ret;
}

/* Calls osSyncPrintf using the architectural state and fmt string address. */
/* TODO: Are we abusing the calling convention or is this method correct? */
static int osSyncPrintf(const struct VR4300 *vr4300, uint32_t address) {
  const char *fmt = (const char*) BusGetRDRAMPointer(vr4300->bus) + address;
  uint64_t args[16];
  unsigned i, a;

  memset(args, 0, sizeof(args));
  args[0] = vr4300->regs[VR4300_REGISTER_A0];
  args[1] = vr4300->regs[VR4300_REGISTER_A1];
  args[2] = vr4300->regs[VR4300_REGISTER_A2];
  args[3] = vr4300->regs[VR4300_REGISTER_A3];
  a = 36; /* start @ $sp + 36? */

  for (i = 4; i < 16; i++) {
    MemoryFunction read;
    void *opaque;
    int s;

    if ((read = BusRead(vr4300->bus, BUS_TYPE_WORD, address, &opaque)) == NULL)
      continue;

    read(opaque, ((vr4300->regs[VR4300_REGISTER_SP] + a) & 0xFFFFFF), &s);
    args[i] = (int64_t) s;
    a += 4;
  }

  for (a = 0, i = 0; i < strlen(fmt); i++) {
    if (fmt[i] == '%') {
      if (fmt[i+1] == 's')
        args[a] = BusGetRDRAMPointer(vr4300->bus) + (args[a] & 0xFFFFFF);

      a++;
    }
  }

  return __osSyncPrintf(fmt, args[0], args[1], args[2],
    args[3], args[4], args[5], args[6], args[7], args[8],
    args[9], args[10], args[11], args[12], args[13],
    args[14], args[15]);
}

