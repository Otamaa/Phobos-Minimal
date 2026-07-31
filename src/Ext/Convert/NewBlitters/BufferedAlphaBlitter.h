#pragma once

#include "Blitters.h"

#include "AlphaCursor.h"

// ---------------------------------------------------------------------------
//  Leaf types that carry extra state. Everything else is a bare 0x10 root.
//
//  Only the buffered pair gets an extra virtual: .rdata gives Blitters[12] a
//  6th vtable slot and RLEBlitters[12] a 4th, both pointing at 0x1004D080.
//  That function is a two-line absolute SETTER for Cursor:
//      this[4] = a2; return a2;
//  One body serves both hierarchies purely because Cursor is at +0x10 in each.
//  The extra virtual and the extra three dwords arrive together, which is why
//  only these two blitters are 0x1C.
// ---------------------------------------------------------------------------
class BufferedAlphaBlitter : public BlitterCore
{
public:
	/* +0x10 */ AlphaCursor Alpha;
};

class RLEBufferedAlphaBlitter : public RLEBlitterCore
{
public:
	/* +0x10 */ AlphaCursor Alpha;
};

static_assert(sizeof(BufferedAlphaBlitter) == 0x1C, "operator new(0x1C)");
static_assert(sizeof(RLEBufferedAlphaBlitter) == 0x1C, "operator new(0x1C)");
// FIX: BlitterCustom / BlitterCustomRLE used to be redeclared in this file as
// BlitterCore-derived overlays, WHILE this header included BlitterCustomRLE.h
// (which pulls in BlitterCustom.h). Two different classes, same names, same
// translation unit:
//     error: redefinition of 'class BlitterCustom'
//     error: redefinition of 'class BlitterCustomRLE'
// The real reimplementations in BlitterCustom.h / BlitterCustomRLE.h supersede
// the overlays, so the duplicates and the include are both gone. Their size
// assertions live with the real classes.
