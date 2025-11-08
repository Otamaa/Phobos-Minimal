#include "Handle.h"

// ============================================================================
// COMPARISON TABLE: When to use what
// ============================================================================

/*
+------------------+-------------+-------------+-------------+---------------+-------------+
| Feature          | OwningPtr   | LinkedPtr   | ObservingPtr| MoveOnlyPtr   | ValueWrapper|
+------------------+-------------+-------------+-------------+---------------+-------------+
| Owns object      | YES         | YES         | NO          | NO            | YES (value) |
| Destroy on dtor  | YES         | YES         | NO          | NO            | YES         |
| Lifetime track   | NO          | YES         | NO          | OPTIONAL      | N/A         |
| Non-copyable     | YES         | YES         | YES         | YES           | YES         |
| Movable          | YES         | YES         | YES         | YES           | YES         |
| Custom deleter   | YES         | YES         | NO          | NO            | NO          |
| Save/Load        | YES         | YES         | YES         | YES           | YES         |
| Auto invalidate  | NO          | YES         | NO          | OPTIONAL      | N/A         |
| Overhead         | Minimal     | Moderate    | Minimal     | Minimal       | None        |
+------------------+-------------+-------------+-------------+---------------+-------------+

USE CASES:

OwningPtr:
  ? Shields, effects, particles you create
  ? Objects with specific cleanup needs
  ? Clear ownership semantics
  ? When object can be destroyed externally

LinkedPtr:
  ? Owned objects that might be destroyed externally
  ? Need automatic invalidation on external destruction
  ? Complex lifetime scenarios
  ? When you need maximum performance (has overhead)

ObservingPtr:
  ? Game objects you don't own
  ? Prevent accidental pointer copying
  ? Temporary references
  ? When you need ownership

MoveOnlyPtr:
  ? Non-owned objects with optional safety checks
  ? Prevent copying but allow moving
  ? Can enable lifetime tracking
  ? When you need ownership

ValueWrapper:
  ? Wrapping expensive-to-copy structs
  ? Preventing accidental value copies
  ? Game data structures
  ? For pointers (use other types)
*/

// ============================================================================
// DECISION TREE: Choose the right pointer type
// ============================================================================

/*
START: What kind of object do you have?

1. Do you CREATE this object (GameCreate, new, etc.)?
   YES ? Go to 2
   NO  ? Go to 5

2. Do you OWN this object (responsible for destruction)?
   YES ? Go to 3
   NO  ? Use ObservingPtr or MoveOnlyPtr

3. Can it be destroyed EXTERNALLY (by game or other systems)?
   YES ? Use LinkedPtr (safer, auto-invalidates)
   NO  ? Go to 4

4. Does it need SPECIAL CLEANUP (not just UnInit)?
   YES ? Use OwningPtr with custom Deleter
   NO  ? Use OwningPtr (or UninitPtr alias)

5. Is it a POINTER or a VALUE?
   POINTER ? Go to 6
   VALUE   ? Use ValueWrapper (prevents copying)

6. Do you want LIFETIME VALIDATION?
   YES ? Use MoveOnlyPtr with USE_LIFETIME_TRACKING
   NO  ? Use ObservingPtr

COMMON PATTERNS:
- Shield/Effect you create  ? OwningPtr or LinkedPtr
- Bullet with mark-death    ? MarkPtr (OwningPtr with MarkForDeathDeleter)
- Unit reference (not owned) ? ObservingPtr
- Temporary pointer         ? ObservingPtr
- Expensive struct          ? ValueWrapper
*/

#pragma once

#include <type_traits>
#include <utility>
#include <vector>
#include <algorithm>

std::vector<Handles*> Handles::Array;