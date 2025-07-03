#include <Utilities/Macro.h>

// Pathfinding crash fixes using ASMJIT patches
// These patches add bounds checking and validation to prevent crashes
// in the Red Alert 2 pathfinding system without breaking game logic

// Constants for pathfinding validation
constexpr DWORD MAX_PATHFIND_NODES = 8192;  // Maximum number of pathfinding nodes
constexpr DWORD MIN_VALID_POINTER = 0x1000; // Minimum valid memory address (4KB)
constexpr DWORD MAX_VALID_POINTER = 0x7FFFFFFF; // Maximum valid memory address (2GB)

// Fix #1: Node array bounds checking at 0x42C507
// Original: mov edx,dword ptr [ecx+eax*4+4]
// Problem: eax (node ID) can exceed array bounds causing crash
ASMJIT_PATCH(0x42C507, PathfindFix_NodeArrayBounds, 6)
{
	GET(DWORD, nodeId, EAX);
	GET(DWORD*, pathfindBase, ECX);
	
	// Validate node ID is within bounds
	if (nodeId >= MAX_PATHFIND_NODES || !pathfindBase) {
		// Jump to pathfinding failure cleanup
		return 0x42C740;
	}
	
	// Bounds are valid, execute original instruction safely
	// mov edx,dword ptr [ecx+eax*4+4]
	DWORD result = pathfindBase[nodeId + 1]; // [ecx+eax*4+4] = pathfindBase[nodeId+1]
	
	// Set EDX register with the result
	R->EDX(result);
	
	return 0x42C50B; // Continue to next instruction
}

// Fix #2: Coordinate access validation at 0x42C554  
// Original: mov si,word ptr [edx+eax*4+18h]
// Problem: edx or eax corrupted causing coordinate access crash
ASMJIT_PATCH(0x42C554, PathfindFix_CoordinateAccess, 6)
{
	GET(DWORD, nodeId, EAX);
	GET(DWORD*, nodeData, EDX);
	
	// Validate node ID and pointer
	if (nodeId >= MAX_PATHFIND_NODES || !nodeData) {
		// Jump to pathfinding failure cleanup
		return 0x42C740;
	}
	
	// Bounds are valid, execute original instruction safely
	// mov si,word ptr [edx+eax*4+18h]
	WORD* coordPtr = (WORD*)((BYTE*)nodeData + nodeId * 4 + 0x18);
	WORD coordValue = *coordPtr;
	
	// Set SI register (lower 16 bits of ESI)
	R->ESI((R->ESI() & 0xFFFF0000) | coordValue);
	
	return 0x42C559; // Continue to next instruction
}

// Remove the problematic 0x42C4FE patch - it's causing register corruption
// The original lea instruction should work fine if the input is validated

// Fix #3: Neighbor pointer validation at 0x42C53E
// Original: mov ebx,dword ptr [eax]
// Problem: eax contains invalid neighbor pointer
ASMJIT_PATCH(0x42C53E, PathfindFix_NeighborPointer, 6)
{
	GET(DWORD*, neighborPtr, EAX);
	
	// Validate pointer range
	if (!neighborPtr || 
		(DWORD)neighborPtr < MIN_VALID_POINTER || 
		(DWORD)neighborPtr > MAX_VALID_POINTER) {
		// Skip to next neighbor processing
		return 0x42C726;
	}
	
	// Pointer seems valid, execute original instruction
	// mov ebx,dword ptr [eax]
	R->EBX(*neighborPtr);
	
	return 0x42C540; // Continue to next instruction
}

// Fix #4: Neighbor count validation at 0x42C50E
// Original: mov ecx,dword ptr [eax+10h]
// Problem: eax points to corrupted node structure
ASMJIT_PATCH(0x42C50E, PathfindFix_NeighborCount, 6)
{
	GET(DWORD*, nodePtr, EAX);
	
	// Validate node structure pointer with proper range checking
	if (!nodePtr || 
		(DWORD)nodePtr < MIN_VALID_POINTER || 
		(DWORD)nodePtr > MAX_VALID_POINTER ||
		IsBadReadPtr(nodePtr, 0x14)) {  // Need to read 0x14 bytes (0x10 + 4)
		// Jump to pathfinding failure cleanup
		return 0x42C740;
	}
	
	// Execute original instruction: mov ecx,dword ptr [eax+10h]
	DWORD neighborCount = *(DWORD*)((BYTE*)nodePtr + 0x10);
	
	// Validate neighbor count is reasonable (max 8 neighbors in grid)
	if (neighborCount > 8) {
		neighborCount = 0; // Set to safe value
	}
	
	R->ECX(neighborCount);
	
	// Continue with next instruction
	return 0x42C514;
} 