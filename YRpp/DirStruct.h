#pragma once

#include <Helpers\EnumFlags.h>
#include <TranslateFixedPoints.h>
#include <bit>
#include <Fixed.h>
#include <YRMath.h>
#include <CoordStruct.h>

enum class FacingType : char
{
	North = 0,
	NorthEast = 1,
	East = 2,
	SouthEast = 3,
	South = 4,
	SouthWest = 5,
	West = 6,
	NorthWest = 7,

	limit = 7,
	Count = 8,
	Min = 0,
	Max = 8,
	None = -1,
};


enum class DirType32 : unsigned char
{
	Min = 0,
	Max = 32,
};

enum class DirType : unsigned char
{
	North = 0,
	NorthEast = 32,
	East = 64,
	SouthEast = 96,
	South = 128,
	SouthWest = 160,
	West = 192,
	NorthWest = 224,

	Min = 0,
	Max = 255,

	HarvesterUnloadingA = 152,      // Direction of harvester while unloading.
	HarvesterUnloadingB = 144,      // Direction of harvester while unloading.
};
MAKE_ENUM_FLAGS(DirType);

// ═══════════════════════════════════════════════════════════════════════════
// COORDINATE SYSTEM PRIMER (read this first!)
// ═══════════════════════════════════════════════════════════════════════════
//
//  The game uses a top-down isometric view. The raw X/Y axes are:
//
//       -Y (North)
//        ↑
//        │
// -X ────┼──── +X (East)
//  (West)│
//        ↓
//       +Y (South)
//
//  IMPORTANT: +Y goes DOWN (southward), opposite to math textbooks.
//  This is why sin() calls often appear with a minus sign.
//
// ═══════════════════════════════════════════════════════════════════════════
// BINARY ANGLE MEASUREMENT (BAM) — what DirStruct::Raw actually stores
// ═══════════════════════════════════════════════════════════════════════════
//
//  Instead of degrees (0..360) or radians (0..2π), the game stores angles
//  as a 16-bit unsigned integer that wraps perfectly around a circle:
//
//      Raw = 0x0000  → North  (up,    -Y direction)
//      Raw = 0x4000  → East   (right, +X direction)
//      Raw = 0x8000  → South  (down,  +Y direction)
//      Raw = 0xC000  → West   (left,  -X direction)
//
//  Wrapping is FREE with unsigned 16-bit math:
//      0x0000 - 1 = 0xFFFF  (just before North, i.e. almost North)
//  No if/else needed to handle "going past 360°".
//
//  The 90° offset (FacingCount/4 = 16384 = 0x4000) exists because:
//      - Standard math atan2() returns 0 for East  (+X direction)
//      - Game BAM uses    0x0000 for North (-Y direction)
//  So North = East rotated 90° counter-clockwise in math space.
//  Every conversion between radians and BAM must account for this shift.
// ═══════════════════════════════════════════════════════════════════════════
// C++ → ASM PATTERN LOOKUP TABLE
// "I wrote this C++, what will IDA show me?"
// ═══════════════════════════════════════════════════════════════════════════
//
// ── GetFacing<8>() — 8 directions ───────────────────────────────────────────
//
//   C++:   dir.GetFacing<8>()
//   IDA:   (((*FacingClass::Current(...) >> 12) + 1) >> 1) & 7
//
//   Shift math:  16 - 3 = 13 bits to reach 8 steps, but uses 12 to overshoot
//                then rounded: (>>12, +1, >>1) & 7
//   Constant:    0x7 = 0b00000111 = 3-bit mask
//
// ── GetFacing<32>() — 32 directions ─────────────────────────────────────────
//
//   C++:   dir.GetFacing<32>()
//   IDA:   (((*FacingClass::Current(...) >> 10) + 1) >> 1) & 0x1F
//
//   Shift math:  16 - 5 = 11 bits, overshoots to 10 for rounding
//   Constant:    0x1F = 0b00011111 = 5-bit mask
//
// ── GetRadian<65536>() — full precision radians for trig ────────────────────
//
//   C++:   dir.GetRadian<65536>()
//   IDA:   movsx eax, word[Raw]
//          sub   eax, 3FFFh          ← always this exact constant
//          fild  [eax]
//          fmul  DIRECTION_FIXED_MAGIC  (-2π/65536 ≈ -0.00009587)
//
//   Used for: cos/sin in movement code, Get_Next_Position
//
// ── GetRadian<16>() — 16-step radians (less common) ─────────────────────────
//
//   C++:   dir.GetRadian<16>()
//   IDA:   (value - 4) * (-2π/16)   where 4 = FacingCount/4
//          fmul  0.3926990816...     (π/8, one 16th-circle step in radians)
//
// // ── GetFacing<64>() — 64 directions ─────────────────────────────────────────
//
//   C++:   dir.GetFacing<64>()
//   IDA:   (((*FacingClass::Current(...) >> 9) + 1) >> 1) & 0x3F
//
//   Shift math:  16 - 6 = 10 bits exact, overshoot to 9 for rounding
//   Constant:    0x3F = 0b00111111 = 6-bit mask
//   Draw angle:  (index - 16) * (-π/32)   where -π/32 ≈ -0.09817477...
//                16 = FacingCount/4 bias (64/4)
//
// ── GetFacing<128>() — 128 directions ───────────────────────────────────────
//
//   C++:   dir.GetFacing<128>()
//   IDA:   (((*FacingClass::Current(...) >> 8) + 1) >> 1) & 0x7F
//
//   Shift math:  16 - 7 = 9 bits exact, overshoot to 8 for rounding
//   Constant:    0x7F = 0b01111111 = 7-bit mask
//   Draw angle:  (index - 32) * (-π/64)   where -π/64 ≈ -0.04908738...
//                32 = FacingCount/4 bias (128/4)
//
// ── SetRadian / DirStruct(x1,y1,x2,y2) — write facing from atan2 ────────────
//
//   C++:   DirStruct(center.X, center.Y, dest.X, dest.Y)
//   IDA:   call  FastMath__Atan2
//          fsub  DEG90_AS_RAD        ← always fsub π/2 right after atan2
//          fmul  BINARY_ANGLE_MAGIC  (65536 / -2π ≈ -10430.37)
//          call  __ftol
//          mov   word [Raw], ax
//
// ── Set_Desired() ────────────────────────────────────────────────────────────
//
//   C++:   facing.Set_Desired(dir)
//   IDA:   mov  word [FacingClass+0], ax   ; DesiredFacing only
//          mov  [FacingClass+4], <current> ; StartFacing = snapshot
//          ; then timer Start() call
//
// ── Set_Current() — instant snap ────────────────────────────────────────────
//
//   C++:   facing.Set_Current(dir)
//   IDA:   mov  word [FacingClass+0], ax   ; DesiredFacing
//          mov  word [FacingClass+4], ax   ; StartFacing = SAME value ← key difference
//          mov  dword [RotationTimer], 0
//
// ── Raw ± 0x3FFF — angular offset by ~90° ───────────────────────────────────
//
//   C++:   DirStruct(Raw + 0x3FFF)   or   DirStruct(Raw - 0x3FFF)
//   IDA:
//     if ( (v4 & 1) != 0 )
//         LOWORD(v19) = FacingClass::Current(...)->un.Facing - 0x3FFF;
//     else
//         LOWORD(v19) = FacingClass::Current(...)->un.Facing + 0x3FFF;
//
//   MEANING: rotate the current facing by ±90° (quarter turn)
//   WHY 0x3FFF not 0x4000:
//     0x4000 would be exact 90°, 0x3FFF = 16383 is one unit short
//     This is compiler/rounding artifact — treat both as "90° turn"
//   Common use: finding perpendicular direction (e.g. strafing, side offset)
//     +0x3FFF = turn 90° counter-clockwise (left)
//     -0x3FFF = turn 90° clockwise (right)
//
// ── MASK QUICK REFERENCE ────────────────────────────────────────────────────
//
//   Mask    Bits  Facings  First shift   Bias(/4)  Per-step rad   C++ call
//   ─────────────────────────────────────────────────────────────────────────
//   & 0x07    3      8       >> 12          2       -π/4           GetFacing<8>()
//   & 0x0F    4     16       >> 11          4       -π/8           GetFacing<16>()
//   & 0x1F    5     32       >> 10          8       -π/16          GetFacing<32>()
//   & 0x3F    6     64       >>  9         16       -π/32          GetFacing<64>()
//   & 0x7F    7    128       >>  8         32       -π/64          GetFacing<128>()
//   & 0xFF    8    256       >>  7         64       -π/128         GetFacing<256>()
//   (none)   16  65536    sub 3FFFh     16384    GetRadian<65536>() → trig use
//
// ── CONSTANT QUICK REFERENCE ────────────────────────────────────────────────
//
//   IDA constant              Value              Meaning
//   ──────────────────────────────────────────────────────────────────────
//   DEG90_AS_RAD              1.5707963...       π/2, used after atan2
//   BINARY_ANGLE_MAGIC       -10430.37...        65536/(-2π), radian→BAM
//   DIRECTION_FIXED_MAGIC    -0.00009587...      (-2π)/65536, BAM→radian
//   0x3FFF or 0x4000          16383 / 16384      ≈ 90° in BAM (bias offset)
//   -0.19634954...            -π/16              per-step for 32-facing draw
//   & 0x1F                    32-1               32-facing index mask
//   & 0x7                     8-1                8-facing index mask
// ── UNIVERSAL FORMULA (so you can derive any future entry yourself) ───────────
//
//   For GetFacing<N> where N = 2^B:
//
//     First shift  = 15 - B          (= 16 - B - 1, one less than exact)
//     Rounding     = always (+1, >>1)
//     Mask         = N - 1           (B ones in binary)
//     Bias         = N / 4           (the 90° offset, subtract from index before trig)
//     Per-step rad = -2π / N         (negative = clockwise convention)
//     Draw angle   = (index - N/4) * (-2π/N)
//
//   Example: GetFacing<16>, N=16, B=4
//     First shift = 15 - 4 = 11  ✓
//     Mask        = 15 = 0x0F    ✓
//     Bias        = 4            ✓
//     Per-step    = -2π/16 = -π/8 ≈ -0.3926990...
//
// ═══════════════════════════════════════════════════════════════════════════
// ASM PATTERN MATCHING GUIDE — "What is this assembly doing to DirStruct?"
// ═══════════════════════════════════════════════════════════════════════════
//
// Use this to identify which DirStruct operation a block of assembly maps to.
//
// ── PATTERN 1: Reading current facing as radians (GetRadian<65536>) ─────────
//
//   ASM:
//     movsx  edx, word [FacingClass+0]   ; read Raw (sign-extended to 32-bit)
//     sub    edx, 3FFFh                  ; ← THE KEY: subtract 16383 (≈ 0x4000)
//     fild   [edx]
//     fmul   DIRECTION_FIXED_MAGIC       ; × (-2π/65536)
//
//   MEANING: GetRadian<65536>() — converts BAM to math radians
//   NOTE: The `sub 3FFFh` (16383) is the FacingCount/4 bias removal.
//         If you see sub 3FFFh before a fmul, it is ALWAYS GetRadian.
//         Without the sub, it would give an angle 90° wrong.
//
//   C++ equivalent:
//     double angle = pFacing->Current().GetRadian<65536>();
//
// ── PATTERN 2: Writing a facing from atan2 (SetRadian via 4-point ctor) ─────
//
//   ASM:
//     fild   [dest.Y]
//     fisub  [center.Y]      ; y2 - y1  (note which is subtracted from which!)
//     fstp   [y_param]
//     fild   [center.X]
//     fisub  [dest.X]        ; x2 - x1
//     fstp   [x_param]
//     call   FastMath__Atan2 ; atan2(y,x)
//     fsub   DEG90_AS_RAD    ; subtract π/2  ← THE KEY
//     fmul   BINARY_ANGLE_MAGIC  ; × (65536 / -2π)  converts to BAM steps
//     call   __ftol
//     mov    word [DirStruct.Raw], ax
//     ... then call FacingClass::Set_Desired
//
//   MEANING: DirStruct(x1,y1, x2,y2) followed by Set_Desired()
//   NOTE: The `fsub DEG90_AS_RAD` after atan2 is the 90° shift compensation.
//         SetRadian adds +FacingCount/4 AFTER the division (same effect).
//         Both ways achieve: math-East(0rad) → game-East(0x4000 Raw).
//
//   C++ equivalent:
//     DirStruct dir(center.X, center.Y, dest.X, dest.Y);
//     pAir->PrimaryFacing.Set_Desired(dir);
//
//   WATCH THE ARGUMENT ORDER to DirStruct(x1,y1,x2,y2):
//     The ctor computes atan2(y2-y1, x2-x1).
//     In the asm, trace which coord is fild vs fisub to determine y2,y1,x2,x1.
//     Common game pattern:
//       y = center.Y - dest.Y  → y1=dest.Y,   y2=center.Y
//       x = dest.X  - center.X → x1=center.X, x2=dest.X
//     So: DirStruct(center.X, dest.Y, dest.X, center.Y)  ← rocket facing
//
// ── PATTERN 3: Instant facing set (Set_Current) ──────────────────────────────
//
//   ASM:
//     mov    [FacingClass.DesiredFacing], ax
//     mov    [FacingClass.StartFacing],   ax   ; BOTH written same value
//     mov    dword [RotationTimer], 0          ; timer zeroed
//
//   MEANING: Set_Current() — teleport, no smooth rotation
//   KEY: Both DesiredFacing AND StartFacing get the same value simultaneously.
//        If only DesiredFacing is written, it is Set_Desired() instead.
//
// ── PATTERN 4: Smooth rotation target (Set_Desired) ──────────────────────────
//
//   ASM:
//     ; compute new dir into ax/eax ...
//     mov    word [FacingClass.DesiredFacing], ax   ; only Desired written
//     mov    [FacingClass.StartFacing], <current>   ; start = snapshot of now
//     ; then timer started with NumSteps value
//
//   MEANING: Set_Desired() — starts smooth rotation
//   KEY: StartFacing is written with the CURRENT facing value (not the new one).
//        This is the snapshot for interpolation.
//
// ── PATTERN 5: Reading Raw directly without bias (NOT GetRadian) ─────────────
//
//   ASM:
//     mov    ax, word [FacingClass+0]   ; raw read
//     ; used directly in bit shifts, comparisons, or array indexing
//     ; NO subtract 3FFFh before trig
//
//   MEANING: GetValue<N>() or GetFacing<N>() — extracting a discrete index
//   Example uses: sprite frame selection, voxel index, drawing code
//   C++ equivalent:
//     size_t frame = pFacing->Current().GetValue<5>(); // 32-facing sprite index
//
// ── PATTERN 6: Checking if two facings are "close enough" ────────────────────
//
//   ASM:
//     movsx  eax, [DesiredFacing.Raw]
//     movsx  ecx, [CurrentFacing.Raw]
//     sub    eax, ecx                  ; signed subtraction = angular gap
//     ; then abs + compare against a threshold
//
//   MEANING: CompareToTwoDir() or manual (desired - current) gap check
//   KEY: Signed 16-bit subtraction on Raw values gives the SHORTEST angular
//        path automatically (wraps correctly). If you see signed sub on two
//        Raw values followed by abs+cmp, it's always an angular distance check.
//
// ── PATTERN 7: Discrete facing index extraction (GetFacing<32> with rounding) ─
//
//   This pattern appears constantly in draw/voxel code:
//
//   ASM / IDA pseudo:
//     ((((*FacingClass::Current(...) >> 10) + 1) >> 1) & 0x1F)
//
//   Breaking it down step by step:
//
//     Raw = 0xFFFF (16-bit, 65536 steps full circle)
//
//     Step 1:  Raw >> 10
//              Reduces from 65536 steps to 64 steps (6-bit rough index)
//              Why 10? Because 65536 / 1024 = 64, and we need 32 steps
//              so we overshoot to 64 first to allow rounding in step 2+3.
//
//     Step 2:  + 1
//              Prepares for round-to-nearest (standard "add half before shift" trick)
//              Without this, >> 1 would always truncate (round down)
//
//     Step 3:  >> 1
//              Halves the 64-step range down to 32 steps
//              Combined with step 2: this is ROUNDED division by 2
//              i.e.  (x + 1) >> 1  ==  floor((x + 1) / 2)  ==  round(x / 2)
//
//     Step 4:  & 0x1F
//              Masks to 5 bits (0..31), handles wrap-around at the 32-facing boundary
//
//   Net result: a 32-direction facing index (0..31), North=0, clockwise
//
//   C++ equivalent (NOTE: GetRadian<32>() truncates, asm rounds — minor difference):
//     size_t facing32 = pDir->GetFacing<32>();   // truncating version
//     // or for the exact rounded asm behavior:
//     size_t facing32 = ((pDir->Raw >> 10) + 1) >> 1) & 0x1F;
//
// ── PATTERN 7b: The facing index used for draw angle ────────────────────────
//
//   In Draw_Matrix / voxel rotation code you almost always see:
//
//     int idx   = (((Raw >> 10) + 1) >> 1) & 0x1F;   // 32-facing index, North=0
//     int adj   = idx - 8;                             // subtract 8 = subtract 90°
//     float rad = adj * -0.1963495408493621f;          // × (-π/16) = (-2π/32)
//     Matrix3D::Rotate_Z(matrix, rad);
//
//   The constant -0.1963495408493621 is exactly -π/16:
//     Full circle = 2π, divided by 32 facings = π/16 per step
//     Negative because game rotates clockwise, math rotates counter-clockwise
//
//   The `- 8` is the same 90° bias correction seen everywhere:
//     8 steps out of 32 = 1/4 circle = 90°
//     Converts game North=0 index into math East=0 index before trig
//
//   C++ equivalent (exact, with rounding):
//     static short GetFacing32Rounded(RocketLocomotionClass* pThis)
//     {
//         unsigned short raw = pThis->Owner->PrimaryFacing.Current().Raw;
//         return short(((raw >> 10) + 1) >> 1) & 0x1F;
//     }
//     // Then in Draw_Matrix:
//     double angle = double(GetFacing32Rounded(pThis) - 8) * (-Math::PI / 16.0);
//     matrix->RotateZ(float(angle));
//
// ── PATTERN 7c: VoxelIndexKey flags set from pitch ───────────────────────────
//
//   In the same Draw_Matrix function, after the RotateY(pitch):
//
//     if (pitch == PitchInitial * DEG90)  → *key |= 0x20   (initial pitch flag)
//     if (pitch == PitchFinal   * DEG90)  → *key |= 0x40   (final pitch flag)
//     else                                → key->Invalidate() (non-standard pitch,
//                                                              cannot use cached voxel)
//
//   The key accumulates flags:
//     bit 0..4  = facing32 index (the & 0x1F from Pattern 7)
//     bit 5     = 0x20, pitch is at PitchInitial (can cache)
//     bit 6     = 0x40, pitch is at PitchFinal   (can cache)
//     key = -1  = invalidated, must re-render this frame (arbitrary pitch angle)
//
//   C++ equivalent (matches your existing _Draw_Matrix):
//     if (key) {
//         if (pRocket->CurrentPitch == float(rocket->PitchInitial * DEG90_AS_RAD))
//             *(int*)(key) |= 0x20;
//         else if (pRocket->CurrentPitch == float(rocket->PitchFinal * DEG90_AS_RAD))
//             *(int*)(key) |= 0x40;
//         else
//             key->Invalidate();   // non-cacheable intermediate pitch
//     }
//     if (key)
//         *(int*)(key) |= GetFacing32Rounded(pThis);  // pack facing into low bits
//
// ── QUICK DECODE TABLE for common shift patterns on Raw ──────────────────────
//
//   Raw >> 13          → 3-bit index  (0..7,  8 facings,  same as Getvalue8())
//   Raw >> 11          → 5-bit index  (0..31, 32 facings, same as Getvalue32(), truncated)
//   (Raw>>10+1)>>1     → 5-bit index  (0..31, 32 facings, ROUNDED version)
//   Raw >> 8           → 8-bit index  (0..255,256 facings, same as Getvalue256())
//
//   Shift amount = 16 - TargetBits:
//     16 - 3  = 13   for 8  facings
//     16 - 5  = 11   for 32 facings
//     16 - 8  =  8   for 256 facings
//
//   If you see an unexpected shift amount, check if it's doing the
//   two-step rounding variant (>> N-1, +1, >> 1) instead of direct >> N.
//
// ── PATTERN 8: GetFacing<8> applied to a BYTE-SCALE value (not Raw) ──────────
//
//   C++:   FacingType((((((int)ret) >> 4) + 1) >> 1) & 7)
//
//   KEY DIFFERENCE from Pattern 7:
//     Normal GetFacing<8> uses >> 12 because input is 16-bit Raw (65536 steps)
//     This pattern uses  >> 4  because input is already  8-bit DirType (256 steps)
//
//   Input scale is DirType / GetDir() / GetDirFixed() range (0..255):
//     North =   0
//     East  =  64
//     South = 128
//     West  = 192
//
//   Step by step:
//     >> 4   : 256 steps → 16 steps  (overshoot, 4-bit)
//     +1,>>1 : rounds 16 → 8 steps   (same rounding trick as always)
//     & 7    : 3-bit mask, 8 facings
//
//   Shift math proof:
//     From 256 steps to 8 steps: exact shift = 8 - 3 = 5 bits
//     Overshoot by 1 for rounding: 5 - 1 = 4  ← why >> 4 not >> 12
//
//   C++ equivalent:
//     FacingType facing = FacingType((((ret >> 4) + 1) >> 1) & 7);
//     // where ret came from GetDir() or GetDirFixed(), NOT from Raw directly
//
//   HOW TO SPOT which scale the input is:
//     Input scale    Steps    Exact shift    Overshoot shift   Mask
//     ────────────────────────────────────────────────────────────────
//     Raw  (16-bit)  65536       13              12            & 7
//     DirType(8-bit)   256        5               4            & 7
//
//   If you see >> 4 before & 7, the value feeding in is ALREADY byte-scale.
//   Trace back where ret came from — it will be a GetDir(), GetDirFixed(),
//   or a value previously masked with & 0xFF or cast to unsigned char.
// ── QUICK CONSTANT REFERENCE ─────────────────────────────────────────────────
//
//   DEG90_AS_RAD        = π/2  ≈ 1.5707963...  (90 degrees in radians)
//   BINARY_ANGLE_MAGIC  = 65536 / (-2π) ≈ -10430.37...
//   DIRECTION_FIXED_MAGIC = -2π / 65536 ≈ -0.00009587...
//   0x3FFF = 16383 ≈ 0x4000 (16384) = FacingCount/4 bias
//
//   Raw values at compass points:
//     0x0000 = North   (up,    facing -Y)
//     0x4000 = East    (right, facing +X)
//     0x8000 = South   (down,  facing +Y)
//     0xC000 = West    (left,  facing -X)
//
//   The sub 3FFFh vs sub 4000h discrepancy:
//     The asm uses 0x3FFF (16383) not 0x4000 (16384).
//     This is a one-off rounding artifact from integer truncation in the
//     original compiler output. GetRadian<65536> uses FacingCount/4 = 16384.
//     The difference is sub-pixel and has no visible effect in-game.
// ── Pattern 9 (corrected — unsigned input, byte cast output) ─────────────────
//
//   IDA: (unsigned __int8)((unsigned int)((*(unsigned __int16 *)v8 >> 7) + 1) >> 1)
//
//   YES — use directly:
//     dir.GetFacing<256>()
//
// ── Pattern 10 (south-relative tunnel code) ───────────────────────────────────
//
//   IDA: (unsigned __int16)((Raw + 0x8000) >> 12) + 1) >> 1) & 7
//
//   NO direct method — closest is:
//     DirStruct(int(dir.Raw) + 0x8000).GetFacing<8>()
//
// ── IF you ever actually see (short) signed cast — NO existing method ─────────
//
//   IDA: ((((short)Raw >> 7) + 1) >> 1)   ← signed, no mask
//
//   Must go manual:
//     int signedIndex = ((int(short(dir.Raw)) >> 7) + 1) >> 1;
//
// ── Simple rule to remember ───────────────────────────────────────────────────
//
//   Unsigned input + no offset  → GetFacing<N>()        works directly
//   Unsigned input + offset     → DirStruct(Raw+offset).GetFacing<N>()
//   Signed   input              → manual only, no method exists
struct DirStruct
{
	// ── Constructors ────────────────────────────────────────────────────────

	// Copy
	COMPILETIMEEVAL DirStruct(const DirStruct& nDir) noexcept : Raw { nDir.Raw } { }

	// Default = North (Raw 0)
	COMPILETIMEEVAL explicit DirStruct() noexcept : Raw { 0 } { }

	// Raw BAM value directly (use when you already have the game's binary angle)
	COMPILETIMEEVAL explicit DirStruct(int raw) noexcept : Raw { (unsigned short)raw } { }

	// From a radian angle (standard math radians: East=0, counter-clockwise positive)
	explicit DirStruct(double rad) noexcept : Raw { 0 }
	{ SetRadian<65536>(rad); }

	// From a DirType enum (8-directional: North/NE/East/SE/South/SW/West/NW)
	COMPILETIMEEVAL explicit DirStruct(const DirType dir) noexcept : Raw { 0 }
	{ SetDir(dir); }

	// From a FacingType enum (same 8-directional, different enum type)
	// Shifts left by 13 bits to scale an 8-facing value up to 16-bit BAM space
	// Example: FacingType::East (=2) → Raw = 2 << 13 = 0x4000 → East in BAM ✓
	COMPILETIMEEVAL explicit DirStruct(const FacingType face) noexcept :
		Raw { ((unsigned short)((unsigned char)face << 13)) } { }

	// From a DirType32 enum (32-directional compass)
	// Shifts left by 11 bits to scale a 32-facing value up to 16-bit BAM space
	// Example: DirType32 value 8 (East) → 8 << 11 = 0x4000 → East in BAM ✓
	COMPILETIMEEVAL explicit DirStruct(const DirType32 face) noexcept :
		Raw { ((unsigned short)((unsigned char)face << 11)) } { }

	// From a bit-depth + DirType pair (e.g. "4-bit facing system, value N")
	COMPILETIMEEVAL explicit DirStruct(size_t bits, const DirType value) noexcept : Raw { 0 }
	{ SetDir(bits, (unsigned short)(value)); }

	// Uninitialized (garbage Raw, use only when you will immediately overwrite)
	COMPILETIMEEVAL explicit DirStruct(const noinit_t&) noexcept { }

	// From a 2D vector given as (Y-component, X-component) in MATH convention
	// Example: "pointing right (+X)" → DirStruct(0.0, 1.0) → East
	// Note the Y-before-X argument order — matches atan2(y,x) convention
	explicit DirStruct(double Y, double X) noexcept : Raw { 0 }
	{ SetRadian<65536>(Math::atan2(Y, X)); }

	// From two 2D points: "what direction do I face standing at (x1,y1) looking at (x2,y2)?"
	// Internally computes atan2(y2-y1, x2-x1) then converts to BAM
	// Example: DirStruct(0,0, 1,0) → pointing East (+X)
	// Example: DirStruct(0,0, 0,-1) → pointing North (-Y, remember Y is inverted!)
	explicit DirStruct(int x1, int y1, int x2, int y2) noexcept : Raw { 0 }
	{ SetRadian<65536>(Math::atan2((double)y2 - y1, (double)x2 - x1)); }

	// ── Arithmetic ──────────────────────────────────────────────────────────
	// All arithmetic on Raw is naturally modular (wraps at 0xFFFF→0x0000)
	// so you never need to clamp or use fmod. Just add/subtract freely.

	COMPILETIMEEVAL FORCEDINLINE DirStruct& operator/=(const short nFace) { Raw /= nFace; return *this; }
	COMPILETIMEEVAL FORCEDINLINE DirStruct& operator+=(const DirStruct& rhs) { Raw += rhs.Raw; return *this; }
	COMPILETIMEEVAL FORCEDINLINE DirStruct& operator-=(const DirStruct& rhs) { Raw -= rhs.Raw; return *this; }
	COMPILETIMEEVAL FORCEDINLINE DirStruct operator-(const DirStruct& rhs) const { return DirStruct(*this) -= rhs; }
	COMPILETIMEEVAL FORCEDINLINE bool operator==(const DirStruct& another) const { return Raw == another.Raw; }
	COMPILETIMEEVAL FORCEDINLINE bool operator!=(const DirStruct& another) const { return Raw != another.Raw; }

	DirStruct* GetDirOver(CoordStruct* coord1, CoordStruct* coord2)
	{ JMP_THIS(0x4265B0); }

	// ── Comparison helpers ──────────────────────────────────────────────────

	// Returns true if the angular distance from this to pBaseDir is <= pDirFrom
	// Used for "am I close enough to my desired facing?" checks
	bool CompareToTwoDir(DirStruct& pBaseDir, DirStruct& pDirFrom)
	{ return Math::abs(pDirFrom.Raw) >= Math::abs(this->Raw - pBaseDir.Raw); }

	// Slew this facing toward pDir2 by at most pDir3 per call (rate-limited turning)
	// If the gap to pDir2 is already smaller than pDir3, snap directly to pDir2
	void Func_5B29C0(DirStruct& pDir2, DirStruct& pDir3)
	{
		if (Math::abs(pDir3.Raw) < Math::abs(this->Raw - pDir2.Raw))
		{
			// Gap is larger than our step: move toward pDir2 by pDir3
			if ((pDir2.Raw - this->Raw) >= 0)
				this->Raw += pDir3.Raw;   // need to turn clockwise
			else
				this->Raw -= pDir3.Raw;   // need to turn counter-clockwise
		}
		else
		{
			// Gap smaller than step: snap to target
			this->Raw = pDir2.Raw;
		}
	}

	// ── Simple setters/getters ──────────────────────────────────────────────

	// Store a DirType (8-direction enum, 0..255 range) scaled to 16-bit BAM
	COMPILETIMEEVAL FORCEDINLINE void SetDir(DirType dir)
	{ Raw = ((unsigned short)((unsigned char)dir * 256)); }

	// Read back as DirType (rounds to nearest 8-direction facing)
	// The +1/2 is standard "round half-up" to avoid always rounding down
	COMPILETIMEEVAL FORCEDINLINE DirType GetDirFixed() const
	{ return (DirType)((((this->Raw / (32768 / 256)) + 1) / 2) & (int)DirType::Max); }

	// Read back as DirType (truncates, no rounding)
	COMPILETIMEEVAL FORCEDINLINE DirType GetDir() const
	{ return (DirType)(this->Raw / 256); }

	// Store a value in a given bit-width facing system (see SetValue<Bits>)
	COMPILETIMEEVAL FORCEDINLINE void SetDir(size_t bit, size_t val)
	{
		if (bit <= 16u)
			Raw = ((unsigned short)TranslateFixedPoint::Normal(bit, 16u, val));
	}

	// ── Bit-scaled value access ─────────────────────────────────────────────
	//
	//  The 16-bit Raw can represent any power-of-2 facing count:
	//    3 bits → 8  facings  (N/NE/E/SE/S/SW/W/NW)
	//    5 bits → 32 facings
	//    8 bits → 256 facings
	//   16 bits → 65536 facings (full raw resolution)
	//
	//  TranslateFixedPoint rescales between these; think of it as
	//  "zoom in or out on the circle without losing the angle meaning".

	// Extract the facing index in a system of (1<<Bits) directions
	// Example: GetValue<3>() → 8-facing index (0..7)
	template<size_t Bits>
	COMPILETIMEEVAL FORCEDINLINE size_t GetValue(size_t offset = 0) const
	{ return TranslateFixedPoint::TemplatedCompileTime<16, Bits>(this->Raw, offset); }

	COMPILETIMEEVAL FORCEDINLINE size_t Getvalue8()   const { return GetValue<3>(); } // 8-direction
	COMPILETIMEEVAL FORCEDINLINE size_t Getvalue32()  const { return GetValue<5>(); } // 32-direction
	COMPILETIMEEVAL FORCEDINLINE size_t Getvalue256() const { return GetValue<8>(); } // 256-direction

	size_t GetValue(size_t Bits = 16, size_t offset = 0) const
	{
		if (Bits <= 16u)
			return TranslateFixedPoint::Normal(16, Bits, this->Raw, offset);

		return 0;
	}

	// Store a facing index in a system of (1<<Bits) directions into Raw
	template<size_t Bits>
	COMPILETIMEEVAL FORCEDINLINE void SetValue(size_t value, size_t offset = 0)
	{ Raw = ((unsigned short)(TranslateFixedPoint::TemplatedCompileTime<Bits, 16>(value, offset))); }

	// Helper for Count-directional systems (Count must be power of 2)
	template<size_t Count>
	COMPILETIMEEVAL FORCEDINLINE size_t GetFacing(size_t offset = 0) const
	{
		static_assert(std::has_single_bit(Count));
		COMPILETIMEEVAL size_t Bits = std::bit_width(Count - 1);
		return GetValue<Bits>(offset);
	}

	template<size_t Count>
	COMPILETIMEEVAL FORCEDINLINE void SetFacing(size_t value, size_t offset = 0)
	{
		static_assert(std::has_single_bit(Count));
		COMPILETIMEEVAL size_t Bits = std::bit_width(Count - 1);
		SetValue<Bits>(value, offset);
	}

	// ── Radian conversion ───────────────────────────────────────────────────
	//
	//  THE 90° SHIFT EXPLAINED:
	//
	//  Math's atan2 world:        Game's BAM world:
	//    East  = 0 rad              North = Raw 0x0000
	//    North = +π/2 rad           East  = Raw 0x4000
	//
	//  To convert, we rotate the math angle by -90° (one quarter turn):
	//    SetRadian: math_angle → BAM:   value = dir + FacingCount/4  (add 90° offset)
	//    GetRadian: BAM → math_angle:   dir   = value - FacingCount/4 (remove 90° offset)
	//
	//  FacingCount/4 in 65536-step space = 16384 = 0x4000 = East in BAM ✓

	// Convert BAM Raw back to a radian angle in standard math convention
	// (East = 0, counter-clockwise positive, same as what atan2 returns)
	// FacingCount controls precision (use 65536 for full 16-bit resolution)
	template <size_t FacingCount = 16>
	COMPILETIMEEVAL double GetRadian() const
	{
		static_assert(std::has_single_bit(FacingCount));
		COMPILETIMEEVAL size_t Bits = std::bit_width(FacingCount - 1);

		size_t value = GetValue<Bits>();
		// Undo the +FacingCount/4 offset that was added during SetRadian
		// This converts from "game North=0" back to "math East=0"
		int dir = static_cast<int>(value) - int(FacingCount / 4);
		// Scale from facing-index to radians
		// (-GAME_TWOPI / FacingCount) = radians per facing step, negative = clockwise
		return dir * (-Math::GAME_TWOPI / FacingCount);
	}

	// Store a radian angle (standard math convention) as a BAM Raw value
	template <size_t FacingCount = 16>
	COMPILETIMEEVAL void SetRadian(double rad)
	{
		static_assert(std::has_single_bit(FacingCount));
		COMPILETIMEEVAL size_t Bits = std::bit_width(FacingCount - 1);
		COMPILETIMEEVAL size_t Max = (1 << Bits) - 1;

		// Convert radian to a facing-index
		// dividing by (-GAME_TWOPI/FacingCount) is the same as multiplying
		// by (-FacingCount / GAME_TWOPI), i.e. "how many steps is this angle?"
		int dir = static_cast<int>(rad / (-Math::GAME_TWOPI / FacingCount));
		// Add 90° offset so that math's East (0 rad) maps to game's East (0x4000)
		// and math's North (+π/2) maps to game's North (0x0000)
		size_t value = dir + FacingCount / 4;
		SetValue<Bits>(value & Max); // mask to prevent overflow
	}

	unsigned short Raw;  // The actual binary angle, 0x0000=North, clockwise positive

private:
	unsigned short Pad {};
};

static_assert(sizeof(DirStruct) == 4, "Invalid Size !");