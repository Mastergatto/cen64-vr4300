/* ============================================================================
 *  TLBTree.c: TLB backing structure for O(lg n) lookups.
 *
 *  VR4300SIM: NEC VR43xx Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#include "Common.h"
#include "TLBTree.h"

#ifdef __cplusplus
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#else
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#endif

/* Internal functions used to inspect entries of the tree. */
static bool DoASIDsMatch(const struct TLBNode *, const struct TLBNode *);
static bool DoEntriesOverlap(const struct TLBNode *, const struct TLBNode *);
static uint32_t GetEntryEndAddress(const struct TLBNode *);
static bool IsGlobalEntry(const struct TLBNode *);
static const struct TLBNode* SearchTree(const struct TLBNode *,
  const struct TLBNode *, uint32_t);

/* Internal functions used to maintain the state of the tree. */
static void RotateLeft(struct TLBNode **,
  const struct TLBNode *, struct TLBNode *);
static void RotateRight(struct TLBNode **,
  const struct TLBNode *, struct TLBNode *);
static void Transplant(struct TLBNode **, const struct TLBNode *,
  struct TLBNode *, struct TLBNode *);
static void TLBTreeFixup(struct TLBNode **,
  struct TLBNode *, struct TLBNode *);

/* ============================================================================
 *  DoEntriesOverlap: Determines if two entries have overlapping addresses.
 * ========================================================================= */
static bool
DoEntriesOverlap(const struct TLBNode *x, const struct TLBNode *y) {
  if (GetEntryEndAddress(x) < GetEntryEndAddress(y)) {
    if (GetEntryEndAddress(x) >= y->tlbEntryHi.vpn)
      return false;
  }

  else if (x->tlbEntryHi.vpn <= GetEntryEndAddress(y))
    return true;

  return false;
}

/* ============================================================================
 *  DoASIDsMatch: Determines if two entries have matching ASID values.
 * ========================================================================= */
static bool
DoASIDsMatch(const struct TLBNode *x, const struct TLBNode *y) {
  return x->tlbEntryHi.asid == y->tlbEntryHi.asid;
}

/* ============================================================================
 *  InitTLBTree: Initializes the TLBTree.
 * ========================================================================= */
void
InitTLBTree(struct TLBTree *tree) {
  memset(tree, 0, sizeof(*tree));

  tree->globalEntryRoot = &tree->globalNilNode;
  tree->asidEntryRoot = &tree->asidNilNode;
}

/* ============================================================================
 *  IsGlobalEntry: Determines if the global bit (G) is set in both EntryLos.
 * ========================================================================= */
static bool
IsGlobalEntry(const struct TLBNode *x) {
  return x->tlbEntryLo0.global && x->tlbEntryLo1.global;
}

/* ============================================================================
 *  GetEntryEndAddress: Determines the highest addressable byte of the entry.
 * ========================================================================= */
static uint32_t
GetEntryEndAddress(const struct TLBNode *x) {
  return x->tlbEntryHi.vpn + x->pageMask;
}

/* ============================================================================
 *  RotateLeft: Perform a left rotation (centered at n).
 * ========================================================================= */
static void
RotateLeft(struct TLBNode **root,
  const struct TLBNode *nil, struct TLBNode *n) {
  struct TLBNode *y = n->right;

  /* Turn y's left subtree into n's right subtree. */
  n->right = y->left;

  if (y->left != nil)
    y->left->parent = n;

  /* Link n's parent to y. */
  y->parent = n->parent;

  if (n->parent == nil)
    *root = y;
  else if (n == n->parent->left)
    n->parent->left = y;
  else
    n->parent->right = y;

  /* Put n on y's left. */
  y->left = n;
  n->parent = y;
}

/* ============================================================================
 *  RotateRight: Perform a right rotation (centered at n).
 * ========================================================================= */
static void
RotateRight(struct TLBNode **root,
  const struct TLBNode *nil, struct TLBNode *n) {
  struct TLBNode *y = n->left;

  /* Turn y's right subtree into n's left subtree. */
  n->left = y->right;

  if (y->right != nil)
    y->right->parent = n;

  /* Link n's parent to y. */
  y->parent = n->parent;

  if (n->parent == nil)
    *root = y;
  else if (n == n->parent->left)
    n->parent->left = y;
  else
    n->parent->right = y;

  /* Put n on y's right. */
  y->right = n;
  n->parent = y;
}

/* ============================================================================
 *  SearchTree: Search a subtree to see if any of its entries maps a VPN.
 * ========================================================================= */
static const struct TLBNode*
SearchTree(const struct TLBNode *node,
  const struct TLBNode *nil, uint32_t vpn) {

  while (node != nil) {
    if (node->tlbEntryHi.vpn == (~node->pageMask & vpn))
      return node;

    else
      node = (vpn < node->tlbEntryHi.vpn)
        ? node->left : node->right;
  }

  return NULL;
}

/* ============================================================================
 *  TLBTreeEvict: Removes a TLB entry from the TLBTree.
 * ========================================================================= */
void
TLBTreeEvict(struct TLBTree *tree, struct TLBNode *node) {
  struct TLBNode *fixup, *original = node, *nil, *remove = node;
  enum TLBTreeColor originalColor = node->color;
  struct TLBNode **root;

  /* Which tree is entry in? */
  if (IsGlobalEntry(node)) {
    root = &tree->globalEntryRoot;
    nil = &tree->globalNilNode;
  }

  else {
    root = &tree->asidEntryRoot;
    nil = &tree->asidNilNode;
  }

  /* Check for the easy way out: is either branch is a leaf? */
  if (node->left == nil) {
    fixup = node->right;
    Transplant(root, nil, remove, node->right);
  }

  else if (node->right == nil) {
    fixup = node->left;
    Transplant(root, nil, remove, node->left);
  }

  /* Neither branch is a leaf... */
  /* Pull up the in-order successor. */
  else {
    original = node->right;
    while (original->left != nil)
      original = original->left;

    originalColor = original->color;
    fixup = original->right;

    if (original->parent == remove)
      fixup->parent = original;

    else {
      Transplant(root, nil, original, original->right);
      original->right = node->right;
      original->right->parent = original;
    }

    Transplant(root, nil, remove, original);
    original->left = node->left;
    original->left->parent = original;
    original->color = node->color;
  }

  /* Do we need to rebalance the tree? */
  if (originalColor == TLBTREE_BLACK) {
    struct TLBNode *sibling;

    while (fixup != *root && fixup->color == TLBTREE_BLACK) {
      if (fixup == fixup->parent->left) {
        sibling = fixup->parent->right;

        /* Case 1: Sibling is red. */
        if (sibling->color == TLBTREE_RED) {
          sibling->color = TLBTREE_BLACK;
          fixup->parent->color = TLBTREE_RED;
          RotateLeft(root, nil, fixup->parent);
          sibling = fixup->parent->right;
        }

        /* Case 2: Both sibling and it's children are all black. */
        if (sibling->left->color == TLBTREE_BLACK &&
            sibling->right->color == TLBTREE_BLACK) {
          sibling->color = TLBTREE_RED;
          fixup = fixup->parent;
        }

        else {

          /* Case 3: Sibling is black, left is red, and right is black. */
          if (sibling->right->color == TLBTREE_BLACK) {
            sibling->left->color = TLBTREE_BLACK;
            sibling->color = TLBTREE_RED;
            RotateRight(root, nil, sibling);
            sibling = fixup->parent->right;
          }

          /* Case 4: Sibling is black, left is black, and right is red. */
          sibling->color = fixup->parent->color;
          fixup->parent->color = TLBTREE_BLACK;
          sibling->right->color = TLBTREE_BLACK;
          RotateLeft(root, nil, fixup->parent);
          fixup = *root;
        }
      }

      else {
        sibling = fixup->parent->left;

        /* Case 1: Sibling is red. */
        if (sibling->color == TLBTREE_RED) {
          sibling->color = TLBTREE_BLACK;
          fixup->parent->color = TLBTREE_RED;
          RotateRight(root, nil, fixup->parent);
          sibling = fixup->parent->left;
        }

        /* Case 2: Both sibling and it's children are all black. */
        if (sibling->left->color == TLBTREE_BLACK &&
            sibling->right->color == TLBTREE_BLACK) {
          sibling->color = TLBTREE_RED;
          fixup = fixup->parent;
        }

        else {

          /* Case 3: Sibling is black, left is black, and right is red. */
          if (sibling->left->color == TLBTREE_BLACK) {
            sibling->right->color = TLBTREE_BLACK;
            sibling->color = TLBTREE_RED;
            RotateLeft(root, nil, sibling);
            sibling = fixup->parent->left;
          }

          /* Case 4: Sibling is black, left is red, and right is black. */
          sibling->color = fixup->parent->color;
          fixup->parent->color = TLBTREE_BLACK;
          sibling->left->color = TLBTREE_BLACK;
          RotateRight(root, nil, fixup->parent);
          fixup = *root;
        }
      }
    }

    fixup->color = TLBTREE_BLACK;
  }
}

/* ============================================================================
 *  TLBTreeFixup: Rebalances the tree after `node` is inserted.
 * ========================================================================= */
static void
TLBTreeFixup(struct TLBNode **root, struct TLBNode *nil, struct TLBNode *node){
  struct TLBNode *cur;

  /* Rebalance the tree if needed. */
  while (node->parent->color == TLBTREE_RED) {
    if (node->parent == node->parent->parent->left) {
      cur = node->parent->parent->right;

      /* Case 1: We only need to update colors. */
      if (cur->color == TLBTREE_RED) {
        node->parent->color = TLBTREE_BLACK;
        cur->color = TLBTREE_BLACK;
        node->parent->parent->color = TLBTREE_RED;
        node = node->parent->parent;
      }

      else {

        /* Case 2: We need to perform a left rotation. */
        if (node == node->parent->right) {
          node = node->parent;
          RotateLeft(root, nil, node);
        }

        /* Case 3: We need to perform a right rotation. */
        node->parent->color = TLBTREE_BLACK;
        node->parent->parent->color = TLBTREE_RED;
        RotateRight(root, nil, node->parent->parent);
      }
    }

    else {
      cur = node->parent->parent->left;

      /* Case 1: We only need to update colors. */
      if (cur->color == TLBTREE_RED) {
        node->parent->color = TLBTREE_BLACK;
        cur->color = TLBTREE_BLACK;
        node->parent->parent->color = TLBTREE_RED;
        node = node->parent->parent;
      }

      else {

        /* Case 2: We need to perform a right rotation. */
        if (node == node->parent->left) {
          node = node->parent;
          RotateRight(root, nil, node);
        }

        /* Case 3: We need to perform a left rotation. */
        node->parent->color = TLBTREE_BLACK;
        node->parent->parent->color = TLBTREE_RED;
        RotateLeft(root, nil, node->parent->parent);
      }
    }
  }

  /* When we rebalanced the tree, we might have accidentally colored */
  /* the root red, so unconditionally color if back after rebalancing. */
  (*root)->color = TLBTREE_BLACK;
}

/* ============================================================================
 *  TLBTreeInsert: Inserts a 64-bit TLBEntry into the TLBTree.
 * ========================================================================= */
int
TLBTreeInsert(struct TLBTree *tree, struct TLBNode *node) {
  struct TLBNode **root, *nil, *check, *cur;

  /* Which tree are we using? */
  if (IsGlobalEntry(node)) {
    root = &tree->globalEntryRoot;
    nil = &tree->globalNilNode;
  }

  else {
    root = &tree->asidEntryRoot;
    nil = &tree->asidNilNode;
  }

  check = *root;
  cur = nil;

  /* Merge the region into the VPN to cut down on lookup time. */
  node->tlbEntryHi.vpn |= ((uint32_t) node->tlbEntryHi.region) << 27;
  const uint32_t entryEndAddress = GetEntryEndAddress(node);

  /* Walk down the tree. */
  if (!IsGlobalEntry(node)) {
    while (check != nil && DoASIDsMatch(node, check)) {
      cur = check;

      check = (node->tlbEntryHi.asid < check->tlbEntryHi.asid)
        ? check->left : check->right;
    }
  }

  while (check != nil) {
    cur = check;

    check = (entryEndAddress < GetEntryEndAddress(check))
      ? check->left : check->right;
  }

  /* Check for overlaps. */
  if (!IsGlobalEntry(node)) {
    struct TLBNode *globalCheck = tree->globalEntryRoot;
    const struct TLBNode *globalCur = &tree->globalNilNode;

    while (globalCheck != &tree->globalNilNode) {
      globalCur = globalCheck;

      globalCheck = (entryEndAddress < GetEntryEndAddress(globalCheck))
        ? globalCheck->left : globalCheck->right;
    }

    if (globalCur != &tree->globalNilNode) {
      if (DoEntriesOverlap(node, globalCur))
        return 1;
    }
  }

  /* Insert the entry. */
  if (cur != nil) {
    if (IsGlobalEntry(node) || DoASIDsMatch(node, cur))
      if (DoEntriesOverlap(node, cur))
        return 1;

    if (entryEndAddress < GetEntryEndAddress(cur))
      cur->left = node;
    else
      cur->right = node;
  }

  else
    *root = node;

  /* Initialize the entry. */
  node->left = check;
  node->parent = cur;
  node->right = check;

  /* Rebalance the tree. */
  node->color = TLBTREE_RED;
  TLBTreeFixup(root, nil, node);

  return 0;
}

/* ============================================================================
 *  TLBTreeLookup: Looks up a physical address for a 64-bit virtual address.
 * ========================================================================= */
const struct TLBNode*
TLBTreeLookup(const struct TLBTree *tree, uint8_t asid, uint64_t address) {
  uint32_t vpn = ((uint32_t) (address >> 13)) & 0x07FFFFFFU;
  uint8_t region = (address >> 62) & 0x3;
  const struct TLBNode *node;

  /* Cut down on lookup time... */
  /* Merge the region into the VPN. */
  vpn |= ((uint32_t) region) << 27;

  /* Search the subtree containing global entries first. */
  if ((node = SearchTree(tree->globalEntryRoot, &tree->globalNilNode,  vpn)))
    return node;

  node = tree->asidEntryRoot;

  /* Otherwise, start walking down the ASIDs of the other subtree. */
  while (node != &tree->asidNilNode && asid != node->tlbEntryHi.asid) {
    node = (asid < node->tlbEntryHi.asid)
      ? node->left : node->right;
  }

  /* Search the part of the tree with the same ASIDs. */
  if ((node = SearchTree(node, &tree->asidNilNode, vpn)))
    return node;

  return NULL;
}

/* ============================================================================
 *  Transplant: Replaces the subtree rooted at u with the subtree rooted at v.
 * ========================================================================= */
static void
Transplant(struct TLBNode **root, const struct TLBNode *nil,
  struct TLBNode *u, struct TLBNode *v) {

  if (u->parent == nil)
    *root = v;
  else if (u == u->parent->left)
    u->parent->left = v;
  else
    u->parent->right = v;

  v->parent = u->parent;
}

