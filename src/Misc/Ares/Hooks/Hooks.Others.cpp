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

#include <New/Type/ArmorTypeClass.h>

#include <Notifications.h>

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
		SpawnAnim(RulesClass::Instance->SmallFire, 64);

		// 50% to create another small fire
		if (ScenarioClass::Instance->Random.RandomFromMax(99) < 50)
		{
			SpawnAnim(RulesClass::Instance->SmallFire, 160);
		}

		// 50% chance to create a large fire
		if (ScenarioClass::Instance->Random.RandomFromMax(99) < 50)
		{
			SpawnAnim(RulesClass::Instance->LargeFire, 112);
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

	if (pThis->AttachedTo->WhatAmI() == AnimClass::AbsID) {
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

DEFINE_OVERRIDE_HOOK_AGAIN(0x75EE2E, WaveClass_Draw_Green, 0x8)
DEFINE_OVERRIDE_HOOK(0x7601C7, WaveClass_Draw_Magnetron, 0x8)
{
	GET(int, Q, EDX);
	if (Q > 0x15F8F)
	{
		Q = 0x15F8F;
	}
	R->EDX(Q);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x7609E3, WaveClass_Draw_NodLaser_Details, 0x5)
{
	R->EAX(2);
	return 0x7609E8;
}

DEFINE_OVERRIDE_SKIP_HOOK(0x760286, WaveClass_Draw_Magnetron3, 0x5, 7602D3)

DEFINE_OVERRIDE_HOOK(0x76110B, WaveClass_RecalculateAffectedCells_Clear, 0x5)
{
	GET(DynamicVectorClass<CellClass*>*, pVec, EBP);
	pVec->Count = 0; //clear count , dont destroy the vector
	return 0x761110;
}

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
		const auto it = std::find(TechnoTypeClass::PipsTypeName.begin(), TechnoTypeClass::PipsTypeName.end(),
			Phobos::readBuffer);

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
		auto v5 = pObject->AttachedTag;
		if (!v5 || (v5->RaiseEvent(static_cast<TriggerEvent>(0x54u), pObject, CellStruct::Empty, false, pAttacker), pObject->IsAlive))
		{
			if (auto v6 = pObject->AttachedTag)
				v6->RaiseEvent(static_cast<TriggerEvent>(0x53u), pObject, CellStruct::Empty, false, pAttacker);
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
	std::sort(nBegin, nEnd, [](ObjectClass* A, ObjectClass* B) {
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
	auto const pWhat = Owner->WhatAmI();

	bool allowed = false;
	if (pWhat == AbstractType::Unit)
	{
		allowed = !Owner->GetTechnoType()->Naval;
	}
	else if (pWhat == AbstractType::Infantry)
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
DEFINE_OVERRIDE_SKIP_HOOK(0x5F698F, ObjectClass_GetCell, 5, 5F69B2)

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

	if (pVictim->WhatAmI() == AbstractType::Building)
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

DEFINE_OVERRIDE_HOOK(0x6B7D50, SpawnManagerClass_CountDockedSpawns, 0x6)
{
	GET(SpawnManagerClass*, pThis, ECX);

	int nCur = 0;

	for (auto const pNode : pThis->SpawnedNodes)
	{
		auto const nStatus = pNode->Status;
		auto const nEligible = (
			nStatus == SpawnNodeStatus::Idle ||
			nStatus == SpawnNodeStatus::Dead ||
			nStatus == SpawnNodeStatus::Reloading) &&
			pNode->SpawnTimer.StartTime >= 0 &&
			!pNode->SpawnTimer.TimeLeft;

		if (nEligible) {
			++nCur;
		}
	}

	R->EAX(nCur);
	return 0x6B7D73;
}

DEFINE_OVERRIDE_HOOK(0x6B72F9, SpawnManagerClass_Update_Buildings, 0x5)
{
	GET(SpawnManagerClass*, pThis, ESI);
	GET(SpawnNode, nNode, EAX);

	auto const pOwner = pThis->Owner;
	return (nNode.Status != SpawnNodeStatus::TakeOff ||
		!pOwner ||
		pOwner->WhatAmI() == AbstractType::Building)
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

	if (nDist < nRadius) {
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

		if (auto pSys = pThis->ParticleSystem) {
			if (!pSys->Particles.Remove(pThis)){ 
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

	if (AnimTypeClass* AnimType = AnimTypeClass::Array->GetItemOrDefault(pThis->Value)) {

		AnimClass* pAnim = GameCreate<AnimClass>(AnimType, nCoord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false);

		if (AnimType->MakeInfantry > -1) {
			AnimTypeExt::SetMakeInfOwner(pAnim, pOwner, nullptr);
		}

		AnimExt::SetAnimOwnerHouseKind(pAnim, pOwner, pOwner,false);
	}

	R->EAX(1);
	return 0x6E2387;
}

DEFINE_OVERRIDE_HOOK(0x731E08 , Select_By_Units_Text_FakeOf, 0x6)
{
	int nCost = 0;
	for (const auto pObj : ObjectClass::CurrentObjects()) {

		if (const auto pTechno = generic_cast<const TechnoClass*>(pObj)) {
			const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTechno->GetTechnoType());

			TechnoTypeClass* pType = pTypeExt->Get();
			if (pTypeExt->Fake_Of.Get(nullptr))
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

DEFINE_OVERRIDE_HOOK(0x7BB445 , XSurface_20, 0x6)
{
	return R->EAX<void*>() ? 0x0 : 0x7BB90C;
}
