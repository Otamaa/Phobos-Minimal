#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <HoverLocomotionClass.h>

#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>

#include <New/Type/CursorTypeClass.h>

/*
; \Enum\CursorTypes.cpp
5BDC8C = MouseClass_UpdateCursor, 7
5BDADF = MouseClass_UpdateCursorMinimapState_UseCursor, 0
5BDDC8 = MouseClass_Update_AnimateCursor, 6
5BDE64 = MouseClass_Update_AnimateCursor2, 6
5BDB90 = MouseClass_GetCursorFirstFrame_Minimap, B
5BE974 = MouseClass_GetCursorFirstFrame, 7
5BE994 = MouseClass_GetCursorFrameCount, 7
5BDBC4 = MouseClass_GetCursorCurrentFrame, 7
5BDC1B = MouseClass_GetCursorHotSpot, 7

; \Ext\Techno\Hooks.Cursor.cpp
417E16 = AircraftClass_GetActionOnObject_Dock, 6
70055D = TechnoClass_GetActionOnObject_AttackCursor, 8
700AA8 = TechnoClass_GetActionOnCell_AttackCursor, 8
7000CD = TechnoClass_GetActionOnObject_SelfDeployCursor, 6
7400F0 = UnitClass_GetActionOnObject_SelfDeployCursor_Bunker, 6
6FFEC0 = TechnoClass_GetActionOnObject_Cursors, 5
700600 = TechnoClass_GetActionOnCell_Cursors, 5

; \Misc\Actions.cpp // SW ?
4AB35A = DisplayClass_SetAction_CustomCursor, 6
5BDDC0 = MouseClass_Update_Reset, 5
4D7524 = FootClass_ActionOnObject_Allow, 9
653CA6 = RadarClass_GetMouseAction_AllowMinimap, 5
6929FC = DisplayClass_ChooseAction_CanSell, 7
4ABFBE = DisplayClass_LeftMouseButtonUp_ExecPowerToggle, 7

51E4ED = InfantryClass_GetActionOnObject_EngineerRepairable, 6
51FA82 = InfantryClass_GetActionOnCell_EngineerRepairable, 6

51EE6B = InfantryClass_GetActionOnObject_Saboteur, 6
51EB48 = InfantryClass_GetActionOnObject_IvanGrinder, A
51E5BB = InfantryClass_GetActionOnObject_MultiEngineerA, 7
51E5E1 = InfantryClass_GetActionOnObject_MultiEngineerB, 7
51E635 = InfantryClass_GetActionOnObject_EngineerOverFriendlyBuilding, 5
51E7BF = InfantryClass_GetActionOnObject_CanCapture, 6

44725F = BuildingClass_GetActionOnObject_TargetABuilding, 5
44731C = BuildingClass_GetActionOnObject_Tunnel, 6
51ED8E = InfantryClass_GetActionOnObject_Tunnel, 6
7004AD = TechnoClass_GetActionOnObject_Saboteur, 6


417DD2 = AircraftClass_GetActionOnObject_NoManualUnload, 6
740031 = UnitClass_GetActionOnObject_NoManualUnload, 6
7008D4 = TechnoClass_GetActionOnCell_NoManualFire, 6
700536 = TechnoClass_GetActionOnObject_NoManualFire, 6
74031A = UnitClass_GetActionOnObject_NoManualEnter, 6
51E748 = InfantryClass_GetActionOnObject_NoSelfGuardArea, 8

447218 = BuildingClass_GetActionOnObject_Deactivated, 6
73FD5A = UnitClass_GetActionOnObject_Deactivated, 5
51E440 = InfantryClass_GetActionOnObject_Deactivated, 8
417CCB = AircraftClass_GetActionOnObject_Deactivated, 5

447548 = BuildingClass_GetActionOnCell_Deactivated, 6
7404B9 = UnitClass_GetActionOnCell_Deactivated, 6
51F808 = InfantryClass_GetActionOnCell_Deactivated, 6
417F83 = AircraftClass_GetActionOnCell_Deactivated, 6

51E710 = InfantryClass_GetActionOnObject_Heal, 7
73FDBD = UnitClass_GetActionOnObject_Heal, 5
51E3B0 = InfantryClass_GetActionOnObject_EMP, 7

6FFEC0 = TechnoClass_GetActionOnObject_IvanBombsA, 5
6FFF9E = TechnoClass_GetActionOnObject_IvanBombsB, 8

51E488 = InfantryClass_GetActionOnObject2, 5
*/

static int CursorIndex[size_t(Action::count)];
static bool CursorIndex_Shrouded[size_t(Action::count)];
namespace MouseCursorTypeClass
{
	ValueableIdx<CursorTypeClass*> Attack_Cursor;
	ValueableIdx<CursorTypeClass*> OutOfRange_Cursor;

	MouseCursorType ByWeapon(TechnoClass* pThis, int nWeaponIdx, bool OutOfRange)
	{
		if (auto pWeaponS = pThis->GetWeapon(nWeaponIdx)){
			if (pWeaponS->WeaponType) {
				return (MouseCursorType)(OutOfRange ? 
					CursorTypeClass::FindIndexById("Enter") : //Dummy
					CursorTypeClass::FindIndexById("NoEnter")); //Dummy
			}
		}
		return OutOfRange ? MouseCursorType::AttackOutOfRange : MouseCursorType::Attack;
	}

	void Clear()
	{
		std::memset(CursorIndex, 0, sizeof(CursorIndex));
		std::memset(CursorIndex_Shrouded, 0, sizeof(CursorIndex_Shrouded));
	}

	void NOINLINE Insert(MouseCursorType nCursorIdx, Action nAction, bool Shrouded)
	{
		CursorIndex[(int)nAction] = (int)nCursorIdx;
		CursorIndex_Shrouded[(int)nAction] = Shrouded;
	}
}

DEFINE_OVERRIDE_HOOK(0x70055D, TechnoClass_GetActionOnObject_AttackCursor, 8)
{
	GET(TechnoClass*, pThis, ESI);
	GET_STACK(int, nWeapon, STACK_OFFS(0x1C, 0x8));

	const auto nCursor = MouseCursorTypeClass::ByWeapon(pThis, nWeapon, false);
	MouseCursorTypeClass::Insert(nCursor, Action::Attack, false);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x700AA8, TechnoClass_GetActionOnCell_AttackCursor, 8)
{
	GET(TechnoClass*, pThis, ESI);
	GET(int, nWeapon, EBP);
	const auto nCursor = MouseCursorTypeClass::ByWeapon(pThis, nWeapon, false);
	MouseCursorTypeClass::Insert(nCursor, Action::Attack, false);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x6FFEC0, TechnoClass_GetActionOnObject_Cursors, 5)
{
	//GET(TechnoClass*, pThis, ECX);
	GET_STACK(ObjectClass*, pTarget, 0x4);

	MouseCursorTypeClass::Insert(MouseCursorType::Attack, Action::Move, false);
	MouseCursorTypeClass::Insert(MouseCursorType::NoMindControl, Action::NoMove, false);

	if (pTarget->GetTechnoType())
	{
		MouseCursorTypeClass::Insert(MouseCursorType::Chronosphere, Action::Repair, false);
		MouseCursorTypeClass::Insert(MouseCursorType::GeneticMutator, Action::Enter, false);
		MouseCursorTypeClass::Insert(MouseCursorType::NoForceShield, Action::NoEnter, false);
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x700600, TechnoClass_GetActionOnCell_Cursors, 5)
{
	//GET(TechnoClass*, pThis, ECX);
	MouseCursorTypeClass::Insert(MouseCursorType::Attack, Action::Move, false);
	MouseCursorTypeClass::Insert(MouseCursorType::NoMindControl, Action::NoMove, false);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x7000CD, TechnoClass_GetActionOnObject_SelfDeployCursor, 6)
{
	//GET(TechnoClass*, pThis, ESI);
	MouseCursorTypeClass::Insert(MouseCursorType::Attack, Action::AreaAttack, false);
	MouseCursorTypeClass::Insert(MouseCursorType::Attack, Action::Self_Deploy, false);
	MouseCursorTypeClass::Insert(MouseCursorType::NoEnter, Action::NoDeploy, false);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x7400F0, UnitClass_GetActionOnObject_SelfDeployCursor_Bunker, 6)
{
	GET(UnitClass*, pThis, ESI);
	if (pThis->BunkerLinkedItem)
	{
		MouseCursorTypeClass::Insert(MouseCursorType::Attack, Action::Self_Deploy, false);
		return 0x73FFE6;
	}

	return pThis->unknown_bool_6AC ? 0x7400FA : 0x740115;
}