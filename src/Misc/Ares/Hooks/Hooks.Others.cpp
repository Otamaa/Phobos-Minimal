#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
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

#include <TerrainTypeClass.h>
#include <Locomotor/HoverLocomotionClass.h>
#include <New/Type/ArmorTypeClass.h>

#include <Misc/PhobosGlobal.h>

#include <Notifications.h>
#include <strsafe.h>

DEFINE_OVERRIDE_HOOK(0x4CA0E3, FactoryClass_AbandonProduction_Invalidate, 0x6)
{
	GET(FactoryClass*, pThis, ESI);

	if (pThis->Owner == HouseClass::CurrentPlayer() && pThis->Object)
	{
		if (Is_Building(pThis->Object))
			pThis->Object->RemoveSidebarObject();
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x420A71, AlphaShapeClass_CTOR_Anims, 0x5)
{
	GET(AlphaShapeClass*, pThis, ESI);

	if (Is_Anim(pThis->AttachedTo))
	{
		PointerExpiredNotification::NotifyInvalidAnim->Add(pThis);
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x421798, AlphaShapeClass_SDDTOR_Anims, 0x6)
{
	GET(AlphaShapeClass*, pThis, ESI);
	PointerExpiredNotification::NotifyInvalidAnim->Remove(pThis);
	return 0;
}

DEFINE_OVERRIDE_SKIP_HOOK(0x565215, MapClass_CTOR_NoInit_Crates, 0x6, 56522D)

DEFINE_HOOK(0x5F6500 , AbstractClass_Distance2DSquared_1 , 0x8)
{
	GET(AbstractClass* , pThis ,ECX);
	GET_STACK(AbstractClass* , pThat , 0x4);

	int nResult = 0;
	if(pThat) {
	  const auto nThisCoord = pThis->GetCoords();
	  const auto nThatCoord = pThat->GetCoords();
	  const auto nY = (int64_t(nThisCoord.Y - nThatCoord.Y) * int64_t(nThisCoord.Y - nThatCoord.Y));
	  const auto nX = (int64_t(nThisCoord.X - nThatCoord.X) * int64_t(nThisCoord.X - nThatCoord.X));
	  const auto nXY = int(nX + nY);

	  nResult = (nXY >= INT_MAX ? INT_MAX : nXY);
	}

	R->EAX(nResult);
	return 0x5F655D;
}

// DEFINE_OVERRIDE_HOOK(0x5F6515, AbstractClass_Distance2DSquared_1, 0x8)
// {
// 	GET(AbstractClass*, pThis, ECX);
// 	GET(AbstractClass*, pThat, EBX);
//
// 	auto const nThisCoord = pThis->GetCoords();
// 	auto const nThatCoord = pThat->GetCoords();
// 	const auto nY = (int64_t(nThisCoord.Y - nThatCoord.Y) * int64_t(nThisCoord.Y - nThatCoord.Y));
// 	const auto nX = (int64_t(nThisCoord.X - nThatCoord.X) * int64_t(nThisCoord.X - nThatCoord.X));
// 	const auto nXY = nX + nY;
//
// 	R->EAX(nXY >= INT_MAX ? INT_MAX : nXY);
// 	return 0x5F6559;
// }

DEFINE_OVERRIDE_HOOK(0x5F6560, AbstractClass_Distance2DSquared_2, 5)
{
	GET(AbstractClass*, pThis, ECX);
	auto const nThisCoord = pThis->GetCoords();
	GET_STACK(CoordStruct*, pThatCoord, 0x4);

	const auto nY = (int64_t(nThisCoord.Y - pThatCoord->Y) * int64_t(nThisCoord.Y - pThatCoord->Y));
	const auto nX = (int64_t(nThisCoord.X - pThatCoord->X) * int64_t(nThisCoord.X - pThatCoord->X));
	const auto nXY = int(nX + nY);

	R->EAX(nXY >= INT_MAX ? INT_MAX : nXY);
	return 0x5F659B;
}

DEFINE_HOOK(0x6E2290, ActionClass_PlayAnimAt, 0x6)
{
	GET(TActionClass*, pThis, ECX);
	GET_STACK(HouseClass*, pOwner, 0x4);
	//GET_STACK(TechnoClass*, pInvoker, 0x8);
	//GET_STACK(TriggerClass*, pTrigger, 0xC);
	//GET_STACK(CellStruct*, pCell, 0x10);

	auto nCell = ScenarioClass::Instance->GetWaypointCoords(pThis->Waypoint);
	auto nCoord = CellClass::Cell2Coord(nCell);
	nCoord.Z = MapClass::Instance->GetZPos(&nCoord);
	auto pCellTarget = MapClass::Instance->GetCellAt(nCell);
	nCoord = pCellTarget->GetCoordsWithBridge();

	if (AnimTypeClass* AnimType = AnimTypeClass::Array->GetItemOrDefault(pThis->Value))
	{
		if (AnimClass* pAnim = GameCreate<AnimClass>(AnimType, nCoord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false))
		{
			pAnim->IsPlaying = true;
			AnimExt::SetAnimOwnerHouseKind(pAnim, pOwner, nullptr, false);
		}
	}

	R->EAX(1);
	return 0x6E2387;
}

DEFINE_OVERRIDE_HOOK(0x4892BE, DamageArea_NullDamage, 0x6)
{
	enum
	{
		DeleteDamageAreaVector = 0x48A4B7,
		ContinueFunction = 0x4892DD,
	};

	GET_BASE(WarheadTypeClass*, pWarhead, 0xC);
	GET(int, Damage, ESI);

	if (!pWarhead
		|| ((ScenarioClass::Instance->SpecialFlags.RawFlags & 0x20) != 0)
		|| !Damage && !WarheadTypeExt::ExtMap.Find(pWarhead)->AllowZeroDamage)
		return DeleteDamageAreaVector;

	R->ESI(pWarhead);
	return ContinueFunction;
}

DEFINE_OVERRIDE_SKIP_HOOK(0x6AD0ED, Game_AllowSinglePlay, 0x5, 6AD16C);

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
DEFINE_OVERRIDE_SKIP_HOOK(0x55CFDF, CopyProtection_DontBlowMeUp, 0x7, 55D059);

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
		if (!isdigit(Phobos::readBuffer[0]))
		{
			// find the pip value with the name specified
			const auto it = std::find_if(TechnoTypeClass::PipsTypeName.begin(), TechnoTypeClass::PipsTypeName.end(),
				[](NamedValue<int>const& Data)
				{
					return Data == Phobos::readBuffer;
				});

			if (it != TechnoTypeClass::PipsTypeName.end())
			{
				R->EAX(it->Value);
				return 0x474907;
			}
		}
		else
		{
			// find the pip value with the number
			const auto it = std::find_if(TechnoTypeClass::PipsTypeName.begin(), TechnoTypeClass::PipsTypeName.end(),
				[](NamedValue<int>const& Data)
				{
					return Data == std::atoi(Phobos::readBuffer);
				});

			if (it != TechnoTypeClass::PipsTypeName.end())
			{
				R->EAX(it->Value);
				return 0x474907;
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
DEFINE_OVERRIDE_SKIP_HOOK(0x483BF1, CellClass_Load_Crates, 0x7, 483BFE)

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

// create enumerator
DEFINE_OVERRIDE_HOOK(0x4895B8, DamageArea_CellSpread1, 0x6)
{
	REF_STACK(CellSpreadEnumerator*, pIter, STACK_OFFS(0xE0, 0xB4));
	GET(int, spread, EAX);

	pIter = nullptr;

	if (spread >= 0)
	{
		pIter = new CellSpreadEnumerator(spread);
	}

	return (pIter && *pIter) ? 0x4895C3 : 0x4899DA;
}

// apply the current value
DEFINE_OVERRIDE_HOOK(0x4895C7, DamageArea_CellSpread2, 0x8)
{
	GET_STACK(CellSpreadEnumerator*, pIter, STACK_OFFS(0xE0, 0xB4));

	auto& offset = **pIter;
	R->DX(offset.X);
	R->AX(offset.Y);

	return 0x4895D7;
}

// advance and delete if done
DEFINE_OVERRIDE_HOOK(0x4899BE, DamageArea_CellSpread3, 0x8)
{
	REF_STACK(CellSpreadEnumerator*, pIter, STACK_OFFS(0xE0, 0xB4));
	REF_STACK(int, index, STACK_OFFS(0xE0, 0xD0));

	// reproduce skipped instruction
	index++;

	// advance iterator
	if (++*pIter)
	{
		return 0x4895C0;
	}

	// all done. delete and go on
	delete pIter;

	return 0x4899DA;
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
	auto const pWhat = GetVtableAddr(Owner);

	bool allowed = false;
	if (pWhat == UnitClass::vtable)
	{
		allowed = !Owner->GetTechnoType()->Naval;
	}
	else if (pWhat == InfantryClass::vtable)
	{
		allowed = true;
	}

	if (allowed && Owner->GetHeight() > 200)
	{
		*XYZ = Owner->Location;
		Owner->IsFallingDown = Owner->IsABomb = true;
	}

	return 0;
}

DEFINE_OVERRIDE_SKIP_HOOK(0x6BB9DD, WinMain_LogClassSizes, 5, 6BBE2B)

// bugfix #187: Westwood idiocy
//DEFINE_OVERRIDE_SKIP_HOOK(0x5F698F, ObjectClass_GetCell, 5, 5F69B2)
DEFINE_HOOK(0x5F6960, ObjectClass_Getcell, 0xA)
{
	GET(ObjectClass*, pThis, ECX);
	R->EAX(MapClass::Instance->GetCellAt(pThis->Location));
	return 0x5F69B6;
}

// skip theme log lines
DEFINE_OVERRIDE_HOOK_AGAIN(0x720C42, Theme_Stop_NoLog, 0x5) // skip Theme::Stop
DEFINE_OVERRIDE_HOOK(0x720DE8, Theme_Stop_NoLog, 0x5) // skip Theme::PlaySong
{
	return R->Origin() + 5;
}

DEFINE_OVERRIDE_SKIP_HOOK(0x720F37, skip_Theme_Stop, 0x5, 720F3C)
DEFINE_OVERRIDE_SKIP_HOOK(0x720A61, skip_Theme_AI, 0x5, 720A66)
DEFINE_OVERRIDE_SKIP_HOOK(0x615BD3, Handle_Static_Messages_LoopingMovie, 0x5, 615BE0)
DEFINE_OVERRIDE_SKIP_HOOK(0x78997B, sub_789960_RemoveWOLResolutionCheck, 0x5, 789A58)
DEFINE_OVERRIDE_SKIP_HOOK(0x4BA61B, DSurface_CTOR_SkipVRAM, 0x6, 4BA623)

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

DEFINE_OVERRIDE_HOOK(0x547043, IsometricTileTypeClass_ReadFromFile, 0x6)
{
	GET(int, FileSize, EBX);
	GET(IsometricTileTypeClass*, pTileType, ESI);

	if (FileSize == 0)
	{
		const char* tile = pTileType->ID;
		if (strlen(tile) > 9)
		{
			Debug::FatalErrorAndExit("Maximum allowed length for tile names, excluding the extension, is 9 characters.\n"
					"The tileset using filename '%s' exceeds this limit - the game cannot proceed.", tile);
		}
		else
		{
			Debug::FatalErrorAndExit("The tileset '%s' contains a file that could not be loaded for some reason - make sure the file exists.", tile);
		}
	}
	return 0;
}

// skip the entire method, we handle it ourselves
DEFINE_OVERRIDE_SKIP_HOOK(0x53AF40, PsyDom_Update, 6, 53B060)

DEFINE_HOOK(0x65EA43, SendReinforcement_Opentopped, 0x6)
{
	GET(AircraftClass*, pPlane, ESI);
	GET(FootClass*, pPassenger, EDI);

	if (pPlane->Type->OpenTopped)
		pPlane->EnteredOpenTopped(pPassenger);

	pPassenger->Transporter = pPlane;

	return 0x0;
}

// issue #1282: remap wall using its owner's colors
DEFINE_OVERRIDE_HOOK(0x47F9A4, CellClass_DrawOverlay_WallRemap, 0x6)
{
	GET(CellClass*, pCell, ESI);

	const int idx = pCell->WallOwnerIndex;

	if (idx >= 0)
	{
		R->EDX(HouseClass::Array->GetItem(idx));
		return 0x47F9AA;
	}
	return 0;
}

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

	if (Is_Building(pVictim))
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
		|| Is_Building(pOwner))
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

		if (auto pSys = pThis->ParticleSystem) {
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

DEFINE_OVERRIDE_HOOK(0x7BB445, XSurface_20, 0x6)
{
	return R->EAX<void*>() ? 0x0 : 0x7BB90C;
}

DEFINE_OVERRIDE_HOOK(0x6FF1FB, TechnoClass_Fire_DetachedRailgun, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);

	const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	const bool IsRailgun = pWeapon->IsRailgun || pWeaponExt->IsDetachedRailgun;

	if (IsRailgun && Is_Aircraft(pThis))
	{
		Debug::Log("TechnoClass_FireAt Aircraft[%s] attempting to fire Railgun !\n", pThis->get_ID());
		//return 0x6FF274;
	}

	return pWeaponExt->IsDetachedRailgun
		? 0x6FF20F : 0x0;
}

DEFINE_OVERRIDE_HOOK(0x6FF26E, TechnoClass_Fire_DetachedRailgun2, 0x6)
{
	GET(WeaponTypeClass*, pWeapon, EBX);

	return WeaponTypeExt::ExtMap.Find(pWeapon)->IsDetachedRailgun
		? 0x6FF274 : 0x0;
}

#include <TeamClass.h>
#include <Ext/TeamType/Body.h>
#include <format>

DEFINE_OVERRIDE_HOOK(0x6EF8A1, TeamClass_GatherAtEnemyBase_Distance, 0x6)
{
	//GET_STACK(TeamClass*, pTeam, STACK_OFFS(0x5C, 0x34));
	GET_BASE(ScriptActionNode*, pTeamM, 0x8);
	//const auto pTeamExt = TeamTypeExt::ExtMap.Find(pTeam->Type);
	//Debug::Log(std::format(__FUNCTION__ " Function With Type {} ! \n",pTeam->Type->ID));
	//R->EDX(pTeamExt->AI_SafeDIstance.Get(RulesClass::Instance->AISafeDistance) + pTeamM->Argument);
	R->EDX(RulesClass::Instance->AISafeDistance + pTeamM->Argument);

	return 0x6EF8A7;
}

DEFINE_OVERRIDE_HOOK(0x6EFB69, TeamClass_GatherAtFriendlyBase_Distance, 0x6)
{
	//GET_STACK(TeamClass*, pTeam, STACK_OFFS(0x4C, 0x2C));
	GET_BASE(ScriptActionNode*, pTeamM, 0x8);
	//Debug::Log("%s", std::format("{} Function With Type {} ! \n", __FUNCTION__, pTeam->Type->ID).c_str());
	//const auto pTeamExt = TeamTypeExt::ExtMap.Find(pTeam->Type);
	//R->EDX(pTeamExt->AI_FriendlyDistance.Get(RulesExt::Global()->AIFriendlyDistance.Get(RulesClass::Instance->AISafeDistance)) + pTeamM->Argument);
	R->EDX(RulesExt::Global()->AIFriendlyDistance.Get(RulesClass::Instance->AISafeDistance) + pTeamM->Argument);
	return 0x6EFB6F;
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

	if (pThis->InLimbo)
		return 0x0;

	if (auto const pOwner = pThis->SlaveOwner)
	{
		if (!pOwner->IsSelected)
			return 0x0;

		Drawing::DrawLinesTo(pOwner->GetRenderCoords(), pThis->Location, pOwner->Owner->Color);
	}

	if (auto const pOwner = pThis->SpawnOwner)
	{
		if (!pOwner->IsSelected)
			return 0x0;

		Drawing::DrawLinesTo(pOwner->GetRenderCoords(), pThis->Location, pOwner->Owner->Color);
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

DEFINE_OVERRIDE_HOOK(0x6CC390, SuperClass_Launch, 0x6)
{
	GET(SuperClass* const, pSuper, ECX);
	GET_STACK(CellStruct* const, pCell, 0x4);
	GET_STACK(bool const, isPlayer, 0x8);

	auto pSuperExt = SuperExt::ExtMap.Find(pSuper);

	pSuperExt->Temp_IsPlayer = isPlayer;
	pSuperExt->Temp_CellStruct = *pCell;

	if (//AresData::SW_Activate(pSuper, *pCell, isPlayer)
		SWTypeExt::ExtData::Activate(pSuper,*pCell , isPlayer)
		)
	{
		Debug::Log("[LAUNCH] %s Handled\n", pSuper->Type->ID);
		const auto pSWExt = SWTypeExt::ExtMap.Find(pSuper->Type);

		//auto pHouseExt = HouseExt::ExtMap.Find(pSuper->Owner);
		//auto pAresExt = AresData::Ares_SWType_ExtMap_Find(pSuper->Type);
		pSWExt->FireSuperWeapon(pSuper, pSuper->Owner, pCell, isPlayer);
		return 0x6CDE40;
	}
	else
	{
		SWTypeExt::LauchData = pSuper;
	}

	return 0x0;
}

DEFINE_HOOK(0x6CDE36, SuperClass_Place_FireNormal, 0xA)
{
	if (const auto pSuper = SWTypeExt::LauchData)
	{
		Debug::Log("[LAUNCH] %s Normal at [0x%x]\n", pSuper->Type->ID, R->Origin());
		const auto pSuperExt = SuperExt::ExtMap.Find(pSuper);
		const auto pSWExt = SWTypeExt::ExtMap.Find(pSuper->Type);
		pSWExt->FireSuperWeapon(pSuper, pSuper->Owner, &pSuperExt->Temp_CellStruct, pSuperExt->Temp_IsPlayer);
		SWTypeExt::LauchData = nullptr;
	}
	else
	{
		Debug::Log("[LAUNCH] SW as Normal but SuperPointer is somewhat invalid at [0x%x]!\n", R->Origin());
	}

	return 0x0;
}

DEFINE_OVERRIDE_HOOK(0x48A2D9, MapClass_DamageArea_ExplodesThreshold, 6)
{
	GET(OverlayTypeClass*, pOverlay, EAX);
	GET_STACK(int, damage, 0x24);

	const bool explodes = pOverlay->Explodes && damage >= RulesExt::Global()->OverlayExplodeThreshold;

	return explodes ? 0x48A2E7 : 0x48A433;
}

DEFINE_OVERRIDE_HOOK(0x738749, UnitClass_Destroy_TiberiumExplosive, 6)
{
	GET(UnitClass*, pThis, ESI);

	if (RulesClass::Instance->TiberiumExplosive)
	{
		if (!ScenarioClass::Instance->SpecialFlags.StructEd.HarvesterImmune)
		{
			if (pThis->Tiberium.GetTotalAmount() > 0.0f)
			{
				// multiply the amounts with their powers and sum them up
				int morePower = 0;
				for (int i = 0; i < TiberiumClass::Array->Count; ++i)
				{
					TiberiumClass* pTiberium = TiberiumClass::Array->GetItem(i);
					morePower += int(pThis->Tiberium.Tiberiums[i] * pTiberium->Power);
				}

				if (morePower > 0)
				{
					CoordStruct crd = pThis->GetCoords();
					if (auto pWH = RulesExt::Global()->Tiberium_ExplosiveWarhead)
					{
						MapClass::DamageArea(crd, morePower, pThis, pWH, pWH->Tiberium, pThis->Owner);
					}

					if (auto pAnim = RulesExt::Global()->Tiberium_ExplosiveAnim)
					{
						if (auto pA = GameCreate<AnimClass>(pAnim, crd, 0, 1, AnimFlag(0x2600), -15, false))
							AnimExt::SetAnimOwnerHouseKind(pA, pThis->Owner, nullptr, false);
					}
				}
			}
		}
	}

	return 0x7387C4;
}

#include <Ext/Tiberium/Body.h>

bool SpawnVisceroid(CoordStruct& crd, ObjectTypeClass* pType, int chance, bool ignoreTibDeathToVisc)
{
	bool ret = false;
	// create a small visceroid if available and the cell is free
	if (ignoreTibDeathToVisc || ScenarioClass::Instance->TiberiumDeathToVisceroid)
	{
		const auto pCell = MapClass::Instance->GetCellAt(crd);

		if (!(pCell->OccupationFlags & 0x20) && ScenarioClass::Instance->Random.RandomFromMax(99) < chance && pType)
		{
			if (const auto pHouse = HouseExt::FindNeutral())
			{
				if (auto pVisc = pType->CreateObject(pHouse))
				{
					++Unsorted::IKnowWhatImDoing;
					ret = true;
					if (!pVisc->Unlimbo(crd, DirType(0)))
					{
						// opposed to TS, we clean up, though
						// the mutex should make it happen.
						GameDelete(pVisc);
						ret = false;
					}
					--Unsorted::IKnowWhatImDoing;
				}
			}
		}
	}
	return ret;
}

// damage the techno when it is moving over a cell containing tiberium
DEFINE_OVERRIDE_HOOK(0x4D85E4, FootClass_UpdatePosition_TiberiumDamage, 9)
{
	GET(FootClass*, pThis, ESI);

	int damage = 0;
	WarheadTypeClass* pWarhead = nullptr;
	int transmogrify = RulesClass::Instance->TiberiumTransmogrify;

	if (RulesExt::Global()->Tiberium_DamageEnabled && pThis->GetHeight() <= RulesClass::Instance->HoverHeight)
	{
		TechnoTypeClass* pType = pThis->GetTechnoType();
		TechnoTypeExt::ExtData* pExt = TechnoTypeExt::ExtMap.Find(pType);

		// default is: infantry can be damaged, others cannot
		bool enabled = (pThis->WhatAmI() != InfantryClass::AbsID);

		if (!pExt->TiberiumProof.Get(enabled) && !pThis->HasAbility(AbilityType::TiberiumProof))
		{
			if (pThis->Health > 0)
			{
				if (auto pTiberium = TiberiumClass::Array->GetItemOrDefault(pThis->GetCell()->GetContainedTiberiumIndex()))
				{

					if (GetVtableAddr(pTiberium) != TiberiumClass::vtable)
					{
						Debug::Log("%x is Not A Tiberium\n", pTiberium);
						return 0x0;
					}

					auto pTibExt = TiberiumExt::ExtMap.Find(pTiberium);

					pWarhead = pTibExt->GetWarhead();
					damage = pTibExt->GetDamage();

					transmogrify = pExt->TiberiumTransmogrify.Get(transmogrify);
				}
			}
		}
	}

	//if(!TiberiumDamage)
	//{
	//	TechnoTypeClass* pType = pThis->GetTechnoType();
	//	if (pThis->Health > 0 && pThis->GetHeight() <= RulesClass::Instance->HoverHeight) {

	//		CellClass* pCell = pThis->GetCell();
	//		if(pCell->OverlayTypeIndex == 0x7E &&
	//			pCell->OverlayData >= 0x30u &&
	//			!pCell->Level &&
	//			(pCell->Flags & CellFlags(0x20000)) == CellFlags::Empty
	//			)
	//		{
	//			if(!pType->ImmuneToVeins && !pThis->HasAbility(AbilityType::VeinProof))
	//			{
	//				pWarhead = RulesClass::Instance->C4Warhead;
	//				damage = RulesClass::Instance->VeinDamage;
	//			}
	//		}
	//	}
	//}

	if (damage && pWarhead)
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

	auto idxTib = pCell->GetContainedTiberiumIndex();

	TiberiumClass* pTib = TiberiumClass::Array->GetItemOrDefault(idxTib);

	if (!pTib)
		return 0x0;

	OverlayTypeClass* pOverlay = OverlayTypeClass::Array->GetItemOrDefault(pCell->OverlayTypeIndex);

	if (!pOverlay || !pOverlay->ChainReaction)
		return 0x0;

	CoordStruct crd = pCell->GetCoords();

	if (ScenarioClass::Instance->Random.RandomRanged(0, 99) < (RulesExt::Global()->ChainReact_Multiplier * pCell->OverlayData))
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
		if (auto pType = AnimTypeClass::Find("INVISO"))
		{
			for (size_t i = 0; i < 8; ++i)
			{
				auto pNeighbour = pCell->GetNeighbourCell(i);

				if (pNeighbour->GetContainedTiberiumIndex() != -1 && pNeighbour->OverlayData > 2)
				{
					if (ScenarioClass::Instance->Random.RandomRanged(0, 99) < RulesExt::Global()->ChainReact_SpreadChance)
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

// hook up the area damage delivery with chain reactions
DEFINE_OVERRIDE_HOOK(0x48964F, DamageArea_CellChainReaction, 5)
{
	GET(CellClass*, pCell, EBX);
	pCell->ChainReaction();
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x424DD3, AnimClass_ReInit_TiberiumChainReaction_Chance, 6)
{
	GET(TiberiumClass*, pTib, EDI);
	auto pExt = TiberiumExt::ExtMap.Find(pTib);

	bool react = ScenarioClass::Instance->Random.RandomRanged(0, 99) < pExt->GetDebrisChance();
	return react ? 0x424DF9 : 0x424E9B;
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
			for (unsigned int i = 0; i < 8; ++i)
			{
				if (auto pTree = pCell->GetNeighbourCell(i)->GetTerrain(false))
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

DEFINE_OVERRIDE_SKIP_HOOK(0x715857, TechnoTypeClass_LoadFromINI_LimitPalettes, 5, 715876)

DEFINE_OVERRIDE_HOOK(0x6F47A0, TechnoClass_GetBuildTime, 5)
{
	GET(TechnoClass*, pThis, ECX);

	const auto pType = pThis->GetTechnoType();
	const auto  pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	double nFinalSpeed = 0.01;

	if (const auto pOwner = pThis->Owner)
	{
		double nSpeed = pType->GetBuildSpeed();

		{// Owner and type mult
			const auto nBuildMult = pOwner->GetBuildTimeMult(pType);
			nSpeed *= nBuildMult;

			const auto nBuildTimeMult = pType->BuildTimeMultiplier;
			nSpeed *= nBuildTimeMult;
		}

		double nPowerSpeedResult = 0.00;
		{//Power

			const auto nPowerPercentage = pOwner->GetPowerPercentage();
			const auto nLowPowerPenalty = pTypeExt->BuildTime_LowPowerPenalty.Get(RulesClass::Instance->LowPowerPenaltyModifier);
			const auto nMinLowPoweProductionSpeed = pTypeExt->BuildTime_MinLowPower.Get(RulesClass::Instance->MinLowPowerProductionSpeed);
			auto nMaxLowPowerProductionSpeed = pTypeExt->BuildTime_MaxLowPower.Get(RulesClass::Instance->MaxLowPowerProductionSpeed);
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
			const auto nFactorySpeed = pTypeExt->BuildTime_MultipleFactory.Get(RulesClass::Instance->MultipleFactory);
			if (nFactorySpeed > 0.0)
			{
				for (int i = (pOwner->FactoryCount(pThis->WhatAmI(), Is_Unit(pThis) ? pType->Naval : false) - 1);
					i > 0;
					--i)
				{
					nFinalSpeed *= nFactorySpeed;
				}
			}
		}

		nFinalSpeed *= BuildingTypeExt::GetExternalFactorySpeedBonus(pThis);
	}

	{ //Exception
		if (Is_Building(pThis) && !pTypeExt->BuildTime_Speed.isset() && static_cast<BuildingTypeClass*>(pType)->Wall)
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

		const auto frames = MaxImpl(static_cast<int>(rate * 900.0), 1);

		if (Unsorted::CurrentFrame % frames == 0)
		{
			const auto strength = pType->Strength;

			const auto percent = pTypeExt->SelfHealing_Max.Get(pThis);
			const auto maxHealth = std::clamp(static_cast<int>(percent * strength), 1, strength);
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
	if (pThis->InLimbo || pThis->IsCrashing || pThis->IsSinking || Is_DriverKilled(pThis))
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

	if (nAmount > 0 || nAmount != 0)
	{
		pThis->Health += nAmount;
	}

	TechnoExt::ApplyGainedSelfHeal(pThis);

	return SkipAnySelfHeal;
}

// spark particle systems created at random intervals
DEFINE_OVERRIDE_HOOK(0x6FAD49, TechnoClass_Update_SparkParticles, 8) // breaks the loop
{
	GET(TechnoClass*, pThis, ESI);
	REF_STACK(DynamicVectorClass<ParticleSystemTypeClass const*>, Systems, 0x60);

	auto pType = pThis->GetTechnoType();
	auto pExt = TechnoTypeExt::ExtMap.Find(pType);

	auto it = pExt->ParticleSystems_DamageSparks.GetElements(pType->DamageParticleSystems);
	auto allowAny = pExt->ParticleSystems_DamageSparks.HasValue();

	for (auto pSystem : it)
	{
		if (allowAny || pSystem->BehavesLike == BehavesLike::Spark)
		{
			Systems.AddItem(pSystem);
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
	TechnoClass* pThis = reinterpret_cast<TechnoClass*>(ptr - 0x9C);
	auto pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	int nDefault = RulesExt::Global()->DecloakSound.Get(RulesClass::Instance->CloakSound);
	R->ECX(pExt->DecloakSound.Get(nDefault));
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

	// find any match
	for (auto i = pNames->begin(); i < pNames->end(); ++i)
	{
		if (IS_SAME_STR_(*i, id))
		{
			if (pThis->CanBeSelectedNow())
			{
				match = true;
				break;
			}

			// buildings are exempt if they can't undeploy
			if (Is_Building(pThis) && pType->UndeploysInto)
			{
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
	const auto& modifier = (force ? pData->ForceShield_Modifier : pData->IronCurtain_Modifier);

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

DEFINE_OVERRIDE_SKIP_HOOK(0x6BB9DD, WinMain_LogGameClasses, 5, 6BBE2B)
DEFINE_OVERRIDE_SKIP_HOOK(0x70CAD8, TechnoClass_DealParticleDamage_DontDestroyCliff, 9, 70CB30)

DEFINE_OVERRIDE_HOOK(0x70CBB0, TechnoClass_DealParticleDamage_AmbientDamage, 6)
{
	GET_BASE(WeaponTypeClass*, pWeapon, 0x14);

	if (!pWeapon->AmbientDamage)
		return 0x70CC3E;

	R->EDI(pWeapon);
	R->ESI(0);
	return (!(R->EAX<int>() <= 0)) ? 0x70CBB9 : 0x70CBF7;
}

DEFINE_OVERRIDE_HOOK(0x0CBDA, TechnoClass_DealParticleDamage, 6)
{
	GET(TechnoClass*, pSource, EDX);
	R->Stack<HouseClass*>(0xC, pSource->Owner);
	return 0;
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


//DEFINE_OVERRIDE_HOOK_AGAIN(0x6FB4A3 , TechnoClass_CreateGap_LargeGap, 7)
//DEFINE_OVERRIDE_HOOK(0x6FB1B5, TechnoClass_CreateGap_LargeGap, 7)
//{
//	GET(TechnoClass*, pThis, ESI);
//	GET(TechnoTypeClass*, pType, EAX);
//	// ares change this to read from ext instead
//	// since this one is `char` so it cant store bigger number i assume
//	pThis->GapRadius = pType->GapRadiusInCells;
//	return R->Origin() + 0xD;
//}

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

// #895225: make the AI smarter. this code was missing from YR.
// it clears the targets and assigns the attacker the team's current focus.
DEFINE_OVERRIDE_HOOK(0x6EB432, TeamClass_AttackedBy_Retaliate, 9)
{
	GET(TeamClass*, pThis, ESI);
	GET(AbstractClass*, pAttacker, EBP);

	// get ot if global option is off
	if (!RulesExt::Global()->TeamRetaliate)
	{
		return 0x6EB47A;
	}

	auto pFocus = abstract_cast<TechnoClass*>(pThis->Focus);
	auto pSpawn = pThis->SpawnCell;

	if (!pFocus || !pFocus->IsArmed() || !pSpawn || pFocus->IsCloseEnoughToAttackCoords(pSpawn->GetCoords()))
	{
		// disallow aircraft, or units considered as aircraft, or stuff not on map like parasites
		if (!Is_Aircraft(pAttacker))
		{
			if (auto pAttackerFoot = abstract_cast<FootClass*>(pAttacker))
			{
				if (pAttackerFoot->InLimbo || pAttackerFoot->GetTechnoType()->ConsideredAircraft)
				{
					return 0x6EB47A;
				}
			}

			pThis->Focus = pAttacker;

			// this is the original code, but commented out because it's responsible for switching
			// targets when the team is attacked by two or more opponents. Now, the team should pick
			// the first target, and keep it. -AlexB
			//for(NextTeamMember i(pThis->FirstUnit); i; ++i) {
			//	if(i->IsAlive && i->Health && (Unsorted::IKnowWhatImDoing || !i->InLimbo)) {
			//		if(i->IsTeamLeader || i->WhatAmI() == AircraftClass::AbsID) {
			//			i->SetTarget(nullptr);
			//			i->SetDestination(nullptr, true);
			//		}
			//	}
			//}
		}
	}

	return 0x6EB47A;
}

// #1260: reinforcements via actions 7 and 80, and chrono reinforcements
// via action 107 cause crash if house doesn't exist
DEFINE_OVERRIDE_HOOK_AGAIN(0x65EC4A, TeamTypeClass_ValidateHouse, 6)
DEFINE_OVERRIDE_HOOK(0x65D8FB, TeamTypeClass_ValidateHouse, 6)
{
	GET(TeamTypeClass*, pThis, ECX);
	HouseClass* pHouse = pThis->GetHouse();

	// house exists; it's either declared explicitly (not Player@X) or a in campaign mode
	// (we don't second guess those), or it's still alive in a multiplayer game
	if (pHouse &&
		(pThis->Owner || SessionClass::Instance->GameMode == GameMode::Campaign || !pHouse->Defeated))
	{
		return 0;
	}

	// no.
	return (R->Origin() == 0x65D8FB) ? 0x65DD1B : 0x65F301;
}

// bugfix #187: Westwood idiocy
DEFINE_OVERRIDE_SKIP_HOOK(0x531726, Game_BulkDataInit_MultipleDataInitFix1, 5, 53173A)

// bugfix #187: Westwood idiocy
DEFINE_OVERRIDE_SKIP_HOOK(0x53173F, Game_BulkDataInit_MultipleDataInitFix2, 5, 531749)

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

DEFINE_OVERRIDE_SKIP_HOOK(0x67F281, LoadGame_LateSkipSides, 7, 67F2BF)

// ==============================

DEFINE_OVERRIDE_SKIP_HOOK(0x5A5C6A, MapSeedClass_Generate_PlacePavedRoads_RoadEndNE, 9, 5A5CC8)

DEFINE_OVERRIDE_SKIP_HOOK(0x5A5D6F, MapSeedClass_Generate_PlacePavedRoads_RoadEndSW, 9, 5A5DB8)

DEFINE_OVERRIDE_SKIP_HOOK(0x5A5F6A, MapSeedClass_Generate_PlacePavedRoads_RoadEndNW, 8, 5A5FF8)

DEFINE_OVERRIDE_SKIP_HOOK(0x5A6464, MapSeedClass_Generate_PlacePavedRoads_RoadEndSE, 9, 5A64AD)

// ==============================

DEFINE_OVERRIDE_SKIP_HOOK(0x59000E, RMG_FixPavedRoadEnd_Bridges_North, 5, 590087)

DEFINE_OVERRIDE_SKIP_HOOK(0x5900F7, RMG_FixPavedRoadEnd_Bridges_South, 5, 59015E)

DEFINE_OVERRIDE_SKIP_HOOK(0x58FCC6, RMG_FixPavedRoadEnd_Bridges_West, 5, 58FD2A)

DEFINE_OVERRIDE_SKIP_HOOK(0x58FBDD, RMG_FixPavedRoadEnd_Bridges_East, 5, 58FC55)

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

template<size_t idx>
static void* AresExtMap_Find(void* const key)
{
	return AresData::AresThiscall<AresData::FunctionIndices::ExtMapFindID, void*, DWORD, void*>()(AresData::AresStaticInstanceFinal[idx], key);
}

#include <Ext/ParticleSystem/Body.h>

struct AresParticleExtData
{
	ParticleSystemClass* OwnerObject;
	InitState State;
	int Type;
	ParticleTypeClass* HoldType;
	/*	std::vector<T> usually compiled like these
	* struct std_vector_T // size 0xC
	* {
	*	 T* first;
	*	 T* last;
	*    T* end;
	* }
	*/
	std::vector<ParticleDatas> DataA; //stored state data
	std::vector<ParticleDatas> DataB; //stored state data (only used on gas)
};
//the alloc size doesnt match the class size for some reason ?
//static_assert(sizeof(AresParticleExtData) == 64);

//DEFINE_HOOK(0x62FD60, ParticleSystemClass_Update, 9)
//{
//	GET(ParticleSystemClass*, pThis, ECX);
//	const auto pThisExt = (AresParticleExtData*)AresExtMap_Find<0>(pThis);
//	if(!pThisExt->DataA.empty() && !pThisExt->DataB.empty())
//	Debug::Log("ParticeSystem [%s] With ExtPtr [%x - offs %x] ! \n", pThis->get_ID(), pThisExt);
//	//return HandleParticleSys(pThis) ? 0x62FE43 : 0;
//	return 0x0;
//}

#include <Utilities/Helpers.h>
#include <Ext/ParticleType/Body.h>

std::pair<TechnoClass*, HouseClass*> GetOwnership(ParticleClass* pThis)
{
	TechnoClass* pAttacker = nullptr;
	HouseClass* pOwner = nullptr;

	if (auto const pSystem = pThis->ParticleSystem)
	{
		if (auto pSystemOwner = pSystem->Owner)
		{
			if (((pSystemOwner->AbstractFlags & AbstractFlags::Techno) != AbstractFlags::None))
				pAttacker = static_cast<TechnoClass*>(pSystemOwner);
			else if (Is_Bullet(pSystemOwner))
				pAttacker = static_cast<BulletClass*>(pSystemOwner)->Owner;
		}

		if (pAttacker)
			pOwner = pAttacker->GetOwningHouse();
	}

	return { pAttacker , pOwner };
}

DEFINE_OVERRIDE_HOOK(0x62C23D, ParticleClass_Update_Gas_DamageRange, 6)
{
	GET(ParticleClass*, pThis, EBP);
	auto pTypeExt = ParticleTypeExt::ExtMap.Find(pThis->Type);

	if (pTypeExt->DamageRange.Get() <= 0.0)
		return 0x0;

	const auto pVec = Helpers::Alex::getCellSpreadItems(pThis->Location, std::ceil(pTypeExt->DamageRange.Get()));
	const auto& [pAttacker, pOwner] = GetOwnership(pThis);

	for (const auto& pItem : pVec)
	{
		if (pItem->Health <= 0 || !pItem->IsAlive || pItem->InLimbo)
			continue;

		if (pItem->IsSinking || pItem->IsCrashing || pItem->TemporalTargetingMe)
			continue;

		if (!Is_Building(pItem) && TechnoExt::IsChronoDelayDamageImmune(static_cast<FootClass*>(pItem)))
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
	GET(ParticleClass*, pParticle, EBP);
	GET(ObjectClass*, pTarget, EDI);
	GET(int, nDistance, ECX);
	GET(int, nDamage, EDX);

	if (pTarget->InLimbo || pTarget->Health <= 0 || !pTarget->IsAlive)
		return 0x62CE09;

	if (auto pTechno = generic_cast<TechnoClass*>(pTarget))
	{
		if (pTechno->IsSinking || pTechno->IsCrashing || pTechno->TemporalTargetingMe)
			return 0x62CE09;

		if (!Is_Building(pTechno) && TechnoExt::IsChronoDelayDamageImmune(static_cast<FootClass*>(pTechno)))
			return 0x62CE09;
	}

	auto const& [pAttacker, pOwner] = GetOwnership(pParticle);

	if (pAttacker == pTarget)
		return 0x62CE09;

	pTarget->ReceiveDamage(&nDamage, nDistance, pParticle->Type->Warhead, pAttacker, false, false, pOwner);

	return 0x62CE09;
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

		if (!Is_Building(pTechno) && TechnoExt::IsChronoDelayDamageImmune(static_cast<FootClass*>(pTechno)))
			return 0x62C309;
	}

	auto const& [pAttacker, pOwner] = GetOwnership(pParticle);
	int nDamage = pParticle->Type->Damage;
	pTarget->ReceiveDamage(&nDamage, nDistance, pParticle->Type->Warhead, pAttacker, false, false, pOwner);

	return 0x62C309;
}

DEFINE_OVERRIDE_HOOK(0x62A020, ParasiteClass_Update, 0xA)
{
	GET(TechnoClass*, pOwner, ECX);
	R->EAX(pOwner->GetWeapon(Ares_ParasiteWeapon(pOwner)));
	return 0x62A02A;
}

DEFINE_OVERRIDE_HOOK(0x62A7B1, Parasite_ExitUnit, 9)
{
	GET(TechnoClass*, pOwner, ECX);
	R->EAX(pOwner->GetWeapon(Ares_ParasiteWeapon(pOwner)));
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
		nFallRate = !bAnimAttached ? pExt->FallRate_NoParachute.Get() : pExt->FallRate_Parachute.Get();
		auto& nCustomMaxFallRate = !bAnimAttached ? pExt->FallRate_NoParachuteMax : pExt->FallRate_ParachuteMax;

		if (nCustomMaxFallRate.isset())
			nMaxFallRate = nCustomMaxFallRate.Get();
	}

	if (pThis->FallRate - nFallRate >= nMaxFallRate)
		nMaxFallRate = pThis->FallRate - nFallRate;

	pThis->FallRate = nMaxFallRate;
	return 0x5F3FFD;
}

DEFINE_OVERRIDE_HOOK(0x481C6C, CellClass_CrateBeingCollected_Armor1, 6)
{
	GET(TechnoClass*, Unit, EDI);
	return (AE_ArmorMult(Unit) == 1.0) ? 0x481D52 : 0x481C86;
}

DEFINE_OVERRIDE_HOOK(0x481CE1, CellClass_CrateBeingCollected_Speed1, 6)
{
	GET(FootClass*, Unit, EDI);
	return (AE_SpeedMult(Unit) == 1.0) ? 0x481D52 : 0x481C86;
}

DEFINE_OVERRIDE_HOOK(0x481D0E, CellClass_CrateBeingCollected_Firepower1, 6)
{
	GET(TechnoClass*, Unit, EDI);
	return (AE_FirePowerMult(Unit) == 1.0) ? 0x481D52 : 0x481C86;
}

DEFINE_OVERRIDE_HOOK(0x481D3D, CellClass_CrateBeingCollected_Cloak1, 6)
{
	GET(TechnoClass*, Unit, EDI);

	if (Unit->CanICloakByDefault() || AE_Cloak(Unit))
	{
		return 0x481C86;
	}

	// cloaking forbidden for type
	return  (!TechnoTypeExt::ExtMap.Find(Unit->GetTechnoType())->CloakAllowed) ? 0x481C86 : 0x481D52;
}

//overrides on actual crate effect applications
DEFINE_OVERRIDE_HOOK(0x48294F, CellClass_CrateBeingCollected_Cloak2, 7)
{
	GET(TechnoClass*, Unit, EDX);
	AE_Cloak(Unit) = true;
	AresData::RecalculateStat(Unit);
	return 0x482956;
}

DEFINE_OVERRIDE_HOOK(0x482E57, CellClass_CrateBeingCollected_Armor2, 6)
{
	GET(TechnoClass*, Unit, ECX);
	GET_STACK(double, Pow_ArmorMultiplier, 0x20);

	auto& nArmor = AE_ArmorMult(Unit);
	if (nArmor == 1.0)
	{
		nArmor = Pow_ArmorMultiplier;
		AresData::RecalculateStat(Unit);
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

	auto& nSpeed = AE_SpeedMult(Unit);
	if (nSpeed == 1.0)
	{
		nSpeed = Pow_SpeedMultiplier;
		AresData::RecalculateStat(Unit);
		R->CL(Unit->GetOwningHouse()->IsInPlayerControl);
		return 0x483078;
	}
	return 0x483081;
}

DEFINE_OVERRIDE_HOOK(0x483226, CellClass_CrateBeingCollected_Firepower2, 6)
{
	GET(TechnoClass*, Unit, ECX);
	GET_STACK(double, Pow_FirepowerMultiplier, 0x20);

	auto& nFP = AE_FirePowerMult(Unit);
	if (nFP == 1.0)
	{
		nFP = Pow_FirepowerMultiplier;
		AresData::RecalculateStat(Unit);
		R->AL(Unit->GetOwningHouse()->IsInPlayerControl);
		return 0x483258;
	}
	return 0x483261;
}

#include <New/Type/TheaterTypeClass.h>

DEFINE_OVERRIDE_HOOK(0x52BA78, _YR_GameInit_Pre, 5)
{
	TheaterTypeClass::LoadAllTheatersToArray();

	// issue #198: animate the paradrop cursor
	MouseCursor::GetCursor(MouseCursorType::ParaDrop).FrameRate = 4;

	// issue #214: also animate the chronosphere cursor
	MouseCursor::GetCursor(MouseCursorType::Chronosphere).FrameRate = 4;

	// issue #1380: the iron curtain cursor
	MouseCursor::GetCursor(MouseCursorType::IronCurtain).FrameRate = 4;

	// animate the engineer damage cursor
	MouseCursor::GetCursor(MouseCursorType::Detonate).FrameRate = 4;

	AresData::MouseCursorTypeLoadDefault();

	//Load the default regardless
	CursorTypeClass::AddDefaults();

	return 0;
}

struct TechnoExt_Stuffs
{
	static bool IsPowered(TechnoClass* pThis)
	{
		auto pType = pThis->GetTechnoType();
		if (pType->PoweredUnit)
		{
			for (const auto& pBuilding : pThis->Owner->Buildings)
			{
				if (pBuilding->Type->PowersUnit == pType
					&& pBuilding->RegisteredAsPoweredUnitSource
					&& !pBuilding->IsUnderEMP()) // alternatively, HasPower, IsPowerOnline()
				{
					return true;
				}
			}
			// if we reach this, we found no building that currently powers this object
			return false;
		}
		else if (const auto pPtr = PoweredUnitUptr(pThis))
		{
			// #617
			return pPtr->Powered;
		}

		return true;
	}
};

bool IsPowered(TechnoClass* pThis)
{
	const auto pType = pThis->GetTechnoType();

	if (pType && pType->PoweredUnit)
	{
		for (const auto& pBuilding : pThis->Owner->Buildings)
		{
			if (pBuilding->Type->PowersUnit == pType
				&& pBuilding->RegisteredAsPoweredUnitSource
				&& !pBuilding->IsUnderEMP()) // alternatively, HasPower, IsPowerOnline()
			{
				return true;
			}
		}
		// if we reach this, we found no building that currently powers this object
		return false;
	}
	else if (auto pPoweredUnit = PoweredUnitUptr(pThis))
	{
		// #617
		return pPoweredUnit->Powered;
	}

	// object doesn't need a particular powering structure, therefore, for the purposes of the game, it IS powered
	return true;
}

DEFINE_OVERRIDE_HOOK(0x736135, UnitClass_Update_Deactivated, 6)
{
	GET(UnitClass*, pThis, ESI);

	// don't sparkle on EMP, Operator, ....
	return IsPowered(pThis)
		? 0x7361A9 : 0;
}

// merge two small visceroids into one large visceroid
DEFINE_OVERRIDE_HOOK(0x739F21, UnitClass_UpdatePosition_Visceroid, 6)
{
	GET(UnitClass*, pThis, EBP);

	auto const Allow = [](UnitClass* pUnit)
	{
		if (!pUnit->IsAlive)
			return false;

		if (pUnit->InLimbo)
			return false;

		if (pUnit->TemporalTargetingMe)
			return false;

		if (Is_DriverKilled(pUnit))
			return false;

		if (pUnit->BerzerkDurationLeft)
			return false;

		if (pUnit->LocomotorSource)
			return false;

		if (TechnoExt::IsInWarfactory(pUnit))
			return false;

		return true;
	};

	// fleshbag erotic
	if (UnitTypeClass* pLargeType = RulesClass::Instance->LargeVisceroid)
	{
		if (pThis->Type->SmallVisceroid && Allow(pThis))
		{
			if (UnitClass* pDest = specific_cast<UnitClass*>(pThis->Destination))
			{
				if (pDest->Type->SmallVisceroid && Allow(pDest))
				{

					// nice to meat you!
					auto crdMe = pThis->GetCoords();
					auto crdHim = pDest->GetCoords();

					auto cellMe = CellClass::Coord2Cell(crdMe);
					auto cellHim = CellClass::Coord2Cell(crdHim);

					// two become one
					if (cellMe == cellHim)
					{

						pDest->Type = pLargeType;
						pDest->Health = pLargeType->Strength;
						pDest->EstimatedHealth = pDest->Health;

						pDest->Target = nullptr;
						pDest->Destination = nullptr;
						pDest->Stun();

						pDest->IsSelected = pThis->IsSelected;

						CellClass* pCell = MapClass::Instance->GetCellAt(pDest->LastMapCoords);
						pDest->UpdateThreatInCell(pCell);

						pDest->QueueMission(Mission::Guard, true);
						GameDelete<true, false>(pThis);
						//pThis->UnInit();
						return 0x73B0A5;
					}
				}
			}
		}
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x746B89, UnitClass_GetUIName, 8)
{
	GET(TechnoClass*, pThis, ESI);
	const auto pType = pThis->GetTechnoType();
	const auto nCurWp = pThis->CurrentWeaponNumber;
	const auto& nVec = GetGunnerName(pType);

	const wchar_t* Text = nullptr;
	if (nCurWp < (int)nVec.size())
	{
		Text = nVec[nCurWp].Text;
	}

	R->EAX(Text);

	return Text != nullptr ? 0x746C78 : 0;
}

DEFINE_OVERRIDE_HOOK(0x73C143, UnitClass_DrawVXL_Deactivated, 5)
{
	GET(UnitClass*, pThis, ECX);
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
		if (Is_Operated(pThis) || AresData::IsOperated(pThis))
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
	if (Is_DriverKilled(pTarget))
	{
		return Disallowed;
	}

	// passed all tests
	return Allowed;
}

DEFINE_OVERRIDE_HOOK(0x51DF38, InfantryClass_Remove, 0xA)
{
	GET(InfantryClass*, pThis, ESI);

	if (auto pGarrison = AresGarrisonedIn(pThis))
	{
		if (!pGarrison->Occupants.Remove(pThis))
		{
			Debug::Log("Infantry %s was garrisoned in building %s, but building didn't find it. WTF?",
				pThis->Type->ID, pGarrison->Type->ID);
		}
	}

	AresGarrisonedIn(pThis) = nullptr;

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x51DFFD, InfantryClass_Put, 5)
{
	GET(InfantryClass*, pThis, EDI);
	AresGarrisonedIn(pThis) = nullptr;

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x518434, InfantryClass_ReceiveDamage_SkipDeathAnim, 7)
{
	GET(InfantryClass*, pThis, ESI);
	//GET_STACK(ObjectClass *, pAttacker, 0xE0);

	// there is not InfantryExt ExtMap yet!
	// too much space would get wasted since there is only four bytes worth of data we need to store per object
	// so those four bytes get stashed in Techno Map instead. they will get their own map if there's ever enough data to warrant it

	return AresGarrisonedIn(pThis) ? 0x5185F1 : 0;
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

DEFINE_OVERRIDE_HOOK(0x4DA53E, FootClass_Update, 6)
{
	GET(FootClass* const, pThis, ESI);

	auto const pType = pThis->GetTechnoType();

	if (ActiveSFW)
	{
		if (pThis->IsAlive && !pThis->InOpenToppedTransport && !pType->IgnoresFirestorm)
		{
			auto const pCell = pThis->GetCell();
			if (auto const pBld = pCell->GetBuilding())
			{
				if (AresData::IsActiveFirestormWall(pBld, nullptr))
				{
					AresData::ImmolateVictim(pBld, pThis, true);
				}
			}
		}
	}

	// tiberium heal, as in Tiberian Sun, but customizable per Tiberium type
	if (pThis->IsAlive && RulesExt::Global()->Tiberium_HealEnabled
		&& pThis->GetHeight() <= RulesClass::Instance->HoverHeight)
	{
		if (pType->TiberiumHeal || pThis->HasAbility(AbilityType::TiberiumHeal))
		{
			if (pThis->Health > 0 && pThis->Health < pType->Strength)
			{
				auto const pCell = pThis->GetCell();
				if (pCell->LandType == LandType::Tiberium)
				{
					auto delay = RulesClass::Instance->TiberiumHeal;
					auto health = pType->GetRepairStep();

					int idxTib = pCell->GetContainedTiberiumIndex();
					if (auto const pTib = TiberiumClass::Array->GetItemOrDefault(idxTib))
					{
						auto pExt = TiberiumExt::ExtMap.Find(pTib);
						delay = pExt->GetHealDelay();
						health = pExt->GetHealStep(pThis);
					}

					if (health != 0)
					{
						if (!(Unsorted::CurrentFrame % int(delay * 900.0)))
						{
							pThis->Health += health;
							if (pThis->Health > pType->Strength)
							{
								pThis->Health = pType->Strength;
							}
						}
					}
				}
			}
		}
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
		if (auto nJammer = std::exchange(RadarJammerUptr(pTarget), nullptr))
		{
			AresData::JammerClassUnjamAll(nJammer);
			AresMemory::Delete(nJammer);
		}

		//AttachEffect handling under Temporal
		AresData::UpdateAEData(&GetAEData(pTarget));
	}

	pThis->WarpRemaining -= pThis->GetWarpPerStep(0);

	R->EAX(pThis->WarpRemaining);
	return 0x71A88D;
}

DEFINE_OVERRIDE_HOOK(0x71AF76, TemporalClass_Fire_PrismForwardAndWarpable, 9)
{
	GET(TechnoClass* const, pThis, EDI);

	// bugfix #874 B: Temporal warheads affect Warpable=no units
	// it has been checked: this is warpable. free captured and destroy spawned units.
	if (pThis->SpawnManager)
	{
		pThis->SpawnManager->KillNodes();
	}

	if (pThis->CaptureManager)
	{
		pThis->CaptureManager->FreeAll();
	}

	// prism forward
	if (Is_Building(pThis))
	{
		AresData::CPrismRemoveFromNetwork(&PrimsForwardingPtr(static_cast<BuildingClass*>(pThis)), true);
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x4424EF, BuildingClass_ReceiveDamage_PrismForward, 6)
{
	GET(BuildingClass* const, pThis, ESI);
	AresData::CPrismRemoveFromNetwork(&PrimsForwardingPtr(pThis), true);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x454B3D, BuildingClass_UpdatePowered_PrismForward, 6)
{
	GET(BuildingClass* const, pThis, ESI);
	// this building just realised it needs to go offline
	// it unregistered itself from powered unit controls but hasn't done anything else yet
	AresData::CPrismRemoveFromNetwork(&PrimsForwardingPtr(pThis), true);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x44EBF0, BuildingClass_Disappear_PrismForward, 5)
{
	GET(BuildingClass* const, pThis, ECX);
	AresData::CPrismRemoveFromNetwork(&PrimsForwardingPtr(pThis), true);
	return 0;
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
			AresData::DisableEMPEffect(pThis);
		}
		else
		{
			// deactivate units that were unloading afterwards
			if (!pThis->Deactivated && EMPulse::IsDeactivationAdvisable(pThis))
			{
				// update the current mission
				AresEMPLastMission(pThis) = pThis->CurrentMission;
				pThis->Deactivate();
			}
		}
	}

	return 0x6FAFFD;
}

// replace the cloak checking functions to include checks for new features
DEFINE_OVERRIDE_HOOK(0x6FB757, TechnoClass_UpdateCloak, 8)
{
	GET(TechnoClass*, pThis, ESI);
	return !AresData::CloakDisallowed(pThis, false) ? 0x6FB7FD : 0x6FB75F;
}

DEFINE_OVERRIDE_HOOK(0x6FBC90, TechnoClass_ShouldNotBeCloaked, 5)
{
	GET(TechnoClass*, pThis, ECX);
	// the original code would not disallow cloaking as long as
	// pThis->Cloakable is set, but this prevents CloakStop from
	// working, because it overrides IsCloakable().
	R->EAX(AresData::CloakDisallowed(pThis, true));
	return 0x6FBDBC;
}

DEFINE_OVERRIDE_HOOK(0x6FBDC0, TechnoClass_ShouldBeCloaked, 5)
{
	GET(TechnoClass*, pThis, ECX);
	R->EAX(AresData::CloakAllowed(pThis));
	return 0x6FBF93;
}

DEFINE_OVERRIDE_HOOK(0x6F6AC9, TechnoClass_Remove_Early, 6)
{
	GET(TechnoClass*, pThis, ESI);

	// if the removed object is a radar jammer, unjam all jammed radars
	if (auto pRJ = std::exchange(RadarJammerUptr(pThis), nullptr))
	{
		AresData::JammerClassUnjamAll(pRJ);
		AresMemory::Delete(pRJ);
	}

	// #617 powered units
	if (auto pPower = std::exchange(PoweredUnitUptr(pThis), nullptr))
	{
		AresMemory::Delete(pPower);
	}

	//#1573, #1623, #255 attached effects
	if (AresData::RemoveAE(&GetAEData(pThis)))
		AresData::RecalculateStat(pThis);

	if (TechnoValueAmount(pThis) != 0)
	{
		AresData::FlyingStringsAdd(pThis, true);
	}

	return pThis->InLimbo ? 0x6F6C93u : 0x6F6AD5u;
}

DEFINE_OVERRIDE_HOOK(0x702E64, TechnoClass_RegisterDestruction_Bounty, 6)
{
	GET(TechnoClass*, pVictim, ESI);
	GET(TechnoClass*, pKiller, EDI);

	//TODO : portThese
	if (pKiller && TechnoTypeExt::ExtMap.Find(pKiller->GetTechnoType())->Bounty)
		AresData::CalculateBounty(pVictim, pKiller);

	return 0x0;
}

DEFINE_OVERRIDE_HOOK_AGAIN(0x6F6D0E, TechnoClass_Put_BuildingLight, 7)
DEFINE_OVERRIDE_HOOK(0x6F6F20, TechnoClass_Put_BuildingLight, 6)
{
	GET(TechnoClass*, pThis, ESI);

	auto pTypeData = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pTypeData->HasSpotlight)
	{
		AresData::SetSpotlight(pThis, GameCreate<BuildingLightClass>(pThis));
	}

	return 0x0;
}

DEFINE_OVERRIDE_HOOK(0x6FD0BF, TechnoClass_GetROF_AttachEffect, 6)
{
	GET(TechnoClass*, pThis, ESI);

	const auto nRof = AE_ROF(pThis);
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
			const auto& nVec = GetPilotTypeVec(pType);

			if ((size_t)pHouse->SideIndex >= nVec.size())
			{
				pCrewType = HouseExt::GetCrew(pHouse);
			}
			else if (auto pPilotType = nVec.at(pHouse->SideIndex))
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

NOINLINE InfantryTypeClass* GetBuildingCrew(BuildingClass* pThis, int nChance)
{
	// with some luck, and if the building has not been captured, spawn an engineer
	if (!pThis->HasBeenCaptured
		&& nChance > 0
		&& ScenarioClass::Instance->Random.RandomFromMax(99) < nChance)
	{
		return HouseExt::GetEngineer(pThis->Owner);
	}

	return pThis->TechnoClass::GetCrew();
}

DEFINE_OVERRIDE_HOOK(0x44EB10, BuildingClass_GetCrew, 9)
{
	GET(BuildingClass*, pThis, ECX);

	// YR defaults to 25 for buildings producing buildings
	R->EAX(GetBuildingCrew(pThis, TechnoTypeExt::ExtMap.Find(pThis->Type)->
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

	if (auto pBld = abstract_cast<BuildingClass*>(pTechno))
	{
		// allies and observers can always see by default
		bool canSeeRepair = HouseClass::CurrentPlayer->IsAlliedWith(pBld->Owner)
			|| HouseClass::IsCurrentPlayerObserver();

		bool showRepair = FileSystem::WRENCH_SHP
			&& pBld->IsBeingRepaired
			// fixes the wrench playing over a temporally challenged building
			&& !pBld->IsBeingWarpedOut()
			&& !pBld->WarpingOut
			// never show to enemies when cloaked, and only if allowed
			&& (canSeeRepair || (pBld->CloakState == CloakState::Uncloaked
				&& RulesExt::Global()->EnemyWrench));

		// display power off marker only for current player's buildings
		bool showPower = FileSystem::POWEROFF_SHP
			&& !ToggalePowerHasPower(pBld)
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

/// make temporal weapons play nice with power toggle.
//// previously, power state was set to true unconditionally.
DEFINE_OVERRIDE_HOOK(0x452287, BuildingClass_GoOnline_TogglePower, 6)
{
	GET(BuildingClass* const, pThis, ESI);
	ToggalePowerHasPower(pThis) = true;
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x452393, BuildingClass_GoOffline_TogglePower, 7)
{
	GET(BuildingClass* const, pThis, ESI);
	ToggalePowerHasPower(pThis) = false;
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x452210, BuildingClass_Enable_TogglePower, 7)
{
	GET(BuildingClass* const, pThis, ECX);
	pThis->HasPower = ToggalePowerHasPower(pThis);
	return 0x452217;
}

DEFINE_OVERRIDE_HOOK(0x70AA60, TechnoClass_DrawExtraInfo, 6)
{
	GET(TechnoClass*, pThis, ECX);
	GET_STACK(Point2D*, pPoint, 0x4);
	//GET_STACK(Point2D*, pOriginalPoint, 0x8);
	//	GET_STACK(unsigned int , nFrame, 0x4);
	GET_STACK(RectangleStruct*, pRect, 0xC);

	if (auto pBuilding = specific_cast<BuildingClass*>(pThis))
	{
		auto const pType = pBuilding->Type;
		auto const pOwner = pBuilding->Owner;
		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

		if (!pType || !pOwner)
			return 0x70AD4C;

		auto DrawTheStuff = [&](const wchar_t* pFormat)
		{
			auto nPoint = *pPoint;
			//DrawingPart
			RectangleStruct nTextDimension;
			Drawing::GetTextDimensions(&nTextDimension, pFormat, nPoint, TextPrintType::Center | TextPrintType::FullShadow | TextPrintType::Efnt, 4, 2);
			auto nIntersect = Drawing::Intersect(nTextDimension, *pRect);
			auto nColorInt = pOwner->Color.ToInit();//0x63DAD0

			DSurface::Temp->Fill_Rect(nIntersect, (COLORREF)0);
			DSurface::Temp->Draw_Rect(nIntersect, (COLORREF)nColorInt);
			Point2D nRet;
			Simple_Text_Print_Wide(&nRet, pFormat, DSurface::Temp.get(), pRect, &nPoint, (COLORREF)nColorInt, (COLORREF)0, TextPrintType::Center | TextPrintType::FullShadow | TextPrintType::Efnt, true);
			pPoint->Y += (nTextDimension.Height);
		};

		const bool IsAlly = pOwner->IsAlliedWith(HouseClass::CurrentPlayer);
		const bool IsObserver = HouseClass::Observer || HouseClass::IsCurrentPlayerObserver();
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


	if (!pThis->IsSelected)
		return 0x43E8F2;

	const auto pTypeExt = BuildingTypeExt::ExtMap.Find(pType);

	// helpers (with support for the new spy effect)
	const bool bAllied = pThis->Owner->IsAlliedWith(HouseClass::CurrentPlayer);
	const bool IsObserver = HouseClass::Observer();
	const bool bReveal = pTypeExt->SpyEffect_RevealProduction && pThis->DisplayProductionTo.Contains(HouseClass::CurrentPlayer);
	//Point2D loc = { pLocation->X , pLocation->Y };

	// show building or house state
	if (bAllied || IsObserver || bReveal)
	{

		Point2D DrawExtraLoc = { pLocation->X , pLocation->Y };
		pThis->DrawExtraInfo(DrawExtraLoc, pLocation, pBounds);


		// display production cameo
		if (IsObserver || bReveal)
		{
			auto pFactory = pThis->Factory;
			if (pThis->Owner->ControlledByPlayer())
			{
				pFactory = pThis->Owner->GetPrimaryFactory(pType->Factory, pType->Naval, BuildCat::DontCare);
			}

			if (pFactory && pFactory->Object)
			{
				auto pProdType = pFactory->Object->GetTechnoType();
				//const int nTotal = pFactory->CountTotal(pProdType);

				// support for pcx cameos
				if (auto pPCX = GetCameoPCXSurface(pProdType))
				{
					const int cameoWidth = 60;
					const int cameoHeight = 48;

					RectangleStruct cameoBounds = { 0, 0, pPCX->Width, pPCX->Height };
					RectangleStruct DefcameoBounds = { 0, 0, cameoWidth, cameoHeight };
					RectangleStruct destRect = { DrawExtraLoc.X - cameoWidth / 2, DrawExtraLoc.Y - cameoHeight / 2, cameoWidth , cameoHeight };

					if (Game::func_007BBE20(&destRect, pBounds, &DefcameoBounds, &cameoBounds))
					{
						Buffer_To_Surface_wrapper(DSurface::Temp, &destRect, pPCX, &DefcameoBounds);
					}
				}
				else
				{
					// old shp cameos, fixed palette
					auto pCameo = pProdType->GetCameo();

					if (!pCameo)
						return 0x43E8F2;

					const auto pCustomConvert = GetCameoSHPConvert(pProdType);;
					DSurface::Temp->DrawSHP(pCustomConvert ? pCustomConvert : FileSystem::CAMEO_PAL(), pCameo, 0, &DrawExtraLoc, pBounds, BlitterFlags(0xE00), 0, 0, 0, 1000, 0, nullptr, 0, 0, 0);
				}

				//auto nPoint = loc;
				////DrawingPart
				//RectangleStruct nTextDimension;
				//wchar_t pOutFormat[0x80];
				//swprintf_s(pOutFormat, L"x%d", nTotal);
				//Drawing::GetTextDimensions(&nTextDimension, pOutFormat, nPoint, TextPrintType::Center | TextPrintType::FullShadow | TextPrintType::Efnt, 4, 2);
				//auto nIntersect = Drawing::Intersect(nTextDimension, *pBounds);
				//auto nColorInt = pFactory->Owner->Color.ToInit();//0x63DAD0

				//DSurface::Temp->Fill_Rect(nIntersect, (COLORREF)0);
				//DSurface::Temp->Draw_Rect(nIntersect, (COLORREF)nColorInt);
				//Point2D nRet;
				//Simple_Text_Print_Wide(&nRet, pOutFormat, DSurface::Temp.get(), pBounds, &nPoint, (COLORREF)nColorInt, (COLORREF)0, TextPrintType::Center | TextPrintType::FullShadow | TextPrintType::Efnt, true);
			}
		}

	}

	return 0x43E8F2;
}

DEFINE_OVERRIDE_HOOK(0x70FD9A, TechnoClass_Drain_PrismForward, 6)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(TechnoClass* const, pDrainee, EDI);
	if (pDrainee->DrainingMe != pThis)
	{ // else we're already being drained, nothing to do
		if (auto const pBld = specific_cast<BuildingClass*>(pDrainee))
		{
			AresData::CPrismRemoveFromNetwork(&PrimsForwardingPtr(pBld), true);
		}
	}
	return 0;
}

void UpdateFactoryQueues(BuildingClass const* const pBuilding)
{
	auto const pType = pBuilding->Type;
	if (pType->Factory != AbstractType::None)
	{
		pBuilding->Owner->Update_FactoriesQueues(
			pType->Factory, pType->Naval, BuildCat::DontCare);
	}
}

DEFINE_OVERRIDE_HOOK(0x452218, BuildingClass_Enable_Temporal_Factories, 6)
{
	GET(BuildingClass*, pThis, ECX);
	UpdateFactoryQueues(pThis);
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

	if (pThis->IsUnderEMP() || !AresData::IsPowered(pThis))
	{
		return 0x70FC85;
	}

	const bool IsOperatored = Is_Operated(pThis) || AresData::IsOperated(pThis);
	if (!IsOperatored)
	{
		return 0x70FC85;
	}

	pThis->Guard();

	if (auto const pFoot = abstract_cast<FootClass*>(pThis))
	{
		pFoot->Locomotor.GetInterfacePtr()->Power_On();
	}

	if (auto const wasDeactivated = std::exchange(pThis->Deactivated, false))
	{
		// change: don't play sound when mutex active
		if (!Unsorted::IKnowWhatImDoing && pType->ActivateSound != -1)
		{
			VocClass::PlayAt(pType->ActivateSound, pThis->Location, nullptr);
		}

		// change: add spotlight
		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
		if (pTypeExt->HasSpotlight)
		{
			++Unsorted::IKnowWhatImDoing;
			AresData::SetSpotlight(pThis, GameCreate<BuildingLightClass>(pThis));
			--Unsorted::IKnowWhatImDoing;
		}

		// change: update factories
		if (auto const pBld = specific_cast<BuildingClass*>(pThis))
		{
			UpdateFactoryQueues(pBld);
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
		if (!Unsorted::IKnowWhatImDoing && pType->DeactivateSound != -1)
		{
			VocClass::PlayAt(pType->DeactivateSound, pThis->Location, nullptr);
		}

		// change: remove spotlight
		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
		if (pTypeExt->HasSpotlight)
		{
			AresData::SetSpotlight(pThis, nullptr);
		}

		// change: update factories
		if (auto const pBld = specific_cast<BuildingClass*>(pThis))
		{
			UpdateFactoryQueues(pBld);
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

	if (auto pJammer = RadarJammerUptr(pThis))
	{
		AresData::JammerClassUnjamAll(pJammer);
	}

	if (TechnoValueAmount(pThis) != 0)
		AresData::FlyingStringsAdd(pThis, true);

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x65DBB3, TeamTypeClass_CreateInstance_Plane, 5)
{
	GET(FootClass*, pFoot, EBP);
	R->ECX(HouseExt::GetParadropPlane(pFoot->Owner));
	++Unsorted::IKnowWhatImDoing();
	return 0x65DBD0;
}

enum AresScripts : int
{
	AuxilarryPower = 65,
	KillDrivers = 66,
	TakeVehicles = 67,
	ConvertType = 68,
	SonarReveal = 69,
	DisableWeapons = 70,
};

bool ScriptExt_Handle(TeamClass* pTeam, ScriptActionNode* pTeamMission, bool bThirdArd)
{
	switch ((AresScripts)pTeamMission->Action)
	{
	case AuxilarryPower:
	{
		AuxPower(pTeam->Owner) += pTeamMission->Argument;
		pTeam->Owner->RecheckPower = 1;
		pTeam->StepCompleted = 1;
		return true;
	}
	case KillDrivers:
	{
		const auto pToHouse = HouseExt::FindSpecial();

		for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		{
			if (pUnit->Health > 0 && pUnit->IsAlive && pUnit->IsOnMap && !pUnit->InLimbo)
			{
				if (!Is_DriverKilled(pUnit) && AresData::IsDriverKillable(pUnit, 1.0))
				{
					AresData::KillDriverCore(pUnit, pToHouse, nullptr, false);
				}
			}
		}

		pTeam->StepCompleted = 1;
		return true;
	}
	case TakeVehicles:
	{
		for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		{
			TakeVehicleMode(pUnit) = true;

			if (pUnit->GarrisonStructure())
				pUnit->Team->RemoveMember(pUnit, -1, 1);
		}

		pTeam->StepCompleted = 1;
		return true;
	}
	case ConvertType:
	{
		for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		{
			const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pUnit->GetTechnoType());
			if (pTypeExt->Convert_Script)
			{
				AresData::ConvertTypeTo(pUnit, pTypeExt->Convert_Script);
			}
		}

		pTeam->StepCompleted = 1;
		return true;
	}
	case SonarReveal:
	{
		const auto nDur = pTeamMission->Argument;
		for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		{
			auto& nSonarTime = GetSonarTimer(pUnit);
			if (nDur > nSonarTime.GetTimeLeft())
			{
				nSonarTime.Start(nDur);
			}
			else if (nDur <= 0)
			{
				if (nDur == 0)
				{
					nSonarTime.Stop();
				}
			}
		}

		pTeam->StepCompleted = 1;
		return true;
	}
	case DisableWeapons:
	{
		const auto nDur = pTeamMission->Argument;
		for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		{
			auto& nTimer = GetDisableWeaponTimer(pUnit);
			if (nDur > nTimer.GetTimeLeft())
			{
				nTimer.Start(nDur);
			}
			else if (nDur <= 0)
			{
				if (nDur == 0)
				{
					nTimer.Stop();
				}
			}
		}


		pTeam->StepCompleted = 1;
		return true;
	}
	default:

		if (pTeamMission->Action == 64)
		{
			for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
			{
				TakeVehicleMode(pUnit) = false;

				if (pUnit->GarrisonStructure())
					pUnit->Team->RemoveMember(pUnit, -1, 1);
			}

			pTeam->StepCompleted = 1;
			return true;
		}

		return false;
	}
}

//DEFINE_OVERRIDE_HOOK(0x6E9443, TeamClass_AI_HandleAres, 8)
//{
//	enum { ReturnFunc = 0x6E95AB, Continue = 0x0 };
//	GET(TeamClass*, pThis, ESI);
//	GET(ScriptActionNode*, pTeamMission, EAX);
//	GET_STACK(bool, bThirdArg, 0x10);
//	return ScriptExt_Handle(pThis, pTeamMission, bThirdArg)
//		? ReturnFunc : Continue;
//}

DEFINE_OVERRIDE_HOOK(0x413FD2, AircraftClass_Init_Academy, 6)
{
	GET(AircraftClass*, pThis, ESI);

	if (pThis->Owner)
	{
		if (pThis->Type->Trainable && Is_AirfieldSpied(pThis->Owner))
			pThis->Veterancy.Veterancy = 1.0f;

		AresData::HouseExt_ExtData_ApplyAcademy(pThis->Owner, pThis, AbstractType::Aircraft);

	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x517D51, InfantryClass_Init_Academy, 6)
{
	GET(InfantryClass*, pThis, ESI);

	if (pThis->Owner) {
		AresData::HouseExt_ExtData_ApplyAcademy(pThis->Owner , pThis, AbstractType::Infantry);
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x442D1B, BuildingClass_Init_Academy, 6)
{
	GET(BuildingClass*, pThis, ESI);

	if (!pThis->Owner)
		return 0x0;

	const auto& vetBld = VeteranBuildings(pThis->Owner->Type);

	for (auto nPos = vetBld.begin(); nPos != vetBld.end(); ++nPos)
	{
		if ((*nPos) == pThis->Type)
		{
			pThis->Veterancy.Veterancy = 1.0f;
			break;
		}
	}

	if (pThis->Type->Trainable && Is_BuildingProductionSpied(pThis->Owner))
		pThis->Veterancy.Veterancy = 1.0f;


	AresData::HouseExt_ExtData_ApplyAcademy(pThis->Owner, pThis, AbstractType::Building);

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

bool CarryallCanLift(AircraftTypeClass* pCarryAll, UnitClass* Target)
{

	if (Target->ParasiteEatingMe)
	{
		return false;
	}

	const auto CarryAllData = TechnoTypeExt::ExtMap.Find(pCarryAll);
	const auto TargetData = TechnoTypeExt::ExtMap.Find(Target->Type);

	UnitTypeClass* TargetType = Target->Type;

	if (!TargetData->CarryallAllowed.Get(!TargetType->Organic && !TargetType->NonVehicle))
		return false;

	const auto& nSize = CarryAllData->CarryallSizeLimit;

	if (nSize.isset() && nSize.Get() != -1) {
		return nSize.Get() >= ((TechnoTypeClass*)Target->Type)->Size;
	}

	return true;
}

// bug #1290: carryall size limit
DEFINE_OVERRIDE_HOOK(0x417D75, AircraftClass_GetActionOnObject_CanTote, 5)
{
	GET(AircraftClass*, pCarryall, ESI);
	GET(UnitClass*, pTarget, EDI);

	return (CarryallCanLift(pCarryall->Type, pTarget))
		? 0u
		: 0x417DF6u
		;
}

DEFINE_OVERRIDE_HOOK(0x416E37, AircraftClass_Mi_MoveCarryall_CanTote, 5)
{
	GET(AircraftClass*, pCarryall, ESI);
	GET(UnitClass*, pTarget, EDI);

	return (CarryallCanLift(pCarryall->Type, pTarget))
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
	return (Is_DriverKilled(pThis) ? 0x4190DDu : 0u);
}

DEFINE_OVERRIDE_HOOK(0x41BE80, ObjectClass_DrawRadialIndicator, 3)
{
	GET(ObjectClass*, pObj, ECX);

	if (const auto pTechno = generic_cast<TechnoClass*>(pObj))
	{
		const auto pType = pTechno->GetTechnoType();

		if (pType->HasRadialIndicator && !pTechno->Deactivated)
		{
			const auto pOwner = pTechno->Owner;

			if (!pOwner)
				return 0x0;

			if (pOwner->ControlledByPlayer_())
			{
				const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
				int nRadius = 0;

				if (pTypeExt->RadialIndicatorRadius.isset())
					nRadius = pTypeExt->RadialIndicatorRadius.Get();
				else if (pType->GapGenerator)
					nRadius = pTypeExt->GapRadiusInCells.Get();
				else
				{
					const auto pWeapons = pTechno->GetPrimaryWeapon();
					if (!pWeapons || !pWeapons->WeaponType || pWeapons->WeaponType->Range <= 0)
						return 0x0;

					nRadius = pWeapons->WeaponType->Range.ToCell();
				}

				if (nRadius > 0)
				{
					auto nCoord = pTechno->GetCoords();
					const auto Color = pTypeExt->RadialIndicatorColor.Get(pOwner->Color);
					Draw_Radial_Indicator(false, true, nCoord, Color, (nRadius * 1.0f), false, true);
				}
			}
		}
	}

	return 0;
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

DEFINE_OVERRIDE_HOOK(0x436A2D, BuildingLightClass_PointerGotInvalid_OwnerCloak, 6)
{
	GET_STACK(bool, bRemoved, 0x10);
	return bRemoved ? 0x0 : 0x436A33;
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

bool IsBaseNormal(BuildingClass* pBuilding)
{
	if (Is_FromSW(pBuilding))
		return false;

	const auto pExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type);

	if (pExt->AIBaseNormal.isset())
		return pExt->AIBaseNormal;

	if (pBuilding->Type->UndeploysInto && pBuilding->Type->ResourceGatherer || pBuilding->IsStrange())
		return false;

	return true;
}

DEFINE_OVERRIDE_HOOK(0x440C08, BuildingClass_Put_AIBaseNormal, 6)
{
	GET(BuildingClass*, pThis, ESI);
	R->EAX(!IsBaseNormal(pThis));
	return 0x440C2C;
}

DEFINE_OVERRIDE_HOOK(0x456370, BuildingClass_UnmarkBaseSpace_AIBaseNormal, 6)
{
	GET(BuildingClass*, pThis, ESI);
	R->EAX(!IsBaseNormal(pThis));
	return 0x456394;
}

DEFINE_OVERRIDE_HOOK(0x445A72, BuildingClass_Remove_AIBaseNormal, 6)
{
	GET(BuildingClass*, pThis, ESI);
	R->EAX(!IsBaseNormal(pThis));
	return 0x445A94;
}

DEFINE_OVERRIDE_HOOK(0x4A8FF5, MapClass_CanBuildingTypeBePlacedHere_Ignore, 5)
{
	GET(BuildingClass*, pBuilding, ESI);
	return Is_FromSW(pBuilding) ? 0x4A8FFA : 0x0;
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

DEFINE_SKIP_HOOK(0x71B09C , TemporalClass_Logic_BuildingUnderAttack_NullptrShit , 0x5 , 71B0E7);

DEFINE_OVERRIDE_HOOK(0x4F94A5, HouseClass_BuildingUnderAttack, 6)
{
	GET(BuildingClass*, pSource, ESI);

	if (auto pWh = std::exchange(BuildingExt::ExtMap.Find(pSource)->ReceiveDamageWarhead, nullptr)) {
		if (!WarheadTypeExt::ExtMap.Find(pWh)->Malicious){
			return 0x4F95D4;
		}
	}

	return 0;
}

/*
 * Fixing issue #722
 */
DEFINE_OVERRIDE_HOOK(0x7384BD, UnitClass_ReceiveDamage_OreMinerUnderAttack, 6)
{
	GET_STACK(WarheadTypeClass*, WH, STACK_OFFS(0x44, -0xC));
	return WH && !WarheadTypeExt::ExtMap.Find(WH)->Malicious ? 0x738535u : 0u;
}

DEFINE_OVERRIDE_HOOK(0x455DA0, BuildingClass_IsFactory_CloningFacility, 6)
{
	GET(BuildingClass*, pThis, ECX);
	return BuildingTypeExt::ExtMap.Find(pThis->Type)->Cloning_Facility.Get()
		? 0x455DCD : 0x0;
}

DEFINE_OVERRIDE_HOOK(0x4444B3, BuildingClass_KickOutUnit_NoAlternateKickout, 6)
{
	GET(BuildingClass*, pThis, ESI);
	return pThis->Type->Factory == AbstractType::None
		|| BuildingTypeExt::ExtMap.Find(pThis->Type)->Cloning_Facility.Get()
		? 0x4452C5 : 0x0;
}

DEFINE_OVERRIDE_HOOK(0x446366, BuildingClass_Place_Academy, 6)
{
	GET(BuildingClass*, pThis, EBP);

	if (Is_Academy(pThis->Type) && pThis->Owner) {
		AresData::UpdateAcademy(pThis->Owner, pThis, true);
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x445905, BuildingClass_Remove_Academy, 6)
{
	GET(BuildingClass*, pThis, ESI);

	if (pThis->IsOnMap && Is_Academy(pThis->Type) && pThis->Owner) {
		AresData::UpdateAcademy(pThis->Owner, pThis, false);
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x448AB2, BuildingClass_ChangeOwnership_Remove_Academy, 6)
{
	GET(BuildingClass*, pThis, ESI);

	if (pThis->IsOnMap && Is_Academy(pThis->Type) && pThis->Owner) {
		AresData::UpdateAcademy(pThis->Owner, pThis, false);
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x4491D5, BuildingClass_ChangeOwnership_Add_Academy, 6)
{
	GET(BuildingClass*, pThis, ESI);

	if (Is_Academy(pThis->Type) && pThis->Owner) {
		AresData::UpdateAcademy(pThis->Owner, pThis, true);
	}

	return 0;
}

void KickOutHospitalArmory(BuildingClass* pThis)
{
	if (pThis->Type->Hospital || pThis->Type->Armory) {
		if (FootClass* Passenger = pThis->Passengers.RemoveFirstPassenger()) {
			pThis->KickOutUnit(Passenger, CellStruct::Empty);
		}
	}
}

DEFINE_OVERRIDE_HOOK(0x44D8A1, BuildingClass_UnloadPassengers_Unload, 6)
{
	GET(BuildingClass*, B, EBP);

	KickOutHospitalArmory(B);
	return 0;
}


DEFINE_OVERRIDE_HOOK(0x447113, BuildingClass_Sell_PrismForward, 6)
{
	GET(BuildingClass* const, pThis, ESI);

	// #754 - evict Hospital/Armory contents
	KickOutHospitalArmory(pThis);
	AresData::CPrismRemoveFromNetwork(&PrimsForwardingPtr(pThis), true);
	return 0x0;
}

DEFINE_OVERRIDE_HOOK(0x446AAF, BuildingClass_Place_SkipFreeUnits, 6)
{
	// allow free units and non-separate aircraft to be created
	// only once.
	GET(BuildingClass*, pBld, EBP);

	// skip handling free units
	if (FreeUnitDone(pBld))
		return 0x446FB6;

	FreeUnitDone(pBld) = true;
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x44019D, BuildingClass_Update_Battery, 6)
{
	GET(BuildingClass*, pThis, ESI);

	const auto& nVec = OverpoweredBuildingType(pThis->Owner);

	for (auto nPos = nVec.begin(); nPos != nVec.end(); ++nPos)
	{
		if ((*nPos) == pThis->Type)
			pThis->IsOverpowered = true;
	}

	return 0x0;

}

// #665: Raidable Buildings - prevent raided buildings from being sold while raided
DEFINE_OVERRIDE_HOOK(0x4494D2, BuildingClass_IsSellable, 6)
{
	enum { Sellable = 0x449532, Unsellable = 0x449536, Undecided = 0 };
	GET(BuildingClass*, pThis, ESI);

	// enemy shouldn't be able to sell "borrowed" buildings
	return Is_CurrentlyRaided(pThis) ? Unsellable : Undecided;
}

DEFINE_OVERRIDE_HOOK(0x449518, BuildingClass_IsSellable_FirestormWall, 6)
{
	enum { CheckHouseFireWallActive = 0x449522, ReturnFalse = 0x449536 };
	//GET(BuildingClass*, pThis, ESI);

	GET(BuildingTypeClass*, pType, ECX);
	return Is_FirestromWall(pType) ? CheckHouseFireWallActive : ReturnFalse;
}

DEFINE_OVERRIDE_HOOK(0x44E550, BuildingClass_Mi_Open_GateDown, 6)
{
	GET(BuildingClass*, pThis, ESI);
	R->ECX(BuildingTypeExt::ExtMap.Find(pThis->Type)->GateDownSound
		.Get(RulesClass::Instance->GateDown));
	return 0x44E556;
}

DEFINE_OVERRIDE_HOOK(0x44E61E, BuildingClass_Mi_Open_GateUp, 6)
{
	GET(DWORD, offset, ESI);
	const auto pThis = reinterpret_cast<BuildingClass*>(offset - 0x9C);
	R->ECX(BuildingTypeExt::ExtMap.Find(pThis->Type)->GateUpSound
		.Get(RulesClass::Instance->GateUp));
	return 0x44E624;
}

DEFINE_OVERRIDE_HOOK(0x4509B4, BuildingClass_UpdateRepair_Funds, 7)
{
	GET(BuildingClass*, pThis, ESI);
	return !pThis->Owner->IsControlledByHuman_() || RulesExt::Global()->RepairStopOnInsufficientFunds
		? 0x0 : 0x4509BB;
}

DEFINE_OVERRIDE_HOOK(0x4521C8, BuildingClass_Disable_Temporal_Factories, 6)
{
	GET(BuildingClass*, pThis, ECX);

	auto const pType = pThis->Type;
	if (pType->Factory != AbstractType::None)
	{
		pThis->Owner->Update_FactoriesQueues(
		pType->Factory, pType->Naval, BuildCat::DontCare);
	}
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x4566B0, BuildingClass_GetRangeOfRadial_Radius, 6)
{
	enum
	{
		SetVal = 0x45674E
		, Nothing = 0x0
	};

	GET(BuildingClass*, pThis, ECX);
	const auto pExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	if (!pExt->RadialIndicatorRadius.isset())
		return Nothing;

	R->EAX(pExt->RadialIndicatorRadius.Get());
	return SetVal;
}

DEFINE_OVERRIDE_HOOK(0x4581CD, BuildingClass_UnloadOccupants_AllOccupantsHaveLeft, 6)
{
	GET(BuildingClass*, pBld, ESI);
	AresData::EvalRaidStatus(pBld);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x458729, BuildingClass_KillOccupiers_AllOccupantsKilled, 6)
{
	GET(BuildingClass*, pBld, ESI);
	AresData::EvalRaidStatus(pBld);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x4586CA, BuildingClass_KillOccupiers_EachOccupierKilled, 6)
{
	GET(BuildingClass*, pBld, ESI);
	//GET(TechnoClass*, pKiller, EBP);
	//GET(int, idxOccupant, EDI);
	AresData::EvalRaidStatus(pBld);
	//return 0x4586F0;
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x46670F, BulletClass_Update_PreImpactAnim, 6)
{
	GET(BulletClass*, pThis, EBP);

	const auto pWarheadTypeExt = WarheadTypeExt::ExtMap.Find(pThis->WH);

	if (!pThis->NextAnim)
		return 0x46671D;

	if (pWarheadTypeExt->PreImpact_Moves.Get())
	{
		auto coords = pThis->NextAnim->GetCoords();
		pThis->Location = coords;
		pThis->Target = MapClass::Instance->TryGetCellAt(coords);
	}

	return 0x467FEE;
}

DEFINE_OVERRIDE_HOOK(0x46867F, BulletClass_SetMovement_Parachute, 5)
{
	GET(CoordStruct*, XYZ, EAX);
	GET(BulletClass*, Bullet, ECX);
	//	GET_BASE(VelocityClass *, Trajectory, 0xC);

	R->EBX<BulletClass*>(Bullet);

	const auto pBulletData = BulletTypeExt::ExtMap.Find(Bullet->Type);

	bool result = false;
	if (pBulletData->Parachuted)
	{
		result = Bullet->SpawnParachuted(*XYZ);
		Bullet->IsABomb = true;
	}
	else
	{
		result = Bullet->Unlimbo(*XYZ, DirType::North);
	}

	R->EAX(result);
	return 0x468689;
}

DEFINE_OVERRIDE_HOOK(0x468EB9, BulletClass_Fire_SplitsA, 6)
{
	//GET(BulletClass*, pThis, ESI);
	GET(BulletTypeClass* const, pType, EAX);
	return !BulletTypeExt::ExtMap.Find(pType)->HasSplitBehavior()
		? 0x468EC7u : 0x468FF4u;
}

DEFINE_OVERRIDE_HOOK(0x468FFA, BulletClass_Fire_SplitsB, 6)
{
	GET(BulletTypeClass* const, pType, EAX);
	return BulletTypeExt::ExtMap.Find(pType)->HasSplitBehavior()
		? 0x46909Au : 0x469008u;
}

DEFINE_OVERRIDE_HOOK(0x469EBA, BulletClass_DetonateAt_Splits, 6)
{
	GET(BulletClass*, pThis, ESI);
	BulletExt::ApplyAirburst(pThis);
	return 0x46A290;
}

DEFINE_OVERRIDE_HOOK(0x468000, BulletClass_GetAnimFrame, 6)
{
	GET(BulletClass*, pThis, ECX);

	int frame = 0;
	if (pThis->Type->AnimLow || pThis->Type->AnimHigh)
	{
		frame = pThis->AnimFrame;
	}
	else if (pThis->Type->Rotates())
	{
		DirStruct dir(-pThis->Velocity.Y, pThis->Velocity.X);
		const auto ReverseFacing32 = *reinterpret_cast<int(*)[8]>(0x7F4890);
		const auto facing = ReverseFacing32[(short)dir.GetValue(5)];
		const int length = BulletTypeExt::ExtMap.Find(pThis->Type)->AnimLength.Get();

		if (length > 1)
		{
			frame = facing * length + ((Unsorted::CurrentFrame / pThis->Type->AnimRate) % length);
		}
		else
		{
			frame = facing;
		}
	}

	R->EAX(frame);
	return 0x468088;
}

DEFINE_HOOK(0x474E40, INIClass_GetMovementZone, 7)
{
	GET(INIClass*, pINI, ECX);
	GET_STACK(const char*, Section, 0x4);
	GET_STACK(const char*, Key, 0x8);
	LEA_STACK(const char*, Default, 0xC);

	if (pINI->ReadString(Section, Key, Default, Phobos::readBuffer)) {
		if (!isdigit(Phobos::readBuffer[0])) {
			for (size_t i = 0; i < TechnoTypeClass::MovementZonesToString.c_size(); ++i) {
				if (!CRT::strcmpi(TechnoTypeClass::MovementZonesToString[i], Phobos::readBuffer)) {
					R->EAX(i);
					return 0x474E96;
				}
			}

		} else {

			const auto nValue = std::atoi(Phobos::readBuffer);
			if ((size_t)nValue < TechnoTypeClass::MovementZonesToString.c_size()) {
				R->EAX(nValue);
				return 0x474E96;
			}
		}

		Debug::INIParseFailed(Section, Key, Phobos::readBuffer, "Expect valid MovementZones");
	}

	R->EAX(0);
	return 0x474E96;
}

DEFINE_OVERRIDE_HOOK(0x4B5EB0, DropPodLocomotionClass_ILocomotion_Process_Smoke, 6)
{
	GET(FootClass*, pFoot, ESI);
	REF_STACK(const CoordStruct, Coords, 0x34);

	// create trailer even without weapon, but only if it is set
	if (!(Unsorted::CurrentFrame % 6)) {
		if (AnimTypeClass* pType = RulesExt::Global()->DropPodTrailer) {
			if (auto pAnim = GameCreate<AnimClass>(pType, Coords)) {
				AnimExt::SetAnimOwnerHouseKind(pAnim, pFoot->Owner, nullptr, pFoot, false);
			}
		}
	}

	if (const auto pWeapon = RulesClass::Instance->DropPodWeapon) {
		R->ESI(pWeapon);
		return 0x4B5F14;
	}

	return 0x4B602D;
}

DEFINE_OVERRIDE_HOOK(0x4B99A2, DropshipLoadout_WriteUnit, 0xA)
{
	GET(TechnoTypeClass*, pType, ESI);

	GET_STACK(bool, Available, STACK_OFFS(0x164, -0x8));

	LEA_STACK(Point2D*, BaseCoords, STACK_OFFS(0x164, 0x14C));
	LEA_STACK(Point2D*, AltCoords, STACK_OFFS(0x164, 0x144));

	const size_t StringLen = 256;

	wchar_t pName[StringLen];
	wchar_t pArmor[StringLen];
	wchar_t pArmament[StringLen];
	wchar_t pCost[StringLen];

	StringCchPrintfW(pName, StringLen, L"Name: %hs", pType->Name);

	if (Available)
	{
		StringCchPrintfW(pCost, StringLen, L"Cost: %d", pType->GetCost());
	}
	else
	{
		StringCchPrintfW(pCost, StringLen, L"Cost: N/A");
	}

	if (auto pPrimary = pType->Weapon[0].WeaponType)
	{
		StringCchPrintfW(pArmament, StringLen, L"Armament: %hs", pPrimary->Name);
	}
	else
	{
		StringCchPrintfW(pArmament, StringLen, L"Armament: NONE");
	}

	if (const auto& pArmorType = ArmorTypeClass::Array[static_cast<unsigned int>(pType->Armor)])
	{
		StringCchPrintfW(pArmor, StringLen, L"Armor: %hs", pArmorType->Name.data());
	}
	else
	{
		StringCchPrintfW(pArmor, StringLen, L"Armor: UNKNOWN");
	}

	auto Color = ColorScheme::Find(Available ? GameStrings::Green() : GameStrings::Red(), 1);

	auto pSurface = DSurface::Hidden();
	RectangleStruct pSurfaceRect = pSurface->Get_Rect();
	Point2D Coords = *BaseCoords;
	Coords.X += 450;
	Coords.Y += 300;

	Drawing::PrintUnicode(AltCoords, pName, pSurface, &pSurfaceRect, &Coords, Color, 0, 70);

	Coords.Y += 15;
	Drawing::PrintUnicode(AltCoords, pArmament, pSurface, &pSurfaceRect, &Coords, Color, 0, 70);

	Coords.Y += 15;
	Drawing::PrintUnicode(AltCoords, pArmor, pSurface, &pSurfaceRect, &Coords, Color, 0, 70);

	Coords.Y += 15;
	Drawing::PrintUnicode(AltCoords, pCost, pSurface, &pSurfaceRect, &Coords, Color, 0, 70);

	return 0x4B9BBF;
}

DEFINE_OVERRIDE_HOOK(0x4B9A52, DropshipLoadout_PrintArmor, 5)
{
	R->Stack(0x4, ArmorTypeClass::Array[R->EDX()].get());
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x4CF3D0, FlyLocomotionClass_sub_4CEFB0_HunterSeeker, 7)
{
	GET_STACK(FlyLocomotionClass* const, pThis, 0x20);
	auto const pObject = pThis->LinkedTo;
	auto const pType = pObject->GetTechnoType();
	auto const pExt = TechnoTypeExt::ExtMap.Find(pType);

	if (pType->HunterSeeker)
	{
		if (auto const pTarget = pObject->Target)
		{
			auto const DetonateProximity = pExt->HunterSeekerDetonateProximity.Get(RulesExt::Global()->HunterSeekerDetonateProximity);
			auto const DescendProximity = pExt->HunterSeekerDescendProximity.Get(RulesExt::Global()->HunterSeekerDescendProximity);

			// get th difference of our position to the target,
			// disregarding the Z component.
			auto crd = pObject->GetCoords();
			crd -= pThis->MovingDestination;
			crd.Z = 0;

			auto const dist = int(crd.Magnitude());

			if (dist >= DetonateProximity)
			{
				// not close enough to detonate, but we might start the decent
				if (dist < DescendProximity)
				{
					// the target's current height
					auto const z = pTarget->GetCoords().Z;

					// the hunter seeker's default flight level
					crd = pObject->GetCoords();
					auto floor = MapClass::Instance->GetCellFloorHeight(crd);
					auto const height = floor + pType->GetFlightLevel();

					// linear interpolation between target's Z and normal flight level
					auto const ratio = dist / static_cast<double>(DescendProximity);
					auto const lerp = z * (1.0 - ratio) + height * ratio;

					// set the descending flight level
					auto level = int(lerp) - floor;
					if (level < 10)
					{
						level = 10;
					}

					pThis->FlightLevel = level;

					return 0x4CF4D2;
				}

				// project the next steps using the current speed
				// and facing. if there's a height difference, use
				// the highest value as the new flight level.
				auto const speed = pThis->Apparent_Speed();
				if (speed > 0)
				{
					double const value = pObject->PrimaryFacing.Current().GetRadian();
					double const cos = std::cos(value);
					double const sin = std::sin(value);

					int maxHeight = 0;
					int currentHeight = 0;
					auto crd2 = pObject->GetCoords();
					for (int i = 0; i < 11; ++i)
					{
						auto const pCell = MapClass::Instance->GetCellAt(crd2);
						auto const z = pCell->GetCoordsWithBridge().Z;

						if (z > maxHeight)
						{
							maxHeight = z;
						}

						if (!i)
						{
							currentHeight = z;
						}

						// advance one step
						crd2.X += int(cos * speed);
						crd2.Y -= int(sin * speed);

						// result is never used in TS, but a break sounds
						// like a good idea.
						auto const cell = CellClass::Coord2Cell(crd2);
						if (!MapClass::Instance->CoordinatesLegal(cell))
						{
							break;
						}
					}

					// pull the old lady up
					if (maxHeight > currentHeight)
					{
						pThis->FlightLevel = pType->GetFlightLevel();
						return 0x4CF4D2;
					}
				}

			}
			else
			{
				// close enough to detonate
				if (auto const pTechno = abstract_cast<TechnoClass*>(pTarget))
				{
					auto const pWeapon = pObject->GetPrimaryWeapon()->WeaponType;

					// damage the target
					auto damage = pWeapon->Damage;
					pTechno->ReceiveDamage(&damage, 0, pWeapon->Warhead, pObject, true, true, nullptr);

					// damage the hunter seeker
					damage = pWeapon->Damage;
					pObject->ReceiveDamage(&damage, 0, pWeapon->Warhead, nullptr, true, true, nullptr);

					// damage the map
					auto const crd2 = pObject->GetCoords();
					MapClass::FlashbangWarheadAt(pWeapon->Damage, RulesClass::Instance->C4Warhead, crd2);
					MapClass::DamageArea(crd2, pWeapon->Damage, pObject, pWeapon->Warhead, true, nullptr);

					// return 0
					R->EBX(0);
					return 0x4CF5F2;
				}
			}
		}
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x4CDE64, FlyLocomotionClass_sub_4CD600_HunterSeeker_Ascent, 6)
{
	GET(FlyLocomotionClass* const, pThis, ESI);
	GET(int const, unk, EDI);
	auto const pObject = pThis->LinkedTo;
	auto const pType = pObject->GetTechnoType();
	auto const pExt = TechnoTypeExt::ExtMap.Find(pType);

	auto ret = pThis->FlightLevel - unk;
	auto max = 16;

	if (!pType->IsDropship)
	{
		if (!pType->HunterSeeker)
		{
			// ordinary aircraft
			max = (R->BL() != 0) ? 10 : 20;

		}
		else
		{
			// is hunter seeker
			if (pThis->IsTakingOff)
			{
				max = pExt->HunterSeekerEmergeSpeed.Get(RulesExt::Global()->HunterSeekerEmergeSpeed);
			}
			else
			{
				max = pExt->HunterSeekerAscentSpeed.Get(RulesExt::Global()->HunterSeekerAscentSpeed);
			}
		}
	}

	if (ret > max)
	{
		ret = max;
	}

	R->EAX(ret);
	return 0x4CDE8F;
}

DEFINE_OVERRIDE_HOOK(0x4CDF54, FlyLocomotionClass_sub_4CD600_HunterSeeker_Descent, 5)
{
	GET(FlyLocomotionClass* const, pThis, ESI);
	GET(int const, max, EDI);
	auto const pObject = pThis->LinkedTo;
	auto const pType = pObject->GetTechnoType();
	auto const pExt = TechnoTypeExt::ExtMap.Find(pType);

	if (pType->HunterSeeker)
	{
		auto ret = pExt->HunterSeekerDescentSpeed.Get(RulesExt::Global()->HunterSeekerDescentSpeed);
		if (max < ret)
		{
			ret = max;
		}

		R->ECX(ret);
		return 0x4CDF81;
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x4D7524, FootClass_ActionOnObject_Allow, 9)
{
	//overwrote the ja, need to replicate it
	GET(Action, CursorIndex, EBP);

	if (CursorIndex == Action::None || CursorIndex > Action::Airstrike) {
		return CursorIndex == Action(127) || CursorIndex == Action(126) ? 0x4D769F : 0x4D7CC0;
	}

	return 0x4D752D;
}

DEFINE_OVERRIDE_HOOK(0x4D9920, FootClass_SelectAutoTarget_Cloaked, 9)
{
	GET(FootClass* const, pThis, ECX);

	if (pThis->Owner->IsControlledByHuman_()
		&& pThis->GetCurrentMission() == Mission::Guard)
	{
		auto const pType = pThis->GetTechnoType();
		auto const pExt = TechnoTypeExt::ExtMap.Find(pType);

		auto allowAquire = true;

		if (!pExt->CanPassiveAcquire_Guard)
		{
			// we are in guard mode
			allowAquire = false;
		}
		else if (!pExt->CanPassiveAcquire_Cloak)
		{
			// passive acquire is disallowed when guarding and cloakable
			if (pThis->IsCloakable() || pThis->HasAbility(AbilityType::Cloak))
			{
				allowAquire = false;
			}
		}

		if (!allowAquire)
		{
			R->EAX(static_cast<TechnoClass*>(nullptr));
			return 0x4D995C;
		}
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x4D9EBD, FootClass_CanBeSold_SellUnit, 6)
{
	GET(BuildingClass*, pBld, EAX);
	GET(TechnoClass*, pDocker, ESI);

	const auto nUnitRepair = BuildingTypeExt::ExtMap.Find(pBld->Type)->UnitSell.Get(pBld->Type->UnitRepair);
	const auto nSellable = TechnoTypeExt::ExtMap.Find(pDocker->GetTechnoType())->Unsellable.Get(RulesExt::Global()->Units_UnSellable);

	if (!nUnitRepair || !nSellable)
	{
		R->CL(false);
	} else {
		R->CL(true);
	}

	return 0x4D9EC9;
}

// move to the next hva frame, even if this unit isn't moving
DEFINE_OVERRIDE_HOOK(0x4DA8B2, FootClass_Update_AnimRate, 6)
{
	GET(FootClass*, pThis, ESI);
	auto pType = pThis->GetTechnoType();
	auto pExt = TechnoTypeExt::ExtMap.Find(pType);

	enum { Undecided = 0u, NoChange = 0x4DAA01u, Advance = 0x4DA9FBu };

	// any of these prevents the animation to advance to the next frame
	if (pThis->IsBeingWarpedOut() || pThis->IsWarpingIn() || pThis->IsAttackedByLocomotor)
	{
		return NoChange;
	}

	// animate unit whenever in air
	if (pExt->AirRate && pThis->GetHeight() > 0)
	{
		return (Unsorted::CurrentFrame % pExt->AirRate) ? NoChange : Advance;
	}

	return Undecided;
}

// rotation when crashing made optional
DEFINE_OVERRIDE_HOOK(0x4DECAE, FootClass_Crash_Spin, 5)
{
	GET(FootClass*, pThis, ESI);
	return TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())->CrashSpin ? 0u : 0x4DED4Bu;
}

DEFINE_OVERRIDE_HOOK(0x518744, InfantryClass_ReceiveDamage_ElectricDeath, 6)
{
	AnimTypeClass* El = RulesExt::Global()->ElectricDeath;

	if (!El) {
		El = AnimTypeClass::Array->GetItem(1);
	}

	R->EDX(El);
	return 0x51874D;
}

DEFINE_OVERRIDE_HOOK(0x51BD4C , InfantryClass_Update_BuildingBelow, 6)
{
	GET(BuildingClass*, pBld, EDI);

	if(Is_Passable(pBld->Type))
		return 0x51BD7D;

	if (Is_FirestromWall(pBld->Type))
		return 0x51BD56;

	return 0x51BD68;
}

DEFINE_OVERRIDE_HOOK(0x51CE9A, InfantryClass_Idle, 5)
{
	GET(InfantryClass*, I, ESI);
	const auto pData = InfantryTypeExt::ExtMap.Find(I->Type);

	// don't play idle when paralyzed
	if (I->IsUnderEMP())
	{
		R->BL(false);
		return 0x51CECD;
	}

	R->EDI(R->EAX()); // argh
	R->BL(pData->Is_Cow); // aaaargh! again!
	return pData->Is_Cow ? 0x51CEAEu : 0x51CECDu;
}

DEFINE_OVERRIDE_HOOK(0x51F76D, InfantryClass_Unload, 5)
{
	GET(InfantryClass*, I, ESI);
	return InfantryTypeExt::ExtMap.Find(I->Type)->Is_Deso ? 0x51F77Du : 0x51F792u;
}

DEFINE_OVERRIDE_HOOK(0x52138c, InfantryClass_UpdateDeployment_Deso2, 6)
{
	GET(InfantryClass*, I, ESI);
	return InfantryTypeExt::ExtMap.Find(I->Type)->Is_Deso ? 0x52139A : 0x5214B9;
}

DEFINE_OVERRIDE_HOOK(0x5215f9, InfantryClass_UpdateDeployment_Deso1, 6)
{
	GET(InfantryClass*, I, ESI);
	return InfantryTypeExt::ExtMap.Find(I->Type)->Is_Deso ? 0x5216B6 : 0x52160D;
}

DEFINE_OVERRIDE_HOOK(0x629804, ParasiteClass_UpdateSquiddy, 9)
{
	GET(ParasiteClass*, pThis, ESI);
	R->EAX(pThis->Owner->GetWeapon(Ares_ParasiteWeapon(pThis->Owner)));
	return 0x62980D;
}

DEFINE_OVERRIDE_HOOK(0x51C4C8, InfantryClass_IsCellOccupied, 6)
{
	GET(BuildingClass* const, pBld, ESI);

	enum { Impassable = 0x51C7D0, Ignore = 0x51C70F, NoDecision = 0x51C4EB, CheckFirestorm = 0x51C4D2 };

	if (Is_Passable(pBld->Type)) {
		return Ignore;
	}

	if (Is_FirestromWall(pBld->Type)) {
		return CheckFirestorm;
	}

	return NoDecision;
}

DEFINE_OVERRIDE_HOOK(0x514A21, HoverLocomotionClass_ILocomotion_Process_DeployToLand, 9)
{
	GET(ILocomotion*, ILoco, ESI);

	auto const bIsMovingNow = ILoco->Is_Moving_Now();
	R->AL(bIsMovingNow);
	auto const pOwner = static_cast<HoverLocomotionClass*>(ILoco)->Owner;

	if (pOwner->InAir)
	{
		auto const pType = pOwner->GetTechnoType();
		if (pType->DeployToLand)
		{
			auto pCell = pOwner->GetCell();
			auto nLand = pCell->LandType;
			if ((nLand == LandType::Beach || nLand == LandType::Water) && !pCell->ContainsBridge())
			{
				pOwner->InAir = false;
				pOwner->QueueMission(Mission::Guard, true);
			}

			if (bIsMovingNow)
			{
				ILoco->Stop_Moving();
				pOwner->SetDestination(nullptr, true);
			}

			if (pType->DeployingAnim)
			{
				auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
				int nDeployDirVal = pTypeExt->DeployDir.isset() ? (int)pTypeExt->DeployDir.Get() << 13 : RulesClass::Instance->DeployDir << 8;
				DirStruct nDeployDir(nDeployDirVal);

				if (pOwner->PrimaryFacing.Current() != nDeployDir) {
					pOwner->PrimaryFacing.Set_Desired(nDeployDir);
				}
			}

			if (pOwner->GetHeight() <= 0)
			{
				pOwner->InAir = false;
				ILoco->Mark_All_Occupation_Bits(0);
			}
		}
	}

	return 0x514A2A;
}

// restored from TS
DEFINE_OVERRIDE_HOOK(0x4F9610, HouseClass_GiveTiberium_Storage, 0xA)
{
	GET(HouseClass* const, pThis, ECX);
	GET_STACK(float, amount, 0x4);
	GET_STACK(int const, idxType, 0x8);

	pThis->SiloMoney += Game::F2I(amount * 5.0);

	if (SessionClass::Instance->GameMode == GameMode::Campaign || pThis->CurrentPlayer)
	{
		// don't change, old values are needed for silo update
		const auto lastStorage = int(pThis->OwnedTiberium.GetTotalAmount());
		const auto lastTotalStorage = pThis->TotalStorage;

		// this is the upper limit for stored tiberium
		if (amount > lastTotalStorage - lastStorage)
		{
			amount = float(lastTotalStorage - lastStorage);
		}

		// go through all buildings and fill them up until all is in there
		for (auto const& pBuilding : pThis->Buildings)
		{
			if (amount <= 0.0)
			{
				break;
			}

			auto const storage = pBuilding->Type->Storage;
			if (pBuilding->IsOnMap && storage > 0)
			{
				// put as much tiberium into this silo
				auto freeSpace = storage - pBuilding->Tiberium.GetTotalAmount();
				if (freeSpace > 0.0)
				{
					if (freeSpace > amount)
					{
						freeSpace = amount;
					}

					pBuilding->Tiberium.AddAmount(freeSpace, idxType);
					pThis->OwnedTiberium.AddAmount(freeSpace, idxType);

					amount -= freeSpace;
				}
			}
		}

		// redraw silos
		pThis->UpdateAllSilos(lastStorage, lastTotalStorage);
	}
	else
	{
		// just add the money. this is the only original YR logic
		auto const pTib = TiberiumClass::Array->GetItem(idxType);
		pThis->Balance += int(amount * pTib->Value * pThis->Type->IncomeMult);
	}

	return 0x4F9664;
}

DEFINE_OVERRIDE_HOOK(0x4F62FF, HouseClass_CTOR_FixNameOverflow, 6)
{
	GET(HouseClass *, H, EBP);
	GET_STACK(HouseTypeClass *, Country, 0x48);

	PhobosCRT::wstrCopy(H->UIName, Country->UIName);

	return 0x4F6312;
}

DEFINE_OVERRIDE_HOOK(0x4F645F, HouseClass_CTOR_FixSideIndices, 5)
{
	GET(HouseClass *, pHouse, EBP);
	if(HouseTypeClass * pCountry = pHouse->Type) {
		if(strcmp(pCountry->ID, GameStrings::Neutral()) &&
			strcmp(pCountry->ID, GameStrings::Special())) {
			pHouse->SideIndex = pCountry->SideIndex;
		}
	}
	return 0x4F6490;
}

void KickOutOfRubble(BuildingClass* pBld)
{
	std::vector<std::pair<FootClass*, bool>> list;

	// iterate over all cells and remove all infantry

	auto const location = MapClass::Instance->GetCellAt(pBld->Location)->MapCoords;
	// get the number of non-end-marker cells and a pointer to the cell data
	for (auto i = pBld->Type->FoundationData; *i != CellStruct{0x7FFF, 0x7FFF}; ++i) {
		// remove every techno that resides on this cell
		for (NextObject obj(MapClass::Instance->GetCellAt(location + *i)->
			 GetContent()); obj; ++obj) {
			if (auto const pFoot = abstract_cast<FootClass*>(*obj)) {
				if (pFoot->Limbo()) {
					list.push_back(std::make_pair(pFoot, pFoot->IsSelected));
				}
			}
		}
	}

	// this part kicks out all units we found in the rubble
	for (auto const& [pFoot, bIsSelected] : list)
	{
		if (pBld->KickOutUnit(pFoot, location) == KickOutResult::Succeeded) {
			if (bIsSelected) {
				pFoot->Select();
			}
		} else {
			pFoot->UnInit();
		}
	}
}

DEFINE_OVERRIDE_HOOK(0x441f2c ,BuildingClass_Destroy_KickOutOfRubble, 5)
{
	GET(BuildingClass*, pThis, ESI);

	const auto pTypeExt = BuildingTypeExt::ExtMap.Find(pThis->Type);

	if (pTypeExt->RubbleDestroyed || pTypeExt->RubbleIntact)
		KickOutOfRubble(pThis);

	return 0x0;
}

DEFINE_OVERRIDE_SKIP_HOOK(0x6F4103, TechnoClass_Init_ThisPartHandled, 6, 6F41C0)

//DEFINE_HOOK_AGAIN(0x747705, TechnoTypeClass_ReadSpeedType_Validate, 0x6)
//DEFINE_HOOK(0x7121E5, TechnoTypeClass_ReadSpeedType_Validate, 0x6)
//{
//	GET(TechnoTypeClass*, pThis, EBP);
//
//	if (pThis->SpeedType == SpeedType::None)
//	{
//
//		const auto nWhat = pThis->WhatAmI();
//		if (nWhat == UnitTypeClass::AbsID || nWhat == AircraftTypeClass::AbsID || nWhat == InfantryTypeClass::AbsID)
//			Debug::INIParseFailed(pThis->ID, GameStrings::SpeedType(), "SpeedType::None(-1)", "Expect Valid SpeedType");
//	}
//
//	return 0x0;
//}
