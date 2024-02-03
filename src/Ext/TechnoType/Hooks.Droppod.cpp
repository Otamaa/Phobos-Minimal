#include "Body.h"

#include <Locomotor/DropPodLocomotionClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/SWType/NewSuperWeaponType/NewSWType.h>

struct DroppodProperties
{

	static int GetSpeed(TechnoTypeClass* pType, FootClass* pFoot, int rulesValue, bool condition)
	{
		const auto pExt = TechnoExtContainer::Instance.Find(pFoot);
		const auto defaultVal = !condition ?
			rulesValue : SWTypeExtContainer::Instance.Find(pExt->LinkedSW->Type)->Droppod_Speed;

		return defaultVal;
	}

	static int GetHeight(TechnoTypeClass* pType, FootClass* pFoot, int rulesValue, bool condition)
	{
		const auto pExt = TechnoExtContainer::Instance.Find(pFoot);
		const auto defaultVal = !condition ?
			rulesValue : SWTypeExtContainer::Instance.Find(pExt->LinkedSW->Type)->Droppod_Height;

		return defaultVal;
	}

	static double GetAngle(TechnoTypeClass* pType, FootClass* pFoot, double rulesValue, bool condition)
	{
		const auto pExt = TechnoExtContainer::Instance.Find(pFoot);
		const auto defaultVal = !condition ?
			rulesValue : SWTypeExtContainer::Instance.Find(pExt->LinkedSW->Type)->Droppod_Angle;

		return defaultVal;
	}

	static SHPStruct* GetPodImage(FootClass* pFoot)
	{
		const auto pExt = TechnoExtContainer::Instance.Find(pFoot);
		const auto defaultVal = !pExt->LinkedSW || (int)pExt->LinkedSW->Type->Type != (int)AresNewSuperType::DropPod ?
			nullptr : SWTypeExtContainer::Instance.Find(pExt->LinkedSW->Type)->Droppod_PodImage_Infantry.Get();

		return defaultVal;
	}

	static AnimTypeClass* GetTrailer(TechnoTypeClass* pType, FootClass* pFoot, AnimTypeClass* rulesValue, bool condition)
	{
		const auto pExt = TechnoExtContainer::Instance.Find(pFoot);
		const auto defaultVal = !condition ?
			rulesValue : SWTypeExtContainer::Instance.Find(pExt->LinkedSW->Type)->Droppod_Trailer.Get();

		return defaultVal;
	}

	static WeaponTypeClass* GetWeapon(TechnoTypeClass* pType, FootClass* pFoot, WeaponTypeClass* rulesValue, bool condition)
	{
		const auto pExt = TechnoExtContainer::Instance.Find(pFoot);
		const auto defaultVal = !condition ?
			rulesValue : SWTypeExtContainer::Instance.Find(pExt->LinkedSW->Type)->Droppod_Weapon.Get();

		return defaultVal;
	}

	static AnimTypeClass* GetPuff(TechnoTypeClass* pType, FootClass* pFoot, AnimTypeClass* rulesValue, bool condition)
	{
		const auto pExt = TechnoExtContainer::Instance.Find(pFoot);
		const auto defaultVal = !condition ?
			rulesValue : SWTypeExtContainer::Instance.Find(pExt->LinkedSW->Type)->Droppod_Puff.Get();

		return defaultVal;
	}

	static AnimTypeClass* GetGroundAnim(TechnoTypeClass* pType, FootClass* pFoot, TypeList<AnimTypeClass*>& rulesvalue, int State, bool condition)
	{
		const auto pExt = TechnoExtContainer::Instance.Find(pFoot);
		const auto defaultVal = !condition ?
			make_iterator(rulesvalue) : SWTypeExtContainer::Instance.Find(pExt->LinkedSW->Type)->Droppod_GroundPodAnim.GetElements();

		return !defaultVal || defaultVal.size() < 2 ? nullptr : defaultVal[State];
	}

	static AnimTypeClass* GetAtmosphereEntry(TechnoTypeClass* pType, FootClass* pFoot, AnimTypeClass* rulesValue, bool condition)
	{
		const auto pExt = TechnoExtContainer::Instance.Find(pFoot);
		const auto defaultVal = !condition ?
			rulesValue : SWTypeExtContainer::Instance.Find(pExt->LinkedSW->Type)->Droppod_AtmosphereEntry.Get();

		return defaultVal;
	}

	// function below originated from PR #1196 by : chaserli
	// i just do some adjustment so my own code can go in 
	static bool Process_(DropPodLocomotionClass* pLoco)
	{
		auto pLinked = pLoco->LinkedTo;
		auto tType = pLinked->GetTechnoType();
		CoordStruct oldLoc = pLinked->Location;
		const auto pLinkedSW = TechnoExtContainer::Instance.Find(pLinked)->LinkedSW;
		const bool condition = pLinkedSW && (int)pLinkedSW->Type->Type == (int)AresNewSuperType::DropPod;
		const double angle = DroppodProperties::GetAngle(tType, pLinked, RulesClass::Instance->DropPodAngle, condition);
		const auto maxspeed = DroppodProperties::GetSpeed(tType, pLinked, RulesClass::Instance->DropPodSpeed, condition);
		const int speed = std::max(maxspeed, pLinked->GetHeight() / 10 + 2);

		CoordStruct coords = pLinked->Location;
		coords.X += int(Math::cos(angle) * speed * (pLoco->OutOfMap ? 1 : -1));
		coords.Z -= int(Math::sin(angle) * speed);

		if (pLinked->GetHeight() > 0)
		{
			pLinked->SetLocation(coords);

			if (AnimTypeClass* pType = DroppodProperties::GetTrailer(tType, pLinked, RulesExtData::Instance()->DropPodTrailer, condition)) {
				if (Unsorted::CurrentFrame % 6 == 0) {
					AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pType, coords, 0, 1, (AnimFlag)0x600, 0, false),
						pLinked->Owner,
						nullptr,
						pLinked,
						false
					);
				}
			}

			if (auto dWpn = DroppodProperties::GetWeapon(tType, pLinked, RulesClass::Instance->DropPodWeapon, condition)) {
				if (Unsorted::CurrentFrame % MaxImpl(dWpn->ROF, 3) == 0) {
					auto cell = MapClass::Instance->GetCellAt(pLoco->CoordDest);
					auto techno = cell->FindTechnoNearestTo({ 0,0 }, false);
					if (!pLinked->Owner->IsAlliedWith(techno))
					{
						// wtf are these???
						auto locnear = MapClass::GetRandomCoordsNear(pLoco->CoordDest, 85, false);

						if (int count = dWpn->Report.Count)
							VocClass::PlayAt(dWpn->Report[Random2Class::Global->RandomFromMax(count - 1)], coords);

						MapClass::DamageArea(locnear, 2 * dWpn->Damage, pLinked, dWpn->Warhead, true, pLinked->Owner);
						if (auto dmgAnim = MapClass::SelectDamageAnimation(2 * dWpn->Damage, dWpn->Warhead, LandType::Clear, locnear))
						{
							AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(dmgAnim, locnear, 0, 1, (AnimFlag)0x2600, -15, false),
								pLinked->Owner,
								nullptr,
								pLinked,
								false
							);
						}
					}
				}
			}
		}
		else
		{
			pLinked->SetHeight(0);
			pLinked->Limbo();
			pLoco->AddRef();
			pLoco->End_Piggyback(&pLinked->Locomotor);

			if (pLinked->Unlimbo(coords, DirType::North))
			{
				if (auto pAnimType = DroppodProperties::GetPuff(tType, pLinked, RulesClass::Instance->DropPodPuff, condition))
				{
					AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnimType, coords, 0, 1, AnimFlag(0x600), 0, 0),
						pLinked->Owner,
						nullptr,
						pLinked,
						false
					);
				}

				if (const auto nDroppod = DroppodProperties::GetGroundAnim(tType, pLinked, RulesClass::Instance->DropPod, pLoco->OutOfMap, condition))
				{
					AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(nDroppod, coords, 0, 1, AnimFlag(0x600), 0, 0),
						pLinked->Owner,
						nullptr,
						pLinked,
						false
					);
				}

				pLinked->UpdatePlacement(PlacementType::Put);
				pLinked->SetHeight(0);
				pLinked->EnterIdleMode(false, true);
				pLinked->NextMission();
				pLinked->Scatter(CoordStruct::Empty, false, false);
			}
			else
			{
				MapClass::DamageArea(coords, 100, pLinked, RulesClass::Instance->C4Warhead, true, pLinked->Owner);
				if (auto dmgAnim = MapClass::SelectDamageAnimation(100, RulesClass::Instance->C4Warhead, LandType::Clear, coords))
				{
					AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(dmgAnim, coords, 0, 1, (AnimFlag)0x2600, -15, false),
						pLinked->Owner,
						nullptr,
						pLinked,
						false
					);
				}
			}

			pLoco->Release();
		}

		return true;
	}

	// function below originated from PR #1196 by : chaserli
	// i just do some adjustment so my own code can go in 
	static void MoveTo_(DropPodLocomotionClass* pLoco, CoordStruct* pCoord)
	{
		if (pLoco->CoordDest.IsValid())
			return;

		pLoco->CoordDest = *pCoord;
		pLoco->CoordDest.Z = MapClass::Instance->GetCellFloorHeight(pCoord);
		auto tType = pLoco->LinkedTo->GetTechnoType();
		const auto pLinkedSW = TechnoExtContainer::Instance.Find(pLoco->LinkedTo)->LinkedSW;
		const bool condition = pLinkedSW && (int)pLinkedSW->Type->Type == (int)AresNewSuperType::DropPod;
		const int height = DroppodProperties::GetHeight(tType, pLoco->LinkedTo, RulesClass::Instance->DropPodHeight, condition);
		const double angle = DroppodProperties::GetAngle(tType, pLoco->LinkedTo, RulesClass::Instance->DropPodAngle, condition);

		CoordStruct coord = *pCoord;
		coord.Z += height;
		if (!MapClass::Instance->IsWithinUsableArea(coord))
		{
			pLoco->OutOfMap = true;
			coord.X -= int(height / Math::tan(angle));
		}
		else
			coord.X += int(height / Math::tan(angle));

		pLoco->LinkedTo->SetLocation(coord);

		if (pLoco->LinkedTo->Unlimbo(coord, DirType::South))
		{
			pLoco->LinkedTo->PrimaryFacing.Set_Current(DirStruct { DirType::South });

			if (auto pAnimType = DroppodProperties::GetAtmosphereEntry(tType, pLoco->LinkedTo, RulesClass::Instance->AtmosphereEntry, condition))
			{
				AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnimType, coord, 0, 1, AnimFlag(0x600), 0, 0),
					pLoco->Owner->Owner,
					nullptr,
					pLoco->Owner,
					false);
			}
		}
	}
};

/*
DEFINE_HOOK(0x4B5BCD, DroppodLoco_Process_Speed, 0x6)
{
	GET(DropPodLocomotionClass*, pThis, EDI);
	GET(RulesClass*, pRules, EDX);

	R->EAX(DroppodProperties::GetSpeed(pThis->Owner , pRules->DropPodSpeed));

	return 0x4B5BD3;
}

DEFINE_HOOK(0x4B5BEC, DroppodLoco_Process_Angle1, 0x6)
{
	GET(DropPodLocomotionClass*, pThis, EDI);
	GET(RulesClass*, pRules, EDX);

	R->EDX(DroppodProperties::GetAngle(pThis->Owner , pRules->DropPodAngle));

	return 0x4B5BF2;
}

DEFINE_HOOK(0x4B5C14, DroppodLoco_Process_Angle2, 0x6)
{
	GET(DropPodLocomotionClass*, pThis, EDI);
	GET(RulesClass*, pRules, EDX);

	R->ECX(DroppodProperties::GetAngle(pThis->Owner, pRules->DropPodAngle));

	return 0x4B5C1A;
}

DEFINE_HOOK(0x4B5C50, DroppodLoco_Process_Angle3, 0x6)
{
	GET(DropPodLocomotionClass*, pThis, EDI);
	GET(RulesClass*, pRules, EAX);

	R->EAX(DroppodProperties::GetAngle(pThis->Owner, pRules->DropPodAngle));

	return 0x4B5C56;
}

DEFINE_OVERRIDE_HOOK(0x4B5EB0, DropPodLocomotionClass_ILocomotion_Process_Smoke, 6)
{
	GET(DropPodLocomotionClass*, pDroppod, EDI);
	GET(FootClass*, pFoot, ESI);
	LEA_STACK(CoordStruct*, pCoords, 0x40 - 0xC);

	// create trailer even without weapon, but only if it is set
	if (!(Unsorted::CurrentFrame % 6))
	{
		if (AnimTypeClass* pType = DroppodProperties::GetTrailer(pFoot, RulesExtData::Instance()->DropPodTrailer))
		{
			AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pType, pCoords, 0, 1, (AnimFlag)0x600, 0, false),
				pFoot->Owner,
				nullptr,
				pFoot,
				false
			);
		}
	}

	if (const auto pWeapon = DroppodProperties::GetWeapon(pFoot, RulesClass::Instance->DropPodWeapon))
	{
		if (!(Unsorted::CurrentFrame % 3))
		{
			// Please dont ask , see the binary yourself ,.. (-Otamaa)
			CoordStruct nDest = *reinterpret_cast<CoordStruct*>(((uintptr_t)(pDroppod)) + 0x1C);

			const auto pCell = MapClass::Instance->GetCellAt(nDest);
			const auto pCellTechno = pCell->FindTechnoNearestTo(Point2D::Empty, false, nullptr);

			if (!pCellTechno || !pFoot->Owner->IsAlliedWith(pCellTechno))
			{
				auto coordDest = MapClass::GetRandomCoordsNear(nDest, 85, false);

				if (pWeapon->Report.Count > 0)
				{
					VocClass::PlayIndexAtPos(ScenarioClass::Instance->Random.RandomFromMax(pWeapon->Report.Count - 1), pCoords, nullptr);
				}

				if (pWeapon->Warhead)
				{
					MapClass::DamageArea(coordDest, 2 * pWeapon->Damage, pFoot, pWeapon->Warhead, pWeapon->Warhead->Tiberium, pFoot->Owner);

					if (auto pWeaponAnimType = MapClass::SelectDamageAnimation(2 * pWeapon->Damage, pWeapon->Warhead, LandType::Clear, coordDest))
					{
						auto zAdjust = Game::AdjustHeight(coordDest.Z);

						AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pWeaponAnimType, coordDest, 0, 1, (AnimFlag)0x2600, zAdjust, false),
							pFoot->Owner,
							pCellTechno ? pCellTechno->Owner : nullptr,
							pFoot,
							false
						);
					}
				}
			}
		}
	}

	return 0x4B602D;
}

//TODO : DropPod WH explosion 4B5D8F ,4B6028
DEFINE_HOOK(0x4B5CF1, DropPodLocomotionClass_Process_DroppodPuff, 0x5)
{
	GET(DropPodLocomotionClass*, pLoco, EDI);
	GET(FootClass*, pFoot, ESI);
	LEA_STACK(CoordStruct*, pCoord, 0x40 - 0x18);

	if (!pFoot->Unlimbo(*pCoord, ScenarioClass::Instance->Random.RandomRangedSpecific<DirType>(DirType::Min, DirType::Max)))
		return 0x4B5D0A;

	const auto pExt = TechnoExtContainer::Instance.Find(pFoot);
	const auto IsLinkedSWEligible = pExt->LinkedSW && (int)pExt->LinkedSW->Type->Type == (int)AresNewSuperType::DropPod;

	if (auto pAnimType = DroppodProperties::GetPuff(pFoot, RulesClass::Instance->DropPodPuff))
	{
		AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnimType, pCoord, 0, 1, AnimFlag(0x600), 0, 0),
			pFoot->Owner,
			nullptr,
			pFoot,
			false
		);
	}

	const auto nDroppod = DroppodProperties::GetGroundAnim(pFoot, RulesClass::Instance->DropPod, pLoco->OutOfMap);

	if (!nDroppod)
		return 0x4B5E4C;

	AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(nDroppod, pCoord, 0, 1, AnimFlag(0x600), 0, 0),
		pFoot->Owner,
		nullptr,
		pFoot,
		false
	);

	return 0x4B5E4C;
}
*/

// function below originated from PR #1196 by : chaserli
// i just do some adjustment so my own code can go in 
DEFINE_HOOK(0x4B5B70, DroppodLoco_ILoco_Process, 0x5)
{
	GET_STACK(ILocomotion*, iloco, 0x4);
	R->AL(DroppodProperties::Process_(static_cast<DropPodLocomotionClass*>(iloco)));
	return 0x4B6036;
}

// function below originated from PR #1196 by : chaserli
// i just do some adjustment so my own code can go in 
DEFINE_HOOK(0x4B6040, DroppodLoco_ILoco_MoveTo, 0x9)
{
	GET_STACK(ILocomotion*, iloco, 0x4);
	REF_STACK(CoordStruct, to, 0x8);
	DroppodProperties::MoveTo_(static_cast<DropPodLocomotionClass*>(iloco), &to);
	return 0x4B61F5;
}

DEFINE_HOOK(0x519168, InfantryClass_DrawIt_DroppodLinked, 0x5)
{
	GET(InfantryClass*, pThis, EBP);

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);

	if (auto pPod = DroppodProperties::GetPodImage(pThis))
	{
		R->EAX(pPod);
		return 0x519172;
	}

	return 0x0;
}

//DEFINE_HOOK(0x4B641D, DroppodLocomotionClass_IPiggy_EndPiggyback, 7)
//{
//	GET(ILocomotion*, pIloco, EAX);
//	const auto pLoco = static_cast<LocomotionClass*>(pIloco);
//
//	if (pLoco->Owner)
//	{
//		const auto pExt = TechnoExtContainer::Instance.Find(pLoco->Owner);
//		if (pExt->LinkedSW && (int)pExt->LinkedSW->Type->Type == (int)AresNewSuperType::DropPod)
//			pExt->LinkedSW = nullptr;
//	}
//
//	return 0x0;
//}

//DEFINE_HOOK(0x4B619F, DropPodLocomotionClass_MoveTo_AtmosphereEntry, 0x5)
//{
//	GET(DropPodLocomotionClass*, pLoco, EDI);
//	LEA_STACK(CoordStruct*, pCoord, 0x1C - 0xC);
//
//	const auto pExt = TechnoExtContainer::Instance.Find(pLoco->Owner);
//
//	if (auto pAnimType = DroppodProperties::GetAtmosphereEntry(pLoco->Owner , RulesClass::Instance->AtmosphereEntry))
//	{
//		AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnimType, pCoord, 0, 1, AnimFlag(0x600), 0, 0),
//			pLoco->Owner->Owner,
//			nullptr,
//			pLoco->Owner,
//			false);
//	}
//
//	return 0x4B61D6;
//}
