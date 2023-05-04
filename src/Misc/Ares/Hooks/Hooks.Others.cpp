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

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/Terrain/Body.h>

#include <TerrainTypeClass.h>
#include <New/Type/ArmorTypeClass.h>

#include <Notifications.h>

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

DEFINE_OVERRIDE_HOOK(0x5F6515, AbstractClass_Distance2DSquared_1, 0x8)
{
	GET(AbstractClass*, pThis, ECX);
	GET(AbstractClass*, pThat, EBX);

	auto const nThisCoord = pThis->GetCoords();
	auto const nThatCoord = pThat->GetCoords();
	auto const nXY =
		((nThisCoord.Y - nThatCoord.Y) * (nThisCoord.Y - nThatCoord.Y)) +
		((nThisCoord.X - nThatCoord.X) * (nThisCoord.X - nThatCoord.X));

	R->EAX(nXY >= INT_MAX ? INT_MAX : nXY);
	return 0x5F6559;
}

DEFINE_OVERRIDE_HOOK(0x5F6560, AbstractClass_Distance2DSquared_2, 5)
{
	GET(AbstractClass*, pThis, ECX);
	auto const nThisCoord = pThis->GetCoords();
	GET_STACK(CoordStruct*, pThatCoord, 0x4);

	auto const nXY =
		((nThisCoord.Y - pThatCoord->Y) * (nThisCoord.Y - pThatCoord->Y)) +
		((nThisCoord.X - pThatCoord->X) * (nThisCoord.X - pThatCoord->X));

	R->EAX(nXY >= INT_MAX ? INT_MAX : nXY);
	return 0x5F659B;
}

DEFINE_OVERRIDE_SKIP_HOOK(0x6AD0ED, Game_AllowSinglePlay, 0x5, 6AD16C);

std::vector<unsigned char> ShpCompression1Buffer;

DEFINE_OVERRIDE_HOOK(0x437CCC, BSurface_DrawSHPFrame1_Buffer, 0x8)
{
	REF_STACK(RectangleStruct const, bounds, STACK_OFFS(0x7C, 0x10));
	//0x89C568
	REF_STACK(unsigned char const*, pBuffer, STACK_OFFS(0x7C, 0x6C));

	auto const width = static_cast<size_t>(std::clamp<short>(
		static_cast<short>(bounds.Width), 0, std::numeric_limits<short>::max()));

	// buffer overrun is now not as forgiving as it was before
	auto& Buffer = ShpCompression1Buffer;
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

	int ret = fallback;
	if (pINI->ReadString(pSection, pKey, Phobos::readDefval, Phobos::readBuffer))
	{
		// find the pip value with the name specified
		const auto it = std::find_if(TechnoTypeClass::PipsTypeName.begin(), TechnoTypeClass::PipsTypeName.end(),
			[](NamedValue<int>const& Data) {
			 return IS_SAME_STR_(Data.Name, Phobos::readBuffer);
		});

		if (it != TechnoTypeClass::PipsTypeName.end())
		{
			ret = it->Value;

		}
		else if (!Parser<int>::TryParse(Phobos::readBuffer, &ret))
		{
			Debug::INIParseFailed(pSection, pKey, Phobos::readBuffer);
			ret = fallback;
		}
	}

	R->EAX(ret);
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
	if (++ * pIter)
	{
		return 0x4895C0;
	}

	// all done. delete and go on
	delete pIter;

	return 0x4899DA;
}

DEFINE_OVERRIDE_HOOK(0x5F57B5, ObjectClass_ReceiveDamage_Trigger, 0x6)
{
	GET(ObjectClass*, pObject, ESI);
	GET(ObjectClass*, pAttacker, EDI);

	if (R->EBP<DamageState>() != DamageState::NowDead && pObject->IsAlive)
	{
		auto v3 = pObject->AttachedTag;
		if (!v3 || (v3->RaiseEvent(TriggerEvent::AttackedByAnybody, pObject, CellStruct::Empty, false, pAttacker), pObject->IsAlive))
		{
			if (auto v4 = pObject->AttachedTag)
				v4->RaiseEvent(TriggerEvent::AttackedByHouse, pObject, CellStruct::Empty, false, pAttacker);
		}
	}

	if (pObject->IsAlive)
	{
		auto pFirstTag = pObject->AttachedTag;
		//84
		if (!pFirstTag || (pFirstTag->RaiseEvent((TriggerEvent)AresTriggerEvents::AttackedOrDestroyedByHouse, pObject, CellStruct::Empty, false, pAttacker), pObject->IsAlive))
		{
			if (auto pSecondTag = pObject->AttachedTag)
				// 83
				pSecondTag->RaiseEvent((TriggerEvent)AresTriggerEvents::AttackedOrDestroyedByAnybody, pObject, CellStruct::Empty, false, pAttacker);
		}
	}

	return 0x5F580C;
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

DEFINE_OVERRIDE_SKIP_HOOK(0x6BB9DD, WinMain_LogNonsense, 5, 6BBE2B)

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

		if (auto pSys = pThis->ParticleSystem)
		{
			if (!pSys->Particles.Remove(pThis))
			{
				return 0x725C08;
			}
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

			if (AnimType->MakeInfantry > -1)
			{
				AnimTypeExt::SetMakeInfOwner(pAnim, pOwner, nullptr);
			}
			else
			{
				AnimExt::SetAnimOwnerHouseKind(pAnim, pOwner, nullptr, false);
			}
		}
	}

	R->EAX(1);
	return 0x6E2387;
}

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
		Debug::Log("TechnoClass_FireAt Aircraft[%s] attempting to fire Railgun ! \n", pThis->get_ID());
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
	Debug::Log("Overlay %s not really tiberium \n", pThis->ID);
	return 0x5FDDC1;
}

DEFINE_OVERRIDE_HOOK(0x716D98, TechnoTypeClass_Load_Palette, 0x5)
{
	GET(TechnoTypeClass*, pThis, EDI);

	pThis->Palette = nullptr;
	return pThis->PaletteFile[0] == 0 ? 0x716DAA : 0x716D9D;
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

SuperClass* TempData;
DEFINE_OVERRIDE_HOOK(0x6CC390, SuperClass_Launch, 0x6)
{
	GET(SuperClass* const, pSuper, ECX);
	GET_STACK(CellStruct* const, pCell, 0x4);
	GET_STACK(bool const, isPlayer, 0x8);

	auto pSuperExt = SuperExt::ExtMap.Find(pSuper);
	pSuperExt->Temp_IsPlayer = isPlayer;
	pSuperExt->Temp_CellStruct = *pCell;

	if (AresData::SW_Activate(pSuper, *pCell, isPlayer))
	{
		Debug::Log("[LAUNCH] %s Handled \n", pSuper->Type->ID);
		const auto pSWExt = SWTypeExt::ExtMap.Find(pSuper->Type);
		pSWExt->FireSuperWeapon(pSuper, pSuper->Owner, pCell, isPlayer);
		return 0x6CDE40;
	}
	else
	{
		TempData = pSuper;
	}

	return 0x0;
}

DEFINE_HOOK_AGAIN(0x6CD065, SuperClass_Launch_NotHandled, 0xA)
DEFINE_HOOK_AGAIN(0x6CCE57, SuperClass_Launch_NotHandled, 0xA)
DEFINE_HOOK_AGAIN(0x6CCDB0, SuperClass_Launch_NotHandled, 0xA)
DEFINE_HOOK_AGAIN(0x6CCD32, SuperClass_Launch_NotHandled, 0xA)
DEFINE_HOOK_AGAIN(0x6CC4A5, SuperClass_Launch_NotHandled, 0xA)
DEFINE_HOOK_AGAIN(0x6CC478, SuperClass_Launch_NotHandled, 0xA)
DEFINE_HOOK_AGAIN(0x6CD2E1, SuperClass_Launch_NotHandled, 0xA)
DEFINE_HOOK_AGAIN(0x6CD52A, SuperClass_Launch_NotHandled, 0xA)
DEFINE_HOOK_AGAIN(0x6CDCE3, SuperClass_Launch_NotHandled, 0xA)
DEFINE_HOOK(0x6CDE36, SuperClass_Launch_NotHandled, 0xA)
{
	if (const auto pSuper = TempData)
	{
		Debug::Log("[LAUNCH] %s Normal at [0x%x] \n", pSuper->Type->ID, R->Origin());
		const auto pSuperExt = SuperExt::ExtMap.Find(pSuper);
		const auto pSWExt = SWTypeExt::ExtMap.Find(pSuper->Type);
		pSWExt->FireSuperWeapon(pSuper, pSuper->Owner, &pSuperExt->Temp_CellStruct, pSuperExt->Temp_IsPlayer);
		TempData = nullptr;
	}
	else
	{
		Debug::Log("[LAUNCH] SW as Normal but SuperPointer is somewhat invalid at [0x%x]! \n", R->Origin());
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

				// go boom
				WarheadTypeClass* pWH = RulesExt::Global()->Tiberium_ExplosiveWarhead;
				if (morePower > 0 && pWH)
				{
					CoordStruct crd = pThis->GetCoords();
					MapClass::DamageArea(crd, morePower, pThis, pWH, pWH->Tiberium, pThis->Owner);
				}
			}
		}
	}

	return 0x7387C4;
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

	//GET_STACK(AbstractClass*, pKiller, 0x4);
	const auto pType = pThis->GetTechnoType();
	const auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (pExt->SupressEVALost
		|| pType->DontScore
		|| pType->Insignificant
		|| pType->Spawned
		|| !pThis->Owner->ControlledByPlayer()
	)
	{
		return Skip;
	}

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	if (pTypeExt->EVA_UnitLost != -1) {
		RadarEventClass::Create(RadarEventType::UnitLost, pThis->GetMapCoords());
		VoxClass::PlayIndex(pTypeExt->EVA_UnitLost.Get(), -1, -1);
	}

	return Skip;
}

DEFINE_OVERRIDE_HOOK(0x44D760, BuildingClass_Destroyed_UnitLost, 7)
{
	GET(BuildingClass*, pThis, ECX);

	const auto pType = pThis->Type;

	if (!pType->Insignificant && !pType->DontScore)
	{
		if (pThis->Owner && pThis->Owner->IsControlledByHuman())
		{
			auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pType);
			auto pTechnoExt = TechnoExt::ExtMap.Find(pThis);

			int lostIdx = pTechnoTypeExt->EVA_UnitLost;
			if (lostIdx >= 0 && !pTechnoExt->SupressEVALost)
			{
				VoxClass::PlayIndex(lostIdx);
			}
		}
	}

	return 0;
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
	GET(TechnoTypeClass*, pThis, ECX);
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis);
	auto const nSeed = pTypeExt->BuildTime_Speed.Get(RulesClass::Instance->BuildSpeed);
	auto const nCost = pTypeExt->BuildTime_Cost.Get(pThis->Cost);

	R->EAX(nSeed * nCost / 1000.0 * 900.0);
	return 0x711EDE;
}

DEFINE_OVERRIDE_HOOK(0x6AB8BB, SelectClass_ProcessInput_BuildTime, 6)
{
	GET(BuildingTypeClass*, pBuildingProduct, ESI);
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pBuildingProduct);

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

DEFINE_OVERRIDE_HOOK(0x70BE80, TechnoClass_ShouldSelfHealOneStep, 5)
{
	GET(TechnoClass* const, pThis, ECX);
	auto const nAmount = AresData::GetSelfHealAmount(pThis);
	R->EAX(nAmount > 0 || nAmount != 0);
	return 0x70BF46;
}

//DEFINE_HOOK(0x6FA793, TechnoClass_AI_SelfHealGain, 0x5)
//{
//	enum { SkipGameSelfHeal = 0x6FA941 };
//
//	GET(TechnoClass*, pThis, ESI);
//	TechnoExt::ApplyGainedSelfHeal(pThis);
//	return SkipGameSelfHeal;
//}

DEFINE_OVERRIDE_HOOK(0x6FA743, TechnoClass_Update_SelfHeal, 0xA)
{
	enum { 
		ContineCheckUpdateSelfHeal = 0x6FA75A ,
		SkipAnySelfHeal = 0x6FA941,
	};

	GET(TechnoClass* const, pThis, ESI);

	if (!pThis->IsAlive || pThis->InLimbo) {
		return SkipAnySelfHeal;
	}

	// prevent crashing and sinking technos from self-healing
	if (pThis->IsCrashing || pThis->IsSinking || Is_DriverKilled(pThis)) {
		return SkipAnySelfHeal;
	}

	const auto nUnit = specific_cast<UnitClass*>(pThis);
	if (nUnit && nUnit->DeathFrameCounter > 0) {
		return SkipAnySelfHeal;
	}

	// this replaces the call to pThis->ShouldSelfHealOneStep()
	const auto nAmount = AresData::GetSelfHealAmount(pThis);

	if (nAmount > 0 || nAmount != 0) {

		const bool wasDamaged = pThis->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow;

		pThis->Health += nAmount;

		if (wasDamaged && (pThis->GetHealthPercentage() > RulesClass::Instance->ConditionYellow
			|| pThis->GetHeight() < -10))
		{
			const auto pWhat = GetVtableAddr(pThis);
			const bool isBuilding = pWhat == BuildingClass::vtable;

			if (isBuilding) {
				const auto pBuilding = static_cast<BuildingClass*>(pThis);
				pBuilding->UpdatePlacement(PlacementType::Redraw);
				pBuilding->ToggleDamagedAnims(false);
			}

			if (pWhat == UnitClass::vtable || isBuilding) {
				if (auto& dmgParticle = pThis->DamageParticleSystem) {
					dmgParticle->UnInit();
					dmgParticle = nullptr;
				}
			}
		}		
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
	R->EAX(int(256.0 * pThis->CloakProgress.Value / stages));
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
	pThis->ForceShielded = force ? TRUE : FALSE;

	R->EAX(DamageState::Unaffected);
	return 0x70E2FD;
}

DEFINE_OVERRIDE_HOOK(0x7327AA, TechnoClass_PlayerOwnedAliveAndNamed_GroupAs, 8)
{
	GET(TechnoClass*, pThis, ESI);
	GET(const char*, pID, EDI);

	bool ret = TechnoTypeExt::HasSelectionGroupID(pThis->GetTechnoType(), pID);

	R->EAX<int>(ret);
	return 0x7327B2;
}

// #912875: respect the remove flag for invalidating SpawnManager owners
DEFINE_OVERRIDE_HOOK(0x707B19, TechnoClass_PointerGotInvalid_SpawnCloakOwner, 6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(void*, ptr, EBP);
	REF_STACK(bool, remove, STACK_OFFS(0x20, -0x8));

	if (const auto pSM = pThis->SpawnManager)
	{
		// ignore disappearing owner
		if (remove || pSM->Owner != ptr)
		{
			R->ECX(pSM);
			return 0x707B23;
		}
	}

	return 0x707B29;
}

CDTimerClass CloakEVASpeak {};
CDTimerClass SubTerraneanEVASpeak {};

void PlayEva(const char* pEva, CDTimerClass& nTimer, double nRate)
{
	if (!nTimer.GetTimeLeft())
	{
		nTimer.Start(GameOptionsClass::Instance->GetAnimSpeed(static_cast<int>(nRate * 900.0)));
		VoxClass::Play(pEva);
	}
}

DEFINE_OVERRIDE_HOOK(0x70DA95, TechnoClass_RadarTrackingUpdate_AnnounceDetected, 6)
{
	GET(TechnoClass*, pThis, ESI);
	GET_STACK(int, detect, 0x10);

	auto pType = pThis->GetTechnoType();
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	if (detect && pTypeExt->SensorArray_Warn)
	{
		switch (detect)
		{
		case 1:
			PlayEva("EVA_CloakedUnitDetected", CloakEVASpeak, RulesExt::Global()->StealthSpeakDelay);
			break;
		case 2:
			PlayEva("EVA_SubterraneanUnitDetected", SubTerraneanEVASpeak, RulesExt::Global()->SubterraneanSpeakDelay);
			break;
		}

		CellStruct cell = CellClass::Coord2Cell(pThis->Location);
		RadarEventClass::Create(RadarEventType::EnemySensed, cell);
	}

	return 0x70DADC;
}

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

	if (!HouseClass::CurrentPlayer->SpySatActive || --pCell->GapsCoveringThisCell > 0)
		return 0x6FB69E;

	--pCell->ShroudCounter;

	if (pCell->ShroudCounter <= 0)
		pCell->AltFlags |= AltCellFlags::Clear;

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
		pCell->AltFlags &= ((AltCellFlags)4294967271u);

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
	if (const auto pConvertData = pTypeExt->Palette) {
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

DEFINE_OVERRIDE_HOOK(0x736135, UnitClass_Update_Deactivated, 6)
{
	GET(UnitClass*, pThis, ESI);

	// don't sparkle on EMP, Operator, ....
	return TechnoExt_Stuffs::IsPowered(pThis)
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

						pThis->UnInit();
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
	if (nCurWp < nVec.size())
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
		if (Is_Operated(pThis ) || AresData::IsOperated(pThis))
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

// temporal per-slot
DEFINE_OVERRIDE_HOOK(0x71A84E, TemporalClass_UpdateA, 5)
{
	GET(TemporalClass* const, pThis, ESI);

	// it's not guaranteed that there is a target
	if (auto const pTarget = pThis->Target)
	{
		const auto nJammer = RadarJammerUptr(pTarget);

		if (nJammer)
		{
			RadarJammerUptr(pTarget) = nullptr;
			AresData::JammerClassUnjamAll(nJammer);
			AresMemory::Delete(nJammer);		
		}

		//AttachEffect handling under Temporal
		auto const AEDataPtr = &GetAEData(pTarget);
		AresData::UpdateAEData(AEDataPtr);
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
		const auto pBld = static_cast<BuildingClass*>(pThis);
		const auto pRism = &PrimsForwardingPtr(pBld);
		AresData::CPrismRemoveFromNetwork(pRism, true);
	}

	return 0;
}
//TODO : 
// better port these
// DEFINE_HOOK(6FB757, TechnoClass_UpdateCloak, 8)

//TODO :
// better port these :s
// DEFINE_HOOK(6FAF0D, TechnoClass_Update_EMPLock, 6)

/* TODO : Addition Weapon shenanegans , need to port whole TechnoClass::Update
DEFINE_HOOK(717890, TechnoTypeClass_SetWeaponTurretIndex, 8)
{
	GET(TechnoTypeClass*, pThis, ECX);
	GET_STACK(int, nTurIdx, 0x4);
	GET_STACK(int, nWeaponIdx, 0x8);

	if (nWeaponIdx < TechnoTypeClass::MaxWeapons)
	{
		pThis->TurretWeapon[nWeaponIdx] = nTurIdx;
	}
	else
	{
		auto const pExt = TechnoTypeExt::ExtMap.Find(pThis);
		pExt->AdditionalTurrentWeapon[nWeaponIdx - TechnoTypeClass::MaxWeapons] = nTurIdx;
	}

	return 0x71789F;
}
*/