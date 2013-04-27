/* ============================================================================
 *  CP0.h: VR4300 Coprocessor #0.
 *
 *  VR4300SIM: NEC VR43xx Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#ifndef __VR4300__CP0__
#define __VR4300__CP0__
#include "Common.h"

/* Register list. */
enum VR4300CP0RegisterID {
  VR4300_CP0_REGISTER_INDEX,
  VR4300_CP0_REGISTER_RANDOM,
  VR4300_CP0_REGISTER_ENTRYLO0,
  VR4300_CP0_REGISTER_ENTRYLO1,
  VR4300_CP0_REGISTER_CONTEXT,
  VR4300_CP0_REGISTER_PAGEMASK,
  VR4300_CP0_REGISTER_WIRED,
  VR4300_CP0_REGISTER_BADVADDR = 8,
  VR4300_CP0_REGISTER_COUNT,
  VR4300_CP0_REGISTER_ENTRYHI,
  VR4300_CP0_REGISTER_COMPARE,
  VR4300_CP0_REGISTER_STATUS,
  VR4300_CP0_REGISTER_CAUSE,
  VR4300_CP0_REGISTER_EPC,
  VR4300_CP0_REGISTER_PRID,
  VR4300_CP0_REGISTER_CONFIG,
  VR4300_CP0_REGISTER_LLADDR,
  VR4300_CP0_REGISTER_WATCHLO,
  VR4300_CP0_REGISTER_WATCHHI,
  VR4300_CP0_REGISTER_XCONTEXT,
  VR4300_CP0_REGISTER_PARITYERROR = 26,
  VR4300_CP0_REGISTER_CACHEERR,
  VR4300_CP0_REGISTER_TAGLO,
  VR4300_CP0_REGISTER_TAGHI,
  VR4300_CP0_REGISTER_ERROREPC,
};

/* TLB constants, types. */
#define PAGEMASK_4K 0x000
#define PAGEMASK_8K 0x003
#define PAGEMASK_16K 0x00F
#define PAGEMASK_64K 0x03F
#define PAGEMASK_256K 0x0FF
#define PAGEMASK_4M 0x3FF
#define PAGEMASK_16M 0xFFF

struct EntryLo {
  uint32_t pfn;
  uint8_t attribute;
  uint8_t dirty;
  uint8_t global;
  uint8_t valid;
};

struct EntryHi {
  uint32_t vpn;
  uint8_t asid;
  uint8_t region;
};

/* List of registers. */
struct VR4300CP0Registers {

  /* BadVAddr (8): 8 bytes. */
  uint64_t badVAddr;

  /* ErrorEPC (30): 8 bytes. */
  uint64_t errorEPC;

  /* EPC (14): 8 bytes. */
  uint64_t epc;

  /* Context (4): 12 bytes. */
  struct {
    uint64_t pteBase;
    uint32_t badVPN2;
  } context;

  /* Compare (11): 4 bytes. */
  uint32_t compare;

  /* Count (9): 4 bytes. */
  uint32_t count;

  /* PErr (26): 4 bytes. */
  uint32_t pErr;

  /* CacheErr (27): 4 bytes. */
  uint32_t cacheErr;

  /* EntryHi (10): 6 bytes. */
  struct EntryHi entryHi;

  /* PageMask (5): 2 bytes. */
  uint16_t pageMask;

  /* Wired (6): 1 byte. */
  uint8_t wired;

  /* LLBit (?): 1 byte. */
  uint8_t llBit;

  /* EntryLo (2,3): 8 bytes. */
  struct EntryLo entryLo0;
  struct EntryLo entryLo1;

  /* LLAddr (17): 4 bytes. */
  uint32_t LLAddr;

  /* TagLo (28): 5 bytes. */
  struct {
    uint32_t pTagLo;
    uint8_t pState;
  } tagLo;

  /* Config (16): 5 bytes. */
  struct {
    uint8_t be;
    uint8_t cu;
    uint8_t ec;
    uint8_t ep;
    uint8_t k0;
  } config;

  /* Index (0): 2 bytes. */
  struct {
    uint8_t probe;
    uint8_t index;
  } index;

  /* PRId (15): 2 bytes. */
  struct {
    uint8_t imp;
    uint8_t rev;
  } prid;

  /* Random (1): 1 byte. */
  uint8_t random;

  /* WatchHi (19): 1 byte. */
  uint8_t watchHi;

  /* Cause (13): 4 bytes. */
  struct {
    uint8_t bd;
    uint8_t ce;
    uint8_t excCode;
    uint8_t ip;
  } cause;

  /* Status (12): 19 bytes. */
  struct {
    struct {
      uint8_t bev;
      uint8_t ce;
      uint8_t ch;
      uint8_t de;
      uint8_t its;
      uint8_t sr;
      uint8_t ts;
    } ds;

    uint8_t cu;
    uint8_t erl;
    uint8_t exl;
    uint8_t fr;
    uint8_t ksu;
    uint8_t kx;
    uint8_t ie;
    uint8_t im;
    uint8_t re;
    uint8_t rp;
    uint8_t sx;
    uint8_t ux;
  } status;

  /* XContext (20): 9 bytes. */
  struct {
    uint32_t badVPN2;
    uint32_t pteBase;
    uint8_t r;
  } xContext;

  /* WatchLo (18): 6 bytes. */
  struct WatchLo {
    uint32_t pAddr0;
    uint8_t r;
    uint8_t w;
  } watchLo;
};

struct VR4300CP0 {
  struct VR4300CP0Registers regs;
  uint8_t canRaiseInterrupt;
};

void VR4300InitCP0(struct VR4300CP0 *);

#endif

