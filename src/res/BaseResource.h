#pragma once

// -----------------------------------------------------------------------------
// BaseResource.h - central resource ID range allocation for Phobos.dll
//
// Every .rc and every *.Resource.h in the DLL derives its IDs from a base
// defined here. Resource IDs only collide within a resource TYPE, but the
// linker merges .res files SILENTLY - no error, no warning, one blob simply
// wins. Disjoint ranges make that structurally impossible instead of
// accidentally avoided.
//
// Hard constraint: MAKEINTRESOURCE puns the ID into a pointer and tests the
// high word, so every ID must be in [1, 65535]. 0 is unusable.
//
// DIFF vs the first draft:
//   - RESHADE_RES_BASE moved 5000 -> 6000. The old value collided head-on with
//     the yrpp-spawner convention, which already owns 5000+ (see below).
//   - Added the OS-reserved band: VS_VERSION_INFO and RT_MANIFEST have IDs
//     mandated by Windows and can never be renumbered into a range.
//   - Added SPAWNER_RES_BASE to document the inherited range explicitly rather
//     than leaving it as an undocumented hole.
//   - Added *_RES_COUNT so ranges can be static_assert'd.
// -----------------------------------------------------------------------------


// --- Band 1-99: RESERVED BY WINDOWS - do not allocate ------------------------
//
// These values are fixed by the OS loader and by the resource APIs. Listed for
// documentation only; do not redefine them, <winuser.h> already has them.
//
//   1   VS_VERSION_INFO                        - GetFileVersionInfo only ever
//                                                looks at RT_VERSION id 1
//   1   CREATEPROCESS_MANIFEST_RESOURCE_ID     - RT_MANIFEST
//   2   ISOLATIONAWARE_MANIFEST_RESOURCE_ID    - RT_MANIFEST (in use:
//                                                ExceptionHandler.rc)
//   3   ISOLATIONAWARE_NOSTATICIMPORT_...      - RT_MANIFEST
//
// NOTE: Phobos.rc's `ID_VERSION 1` is correct and MUST stay 1.
#define WINRESERVED_RES_BASE        1      // 1 - 99


// --- Band 1000-1999: Phobos's own resources ----------------------------------
#define PHOBOS_RES_BASE             1000   // 1000 - 1999
#define PHOBOS_RES_COUNT            1000

// --- Band 2000-2999: Phobos dialogs ------------------------------------------
// Split out from PHOBOS_RES_BASE because RT_DIALOG is the type most likely to
// gain entries over time (ExceptionHandler, future config UI, etc).
#define PHOBOSDLG_RES_BASE          2000   // 2000 - 2999
#define PHOBOSDLG_RES_COUNT         1000

// --- Band 5000-5999: yrpp-spawner / CnCNet convention - DO NOT REALLOCATE -----
//
// INHERITED, NOT OURS. The spawner convention is that brand-new control IDs
// start at 5000 so they never collide with the engine's own dialog and control
// IDs. DesyncDialog.Resource.h already occupies 5000-5022 under this rule.
//
// This range is shared with CnCNet tooling - changing it here does not change
// it for them. Treat it as read-only and allocate new desync/spawner IDs from
// inside it, contiguously.
#define SPAWNER_RES_BASE            5000   // 5000 - 5999
#define SPAWNER_RES_COUNT           1000

// --- Band 6000-6999: vendored ReShade ----------------------------------------
// Currently 101-112 (RCDATA shader blobs) + 600 + 701-711 upstream. Rebasing
// these means editing vendored headers - mark every edit with // PHOBOS: so it
// survives the next rebase against 4.9.1-lineage source.
#define RESHADE_RES_BASE            6000   // 6000 - 6999
#define RESHADE_RES_COUNT           1000

// --- Band 9000-9999: PhobosLib_* static libraries ----------------------------
// Reserved. None of the six libs currently ships resources; this exists so that
// if one ever does, it lands somewhere predictable.
#define PHOBOSLIB_RES_BASE          9000   // 9000 - 9999
#define PHOBOSLIB_RES_COUNT         1000


// --- Range validation --------------------------------------------------------
//
// Include this header from ONE .cpp that also includes every *.Resource.h, then
// assert each ID. Anything that drifts out of its band becomes a compile error
// rather than a silent runtime blob swap.
//
// This catches the CAUSE (an ID outside its band); it does NOT catch two IDs
// colliding INSIDE one band. For that, keep the post-build .rsrc count audit -
// same script that scans for the 31 C0 C3 hook stubs.
//
// Usage in that .cpp:
//
//   #include "BaseResource.h"
//   #include "DesyncDialog.Resource.h"
//   #include "Reshade.Resource.h"
//   ...
//   PHOBOS_ASSERT_RES_RANGE(IDD_DESYNC_HOST, SPAWNER_RES_BASE, SPAWNER_RES_COUNT);
//   PHOBOS_ASSERT_RES_RANGE(IDR_COPY_PS,     RESHADE_RES_BASE, RESHADE_RES_COUNT);

#define PHOBOS_ASSERT_RES_RANGE(id, base, count)                               \
    static_assert((id) >= (base) && (id) < (base) + (count),                   \
        #id " is outside its allocated resource ID band - see BaseResource.h")

// MAKEINTRESOURCE puns the ID into a pointer; 0 and >65535 both break it.
#define PHOBOS_ASSERT_RES_VALID(id)                                            \
    static_assert((id) >= 1 && (id) <= 65535,                                  \
        #id " is not a valid MAKEINTRESOURCE id (must be 1..65535)")

