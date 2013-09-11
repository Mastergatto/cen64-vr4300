/* ============================================================================
 *  CP1.c: VR4300 Coprocessor #1.
 *
 *  VR4300SIM: NEC VR43xx Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 *
 *  TODO: We assume little-endian everywhere (because of the CP1Register
 *  union, we've forced to assume some byte-ordering). This will need to be
 *  fixed before big-endian builds are released.
 *
 *  Also, due to the fact that there seems to be no portable (or really, any)
 *  way of easily telling the compiler about use/defines of the FPU status
 *  word, the code must be littered with volatile inline assembler blocks.
 *
 *  TODO: Make sure exceptions are being raised.
 * ========================================================================= */
#include "Common.h"
#include "CP1.h"
#include "CPU.h"

#ifdef __cplusplus
#include <cassert>
#include <cfenv>
#include <cstring>
#else
#include <assert.h>
#include <fenv.h>
#include <string.h>
#endif

typedef void (*FPUOperation)(struct VR4300 *);

/* ============================================================================
 *  Checks for pending interrupts and queues them up if present.
 * ========================================================================= */
static int
FPUCheckUsable(struct VR4300 *vr4300) {
  const struct VR4300Opcode *opcode = &vr4300->pipeline.rfexLatch.opcode;

  /* The co-processor is likely marked usable. */
  if (likely(vr4300->cp0.regs.status.cu & 0x2))
    return 1;

  /* Queue the exception up, prepare to kill stages. */
  QueueFault(&vr4300->pipeline.faultManager, VR4300_FAULT_CPU,
    vr4300->pipeline.rfexLatch.pc, opcode->flags, 1 /* COp ID */,
    VR4300_PIPELINE_STAGE_EX);

  return 0;
}

/* ============================================================================
 *  FPUClearExceptions: Wipes out the FPU exception bits on the host.
 * ========================================================================= */
static void
FPUClearExceptions(void) {
#ifdef USE_X87FPU
  __asm__ volatile("fclex\n\t");
#else

  /* POSIX interfaces are far too slow... */
  feclearexcept(FE_ALL_EXCEPT);
#endif
}

/* ============================================================================
 *  FPURaiseException: Raises a floating point exception.
 * ========================================================================= */
static void
FPURaiseException(struct VR4300 *unused(vr4300)) {
  assert(0 && "Should raise FPU exception.");
}

/* ============================================================================
 *  FPUUpdateState: Updates the state of the FPU. Returns 1 when enable = cause.
 * ========================================================================= */
static int
FPUUpdateState(struct VR4300CP1 *cp1) {
  uint16_t flags;

#ifdef USE_X87FPU
  __asm__ volatile(
    "fstsw %%ax\n\t"
    : "=a"(flags)
  );
#else

  /* POSIX interfaces are far too slow... */
  flags = fetestexcept(FE_ALL_EXCEPT);
#endif

  cp1->control.nativeFlags |= flags;
  cp1->control.nativeCause = flags;

  return (flags & cp1->control.nativeEnables)
    ? 1 : 0;
}

/* ============================================================================
 *  Instruction: ABS.d (Floating-Point Absolute Value).
 * ========================================================================= */
static void
VR4300ABSd(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  union VR4300CP1Register *fd = &cp1->regs[GET_FD(rfexLatch->iw)];
  double value;

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "fldl %1\n\t"
    "fabs\n\t"
    "fstpl %0\n\t"
    : "=m" (value)
    : "m" (fs->d.data)
    : "st"
  );
#endif

  if (FPUUpdateState(cp1)) {
    FPURaiseException(vr4300);
    return;
  }

  fd->d.data = value;
}

/* ============================================================================
 *  Instruction: ABS.s (Floating-Point Absolute Value).
 * ========================================================================= */
static void
VR4300ABSs(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  union VR4300CP1Register *fd = &cp1->regs[GET_FD(rfexLatch->iw)];
  float value;

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "flds %1\n\t"
    "fabs\n\t"
    "fstp %0\n\t"
    : "=m" (value)
    : "m" (fs->s.data[0])
    : "st"
  );
#endif

  if (FPUUpdateState(cp1)) {
    FPURaiseException(vr4300);
    return;
  }

  fd->s.data[0] = value;
}

/* ============================================================================
 *  Instruction: ADD.d (Floating-Point Add).
 * ========================================================================= */
static void
VR4300ADDd(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  const union VR4300CP1Register *ft = &cp1->regs[GET_FT(rfexLatch->iw)];
  union VR4300CP1Register *fd = &cp1->regs[GET_FD(rfexLatch->iw)];
  double value;

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "fldl %1\n\t"
    "fldl %2\n\t"
    "faddp\n\t"
    "fstpl %0\n\t"
    : "=m" (value)
    : "m" (fs->d.data),
      "m" (ft->d.data)
    : "st"
    );
#endif

  if (FPUUpdateState(cp1)) {
    FPURaiseException(vr4300);
    return;
  }

  fd->d.data = value;
}

/* ============================================================================
 *  Instruction: ADD.s (Floating-Point Add).
 * ========================================================================= */
static void
VR4300ADDs(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  const union VR4300CP1Register *ft = &cp1->regs[GET_FT(rfexLatch->iw)];
  union VR4300CP1Register *fd = &cp1->regs[GET_FD(rfexLatch->iw)];
  float value;

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "flds %1\n\t"
    "flds %2\n\t"
    "faddp\n\t"
    "fstp %0\n\t"
    : "=m" (value)
    : "m" (fs->s.data[0]),
      "m" (ft->s.data[0])
    : "st"
    );
#endif

  if (FPUUpdateState(cp1)) {
    FPURaiseException(vr4300);
    return;
  }

  fd->s.data[0] = value;
}

/* ============================================================================
 *  Instruction: C.eq.d (Floating-Point Compare).
 * ========================================================================= */
static void
VR4300Ceqd(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  const union VR4300CP1Register *ft = &cp1->regs[GET_FT(rfexLatch->iw)];
  uint8_t NOTun, eq;

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "fldl %3\n\t"
    "fldl %2\n\t"
    "fcomip\n\t"
    "setnp %0\n\t"
    "sete %1\n\t"
    "fstp %%st(0)\n\t"
    : "=m" (NOTun),
      "=m" (eq)
    : "m" (fs->d.data),
      "m" (ft->d.data)
    : "st"
    );

    cp1->control.coc = NOTun & eq;
#endif

  FPUUpdateState(cp1);
  cp1->control.c = cp1->control.coc;
}

/* ============================================================================
 *  Instruction: C.eq.s (Floating-Point Compare).
 * ========================================================================= */
static void
VR4300Ceqs(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  const union VR4300CP1Register *ft = &cp1->regs[GET_FT(rfexLatch->iw)];
  uint8_t NOTun, eq;

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "flds %3\n\t"
    "flds %2\n\t"
    "fcomip\n\t"
    "setnp %0\n\t"
    "sete %1\n\t"
    "fstp %%st(0)\n\t"
    : "=m" (NOTun),
      "=m" (eq)
    : "m" (fs->s.data[0]),
      "m" (ft->s.data[0])
    : "st"
    );

    cp1->control.coc = NOTun & eq;
#endif

  FPUUpdateState(cp1);
  cp1->control.c = cp1->control.coc;
}

/* ============================================================================
 *  Instruction: C.f.d (Floating-Point Compare).
 * ========================================================================= */
static void
VR4300Cfd(struct VR4300 *vr4300) {
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  FPUClearExceptions();
  cp1->control.coc = 0;

  FPUUpdateState(cp1);
  cp1->control.c = cp1->control.coc;
}

/* ============================================================================
 *  Instruction: C.f.s (Floating-Point Compare).
 * ========================================================================= */
static void
VR4300Cfs(struct VR4300 *vr4300) {
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  FPUClearExceptions();
  cp1->control.coc = 0;

  FPUUpdateState(cp1);
  cp1->control.c = cp1->control.coc;
}

/* ============================================================================
 *  Instruction: C.le.d (Floating-Point Compare).
 * ========================================================================= */
static void
VR4300Cled(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  const union VR4300CP1Register *ft = &cp1->regs[GET_FT(rfexLatch->iw)];
  uint8_t un, le;

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "fldl %3\n\t"
    "fldl %2\n\t"
    "fcomip\n\t"
    "setp %0\n\t"
    "setbe %1\n\t"
    "fstp %%st(0)\n\t"
    : "=m" (un),
      "=m" (le)
    : "m" (fs->d.data),
      "m" (ft->d.data)
    : "st"
    );

    if (un) {
      /* TODO/FIXME: InvalidOperationException. */
      assert(0 && "InvalidOperationException.");
      le = 0;
    }

    cp1->control.coc = un | le;
#endif

  FPUUpdateState(cp1);
  cp1->control.c = cp1->control.coc;
}

/* ============================================================================
 *  Instruction: C.le.s (Floating-Point Compare).
 * ========================================================================= */
static void
VR4300Cles(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  const union VR4300CP1Register *ft = &cp1->regs[GET_FT(rfexLatch->iw)];
  uint8_t un, le;

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "flds %3\n\t"
    "flds %2\n\t"
    "fcomip\n\t"
    "setp %0\n\t"
    "setbe %1\n\t"
    "fstp %%st(0)\n\t"
    : "=m" (un),
      "=m" (le)
    : "m" (fs->s.data[0]),
      "m" (ft->s.data[0])
    : "st"
    );

    if (un) {
      /* TODO/FIXME: InvalidOperationException. */
      assert(0 && "InvalidOperationException.");
      le = 0;
    }

    cp1->control.coc = un | le;
#endif

  FPUUpdateState(cp1);
  cp1->control.c = cp1->control.coc;
}

/* ============================================================================
 *  Instruction: C.lt.d (Floating-Point Compare).
 * ========================================================================= */
static void
VR4300Cltd(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  const union VR4300CP1Register *ft = &cp1->regs[GET_FT(rfexLatch->iw)];
  uint8_t un, lt;

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "fldl %3\n\t"
    "fldl %2\n\t"
    "fcomip\n\t"
    "setp %0\n\t"
    "setb %1\n\t"
    "fstp %%st(0)\n\t"
    : "=m" (un),
      "=m" (lt)
    : "m" (fs->d.data),
      "m" (ft->d.data)
    : "st"
    );

    if (un) {
      /* TODO/FIXME: InvalidOperationException. */
      assert(0 && "InvalidOperationException.");
      lt = 0;
    }

    cp1->control.coc = lt;
#endif

  FPUUpdateState(cp1);
  cp1->control.c = cp1->control.coc;
}

/* ============================================================================
 *  Instruction: C.lt.s (Floating-Point Compare).
 * ========================================================================= */
static void
VR4300Clts(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  const union VR4300CP1Register *ft = &cp1->regs[GET_FT(rfexLatch->iw)];
  uint8_t un, lt;

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "flds %3\n\t"
    "flds %2\n\t"
    "fcomip\n\t"
    "setp %0\n\t"
    "setb %1\n\t"
    "fstp %%st(0)\n\t"
    : "=m" (un),
      "=m" (lt)
    : "m" (fs->s.data[0]),
      "m" (ft->s.data[0])
    : "st"
    );

    if (un) {
      /* TODO/FIXME: InvalidOperationException. */
      assert(0 && "InvalidOperationException.");
      lt = 0;
    }

    cp1->control.coc = lt;
#endif

  FPUUpdateState(cp1);
  cp1->control.c = cp1->control.coc;
}

/* ============================================================================
 *  Instruction: C.ngl.d (Floating-Point Compare).
 * ========================================================================= */
static void
VR4300Cngld(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  const union VR4300CP1Register *ft = &cp1->regs[GET_FT(rfexLatch->iw)];
  uint8_t un, eq;

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "fldl %3\n\t"
    "fldl %2\n\t"
    "fcomip\n\t"
    "setp %0\n\t"
    "sete %1\n\t"
    "fstp %%st(0)\n\t"
    : "=m" (un),
      "=m" (eq)
    : "m" (fs->d.data),
      "m" (ft->d.data)
    : "st"
    );

    if (un) {
      /* TODO/FIXME: InvalidOperationException. */
      assert(0 && "InvalidOperationException.");
    }

    cp1->control.coc = un | eq;
#endif

  FPUUpdateState(cp1);
  cp1->control.c = cp1->control.coc;
}

/* ============================================================================
 *  Instruction: C.ngl.s (Floating-Point Compare).
 * ========================================================================= */
static void
VR4300Cngls(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  const union VR4300CP1Register *ft = &cp1->regs[GET_FT(rfexLatch->iw)];
  uint8_t un, eq;

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "flds %3\n\t"
    "flds %2\n\t"
    "fcomip\n\t"
    "setp %0\n\t"
    "sete %0\n\t"
    "fstp %%st(0)\n\t"
    : "=m" (un),
      "=m" (eq)
    : "m" (fs->s.data[0]),
      "m" (ft->s.data[0])
    : "st"
    );

    if (un) {
      /* TODO/FIXME: InvalidOperationException. */
      assert(0 && "InvalidOperationException.");
    }

    cp1->control.coc = un | eq;
#endif

  FPUUpdateState(cp1);
  cp1->control.c = cp1->control.coc;
}

/* ============================================================================
 *  Instruction: C.nge.d (Floating-Point Compare).
 * ========================================================================= */
static void
VR4300Cnged(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  const union VR4300CP1Register *ft = &cp1->regs[GET_FT(rfexLatch->iw)];
  uint8_t un, lt;

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "fldl %3\n\t"
    "fldl %2\n\t"
    "fcomip\n\t"
    "setp %0\n\t"
    "setb %1\n\t"
    "fstp %%st(0)\n\t"
    : "=m" (un),
      "=m" (lt)
    : "m" (fs->d.data),
      "m" (ft->d.data)
    : "st"
    );

    if (un) {
      /* TODO/FIXME: InvalidOperationException. */
      assert(0 && "InvalidOperationException.");
    }

    cp1->control.coc = un | lt;
#endif

  FPUUpdateState(cp1);
  cp1->control.c = cp1->control.coc;
}

/* ============================================================================
 *  Instruction: C.nge.s (Floating-Point Compare).
 * ========================================================================= */
static void
VR4300Cnges(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  const union VR4300CP1Register *ft = &cp1->regs[GET_FT(rfexLatch->iw)];
  uint8_t un, lt;

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "flds %3\n\t"
    "flds %2\n\t"
    "fcomip\n\t"
    "setp %0\n\t"
    "setb %1\n\t"
    "fstp %%st(0)\n\t"
    : "=m" (un),
      "=m" (lt)
    : "m" (fs->s.data[0]),
      "m" (ft->s.data[0])
    : "st"
    );

    if (un) {
      /* TODO/FIXME: InvalidOperationException. */
      assert(0 && "InvalidOperationException.");
    }

    cp1->control.coc = un | lt;
#endif

  FPUUpdateState(cp1);
  cp1->control.c = cp1->control.coc;
}

/* ============================================================================
 *  Instruction: C.ngle.d (Floating-Point Compare).
 * ========================================================================= */
static void
VR4300Cngled(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  const union VR4300CP1Register *ft = &cp1->regs[GET_FT(rfexLatch->iw)];

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "fldl %2\n\t"
    "fldl %1\n\t"
    "fcomip\n\t"
    "setp %0\n\t"
    "fstp %%st(0)\n\t"
    : "=m" (cp1->control.coc)
    : "m" (fs->d.data),
      "m" (ft->d.data)
    : "st"
    );

    if (cp1->control.coc) {
      /* TODO/FIXME: InvalidOperationException. */
      assert(0 && "InvalidOperationException.");
    }
#endif

  FPUUpdateState(cp1);
  cp1->control.c = cp1->control.coc;
}

/* ============================================================================
 *  Instruction: C.ngle.s (Floating-Point Compare).
 * ========================================================================= */
static void
VR4300Cngles(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  const union VR4300CP1Register *ft = &cp1->regs[GET_FT(rfexLatch->iw)];

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "flds %2\n\t"
    "flds %1\n\t"
    "fcomip\n\t"
    "setp %0\n\t"
    "fstp %%st(0)\n\t"
    : "=m" (cp1->control.coc)
    : "m" (fs->s.data[0]),
      "m" (ft->s.data[0])
    : "st"
    );

    if (cp1->control.coc) {
      /* TODO/FIXME: InvalidOperationException. */
      assert(0 && "InvalidOperationException.");
    }
#endif

  FPUUpdateState(cp1);
  cp1->control.c = cp1->control.coc;
}

/* ============================================================================
 *  Instruction: C.ngt.d (Floating-Point Compare).
 * ========================================================================= */
static void
VR4300Cngtd(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  const union VR4300CP1Register *ft = &cp1->regs[GET_FT(rfexLatch->iw)];
  uint8_t un, le;

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "fldl %3\n\t"
    "fldl %2\n\t"
    "fcomip\n\t"
    "setp %0\n\t"
    "setbe %1\n\t"
    "fstp %%st(0)\n\t"
    : "=m" (un),
      "=m" (le)
    : "m" (fs->d.data),
      "m" (ft->d.data)
    : "st"
    );

    if (un) {
      /* TODO/FIXME: InvalidOperationException. */
      assert(0 && "InvalidOperationException.");
    }

    cp1->control.coc = un | le;
#endif

  FPUUpdateState(cp1);
  cp1->control.c = cp1->control.coc;
}

/* ============================================================================
 *  Instruction: C.ngt.s (Floating-Point Compare).
 * ========================================================================= */
static void
VR4300Cngts(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  const union VR4300CP1Register *ft = &cp1->regs[GET_FT(rfexLatch->iw)];
  uint8_t un, le;

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "flds %2\n\t"
    "flds %1\n\t"
    "fcomip\n\t"
    "setp %0\n\t"
    "setbe %1\n\t"
    "fstp %%st(0)\n\t"
    : "=m" (un),
      "=m" (le)
    : "m" (fs->s.data[0]),
      "m" (ft->s.data[0])
    : "st"
    );

    if (un) {
      /* TODO/FIXME: InvalidOperationException. */
      assert(0 && "InvalidOperationException.");
    }

    cp1->control.coc = un | le;
#endif

  FPUUpdateState(cp1);
  cp1->control.c = cp1->control.coc;
}

/* ============================================================================
 *  Instruction: C.ole.d (Floating-Point Compare).
 * ========================================================================= */
static void
VR4300Coled(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  const union VR4300CP1Register *ft = &cp1->regs[GET_FT(rfexLatch->iw)];
  uint8_t NOTun, le;

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "fldl %2\n\t"
    "fldl %1\n\t"
    "fcomip\n\t"
    "setnp %0\n\t"
    "setbe %1\n\t"
    "fstp %%st(0)\n\t"
    : "=m" (NOTun),
      "=m" (le)
    : "m" (fs->d.data),
      "m" (ft->d.data)
    : "st"
    );

    cp1->control.coc = NOTun & le;
#endif

  FPUUpdateState(cp1);
  cp1->control.c = cp1->control.coc;
}

/* ============================================================================
 *  Instruction: C.ole.s (Floating-Point Compare).
 * ========================================================================= */
static void
VR4300Coles(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  const union VR4300CP1Register *ft = &cp1->regs[GET_FT(rfexLatch->iw)];
  uint8_t NOTun, le;

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "flds %2\n\t"
    "flds %1\n\t"
    "fcomip\n\t"
    "setnp %0\n\t"
    "setbe %1\n\t"
    "fstp %%st(0)\n\t"
    : "=m" (NOTun),
      "=m" (le)
    : "m" (fs->s.data[0]),
      "m" (ft->s.data[0])
    : "st"
    );

    cp1->control.coc = NOTun & le;
#endif

  FPUUpdateState(cp1);
  cp1->control.c = cp1->control.coc;
}

/* ============================================================================
 *  Instruction: C.olt.d (Floating-Point Compare).
 * ========================================================================= */
static void
VR4300Coltd(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  const union VR4300CP1Register *ft = &cp1->regs[GET_FT(rfexLatch->iw)];
  uint8_t NOTun, lt;

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "fldl %3\n\t"
    "fldl %2\n\t"
    "fcomip\n\t"
    "setnp %0\n\t"
    "setb %1\n\t"
    "fstp %%st(0)\n\t"
    : "=m" (NOTun),
      "=m" (lt)
    : "m" (fs->d.data),
      "m" (ft->d.data)
    : "st"
    );

    cp1->control.coc = NOTun & lt;
#endif

  FPUUpdateState(cp1);
  cp1->control.c = cp1->control.coc;
}

/* ============================================================================
 *  Instruction: C.olt.s (Floating-Point Compare).
 * ========================================================================= */
static void
VR4300Colts(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  const union VR4300CP1Register *ft = &cp1->regs[GET_FT(rfexLatch->iw)];
  uint8_t NOTun, lt;

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "flds %3\n\t"
    "flds %2\n\t"
    "fcomip\n\t"
    "setnp %0\n\t"
    "setb %1\n\t"
    "fstp %%st(0)\n\t"
    : "=m" (NOTun),
      "=m" (lt)
    : "m" (fs->s.data[0]),
      "m" (ft->s.data[0])
    : "st"
    );
#endif

  if (FPUUpdateState(cp1)) {
    FPURaiseException(vr4300);
  }

  FPUUpdateState(cp1);
  cp1->control.c = cp1->control.coc;
}

/* ============================================================================
 *  Instruction: C.seq.d (Floating-Point Compare).
 * ========================================================================= */
static void
VR4300Cseqd(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  const union VR4300CP1Register *ft = &cp1->regs[GET_FT(rfexLatch->iw)];
  uint8_t un, eq;

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "fldl %3\n\t"
    "fldl %2\n\t"
    "fcomip\n\t"
    "setp %0\n\t"
    "sete %1\n\t"
    "fstp %%st(0)\n\t"
    : "=m" (un),
      "=m" (eq)
    : "m" (fs->d.data),
      "m" (ft->d.data)
    : "st"
    );

    if (un) {
      /* TODO/FIXME: InvalidOperationException. */
      assert(0 && "InvalidOperationException.");
      eq = 0;
    }

    cp1->control.coc = eq;
#endif

  FPUUpdateState(cp1);
  cp1->control.c = cp1->control.coc;
}

/* ============================================================================
 *  Instruction: C.seq.s (Floating-Point Compare).
 * ========================================================================= */
static void
VR4300Cseqs(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  const union VR4300CP1Register *ft = &cp1->regs[GET_FT(rfexLatch->iw)];
  uint8_t un, eq;

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "flds %3\n\t"
    "flds %2\n\t"
    "fcomip\n\t"
    "setp %0\n\t"
    "sete %1\n\t"
    "fstp %%st(0)\n\t"
    : "=m" (un),
      "=m" (eq)
    : "m" (fs->s.data[0]),
      "m" (ft->s.data[0])
    : "st"
    );

    if (un) {
      /* TODO/FIXME: InvalidOperationException. */
      assert(0 && "InvalidOperationException.");
      eq = 0;
    }

    cp1->control.coc = eq;
#endif

  FPUUpdateState(cp1);
  cp1->control.c = cp1->control.coc;
}

/* ============================================================================
 *  Instruction: C.sf.d (Floating-Point Compare).
 * ========================================================================= */
static void
VR4300Csfd(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  const union VR4300CP1Register *ft = &cp1->regs[GET_FT(rfexLatch->iw)];
  uint8_t un;

  FPUClearExceptions();
  cp1->control.coc = 0;

#ifdef USE_X87FPU
  __asm__ volatile(
    "fldl %2\n\t"
    "fldl %1\n\t"
    "fcomip\n\t"
    "setp %0\n\t"
    "fstp %%st(0)\n\t"
    : "=m" (un)
    : "m" (fs->d.data),
      "m" (ft->d.data)
    : "st"
    );

    if (un) {
      /* TODO/FIXME: InvalidOperationException. */
      assert(0 && "InvalidOperationException.");
    }
#endif

  FPUUpdateState(cp1);
  cp1->control.c = cp1->control.coc;
}

/* ============================================================================
 *  Instruction: C.sf.s (Floating-Point Compare).
 * ========================================================================= */
static void
VR4300Csfs(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  const union VR4300CP1Register *ft = &cp1->regs[GET_FT(rfexLatch->iw)];
  uint8_t un;

  FPUClearExceptions();
  cp1->control.coc = 0;

#ifdef USE_X87FPU
  __asm__ volatile(
    "flds %2\n\t"
    "flds %1\n\t"
    "fcomip\n\t"
    "setp %0\n\t"
    "fstp %%st(0)\n\t"
    : "=m" (un)
    : "m" (fs->s.data[0]),
      "m" (ft->s.data[0])
    : "st"
    );

    if (un) {
      /* TODO/FIXME: InvalidOperationException. */
      assert(0 && "InvalidOperationException.");
      return;
    }
#endif

  FPUUpdateState(cp1);
  cp1->control.c = cp1->control.coc;
}

/* ============================================================================
 *  Instruction: C.ueq.d (Floating-Point Compare).
 * ========================================================================= */
static void
VR4300Cueqd(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  const union VR4300CP1Register *ft = &cp1->regs[GET_FT(rfexLatch->iw)];
  uint8_t un, eq;

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "fldl %3\n\t"
    "fldl %2\n\t"
    "fcomip\n\t"
    "setp %0\n\t"
    "sete %1\n\t"
    "fstp %%st(0)\n\t"
    : "=m" (un),
      "=m" (eq)
    : "m" (fs->d.data),
      "m" (ft->d.data)
    : "st"
    );

    cp1->control.coc = un | eq;
#endif

  FPUUpdateState(cp1);
  cp1->control.c = cp1->control.coc;
}

/* ============================================================================
 *  Instruction: C.ueq.s (Floating-Point Compare).
 * ========================================================================= */
static void
VR4300Cueqs(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  const union VR4300CP1Register *ft = &cp1->regs[GET_FT(rfexLatch->iw)];
  uint8_t un, eq;

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "flds %3\n\t"
    "flds %2\n\t"
    "fcomip\n\t"
    "setp %0\n\t"
    "sete %1\n\t"
    "fstp %%st(0)\n\t"
    : "=m" (un),
      "=m" (eq)
    : "m" (fs->s.data[0]),
      "m" (ft->s.data[0])
    : "st"
    );

    cp1->control.coc = un | eq;
#endif

  FPUUpdateState(cp1);
  cp1->control.c = cp1->control.coc;
}

/* ============================================================================
 *  Instruction: C.ult.d (Floating-Point Compare).
 * ========================================================================= */
static void
VR4300Cultd(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  const union VR4300CP1Register *ft = &cp1->regs[GET_FT(rfexLatch->iw)];
  uint8_t un, lt;

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "fldl %3\n\t"
    "fldl %2\n\t"
    "fcomip\n\t"
    "setp %0\n\t"
    "setb %1\n\t"
    "fstp %%st(0)\n\t"
    : "=m" (un),
      "=m" (lt)
    : "m" (fs->d.data),
      "m" (ft->d.data)
    : "st"
    );

    cp1->control.coc = un | lt;
#endif

  FPUUpdateState(cp1);
  cp1->control.c = cp1->control.coc;
}

/* ============================================================================
 *  Instruction: C.ult.s (Floating-Point Compare).
 * ========================================================================= */
static void
VR4300Cults(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  const union VR4300CP1Register *ft = &cp1->regs[GET_FT(rfexLatch->iw)];
  uint8_t un, lt;

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "flds %3\n\t"
    "flds %2\n\t"
    "fcomip\n\t"
    "setp %0\n\t"
    "setb %1\n\t"
    "fstp %%st(0)\n\t"
    : "=m" (un),
      "=m" (lt)
    : "m" (fs->s.data[0]),
      "m" (ft->s.data[0])
    : "st"
    );

    cp1->control.coc = un | lt;
#endif

  FPUUpdateState(cp1);
  cp1->control.c = cp1->control.coc;
}

/* ============================================================================
 *  Instruction: C.ule.d (Floating-Point Compare).
 * ========================================================================= */
static void
VR4300Culed(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  const union VR4300CP1Register *ft = &cp1->regs[GET_FT(rfexLatch->iw)];
  uint8_t un, le;

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "fldl %3\n\t"
    "fldl %2\n\t"
    "fcomip\n\t"
    "setp %0\n\t"
    "setbe %1\n\t"
    "fstp %%st(0)\n\t"
    : "=m" (un),
      "=m" (le)
    : "m" (fs->d.data),
      "m" (ft->d.data)
    : "st"
    );

    cp1->control.coc = un | le;
#endif

  FPUUpdateState(cp1);
  cp1->control.c = cp1->control.coc;
}

/* ============================================================================
 *  Instruction: C.ule.s (Floating-Point Compare).
 * ========================================================================= */
static void
VR4300Cules(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  const union VR4300CP1Register *ft = &cp1->regs[GET_FT(rfexLatch->iw)];
  uint8_t un, le;

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "flds %3\n\t"
    "flds %2\n\t"
    "fcomip\n\t"
    "setp %0\n\t"
    "setbe %1\n\t"
    "fstp %%st(0)\n\t"
    : "=m" (un),
      "=m" (le)
    : "m" (fs->s.data[0]),
      "m" (ft->s.data[0])
    : "st"
    );

    cp1->control.coc = un | le;
#endif

  FPUUpdateState(cp1);
  cp1->control.c = cp1->control.coc;
}

/* ============================================================================
 *  Instruction: C.un.d (Floating-Point Compare).
 * ========================================================================= */
static void
VR4300Cund(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  const union VR4300CP1Register *ft = &cp1->regs[GET_FT(rfexLatch->iw)];

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "fldl %2\n\t"
    "fldl %1\n\t"
    "fcomip\n\t"
    "setp %0\n\t"
    "fstp %%st(0)\n\t"
    : "=m" (cp1->control.coc)
    : "m" (fs->d.data),
      "m" (ft->d.data)
    : "st"
    );
#endif

  FPUUpdateState(cp1);
  cp1->control.c = cp1->control.coc;
}

/* ============================================================================
 *  Instruction: C.un.s (Floating-Point Compare).
 * ========================================================================= */
static void
VR4300Cuns(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  const union VR4300CP1Register *ft = &cp1->regs[GET_FT(rfexLatch->iw)];

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "flds %2\n\t"
    "flds %1\n\t"
    "fcomip\n\t"
    "setp %0\n\t"
    "fstp %%st(0)\n\t"
    : "=m" (cp1->control.coc)
    : "m" (fs->s.data[0]),
      "m" (ft->s.data[0])
    : "st"
    );
#endif

  FPUUpdateState(cp1);
  cp1->control.c = cp1->control.coc;
}

/* ============================================================================
 *  Instruction: CFC1 (Move Control From Coprocessor 1)
 * ========================================================================= */
static int
CFC1NativeToSimulated(int hostexcepts) {
  int excepts = 0;

  if (hostexcepts & FE_INEXACT)
    excepts |= 0x1;
  if (hostexcepts & FE_UNDERFLOW)
    excepts |= 0x2;
  if (hostexcepts & FE_OVERFLOW)
    excepts |= 0x4;
  if (hostexcepts & FE_DIVBYZERO)
    excepts |= 0x8;

  /* Overkill, but whatever... */
  if (hostexcepts & FE_INVALID)
    excepts |= 0x10;

  return excepts;
}

void
VR4300CFC1(struct VR4300 *vr4300, uint64_t unused(rs), uint64_t unused(rt)) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;
  struct VR4300CP1Control *control = &vr4300->cp1.control;
  unsigned rt = GET_RT(rfexLatch->iw); 
  int32_t result;

#ifndef NDEBUG
  unsigned fs = GET_FS(rfexLatch->iw);
  assert ((fs == 31) && "Tried to read reserved CP1 register.");
#endif

  if (!FPUCheckUsable(vr4300)) 
    return;

  result = control->rm;
  result |= CFC1NativeToSimulated(control->nativeFlags) << 2;
  result |= CFC1NativeToSimulated(control->nativeEnables) << 7;
  result |= CFC1NativeToSimulated(control->nativeCause) << 12;
  result |= control->c << 23;
  result |= control->fs << 24;

  exdcLatch->result.data = (int64_t) result;
  exdcLatch->result.dest = rt;
}

/* ============================================================================
 *  Instruction: CTC1 (Move Control to Coprocessor 1)
 * ========================================================================= */
static int
CTC1SimulatedToNative(int excepts) {
  int hostexcepts = 0;

  if (excepts & 0x1)
    hostexcepts |= FE_INEXACT;
  if (excepts & 0x2)
    hostexcepts |= FE_UNDERFLOW;
  if (excepts & 0x4)
    hostexcepts |= FE_OVERFLOW;
  if (excepts & 0x8)
    hostexcepts |= FE_DIVBYZERO;
  if (excepts & 0x10)
    hostexcepts |= FE_INVALID;

  return hostexcepts;
}

void
VR4300CTC1(struct VR4300 *vr4300, uint64_t unused(rs), uint64_t rt) {
  struct VR4300CP1Control *control = &vr4300->cp1.control;

#ifndef NDEBUG
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  unsigned fs = GET_FS(rfexLatch->iw);

  assert(fs == 31 && "Tried to modify reserved CP1 register.");
#endif

  if (!FPUCheckUsable(vr4300))
    return;

  control->rm = rt >> 0 & 0x3;
  control->flags = rt >> 2 & 0x1F;
  control->enables = rt >> 7 & 0x1F;
  control->cause = rt >> 12 & 0x3F;
  control->c = rt >> 23 & 0x1;
  control->fs = rt >> 24 & 0x1;
  control->coc = control->c;

  control->nativeCause = CTC1SimulatedToNative(control->cause);
  control->nativeEnables = CTC1SimulatedToNative(control->enables);
  control->nativeFlags = CTC1SimulatedToNative(control->flags);
}

/* ============================================================================
 *  Instruction: CVT.d.s: (Floating-Point Convert To Double Floating-Point).
 * ========================================================================= */
static void
VR4300CVTds(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  union VR4300CP1Register *fd = &cp1->regs[GET_FD(rfexLatch->iw)];
  double value;

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "flds %1\n\t"
    "fstpl %0\n\t"
    : "=m" (value)
    : "m" (fs->w.data[0])
    : "st"
  );
#endif

  if (FPUUpdateState(cp1)) {
    FPURaiseException(vr4300);
    return;
  }

  fd->d.data = value;
}

/* ============================================================================
 *  Instruction: CVT.d.w: (Floating-Point Convert To Double Floating-Point).
 * ========================================================================= */
static void
VR4300CVTdw(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  union VR4300CP1Register *fd = &cp1->regs[GET_FD(rfexLatch->iw)];
  double value;

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "fildl %1\n\t"
    "fstpl %0\n\t"
    : "=m" (value)
    : "m" (fs->w.data[0])
    : "st"
  );
#endif

  if (FPUUpdateState(cp1)) {
    FPURaiseException(vr4300);
    return;
  }

  fd->d.data = value;
}

/* ============================================================================
 *  Instruction: CVT.s.d: (Floating-Point Convert To Single Floating-Point).
 * ========================================================================= */
static void
VR4300CVTsd(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  union VR4300CP1Register *fd = &cp1->regs[GET_FD(rfexLatch->iw)];
  float value;

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "fldl %1\n\t"
    "fstp %0\n\t"
    : "=m" (value)
    : "m" (fs->d.data)
    : "st"
  );
#endif

  if (FPUUpdateState(cp1)) {
    FPURaiseException(vr4300);
    return;
  }

  fd->s.data[0] = value;
}

/* ============================================================================
 *  Instruction: CVT.s.w: (Floating-Point Convert To Single Floating-Point).
 * ========================================================================= */
static void
VR4300CVTsw(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  union VR4300CP1Register *fd = &cp1->regs[GET_FD(rfexLatch->iw)];
  float value;

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "fildl %1\n\t"
    "fstp %0\n\t"
    : "=m" (value)
    : "m" (fs->w.data[0])
    : "st"
  );
#endif

  if (FPUUpdateState(cp1)) {
    FPURaiseException(vr4300);
    return;
  }

  fd->s.data[0] = value;
}

/* ============================================================================
 *  Instruction: CVT.w.d: (Floating-Point Convert To Single Fixed-Point).
 * ========================================================================= */
static void
VR4300CVTwd(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  union VR4300CP1Register *fd = &cp1->regs[GET_FD(rfexLatch->iw)];
  float value;

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "fldl %1\n\t"
    "fistpl %0\n\t"
    : "=m" (value)
    : "m" (fs->w.data[0])
    : "st"
  );
#endif

  if (FPUUpdateState(cp1)) {
    FPURaiseException(vr4300);
    return;
  }

  fd->s.data[0] = value;
}

/* ============================================================================
 *  Instruction: CVT.w.s: (Floating-Point Convert To Single Fixed-Point).
 * ========================================================================= */
static void
VR4300CVTws(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  union VR4300CP1Register *fd = &cp1->regs[GET_FD(rfexLatch->iw)];
  uint32_t value;

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "flds %1\n\t"
    "fistpl %0\n\t"
    : "=m" (value)
    : "m" (fs->w.data[0])
    : "st"
  );
#endif

  if (FPUUpdateState(cp1)) {
    FPURaiseException(vr4300);
    return;
  }

  fd->s.data[0] = value;
}

/* ============================================================================
 *  Instruction: DIV.d (Floating-Point Divide).
 * ========================================================================= */
static void
VR4300DIVd(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  const union VR4300CP1Register *ft = &cp1->regs[GET_FT(rfexLatch->iw)];
  union VR4300CP1Register *fd = &cp1->regs[GET_FD(rfexLatch->iw)];
  double value;

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "fldl %1\n\t"
    "fldl %2\n\t"
    "fdivrp\n\t"
    "fstpl %0\n\t"
    : "=m" (value)
    : "m" (fs->d.data),
      "m" (ft->d.data)
    : "st"
    );
#endif

  if (FPUUpdateState(cp1)) {
    FPURaiseException(vr4300);
    return;
  }

  fd->d.data = value;
}

/* ============================================================================
 *  Instruction: DIV.s (Floating-Point Divide).
 * ========================================================================= */
static void
VR4300DIVs(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  const union VR4300CP1Register *ft = &cp1->regs[GET_FT(rfexLatch->iw)];
  union VR4300CP1Register *fd = &cp1->regs[GET_FD(rfexLatch->iw)];
  float value;

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "flds %1\n\t"
    "flds %2\n\t"
    "fdivrp\n\t"
    "fstp %0\n\t"
    : "=m" (value)
    : "m" (fs->s.data[0]),
      "m" (ft->s.data[0])
    : "st"
    );
#endif

  if (FPUUpdateState(cp1)) {
    FPURaiseException(vr4300);
    return;
  }

  fd->s.data[0] = value;
}

/* ============================================================================
 *  Instruction: LDC1 (Load Doubleword To Coprocessor 1)
 * ========================================================================= */
void
VR4300LDC1(struct VR4300 *vr4300, uint64_t rs, uint64_t unused(rt)) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;

  unsigned ft = GET_FT(rfexLatch->iw);
  int64_t imm = (int16_t) rfexLatch->iw;
  uint64_t address = rs + imm;

  if (!FPUCheckUsable(vr4300))
    return;

  if (vr4300->cp0.regs.status.fr)
    exdcLatch->memoryData.target = &vr4300->cp1.regs[ft].l.data;
  else
    exdcLatch->memoryData.target = &vr4300->cp1.regs[ft & 0x1E].l.data;

  exdcLatch->memoryData.address = address;
  exdcLatch->memoryData.function = &VR4300LoadDWord;
}

/* ============================================================================
 *  Instruction: LWC1 (Load Word To Coprocessor 1)
 * ========================================================================= */
void
VR4300LWC1(struct VR4300 *vr4300, uint64_t rs, uint64_t unused(rt)) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;

  unsigned rt = GET_RT(rfexLatch->iw);
  int64_t imm = (int16_t) rfexLatch->iw;
  uint64_t address = rs + imm;

  if (!FPUCheckUsable(vr4300))
    return;

  if (vr4300->cp0.regs.status.fr)
    exdcLatch->memoryData.target = &vr4300->cp1.regs[rt].w.data[0];
  else
    exdcLatch->memoryData.target = &vr4300->cp1.
      regs[rt & 0x1E].w.data[rt & 0x1];

  exdcLatch->memoryData.address = address;
  exdcLatch->memoryData.function = &VR4300LoadWordFPU;
}

/* ============================================================================
 *  Instruction: MOV.d (Floating-Point Move).
 * ========================================================================= */
static void
VR4300MOVd(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  union VR4300CP1Register *fd = &cp1->regs[GET_FD(rfexLatch->iw)];
  double value;

  value = fs->l.data;
  fd->l.data = value;
}

/* ============================================================================
 *  Instruction: MOV.s (Floating-Point Move).
 * ========================================================================= */
static void
VR4300MOVs(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  union VR4300CP1Register *fd = &cp1->regs[GET_FD(rfexLatch->iw)];
  float value;

  value = fs->s.data[0];
  fd->s.data[0] = value;
}

/* ============================================================================
 *  Instruction: MFC1 (Move From Coprocessor 1)
 * ========================================================================= */
void
VR4300MFC1(struct VR4300 *vr4300, uint64_t unused(rs), uint64_t unused(rt)) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;
  int32_t result;

  unsigned fs = GET_FS(rfexLatch->iw);
  unsigned rt = GET_RT(rfexLatch->iw);

  if (!FPUCheckUsable(vr4300))
    return;

  if (vr4300->cp0.regs.status.fr)
    result = exdcLatch->result.data = vr4300->cp1.regs[fs].w.data[0];
  else
    result = vr4300->cp1.regs[fs & 0x1E].w.data[fs & 0x1];

  exdcLatch->result.data = (int64_t) result;
  exdcLatch->result.dest = rt;
}

/* ============================================================================
 *  Instruction: MTC1 (Move To Coprocessor 1)
 * ========================================================================= */
void
VR4300MTC1(struct VR4300 *vr4300, uint64_t unused(rs), uint64_t rt) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  unsigned fs = GET_FS(rfexLatch->iw);

  if (!FPUCheckUsable(vr4300))
    return;

  if (vr4300->cp0.regs.status.fr)
    vr4300->cp1.regs[fs].w.data[0] = rt;
  else
    vr4300->cp1.regs[fs & 0x1E].w.data[fs & 0x1] = rt;
}

/* ============================================================================
 *  Instruction: MUL.d (Floating-Point Multiply).
 * ========================================================================= */
static void
VR4300MULd(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  const union VR4300CP1Register *ft = &cp1->regs[GET_FT(rfexLatch->iw)];
  union VR4300CP1Register *fd = &cp1->regs[GET_FD(rfexLatch->iw)];
  double value;

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "fldl %1\n\t"
    "fldl %2\n\t"
    "fmulp\n\t"
    "fstpl %0\n\t"
    : "=m" (value)
    : "m" (fs->d.data),
      "m" (ft->d.data)
    : "st"
    );
#endif

  if (FPUUpdateState(cp1)) {
    FPURaiseException(vr4300);
    return;
  }

  fd->d.data = value;
}

/* ============================================================================
 *  Instruction: MUL.s (Floating-Point Multiply).
 * ========================================================================= */
static void
VR4300MULs(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  const union VR4300CP1Register *ft = &cp1->regs[GET_FT(rfexLatch->iw)];
  union VR4300CP1Register *fd = &cp1->regs[GET_FD(rfexLatch->iw)];
  float value;

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "flds %1\n\t"
    "flds %2\n\t"
    "fmulp\n\t"
    "fstp %0\n\t"
    : "=m" (value)
    : "m" (fs->s.data[0]),
      "m" (ft->s.data[0])
    : "st"
    );
#endif

  if (FPUUpdateState(cp1)) {
    FPURaiseException(vr4300);
    return;
  }

  fd->s.data[0] = value;
}

/* ============================================================================
 *  Instruction: NEG.d (Floating-Point Negate).
 * ========================================================================= */
static void
VR4300NEGd(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  union VR4300CP1Register *fd = &cp1->regs[GET_FD(rfexLatch->iw)];
  double value;

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "fldl %1\n\t"
    "fchs\n\t"
    "fstpl %0\n\t"
    : "=m" (value)
    : "m" (fs->d.data)
    : "st"
    );
#endif

  if (FPUUpdateState(cp1)) {
    FPURaiseException(vr4300);
    return;
  }

  fd->d.data = value;
}

/* ============================================================================
 *  Instruction: NEG.s (Floating-Point Negate).
 * ========================================================================= */
static void
VR4300NEGs(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  union VR4300CP1Register *fd = &cp1->regs[GET_FD(rfexLatch->iw)];
  float value;

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "flds %1\n\t"
    "fchs\n\t"
    "fstp %0\n\t"
    : "=m" (value)
    : "m" (fs->s.data[0])
    : "st"
    );
#endif

  if (FPUUpdateState(cp1)) {
    FPURaiseException(vr4300);
    return;
  }

  fd->s.data[0] = value;
}

/* ============================================================================
 *  Instruction: SDC1 (Store Doubleword From Coprocessor 1)
 * ========================================================================= */
void
VR4300SDC1(struct VR4300 *vr4300, uint64_t rs, uint64_t unused(rt)) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;

  unsigned ft = GET_FT(rfexLatch->iw);
  int64_t imm = (int16_t) rfexLatch->iw;
  uint64_t address = rs + imm;

  if (!FPUCheckUsable(vr4300))
    return;

  if (vr4300->cp0.regs.status.fr)
    exdcLatch->memoryData.data = vr4300->cp1.regs[ft].l.data;
  else
    exdcLatch->memoryData.data = vr4300->cp1.regs[ft & 0x1E].l.data;

  exdcLatch->memoryData.address = address;
  exdcLatch->memoryData.function = &VR4300StoreDWord;
}

/* ============================================================================
 *  Instruction: SQRT.d (Floating-Point Square Root).
 * ========================================================================= */
static void
VR4300SQRTd(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  union VR4300CP1Register *fd = &cp1->regs[GET_FD(rfexLatch->iw)];
  double value;

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "fldl %1\n\t"
    "fsqrt\n\t"
    "fstpl %0\n\t"
    : "=m" (value)
    : "m" (fs->d.data)
    : "st"
    );
#endif

  if (FPUUpdateState(cp1)) {
    FPURaiseException(vr4300);
    return;
  }

  fd->d.data = value;
}

/* ============================================================================
 *  Instruction: SQRT.s (Floating-Point Square Root).
 * ========================================================================= */
static void
VR4300SQRTs(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  union VR4300CP1Register *fd = &cp1->regs[GET_FD(rfexLatch->iw)];
  float value;

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "flds %1\n\t"
    "fsqrt\n\t"
    "fstp %0\n\t"
    : "=m" (value)
    : "m" (fs->s.data[0])
    : "st"
    );
#endif

  if (FPUUpdateState(cp1)) {
    FPURaiseException(vr4300);
    return;
  }

  fd->s.data[0] = value;
}

/* ============================================================================
 *  Instruction: SUB.d (Floating-Point Subtract).
 * ========================================================================= */
static void
VR4300SUBd(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  const union VR4300CP1Register *ft = &cp1->regs[GET_FT(rfexLatch->iw)];
  union VR4300CP1Register *fd = &cp1->regs[GET_FD(rfexLatch->iw)];
  double value;

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "fldl %1\n\t"
    "fldl %2\n\t"
    "fsubrp\n\t"
    "fstpl %0\n\t"
    : "=m" (value)
    : "m" (fs->d.data),
      "m" (ft->d.data)
    : "st"
    );
#endif

  if (FPUUpdateState(cp1)) {
    FPURaiseException(vr4300);
    return;
  }

  fd->d.data = value;
}

/* ============================================================================
 *  Instruction: SUB.s (Floating-Point Subtract).
 * ========================================================================= */
static void
VR4300SUBs(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  const union VR4300CP1Register *ft = &cp1->regs[GET_FT(rfexLatch->iw)];
  union VR4300CP1Register *fd = &cp1->regs[GET_FD(rfexLatch->iw)];
  float value;

  FPUClearExceptions();

#ifdef USE_X87FPU
  __asm__ volatile(
    "flds %1\n\t"
    "flds %2\n\t"
    "fsubrp\n\t"
    "fstp %0\n\t"
    : "=m" (value)
    : "m" (fs->s.data[0]),
      "m" (ft->s.data[0])
    : "st"
    );
#endif

  if (FPUUpdateState(cp1)) {
    FPURaiseException(vr4300);
    return;
  }

  fd->s.data[0] = value;
}

/* ============================================================================
 *  Instruction: SWC1 (Store Word From Coprocessor 1)
 * ========================================================================= */
void
VR4300SWC1(struct VR4300 *vr4300, uint64_t rs, uint64_t unused(rt)) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300EXDCLatch *exdcLatch = &vr4300->pipeline.exdcLatch;

  unsigned ft = GET_FT(rfexLatch->iw);
  int64_t imm = (int16_t) rfexLatch->iw;
  uint64_t address = rs + imm;

  if (!FPUCheckUsable(vr4300))
    return;

  if (vr4300->cp0.regs.status.fr)
    exdcLatch->memoryData.data = vr4300->cp1.regs[ft].w.data[0];
  else
    exdcLatch->memoryData.data = vr4300->cp1.regs[ft & 0x1E].w.data[ft & 0x1];

  exdcLatch->memoryData.address = address;
  exdcLatch->memoryData.function = &VR4300StoreWord;
}

/* ============================================================================
 *  Instruction: TRUNC.w.d (Floating-Point Truncate To Single Fixed-Point)
 * ========================================================================= */
static void
VR4300TRUNCwd(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  unsigned fd = GET_FD(rfexLatch->iw);
  uint32_t value;
  int mode;

  FPUClearExceptions();
  mode = fegetround();
  fesetround(FE_TOWARDZERO);

#ifdef USE_X87FPU
  __asm__ volatile(
    "fldl %1\n\t"
    "fistpl %0\n\t"
    : "=m" (value)
    : "m" (fs->l.data)
    : "st"
  );
#endif

  fesetround(mode);
  if (FPUUpdateState(cp1)) {
    FPURaiseException(vr4300);
    return;
  }

  if (vr4300->cp0.regs.status.fr)
    cp1->regs[fd].w.data[0] = value;
  else
    cp1->regs[fd & 0x1E].w.data[fd & 0x1] = value;
}

/* ============================================================================
 *  Instruction: TRUNC.w.s (Floating-Point Truncate To Single Fixed-Point)
 * ========================================================================= */
static void
VR4300TRUNCws(struct VR4300 *vr4300) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  const union VR4300CP1Register *fs = &cp1->regs[GET_FS(rfexLatch->iw)];
  unsigned fd = GET_FD(rfexLatch->iw);
  uint32_t value;
  int mode;

  FPUClearExceptions();
  mode = fegetround();
  fesetround(FE_TOWARDZERO);

#ifdef USE_X87FPU
  __asm__ volatile(
    "flds %1\n\t"
    "fistpl %0\n\t"
    : "=m" (value)
    : "m" (fs->s.data[0])
    : "st"
  );
#endif

  fesetround(mode);
  if (FPUUpdateState(cp1)) {
    FPURaiseException(vr4300);
    return;
  }

  if (vr4300->cp0.regs.status.fr)
    cp1->regs[fd].w.data[0] = value;
  else
    cp1->regs[fd & 0x1E].w.data[fd & 0x1] = value;
}

/* ============================================================================
 *  Instruction: BC1 (Branch On Coprocessor 1 Instructions)
 * ========================================================================= */
void
VR4300BC1(struct VR4300 *vr4300, uint64_t unused(rs), uint64_t unused(rt)) {
  struct VR4300ICRFLatch *icrfLatch = &vr4300->pipeline.icrfLatch;
  struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;
  struct VR4300CP1 *cp1 = &vr4300->cp1;

  int64_t address = ((int16_t) rfexLatch->iw << 2) - 4;

  if (!FPUCheckUsable(vr4300))
    return;

  switch(rfexLatch->iw >> 16 & 0x3) {
    case 0: /* BC1F */
      if (cp1->control.coc != 0)
        return;

      break;

    case 1: /* BC1T */
      if (cp1->control.coc == 0)
        return;

      break;

    case 2: /* BC1FL */
      if (cp1->control.coc != 0) {
        icrfLatch->iwMask = 0;
        return;
      }

      break;

    case 3: /* BC1TL */
      if (cp1->control.coc == 0) {
        icrfLatch->iwMask = 0;
        return;
      }

      break;
  }

  icrfLatch->pc += address;
}

/* ============================================================================
 *  Instruction: FPU.d (Coprocessor 1 FPU.d Operation)
 * ========================================================================= */
static void
VR4300FPUDInvalid(struct VR4300 *unused(vr4300)) {
  debug("Unimplemented function: FPU.d.");
}

static const FPUOperation fpudFunctions[64] = {
  VR4300ADDd,        VR4300SUBd,        VR4300MULd,        VR4300DIVd,
  VR4300SQRTd,       VR4300ABSd,        VR4300MOVd,        VR4300NEGd,
  VR4300FPUDInvalid, VR4300FPUDInvalid, VR4300FPUDInvalid, VR4300FPUDInvalid,
  VR4300FPUDInvalid, VR4300TRUNCwd,     VR4300FPUDInvalid, VR4300FPUDInvalid,
  VR4300FPUDInvalid, VR4300FPUDInvalid, VR4300FPUDInvalid, VR4300FPUDInvalid,
  VR4300FPUDInvalid, VR4300FPUDInvalid, VR4300FPUDInvalid, VR4300FPUDInvalid,
  VR4300FPUDInvalid, VR4300FPUDInvalid, VR4300FPUDInvalid, VR4300FPUDInvalid,
  VR4300FPUDInvalid, VR4300FPUDInvalid, VR4300FPUDInvalid, VR4300FPUDInvalid,
  VR4300CVTsd,       VR4300FPUDInvalid, VR4300FPUDInvalid, VR4300FPUDInvalid,
  VR4300CVTwd,       VR4300FPUDInvalid, VR4300FPUDInvalid, VR4300FPUDInvalid,
  VR4300FPUDInvalid, VR4300FPUDInvalid, VR4300FPUDInvalid, VR4300FPUDInvalid,
  VR4300FPUDInvalid, VR4300FPUDInvalid, VR4300FPUDInvalid, VR4300FPUDInvalid,
  VR4300Cfd,         VR4300Cund,        VR4300Ceqd,        VR4300Cueqd,
  VR4300Coltd,       VR4300Cultd,       VR4300Coled,       VR4300Culed,
  VR4300Csfd,        VR4300Cngled,      VR4300Cseqd,       VR4300Cngld,
  VR4300Cltd,        VR4300Cnged,       VR4300Cled,        VR4300Cngtd
};

void
VR4300FPUD(struct VR4300 *vr4300, uint64_t unused(rs), uint64_t unused(rt)) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;

  if (!FPUCheckUsable(vr4300))
    return;

  fpudFunctions[rfexLatch->iw & 0x3F](vr4300);
}

/* ============================================================================
 *  Instruction: FPU.l (Coprocessor 1 FPU.l Operation)
 * ========================================================================= */
static void
VR4300FPULInvalid(struct VR4300 *unused(vr4300)) {
  debug("Unimplemented function: FPU.l.");
}

static const FPUOperation fpulFunctions[64] = {
  VR4300FPULInvalid, VR4300FPULInvalid, VR4300FPULInvalid, VR4300FPULInvalid,
  VR4300FPULInvalid, VR4300FPULInvalid, VR4300FPULInvalid, VR4300FPULInvalid,
  VR4300FPULInvalid, VR4300FPULInvalid, VR4300FPULInvalid, VR4300FPULInvalid,
  VR4300FPULInvalid, VR4300FPULInvalid, VR4300FPULInvalid, VR4300FPULInvalid,
  VR4300FPULInvalid, VR4300FPULInvalid, VR4300FPULInvalid, VR4300FPULInvalid,
  VR4300FPULInvalid, VR4300FPULInvalid, VR4300FPULInvalid, VR4300FPULInvalid,
  VR4300FPULInvalid, VR4300FPULInvalid, VR4300FPULInvalid, VR4300FPULInvalid,
  VR4300FPULInvalid, VR4300FPULInvalid, VR4300FPULInvalid, VR4300FPULInvalid,
  VR4300FPULInvalid, VR4300FPULInvalid, VR4300FPULInvalid, VR4300FPULInvalid,
  VR4300FPULInvalid, VR4300FPULInvalid, VR4300FPULInvalid, VR4300FPULInvalid,
  VR4300FPULInvalid, VR4300FPULInvalid, VR4300FPULInvalid, VR4300FPULInvalid,
  VR4300FPULInvalid, VR4300FPULInvalid, VR4300FPULInvalid, VR4300FPULInvalid,
  VR4300FPULInvalid, VR4300FPULInvalid, VR4300FPULInvalid, VR4300FPULInvalid,
  VR4300FPULInvalid, VR4300FPULInvalid, VR4300FPULInvalid, VR4300FPULInvalid,
  VR4300FPULInvalid, VR4300FPULInvalid, VR4300FPULInvalid, VR4300FPULInvalid,
  VR4300FPULInvalid, VR4300FPULInvalid, VR4300FPULInvalid, VR4300FPULInvalid
};

void
VR4300FPUL(struct VR4300 *vr4300, uint64_t unused(rs), uint64_t unused(rt)) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;

  if (!FPUCheckUsable(vr4300))
    return;

  fpulFunctions[rfexLatch->iw & 0x3F](vr4300);
}

/* ============================================================================
 *  Instruction: FPU.s (Coprocessor 1 FPU.s Operation)
 * ========================================================================= */
static void
VR4300FPUSInvalid(struct VR4300 *unused(vr4300)) {
  debug("Unimplemented function: FPU.s.");
}

static const FPUOperation fpusFunctions[64] = {
  VR4300ADDs,        VR4300SUBs,        VR4300MULs,        VR4300DIVs,
  VR4300SQRTs,       VR4300ABSs,        VR4300MOVs,        VR4300NEGs,
  VR4300FPUSInvalid, VR4300FPUSInvalid, VR4300FPUSInvalid, VR4300FPUSInvalid,
  VR4300FPUSInvalid, VR4300TRUNCws,     VR4300FPUSInvalid, VR4300FPUSInvalid,
  VR4300FPUSInvalid, VR4300FPUSInvalid, VR4300FPUSInvalid, VR4300FPUSInvalid,
  VR4300FPUSInvalid, VR4300FPUSInvalid, VR4300FPUSInvalid, VR4300FPUSInvalid,
  VR4300FPUSInvalid, VR4300FPUSInvalid, VR4300FPUSInvalid, VR4300FPUSInvalid,
  VR4300FPUSInvalid, VR4300FPUSInvalid, VR4300FPUSInvalid, VR4300FPUSInvalid,
  VR4300FPUSInvalid, VR4300CVTds,       VR4300FPUSInvalid, VR4300FPUSInvalid,
  VR4300CVTws,       VR4300FPUSInvalid, VR4300FPUSInvalid, VR4300FPUSInvalid,
  VR4300FPUSInvalid, VR4300FPUSInvalid, VR4300FPUSInvalid, VR4300FPUSInvalid,
  VR4300FPUSInvalid, VR4300FPUSInvalid, VR4300FPUSInvalid, VR4300FPUSInvalid,
  VR4300Cfs,         VR4300Cuns,        VR4300Ceqs,        VR4300Cueqs,
  VR4300Colts,       VR4300Cults,       VR4300Coles,       VR4300Cules,
  VR4300Csfs,        VR4300Cngles,      VR4300Cseqs,       VR4300Cngls,
  VR4300Clts,        VR4300Cnges,       VR4300Cles,        VR4300Cngts
};

void
VR4300FPUS(struct VR4300 *vr4300, uint64_t unused(rs), uint64_t unused(rt)) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;

  if (!FPUCheckUsable(vr4300))
    return;

  fpusFunctions[rfexLatch->iw & 0x3F](vr4300);
}

/* ============================================================================
 *  Instruction: FPU.w (Coprocessor 1 FPU.w Operation)
 * ========================================================================= */
static void
VR4300FPUWInvalid(struct VR4300 *unused(vr4300)) {
  debug("Unimplemented function: FPU.w.");
}

static const FPUOperation fpuwFunctions[64] = {
  VR4300FPUWInvalid, VR4300FPUWInvalid, VR4300FPUWInvalid, VR4300FPUWInvalid,
  VR4300FPUWInvalid, VR4300FPUWInvalid, VR4300FPUWInvalid, VR4300FPUWInvalid,
  VR4300FPUWInvalid, VR4300FPUWInvalid, VR4300FPUWInvalid, VR4300FPUWInvalid,
  VR4300FPUWInvalid, VR4300FPUWInvalid, VR4300FPUWInvalid, VR4300FPUWInvalid,
  VR4300FPUWInvalid, VR4300FPUWInvalid, VR4300FPUWInvalid, VR4300FPUWInvalid,
  VR4300FPUWInvalid, VR4300FPUWInvalid, VR4300FPUWInvalid, VR4300FPUWInvalid,
  VR4300FPUWInvalid, VR4300FPUWInvalid, VR4300FPUWInvalid, VR4300FPUWInvalid,
  VR4300FPUWInvalid, VR4300FPUWInvalid, VR4300FPUWInvalid, VR4300FPUWInvalid,
  VR4300CVTsw,       VR4300CVTdw,       VR4300FPUWInvalid, VR4300FPUWInvalid,
  VR4300FPUWInvalid, VR4300FPUWInvalid, VR4300FPUWInvalid, VR4300FPUWInvalid,
  VR4300FPUWInvalid, VR4300FPUWInvalid, VR4300FPUWInvalid, VR4300FPUWInvalid,
  VR4300FPUWInvalid, VR4300FPUWInvalid, VR4300FPUWInvalid, VR4300FPUWInvalid,
  VR4300FPUWInvalid, VR4300FPUWInvalid, VR4300FPUWInvalid, VR4300FPUWInvalid,
  VR4300FPUWInvalid, VR4300FPUWInvalid, VR4300FPUWInvalid, VR4300FPUWInvalid,
  VR4300FPUWInvalid, VR4300FPUWInvalid, VR4300FPUWInvalid, VR4300FPUWInvalid,
  VR4300FPUWInvalid, VR4300FPUWInvalid, VR4300FPUWInvalid, VR4300FPUWInvalid
};

void
VR4300FPUW(struct VR4300 *vr4300, uint64_t unused(rs), uint64_t unused(rt)) {
  const struct VR4300RFEXLatch *rfexLatch = &vr4300->pipeline.rfexLatch;

  if (!FPUCheckUsable(vr4300))
    return;

  fpuwFunctions[rfexLatch->iw & 0x3F](vr4300);
}

/* ============================================================================
 *  VR4300InitCP1: Initializes the co-processor.
 * ========================================================================= */
void
VR4300InitCP1(struct VR4300CP1 *cp1) {
  debug("Initializing CP1.");
  memset(cp1, 0, sizeof(*cp1));
}

