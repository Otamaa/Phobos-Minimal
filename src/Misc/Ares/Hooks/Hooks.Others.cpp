 #include <AbstractClass.h>
#include <TechnoClass.h>
#include <TeamClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <SlaveManagerClass.h>

#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/Terrain/Body.h>
#include <Ext/InfantryType/Body.h>
#include <Ext/TeamType/Body.h>
#include <Ext/HouseType/Body.h>

#include <TerrainTypeClass.h>
#include <Locomotor/HoverLocomotionClass.h>
#include <New/Type/ArmorTypeClass.h>

#include <Misc/PhobosGlobal.h>

#include <Notifications.h>
#include <strsafe.h>

#include "Header.h"
#include <Ares_TechnoExt.h>

DEFINE_DISABLE_HOOK(0x41BE80, ObjectClass_DrawRadialIndicator_ares)
DEFINE_DISABLE_HOOK(0x6FC339, TechnoClass_GetFireError_OpenToppedGunnerTemporal_ares)

DEFINE_OVERRIDE_HOOK(0x4CA0E3, FactoryClass_AbandonProduction_Invalidate, 0x6)
{
	GET(FactoryClass*, pThis, ESI);

	if (pThis->Owner == HouseClass::CurrentPlayer() &&
		pThis->Object &&
		pThis->Object->WhatAmI() == BuildingClass::AbsID
		)
	{
		pThis->Object->RemoveSidebarObject();
	}

	return 0;
}


DEFINE_DISABLE_HOOK(0x565215, MapClass_CTOR_NoInit_Crates_ares)//, 0x6, 56522D)
DEFINE_JUMP(LJMP, 0x565215, 0x56522D);

int squared(Point2D a, Point2D b)
{
	return (int)(std::sqrt((CoordStruct { a.X , a.Y, 0 } - CoordStruct { b.X, b.Y, 0 }).MagnitudeSquared()));
}

DEFINE_HOOK(0x5F6500, AbstractClass_Distance2DSquared_1, 0)
{
	GET(AbstractClass*, pThis, ECX);
	GET_STACK(AbstractClass*, pThat, 0x4);

	int nResult = 0;
	if (pThat)
	{
		const auto nThisCoord = pThis->GetCoords();
		const auto nThatCoord = pThat->GetCoords();
		nResult = squared({ nThisCoord.X , nThisCoord.Y }, { nThatCoord.X, nThatCoord.Y });
	}

	R->EAX(nResult);
	return 0x5F655D;
}

DEFINE_OVERRIDE_HOOK(0x5F6560, AbstractClass_Distance2DSquared_2, 0)
{
	GET(AbstractClass*, pThis, ECX);
	auto const nThisCoord = pThis->GetCoords();
	GET_STACK(CoordStruct*, pThatCoord, 0x4);
	R->EAX(squared({ nThisCoord.X , nThisCoord.Y }, { pThatCoord->X,  pThatCoord->Y }));
	return 0x5F659B;
}

//DEFINE_HOOK(0x6E2290, ActionClass_PlayAnimAt, 0x6)
//{
//	GET(TActionClass*, pThis, ECX);
//	GET_STACK(HouseClass*, pOwner, 0x4);
//	//GET_STACK(TechnoClass*, pInvoker, 0x8);
//	//GET_STACK(TriggerClass*, pTrigger, 0xC);
//	//GET_STACK(CellStruct*, pCell, 0x10);
//
//	auto nCell = ScenarioClass::Instance->GetWaypointCoords(pThis->Waypoint);
//	auto nCoord = CellClass::Cell2Coord(nCell);
//	nCoord.Z = MapClass::Instance->GetZPos(&nCoord);
//	auto pCellTarget = MapClass::Instance->GetCellAt(nCell);
//	nCoord = pCellTarget->GetCoordsWithBridge();
//
//	if (AnimTypeClass* AnimType = AnimTypeClass::Array->GetItemOrDefault(pThis->Value))
//	{
//		if (AnimClass* pAnim = GameCreate<AnimClass>(AnimType, nCoord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false))
//		{
//			pAnim->IsPlaying = true;
//			AnimExt::SetAnimOwnerHouseKind(pAnim, pOwner, nullptr, false);
//		}
//	}
//
//	R->EAX(1);
//	return 0x6E2387;
//}

DEFINE_DISABLE_HOOK(0x6AD0ED, Game_AllowSinglePlay_ares)//, 0x5, 6AD16C);
DEFINE_JUMP(LJMP, 0x6AD0ED, 0x6AD16C);

DEFINE_OVERRIDE_HOOK(0x437CCC, BSurface_DrawSHPFrame1_Buffer, 0x8)
{
	REF_STACK(RectangleStruct const, bounds, STACK_OFFS(0x7C, 0x10));
	//0x89C568
	REF_STACK(unsigned char const*, pBuffer, STACK_OFFS(0x7C, 0x6C));

	auto const width = static_cast<size_t>(std::clamp<short>(
		static_cast<short>(bounds.Width), 0, std::numeric_limits<short>::max()));

	// buffer overrun is now not as forgiving as it was before
	auto& Buffer = PhobosGlobal::Instance()->ShpCompression1Buffer;
	if (Buffer.size() < width)
	{
		Buffer.insert(Buffer.end(), width - Buffer.size(), 0u);
	}

	pBuffer = Buffer.data();

	return 0x437CD4;
}

// this douchebag blows your base up when it thinks you're cheating
DEFINE_OVERRIDE_SKIP_HOOK(0x55CFDF, CopyProtection_DontBlowMeUp, 0, 55D059);
//DEFINE_JUMP(LJMP, 0x55CFDF, 0x55D059);

namespace ShakeScreenHandle
{
	void ShakeScreen(TechnoClass* pThis, int nValToCalc, int nRules)
	{
		if (pThis->IsOnMyView())
		{
			auto nFirst = GeneralUtils::GetValue(nValToCalc);
			auto nSec = nFirst - GeneralUtils::GetValue(nRules) + 4;
			GeneralUtils::CalculateShakeVal(GScreenClass::Instance->ScreenShakeX, nSec >> 1);
			GeneralUtils::CalculateShakeVal(GScreenClass::Instance->ScreenShakeY, nSec);
		}
	}
}

//handle everything ourself
DEFINE_OVERRIDE_HOOK(0x441C21, BuildingClass_Destroyed_Shake, 0x6)
{
	GET(BuildingClass* const, pBld, ESI);

	if (!pBld || !pBld->Type || !RulesClass::Instance->ShakeScreen)
		return 0x441C39;

	GET(int, BuildingCost, EAX);

	if (!BuildingCost)
		return 0x441C39;

	if (!TechnoTypeExt::ExtMap.Find(pBld->Type)->DontShake.Get())
		ShakeScreenHandle::ShakeScreen(pBld, BuildingCost, RulesClass::Instance->ShakeScreen);

	return 0x441C39; //return 0 causing crash
}

DEFINE_OVERRIDE_HOOK(0x7387D1, UnitClass_Destroyed_Shake, 0x56)
{
	GET(UnitClass* const, pUnit, ESI); //forEXT

	if (!pUnit || !pUnit->Type || !RulesClass::Instance->ShakeScreen)
		return 0x738801;

	if (!pUnit->Type->Strength)
		return 0x738801;

	if (!TechnoTypeExt::ExtMap.Find(pUnit->Type)->DontShake.Get())
		ShakeScreenHandle::ShakeScreen(pUnit, pUnit->Type->Strength, RulesClass::Instance->ShakeScreen);

	return 0x738801;
}

// replaces entire function (without the pip distortion bug)
DEFINE_OVERRIDE_HOOK(0x4748A0, INIClass_GetPipIdx, 0x7)
{
	GET(INIClass*, pINI, ECX);
	GET_STACK(const char*, pSection, 0x4);
	GET_STACK(const char*, pKey, 0x8);
	GET_STACK(int, fallback, 0xC);

	if (pINI->ReadString(pSection, pKey, Phobos::readDefval, Phobos::readBuffer))
	{
		int nbuffer;
		if (Parser<int>::TryParse(Phobos::readBuffer, &nbuffer))
		{
			R->EAX(nbuffer);
			return 0x474907;
		}
		else
		{

			// find the pip value with the name specified
			for (const auto& data : TechnoTypeClass::PipsTypeName)
			{
				if (data == Phobos::readBuffer)
				{
					//Debug::Log("[%s]%s=%s ([%d] from [%s]) \n", pSection, pKey, Phobos::readBuffer, it->Value, it->Name);
					R->EAX(data.Value);
					return 0x474907;
				}
			}
		}

		Debug::INIParseFailed(pSection, pKey, Phobos::readBuffer, "Expect valid pip");
	}

	R->EAX(fallback);
	return 0x474907;
}

// replaced entire function. error was using delete[] instead of delete.
// it potentially crashed when any of the files were present in the
// game directory.
DEFINE_OVERRIDE_HOOK(0x5F77F0, ObjectTypeClass_UnloadPipsSHP, 0x5)
{
	for (int i = 0; i < (int)TechnoTypeClass::ShapesIsAllocated.size(); ++i)
	{
		if (TechnoTypeClass::ShapesIsAllocated[i] && FileSystem::ShapesAllocated[i])
		{
			GameDelete<true, false>(FileSystem::ShapesAllocated[i]);
			TechnoTypeClass::ShapesIsAllocated[i] = false;
		}
	}

	return 0x5F78FB;
}

// naive way to fix negative indexes to be generated. proper way would be to replace
// the entire function, and the function consuming the indexes. it is not yet known
// whether the out of bounds read causes desync errors. this function appears to
// have been inlined prominently in 585F40
DEFINE_OVERRIDE_HOOK(0x56BC54, ThreatPosedEstimates_GetIndex, 0x5)
{
	GET(const CellStruct*, pCell, ECX);

	int index = -1;
	if (pCell->X >= 0 && pCell->Y >= 0 && pCell->X < 512 && pCell->Y < 512)
	{
		index = pCell->X / 4 + 130 * pCell->Y / 4 + 131;
	}

	R->EAX(index);
	return 0x56BC7D;
}

// invalid or not set edge reads array out of bounds
DEFINE_OVERRIDE_HOOK(0x4759D4, INIClass_WriteEdge, 0x7)
{
	GET(int const, index, EAX);

	if (index < 0 || index > 4)
	{
		R->EDX(GameStrings::NoneStrb());
		return 0x4759DB;
	}

	return 0;
}

// don't set the focus when selling (true selling, thus no focus set atm)
DEFINE_OVERRIDE_HOOK(0x4C6DDB, Networking_RespondToEvent_Selling, 0x8)
{
	GET(TechnoClass* const, pTechno, EDI);
	GET(AbstractClass* const, pFocus, EAX);

	if (pTechno->CurrentMission != Mission::Selling || pTechno->Focus)
	{
		pTechno->SetFocus(pFocus);
	}

	return 0x4C6DE3;
}

// #895374: skip the code that removes the crates (size 7)
DEFINE_DISABLE_HOOK(0x483BF1, CellClass_Load_Crates_ares)//, 0x7, 483BFE)
DEFINE_JUMP(LJMP, 0x483BF1, 0x483BFE);

DEFINE_OVERRIDE_HOOK(0x49F5C0, CopyProtection_IsLauncherRunning, 0x8)
{
	R->AL(1);
	return 0x49F61A;
}

DEFINE_OVERRIDE_HOOK(0x49F620, CopyProtection_NotifyLauncher, 0x5)
{
	R->AL(1);
	return 0x49F733;
}

DEFINE_OVERRIDE_HOOK(0x49F7A0, CopyProtection_CheckProtectedData, 0x8)
{
	R->AL(1);
	return 0x49F8A7;
}

DEFINE_OVERRIDE_HOOK(0x699C1C, Game_ParsePKTs_ClearFile, 0x7)
{
	LEA_STACK(CCINIClass*, pINI, 0x24);
	pINI->Clear(nullptr, nullptr);
	return 0;
}

// Guard command failure
DEFINE_OVERRIDE_HOOK(0x730DB0, GuardCommandClass_Execute, 0xA)
{
	GET(TechnoClass*, T, ESI);
	return (T->Owner != HouseClass::CurrentPlayer() || !T->IsControllable())
		? 0x730E62
		: 0x730DBE
		;
}

/* #367 - do we need to draw a link to this victim */
DEFINE_OVERRIDE_HOOK(0x472198, CaptureManagerClass_DrawLinks, 0x6)
{
	enum { Draw_Maybe = 0, Draw_Yes = 0x4721E6, Draw_No = 0x472287 };

	GET(CaptureManagerClass*, Controlled, EDI);
	//GET(TechnoClass *, Item, ECX);

	if (FootClass* F = generic_cast<FootClass*>(Controlled->Owner))
	{
		if (F->ParasiteImUsing && F->InLimbo)
		{
			return Draw_No;
		}
	}

	return Draw_Maybe;
}

DEFINE_OVERRIDE_HOOK(0x551A30, LayerClass_YSortReorder, 0x5)
{
	GET(LayerClass*, pThis, ECX);

	auto const nCount = pThis->Count;
	auto nBegin = &pThis->Items[nCount / 15 * (Unsorted::CurrentFrame % 15)];
	auto nEnd = (Unsorted::CurrentFrame % 15 >= 14) ? (&pThis->Items[nCount]) : (&nBegin[nCount / 15 + nCount / 15 / 4]);
	std::sort(nBegin, nEnd, [](const ObjectClass* A, const ObjectClass* B)
	{
		return A->GetYSort() < B->GetYSort();
	});

	return 0x551A84;
}

DEFINE_OVERRIDE_HOOK(0x5F6612, ObjectClass_UnInit_SkipInvalidation, 0x9)
{
	GET(ObjectClass*, pThis, ESI);

	if (!pThis->Limbo())
		pThis->AnnounceExpiredPointer();

	return 0x5F6625;
}

//speeds up preview drawing by insane amounts
DEFINE_OVERRIDE_HOOK(0x5FED00, OverlayTypeClass_GetRadarColor, 0x6)
{
	GET(OverlayTypeClass*, ovType, ECX);
	GET_STACK(ColorStruct*, color, 0x04);
	*color = ovType->RadarColor;
	R->EAX<ColorStruct*>(color);
	return 0x5FEDDA;
}

// only eject the parasite if the unit leaves the battlefield,
// not just when it goes out of sight.
DEFINE_OVERRIDE_HOOK(0x62A283, ParasiteClass_PointerGotInvalid_Cloak, 0x9)
{
	GET(ParasiteClass*, pThis, ESI);
	GET(void*, ptr, EAX);
	GET_STACK(bool, remove, 0x2C);

	// remove only if pointer really got removed
	return (remove && pThis->Victim == ptr) ? 0x62A28C : 0x62A484;
}

/* #746 - don't set parasite eject point to cell center, but set it to fall and explode like a bomb */
DEFINE_OVERRIDE_HOOK(0x62A2F8, ParasiteClass_PointerGotInvalid, 0x6)
{
	GET(ParasiteClass*, Parasite, ESI);
	GET(CoordStruct*, XYZ, EAX);

	auto Owner = Parasite->Owner;
	auto const pWhat = Owner->WhatAmI();

	bool allowed = false;
	if (pWhat == UnitClass::AbsID)
	{
		allowed = !Owner->GetTechnoType()->Naval;
	}
	else if (pWhat == InfantryClass::AbsID)
	{
		allowed = true;
	}

	if (allowed && Owner->GetHeight() > 200)
	{
		*XYZ = Owner->Location;
		Owner->IsFallingDown = Owner->IsABomb = true;
	}

	CoordStruct result = *XYZ;
	CellStruct result_cell = CellClass::Coord2Cell(result);

	if (result == CoordStruct::Empty || result_cell == CellStruct::Empty) {
		Debug::Log("Parasite[%x : %s] With Invalid Location ! , Removing ! \n", Parasite, Parasite->Owner->get_ID());
		TechnoExt::HandleRemove(Parasite->Owner, nullptr, false, false);
		Parasite->Victim = nullptr;
		return 0x62A47B; //pop the registers
	}

	return 0;
}

// bugfix #187: Westwood idiocy
DEFINE_HOOK(0x5F6960, ObjectClass_Getcell, 0xA)
{
	GET(ObjectClass*, pThis, ECX);
	R->EAX(MapClass::Instance->GetCellAt(pThis->Location));
	return 0x5F69B6;
}

#include <ThemeClass.h>
// skip theme log lines
DEFINE_DISABLE_HOOK(0x720C42, Theme_PlaySong_DisableStopLog_ares) // skip Theme::Stop
DEFINE_HOOK(0x720C3C,  Theme_PlaySong_DisableStopLog, 0x6) // skip Theme::PlaySong
{
	GET(ThemeClass*, pThis, ESI);
	R->ECX(pThis->Stream);
	return 0x720C4D;
}

DEFINE_DISABLE_HOOK(0x720DE8, ThemeClass_PlaySong_DisablePlaySongLog_ares)
DEFINE_HOOK(0x720DBF , ThemeClass_PlaySong_DisablePlaySongLog , 0x5)
{
	GET(ThemeClass*, pThis, ESI);
	R->AL(pThis->IsScoreRepeat);
	return 0x720DF3;
}

DEFINE_DISABLE_HOOK(0x720F37, ThemeClass_Stop_DisableStopLog_ares)
DEFINE_HOOK(0x720F2E , ThemeClass_Stop_DisableStopLog , 0x9) {
	GET(ThemeClass*, pThis, ESI);
	R->ECX(pThis->Stream);
	return 0x720F42;
}

DEFINE_DISABLE_HOOK(0x720A61, skip_Theme_AI_ares)
DEFINE_HOOK(0x720A58 , ThemeClass_AI_DisableLog,0x6)
{
	GET(ThemeClass*, pThis, ESI);
	pThis->QueuedTheme = R->EAX<int>();
	return 0x720A69;
}

DEFINE_DISABLE_HOOK(0x615BD3, Handle_Static_Messages_LoopingMovie_ares)//, 0x5, 615BE0)
DEFINE_JUMP(LJMP, 0x615BD3, 0x615BE0);
DEFINE_DISABLE_HOOK(0x78997B, sub_789960_RemoveWOLResolutionCheck_ares)//, 0x5, 789A58)
DEFINE_JUMP(LJMP, 0x78997B, 0x789A58);
DEFINE_DISABLE_HOOK(0x4BA61B, DSurface_CTOR_SkipVRAM_ares)//, 0x6, 4BA623)
DEFINE_JUMP(LJMP, 0x4BA61B, 0x4BA623);

DEFINE_OVERRIDE_HOOK(0x74A884, VoxelAnimClass_UpdateBounce_Damage, 0x6)
{
	GET(VoxelAnimClass*, pThis, EBX);

	const auto pType = pThis->Type;
	const auto nRadius = pType->DamageRadius;

	if (nRadius < 0 || !pType->Damage || !pType->Warhead)
		return 0x74A934;

	const auto nCoord = pThis->Bounce.GetCoords();
	const auto pCell = MapClass::Instance->GetCellAt(nCoord);
	const auto pInvoker = VoxelAnimExt::GetTechnoOwner(pThis);

	for (NextObject j(pCell->GetContent()); j; ++j)
	{
		const auto pObj = *j;

		if (!pObj->IsAlive || pObj->InLimbo || pObj->Health <= 0)
			continue;

		const auto nLoc = pObj->Location;
		const auto nDist = abs(nLoc.X - nCoord.X) + abs(nLoc.Y - nCoord.Y);

		if (nDist < nRadius)
		{
			pObj->ReceiveDamage(&pType->Damage, TacticalClass::AdjustForZ(nDist), pType->Warhead, pInvoker, false, false, pInvoker ? pInvoker->Owner : nullptr);
		}
	}

	return 0x74A934;
}

DEFINE_OVERRIDE_HOOK(0x545904, IsometricTileTypeClass_CreateFromINIList_MediansFix, 0x7)
{
	if (R->EAX() == -1)
	{
		// all theaters except snow have this set, so I'll assume that this was tripped by snow.
		// don't like it? put the damned tag in the INI.
		R->EAX(71);
	}
	return 0;
}

// skip the entire method, we handle it ourselves
DEFINE_DISABLE_HOOK(0x53AF40, PsyDom_Update_ares)//, 6, 53B060)
DEFINE_JUMP(LJMP, 0x53AF40, 0x53B060);

DEFINE_HOOK(0x65EA43, SendReinforcement_Opentopped, 0x6)
{
	GET(AircraftClass*, pPlane, ESI);
	GET(FootClass*, pPassenger, EDI);

	if (pPlane->Type->OpenTopped)
		pPlane->EnteredOpenTopped(pPassenger);

	pPassenger->Transporter = pPlane;

	return 0x0;
}

// TODO : remove this
// issue #1282: remap wall using its owner's colors
//DEFINE_OVERRIDE_HOOK(0x47F9A4, CellClass_DrawOverlay_WallRemap, 0x6)
//{
//	GET(CellClass*, pCell, ESI);
//
//	const int idx = pCell->WallOwnerIndex;
//
//	if (idx >= 0)
//	{
//		R->EDX(HouseClass::Array->GetItem(idx));
//		return 0x47F9AA;
//	}
//
//	return 0;
//}

// issue 1520: logging stupid shit crashes the game
DEFINE_OVERRIDE_HOOK(0x4CA437, FactoryClass_GetCRC, 0x8)
{
	GET(FactoryClass*, pThis, ECX);
	GET_STACK(DWORD, pCRC, 0xC);

	R->ESI<FactoryClass*>(pThis);
	R->EDI(pCRC);

	return 0x4CA501;
}

DEFINE_OVERRIDE_HOOK(0x47243F, CaptureManagerClass_DecideUnitFate_BuildingFate, 0x6)
{
	GET(TechnoClass*, pVictim, EBX);

	if (pVictim->WhatAmI() == BuildingClass::AbsID)
	{
		// 1. add to team and other fates don't really make sense for buildings
		// 2. BuildingClass::Mission_Hunt() implementation is to do nothing!
		pVictim->QueueMission(Mission::Guard, 0);
		return 0x472604;
	}

	return 0;
}

// PrismSupportModifier repair
DEFINE_OVERRIDE_HOOK(0x671152, RulesClass_Addition_General_PrismSupportModifier, 0x6)
{
	GET(RulesClass*, pThis, ESI);
	REF_STACK(double, param, 0x0);
	param = pThis->PrismSupportModifier / 100.0;
	return 0x67115B;
}

DEFINE_OVERRIDE_HOOK(0x6B72F9, SpawnManagerClass_Update_Buildings, 0x5)
{
	GET(SpawnManagerClass*, pThis, ESI);
	GET(SpawnNode, nNode, EAX);

	auto const pOwner = pThis->Owner;
	return (nNode.Status != SpawnNodeStatus::TakeOff
		|| !pOwner
		|| pOwner->WhatAmI() == BuildingClass::AbsID)
		? 0x6B735C : 0x6B72FE;
}

// #896027: do not announce pointers as expired to bombs
// if the pointed to object is staying in-game.
DEFINE_OVERRIDE_HOOK(0x725961, AnnounceInvalidPointer_BombCloak, 0x6)
{
	GET(bool, remove, EDI);
	return remove ? 0 : 0x72596C;
}

// causing cresh without all system intact ?
DEFINE_OVERRIDE_HOOK(0x72590E, AnnounceInvalidPointer_Particle, 0x9)
{
	GET(AbstractType, nWhat, EBX);

	if (nWhat == AbstractType::Particle)
	{
		GET(ParticleClass*, pThis, ESI);

		if (auto pSys = pThis->ParticleSystem)
		{
			pSys->Particles.Remove(pThis);
		}

		return 0x725C08;
	}

	return nWhat == AbstractType::ParticleSystem ?
		0x725917 : 0x7259DA;
}

DEFINE_OVERRIDE_HOOK(0x725A1F, AnnounceInvalidPointer_SkipBehind, 0x5)
{
	GET(AnimClass*, pAnim, ESI);
	return pAnim->Type == RulesClass::Instance->Behind ?
		0x725C08 : 0x0;
}

DEFINE_OVERRIDE_HOOK(0x6E20D8, TActionClass_DestroyAttached_Loop, 0x5)
{
	GET(int, nLoopVal, EAX);
	return nLoopVal < 4 ? 0x6E20E0 : 0x0;
}

//sub_731D90_FakeOf
DEFINE_OVERRIDE_HOOK(0x731E08, Select_By_Units_Text_FakeOf, 0x6)
{
	int nCost = 0;

	for (const auto pObj : ObjectClass::CurrentObjects())
	{
		if (const auto pTechno = generic_cast<const TechnoClass*>(pObj))
		{
			const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTechno->GetTechnoType());

			TechnoTypeClass* pType = pTypeExt->Get();
			if (pTypeExt->Fake_Of.isset())
				pType = pTypeExt->Fake_Of.Get();

			nCost += pType->GetActualCost(pTechno->Owner);
		}
	}

	R->EBX(nCost);
	return 0x731E4D;
}

DEFINE_OVERRIDE_HOOK(0x6DA665, sub_6DA5C0_GroupAs, 0xA)
{
	GET(ObjectClass*, pThis, ESI);
	R->EAX(TechnoTypeExt::GetSelectionGroupID(pThis->GetType()));
	return R->Origin() + 13;
}

// DEFINE_OVERRIDE_HOOK(0x7BB445, XSurface_20, 0x6)
// {
// 	return R->EAX<void*>() ? 0x0 : 0x7BB90C;
// }

DEFINE_OVERRIDE_HOOK(0x6FF1FB, TechnoClass_Fire_DetachedRailgun, 0x6)
{
	//GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);

	const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	//const bool IsRailgun = pWeapon->IsRailgun || pWeaponExt->IsDetachedRailgun;

	//if (IsRailgun && Is_Aircraft(pThis))
	//{
		//Debug::Log("TechnoClass_FireAt Aircraft[%s] attempting to fire Railgun !\n", pThis->get_ID());
		//return 0x6FF274;
	//}

	return pWeaponExt->IsDetachedRailgun
		? 0x6FF20F : 0x0;
}

DEFINE_OVERRIDE_HOOK(0x6FF26E, TechnoClass_Fire_DetachedRailgun2, 0x6)
{
	GET(WeaponTypeClass*, pWeapon, EBX);

	return WeaponTypeExt::ExtMap.Find(pWeapon)->IsDetachedRailgun
		? 0x6FF274 : 0x0;
}

DEFINE_OVERRIDE_HOOK(0x5FDDA4, OverlayClass_GetTiberiumType_NotReallyTiberiumLog, 0x6)
{
	GET(OverlayTypeClass*, pThis, EAX);
	Debug::Log("Overlay %s not really tiberium\n", pThis->ID);
	return 0x5FDDC1;
}

DEFINE_OVERRIDE_HOOK(0x716D98, TechnoTypeClass_Load_Palette, 0x5)
{
	GET(TechnoTypeClass*, pThis, EDI);

	pThis->Palette = nullptr;
	return pThis->PaletteFile[0] == 0 ? 0x716DAA : 0x716D9D;
}

#include <Commands/ShowTeamLeader.h>

DEFINE_HOOK(0x6D47A6, TacticalClass_Render_Techno, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	// if(auto pTargetTech = generic_cast<ObjectClass*>(pThis))
	// 		Drawing::DrawLinesTo(pTargetTech->GetRenderCoords(), pThis->Location, pThis->Owner->LaserColor);

	if (pThis->InLimbo)
		return 0x0;

	if (auto const pOwner = pThis->SlaveOwner)
	{
		if (!pOwner->IsSelected)
			return 0x0;

		Drawing::DrawLinesTo(pOwner->GetRenderCoords(), pThis->Location, pOwner->Owner->Color);
	}

	if(Phobos::Otamaa::IsAdmin) {
		if (auto const pOwner = pThis->SpawnOwner)
		{
			if (!pOwner->IsSelected)
				return 0x0;

			Drawing::DrawLinesTo(pOwner->GetRenderCoords(), pThis->Location, pOwner->Owner->Color);
		}
	}

	if (ShowTeamLeaderCommandClass::IsActivated())
	{
		if (auto const pFoot = generic_cast<FootClass*>(pThis))
		{
			if (!pFoot->BelongsToATeam())
				return 0x0;

			if (auto pTeam = pFoot->Team)
			{
				if (auto const pTeamLeader = pTeam->FetchLeader())
				{

					if (pTeamLeader != pFoot)
						Drawing::DrawLinesTo(pTeamLeader->GetRenderCoords(), pThis->Location, pTeamLeader->Owner->Color);
				}
			}
		}
	}

	return 0x0;
}

DEFINE_HOOK(0x6F5190, TechnoClass_DrawIt_Add, 0x6)
{
	GET(TechnoClass*, pThis, ECX);
	GET_STACK(Point2D*, pLocation, 0x4);
	GET_STACK(RectangleStruct*, pBound, 0x8);

	auto DrawTheStuff = [&pLocation, &pThis, &pBound](const wchar_t* pFormat)
	{
		auto nPoint = *pLocation;
		//DrawingPart
		RectangleStruct nTextDimension;
		Drawing::GetTextDimensions(&nTextDimension, pFormat, nPoint, TextPrintType::Center | TextPrintType::FullShadow | TextPrintType::Efnt, 4, 2);
		auto nIntersect = Drawing::Intersect(nTextDimension, *pBound);
		auto nColorInt = pThis->Owner->Color.ToInit();//0x63DAD0

		DSurface::Temp->Fill_Rect(nIntersect, (COLORREF)0);
		DSurface::Temp->Draw_Rect(nIntersect, (COLORREF)nColorInt);
		Point2D nRet;
		Simple_Text_Print_Wide(&nRet, pFormat, DSurface::Temp.get(), pBound, &nPoint, (COLORREF)nColorInt, (COLORREF)0, TextPrintType::Center | TextPrintType::FullShadow | TextPrintType::Efnt, true);
	};

	if (ShowTeamLeaderCommandClass::IsActivated())
	{
		if (auto const pFoot = generic_cast<FootClass*>(pThis))
		{
			if (auto pTeam = pFoot->Team)
			{
				if (auto const pTeamLeader = pTeam->FetchLeader())
				{
					if (pTeamLeader == pThis)
					{
						DrawTheStuff(L"Team Leader");
					}
				}
			}
		}
	}

	//if(pThis->IsTethered)
	//	DrawTheStuff(L"IsTethered");

	return 0x0;
}

#include <Misc/AresData.h>
#include <Ext/SWType/Body.h>
#include <Ext/Super/Body.h>
#include <Ext/Techno/Body.h>

DEFINE_OVERRIDE_HOOK(0x738749, UnitClass_Destroy_TiberiumExplosive, 6)
{
	GET(const UnitClass* const, pThis, ESI);

	if (RulesClass::Instance->TiberiumExplosive
		&& !pThis->Type->Weeder
		&& !ScenarioClass::Instance->SpecialFlags.StructEd.HarvesterImmune
		&& pThis->Tiberium.GetTotalAmount() > 0.0f)
	{
		// multiply the amounts with their powers and sum them up
		int morePower = 0;
		for (int i = 0; i < TiberiumClass::Array->Count; ++i) {
			morePower += int(pThis->Tiberium.Tiberiums[i] * TiberiumClass::Array->Items[i]->Power);
		}

		if (morePower > 0) {

			CoordStruct crd = pThis->GetCoords();
			if (auto pWH = RulesExt::Global()->Tiberium_ExplosiveWarhead) {
				MapClass::DamageArea(crd, morePower, const_cast<UnitClass*>(pThis), pWH, pWH->Tiberium, pThis->Owner);
			}

			if (auto pAnim = RulesExt::Global()->Tiberium_ExplosiveAnim)
			{
				AnimExt::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnim, crd, 0, 1, AnimFlag(0x2600), -15, false),
					pThis->Owner,
					nullptr,
					false
				);
			}
		}
	}

	return 0x7387C4;
}

#include <Ext/Tiberium/Body.h>

void SpawnVisceroid(CoordStruct& crd, ObjectTypeClass* pType, int chance, bool ignoreTibDeathToVisc)
{
	bool created = false;
	// create a small visceroid if available and the cell is free
	// dont create if `pType` is 0 strength because it is not properly listed
	if (ignoreTibDeathToVisc || ScenarioClass::Instance->TiberiumDeathToVisceroid || pType->Strength > 0)
	{
		const auto pCell = MapClass::Instance->GetCellAt(crd);

		if (!(pCell->OccupationFlags & 0x20) && ScenarioClass::Instance->Random.RandomFromMax(99) < chance && pType)
		{
			if (const auto pHouse = HouseExt::FindNeutral())
			{
				if (auto pVisc = pType->CreateObject(pHouse))
				{
					++Unsorted::ScenarioInit;
					created = pVisc->Unlimbo(crd, DirType(0));
					--Unsorted::ScenarioInit;

					if (!created)
					{
						// opposed to TS, we clean up, though
						// the mutex should make it happen.
						GameDelete(pVisc);
					}
				}
			}
		}
	}
}

// damage the techno when it is moving over a cell containing tiberium
DEFINE_OVERRIDE_HOOK(0x4D85E4, FootClass_UpdatePosition_TiberiumDamage, 9)
{
	GET(FootClass*, pThis, ESI);

	if (!pThis->IsAlive)
		return 0x0;

	int damage = 0;
	WarheadTypeClass* pWarhead = nullptr;
	int transmogrify = RulesClass::Instance->TiberiumTransmogrify;

	if (RulesExt::Global()->Tiberium_DamageEnabled && pThis->GetHeight() <= RulesClass::Instance->HoverHeight)
	{
		TechnoTypeClass* pType = pThis->GetTechnoType();
		TechnoTypeExt::ExtData* pExt = TechnoTypeExt::ExtMap.Find(pType);

		// default is: infantry can be damaged, others cannot
		const bool enabled = (pThis->WhatAmI() != InfantryClass::AbsID);

		if (!pExt->TiberiumProof.Get(enabled) && !pThis->HasAbility(AbilityType::TiberiumProof))
		{
			if (pThis->Health > 0)
			{
				if (auto pTiberium = TiberiumClass::Array->GetItemOrDefault(pThis->GetCell()->GetContainedTiberiumIndex()))
				{
					auto pTibExt = TiberiumExt::ExtMap.Find(pTiberium);

					pWarhead = pTibExt->GetWarhead();
					damage = pTibExt->GetDamage();

					transmogrify = pExt->TiberiumTransmogrify.Get(transmogrify);
				}
			}
		}
	}

	if (damage != 0 && pWarhead)
	{
		CoordStruct crd = pThis->GetCoords();

		if (pThis->ReceiveDamage(&damage, 0, pWarhead, nullptr, false, false, nullptr) == DamageState::NowDead)
		{
			SpawnVisceroid(crd, RulesClass::Instance->SmallVisceroid, transmogrify, false);
			return 0x4D8F29;
		}
	}

	return 0;
}

// this was only a leftover stub from TS. reimplemented
// using the same mechanism.
DEFINE_OVERRIDE_HOOK(0x489270, CellChainReact, 5)
{
	GET(CellStruct*, cell, ECX);

	const auto pCell = MapClass::Instance->GetCellAt(*cell);

	if (pCell->OverlayData <= 0)
		return 0x0;

	TiberiumClass* pTib = TiberiumClass::Array->GetItemOrDefault(pCell->GetContainedTiberiumIndex());

	if (!pTib)
		return 0x0;

	OverlayTypeClass* pOverlay = OverlayTypeClass::Array->GetItemOrDefault(pCell->OverlayTypeIndex);

	if (!pOverlay || !pOverlay->ChainReaction)
		return 0x0;

	CoordStruct crd = pCell->GetCoords();

	if (ScenarioClass::Instance->Random.RandomFromMax(99) <
		(RulesExt::Global()->ChainReact_Multiplier * pCell->OverlayData))
	{
		bool wasFullGrown = (pCell->OverlayData >= 11);

		unsigned char delta = pCell->OverlayData / 2;
		int damage = pTib->Power * delta;

		// remove some of the tiberium
		pCell->OverlayData -= delta;
		pCell->MarkForRedraw();

		// get the warhead
		auto pExt = TiberiumExt::ExtMap.Find(pTib);
		auto pWarhead = pExt->GetExplosionWarhead();

		// create an explosion
		if (auto pType = MapClass::SelectDamageAnimation(4 * damage, pWarhead, pCell->LandType, crd))
		{
			GameCreate<AnimClass>(pType, crd, 0, 1, 0x600, 0);
		}

		// damage the area, without affecting tiberium
		MapClass::DamageArea(crd, damage, nullptr, pWarhead, false, nullptr);

		// spawn some animation on the neighbour cells
		if (auto pType = AnimTypeClass::Find(GameStrings::Anim_INVISO()))
		{
			for (int i = 0; i < 8; ++i)
			{
				auto pNeighbour = pCell->GetNeighbourCell((FacingType)i);

				if (pNeighbour->GetContainedTiberiumIndex() != -1 && pNeighbour->OverlayData > 2)
				{
					if (ScenarioClass::Instance->Random.RandomFromMax(99) < RulesExt::Global()->ChainReact_SpreadChance)
					{
						int delay = ScenarioClass::Instance->Random.RandomRanged(RulesExt::Global()->ChainReact_MinDelay, RulesExt::Global()->ChainReact_MaxDelay);
						crd = pNeighbour->GetCoords();

						GameCreate<AnimClass>(pType, crd, delay, 1, 0x600, 0);
					}
				}
			}
		}

		if (wasFullGrown)
		{
			pTib->RegisterForGrowth(cell);
		}
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x424DD3, AnimClass_ReInit_TiberiumChainReaction_Chance, 6)
{
	GET(TiberiumClass*, pTib, EDI);

	return ScenarioClass::Instance->Random.RandomFromMax(99) < TiberiumExt::ExtMap.Find(pTib)->GetDebrisChance()
		? 0x424DF9 : 0x424E9B;
}

DEFINE_OVERRIDE_HOOK(0x424EC5, AnimClass_ReInit_TiberiumChainReaction_Damage, 6)
{
	GET(TiberiumClass*, pTib, EDI);
	auto pExt = TiberiumExt::ExtMap.Find(pTib);

	R->Stack(0x0, pExt->GetExplosionWarhead());
	R->EDX(pExt->GetExplosionDamage());

	return 0x424ECB;
}

DEFINE_OVERRIDE_HOOK(0x71C7C2, TerrainClass_Update_ForestFire, 6)
{
	GET(TerrainClass*, pThis, ESI);

	const auto& flammability = RulesClass::Instance->TreeFlammability;

	// burn spread probability this frame
	if (flammability > 0.0)
	{
		if (pThis->IsBurning && ScenarioClass::Instance->Random.RandomFromMax(99) == 0)
		{
			const auto pCell = pThis->GetCell();

			// check all neighbour cells that contain terrain objects and
			// roll the dice for each of them.
			for (int i = 0; i < 8; ++i)
			{
				if (auto pTree = pCell->GetNeighbourCell((FacingType)i)->GetTerrain(false))
				{
					if (!pTree->IsBurning && ScenarioClass::Instance->Random.RandomDouble() < flammability)
					{
						pTree->Ignite();
					}
				}
			}
		}
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x71C5D2, TerrainClass_Ignite_IsFlammable, 6)
{
	GET(TerrainClass*, pThis, EDI);

	enum { Ignite = 0x71C5F3, CantBurn = 0x71C69D };

	// prevent objects from burning that aren't flammable also
	return (pThis->Type->SpawnsTiberium || !pThis->Type->IsFlammable)
		? CantBurn : Ignite;
}

// complete rewrite
DEFINE_OVERRIDE_HOOK(0x4D98C0, FootClass_Destroyed_PlayEvent, 0xA)
{
	enum { Skip = 0x4D9918 };
	GET(FootClass*, pThis, ECX);
	//GET_STACK(ObjectClass*, pKiller, 0x4);

	const auto pType = pThis->GetTechnoType();
	const auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (pExt->SupressEVALost
		|| pType->DontScore
		|| pType->Insignificant
		|| pType->Spawned
		|| !pThis->Owner
		|| !pThis->Owner->ControlledByPlayer()
	)
	{
		return Skip;
	}

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	if (RadarEventClass::Create(RadarEventType::UnitLost, pThis->GetMapCoords()))
		VoxClass::PlayIndex(pTypeExt->EVA_UnitLost, -1, -1);

	return Skip;
}

DEFINE_OVERRIDE_HOOK(0x44D760, BuildingClass_Destroyed_UnitLost, 7)
{
	GET(BuildingClass*, pThis, ECX);
	GET_STACK(ObjectClass*, pKiller, 0x4);

	const auto pType = pThis->Type;
	auto pTechnoExt = TechnoExt::ExtMap.Find(pThis);

	if (pTechnoExt->SupressEVALost
		|| pType->DontScore
		|| pType->Insignificant
		|| !pThis->Owner
		|| !pThis->Owner->IsControlledByHuman())
	{
		return 0x44D7C9;
	}

	VoxClass::PlayIndex(TechnoTypeExt::ExtMap.Find(pType)->EVA_UnitLost);

	if (pKiller)
	{
		CoordStruct nDest = pThis->GetDestination();
		RadarEventClass::Create(CellClass::Coord2Cell(nDest));
	}

	return 0x44D7C9;
}

DEFINE_DISABLE_HOOK(0x715857, TechnoTypeClass_LoadFromINI_LimitPalettes_ares);
//DEFINE_OVERRIDE_SKIP_HOOK(0x715857, TechnoTypeClass_LoadFromINI_LimitPalettes, 5, 715876)
DEFINE_JUMP(LJMP, 0x715857, 0x715876);

DEFINE_OVERRIDE_HOOK(0x6F47A0, TechnoClass_GetBuildTime, 5)
{
	GET(TechnoClass*, pThis, ECX);

	const auto pType = pThis->GetTechnoType();
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	double nFinalSpeed = 0.01;
	const auto what = pThis->WhatAmI();
	const bool isNaval = what == UnitClass::AbsID && pType->Naval;

	if (const auto pOwner = pThis->Owner)
	{
		double nSpeed = (double)pType->GetBuildSpeed();
		const int cap = RulesExt::Global()->MultipleFactoryCap.Get(what , isNaval);
		const double nFactorySpeed = pTypeExt->BuildTime_MultipleFactory.Get(RulesClass::Instance->MultipleFactory);

		{// Owner and type mult
			const double nBuildMult = pOwner->GetBuildTimeMult(pType);
			nSpeed *= nBuildMult;

			const float nBuildTimeMult = pType->BuildTimeMultiplier;
			nSpeed *= nBuildTimeMult;
		}

		double nPowerSpeedResult = 0.00;
		{//Power

			const double nPowerPercentage = pOwner->GetPowerPercentage();
			const double nLowPowerPenalty = pTypeExt->BuildTime_LowPowerPenalty.Get(RulesClass::Instance->LowPowerPenaltyModifier);
			const double nMinLowPoweProductionSpeed = pTypeExt->BuildTime_MinLowPower.Get(RulesClass::Instance->MinLowPowerProductionSpeed);
			double nMaxLowPowerProductionSpeed = pTypeExt->BuildTime_MaxLowPower.Get(RulesClass::Instance->MaxLowPowerProductionSpeed);
			double nPowerSpeed = 1.0 - nLowPowerPenalty * (1.0 - nPowerPercentage);

			if (nMinLowPoweProductionSpeed > nPowerSpeed)
				nPowerSpeed = nMinLowPoweProductionSpeed;

			if (nPowerPercentage >= 1.0 || nPowerSpeed <= nMaxLowPowerProductionSpeed)
				nMaxLowPowerProductionSpeed = nPowerSpeed;

			if (nMaxLowPowerProductionSpeed <= 0.01)
				nMaxLowPowerProductionSpeed = 0.01;

			nPowerSpeedResult = nMaxLowPowerProductionSpeed;
		}

		nFinalSpeed = nSpeed / nPowerSpeedResult;

		{//Multiple Factory

			int divisor = pOwner->FactoryCount(what, isNaval);
			divisor = (cap > -1 && divisor > cap) ? cap : divisor;

			if (nFactorySpeed > 0.0 && (divisor-1) > 0) {
				while (divisor) {
					nFinalSpeed *= nFactorySpeed;
					--divisor;
				}
			}
		}

		nFinalSpeed *= BuildingTypeExt::GetExternalFactorySpeedBonus(pThis);
	}

	{ //Exception
		if (what == BuildingClass::AbsID && !pTypeExt->BuildTime_Speed.isset() && static_cast<BuildingTypeClass*>(pType)->Wall)
			nFinalSpeed *= RulesClass::Instance->WallBuildSpeedCoefficient;
	}

	R->EAX((int)nFinalSpeed);
	return 0x6F4955;
}

DEFINE_OVERRIDE_HOOK(0x711EE0, TechnoTypeClass_GetBuildSpeed, 6)
{
	GET(TechnoTypeClass* const, pThis, ECX);
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis);
	const auto nSeed = pTypeExt->BuildTime_Speed.Get(RulesClass::Instance->BuildSpeed);
	const auto nCost = pTypeExt->BuildTime_Cost.Get(pThis->Cost);
	R->EAX(int(nSeed * nCost / 1000.0 * 900.0));
	return 0x711EDE;
}

DEFINE_OVERRIDE_HOOK(0x6AB8BB, SelectClass_ProcessInput_BuildTime, 6)
{
	GET(BuildingTypeClass* const, pBuildingProduct, ESI);
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pBuildingProduct);

	bool IsAWall = pBuildingProduct->Wall;
	if (pBuildingProduct->Wall && pTypeExt->BuildTime_Speed.isset())
		IsAWall = false;

	R->AL(IsAWall);
	return 0x6AB8C1;
}

/* this is a wtf: it unsets target if the unit can no longer affect its current target.
 * Makes sense, except Aircraft that lose the target so crudely in the middle of the attack
 * (i.e. ivan bomb weapon) go wtfkerboom with an IE
 */
DEFINE_OVERRIDE_HOOK(0x6FA4C6, TechnoClass_Update_ZeroOutTarget, 5)
{
	GET(TechnoClass* const, pThis, ESI);
	return (pThis->WhatAmI() == AbstractType::Aircraft) ? 0x6FA4D1 : 0;
}

int NOINLINE GetSelfHealAmount(TechnoClass* pThis)
{
	auto const pType = pThis->GetTechnoType();
	auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (pType->SelfHealing || pThis->HasAbility(AbilityType::SelfHeal))
	{
		const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

		if (pExt->SelfHealing_CombatDelay.GetTimeLeft())
			return 0x0;

		const auto rate = pTypeExt->SelfHealing_Rate.Get(
		RulesClass::Instance->RepairRate);

		const auto frames = MaxImpl(int(rate * 900.0), 1);

		if (Unsorted::CurrentFrame % frames == 0)
		{
			const auto strength = pType->Strength;

			const auto percent = pTypeExt->SelfHealing_Max.Get(pThis);
			const auto maxHealth = std::clamp(int(percent * strength), 1, strength);
			const auto health = pThis->Health;

			if (health < maxHealth)
			{
				const auto amount = pTypeExt->SelfHealing_Amount.Get(pThis);
				return std::clamp(amount, 0, maxHealth - health);
			}
		}
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x70BE80, TechnoClass_ShouldSelfHealOneStep, 5)
{
	GET(TechnoClass* const, pThis, ECX);
	auto const nAmount = GetSelfHealAmount(pThis);
	R->EAX(nAmount > 0 || nAmount != 0);
	return 0x70BF46;
}

DEFINE_OVERRIDE_HOOK(0x6FA743, TechnoClass_Update_SelfHeal, 0xA)
{
	enum
	{
		ContineCheckUpdateSelfHeal = 0x6FA75A,
		SkipAnySelfHeal = 0x6FA941,
	};

	GET(TechnoClass* const, pThis, ESI);

	// prevent crashing and sinking technos from self-healing
	if (pThis->InLimbo || pThis->IsCrashing || pThis->IsSinking || pThis->align_154->Is_DriverKilled)
	{
		return SkipAnySelfHeal;
	}

	const auto nUnit = specific_cast<UnitClass*>(pThis);
	if (nUnit && nUnit->DeathFrameCounter > 0)
	{
		return SkipAnySelfHeal;
	}

	//if(!GetAresTechnoExt(pThis)) {
	//	return SkipAnySelfHeal;
	//}

	// this replaces the call to pThis->ShouldSelfHealOneStep()
	const auto nAmount = GetSelfHealAmount(pThis);
	bool wasDamaged = pThis->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow;
	if (nAmount > 0 || nAmount != 0) {
		pThis->Health += nAmount;
	}

	TechnoExt::ApplyGainedSelfHeal(pThis , wasDamaged);

	return SkipAnySelfHeal;
}

// spark particle systems created at random intervals
DEFINE_OVERRIDE_HOOK(0x6FAD49, TechnoClass_Update_SparkParticles, 8) // breaks the loop
{
	GET(TechnoClass*, pThis, ESI);
	REF_STACK(DynamicVectorClass<ParticleSystemTypeClass const*>, Systems, 0x60);

	auto pType = pThis->GetTechnoType();
	auto pExt = TechnoTypeExt::ExtMap.Find(pType);

	if(auto it = pExt->ParticleSystems_DamageSparks.GetElements(pType->DamageParticleSystems)){
		auto allowAny = pExt->ParticleSystems_DamageSparks.HasValue();

		for (auto pSystem : it) {
			if (allowAny || pSystem->BehavesLike == BehavesLike::Spark) {
				Systems.AddItem(pSystem);
			}
		}
	}

	return 0x6FADB3;
}

// customizable cloaking stages
DEFINE_OVERRIDE_HOOK(0x7036EB, TechnoClass_Uncloak_CloakingStages, 6)
{
	GET(TechnoClass*, pThis, ESI);
	R->ECX(TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())->CloakStages.Get(RulesClass::Instance->CloakingStages));
	return 0x7036F1;
}

DEFINE_OVERRIDE_HOOK(0x703A79, TechnoClass_VisualCharacter_CloakingStages, 0xA)
{
	GET(TechnoClass*, pThis, ESI);
	int stages = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())->CloakStages.Get(RulesClass::Instance->CloakingStages);
	R->EAX(int(pThis->CloakProgress.Value * 256.0 / stages));
	return 0x703A94;
}

// make damage sparks customizable, using game setting as default.
DEFINE_OVERRIDE_HOOK(0x6FACD9, TechnoClass_Update_DamageSparks, 6)
{
	GET(TechnoClass*, pThis, ESI);

	if (!pThis->SparkParticleSystem)
		return 0x6FAF01;

	GET(TechnoTypeClass*, pType, EBX);

	if (pThis->GetHealthPercentage() >= RulesClass::Instance->ConditionYellow || pThis->GetHeight() <= -10)
		return 0x6FAF01;

	return TechnoTypeExt::ExtMap.Find(pType)->DamageSparks.Get(pType->DamageSparks) ?
		0x6FAD17 : 0x6FAF01;
}

DEFINE_OVERRIDE_HOOK(0x70380A, TechnoClass_Cloak_CloakSound, 6)
{
	GET(TechnoClass*, pThis, ESI);
	auto pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	R->ECX(pExt->CloakSound.Get(RulesClass::Instance->CloakSound));
	return 0x703810;
}

DEFINE_OVERRIDE_HOOK(0x70375B, TechnoClass_Uncloak_DecloakSound, 6)
{
	GET(int, ptr, ESI);
	const TechnoClass* pThis = reinterpret_cast<TechnoClass*>(ptr - 0x9C);
	const int nDefault = RulesExt::Global()->DecloakSound.Get(RulesClass::Instance->CloakSound);
	R->ECX(TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())->DecloakSound.Get(nDefault));
	return 0x703761;
}

// linking units for type selection
DEFINE_OVERRIDE_HOOK(0x732C30, TechnoClass_IDMatches, 5)
{
	GET(TechnoClass*, pThis, ECX);
	GET(DynamicVectorClass<const char*>*, pNames, EDX);

	TechnoTypeClass* pType = pThis->GetTechnoType();
	const char* id = TechnoTypeExt::ExtMap.Find(pType)
		->GetSelectionGroupID();
	bool match = false;
	const auto what = pThis->WhatAmI();

	// find any match
	for (auto i = pNames->begin(); i < pNames->end(); ++i) {
		if (IS_SAME_STR_(*i, id)) {
			if (pThis->CanBeSelectedNow()) {
				match = true;
				break;
			}

			// buildings are exempt if they can't undeploy
			if (what == BuildingClass::AbsID && pType->UndeploysInto) {
				match = true;
				break;
			}
		}
	}

	R->EAX(match ? 1 : 0);
	return 0x732C97;
}

DEFINE_OVERRIDE_HOOK(0x6F3950, TechnoClass_GetCrewCount, 8)
{
	GET(TechnoClass*, pThis, ECX);
	auto pType = pThis->GetTechnoType();

	// previous default for crew count was -1
	int count = TechnoTypeExt::ExtMap.Find(pType)->Survivors_PilotCount.Get();
	// default to original formula
	if (count < 0)
	{
		count = pType->Crewed ? 1 : 0;
	}

	R->EAX(count);
	return 0x6F3967;
}

// Support per unit modification of Iron Curtain effect duration
DEFINE_OVERRIDE_HOOK(0x70E2B0, TechnoClass_IronCurtain, 5)
{
	GET(TechnoClass*, pThis, ECX);
	GET_STACK(int, duration, STACK_OFFS(0x0, -0x4));
	//GET_STACK(HouseClass*, source, STACK_OFFS(0x0, -0x8));
	GET_STACK(bool, force, STACK_OFFS(0x0, -0xC));

	// if it's no force shield then it's the iron curtain.
	const auto pData = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	const auto modifier = (force ? pData->ForceShield_Modifier : pData->IronCurtain_Modifier).Get();

	pThis->IronCurtainTimer.Start(int(duration * modifier));
	pThis->IronTintStage = 0;
	pThis->ProtectType = force ? ProtectTypes::ForceShield : ProtectTypes::IronCurtain;

	R->EAX(DamageState::Unaffected);
	return 0x70E2FD;
}

DEFINE_OVERRIDE_HOOK(0x7327AA, TechnoClass_PlayerOwnedAliveAndNamed_GroupAs, 8)
{
	GET(TechnoClass*, pThis, ESI);
	GET(const char*, pID, EDI);

	R->EAX<int>(TechnoTypeExt::HasSelectionGroupID(pThis->GetTechnoType(), pID));
	return 0x7327B2;
}

// #912875: respect the remove flag for invalidating SpawnManager owners
DEFINE_OVERRIDE_HOOK(0x707B19, TechnoClass_PointerGotInvalid_SpawnCloakOwner, 6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(AbstractClass*, ptr, EBP);
	REF_STACK(bool, remove, STACK_OFFS(0x20, -0x8));

	if (const auto pSM = pThis->SpawnManager)
	{
		// ignore disappearing owner
		if (remove || pSM->Owner != ptr)
		{
			pSM->UnlinkPointer(ptr);
		}
	}

	return 0x707B29;
}

DEFINE_OVERRIDE_HOOK(0x70DA95, TechnoClass_RadarTrackingUpdate_AnnounceDetected, 6)
{
	GET(TechnoClass*, pThis, ESI);
	GET_STACK(int, detect, 0x10);

	const auto pType = pThis->GetTechnoType();
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	auto PlayEva = [](const char* pEva, CDTimerClass& nTimer, double nRate)
	{
		if (!nTimer.GetTimeLeft())
		{
			nTimer.Start(GameOptionsClass::Instance->GetAnimSpeed(static_cast<int>(nRate * 900.0)));
			VoxClass::Play(pEva);
		}
	};

	if (detect && pTypeExt->SensorArray_Warn)
	{
		switch (detect)
		{
		case 1:
			PlayEva("EVA_CloakedUnitDetected", HouseExt::CloakEVASpeak, RulesExt::Global()->StealthSpeakDelay);
			break;
		case 2:
			PlayEva("EVA_SubterraneanUnitDetected", HouseExt::SubTerraneanEVASpeak, RulesExt::Global()->SubterraneanSpeakDelay);
			break;
		}

		CellStruct cell = CellClass::Coord2Cell(pThis->Location);
		RadarEventClass::Create(RadarEventType::EnemySensed, cell);
	}

	return 0x70DADC;
}

DEFINE_DISABLE_HOOK(0x6BB9DD, WinMain_LogGameClasses_ares)//, 5, 6BBE2B)
DEFINE_JUMP(LJMP, 0x6BB9DD, 0x6BBE2B);
DEFINE_DISABLE_HOOK(0x70CAD8, TechnoClass_DealParticleDamage_DontDestroyCliff_ares)//, 9, 70CB30)
DEFINE_JUMP(LJMP, 0x70CAD8, 0x70CB30);

DEFINE_OVERRIDE_HOOK(0x70CBB0, TechnoClass_DealParticleDamage_AmbientDamage, 6)
{
	GET_BASE(WeaponTypeClass*, pWeapon, 0x14);

	if (!pWeapon->AmbientDamage)
		return 0x70CC3E;

	R->EDI(pWeapon);
	R->ESI(0);
	return (!(R->EAX<int>() <= 0)) ? 0x70CBB9 : 0x70CBF7;
}

// the fuck , game calling `MapClass[]` multiple times , fixed it
DEFINE_OVERRIDE_HOOK(0x6FB5F0, TechnoClass_DeleteGap_Optimize, 6)
{
	GET(CellClass*, pCell, EAX);

	const int nDecrease = pCell->GapsCoveringThisCell - 1;
	pCell->GapsCoveringThisCell = nDecrease;

	if (!HouseClass::CurrentPlayer->SpySatActive || nDecrease > 0)
		return 0x6FB69E;

	--pCell->ShroudCounter;

	if (pCell->ShroudCounter <= 0)
		pCell->AltFlags |= (AltCellFlags::NoFog | AltCellFlags::Mapped);

	return 0x6FB69E;
}

DEFINE_OVERRIDE_HOOK(0x6FB306, TechnoClass_CreateGap_Optimize, 6)
{
	GET(CellClass*, pCell, EAX);

	int nCounter = pCell->ShroudCounter;
	int nCounter_b = nCounter;
	if (nCounter >= 0 && nCounter != 1)
	{
		nCounter_b = nCounter + 1;
		pCell->ShroudCounter = nCounter + 1;
	}
	++pCell->GapsCoveringThisCell;
	if (nCounter_b >= 1)
		pCell->AltFlags &= ((AltCellFlags)0xFFFFFFE7);

	return 0x6FB3BD;
}

// bugfix #187: Westwood idiocy
DEFINE_DISABLE_HOOK(0x531726, Game_BulkDataInit_MultipleDataInitFix1_ares) //, 5, 53173A)
DEFINE_JUMP(LJMP, 0x531726, 0x53173A);

// bugfix #187: Westwood idiocy
DEFINE_DISABLE_HOOK(0x53173F, Game_BulkDataInit_MultipleDataInitFix2_ares)//, 5, 531749)
DEFINE_JUMP(LJMP, 0x53173F, 0x531749);

//this hook taking a lot of time , i guess because of UnitTypeClass::InitOneTimeData thing
// DEFINE_OVERRIDE_HOOK(0x531726, Game_BulkDataInit_MultipleDataInitFix, 5)
// {
// 	BuildingTypeClass::InitOneTimeData();
// 	UnitTypeClass::InitOneTimeData();
// 	return 0x531749;
// }

DEFINE_OVERRIDE_HOOK(0x535DB6, SetStructureTabCommandClass_Execute_Power, 6)
{
	GET(BuildingClass*, pBuild, EAX);
	R->EAX(pBuild->FindFactory(false, true));
	return 0x535DC2;
}

DEFINE_OVERRIDE_HOOK(0x535E76, SetDefenseTabCommandClass_Execute_Power, 6)
{
	GET(BuildingClass*, pBuild, EAX);
	R->EAX(pBuild->FindFactory(false, true));
	return 0x535E82;
}

DEFINE_OVERRIDE_HOOK(0x4B93BD, ScenarioClass_GenerateDropshipLoadout_FreeAnims, 7)
{
	GET_STACK(SHPStruct*, pBackground, 0xAC);
	if (pBackground)
	{
		GameDelete<true, false>(pBackground);
		pBackground = nullptr;
	}

	LEA_STACK(SHPStruct**, pSwipeAnims, 0x290);

	for (auto i = 0; i < 4; ++i)
	{
		if (auto pAnim = pSwipeAnims[i])
		{
			GameDelete<true, false>(pAnim);
			pSwipeAnims[i] = nullptr;
		}
	}

	return 0x4B9445;
}

/* fixes to reorder the savegame */
DEFINE_OVERRIDE_HOOK(0x67D315, SaveGame_EarlySaveSides, 5)
{
	GET(LPSTREAM, pStm, ESI);
	return (Game::Save_Sides(pStm, SideClass::Array) >= 0)
		? 0
		: 0x67E0B8
		;
}

DEFINE_OVERRIDE_HOOK(0x67E09A, SaveGame_LateSkipSides, 5)
{
	GET(int, success, EAX);
	return success >= 0
		? 0x67E0C2
		: 0x67E0B8
		;
}

DEFINE_OVERRIDE_HOOK(0x67E74A, LoadGame_EarlyLoadSides, 5)
{
	GET(LPSTREAM, pStm, ESI);

	int length = 0;
	LPVOID out;
	if (pStm->Read(&length, 4, 0) < 0)
	{
		return 0x67F7A3;
	}
	for (int i = 0; i < length; ++i)
	{
		if ((Imports::OleLoadFromStream.get()(pStm, &IIDs::IUnknown, &out)) < 0)
		{
			return 0x67F7A3;
		}
	}

	return 0;
}

DEFINE_DISABLE_HOOK(0x67F281, LoadGame_LateSkipSides_ares) //, 7, 67F2BF)
DEFINE_JUMP(LJMP, 0x67F281, 0x67F2BF);

DEFINE_DISABLE_HOOK(0x5A5C6A, MapSeedClass_Generate_PlacePavedRoads_RoadEndNE_ares) //, 9, 5A5CC8)
DEFINE_JUMP(LJMP, 0x5A5C6A, 0x5A5CC8);

DEFINE_DISABLE_HOOK(0x5A5D6F, MapSeedClass_Generate_PlacePavedRoads_RoadEndSW_ares) //, 9, 5A5DB8)
DEFINE_JUMP(LJMP, 0x5A5D6F, 0x5A5DB8);

DEFINE_DISABLE_HOOK(0x5A5F6A, MapSeedClass_Generate_PlacePavedRoads_RoadEndNW_ares) //, 8, 5A5FF8)
DEFINE_JUMP(LJMP, 0x5A5F6A, 0x5A5FF8);

DEFINE_DISABLE_HOOK(0x5A6464, MapSeedClass_Generate_PlacePavedRoads_RoadEndSE_ares) //, 9, 5A64AD)
DEFINE_JUMP(LJMP, 0x5A6464, 0x5A64AD);

DEFINE_DISABLE_HOOK(0x59000E, RMG_FixPavedRoadEnd_Bridges_North) //, 5, 590087)
DEFINE_JUMP(LJMP, 0x59000E, 0x590087);

DEFINE_DISABLE_HOOK(0x5900F7, RMG_FixPavedRoadEnd_Bridges_South) //, 5, 59015E)
DEFINE_JUMP(LJMP, 0x5900F7, 0x59015E);

DEFINE_DISABLE_HOOK(0x58FCC6, RMG_FixPavedRoadEnd_Bridges_West) //, 5, 58FD2A)
DEFINE_JUMP(LJMP, 0x58FCC6, 0x58FD2A);

DEFINE_DISABLE_HOOK(0x58FBDD, RMG_FixPavedRoadEnd_Bridges_East) //, 5, 58FC55)
DEFINE_JUMP(LJMP, 0x58FBDD, 0x58FC55);

DEFINE_OVERRIDE_HOOK(0x58FA51, RMG_PlaceWEBridge, 6)
{
	LEA_STACK(const RectangleStruct* const, pRect, 0x14);

	//it's a WE bridge
	return (pRect->Width > pRect->Height)
		? 0x58FA73
		: 0;
}

DEFINE_OVERRIDE_HOOK(0x58FE7B, RMG_PlaceNSBridge, 8)
{
	LEA_STACK(const RectangleStruct* const, pRect, 0x14);

	//it's a NS bridge
	return (pRect->Height > pRect->Width)
		? 0x58FE91
		: 0;
}

// fix for ultra-fast processors overrunning the performance evaluator function
DEFINE_OVERRIDE_HOOK(0x5CB0B1, Game_QueryPerformance, 5)
{
	if (!R->EAX())
	{
		R->EAX(1);
	}

	return 0;
}

// TiberiumTransmogrify is never initialized explitly, thus do that here
DEFINE_OVERRIDE_HOOK(0x66748A, RulesClass_CTOR_TiberiumTransmogrify, 6)
{
	GET(RulesClass*, pThis, ESI);
	pThis->TiberiumTransmogrify = 0;
	return 0;
}

DEFINE_OVERRIDE_HOOK_AGAIN(0x657CF2, MapClass_MinimapChanged_Lock, 6)
DEFINE_OVERRIDE_HOOK(0x657D3D, MapClass_MinimapChanged_Lock, 6)
{
	RadarClass::RadarEvenSurface->Lock();
	RadarClass::RadarEvenSurface_B->Lock();
	return 0;
}

DEFINE_OVERRIDE_HOOK_AGAIN(0x657D35, MapClass_MinimapChanged_Unlock, 7)
DEFINE_OVERRIDE_HOOK(0x657D8A, MapClass_MinimapChanged_Unlock, 7)
{
	RadarClass::RadarEvenSurface->Unlock();
	RadarClass::RadarEvenSurface_B->Unlock();
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x65731F, RadarClass_UpdateMinimap_Lock, 6)
{
	GET(RadarClass*, pRadar, ESI);
	pRadar->unknown_121C->Lock();
	pRadar->unknown_1220->Lock();
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x65757C, RadarClass_UpdateMinimap_Unlock, 8)
{
	GET(RadarClass*, pRadar, ESI);
	pRadar->unknown_1220->Unlock();
	pRadar->unknown_121C->Unlock();

	return R->EAX() ? 0x657584 : 0x6576A5;
}

#include <WWKeyboardClass.h>

DEFINE_OVERRIDE_HOOK(0x4B769B, ScenarioClass_GenerateDropshipLoadout, 5)
{
	WWKeyboardClass::Instance->Clear();
	WWMouseClass::Instance->ShowCursor();
	return 0x4B76A0;
}

#include <Utilities/Helpers.h>
#include <Ext/ParticleType/Body.h>

std::pair<TechnoClass*, HouseClass*> GetOwnership(ParticleClass* pThis)
{
	TechnoClass* pAttacker = nullptr;
	HouseClass* pOwner = nullptr;
	BulletClass* pBullet = nullptr;

	if (auto const pSystem = pThis->ParticleSystem)
	{
		if (auto pSystemOwner = pSystem->Owner)
		{
			if (((pSystemOwner->AbstractFlags & AbstractFlags::Techno) != AbstractFlags::None))
				pAttacker = static_cast<TechnoClass*>(pSystemOwner);
			else if (pSystemOwner->WhatAmI() == BulletClass::AbsID)
			{
				pBullet = static_cast<BulletClass*>(pSystemOwner);
				pAttacker = static_cast<BulletClass*>(pSystemOwner)->Owner;
			}

		}

		if (pAttacker)
			pOwner = pAttacker->GetOwningHouse();
		else if (pBullet)
			pOwner = BulletExt::ExtMap.Find(pBullet)->Owner;
	}

	return { pAttacker , pOwner };
}

 DEFINE_OVERRIDE_HOOK(0x62C2C2, ParticleClass_Update_Gas_Damage, 6)
 {
 	GET(ParticleClass*, pParticle, EBP);
 	GET(ObjectClass*, pTarget, ESI);
 	GET(int, nDistance, ECX);

 	if (pTarget->InLimbo)
 		return 0x62C309;

 	if (auto pTechno = generic_cast<TechnoClass*>(pTarget))
 	{
 		if (pTechno->IsSinking || pTechno->IsCrashing || pTechno->TemporalTargetingMe)
 			return 0x62C309;

 		if (pTechno->WhatAmI() != BuildingClass::AbsID && TechnoExt::IsChronoDelayDamageImmune(static_cast<FootClass*>(pTechno)))
 			return 0x62C309;
 	}

 	auto const& [pAttacker, pOwner] = GetOwnership(pParticle);
 	int nDamage = pParticle->Type->Damage;
 	pTarget->ReceiveDamage(&nDamage, nDistance, pParticle->Type->Warhead, pAttacker, false, false, pOwner);

 	return 0x62C309;
 }

DEFINE_OVERRIDE_HOOK(0x62C23D, ParticleClass_Update_Gas_DamageRange, 6)
{
	GET(ParticleClass*, pThis, EBP);
	auto pTypeExt = ParticleTypeExt::ExtMap.Find(pThis->Type);

	if (pTypeExt->DamageRange.Get() <= 0.0)
		return 0x0;

	const auto pVec = Helpers::Alex::getCellSpreadItems(pThis->Location, std::ceil(pTypeExt->DamageRange.Get()));
	const auto& [pAttacker, pOwner] = GetOwnership(pThis);

	for (const auto pItem : pVec)
	{
		if (pItem->Health <= 0 || !pItem->IsAlive || pItem->InLimbo)
			continue;

		if (pItem->IsSinking || pItem->IsCrashing || pItem->TemporalTargetingMe)
			continue;

		if (pItem->WhatAmI() != BuildingClass::AbsID && TechnoExt::IsChronoDelayDamageImmune(static_cast<FootClass*>(pItem)))
			continue;

		auto nDamge = pThis->Type->Damage;
		auto nX = abs(pThis->Location.X - pItem->Location.X);
		auto nY = abs(pThis->Location.Y - pItem->Location.Y);
		int nDistance = Game::AdjustForZ(nX + nY);
		pItem->ReceiveDamage(&nDamge, nDistance, pThis->Type->Warhead, pAttacker, false, false, pOwner);
	}

	return 0x62C313;
}

DEFINE_OVERRIDE_HOOK(0x62D015, ParticleClass_Draw_Palette, 6)
{
	GET(ParticleClass*, pThis, EDI);

	ConvertClass* pConvert = FileSystem::ANIM_PAL();
	const auto pTypeExt = ParticleTypeExt::ExtMap.Find(pThis->Type);
	if (const auto pConvertData = pTypeExt->Palette)
	{
		pConvert = pConvertData->GetConvert<PaletteManager::Mode::Temperate>();
	}

	R->EDX(pConvert);
	return 0x62D01B;
}

DEFINE_OVERRIDE_HOOK(0x62CDB6, ParticleClass_Update_Fire, 7)
{
	GET(ParticleClass*, pParticle, ESI);
	GET(ObjectClass*, pTarget, EDI);
	GET(int, nDistance, ECX);
	GET(int, nDamage, EDX);

	if (pTarget->InLimbo || pTarget->Health <= 0 || !pTarget->IsAlive)
		return 0x62CE09;

	if (auto pTechno = generic_cast<TechnoClass*>(pTarget))
	{
		if (pTechno->IsSinking || pTechno->IsCrashing || pTechno->TemporalTargetingMe)
			return 0x62CE09;

		if (pTechno->WhatAmI() != BuildingClass::AbsID && TechnoExt::IsChronoDelayDamageImmune(static_cast<FootClass*>(pTechno)))
			return 0x62CE09;
	}

	auto const& [pAttacker, pOwner] = GetOwnership(pParticle);

	if (pAttacker == pTarget)
		return 0x62CE09;

	pTarget->ReceiveDamage(&nDamage, nDistance > 0 ? nDistance / 10 : nDistance, pParticle->Type->Warhead, pAttacker, false, false, pOwner);

	return 0x62CE09;
}

DEFINE_OVERRIDE_HOOK(0x62A020, ParasiteClass_Update, 0xA)
{
	GET(TechnoClass*, pOwner, ECX);
	R->EAX(pOwner->GetWeapon(pOwner->align_154->idxSlot_Parasite));
	return 0x62A02A;
}

DEFINE_OVERRIDE_HOOK(0x62A7B1, Parasite_ExitUnit, 9)
{
	GET(TechnoClass*, pOwner, ECX);
	R->EAX(pOwner->GetWeapon(pOwner->align_154->idxSlot_Parasite));
	return 0x62A7BA;
}

DEFINE_OVERRIDE_HOOK(0x48248D, CellClass_CrateBeingCollected_MoneyRandom, 6)
{
	GET(int, nCur, EAX);

	const auto nAdd = RulesExt::Global()->RandomCrateMoney;

	if (nAdd > 0)
		nCur += ScenarioClass::Instance->Random.RandomFromMax(nAdd);

	R->EDI(nCur);
	return 0x4824A7;
}

DEFINE_OVERRIDE_HOOK(0x5F3FB2, ObjectClass_Update_MaxFallRate, 6)
{
	GET(ObjectClass*, pThis, ESI);

	const auto pTechnoType = pThis->GetTechnoType();
	const bool bAnimAttached = pTechnoType ? pThis->Parachute != 0 : pThis->HasParachute;

	int nFallRate = 1;
	int nMaxFallRate = bAnimAttached ? RulesClass::Instance->ParachuteMaxFallRate : RulesClass::Instance->NoParachuteMaxFallRate;

	if (pTechnoType)
	{
		const auto pExt = TechnoTypeExt::ExtMap.Find(pTechnoType);
		nFallRate = (!bAnimAttached ? pExt->FallRate_NoParachute : pExt->FallRate_Parachute).Get();
		auto& nCustomMaxFallRate = (!bAnimAttached ? pExt->FallRate_NoParachuteMax : pExt->FallRate_ParachuteMax);

		if (nCustomMaxFallRate.isset())
			nMaxFallRate = nCustomMaxFallRate;
	}

	if (pThis->FallRate - nFallRate >= nMaxFallRate)
		nMaxFallRate = pThis->FallRate - nFallRate;

	pThis->FallRate = nMaxFallRate;
	return 0x5F3FFD;
}

DEFINE_OVERRIDE_HOOK(0x481C6C, CellClass_CrateBeingCollected_Armor1, 6)
{
	GET(TechnoClass*, Unit, EDI);
	return (Unit->align_154->AE_ArmorMult == 1.0) ? 0x481D52 : 0x481C86;
}

DEFINE_OVERRIDE_HOOK(0x481CE1, CellClass_CrateBeingCollected_Speed1, 6)
{
	GET(FootClass*, Unit, EDI);
	return (Unit->align_154->AE_SpeedMult == 1.0) ? 0x481D52 : 0x481C86;
}

DEFINE_OVERRIDE_HOOK(0x481D0E, CellClass_CrateBeingCollected_Firepower1, 6)
{
	GET(TechnoClass*, Unit, EDI);
	return (Unit->align_154->AE_FirePowerMult == 1.0) ? 0x481D52 : 0x481C86;
}

DEFINE_OVERRIDE_HOOK(0x481D3D, CellClass_CrateBeingCollected_Cloak1, 6)
{
	GET(TechnoClass*, Unit, EDI);

	if (Unit->CanICloakByDefault() || Unit->align_154->AE_Cloak)
	{
		return 0x481C86;
	}

	// cloaking forbidden for type
	return  (!TechnoTypeExt::ExtMap.Find(Unit->GetTechnoType())->CloakAllowed)
		? 0x481C86 : 0x481D52;
}

//overrides on actual crate effect applications
DEFINE_OVERRIDE_HOOK(0x48294F, CellClass_CrateBeingCollected_Cloak2, 7)
{
	GET(TechnoClass*, Unit, EDX);
	Unit->align_154->AE_Cloak = true;
	TechnoExt_ExtData::RecalculateStat(Unit);
	return 0x482956;
}

DEFINE_OVERRIDE_HOOK(0x482E57, CellClass_CrateBeingCollected_Armor2, 6)
{
	GET(TechnoClass*, Unit, ECX);
	GET_STACK(double, Pow_ArmorMultiplier, 0x20);

	if (Unit->align_154->AE_ArmorMult == 1.0)
	{
		Unit->align_154->AE_ArmorMult = Pow_ArmorMultiplier;
		TechnoExt_ExtData::RecalculateStat(Unit);
		R->AL(Unit->GetOwningHouse()->IsInPlayerControl);
		return 0x482E89;
	}
	return 0x482E92;
}

DEFINE_OVERRIDE_HOOK(0x48303A, CellClass_CrateBeingCollected_Speed2, 6)
{
	GET(FootClass*, Unit, EDI);
	GET_STACK(double, Pow_SpeedMultiplier, 0x20);

	// removed aircraft check
	// these originally not allow those to gain speed mult

	if (Unit->align_154->AE_SpeedMult == 1.0)
	{
		Unit->align_154->AE_SpeedMult = Pow_SpeedMultiplier;
		TechnoExt_ExtData::RecalculateStat(Unit);
		R->CL(Unit->GetOwningHouse()->IsInPlayerControl);
		return 0x483078;
	}
	return 0x483081;
}

DEFINE_OVERRIDE_HOOK(0x483226, CellClass_CrateBeingCollected_Firepower2, 6)
{
	GET(TechnoClass*, Unit, ECX);
	GET_STACK(double, Pow_FirepowerMultiplier, 0x20);

	if (Unit->align_154->AE_FirePowerMult == 1.0)
	{
		Unit->align_154->AE_FirePowerMult = Pow_FirepowerMultiplier;
		TechnoExt_ExtData::RecalculateStat(Unit);
		R->AL(Unit->GetOwningHouse()->IsInPlayerControl);
		return 0x483258;
	}
	return 0x483261;
}

DEFINE_OVERRIDE_HOOK(0x736135, UnitClass_Update_Deactivated, 6)
{
	GET(UnitClass*, pThis, ESI);

	// don't sparkle on EMP, Operator, ....
	return TechnoExt_ExtData::IsPowered(pThis)
		? 0x7361A9 : 0;
}

// merge two small visceroids into one large visceroid
DEFINE_OVERRIDE_HOOK(0x739F21, UnitClass_UpdatePosition_Visceroid, 6)
{
	GET(UnitClass*, pThis, EBP);

	if (!pThis->Destination
		|| pThis->Destination->WhatAmI() != UnitClass::AbsID
		|| !RulesClass::Instance->LargeVisceroid
		|| RulesClass::Instance->LargeVisceroid->Strength <= 0
		|| !TechnoExt_ExtData::IsUnitAlive(pThis))
		return 0x0;

	UnitClass* pDest = static_cast<UnitClass*>(pThis->Destination);

	// fleshbag erotic
	if (pThis->Type->SmallVisceroid && pDest->Type->SmallVisceroid && TechnoExt_ExtData::IsUnitAlive(pDest))
	{
			// nice to meat you!
		if (CellClass::Coord2Cell(pThis->GetCoords()) == CellClass::Coord2Cell(pDest->GetCoords()))
		{
				// two become one
			pDest->Type = RulesClass::Instance->LargeVisceroid;
			pDest->Health = RulesClass::Instance->LargeVisceroid->Strength;
			pDest->EstimatedHealth = pDest->Health;
			pDest->Target = nullptr;
			pDest->Destination = nullptr;
			pDest->Stun();

			pDest->IsSelected = pThis->IsSelected;

			CellClass* pCell = MapClass::Instance->GetCellAt(pDest->LastMapCoords);
			pDest->UpdateThreatInCell(pCell);
			pDest->QueueMission(Mission::Guard, true);

			Debug::Log(__FUNCTION__" Executed!\n");
			TechnoExt::HandleRemove(pThis, nullptr, false, false);
			return 0x73B0A5;
		}
	}

	return 0;
}

DEFINE_DISABLE_HOOK(0x73C143, UnitClass_DrawVXL_Deactivated_ares)
DEFINE_HOOK(0x73C141, UnitClass_DrawVXL_Deactivated,  7)
{
	GET(UnitClass*, pThis, EBP);
	REF_STACK(int, Value, 0x1E0);

	const auto pRules = RulesExt::Global();
	double factor = 1.0;

	if (pThis->IsUnderEMP())
	{
		factor = pRules->DeactivateDim_EMP;
	}
	else if (pThis->IsDeactivated())
	{
		// use the operator check because it is more
		// efficient than the powered check.
		if (TechnoExt_ExtData::IsOperatedB(pThis))
		{
			factor = pRules->DeactivateDim_Powered;
		}
		else
		{
			factor = pRules->DeactivateDim_Operator;
		}
	}

	Value = int(Value * factor);

	return 0x73C15F;
}

DEFINE_OVERRIDE_HOOK(0x471C96, CaptureManagerClass_CanCapture, 0xA)
{
	// this is a complete rewrite, because it might be easier to change
	// this in a central place than spread all over the source code.
	enum
	{
		Allowed = 0x471D2E, // this can be captured
		Disallowed = 0x471D35 // can't be captured
	};

	GET(CaptureManagerClass*, pThis, ECX);
	GET(TechnoClass*, pTarget, ESI);
	TechnoClass* pCapturer = pThis->Owner;

	// target exists and doesn't belong to capturing player
	if (!pTarget || pTarget->Owner == pCapturer->Owner)
	{
		return Disallowed;
	}

	// generally not capturable
	if (TechnoExt::IsPsionicsImmune(pTarget))
	{
		return Disallowed;
	}

	// disallow capturing bunkered units
	if (pTarget->BunkerLinkedItem && pTarget->BunkerLinkedItem->WhatAmI() == AbstractType::Unit)
	{
		return Disallowed;
	}

	// TODO: extend this for mind-control priorities
	if (pTarget->IsMindControlled() || pTarget->MindControlledByHouse)
	{
		return Disallowed;
	}

	// free slot? (move on if infinite or single slot which will be freed if used)
	if (!pThis->InfiniteMindControl && pThis->MaxControlNodes != 1 && pThis->ControlNodes.Count >= pThis->MaxControlNodes)
	{
		return Disallowed;
	}

	// currently disallowed
	auto mission = pTarget->CurrentMission;
	if (pTarget->IsIronCurtained() || mission == Mission::Selling || mission == Mission::Construction)
	{
		return Disallowed;
	}

	// driver killed. has no mind.
	if (pTarget->align_154->Is_DriverKilled)
	{
		return Disallowed;
	}

	// passed all tests
	return Allowed;
}

DEFINE_OVERRIDE_HOOK(0x51DF38, InfantryClass_Remove, 0xA)
{
	GET(InfantryClass*, pThis, ESI);

	if (auto pGarrison = pThis->align_154->GarrisonedIn)
	{
		if (!pGarrison->Occupants.Remove(pThis))
		{
			Debug::Log("Infantry %s was garrisoned in building %s, but building didn't find it. WTF?",
				pThis->Type->ID, pGarrison->Type->ID);
		}
	}

	pThis->align_154->GarrisonedIn = nullptr;

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x51DFFD, InfantryClass_Put, 5)
{
	GET(InfantryClass*, pThis, EDI);
	pThis->align_154->GarrisonedIn = nullptr;
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x518434, InfantryClass_ReceiveDamage_SkipDeathAnim, 7)
{
	GET(InfantryClass*, pThis, ESI);
	//GET_STACK(ObjectClass *, pAttacker, 0xE0);

	// there is not InfantryExt ExtMap yet!
	// too much space would get wasted since there is only four bytes worth of data we need to store per object
	// so those four bytes get stashed in Techno Map instead. they will get their own map if there's ever enough data to warrant it

	return pThis->align_154->GarrisonedIn ? 0x5185F1 : 0;
}

DEFINE_OVERRIDE_HOOK(0x446EE2, BuildingClass_Place_InitialPayload, 6)
{
	GET(BuildingClass* const, pThis, EBP);
	TechnoExt::ExtMap.Find(pThis)->CreateInitialPayload();
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x4D718C, FootClass_Put_InitialPayload, 6)
{
	GET(FootClass* const, pThis, ESI);

	if (pThis->WhatAmI() != AbstractType::Infantry)
	{
		TechnoExt::ExtMap.Find(pThis)->CreateInitialPayload();
	}

	return 0;
}

// temporal per-slot
DEFINE_OVERRIDE_HOOK(0x71A84E, TemporalClass_UpdateA, 5)
{
	GET(TemporalClass* const, pThis, ESI);

	// it's not guaranteed that there is a target
	if (auto const pTarget = pThis->Target)
	{
		if (auto pJammer = std::exchange(pTarget->align_154->RadarJammerUptr, nullptr))
		{
			((AresJammer*)pJammer)->UnjamAll();
			AresMemory::Delete(pJammer);
		}

		//AttachEffect handling under Temporal
		AresData::UpdateAEData(&pTarget->align_154->AEDatas);
	}

	pThis->WarpRemaining -= pThis->GetWarpPerStep(0);

	R->EAX(pThis->WarpRemaining);
	return 0x71A88D;
}

// replace the cloak checking functions to include checks for new features
DEFINE_OVERRIDE_HOOK(0x6FB757, TechnoClass_UpdateCloak, 8)
{
	GET(TechnoClass*, pThis, ESI);
	return !TechnoExt_ExtData::CloakDisallowed(pThis, false) ? 0x6FB7FD : 0x6FB75F;
}

DEFINE_OVERRIDE_HOOK(0x6FBC90, TechnoClass_ShouldNotBeCloaked, 5)
{
	GET(TechnoClass*, pThis, ECX);
	// the original code would not disallow cloaking as long as
	// pThis->Cloakable is set, but this prevents CloakStop from
	// working, because it overrides IsCloakable().
	R->EAX(TechnoExt_ExtData::CloakDisallowed(pThis, true));
	return 0x6FBDBC;
}

DEFINE_OVERRIDE_HOOK(0x6FBDC0, TechnoClass_ShouldBeCloaked, 5)
{
	GET(TechnoClass*, pThis, ECX);
	R->EAX(TechnoExt_ExtData::CloakAllowed(pThis));
	return 0x6FBF93;
}

// DEFINE_OVERRIDE_HOOK(0x6F6AC9, TechnoClass_Remove_Early, 6)
// {
// 	GET(TechnoClass*, pThis, ESI);
//
// 	// if the removed object is a radar jammer, unjam all jammed radars
// 	if (auto pRJ = std::exchange(pThis->align_154->RadarJammerUptr, nullptr))
// 	{
// 		AresData::JammerClassUnjamAll(pRJ);
// 		AresMemory::Delete(pRJ);
// 	}
//
// 	// #617 powered units
// 	if (auto pPower = std::exchange(pThis->align_154->PoweredUnitUptr, nullptr))
// 	{
// 		AresMemory::Delete(pPower);
// 	}
//
// 	//#1573, #1623, #255 attached effects
// 	if (AresData::RemoveAE(&pThis->align_154->AEDatas))
// 		AresData::RecalculateStat(pThis);
//
// 	if (pThis->align_154->TechnoValueAmount != 0)
// 	{
// 		AresData::FlyingStringsAdd(pThis, true);
// 	}
//
// 	return pThis->InLimbo ? 0x6F6C93u : 0x6F6AD5u;
// }

DEFINE_OVERRIDE_HOOK_AGAIN(0x6F6D0E, TechnoClass_Put_BuildingLight, 7)
DEFINE_OVERRIDE_HOOK(0x6F6F20, TechnoClass_Put_BuildingLight, 6)
{
	GET(TechnoClass*, pThis, ESI);

	const auto pExt = TechnoExt::ExtMap.Find(pThis);
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pExt->Type);

	//only update the SW if really needed it
	if (pThis->Owner && pThis->WhatAmI() != BuildingClass::AbsID && !pTypeExt->Linked_SW.empty())
		pThis->Owner->UpdateSuperWeaponsUnavailable();

	if (pTypeExt->HasSpotlight)
	{
		TechnoExt_ExtData::SetSpotlight(pThis, GameCreate<BuildingLightClass>(pThis));
	}

	return 0x0;
}

DEFINE_OVERRIDE_HOOK(0x6FD0BF, TechnoClass_GetROF_AttachEffect, 6)
{
	GET(TechnoClass*, pThis, ESI);

	const auto nRof = pThis->align_154->AE_ROF;
	__asm { fmul nRof };
	return 0x0;
}

DEFINE_OVERRIDE_HOOK(0x451330, BuildingClass_GetCrewCount, 0xA)
{
	GET(BuildingClass*, pThis, ECX);

	int count = 0;

	if (!pThis->NoCrew && pThis->Type->Crewed)
	{
		auto pHouse = pThis->Owner;

		// get the divisor
		int divisor = HouseExt::GetSurvivorDivisor(pHouse);

		if (divisor > 0)
		{
			// if captured, less survivors
			if (pThis->HasBeenCaptured)
			{
				divisor *= 2;
			}

			// value divided by "cost per survivor"
			count = pThis->Type->GetRefund(pHouse, 0) / divisor;

			// clamp between 1 and 5
			if (count < 1)
			{
				count = 1;
			}
			if (count > 5)
			{
				count = 5;
			}
		}
	}

	R->EAX(count);
	return 0x4513CD;
}

DEFINE_OVERRIDE_HOOK(0x707D20, TechnoClass_GetCrew, 5)
{
	GET(TechnoClass*, pThis, ECX);
	auto pType = pThis->GetTechnoType();
	auto pExt = TechnoTypeExt::ExtMap.Find(pType);

	auto pHouse = pThis->Owner;
	InfantryTypeClass* pCrewType = nullptr;

	// YR defaults to 15 for armed objects,
	// Ares < 0.5 defaulted to 0 for non-buildings.
	const int TechnicianChance = pExt->Crew_TechnicianChance.Get(
		abstract_cast<FootClass*>(pThis) ?
		0 :
		pThis->IsArmed() ? 15 : 0
	);

	if (pType->Crewed)
	{
		// for civilian houses always technicians. random for others
		const bool isTechnician = pHouse->Type->SideIndex == -1 ? true :
			TechnicianChance > 0 && ScenarioClass::Instance->Random.RandomFromMax(99) < TechnicianChance
			? true : false;

		// chose the appropriate type
		if (!isTechnician)
		{
			// customize with this techno's pilot type
			// only use it if non-null, as documented

			const auto& nVec = pExt->Survivors_Pilots;

			if ((size_t)pHouse->SideIndex >= nVec.size())
			{
				pCrewType = HouseExt::GetCrew(pHouse);
			}
			else if (auto pPilotType = nVec[pHouse->SideIndex])
			{
				pCrewType = pPilotType;
			}
			else
			{
				pCrewType = HouseExt::GetCrew(pHouse);
			}
		}
		else
		{
			// either civilian side or chance
			pCrewType = HouseExt::GetTechnician(pHouse);
		}
	}

	R->EAX(pCrewType);
	return 0x707DCF;
}


DEFINE_OVERRIDE_HOOK(0x44EB10, BuildingClass_GetCrew, 9)
{
	GET(BuildingClass*, pThis, ECX);

	// YR defaults to 25 for buildings producing buildings
	R->EAX(TechnoExt_ExtData::GetBuildingCrew(pThis, TechnoTypeExt::ExtMap.Find(pThis->Type)->
		Crew_EngineerChance.Get((pThis->Type->Factory == BuildingTypeClass::AbsID) ? 25 : 0)));

	return 0x44EB5B;
}

DEFINE_OVERRIDE_HOOK(0x6FD438, TechnoClass_FireLaser, 6)
{
	GET(WeaponTypeClass*, pWeapon, ECX);
	GET(LaserDrawClass*, pBeam, EAX);

	auto const pData = WeaponTypeExt::ExtMap.Find(pWeapon);

	if (pData->Laser_Thickness > 1)
	{
		pBeam->Thickness = pData->Laser_Thickness;
	}

	return 0;
}

//// issue #1324: enemy repair wrench visible when it shouldn't
DEFINE_OVERRIDE_HOOK(0x6f526c, TechnoClass_DrawExtras_PowerOff, 5)
{
	GET(TechnoClass*, pTechno, EBP);
	GET_STACK(RectangleStruct*, pRect, 0xA0);

	if (auto pBld = specific_cast<BuildingClass*>(pTechno))
	{
		const auto pBldExt = BuildingExt::ExtMap.Find(pBld);

		// allies and observers can always see by default
		const bool canSeeRepair = HouseClass::CurrentPlayer->IsAlliedWith_(pBld->Owner)
			|| HouseClass::IsCurrentPlayerObserver();

		const bool showRepair = FileSystem::WRENCH_SHP
			&& pBld->IsBeingRepaired
			// fixes the wrench playing over a temporally challenged building
			&& !pBld->IsBeingWarpedOut()
			&& !pBld->WarpingOut
			// never show to enemies when cloaked, and only if allowed
			&& (canSeeRepair || (pBld->CloakState == CloakState::Uncloaked
				&& RulesExt::Global()->EnemyWrench));

		// display power off marker only for current player's buildings
		const bool showPower = FileSystem::POWEROFF_SHP
			&& (!pBldExt->TogglePower_HasPower || (pBld->Type->PowerDrain && pBld->Owner->HasLowPower()) || pBld->TemporalTargetingMe)
			// only for owned buildings, but observers got magic eyes
			&& (pBld->Owner->ControlledByPlayer() || HouseClass::IsCurrentPlayerObserver());

		// display any?
		if (showPower || showRepair)
		{
			auto cell = pBld->GetMapCoords();

			if (!MapClass::Instance->GetCellAt(cell)->IsShrouded())
			{
				CoordStruct crd = pBld->GetCenterCoords();

				Point2D point {};
				TacticalClass::Instance->CoordsToClient(&crd, &point);

				// offset the markers
				Point2D ptRepair = point;
				if (showPower)
				{
					ptRepair.X -= 7;
					ptRepair.Y -= 7;
				}

				Point2D ptPower = point;
				if (showRepair)
				{
					ptPower.X += 18;
					ptPower.Y += 18;
				}

				// animation display speed
				// original frame calculation: ((currentframe%speed)*6)/(speed-1)
				int speed = GameOptionsClass::Instance->GetAnimSpeed(14) / 4;
				if (speed < 2)
				{
					speed = 2;
				}

				// draw the markers
				if (showRepair)
				{
					int frame = (FileSystem::WRENCH_SHP->Frames * (Unsorted::CurrentFrame % speed)) / speed;
					DSurface::Temp->DrawSHP(FileSystem::MOUSE_PAL, FileSystem::WRENCH_SHP,
						frame, &ptRepair, pRect, BlitterFlags(0xE00), 0, 0, 0, 1000, 0, 0, 0, 0, 0);
				}

				if (showPower)
				{
					int frame = (FileSystem::POWEROFF_SHP->Frames * (Unsorted::CurrentFrame % speed)) / speed;
					DSurface::Temp->DrawSHP(FileSystem::MOUSE_PAL, FileSystem::POWEROFF_SHP,
						frame, &ptPower, pRect, BlitterFlags(0xE00), 0, 0, 0, 1000, 0, 0, 0, 0, 0);
				}
			}
		}
	}

	return 0x6F5347;
}

DEFINE_OVERRIDE_HOOK(0x70AA60, TechnoClass_DrawExtraInfo, 6)
{
	GET(TechnoClass*, pThis, ECX);
	GET_STACK(Point2D*, pPoint, 0x4);
	//GET_STACK(Point2D*, pOriginalPoint, 0x8);
	//	GET_STACK(unsigned int , nFrame, 0x4);
	GET_STACK(RectangleStruct*, pRect, 0xC);

	if (!HouseClass::CurrentPlayer)
		return 0x70AD4C;

	if (auto pBuilding = specific_cast<BuildingClass*>(pThis))
	{
		auto const pType = pBuilding->Type;
		auto const pOwner = pBuilding->Owner;
		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

		if (!pType || !pOwner)
			return 0x70AD4C;

		Point2D DrawLoca = *pPoint;
		auto DrawTheStuff = [&](const wchar_t* pFormat)
		{
			//DrawingPart
			RectangleStruct nTextDimension;
			Drawing::GetTextDimensions(&nTextDimension, pFormat, DrawLoca, TextPrintType::Center | TextPrintType::FullShadow | TextPrintType::Efnt, 4, 2);
			auto nIntersect = Drawing::Intersect(nTextDimension, *pRect);
			auto nColorInt = pOwner->Color.ToInit();//0x63DAD0

			DSurface::Temp->Fill_Rect(nIntersect, (COLORREF)0);
			DSurface::Temp->Draw_Rect(nIntersect, (COLORREF)nColorInt);
			Point2D nRet;
			Simple_Text_Print_Wide(&nRet, pFormat, DSurface::Temp.get(), pRect, &DrawLoca, (COLORREF)nColorInt, (COLORREF)0, TextPrintType::Center | TextPrintType::FullShadow | TextPrintType::Efnt, true);
			DrawLoca.Y += (nTextDimension.Height) + 2; //extra number for the background
		};

		const bool IsAlly = pOwner->IsAlliedWith_(HouseClass::CurrentPlayer);
		const bool IsObserver = HouseClass::CurrentPlayer->IsObserver();
		const bool isFake = pTypeExt->Fake_Of.Get();
		const bool bReveal = pThis->DisplayProductionTo.Contains(HouseClass::CurrentPlayer);

		if (IsAlly || IsObserver || bReveal)
		{
			if (isFake)
				DrawTheStuff(StringTable::LoadString("TXT_FAKE"));

			if (pType->PowerBonus > 0)
			{
				auto pDrainFormat = StringTable::LoadString(GameStrings::TXT_POWER_DRAIN2());
				wchar_t pOutDraimFormat[0x80];
				auto pDrain = (int)pOwner->Power_Drain();
				auto pOutput = (int)pOwner->Power_Output();
				swprintf_s(pOutDraimFormat, pDrainFormat, pOutput, pDrain);
				DrawTheStuff(pOutDraimFormat);
			}

			if (pType->Storage > 0)
			{
				auto pMoneyFormat = StringTable::LoadString(GameStrings::TXT_MONEY_FORMAT_1());
				wchar_t pOutMoneyFormat[0x80];
				auto nMoney = pOwner->Available_Money();
				swprintf_s(pOutMoneyFormat, pMoneyFormat, nMoney);
				DrawTheStuff(pOutMoneyFormat);
			}

			if (pThis->IsPrimaryFactory)
			{
				DrawTheStuff(StringTable::LoadString((pType->GetFoundationWidth() != 1) ?
					GameStrings::TXT_PRIMARY() : GameStrings::TXT_PRI()));
			}
		}
	}

	return 0x70AD4C;
}


DEFINE_OVERRIDE_HOOK(0x43E7B0, BuildingClass_DrawVisible, 5)
{
	GET(BuildingClass*, pThis, ECX);
	GET_STACK(Point2D*, pLocation, 0x4);
	GET_STACK(RectangleStruct*, pBounds, 0x8);

	auto pType = pThis->Type;

	if (!pThis->IsSelected || !HouseClass::CurrentPlayer)
		return 0x43E8F2;

	const auto pTypeExt = BuildingTypeExt::ExtMap.Find(pType);

	// helpers (with support for the new spy effect)
	const bool bAllied = pThis->Owner->IsAlliedWith_(HouseClass::CurrentPlayer);
	const bool IsObserver = HouseClass::CurrentPlayer->IsObserver();
	const bool bReveal = pTypeExt->SpyEffect_RevealProduction && pThis->DisplayProductionTo.Contains(HouseClass::CurrentPlayer);

	// show building or house state
	if (bAllied || IsObserver || bReveal)
	{
		Point2D DrawExtraLoc = { pLocation->X , pLocation->Y };
		pThis->DrawExtraInfo(DrawExtraLoc, pLocation, pBounds);

		// display production cameo
		if (IsObserver || bReveal)
		{
			const auto pFactory = pThis->Owner->IsControlledByHuman_() ?
				pThis->Owner->GetPrimaryFactory(pType->Factory, pType->Naval, BuildCat::DontCare)
				: pThis->Factory;

			if (pFactory && pFactory->Object)
			{
				auto pProdType = pFactory->Object->GetTechnoType();
				const int nTotal = pFactory->CountTotal(pProdType);
				Point2D DrawCameoLoc = { pLocation->X , pLocation->Y + 45 };
				const auto pProdTypeExt = TechnoTypeExt::ExtMap.Find(pProdType);
				RectangleStruct cameoRect {};

				// support for pcx cameos
				if (auto pPCX = TechnoTypeExt_ExtData::GetPCXSurface(pProdType , pThis->Owner))
				{
					const int cameoWidth = 60;
					const int cameoHeight = 48;

					RectangleStruct cameoBounds = { 0, 0, pPCX->Width, pPCX->Height };
					RectangleStruct DefcameoBounds = { 0, 0, cameoWidth, cameoHeight };
					RectangleStruct destRect = { DrawCameoLoc.X - cameoWidth / 2, DrawCameoLoc.Y - cameoHeight / 2, cameoWidth , cameoHeight };

					if (Game::func_007BBE20(&destRect, pBounds, &DefcameoBounds, &cameoBounds))
					{
						cameoRect = destRect;
						AresPcxBlit<WORD> blithere((0xFFu >> ColorStruct::BlueShiftRight << ColorStruct::BlueShiftLeft) | (0xFFu >> ColorStruct::RedShiftRight << ColorStruct::RedShiftLeft));
						Buffer_To_Surface_wrapper(DSurface::Temp, &destRect, pPCX, &DefcameoBounds, &blithere, 0, 3, 1000, 0);
					}
				}
				else
				{
					// old shp cameos, fixed palette
					if (auto pCameo = pProdType->GetCameo())
					{
						cameoRect = { DrawCameoLoc.X, DrawCameoLoc.Y, pCameo->Width, pCameo->Height };

						ConvertClass* pPal = FileSystem::CAMEO_PAL();
						if (auto pManager = pProdTypeExt->CameoPal)
							pPal = pManager->GetConvert<PaletteManager::Mode::Default>();

						DSurface::Temp->DrawSHP(pPal, pCameo, 0, &DrawCameoLoc, pBounds, BlitterFlags(0xE00), 0, 0, 0, 1000, 0, nullptr, 0, 0, 0);
					}
				}

				//auto nColorInt = pThis->Owner->Color.ToInit();//0x63DAD0
				//DSurface::Temp->Draw_Rect(cameoRect, (COLORREF)nColorInt);
				//Point2D DrawTextLoc = { DrawCameoLoc.X - 20 , DrawCameoLoc.Y - 20 };
				//std::wstring pFormat = std::to_wstring(nTotal);
				//pFormat += L"X";
				//RectangleStruct nTextDimension;
				//Drawing::GetTextDimensions(&nTextDimension, pFormat.c_str(), DrawTextLoc, TextPrintType::Center | TextPrintType::FullShadow | TextPrintType::Efnt, 4, 2);
				//auto nIntersect = Drawing::Intersect(nTextDimension, cameoRect);

				//DSurface::Temp->Fill_Rect(nIntersect, (COLORREF)0);
				//DSurface::Temp->Draw_Rect(nIntersect, (COLORREF)nColorInt);

				//Point2D nRet;
				//Simple_Text_Print_Wide(&nRet, pFormat.c_str(), DSurface::Temp.get(), &cameoRect, &DrawTextLoc, (COLORREF)nColorInt, (COLORREF)0, TextPrintType::Center | TextPrintType::FullShadow | TextPrintType::Efnt, true);
			}
		}
	}

	return 0x43E8F2;
}

DEFINE_OVERRIDE_HOOK(0x452218, BuildingClass_Enable_Temporal_Factories, 6)
{
	GET(BuildingClass*, pThis, ECX);
	TechnoExt_ExtData::UpdateFactoryQueues(pThis);
	return 0;
}

// complete replacement
DEFINE_OVERRIDE_HOOK(0x70FBE0, TechnoClass_Activate_AresReplace, 6)
{
	GET(TechnoClass* const, pThis, ECX);

	const auto pType = pThis->GetTechnoType();

	if (pType->PoweredUnit && pThis->Owner)
	{
		pThis->Owner->RecheckPower = true;
	}

	/* Check abort conditions:
		- Is the object currently EMP'd?
		- Does the object need an operator, but doesn't have one?
		- Does the object need a powering structure that is offline?
		If any of the above conditions, bail out and don't activate the object.
	*/

	if (pThis->IsUnderEMP() || !TechnoExt_ExtData::IsPowered(pThis))
	{
		return 0x70FC85;
	}

	if (TechnoExt_ExtData::IsOperatedB(pThis))
	{
		pThis->Guard();

		if (auto const pFoot = abstract_cast<FootClass*>(pThis))
		{
			pFoot->Locomotor.GetInterfacePtr()->Power_On();
		}

		if (auto const wasDeactivated = std::exchange(pThis->Deactivated, false))
		{
			// change: don't play sound when mutex active
			if (!Unsorted::ScenarioInit && pType->ActivateSound != -1)
			{
				VocClass::PlayAt(pType->ActivateSound, pThis->Location, nullptr);
			}

			// change: add spotlight
			auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
			if (pTypeExt->HasSpotlight)
			{
				++Unsorted::ScenarioInit;
				TechnoExt_ExtData::SetSpotlight(pThis, GameCreate<BuildingLightClass>(pThis));
				--Unsorted::ScenarioInit;
			}

			// change: update factories
			if (auto const pBld = specific_cast<BuildingClass*>(pThis))
			{
				TechnoExt_ExtData::UpdateFactoryQueues(pBld);
			}
		}
	}

	return 0x70FC85;
}

// complete replacement
DEFINE_OVERRIDE_HOOK(0x70FC90, TechnoClass_Deactivate_AresReplace, 6)
{
	GET(TechnoClass* const, pThis, ECX);

	const auto pType = pThis->GetTechnoType();

	if (pType->PoweredUnit && pThis->Owner)
	{
		pThis->Owner->RecheckPower = true;
	}

	// don't deactivate when inside/on the linked building
	if (pThis->IsTethered)
	{
		auto const pLink = pThis->GetNthLink(0);

		if (pLink && pThis->GetCell()->GetBuilding() == pLink)
		{
			return 0x70FD6E;
		}
	}

	pThis->Guard();
	pThis->Deselect();

	if (auto const pFoot = abstract_cast<FootClass*>(pThis))
	{
		pFoot->Locomotor.GetInterfacePtr()->Power_Off();
	}

	auto const wasDeactivated = std::exchange(pThis->Deactivated, true);

	if (!wasDeactivated)
	{
		// change: don't play sound when mutex active
		if (!Unsorted::ScenarioInit && pType->DeactivateSound != -1)
		{
			VocClass::PlayAt(pType->DeactivateSound, pThis->Location, nullptr);
		}

		// change: remove spotlight
		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
		if (pTypeExt->HasSpotlight)
		{
			TechnoExt_ExtData::SetSpotlight(pThis, nullptr);
		}

		// change: update factories
		if (auto const pBld = specific_cast<BuildingClass*>(pThis))
		{
			TechnoExt_ExtData::UpdateFactoryQueues(pBld);
		}
	}

	return 0x70FD6E;
}

DEFINE_OVERRIDE_HOOK_AGAIN(0x6FB4A3, TechnoClass_CreateGap_LargeGap, 7)
DEFINE_OVERRIDE_HOOK(0x6FB1B5, TechnoClass_CreateGap_LargeGap, 7)
{
	GET(TechnoClass*, pThis, ESI);
	GET(TechnoTypeClass*, pType, EAX);

	pThis->GapRadius = TechnoTypeExt::ExtMap.Find(pType)->GapRadiusInCells;
	return R->Origin() + 0xD;
}

// Radar Jammers (#305) unjam all on owner change
DEFINE_OVERRIDE_HOOK(0x7014D5, TechnoClass_ChangeOwnership_Additional, 6)
{
	GET(TechnoClass* const, pThis, ESI);

	if (auto pJammer = pThis->align_154->RadarJammerUptr) {
		((AresJammer*)pJammer)->UnjamAll();
	}

	if (pThis->align_154->TechnoValueAmount != 0)
		AresData::FlyingStringsAdd(pThis, true);

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x413FD2, AircraftClass_Init_Academy, 6)
{
	GET(AircraftClass*, pThis, ESI);

	if (pThis->Owner)
	{
		if (pThis->Type->Trainable && HouseExt::ExtMap.Find(pThis->Owner)->Is_AirfieldSpied)
			pThis->Veterancy.Veterancy = 1.0f;

		HouseExt::ApplyAcademy(pThis->Owner, pThis, AbstractType::Aircraft);
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x517D51, InfantryClass_Init_Academy, 6)
{
	GET(InfantryClass*, pThis, ESI);

	if (pThis->Owner) {
		HouseExt::ApplyAcademy(pThis->Owner, pThis, AbstractType::Infantry);
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x442D1B, BuildingClass_Init_Academy, 6)
{
	GET(BuildingClass*, pThis, ESI);

	if (!pThis->Owner)
		return 0x0;

	if(HouseTypeExt::ExtMap.Find(pThis->Owner->Type)->VeteranBuildings.Contains(pThis->Type)) {
		pThis->Veterancy.Veterancy = 1.0f;
	}

	if (pThis->Type->Trainable && HouseExt::ExtMap.Find(pThis->Owner)->Is_ConstructionYardSpied)
		pThis->Veterancy.Veterancy = 1.0f;


	HouseExt::ApplyAcademy(pThis->Owner, pThis, AbstractType::Building);

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x415085, AircraftClass_Update_DamageSmoke, 7)
{
	GET(AircraftClass*, pThis, ESI);
	auto pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pThis->GetHealthPercentage() < RulesClass::Instance->ConditionRed)
	{
		if (pThis->GetHeight() > 0)
		{
			if (AnimTypeClass* pType = pExt->SmokeAnim)
			{
				const int chance = ((pThis->Health > 0) ? pExt->SmokeChanceRed : pExt->SmokeChanceDead).Get();

				if (ScenarioClass::Instance->Random.RandomFromMax(99) < chance)
				{
					if (auto pAnim = GameCreate<AnimClass>(pType, pThis->Location))
						pAnim->Owner = pThis->GetOwningHouse();
				}
			}
		}
	}

	return 0x41512C;
}


// bug #1290: carryall size limit
DEFINE_OVERRIDE_HOOK(0x417D75, AircraftClass_GetActionOnObject_CanTote, 5)
{
	GET(AircraftClass*, pCarryall, ESI);
	GET(UnitClass*, pTarget, EDI);

	return (TechnoTypeExt_ExtData::CarryallCanLift(pCarryall->Type, pTarget))
		? 0u
		: 0x417DF6u
		;
}

DEFINE_OVERRIDE_HOOK(0x416E37, AircraftClass_Mi_MoveCarryall_CanTote, 5)
{
	GET(AircraftClass*, pCarryall, ESI);
	GET(UnitClass*, pTarget, EDI);

	return (TechnoTypeExt_ExtData::CarryallCanLift(pCarryall->Type, pTarget))
		? 0u
		: 0x416EC9u
		;
}

DEFINE_OVERRIDE_HOOK(0x41949F, AircraftClass_ReceivedRadioCommand_SpecificPassengers, 6)
{
	GET(AircraftClass* const, pThis, ESI);
	GET_STACK(TechnoClass const* const, pSender, 0x14);

	enum { Allowed = 0x41945Fu, Disallowed = 0x41951Fu };

	auto const pType = pThis->Type;

	if (pThis->Passengers.NumPassengers >= pType->Passengers)
	{
		return Disallowed;
	}

	auto const pSenderType = pSender->GetTechnoType();

	return TechnoTypeExt::PassangersAllowed(pType, pSenderType) ? Allowed : Disallowed;
}

DEFINE_OVERRIDE_HOOK(0x41946B, AircraftClass_ReceivedRadioCommand_QueryEnterAsPassenger_KillDriver, 6)
{
	// prevent units from getting the enter cursor on transports
	// with killed drivers.
	GET(TechnoClass*, pThis, ESI);
	return (pThis->align_154->Is_DriverKilled ? 0x4190DDu : 0u);
}


// issue #279: per unit AirstrikeAttackVoice and AirstrikeAbortSound
DEFINE_OVERRIDE_HOOK(0x41D940, AirstrikeClass_Fire_AirstrikeAttackVoice, 5)
{
	GET(AirstrikeClass*, pAirstrike, EDI);

	// get default from rules
	int index = RulesClass::Instance->AirstrikeAttackVoice;

	// get from aircraft
	const auto pAircraftExt = TechnoTypeExt::ExtMap.Find(pAirstrike->FirstObject->GetTechnoType());
	if (pAircraftExt->VoiceAirstrikeAttack.isset())
		index = pAircraftExt->VoiceAirstrikeAttack.Get();

	// get from designator
	if (const auto pOwner = pAirstrike->Owner)
	{
		auto pOwnerExt = TechnoTypeExt::ExtMap.Find(pOwner->GetTechnoType());

		if (pOwnerExt->VoiceAirstrikeAttack.isset())
			index = pOwnerExt->VoiceAirstrikeAttack.Get();
	}

	VocClass::PlayAt(index, pAirstrike->FirstObject->Location, nullptr);
	return 0x41D970;
}

DEFINE_OVERRIDE_HOOK(0x41D5AE, AirstrikeClass_PointerGotInvalid_AirstrikeAbortSound, 9)
{
	GET(AirstrikeClass*, pAirstrike, ESI);

	// get default from rules
	int index = RulesClass::Instance->AirstrikeAbortSound;

	// get from aircraft
	const auto pAircraftExt = TechnoTypeExt::ExtMap.Find(pAirstrike->FirstObject->GetTechnoType());
	if (pAircraftExt->VoiceAirstrikeAbort.isset())
		index = pAircraftExt->VoiceAirstrikeAbort.Get();

	// get from designator
	if (const auto pOwner = pAirstrike->Owner)
	{
		auto pOwnerExt = TechnoTypeExt::ExtMap.Find(pOwner->GetTechnoType());
		if (pOwnerExt->VoiceAirstrikeAbort.isset())
			index = pOwnerExt->VoiceAirstrikeAbort.Get();
	}

	VocClass::PlayAt(index, pAirstrike->FirstObject->Location, nullptr);
	return 0x41D5E0;
}

DEFINE_OVERRIDE_HOOK(0x43FE8E, BuildingClass_Update_Reload, 6)
{
	GET(BuildingClass*, B, ESI);
	BuildingTypeClass* BType = B->Type;
	if (!BType->Hospital && !BType->Armory)
	{ // TODO: rethink this
		B->Reload();
	}
	return 0x43FEBE;
}

DEFINE_OVERRIDE_HOOK(0x440C08, BuildingClass_Put_AIBaseNormal, 6)
{
	GET(BuildingClass*, pThis, ESI);
	R->EAX(TechnoExt_ExtData::IsBaseNormal(pThis));
	return 0x440C2C;
}

DEFINE_OVERRIDE_HOOK(0x456370, BuildingClass_UnmarkBaseSpace_AIBaseNormal, 6)
{
	GET(BuildingClass*, pThis, ESI);
	R->EAX(TechnoExt_ExtData::IsBaseNormal(pThis));
	return 0x456394;
}

DEFINE_OVERRIDE_HOOK(0x445A72, BuildingClass_Remove_AIBaseNormal, 6)
{
	GET(BuildingClass*, pThis, ESI);
	R->EAX(TechnoExt_ExtData::IsBaseNormal(pThis));
	return 0x445A94;
}

DEFINE_OVERRIDE_HOOK(0x4A8FF5, MapClass_CanBuildingTypeBePlacedHere_Ignore, 5)
{
	GET(BuildingClass*, pBuilding, ESI);
	return BuildingExt::ExtMap.Find(pBuilding)->IsFromSW ? 0x4A8FFA : 0x0;
}

DEFINE_OVERRIDE_HOOK(0x506306, HouseClass_FindPlaceToBuild_Evaluate, 6)
{
	GET(BuildingTypeClass*, pBuilding, EDX);
	auto pEXt = BuildingTypeExt::ExtMap.Find(pBuilding);
	R->CL(pEXt->AIInnerBase.Get(pBuilding->CloakGenerator));
	return 0x50630C;
}

DEFINE_OVERRIDE_HOOK(0x442974, BuildingClass_ReceiveDamage_Malicious, 6)
{
	GET(BuildingClass*, pThis, ESI);
	GET_STACK(WarheadTypeClass*, pWH, 0xA8);

	BuildingExt::ExtMap.Find(pThis)->ReceiveDamageWarhead = pWH;
	pThis->BuildingUnderAttack();

	return 0x442980;
}

//DEFINE_SKIP_HOOK(0x71B09C, TemporalClass_Logic_BuildingUnderAttack_NullptrShit, 0x5, 71B0E7);
DEFINE_JUMP(LJMP, 0x71B09C, 0x71B0E7);

DEFINE_OVERRIDE_HOOK(0x4F94A5, HouseClass_BuildingUnderAttack, 6)
{
	GET(BuildingClass*, pSource, ESI);

	if (auto pWh = std::exchange(BuildingExt::ExtMap.Find(pSource)->ReceiveDamageWarhead, nullptr))
	{
		if (!WarheadTypeExt::ExtMap.Find(pWh)->Malicious)
		{
			return 0x4F95D4;
		}
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x7384BD, UnitClass_ReceiveDamage_OreMinerUnderAttack, 6)
{
	GET_STACK(WarheadTypeClass*, WH, STACK_OFFS(0x44, -0xC));
	return WH && !WarheadTypeExt::ExtMap.Find(WH)->Malicious ? 0x738535u : 0u;
}

DEFINE_OVERRIDE_HOOK(0x6BED08, Game_Terminate_Mouse, 7)
{
	GameDelete<false, false>(R->ECX<void*>());
	return 0x6BED34;
}

DEFINE_DISABLE_HOOK(0x56017A, OptionsDlg_WndProc_RemoveResLimit_ares)//, 0x5, 560183)
DEFINE_JUMP(LJMP, 0x56017A, 0x560183);

DEFINE_DISABLE_HOOK(0x5601E3, OptionsDlg_WndProc_RemoveHiResCheck_ares)//, 0x9, 5601FC)
DEFINE_JUMP(LJMP, 0x5601E3, 0x5601FC);

DEFINE_OVERRIDE_HOOK(0x702E64, TechnoClass_RegisterDestruction_Bounty, 6)
{
	GET(TechnoClass*, pVictim, ESI);
	GET(TechnoClass*, pKiller, EDI);

	TechnoExt_ExtData::GiveBounty(pVictim, pKiller);

	return 0x0;
}

DEFINE_OVERRIDE_HOOK(0x51E7BF, InfantryClass_GetActionOnObject_CanCapture, 6)
{
	enum
	{
		Capture = 0x51E84B,  // the game will return an Enter cursor no questions asked
		DontCapture = 0x51E85A, // the game will assume this is not a VehicleThief and will check for other cursors normally
		Select = 0x51E7EF, // select target instead of ordering this
		DontMindMe = 0, // the game will check if this is a VehicleThief
	};

	GET(InfantryClass*, pSelected, EDI);
	GET(ObjectClass*, pTarget, ESI);

	TechnoClass* pTechnoTarget = generic_cast<TechnoClass*>(pTarget);

	if (!pTechnoTarget)
		return DontCapture;

	const auto pSelectedType = pSelected->Type;
	if (!pSelectedType->VehicleThief && !TechnoTypeExt::ExtMap.Find(pSelectedType)->CanDrive)
		return DontCapture;

	if (pTechnoTarget->GetTechnoType()->IsTrain)
		return Select;

	//const auto nResult = (AresHijackActionResult)AresData::TechnoExt_GetActionHijack(pSelected, pTechnoTarget);
	const auto nResult = TechnoExt_ExtData::GetActionHijack(pSelected, pTechnoTarget);
	if (nResult == AresHijackActionResult::None)
		return DontCapture;

	if (nResult == AresHijackActionResult::Drive && InputManagerClass::Instance->IsForceFireKeyPressed())
		return DontCapture;

	MouseCursorFuncs::SetMouseCursorAction(92, Action::Capture, false);
	return Capture;
}

// the hijacker is close to the target. capture.
DEFINE_OVERRIDE_HOOK(0x5203F7, InfantryClass_UpdateVehicleThief_Hijack, 5)
{
	enum { GoOn = 0x5206A1, Stop = 0x520473 };

	GET(InfantryClass*, pThis, ESI);
	GET(FootClass*, pTarget, EDI);
	TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pThis);

	bool finalize = TechnoExt_ExtData::PerformActionHijack(pThis, pTarget);
	if (finalize)
	{
		// manually deinitialize this infantry
		if(pThis->IsAlive)
			pThis->UnInit();
	}
	return finalize ? Stop : GoOn;
}

// change all the special things infantry do, like vehicle thief, infiltration,
// bridge repair, enter transports or bio reactors, ...
DEFINE_OVERRIDE_HOOK(0x519675, InfantryClass_UpdatePosition_BeforeInfantrySpecific, 0xA)
{
	// called after FootClass:UpdatePosition has been called and before
	// all specific infantry handling takes place.
	enum
	{
		Return = 0x51AA01, // skip the original logic
		Destroy = 0x51A010, // uninits this infantry and returns
		Handle = 0 // resume the original function
	} DoWhat = Handle;

	GET(InfantryClass*, pThis, ESI);

	if (pThis)
	{
		// steal vehicles / reclaim KillDriver'd units using CanDrive
		if (pThis->CurrentMission == Mission::Capture)
		{
			if (TechnoClass* pDest = generic_cast<TechnoClass*>(pThis->Destination))
			{
				// this is the possible target we stand on
				CellClass* pCell = pThis->GetCell();
				TechnoClass* pTarget = pCell->GetUnit(pThis->OnBridge);
				if (!pTarget)
				{
					pTarget = pCell->GetAircraft(pThis->OnBridge);
					if (!pTarget)
					{
						pTarget = pCell->GetBuilding();
						if (pTarget && !pTarget->IsStrange())
						{
							return 0;
						}
					}
				}

				// reached its destination?
				if (pTarget && pTarget == pDest)
				{
					// reached the target. capture.
					DoWhat = TechnoExt_ExtData::PerformActionHijack(pThis, pTarget) ? Destroy : Return;
				}
			}
		}
	}

	return DoWhat;
}

// update the vehicle thief's destination. needed to follow a
// target without the requirement to also enable Thief=yes.
DEFINE_OVERRIDE_HOOK(0x5202F9, InfantryClass_UpdateVehicleThief_Check, 6)
{
	GET(InfantryClass*, pThis, ESI);

	// good old WW checks for Thief. idiots.
	if (!pThis->Type->VehicleThief)
	{
		// also allow for drivers, because vehicles may still drive around. usually they are not.
		if (!TechnoTypeExt::ExtMap.Find(pThis->Type)->CanDrive)
		{
			return 0x5206A1;
		}
	}

	return 0x52030D;
}


DEFINE_OVERRIDE_HOOK(0x4DFE00, FootClass_GarrisonStructure_TakeVehicle, 6)
{
	GET(FootClass*, pThis, ECX);

	if (!pThis->align_154->TakeVehicleMode)
		return 0x0;

	R->EAX(TechnoExt_ExtData::FindAndTakeVehicle(pThis));
	return 0x4DFF3E;
}

// replaces the UnitReload handling and makes each docker independent of all
// others. this means planes don't have to wait one more ReloadDelay because
// the first docker triggered repair mission while the other dockers arrive
// too late and need to be put to sleep first.
DEFINE_OVERRIDE_HOOK(0x44C844, BuildingClass_MissionRepair_Reload, 6)
{
	GET(BuildingClass* const, pThis, EBP);
	auto const pExt = BuildingExt::ExtMap.Find(pThis);

	// ensure there are enough slots
	pExt->DockReloadTimers.resize(pThis->RadioLinks.Capacity, -1);

	// update all dockers, check if there's
	// at least one needing more attention
	bool keep_reloading = false;
	for (auto i = 0; i < pThis->RadioLinks.Capacity; ++i)
	{
		if (auto const pLink = pThis->GetNthLink(i))
		{

			auto const SendCommand = [=](RadioCommand command)
			{
				return pThis->SendCommand(command, pLink) == RadioCommand::AnswerPositive;
			};

			// check if reloaded and repaired already
			auto const pLinkType = pLink->GetTechnoType();
			auto done = SendCommand(RadioCommand::QueryReadiness)
				&& pLink->Health == pLinkType->Strength;

			if (!done)
			{
				// check if docked
				auto const miss = pLink->GetCurrentMission();
				if (miss == Mission::Enter
					|| !SendCommand(RadioCommand::QueryMoving))
				{
					continue;
				}

				keep_reloading = true;

				// make the unit sleep first
				if (miss != Mission::Sleep)
				{
					pLink->QueueMission(Mission::Sleep, false);
					continue;
				}

				// check whether the timer completed
				auto const last_timer = pExt->DockReloadTimers[i];
				if (last_timer > Unsorted::CurrentFrame)
				{
					continue;
				}

				// set the next frame
				auto const pLinkExt = TechnoTypeExt::ExtMap.Find(pLinkType);
				auto const defaultRate = RulesClass::Instance->ReloadRate;
				auto const rate = pLinkExt->ReloadRate.Get(defaultRate);
				auto const frames = static_cast<int>(rate * 900);
				pExt->DockReloadTimers[i] = Unsorted::CurrentFrame + frames;

				// only reload if the timer was not outdated
				if (last_timer != Unsorted::CurrentFrame)
				{
					continue;
				}

				// reload and repair, return true if both failed
				done = !SendCommand(RadioCommand::RequestReload)
					&& !SendCommand(RadioCommand::RequestRepair);
			}

			if (done)
			{
				pLink->EnterIdleMode(false, 1);
				pLink->ForceMission(Mission::Guard);
				pLink->ProceedToNextPlanningWaypoint();

				pExt->DockReloadTimers[i] = -1;
			}
		}
	}

	if (keep_reloading)
	{
		// update each frame
		R->EAX(1);
	}
	else
	{
		pThis->QueueMission(Mission::Guard, false);
		R->EAX(3);
	}

	return 0x44C968;
}

DEFINE_OVERRIDE_HOOK(0x621B80, DSurface_FillRectWithColor, 5)
{
	GET(RectangleStruct*, rect, ECX);
	GET(Surface*, surface, EDX);

	int surfaceWidth = surface->Get_Width();
	int surfaceHeight = surface->Get_Height();

	//make sure the rectangle to fill is within the surface's boundaries, this should do the trick
	rect->X = (rect->X >= 0) ? rect->X : 0;
	rect->Y = (rect->Y >= 0) ? rect->Y : 0;
	rect->Width = (rect->X + rect->Width <= surfaceWidth) ? rect->Width : surfaceWidth - rect->X;
	rect->Height = (rect->Y + rect->Height <= surfaceHeight) ? rect->Height : surfaceHeight - rect->Y;

	if (rect->Width == 0 || rect->Height == 0)
		return 0x621D26;
	else
		return 0;
}


DEFINE_OVERRIDE_HOOK(0x4ABFBE, DisplayClass_LeftMouseButtonUp_ExecPowerToggle, 7)
{
	GET(TechnoClass*, Target, ESI);
	return (Target && Target->Owner->IsControlledByHuman_() && Target->WhatAmI() == AbstractType::Building)
		? 0x4ABFCE
		: 0x4AC294
		;
}

DEFINE_OVERRIDE_HOOK(0x52E9AA, Frontend_WndProc_Checksum, 5)
{
	if (SessionClass::Instance->GameMode == GameMode::LAN || SessionClass::Instance->GameMode == GameMode::Internet)
	{
		auto nHashes = HashData::GetINIChecksums();
		Debug::Log("Rules checksum: %08X\n", nHashes.Rules);
		Debug::Log("Art checksum: %08X\n", nHashes.Art);
		Debug::Log("AI checksum: %08X\n", nHashes.AI);
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x480534, CellClass_AttachesToNeighbourOverlay, 5)
{
	GET(int, idxOverlay, EAX);
	bool Wall = idxOverlay != -1 && OverlayTypeClass::Array->GetItem(idxOverlay)->Wall;
	return Wall ? 0x480549 : 0x480552;
}

DEFINE_OVERRIDE_HOOK(0x4A76ED, DiskLaserClass_Update_Anim, 7)
{
	GET(DiskLaserClass* const, pThis, ESI);
	REF_STACK(CoordStruct, coords, STACK_OFFS(0x54, 0x1C));

	auto const pWarhead = pThis->Weapon->Warhead;

	if (RulesExt::Global()->DiskLaserAnimEnabled)
	{
		auto const pType = MapClass::SelectDamageAnimation(
			pThis->Damage, pWarhead, LandType::Clear, coords);

		if (pType) {
			AnimExt::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pType, coords),
				pThis->Owner->Owner,
				pThis->Target ? pThis->Target->Owner : nullptr,
				false,
				pThis->Owner
			);
		}
	}

	MapClass::FlashbangWarheadAt(pThis->Damage, pWarhead, coords);

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x48a4f9, SelectDamageAnimation_FixNegatives, 6)
{
	GET(int, Damage, EDI);
	Damage = abs(Damage);
	R->EDI(Damage);
	return Damage ? 0x48A4FF : 0x48A618;
}

// drain affecting only the drained power plant
DEFINE_OVERRIDE_HOOK(0x508D32, HouseClass_UpdatePower_LocalDrain1, 5)
{
	GET(HouseClass*, pThis, ESI);
	GET(BuildingClass*, pBld, EDI);

	bool fullDrain = true;

	auto output = pBld->GetPowerOutput();

	if (output > 0)
	{
		auto pBldTypeExt = TechnoTypeExt::ExtMap.Find(pBld->Type);
		auto pDrainTypeExt = TechnoTypeExt::ExtMap.Find(pBld->DrainingMe->GetTechnoType());

		// local, if any of the participants in the drain is local
		if (pBldTypeExt->Drain_Local || pDrainTypeExt->Drain_Local)
		{
			fullDrain = false;

			// use the sign to select min or max.
			// 0 means no change (maximum of 0 and a positive value)
			auto limit = [](int value, int limit)
			{
				if (limit <= 0)
				{
					return MaxImpl(value, -limit);
				}
				else
				{
					return MinImpl(value, limit);
				}
			};

			// drains the entire output of this building by default
			// (the local output). building has the last word though.
			auto drain = limit(output, pDrainTypeExt->Drain_Amount);
			drain = limit(drain, pBldTypeExt->Drain_Amount);

			if (drain > 0)
			{
				pThis->PowerOutput -= drain;
			}
		}
	}

	return fullDrain ? 0 : 0x508D37;
}

// replaced the entire function, to have one centralized implementation
DEFINE_OVERRIDE_HOOK(0x5051E0, HouseClass_FirstBuildableFromArray, 5)
{
	GET(HouseClass* , pThis, ECX);
	GET_STACK(const DynamicVectorClass<BuildingTypeClass*>*const, pList, 0x4);
	R->EAX(HouseExt::FindBuildable(pThis, pThis->Type->FindParentCountryIndex(), make_iterator(*pList)));
	return 0x505300;
}

//InitGame_Delay
DEFINE_JUMP(LJMP, 0x52CA37, 0x52CA65)

DEFINE_OVERRIDE_HOOK(0x51E5E1, InfantryClass_GetActionOnObject_MultiEngineerB, 7)
{
	GET(BuildingClass*, pBld, ECX);
	Action ret = TechnoExt_ExtData::GetEngineerEnterEnemyBuildingAction(pBld);

	// use a dedicated cursor
	if (ret == Action::Damage) {
		MouseCursorFuncs::SetMouseCursorAction(RulesExt::Global()->EngineerDamageCursor, Action::Damage, false);
	}

	// return our action
	R->EAX(ret);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x519D9C, InfantryClass_UpdatePosition_MultiEngineer, 5)
{
	GET(InfantryClass*, pEngi, ESI);
	GET(BuildingClass*, pBld, EDI);

	// damage or capture
	Action action = TechnoExt_ExtData::GetEngineerEnterEnemyBuildingAction(pBld);

	if (action == Action::Damage)
	{
		int Damage = int(std::ceil(pBld->Type->Strength * RulesExt::Global()->EngineerDamage));
		pBld->ReceiveDamage(&Damage, 0, RulesClass::Global()->C4Warhead, pEngi, true, false, pEngi->Owner);
		return 0x51A010;
	}

	return 0x519EAA;
}
#include <MPGameModeClass.h>

//Wrong register ?
// game crash here , ugh
DEFINE_OVERRIDE_HOOK(0x5D6F61, MPGameModeClass_CreateStartingUnits_BaseCenter, 8)
{
	GET(MPGameModeClass*, pMode, ECX);
	GET(HouseClass*, pHouse, ESI);
	GET(int*, AmountToSpend, EAX);

	*AmountToSpend = R->EBP<int>();
	CellStruct nBase = pHouse->BaseSpawnCell;

	if(!pMode->SpawnBaseUnits(pHouse, AmountToSpend))
		return 0x5D701B;

	pHouse->ConYards.for_each([](BuildingClass* pConyards) {
		pConyards->QueueMission(Mission::Construction, true);
		++Unsorted::ScenarioInit();
		pConyards->EnterIdleMode(false, 1);
		--Unsorted::ScenarioInit();
	});

	if (!nBase.IsValid())
		pHouse->BaseSpawnCell = nBase;

	return 0x5D6F77;
}

DEFINE_OVERRIDE_HOOK(0x687C56, INIClass_ReadScenario_ResetLogStatus, 5)
{
	// reset this so next scenario startup log is cleaner
	Debug_bTrackParseErrors = false;
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x5f5add, ObjectClass_SpawnParachuted_Animation, 6)
{
	GET(ObjectClass*, pThis, ESI);

	if (const auto pTechno = generic_cast<TechnoClass*>(pThis))
	{
		auto pType = pTechno->GetTechnoType();
		auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

		if (pTypeExt->IsBomb)
			pThis->IsABomb = true;

		R->EDX(pTypeExt->ParachuteAnim ? pTypeExt->ParachuteAnim : HouseExt::GetParachuteAnim(pTechno->Owner));
		return 0x5F5AE3;
	}

	return 0x0;
}

DEFINE_OVERRIDE_HOOK(0x444DBC, BuildingClass_KickOutUnit_Infantry, 5)
{
	GET(TechnoClass*, Production, EDI);
	GET(BuildingClass*, Factory, ESI);

	// turn it off
	--Unsorted::ScenarioInit;

	TechnoExt_ExtData::KickOutClones(Factory, Production);

	// turn it back on so the game can turn it off again
	++Unsorted::ScenarioInit;

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x4445F6, BuildingClass_KickOutUnit_Clone_NonNavalUnit, 5)
{
	GET(TechnoClass*, Production, EDI);
	GET(BuildingClass*, Factory, ESI);

	// turn it off
	--Unsorted::ScenarioInit;

	TechnoExt_ExtData::KickOutClones(Factory, Production);

	// turn it back on so the game can turn it off again
	++Unsorted::ScenarioInit;

	return 0x444971;
}

DEFINE_OVERRIDE_HOOK(0x44441A, BuildingClass_KickOutUnit_Clone_NavalUnit, 6)
{
	GET(TechnoClass*, Production, EDI);
	GET(BuildingClass*, Factory, ESI);

	TechnoExt_ExtData::KickOutClones(Factory, Production);

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x4444E2, BuildingClass_KickOutUnit_FindAlternateKickout, 6)
{
	GET(BuildingClass*, Src, ESI);
	GET(BuildingClass*, Tst, EBP);
	GET(TechnoClass*, Production, EDI);

	if (Src != Tst
	 && Tst->GetCurrentMission() == Mission::Guard
	 && Tst->Type->Factory == Src->Type->Factory
	 && Tst->Type->Naval == Src->Type->Naval
	 && TechnoTypeExt::CanBeBuiltAt(Production->GetTechnoType() , Tst->Type)
	 && !Tst->Factory)
	{
		return 0x44451F;
	}

	return 0x444508;
}

DEFINE_OVERRIDE_HOOK(0x6FAF0D, TechnoClass_Update_EMPLock, 6)
{
	GET(TechnoClass*, pThis, ESI);

	// original code.
	if (pThis->EMPLockRemaining)
	{
		--pThis->EMPLockRemaining;
		if (!pThis->EMPLockRemaining)
		{
			// the forced vacation just ended. we use our own
			// function here that is quicker in retrieving the
			// EMP animation and does more stuff.
			AresEMPulse::DisableEMPEffect(pThis);
		}
		else
		{
			// deactivate units that were unloading afterwards
			if (!pThis->Deactivated && AresEMPulse::IsDeactivationAdvisable(pThis))
			{
				// update the current mission
				pThis->align_154->EMPLastMission = pThis->CurrentMission;
				pThis->Deactivate();
			}
		}
	}

	return 0x6FAFFD;
}

// copy the remaining EMP duration to the unit when undeploying a building.
DEFINE_OVERRIDE_HOOK(0x44A04C, BuildingClass_Unload_CopyEMPDuration, 6)
{
	GET(TechnoClass*, pBuilding, EBP);
	GET(TechnoClass*, pUnit, EBX);

	// reuse the EMP duration of the deployed/undeployed Techno.
	pUnit->EMPLockRemaining = pBuilding->EMPLockRemaining;
	AresEMPulse::UpdateSparkleAnim(pUnit, pBuilding);

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x5d7337, MPGameMode_SpawnStartingUnits_NoInfantry, 5)
{
	return R->Stack<int>(0x28) ? 0x0 : 0x5D734F;
}

DEFINE_OVERRIDE_HOOK(0x5D705E, MPGameMode_SpawnBaseUnit_BaseUnit, 6)
{
	enum { hasBaseUnit = 0x5D7064, hasNoBaseUnit = 0x5D70DB };

	GET(HouseClass*, pHouse, EDI);
	GET(UnitTypeClass*, pBaseUnit, EAX);
	R->ESI(pBaseUnit);

	if(pBaseUnit)
		return hasBaseUnit;

	Debug::Log("House of country [%s] cannot build anything from [General]BaseUnit=.\n", pHouse->Type->ID);
	return hasNoBaseUnit;
}

DEFINE_OVERRIDE_HOOK(0x4C850B, Exception_Dialog, 5) {
	Debug::FreeMouse();
	return 0;
}

DEFINE_HOOK(0x6F3F88, TechnoClass_Init_1, 5)
{
	GET(TechnoClass* const, pThis, ESI);
	auto const pType = pThis->GetTechnoType();
	auto const pData = pThis->align_154;

	CaptureManagerClass* pCapturer = nullptr;
	ParasiteClass* pParasite = nullptr;
	TemporalClass* pTemporal = nullptr;
	SpawnManagerClass* pSpawnManager = nullptr;
	SlaveManagerClass* pSlaveManager = nullptr;
	AirstrikeClass* pAirstrike = nullptr;

	if (pType->Spawns) {
		if (auto pSpawn = GameCreate<SpawnManagerClass>(pThis , pType->Spawns, pType->SpawnsNumber , pType->SpawnRegenRate , pType->SpawnReloadRate)) {
			pSpawnManager = pSpawn;
		}
	}

	if (pType->Enslaves) {
		if (auto pSlaveMan = GameCreate<SlaveManagerClass>(pThis, pType->Enslaves, pType->SlavesNumber, pType->SlaveRegenRate, pType->SlaveReloadRate)) {
			pSlaveManager = pSlaveMan;
		}
	}

	if (pType->AirstrikeTeam > 0 && pType->AirstrikeTeamType) {
		if(auto pAir = GameCreate<AirstrikeClass>(pThis)) {
			pAirstrike = pAir;
		}
	}

	const bool IsFoot = pThis->WhatAmI() != BuildingClass::AbsID;

	const int WeaponCount = pType->TurretCount <= 0  ? 2 : pType->WeaponCount;

	for (auto i = 0; i < WeaponCount; ++i) {

		if (auto const pWeapon = pType->GetWeapon(i)->WeaponType) {
			TechnoExt_ExtData::InitWeapon(pThis, pType, pWeapon, i, pCapturer , pParasite , pTemporal , "Weapon" , IsFoot);
		}

		if (auto const pWeaponE = pType->GetEliteWeapon(i)->WeaponType) {
				TechnoExt_ExtData::InitWeapon(pThis, pType, pWeaponE, i, pCapturer, pParasite, pTemporal, "EliteWeapon", IsFoot);
		}
	}

	pThis->CaptureManager = pCapturer;
	pThis->TemporalImUsing = pTemporal;
	if (IsFoot) {
		((FootClass*)pThis)->ParasiteImUsing = pParasite;
	}
	pThis->SpawnManager = pSpawnManager;
	pThis->SlaveManager = pSlaveManager;
	pThis->Airstrike = pAirstrike;

	if(auto pOwner = pThis->Owner){
		const auto pHouseType = pOwner->Type;
		const auto pParentHouseType = pHouseType->FindParentCountry();
		pData->OriginalHouseType = pParentHouseType ? pParentHouseType : pHouseType;
	}
	else {
		Debug::Log("Techno[%s] Init Without any ownership!\n", pType->ID);
	}
	// if override is in effect, do not create initial payload.
	// this object might have been deployed, undeployed, ...
	if (Unsorted::ScenarioInit && Unsorted::CurrentFrame) {
		TechnoExt::ExtMap.Find(pThis)->PayloadCreated = true;
	}

	R->EAX(pType);
	return 0x6F4212;
}

DEFINE_DISABLE_HOOK(0x4E3792, HTExt_Unlimit1_ares)//, 0){ return 0x4E37AD; }
DEFINE_JUMP(LJMP, 0x4E3792, 0x4E37AD);

DEFINE_DISABLE_HOOK(0x4E3A9C, HTExt_Unlimit2_ares)//, 0){ return 0x4E3AA1; }
DEFINE_JUMP(LJMP, 0x4E3A9C, 0x4E3AA1);

DEFINE_DISABLE_HOOK(0x4E3F31, HTExt_Unlimit3_ares)//, 0){ return 0x4E3F4C; }
DEFINE_JUMP(LJMP, 0x4E3F31, 0x4E3F4C);

DEFINE_DISABLE_HOOK(0x4E412C, HTExt_Unlimit4_ares)//, 0){ return 0x4E4147; }
DEFINE_JUMP(LJMP, 0x4E412C, 0x4E4147);

DEFINE_DISABLE_HOOK(0x4E41A7, HTExt_Unlimit5_ares)//, 0){ return 0x4E41C3; }
DEFINE_JUMP(LJMP, 0x4E41A7, 0x4E41C3);

DEFINE_DISABLE_HOOK(0x55afb3, LogicClass_Update_ares)

//also hook with `RulesData_LoadAfterTypeData` for ares
DEFINE_DISABLE_HOOK(0x679caf , RulesClass_LoadAfterTypeData_CompleteInitialization_ares)
DEFINE_OVERRIDE_HOOK(0x5d7048, MPGameMode_SpawnBaseUnit_BuildConst, 5)
{
	GET_STACK(HouseClass*, pHouse, 0x18);

	auto pHouseType = pHouse->Type;

	if (!HouseTypeExt::ExtMap.Find(pHouseType)->StartInMultiplayer_WithConst)
		return 0;

	auto idxParentCountry = HouseClass::FindIndexByName(pHouseType->ParentCountry);
	const auto v7 = HouseExt::FindBuildable(pHouse, idxParentCountry, make_iterator(RulesClass::Instance->BuildConst), 0);

	if (!v7) {
		Debug::Log("House of country [%s] cannot build anything from [General]BuildConst=.\n", pHouse->Type->ID);
		return 0x5D70DB;
	}

	const auto pBld = (BuildingClass*)v7->CreateObject(pHouse);

	if (!pBld)
		return 0x5D70DB;

	pBld->ForceMission(Mission::Guard);

	if (v7->GetFoundationWidth() > 2 || v7->GetFoundationHeight(0) > 2)
	{
		--pHouse->BaseSpawnCell.X;
		--pHouse->BaseSpawnCell.Y;
	}

	if (!pHouse->IsControlledByHuman_())
	{
		pHouse->Func_505180();
		CellStruct base = pHouse->GetBaseCenter();

		pHouse->Base.Center = base;
		pHouse->Base.BaseNodes.Items->MapCoords = base;
		pHouse->Production = 1;
		pHouse->AITriggersActive = true;
	}

	R->EAX(pBld);
	R->EDI(pHouse);
	return 0x5D707E;
}

DEFINE_DISABLE_HOOK(0x62CDE8, ParticleClass_Update_Fire_ares) //, 5)
DEFINE_DISABLE_HOOK(0x62C2ED, ParticleClass_Update_Gas_ares) //, 6)
