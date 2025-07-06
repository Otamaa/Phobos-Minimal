#include <Utilities/Macro.h>
#include <HouseClass.h>
#include <Unsorted.h>
#include <Ext/Rules/Body.h>
#include "VanillaAI.Enhanced.h"

// Fix potential timer overflow in HouseClass::AI
// The vanilla code at 0x4F8440 has potential integer overflow issues
// when CurrentFrame gets very large (after ~24 days of gameplay)
/*
ASMJIT_PATCH(0x4F8478, HouseClass_AI_TimerOverflowFix1, 0x6)
{
	GET(HouseClass*, pThis, ESI);
	
	// Original code: iVar10 = DAT_00a8ed84 - *(int *)((int)param_1 + 0x2a4);
	// Fixed code: Check for overflow before subtraction
	
	int currentFrame = Unsorted::CurrentFrame;
	int timerStart = *(int*)((int)pThis + 0x2A4);
	
	// Prevent overflow by clamping the difference
	int timeDiff = 0;
	if (currentFrame >= timerStart && timerStart != -1) {
		timeDiff = currentFrame - timerStart;
		// Clamp to prevent overflow in subsequent calculations
		if (timeDiff < 0) timeDiff = 0;
	}
	
	R->EAX(timeDiff);
	return 0x4F8485;
}

ASMJIT_PATCH(0x4F84BB, HouseClass_AI_TimerOverflowFix2, 0x6)
{
	GET(HouseClass*, pThis, ESI);
	
	// Same fix for the second timer at offset 0x2B0
	int currentFrame = Unsorted::CurrentFrame;
	int timerStart = *(int*)((int)pThis + 0x2B0);
	
	int timeDiff = 0;
	if (currentFrame >= timerStart && timerStart != -1) {
		timeDiff = currentFrame - timerStart;
		if (timeDiff < 0) timeDiff = 0;
	}
	
	R->EAX(timeDiff);
	return 0x4F84C8;
}

// Fix potential division by zero in team delay calculation
ASMJIT_PATCH(0x4F8508, HouseClass_AI_TeamDelayFix, 0x5)
{
	GET(HouseClass*, pThis, ESI);
	
	// Original code has potential division by zero when accessing team delay array
	// Add bounds checking
	
	int difficulty = (int)pThis->AIDifficulty;
	if (difficulty < 0 || difficulty >= 3) {
		difficulty = 1; // Default to normal difficulty
	}
	
	int teamDelay = RulesClass::Instance->TeamDelays[difficulty];
	if (teamDelay <= 0) {
		teamDelay = 900; // Default team delay (15 seconds at 60 FPS)
	}
	
	pThis->TeamDelayTimer.Start(teamDelay);
	
	return 0x4F8520;
}

// Fix potential null pointer dereference in team creation
ASMJIT_PATCH(0x4F8A70, HouseClass_AI_TeamCreationFix, 0x7)
{
	GET(HouseClass*, pThis, ESI);
	
	// Add null checking before team creation
	if (!pThis || pThis->Defeated || !pThis->Type) {
		return 0x4F8B08; // Skip team creation
	}
	
	// Additional safety check for AI difficulty
	if ((int)pThis->AIDifficulty < 0 || (int)pThis->AIDifficulty >= 3) {
		return 0x4F8B08; // Skip if invalid difficulty
	}
	
	return 0x4F8A77;
}

// Fix array bounds checking for the decrementation loop
// The vanilla code decrements values in an array without proper bounds checking
ASMJIT_PATCH(0x4F8516, HouseClass_AI_ArrayBoundsCheck, 0x8)
{
	GET(HouseClass*, pThis, ESI);
	
	// Original code: if (((int)DAT_00a8ed84 % 100 == 0) && (iVar3 = 0, 0 < *(int *)((int)param_1 + 0x5614)))
	// Add bounds checking for the array access
	
	if ((Unsorted::CurrentFrame % 100) == 0) {
		int arrayCount = *(int*)((int)pThis + 0x5614);
		int* arrayPtr = *(int**)((int)pThis + 0x5608);
		
		// Bounds check the array
		if (arrayCount > 0 && arrayCount < 1000 && arrayPtr) { // Reasonable upper limit
			for (int i = 0; i < arrayCount; i++) {
				int* valuePtr = &arrayPtr[4 + i * 8]; // Original: *(int *)(*(int *)((int)param_1 + 0x5608) + 4 + iVar3 * 8)
				if (valuePtr && *valuePtr > 1) {
					(*valuePtr)--;
				}
			}
		}
	}
	
	return 0x4F8550; // Skip original loop
}

// Fix potential memory corruption in function call at offset 0x57e0
ASMJIT_PATCH(0x4F8560, HouseClass_AI_VTableCallFix, 0x8)
{
	GET(HouseClass*, pThis, ESI);
	
	// Original code: piVar15 = *(int **)((int)param_1 + 0x57e0);
	// Add null checking before virtual function call
	
	int** vtablePtr = (int**)((int)pThis + 0x57e0);
	if (vtablePtr && *vtablePtr) {
		int* vtable = *vtablePtr;
		if (vtable && vtable[4]) { // Check if vtable and function pointer are valid
			// Call the virtual function safely using function pointer
			typedef void(__thiscall* VTableFunc)(void*, int*);
			VTableFunc func = (VTableFunc)vtable[4]; // Function at offset 0x10 (4 * sizeof(void*))
			if (func) {
				int result = 0;
				func(vtablePtr, &result);
			}
		}
	}
	
	return 0x4F8580; // Skip original call
}

// Fix potential weight overflow in Suggested_New_Team function
// The vanilla code at 0x6F0AB0 can have integer overflow in weight calculations
ASMJIT_PATCH(0x6F0C60, Suggested_New_Team_WeightOverflowFix, 0x6)
{
	GET(int, currentWeight, EAX);
	GET(int, triggerWeight, EDX);
	
	// Prevent integer overflow in weight addition
	// Check if adding triggerWeight would cause overflow
	if (currentWeight > INT_MAX - triggerWeight) {
		// Cap at maximum safe value
		R->EAX(INT_MAX);
	} else {
		R->EAX(currentWeight + triggerWeight);
	}
	
	return 0x6F0C70;
}

// Fix potential array bounds issue in enemy house lookup
ASMJIT_PATCH(0x6F0AE8, Suggested_New_Team_EnemyHouseFix, 0x8)
{
	GET(HouseClass*, pHouse, EBX);
	
	// Add bounds checking for enemy house lookup
	if (pHouse && pHouse->EnemyHouseIndex >= 0 && pHouse->EnemyHouseIndex < HouseClass::Array->Count) {
		HouseClass* pEnemy = HouseClass::Array->Items[pHouse->EnemyHouseIndex];
		if (pEnemy && !pEnemy->Defeated) {
			// Safe to proceed with enemy house
			R->EBX(pEnemy);
		} else {
			// Invalid enemy house, clear it
			pHouse->EnemyHouseIndex = -1;
			R->EBX((DWORD)0);
		}
	} else {
		R->EBX((DWORD)0);
	}
	
	return 0x6F0AF8;
}
*/
// Enhanced AI Integration Hook
// Note: This hook is disabled because it conflicts with New Team Selector
// The Enhanced Vanilla AI is integrated into the New Team Selector instead
// See src/Ext/Team/Hook.NewTeamSelector.cpp for the integration

/*
ASMJIT_PATCH(0x4F8A63, HouseClass_AI_EnhancedTeamSelection, 0x7)
{
	GET(HouseClass*, pThis, ESI);
	
	// Update configuration flags from single UseEnhancedAI setting
	bool useEnhanced = RulesExtData::Instance()->UseEnhancedAI;
	VanillaAI::g_EnablePerformanceOptimizations = useEnhanced;
	VanillaAI::g_EnableSmartTeamBuilding = useEnhanced;
	VanillaAI::g_EnableResourceAwareness = useEnhanced;
	VanillaAI::g_EnableBattlefieldAnalysis = useEnhanced;
	
	// Try enhanced AI if enabled
	if (useEnhanced)
	{
		bool result = VanillaAI::g_EnhancedAI.UpdateHouseAI(pThis);
		if (result) {
			// Enhanced AI successfully created a team
			return 0x4F8B08;
		}
	}
	
	// Fall back to vanilla logic (with existing fixes applied)
	return 0x4F8A6A;
}
*/

// Enhanced Suggested_New_Team Hook
// Note: This hook is disabled because it's not needed when integrated with New Team Selector
// The Enhanced Vanilla AI is integrated into the New Team Selector instead

/*
ASMJIT_PATCH(0x6F0AB0, Suggested_New_Team_Enhanced, 0x8)
{
	GET_STACK(TypeList<TeamTypeClass*>*, possibleTeams, 0x4);
	GET_STACK(HouseClass*, pHouse, 0x8);
	GET_STACK(bool, alerted, 0xC);
	
	// Try enhanced version if UseEnhancedAI is enabled
	if (RulesExtData::Instance()->UseEnhancedAI) {
		TeamTypeClass* selectedTeam = VanillaAI::g_EnhancedAI.SuggestNewTeam(pHouse, alerted);
		if (selectedTeam) {
			// Clear the list and add our selection
			possibleTeams->Clear();
			possibleTeams->Add(selectedTeam);
			return 0x6F0C80; // Return success
		}
	}
	
	// Fall back to vanilla logic (with existing fixes applied)
	return 0x6F0AB8;
}
*/