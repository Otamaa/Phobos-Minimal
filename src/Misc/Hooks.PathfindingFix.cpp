#include <Utilities/Macro.h>

// Memory Safety Fix: Null Pointer Dereference at 0x42A499
// Original code sequence:
// 0042A495  mov         eax,dword ptr [esp+18h]  ; Load pointer from stack
// 0042A499  test        ebx,ebx                  ; Check if ebx is null
// 0042A49B  mov         dword ptr [edi],eax      ; Store eax to [edi]
// 0042A49D  je          0042A516                 ; Jump if ebx is null
// 0042A49F  mov         ecx,dword ptr [ebx]      ; Dereference ebx
// 0042A4A1  mov         dword ptr [edi+8],ecx    ; Store result
// 0042A4A4  mov         edx,dword ptr [ebx]      ; Dereference ebx again
// 0042A4A6  mov         eax,dword ptr [eax]      ; CRASH: Dereference eax without checking if null
//
// Problem: The code checks if ebx is null but never checks if eax is null.
// If eax contains a null pointer, the instruction at 0x42A4A6 will crash.
//
// Fix: Add null check for eax before it's dereferenced
ASMJIT_PATCH(0x42A499, Pathfinding_NullPointerFix, 6)
{
	GET(DWORD, ebxValue, EBX);
	GET(DWORD, eaxValue, EAX);
	GET(DWORD, ediValue, EDI);

	// First, execute the original instruction that was overwritten
	// mov dword ptr [edi],eax
	if (ediValue >= 0x1000 && ediValue <= 0x7FFFFFFF)
	{
		*(DWORD*)ediValue = eaxValue;
	}

	// Check if ebx is null (original check)
	if (!ebxValue)
	{
		// Jump to the original safe path at 0x42A516
		return 0x42A516;
	}

	// NEW: Check if eax is null before dereferencing it
	if (!eaxValue)
	{
		// If eax is null, jump to the safe path at 0x42A516
		// This prevents the crash at 0x42A4A6
		return 0x42A516;
	}

	// Both pointers are valid, continue with normal execution
	// The next instruction will be at 0x42A49F (mov ecx,dword ptr [ebx])
	return 0x42A49F;
}

// Hierarchical Pathfinding Fix: Premature Exit at 0x42C4E5
// Original code sequence:
// 0042C497  mov         edx,dword ptr [ecx]      ; Get number of nodes in open list
// 0042C499  test        edx,edx                  ; Check if list is empty
// 0042C49B  jne         0042C4A1                 ; If not empty, continue searching
// 0042C49D  xor         eax,eax                  ; Set eax to 0 (no path found)
// 0042C49F  jmp         0042C4C7                 ; Jump to check result
// 0042C4E3  test        eax,eax                  ; Check if path was found
// 0042C4E5  je          0042C8E2                 ; BUG: Exit function if no path
//
// Problem: When pathfinding fails on a high-level map (level 2), the function
// exits immediately instead of trying the next lower level (level 1, then level 0).
// This causes units to get stuck in infinite loops of failed path requests.
//
// Fix: Change the exit jump to continue to the next iteration of the level loop
ASMJIT_PATCH(0x42C4E5, HierarchicalPathfinding_ContinueToNextLevel, 6)
{
	GET(DWORD, eaxValue, EAX);
	
	// Check if path was found at current level
	if (eaxValue == 0) {
		// No path found at current level - continue to next iteration
		// instead of exiting the function
		// This allows trying the next lower, more detailed level
		return 0x42C8D6; // Jump to end of loop iteration (dec esi, jmp 0042C309)
	}
	
	// Path was found, continue with normal execution
	// The next instruction will be at 0x42C849 (mov edx,dword ptr [esp+20h])
	return 0x42C849;
}

// Regular Pathfinding Fix: Iteration Limit Recovery at 0x42A3E8
// Original code sequence:
// 0042A3E2  cmp         ecx,2710h              ; Compare iteration counter with 10,000
// 0042A3E8  je          0042A43E               ; BUG: Jump to hard-fail exit if limit reached
// 0042A3EA  mov         eax,dword ptr [esp+14h] ; Recovery code block starts here
// 0042A3EE  test        eax,eax
// 0042A3F0  je          0042A43E
// 0042A3F2  cmp         ecx,dword ptr [esp+70h]
// 0042A3F6  je          0042A43E
// 0042A3F8  cmp         dword ptr [eax+0Ch],2
// 0042A3FC  jl          0042A43E
// 0042A3FE  mov         ecx,dword ptr [esp+6Ch]
// 0042A402  push        ecx
// 0042A403  push        eax
// 0042A404  mov         ecx,esi
// 0042A406  call        0042AA90               ; Path smoothing/recovery function
//
// Problem: When the iteration counter reaches 10,000, the function exits immediately
// instead of attempting to recover and produce a usable (though suboptimal) path.
// This happens when the counter isn't reset between different unit pathfinding requests,
// causing units to get stuck in infinite loops and creating game stutter.
//
// Fix: Disable the premature exit jump to allow the recovery code to execute
ASMJIT_PATCH(0x42A3E8, RegularPathfinding_IterationLimitRecovery, 6)
{
	GET(DWORD, iterationCount, ECX);
	
	// Check if iteration limit is reached (10,000 = 0x2710)
	if (iterationCount >= 0x2710) {
		// Instead of exiting, continue to recovery code
		// This allows the pathfinding to attempt recovery and produce a usable path
		// The next instruction will be at 0x42A3EA (mov eax,dword ptr [esp+14h])
		return 0x42A3EA;
	}
	
	// Iteration count is within limits, continue with normal execution
	// The next instruction will be at 0x42A3EA (mov eax,dword ptr [esp+14h])
	return 0x42A3EA;
}