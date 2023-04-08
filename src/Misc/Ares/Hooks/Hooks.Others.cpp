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

DEFINE_OVERRIDE_HOOK_AGAIN(0x42511B, AnimClass_Expired_ScorchFlamer, 0x7)
DEFINE_OVERRIDE_HOOK_AGAIN(0x4250C9, AnimClass_Expired_ScorchFlamer, 0x7)
DEFINE_OVERRIDE_HOOK(0x42513F, AnimClass_Expired_ScorchFlamer, 0x7)
{
	GET(AnimClass*, pThis, ESI);
	auto pType = pThis->Type;

	CoordStruct crd = pThis->GetCoords();

	auto SpawnAnim = [&crd](AnimTypeClass* pType, int dist)
	{
		if (!pType)
		{
			return static_cast<AnimClass*>(nullptr);
		}

		CoordStruct crdAnim = crd;
		if (dist > 0)
		{
			auto crdNear = MapClass::GetRandomCoordsNear(crd, dist, false);
			crdAnim = MapClass::PickInfantrySublocation(crdNear, true);
		}

		auto count = ScenarioClass::Instance->Random.RandomRanged(1, 2);
		return GameCreate<AnimClass>(pType, crdAnim, 0, count, 0x600u, 0, false);
	};

	if (pType->Flamer)
	{
		// always create at least one small fire
		if (auto const pAnim1 = SpawnAnim(RulesClass::Instance->SmallFire, 64))
			AnimExt::SetAnimOwnerHouseKind(pAnim1, pAnim1->Owner, nullptr);

		// 50% to create another small fire
		if (ScenarioClass::Instance->Random.RandomFromMax(99) < 50)
		{
			if (auto const pAnim2 = SpawnAnim(RulesClass::Instance->SmallFire, 160))
				AnimExt::SetAnimOwnerHouseKind(pAnim2, pAnim2->Owner, nullptr);
		}

		// 50% chance to create a large fire
		if (ScenarioClass::Instance->Random.RandomFromMax(99) < 50)
		{
			if (auto const pAnim3 = SpawnAnim(RulesClass::Instance->LargeFire, 112))
				AnimExt::SetAnimOwnerHouseKind(pAnim3, pAnim3->Owner, nullptr);
		}

	}
	else if (pType->Scorch)
	{
		// creates a SmallFire anim that is attached to the same object
		// this anim is attached to.
		if (pThis->GetHeight() < 10)
		{
			switch (pThis->GetCell()->LandType)
			{
			case LandType::Water:
			case LandType::Beach:
			case LandType::Ice:
			case LandType::Rock:
				break;
			default:
				if (auto pAnim = SpawnAnim(RulesClass::Instance->SmallFire, 0))
				{
					if (pThis->OwnerObject)
					{
						pAnim->SetOwnerObject(pThis->OwnerObject);
					}
				}
			}
		}
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x420A71, AlphaShapeClass_CTOR_Anims, 0x5)
{
	GET(AlphaShapeClass*, pThis, ESI);

	if (Is_Anim(pThis))
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

DEFINE_OVERRIDE_HOOK(0x41E893, AITriggerTypeClass_ConditionMet_SideIndex, 0xA)
{
	GET(HouseClass*, House, EDI);
	GET(int, triggerSide, EAX);

	enum { Eligible = 0x41E8D7, NotEligible = 0x41E8A1 };

	if (!triggerSide)
	{
		return Eligible;
	}

	--triggerSide;
	return(triggerSide == House->SideIndex)
		? Eligible
		: NotEligible
		;
}

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

DEFINE_OVERRIDE_HOOK(0x6AD0ED, Game_AllowSinglePlay, 0x5)
{
	return 0x6AD16C;
}

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
DEFINE_OVERRIDE_HOOK(0x55CFDF, CopyProtection_DontBlowMeUp, 0x0)
{
	return 0x55D059;
}

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
		const auto it = std::find_if(TechnoTypeClass::PipsTypeName.begin(), TechnoTypeClass::PipsTypeName.end(), [](NamedValue<int>const& Data)
{
	return CRT::strcmpi(Data.Name, Phobos::readBuffer) == 0;
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
		if (!pFirstTag || (pFirstTag->RaiseEvent((TriggerEvent)AresNewTriggerEvents::AttackedOrDestroyedByHouse, pObject, CellStruct::Empty, false, pAttacker), pObject->IsAlive))
		{
			if (auto pSecondTag = pObject->AttachedTag)
				// 83
				pSecondTag->RaiseEvent((TriggerEvent)AresNewTriggerEvents::AttackedOrDestroyedByAnybody, pObject, CellStruct::Empty, false, pAttacker);
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
	std::sort(nBegin, nEnd, [](ObjectClass* A, ObjectClass* B)
 {
	 return A && B && A->IsAlive && B->IsAlive && A->GetYSort() < B->GetYSort();
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
//
DEFINE_OVERRIDE_SKIP_HOOK(0x6BB9DD, WinMain_LogNonsense, 5, 6BBE2B)

// bugfix #187: Westwood idiocy
DEFINE_OVERRIDE_SKIP_HOOK(0x531726, StupidPips1, 5, 53173A)

// bugfix #187: Westwood idiocy
DEFINE_OVERRIDE_SKIP_HOOK(0x53173F, StupidPips2, 5, 531749)

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

	auto const pType = pThis->Type;
	auto const nRadius = pType->DamageRadius;

	if (nRadius < 0 || !pType->Damage || !pType->Warhead)
		return 0x74A934;

	auto const nCoord = pThis->Bounce.GetCoords();
	auto const pCell = MapClass::Instance->GetCellAt(nCoord);
	auto const pInvoker = VoxelAnimExt::GetTechnoOwner(pThis);

	for (NextObject j(pCell->GetContent()); j; ++j)
	{
		auto const pObj = *j;
		auto const nLoc = pObj->Location;
		auto const nDist = abs(nLoc.X - nCoord.X) + abs(nLoc.Y - nCoord.Y);

		if (nDist < nRadius)
		{
			auto nDamage = pType->Damage;
			pObj->ReceiveDamage(&nDamage, TacticalClass::AdjustForZ(nDist), pType->Warhead, pInvoker, false, false, pInvoker ? pInvoker->Owner : nullptr);
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

// issue #1437: crash when warping out buildings infantry wants to garrison
DEFINE_OVERRIDE_HOOK(0x71AA52, TemporalClass_Update_AnnounceInvalidPointer, 0x8)
{
	GET(TechnoClass*, pVictim, ECX);
	pVictim->IsAlive = false;
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

// MakeInfantry that fails to place will just end the source animation and cleanup instead of memleaking to game end
DEFINE_OVERRIDE_HOOK(0x424B23, AnimClass_Update_FailedToUnlimboInfantry, 0x6)
{
	GET(AnimClass*, pThis, ESI);
	GET(InfantryClass*, pInf, EDI);

	pInf->UnInit();
	pThis->TimeToDie = 1;
	pThis->UnInit();

	return 0x424B29;
}

DEFINE_OVERRIDE_HOOK(0x4239F0, AnimClass_UpdateBounce_Damage, 0x8)
{
	enum
	{
		DoNotDealDamage = 0x423A92,
		DealDamage = 0x4239F8,
		GoToNext = 0x423A83,
	};

	GET(ObjectClass*, pObj, EDI);
	GET(AnimClass*, pThis, EBP);

	auto const pType = pThis->Type;
	auto const nRadius = pType->DamageRadius;

	if (!pObj || nRadius < 0 || CLOSE_ENOUGH(pType->Damage, 0.0) || !pType->Warhead)
		return DoNotDealDamage;

	auto const nCoord = pThis->Bounce.GetCoords();
	auto const pAnimTypeExt = AnimTypeExt::ExtMap.Find(pType);
	TechnoClass* const pInvoker = AnimExt::GetTechnoInvoker(pThis, pAnimTypeExt->Damage_DealtByInvoker);
	auto const nLoc = pObj->Location;
	auto const nDist = abs(nLoc.Y - nCoord.Y) + abs(nLoc.X - nCoord.X);

	if (nDist < nRadius)
	{
		auto nDamage = (int)pType->Damage;
		pObj->ReceiveDamage(&nDamage, TacticalClass::AdjustForZ(nDist), pType->Warhead, pInvoker, false, false, pInvoker ? pInvoker->Owner : pThis->Owner);
	}

	//return !pObj || !pType->Warhead ||
	//	pType->DamageRadius < 0 || pType->Damage == 0.0 ?
	//	DoNotDealDamage : DealDamage;
	return GoToNext;
}

DEFINE_OVERRIDE_HOOK(0x4242CA, AnimClass_Update_FixIE_TrailerSeperation, 0x6)
{
	enum
	{
		PlayTrail = 0x4242D5,
		SkopTrail = 0x424322,
	};

	GET(AnimTypeClass*, AT, EAX);
	int trailSep = AT->TrailerSeperation;

	R->ECX(trailSep);

	return trailSep >= 1
		? PlayTrail : SkopTrail;
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

		AnimClass* pAnim = GameCreate<AnimClass>(AnimType, nCoord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false);

		if (AnimType->MakeInfantry > -1)
		{
			AnimTypeExt::SetMakeInfOwner(pAnim, pOwner, nullptr);
		}

		AnimExt::SetAnimOwnerHouseKind(pAnim, pOwner, pOwner, false);
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

DEFINE_OVERRIDE_HOOK(0x6EF8A1, TeamClass_GatherAtEnemyBase_Distance, 0x6)
{
	GET_BASE(ScriptActionNode*, pTeamM, 0x8);
	R->EDX(RulesClass::Instance->AISafeDistance + pTeamM->Argument);
	return 0x6EF8A7;
}

DEFINE_OVERRIDE_HOOK(0x6EFB69, TeamClass_GatherAtFriendlyBase_Distance, 0x6)
{
	GET_BASE(ScriptActionNode*, pTeamM, 0x8);
	R->EDX(RulesExt::Global()->AIFriendlyDistance.Get(RulesClass::Instance->AISafeDistance) + pTeamM->Argument);
	return 0x6EFB6F;
}

DEFINE_OVERRIDE_HOOK(0x5FDDA4, OverlayClass_GetTiberiumType_NotReallyTiberiumLog, 0x6)
{
	GET(OverlayTypeClass*, pThis, EAX);
	Debug::Log("Overlay %s not really tiberium \n", pThis->ID);
	return 0x5FDDC1;
}

DEFINE_OVERRIDE_HOOK(0x424538, AnimClass_AI_DamageDelay, 0x6)
{
	enum
	{
		SkipDamageDelay = 0x42465D,
		CheckIsAlive = 0x42464C
	};

	GET(AnimClass*, pThis, ESI);
	return AnimExt::DealDamageDelay(pThis) ? SkipDamageDelay : CheckIsAlive;
}

DEFINE_OVERRIDE_HOOK(0x701A5C, TechnoClass_ReceiveDamage_IronCurtainFlash, 0x7)
{
	GET_STACK(WarheadTypeClass*, pWh, 0xD0);
	GET(TechnoClass*, pThis, ESI);

	if (!WarheadTypeExt::ExtMap.Find(pWh)->IC_Flash.Get(RulesExt::Global()->IC_Flash.Get()))
		return 0x701A98;

	return (pThis->ForceShielded == 1) ? 0x701A65 : 0x701A69;
}

DEFINE_OVERRIDE_HOOK(0x701914, TechnoClass_ReceiveDamage_Damaging, 0x7)
{
	R->Stack(0xE, R->EAX() > 0);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x7021F5, TechnoClass_ReceiveDamage_OverrideDieSound, 0x6)
{
	GET_STACK(WarheadTypeClass*, pWh, 0xD0);
	GET(TechnoClass*, pThis, ESI);

	auto const& nSound = WarheadTypeExt::ExtMap.Find(pWh)->DieSound_Override;

	if (nSound.isset())
	{

		auto const nIdx = nSound.Get();
		if (nIdx >= 0)
			VocClass::PlayAt(nIdx, pThis->Location);

		return 0x702200;
	}

	return 0x0;
}

DEFINE_OVERRIDE_HOOK(0x702185, TechnoClass_ReceiveDamage_OverrideVoiceDie, 0x6)
{
	GET_STACK(WarheadTypeClass*, pWh, 0xD0);
	GET(TechnoClass*, pThis, ESI);

	auto const& nSound = WarheadTypeExt::ExtMap.Find(pWh)->VoiceSound_Override;

	if (nSound.isset())
	{

		auto const nIdx = nSound.Get();
		if (nIdx >= 0)
			VocClass::PlayAt(nIdx, pThis->Location);

		return 0x702200;
	}

	return 0x0;
}

DEFINE_OVERRIDE_HOOK(0x702CFE, TechnoClass_ReceiveDamage_PreventScatter, 0x6)
{
	GET(FootClass*, pThis, ESI);
	GET_STACK(WarheadTypeClass*, pWarhead, STACK_OFFS(0xC4, -0xC));

	auto pExt = WarheadTypeExt::ExtMap.Find(pWarhead);

	// only allow to scatter if not prevented
	if (!pExt->PreventScatter)
	{
		pThis->Scatter(CoordStruct::Empty, true, false);
	}

	return 0x702D11;
}

// #1283653: fix for jammed buildings and attackers in open topped transports
DEFINE_OVERRIDE_HOOK(0x702A38, TechnoClass_ReceiveDamage_OpenTopped, 0x7)
{
	REF_STACK(TechnoClass*, pAttacker, STACK_OFFS(0xC4, -0x10));

	// decide as if the transporter fired at this building
	if (pAttacker && pAttacker->InOpenToppedTransport && pAttacker->Transporter)
	{
		pAttacker = pAttacker->Transporter;
	}

	R->EDI(pAttacker);
	return 0x702A3F;
}

DEFINE_OVERRIDE_HOOK(0x702669, TechnoClass_ReceiveDamage_SuppressDeathWeapon, 0x9)
{
	GET(TechnoClass* const, pThis, ESI);
	GET_STACK(WarheadTypeClass* const, pWarhead, STACK_OFFS(0xC4, -0xC));

	if (!WarheadTypeExt::ExtMap.Find(pWarhead)->ApplySuppressDeathWeapon(pThis)) {
		pThis->FireDeathWeapon(0);
	}

	return 0x702672;
}

DEFINE_OVERRIDE_HOOK(0x517FC1, InfantryClass_ReceiveDamage_DeployedDamage, 0x6)
{

	GET(InfantryClass*, I, ESI);
	const bool IgnoreDefenses = R->BL() != 0;

	if (!I->IsDeployed() || IgnoreDefenses)
	{
		return 0;
	}

	GET(WarheadTypeClass*, pWH, EBP);
	GET(int*, pDamage, EDI);

	// yes, let's make sure the pointer's safe AFTER we've dereferenced it... Failstwood!
	if (pWH)
	{
		*pDamage = static_cast<int>(*pDamage * WarheadTypeExt::ExtMap.Find(pWH)->DeployedDamage);
		return 0x517FF9u;
	}

	return 0x518016u;
}

DEFINE_OVERRIDE_HOOK(0x716D98, TechnoTypeClass_Load_Palette, 0x5)
{
	GET(TechnoTypeClass*, pThis, EDI);

	if (pThis->PaletteFile[0] == 0)
	{
		return 0x716DAA;
	}

	pThis->Palette = nullptr;
	return 0x716D9D;
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

	if (auto const pOwner = pThis->SlaveOwner)
	{

		if (!pOwner->IsSelected || pThis->InLimbo)
			return 0x0;

		Drawing::DrawLinesTo(pOwner->GetRenderCoords(), pThis->Location, pOwner->Owner->Color);
	}

	if (auto const pOwner = pThis->SpawnOwner)
	{

		if (!pOwner->IsSelected || pThis->InLimbo)
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

	if(pThis->IsTethered)
		DrawTheStuff(L"IsTethered");

	return 0x0;
}


#define GetAresAresWarheadTypeExt(wh) ((void*)wh->unused_1CC)

#define Is_MaliciousWH(wh) (*(bool*)(((char*)wh->unused_1CC) + 0x75))

/*
 * Fixing issue #722
 */
DEFINE_OVERRIDE_HOOK(0x7384BD, UnitClass_ReceiveDamage_OreMinerUnderAttack, 6)
{
	GET_STACK(WarheadTypeClass*, WH, STACK_OFFS(0x44, -0xC));
	return !Is_MaliciousWH(WH) ? 0x738535u : 0u;
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

	Debug::Log("[LAUNCH] %s Handled \n", pSuper->Type->ID);
	auto pSuperExt = SuperExt::ExtMap.Find(pSuper);
	pSuperExt->Temp_IsPlayer = isPlayer;
	pSuperExt->Temp_CellStruct = *pCell;

	if (AresData::SW_Activate(pSuper, *pCell, isPlayer))
	{
		const auto pSWExt = SWTypeExt::ExtMap.Find(pSuper->Type);
		pSWExt->FireSuperWeapon(pSuper, pSuper->Owner, pCell, isPlayer);
		return 0x6CDE40;
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
	GET(SuperClass*, pSuper, ESI);

	if (Is_SW(pSuper)) {
		Debug::Log("[LAUNCH] %s Normal at [0x%x] \n", pSuper->Type->ID, R->Origin());
		const auto pSuperExt = SuperExt::ExtMap.Find(pSuper);
		const auto pSWExt = SWTypeExt::ExtMap.Find(pSuper->Type);
		pSWExt->FireSuperWeapon(pSuper, pSuper->Owner, &pSuperExt->Temp_CellStruct, pSuperExt->Temp_IsPlayer);
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

	if (RulesClass::Instance->TiberiumExplosive) {
		if (!ScenarioClass::Instance->SpecialFlags.StructEd.HarvesterImmune) {
			if (pThis->Tiberium.GetTotalAmount() > 0.0f) {

				// multiply the amounts with their powers and sum them up
				int morePower = 0;
				for (int i = 0; i < TiberiumClass::Array->Count; ++i) {
					TiberiumClass* pTiberium = TiberiumClass::Array->GetItem(i);
					morePower += int(pThis->Tiberium.Tiberiums[i] * pTiberium->Power);
				}

				// go boom
				WarheadTypeClass* pWH = RulesExt::Global()->Tiberium_ExplosiveWarhead;
				if (morePower > 0 && pWH) {
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
	if (flammability > 0.0) {
		if (pThis->IsBurning && ScenarioClass::Instance->Random.RandomFromMax(99) == 0) {
			const auto pCell = pThis->GetCell();

			// check all neighbour cells that contain terrain objects and
			// roll the dice for each of them.
			for (unsigned int i = 0; i < 8; ++i) {
				if (auto pTree = pCell->GetNeighbourCell(i)->GetTerrain(false)) {
					if (!pTree->IsBurning && ScenarioClass::Instance->Random.RandomDouble() < flammability) {
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

// TODO :
#define Ares_TemporalWeapon(var) (*(bool*)(((char*)GetAresTechnoExt(var)) + 0xA))

// issue 472: deglob WarpAway
DEFINE_OVERRIDE_HOOK(0x71A900, TemporalClass_Update_WarpAway, 6)
{
	GET(TemporalClass*, pThis, ESI);
	auto const pWeapon = pThis->Owner->GetWeapon(Ares_TemporalWeapon(pThis->Owner))->WeaponType;
	R->EDX<AnimTypeClass*>(WarheadTypeExt::ExtMap.Find(pWeapon->Warhead)->Temporal_WarpAway.Get(RulesClass::Global()->WarpAway));
	return 0x71A906;
}

// bugfix #379: Temporal friendly kills give veterancy
// bugfix #1266: Temporal kills gain double experience
// 
DEFINE_OVERRIDE_HOOK(0x71A917, TemporalClass_Update_Erase, 5)
{
	GET(TemporalClass*, pThis, ESI);

	auto pOwner = pThis->Owner;
	auto const pWeapon = pThis->Owner->GetWeapon(Ares_TemporalWeapon(pThis->Owner))->WeaponType;
	auto pOwnerExt = TechnoExt::ExtMap.Find(pOwner);
	auto pWarheadExt = WarheadTypeExt::ExtMap.Find(pWeapon->Warhead);

	if (pWarheadExt->Supress_LostEva)
		pOwnerExt->SupressEVALost = true;

	return 0x71A97D;
}

DEFINE_OVERRIDE_HOOK(0x702050, TechnoClass_ReceiveDamage_SuppressUnitLost, 6)
{
	GET(TechnoClass*, pThis, ESI);
	GET_STACK(WarheadTypeClass*, pWarhead, 0xD0);

	auto pWarheadExt = WarheadTypeExt::ExtMap.Find(pWarhead);
	auto pTechExt = TechnoExt::ExtMap.Find(pThis);

	if (pWarheadExt->Supress_LostEva.Get())
		pTechExt->SupressEVALost = true;

	return 0x0;
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

	if (pTypeExt->EVA_UnitLost != -1)
	{
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

int GetWarpPerStep(TemporalClass* pThis, int nStep)
{
	int nAddStep = 0;
	int nStepR = 0;

	for (; pThis;)
	{
		if (nStep > 50)
			break;

		++nStep;
		auto const pWeapon = pThis->Owner->GetWeapon(Ares_TemporalWeapon(pThis->Owner))->WeaponType;

		if (auto const pTarget = pThis->Target)
			nStepR = MapClass::GetTotalDamage(pWeapon->Damage, pWeapon->Warhead, pTarget->GetTechnoType()->Armor , 0);
		else
			nStepR = pWeapon->Damage;

		nAddStep += nStepR;
		pThis->WarpPerStep = nStepR;
		pThis = pThis->PrevTemporal;
	}

	return nAddStep;
}

DEFINE_OVERRIDE_HOOK(0x71AB10, TemporalClass_GetWarpPerStep, 6)
{
	GET_STACK(int, nStep, 0x4);
	GET(TemporalClass*, pThis, ESI);
	R->EAX(GetWarpPerStep(pThis, nStep));
	return 0x71AB57;
}

// bugfix #874 A: Temporal warheads affect Warpable=no units
// skip freeing captured and destroying spawned units,
// as it is not clear here if this is warpable at all.
DEFINE_OVERRIDE_SKIP_HOOK(0x71AF2B, TemporalClass_Fire_UnwarpableA,0xA, 71AF4D)

DEFINE_HOOK(0x71AC50, TemporalClass_LetItGo_ExpireEffect, 0x5)
{
	GET(TemporalClass* const, pThis, ESI);

	if (auto const pTarget = pThis->Target) {
		pTarget->UpdatePlacement(PlacementType::Redraw);

		auto nTotal = pThis->GetWarpPerStep();
		if (nTotal) {
			auto const pWeapon = pThis->Owner->GetWeapon(Ares_TemporalWeapon(pThis->Owner))->WeaponType;

			if (auto const Warhead = pWeapon->Warhead) {

				auto const pTempOwner = pThis->Owner;
				auto const peWHext = WarheadTypeExt::ExtMap.Find(Warhead);

				if (auto pExpireAnim = peWHext->TemporalExpiredAnim.Get()) {

					auto nCoord = pTarget->GetCenterCoords();

					if (auto const pAnim = GameCreate<AnimClass>(pExpireAnim, nCoord)) {
						pAnim->ZAdjust = pTarget->GetZAdjustment() - 3;
						AnimExt::SetAnimOwnerHouseKind(pAnim, pTempOwner->GetOwningHouse()
							, pTarget->GetOwningHouse(), pThis->Owner, false) ;
					}
				}

				if (peWHext->TemporalExpiredApplyDamage.Get())
				{
					auto const pTargetStreght = pTarget->GetTechnoType()->Strength;

					if (pThis->WarpRemaining > 0) {

						auto damage = int((pTargetStreght * ((1.0 - pThis->WarpRemaining / 10.0 / pTargetStreght)
							* (pWeapon->Damage * peWHext->TemporalDetachDamageFactor.Get()) / 100)));

						if (pTarget->IsAlive && !pTarget->IsSinking && !pTarget->IsCrashing)
							pTarget->ReceiveDamage(&damage, pTempOwner->DistanceFrom(pTarget), Warhead, pTempOwner, false, static_cast<bool>(ScenarioClass::Instance->Random(0, 1)), pTempOwner->Owner);
					}
				}
			}
		}
	}

	return 0x71AC5D;
}

DEFINE_OVERRIDE_HOOK(0x71AFB2, TemporalClass_Fire_HealthFactor, 5)
{
	GET(TechnoClass*, pTarget, ECX);
	GET(TemporalClass*, pThis, ESI);
	GET(int, nStreght, EAX);

	auto const pWeapon = pThis->Owner->GetWeapon(Ares_TemporalWeapon(pThis->Owner))->WeaponType;;
	const auto pWarhead = pWeapon->Warhead;
	const auto pWarheadExt = WarheadTypeExt::ExtMap.Find(pWarhead);
	const double nCalc = (1.0 - pTarget->Health / pTarget->GetTechnoType()->Strength) * pWarheadExt->Temporal_HealthFactor.Get();
	const double nCalc_b = (1.0 - nCalc) * (10 * nStreght) + nCalc * 0.0;

	R->EAX(nCalc_b <= 1.0 ? 1 : static_cast<int>(nCalc_b));
	return 0x71AFB7;
}

//TODO :
#define Ares_AboutToChronoshift(var) (*(bool*)(((char*)GetAresBuildingExt(var)) + 0xD))

bool Warpable(TechnoClass* pTarget)
{
	if (!pTarget || pTarget->IsSinking || pTarget->IsCrashing || pTarget->IsIronCurtained())
		return false;

	if (TechnoExt::HasAbility(pTarget, PhobosAbilityType::Unwarpable))
		return false;

	if (Is_Building(pTarget)) {

		if(Ares_AboutToChronoshift(pTarget)){ 
			return false;
		}
	}
	else if (Is_Unit(pTarget) &&
		!TechnoExt::IsInWarfactory(pTarget)) { 
		return false;
	}

	return true;
}

DEFINE_OVERRIDE_HOOK(0x71AE50, TemporalClass_CanWarpTarget, 8)
{
	GET_STACK(TechnoClass*, pTarget, 0x4);
	R->EAX(Warpable(pTarget));
	return 0x71AF19;
}

DEFINE_OVERRIDE_HOOK(0x71944E, TeleportLocomotionClass_ILocomotion_Process, 6)
{
	GET(FootClass*, pObject, ECX);
	GET(CoordStruct*, XYZ, EDX);
	*XYZ = pObject->GetCoords();
	R->EAX<CoordStruct*>(XYZ);

	if (auto pType = pObject->GetTechnoType())
	{
		if (const auto pImage = pType->AlphaImage)
		{
			Point2D xy;
			TacticalClass::Instance->CoordsToClient(XYZ, &xy);
			RectangleStruct ScreenArea = TacticalClass::Instance->VisibleArea();
			Point2D off = { ScreenArea.X - (pImage->Width / 2), ScreenArea.Y - (pImage->Height / 2) };
			xy += off;
			RectangleStruct Dirty =
			{ xy.X - ScreenArea.X, xy.Y - ScreenArea.Y,
			  pImage->Width, pImage->Height };
			TacticalClass::Instance->RegisterDirtyArea(Dirty, true);
		}
	}

	return 0x719454;
}

DEFINE_OVERRIDE_HOOK(0x713C10, TechnoTypeClass_LoadFromINI_SkipLists2, 7)
{
	GET(TechnoTypeClass*, pThis, EBP);
	GET(const CoordStruct*, pResult, EAX);
	pThis->NaturalParticleSystemLocation = *pResult;
	return 0x713E1A;
}

DEFINE_OVERRIDE_SKIP_HOOK(0x715857, TechnoTypeClass_LoadFromINI_LimitPalettes, 5, 715876)

DEFINE_OVERRIDE_HOOK(0x713171, TechnoTypeClass_LoadFromINI_SkipLists1, 9)
{
	GET(TechnoTypeClass*, pThis, EBP);
	GET(Category, category, EAX);
	pThis->Category = category;
	return 0x713264;
}

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

		double nPowerSpeedResult = 0.0f;
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
			auto const nFactorySpeed = pTypeExt->BuildTime_MultipleFactory.Get(RulesClass::Instance->MultipleFactory);
			if (nFactorySpeed > 0.0)
			{
				for (int i = (pOwner->FactoryCount(pThis->WhatAmI(), Is_Unit(pThis) ? pType->Naval : false) - 1); i > 0; --i)
				{
					nFinalSpeed *= nFactorySpeed;
				}
			}
		}
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
DEFINE_OVERRIDE_HOOK(0x6FA4C6, TechnoClass_Update_ZeroOutTarget, 5) {
	return (Is_Aircraft(R->ESI<TechnoClass*>())) ? 0x6FA4D1 : 0;
}

DEFINE_OVERRIDE_HOOK(0x70BE80, TechnoClass_ShouldSelfHealOneStep, 5)
{
	GET(TechnoClass* const, pThis, ECX);
	auto const nAmount = AresData::GetSelfHealAmount(pThis);
	R->EAX(nAmount > 0 || nAmount != 0);
	return 0x70BF46;
}

// TODO : port this
#define Is_DriverKilled(techno) \
(*(bool*)((char*)techno->align_154 + 0x9C))

DEFINE_OVERRIDE_HOOK(0x6FA743, TechnoClass_Update_SelfHeal, 0xA)
{
	GET(TechnoClass* const, pThis, ESI);

	// prevent crashing and sinking technos from self-healing
	if (pThis->IsCrashing || pThis->IsSinking || Is_DriverKilled(pThis)) {
		return 0x6FA941;
	}

	// this replaces the call to pThis->ShouldSelfHealOneStep()
	if (auto const amount = AresData::GetSelfHealAmount(pThis))
	{
		pThis->Health += amount;

		R->ECX(pThis);
		return 0x6FA75A;
	}

	return 0x6FA793;
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

	if (pThis->GetHealthPercentage_() >= RulesClass::Instance->ConditionYellow || pThis->GetHeight() <= -10)
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
	for (auto i = pNames->begin(); i < pNames->end(); ++i) {
		if (!_strcmpi(*i, id)) {
			if (pThis->CanBeSelectedNow()) {
				match = true;
				break;
			}

			// buildings are exempt if they can't undeploy
			if (Is_Building(pThis) && pType->UndeploysInto) {
				match = true;
				break;
			}
		}
	}

	R->EAX(match ? 1 : 0);
	return 0x732C97;
}

// UNTESTED!!
// bugfix #388: Units firing from inside a transport do not obey DecloakToFire
DEFINE_OVERRIDE_HOOK(0x6FCA30, TechnoClass_GetWeaponState_Insidetransport, 6)
{
	GET(TechnoClass*, Techno, ESI);
	return (Techno->Transporter && Techno->Transporter->CloakState != CloakState::Uncloaked)
		? 0x6FCA4F : 0;
}

DEFINE_OVERRIDE_HOOK(0x6F3950, TechnoClass_GetCrewCount, 8)
{
	GET(TechnoClass*, pThis, ECX);
	auto pType = pThis->GetTechnoType();

	// previous default for crew count was -1
	int count = TechnoTypeExt::ExtMap.Find(pType)->Survivors_PilotCount.Get();
	// default to original formula
	if (count < 0) {
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
	pThis->ForceShielded = force  ? TRUE : FALSE;

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

	if (auto pSM = pThis->SpawnManager)
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

DEFINE_OVERRIDE_HOOK(0x70CBB0 ,TechnoClass_DealParticleDamage_AmbientDamage, 6)
{
	GET_STACK(WeaponTypeClass*, pWeapon, 0x14);

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
DEFINE_OVERRIDE_HOOK(0x6FB5F0 , TechnoClass_DeleteGap_Optimize, 6)
{
	GET(CellClass*, pCell, EAX);

	if (!HouseClass::CurrentPlayer->SpySatActive || --pCell->GapsCoveringThisCell > 0)
		return 0x6FB69E;

	--pCell->ShroudCounter;

	if (pCell->ShroudCounter <= 0)
		pCell->AltFlags |= AltCellFlags::Clear;

	return 0x6FB69E;
}

// weapons can take more than one round of ammo
DEFINE_OVERRIDE_HOOK(0x6FCA0D, TechnoClass_CanFire_Ammo, 6)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(WeaponTypeClass* const, pWeapon, EBX);

	return (pThis->Ammo < 0 || pThis->Ammo >= WeaponTypeExt::ExtMap.Find(pWeapon)->Ammo)
		? 0x6FCA26u : 0x6FCA17u;
}

DEFINE_OVERRIDE_HOOK_AGAIN(0x6FB4A3 , TechnoClass_CreateGap_LargeGap, 7)
DEFINE_OVERRIDE_HOOK(0x6FB1B5, TechnoClass_CreateGap_LargeGap, 7)
{
	GET(TechnoClass*, pThis, ESI);
	GET(TechnoTypeClass*, pType, EAX);
	// ares change this to read from ext instead
	// since this one is `char` so it cant store bigger number i assume
	pThis->GapRadius = pType->GapRadiusInCells;
	return R->Origin() + 0xD;
}

DEFINE_OVERRIDE_HOOK(0x6FB306 , TechnoClass_CreateGap_Optimize, 6)
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

DEFINE_OVERRIDE_HOOK(0x421371, TacticalClass_UpdateAlphasInRectangle_ShouldDraw, 5)
{
	GET(int, AlphaLightIndex, EBX);
	auto pAlpha = AlphaShapeClass::Array->Items[AlphaLightIndex];

	bool shouldDraw = !pAlpha->IsObjectGone;

	if (shouldDraw) {
		if (const auto pTechno = abstract_cast<TechnoClass*>(pAlpha->AttachedTo)) {
			shouldDraw = pTechno->VisualCharacter(VARIANT_TRUE, pTechno->Owner) == VisualType::Normal &&
				!pTechno->Disguised;
		}
	}

	return shouldDraw ? 0 : 0x421694;
}

#include <Conversions.h>

DEFINE_OVERRIDE_HOOK(0x42146E, TacticalClass_UpdateAlphasInRectangle_Header, 5)
{
	GET(int, AlphaLightIndex, EBX);
	GET(RectangleStruct*, buffer, EDX);
	GET(SHPStruct*, pImage, EDI);

	const auto pAlpha = AlphaShapeClass::Array->Items[AlphaLightIndex];
	unsigned int idx = 0;

	if (const auto pTechno = abstract_cast<TechnoClass*>(pAlpha->AttachedTo))  {
		if (pImage->Frames > 0) {
			const int countFrames = Conversions::Int2Highest(pImage->Frames);
			const DirStruct PrimaryFacing = pTechno->PrimaryFacing.Current();
			idx = ((PrimaryFacing.Raw) >> (16 - countFrames));
		}
	}

	R->EAX(pImage->GetFrameBounds(*buffer, idx));
	return 0x421478;
}

DEFINE_OVERRIDE_HOOK(0x42152C, TacticalClass_UpdateAlphasInRectangle_Body, 8)
{
	GET_STACK(int, AlphaLightIndex, STACK_OFFS(0xA4, 0x78));
	GET(SHPStruct*, pImage, ECX);

	const auto pAlpha = AlphaShapeClass::Array->Items[AlphaLightIndex];
	if (const auto pTechno = abstract_cast<TechnoClass*>(pAlpha->AttachedTo)) {
		if (pImage->Frames > 0) {
			const int countFrames = Conversions::Int2Highest(pImage->Frames);
			const DirStruct PrimaryFacing = pTechno->PrimaryFacing.Current();
			R->Stack(0x0, ((unsigned short)(PrimaryFacing.Raw) >> (16 - countFrames)));
		}
	}

	return 0;
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

std::pair<TriggerAttachType, bool> AresGetFlag(AresNewTriggerAction nAction)
{
	switch (nAction)
	{
	case AresNewTriggerAction::AuxiliaryPower:
	case AresNewTriggerAction::SetEVAVoice:
		return { TriggerAttachType::None , true };
	case AresNewTriggerAction::KillDriversOf:
	case AresNewTriggerAction::SetGroup:
		return { TriggerAttachType::Object , true };
	default:
		return { TriggerAttachType::None , false };
	}
}

DEFINE_OVERRIDE_HOOK(0x6E3EE0, TActionClass_GetFlags, 5)
{
	GET(AresNewTriggerAction, nAction, ECX);

	auto const& [SomeFlag, Handled] = AresGetFlag(nAction);

	if (!Handled)
		return 0;

	R->EAX(SomeFlag);
	return 0x6E3EFE;
}

std::pair<LogicNeedType, bool> GetMode(AresNewTriggerAction nAction)
{
	switch (nAction)
	{
	case AresNewTriggerAction::AuxiliaryPower:
		return { LogicNeedType::NumberNSuper  , true };
	case AresNewTriggerAction::KillDriversOf:
		return { LogicNeedType::None , true };
	case AresNewTriggerAction::SetEVAVoice:
	case AresNewTriggerAction::SetGroup:
		return { LogicNeedType::Number, true };
	default:
		return { LogicNeedType::None , false };
	}
}

DEFINE_OVERRIDE_HOOK(0x6E3B60, TActionClass_GetMode, 8)
{
	GET(AresNewTriggerAction, nAction, ECX);

	auto const& [SomeFlag, Handled] = GetMode(nAction);
	if (Handled)
	{
		R->EAX(SomeFlag);
		return 0x6E3C4B;
	}
	else
	{
		R->EAX(((int)nAction) - 1);
		return ((int)nAction) > 0x8F ? 0x6E3C49 : 0x6E3B6E;
	}
}

enum class Presistable
{
	None = 0,
	unk_0x100 = 256,
	unk_0x101 = 257
};

std::pair<Presistable, bool> GetPresistableFlag(AresNewTriggerEvents nAction)
{
	switch (nAction)
	{
	case AresNewTriggerEvents::UnderEMP:
	case AresNewTriggerEvents::UnderEMP_ByHouse:
	case AresNewTriggerEvents::RemoveEMP:
	case AresNewTriggerEvents::RemoveEMP_ByHouse:
	case AresNewTriggerEvents::EnemyInSpotlightNow:
	case AresNewTriggerEvents::ReverseEngineered:
	case AresNewTriggerEvents::HouseOwnTechnoType:
	case AresNewTriggerEvents::HouseDoesntOwnTechnoType:
	case AresNewTriggerEvents::AttackedOrDestroyedByAnybody:
	case AresNewTriggerEvents::AttackedOrDestroyedByHouse:
	case AresNewTriggerEvents::TechnoTypeDoesntExistMoreThan:
		return { Presistable::unk_0x100  , true };
	case AresNewTriggerEvents::DriverKiller:
	case AresNewTriggerEvents::DriverKilled_ByHouse:
	case AresNewTriggerEvents::VehicleTaken:
	case AresNewTriggerEvents::VehicleTaken_ByHouse:
	case AresNewTriggerEvents::Abducted:
	case AresNewTriggerEvents::Abducted_ByHouse:
	case AresNewTriggerEvents::AbductSomething:
	case AresNewTriggerEvents::AbductSomething_OfHouse:
	case AresNewTriggerEvents::SuperActivated:
	case AresNewTriggerEvents::SuperDeactivated:
	case AresNewTriggerEvents::SuperNearWaypoint:
	case AresNewTriggerEvents::ReverseEngineerAnything:
	case AresNewTriggerEvents::ReverseEngineerType:
	case AresNewTriggerEvents::DestroyedByHouse:
	case AresNewTriggerEvents::AllKeepAlivesDestroyed:
	case AresNewTriggerEvents::AllKeppAlivesBuildingDestroyed:
		return { Presistable::unk_0x101  , true };
	default:
		return { Presistable::None  , false };
	}

}

DEFINE_OVERRIDE_HOOK(0x71F9C0, TEventClass_Persistable_AresNewTriggerEvents, 6)
{
	GET(TEventClass*, pThis, ECX);
	auto const& [Flag, Handled] = 
		GetPresistableFlag((AresNewTriggerEvents)pThis->EventKind);
	if (!Handled)
		return 0x0;

	R->EAX(Flag);
	return 0x71F9DF;
}

// Resolves a param to a house.
HouseClass* ResolveHouseParam(int const param, HouseClass* const pOwnerHouse = nullptr)
{
	if (param == 8997)
	{
		return pOwnerHouse;
	}

	if (HouseClass::Index_IsMP(param))
	{
		return HouseClass::FindByIndex(param);
	}

	return HouseClass::FindByCountryIndex(param);
}

// the general events requiring a house
DEFINE_OVERRIDE_HOOK(0x71F06C, EventClass_HasOccured_PlayerAtX1, 5)
{
	GET(int const, param, ECX);

	auto const pHouse = ResolveHouseParam(param);
	R->EAX(pHouse);

	// continue normally if a house was found or this isn't Player@X logic,
	// otherwise return false directly so events don't fire for non-existing
	// players.
	return (pHouse || !HouseClass::Index_IsMP(param)) ? 0x71F071u : 0x71F0D5u;
}

// validation for Spy as House, the Entered/Overflown Bys and the Crossed V/H Lines
DEFINE_OVERRIDE_HOOK_AGAIN(0x71ED33, EventClass_HasOccured_PlayerAtX2, 5)
DEFINE_OVERRIDE_HOOK_AGAIN(0x71F1C9, EventClass_HasOccured_PlayerAtX2, 5)
DEFINE_OVERRIDE_HOOK_AGAIN(0x71F1ED, EventClass_HasOccured_PlayerAtX2, 5)
DEFINE_OVERRIDE_HOOK(0x71ED01, EventClass_HasOccured_PlayerAtX2, 5)
{
	GET(int const, param, ECX);
	R->EAX(ResolveHouseParam(param));
	return R->Origin() + 5;
}

// param for Attacked by House is the array index
DEFINE_OVERRIDE_HOOK(0x71EE79, EventClass_HasOccured_PlayerAtX3, 9)
{
	GET(int, param, EAX);
	GET(HouseClass* const, pHouse, EDX);

	// convert Player @ X to real index
	if (HouseClass::Index_IsMP(param))
	{
		auto const pPlayer = ResolveHouseParam(param);
		param = pPlayer ? pPlayer->ArrayIndex : -1;
	}

	return (pHouse->ArrayIndex == param) ? 0x71EE82u : 0x71F163u;
}

namespace TEventExt_dummy
{
	// the function return is deciding if the case is handled or not
	// the bool result pointer is for the result of the Event itself
	bool NOINLINE HasOccured(TEventClass* pThis, EventArgs const Args, bool* result)
	{
		return false;
	}
}


//TODO : 
// before doing this , need to port keep-alive
//DEFINE_OVERRIDE_HOOK(0x71E949 , TEventClass_HasOccured, 7)
//{
//
//	GET(TEventClass*, pThis, EBP);
//	GET_BASE(EventArgs const, args, STACK_OFFSET(0x2C,0x4));
//	enum { return_true = 0x71F1B1, return_false = 0x71F163};
//	bool result = false;
//	if (TEventExt_dummy::HasOccured(pThis, args, &result)) {
//		return result ? return_true : return_false;
//	}
//
//	return 0;
//}

DEFINE_OVERRIDE_HOOK(0x6CF350, SwizzleManagerClass_ConvertNodes, 7)
{
	PhobosSwizzle::Instance.ConvertNodes();
	PhobosSwizzle::Instance.Clear();

	return 0x6CF400;
}

DEFINE_OVERRIDE_HOOK(0x6CF2C0, SwizzleManagerClass_Here_I_Am, 5)
{
	GET_STACK(void*, oldP, 0x8);
	GET_STACK(void*, newP, 0xC);
	R->EAX<HRESULT>(PhobosSwizzle::Instance.RegisterChange(oldP, newP));
	return 0x6CF316;
}

DEFINE_OVERRIDE_HOOK(0x6CF240, SwizzleManagerClass_Swizzle, 7)
{
	GET_STACK(void**, ptr, 0x8);
	R->EAX<HRESULT>(PhobosSwizzle::Instance.RegisterForChange(ptr));
	return 0x6CF2B3;
}
//TODO : 
// better port these
// DEFINE_HOOK(6FB757, TechnoClass_UpdateCloak, 8)

//TODO :
// better port these :s
// DEFINE_HOOK(6FAF0D, TechnoClass_Update_EMPLock, 6)

// TODO : still not correct !
//DEFINE_HOOK(718279, TeleportLocomotionClass_MakeRoom, 5)
//{
//	GET(CoordStruct*, pCoord, EAX);
//	GET(TeleportLocomotionClass*, pLoco, EBP);
//
//	auto const pLinked = pLoco->LinkedTo;
//	auto const pLinkedIsInf = pLinked->WhatAmI() == AbstractType::Infantry;
//	auto const pCell = Map.TryGetCellAt(*pCoord);
//
//	R->Stack(0x48, false);
//	R->EBX(pCell->OverlayTypeIndex);
//	R->EDI(0);
//
//	for (NextObject obj(pCell->GetContent()); obj; ++obj)
//	{
//
//		auto const pObj = (*obj);
//		auto const bIsObjFoot = pObj->AbstractFlags & AbstractFlags::Foot;
//		auto const pObjIsInf = pObj->WhatAmI() == AbstractType::Infantry;
//		auto bIsObjectInvicible = pObj->IsIronCurtained();
//		auto const pType = pObj->GetTechnoType();
//		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
//
//		if (pType && !pTypeExt->Chronoshift_Crushable)
//			bIsObjectInvicible = true;
//
//		if (!bIsObjectInvicible && pObjIsInf && pLinkedIsInf)
//		{
//			auto const bEligible = pLinked->Owner && !pLinked->Owner->IsAlliedWith(pObj);
//			auto const pAttackerHouse = bEligible ? pLinked->Owner : nullptr;
//			auto const pAttackerTechno = bEligible ? pLinked : nullptr;
//
//			auto nCoord = pObj->GetCoords();
//			if (nCoord == *pCoord)
//			{
//				auto nDamage = pObj->Health;
//				pObj->ReceiveDamage(&nDamage, 0, RulesClass::Instance->C4Warhead, pAttackerTechno, true, false, pAttackerHouse);
//			}
//		}
//		else if (bIsObjectInvicible || !bIsObjFoot)
//		{
//			if (bIsObjectInvicible)
//			{
//				auto const pObjHouse = pObj->GetOwningHouse();
//				auto const pAttackerHouse = pObjHouse && !pObjHouse->IsAlliedWith(pObj) ? pObjHouse : nullptr;
//				auto const pAttackerTechno = reinterpret_cast<TechnoClass*>(pObj);
//
//				auto nDamage = pLinked->Health;
//				pLinked->ReceiveDamage(&nDamage, 0, RulesClass::Instance->C4Warhead, pAttackerTechno, true, false, pAttackerHouse);
//			}
//			else if (!bIsObjFoot)
//			{
//				R->Stack(0x48, true);
//			}
//		}
//		else
//		{
//
//			auto const bEligible = pLinked->Owner && !pLinked->Owner->IsAlliedWith(pObj);
//			auto const pAttackerHouse = bEligible ? pLinked->Owner : nullptr;
//			auto const pAttackerTechno = bEligible ? pLinked : nullptr;
//			auto nDamage = pObj->Health;
//			pObj->ReceiveDamage(&nDamage, 0, RulesClass::Instance->C4Warhead, pAttackerTechno, true, false, pAttackerHouse);
//		}
//	}
//
//	auto const nFlag300 = CellFlags::Bridge | CellFlags::Unknown_200;
//	if ((pCell->Flags & nFlag300) == CellFlags::Bridge)
//		R->Stack(0x48, true);
//
//	R->Stack(0x20, pLinked->GetMapCoords());
//	R->EAX(true);
//
//	return 0x7184CE;
//}

//TODO :
// This fuckery need more than this 
// need to port Bulletclass::Detonate stuffs 
// otherwise it will cause some inconsistency
//DEFINE_HOOK(71AAAC, TemporalClass_Update_Abductor, 6)
//{
//	GET(TemporalClass*, pThis, ESI);
//
//	const auto pOwner = pThis->Owner;
//	const auto pTempExt = TemporalExt::ExtMap.Find(pThis);
//	const auto pWeapon = pTempExt->GetWeapon();
//
//	return (WeaponTypeExt::ExtMap.Find(pWeapon)->conductAbduction(pOwner, pThis->Target))
//		? 0x71AAD5 : 0x0;
//}

//TODO :
// temporal per-slot
// HAHA AE and Jammer stuffs go brrr
//DEFINE_HOOK(71A84E, TemporalClass_UpdateA, 5)
//{
//	GET(TemporalClass* const, pThis, ESI);
//
//	// it's not guaranteed that there is a target
//	if (auto const pTarget = pThis->Target)
//	{
//		auto const pExt = TechnoExt::ExtMap.Find(pTarget);
//		// Temporal should disable RadarJammers
//		pExt->RadarJam = nullptr;
//
//		//AttachEffect handling under Temporal
//		if (!pExt->AttachEffects_NeedTo_RecreateAnims)
//		{
//			for (auto& Item : pExt->AttachedEffects)
//			{
//				if (Item.Type->TemporalHidesAnim)
//				{
//					Item.KillAnim();
//				}
//			}
//			pExt->AttachEffects_NeedTo_RecreateAnims = true;
//		}
//	}
//
//	pThis->WarpRemaining -= pThis->GetWarpPerStep(0);
//
//	R->EAX(pThis->WarpRemaining);
//	return 0x71A88D;
//}

// TODO :
// Prism fuckery 
//DEFINE_HOOK(71AF76, TemporalClass_Fire_PrismForwardAndWarpable, 9)
//{
//	GET(TechnoClass* const, pThis, EDI);
//
//	// bugfix #874 B: Temporal warheads affect Warpable=no units
//	// it has been checked: this is warpable. free captured and destroy spawned units.
//	if (pThis->SpawnManager)
//	{
//		pThis->SpawnManager->KillNodes();
//	}
//
//	if (pThis->CaptureManager)
//	{
//		pThis->CaptureManager->FreeAll();
//	}
//
//	// prism forward
//	if (pThis->WhatAmI() == AbstractType::Building)
//	{
//		auto const pData = BuildingExt::ExtMap.Find(reinterpret_cast<BuildingClass*>(pThis));
//		pData->PrismForwarding.RemoveFromNetwork(true);
//	}
//
//	return 0;
//}

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