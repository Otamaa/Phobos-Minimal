#include "Body.h"

#include <Locomotor/DropPodLocomotionClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/SWType/NewSuperWeaponType/NewSWType.h>

#include <Misc/DamageArea.h>

struct DroppodProperties_
{
	static int GetTrailerDelay(TechnoTypeClass* pType, FootClass* pFoot, bool condition)
	{
		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
		const auto pExt = TechnoExtContainer::Instance.Find(pFoot);
		const auto defaultres =  condition ? SWTypeExtContainer::Instance.Find(pExt->LinkedSW->Type)->DroppodProp.Droppod_Trailer_SpawnDelay : RulesExtData::Instance()->DroppodTrailerSpawnDelay;

		return pTypeExt->DropPodProp.Droppod_Trailer_SpawnDelay.Get(defaultres);
	}

	static bool GetIsTrailerAttached(TechnoTypeClass* pType, FootClass* pFoot, bool condition)
	{
		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
		const auto pExt = TechnoExtContainer::Instance.Find(pFoot);
		const auto defaultres = condition ? SWTypeExtContainer::Instance.Find(pExt->LinkedSW->Type)->DroppodProp.Droppod_Trailer_Attached : false;

		return pTypeExt->DropPodProp.Droppod_Trailer_Attached.Get(defaultres);
	}

	static int GetSpeed(TechnoTypeClass* pType, FootClass* pFoot,bool condition)
	{
		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
		const auto pExt = TechnoExtContainer::Instance.Find(pFoot);
		const auto defaultres = condition ? SWTypeExtContainer::Instance.Find(pExt->LinkedSW->Type)->DroppodProp.Droppod_Speed : RulesClass::Instance->DropPodSpeed;

		return pTypeExt->DropPodProp.Droppod_Speed.Get(defaultres);
	}

	static int GetHeight(TechnoTypeClass* pType, FootClass* pFoot,bool condition)
	{
		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
		const auto pExt = TechnoExtContainer::Instance.Find(pFoot);
		const auto defaultres = condition ? SWTypeExtContainer::Instance.Find(pExt->LinkedSW->Type)->DroppodProp.Droppod_Height : RulesClass::Instance->DropPodHeight;

		return pTypeExt->DropPodProp.Droppod_Height.Get(defaultres);
	}

	static double GetAngle(TechnoTypeClass* pType, FootClass* pFoot, bool condition)
	{
		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
		const auto pExt = TechnoExtContainer::Instance.Find(pFoot);
		const auto defaultres = condition ? SWTypeExtContainer::Instance.Find(pExt->LinkedSW->Type)->DroppodProp.Droppod_Angle : RulesClass::Instance->DropPodAngle;

		return pTypeExt->DropPodProp.Droppod_Angle.Get(defaultres);
	}

	static SHPStruct* GetPodImage(FootClass* pFoot)
	{
		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pFoot->GetTechnoType());
		const auto pExt = TechnoExtContainer::Instance.Find(pFoot);
		SHPStruct* result = pTypeExt->DropPodProp.Droppod_PodImage_Infantry.Get(RulesExtData::Instance()->Droppod_ImageInfantry);

		if (TechnoExtContainer::Instance.Find(pFoot)->LinkedSW && SWTypeExtContainer::Instance.Find(pExt->LinkedSW->Type)->DroppodProp.Droppod_PodImage_Infantry)
			result = SWTypeExtContainer::Instance.Find(pExt->LinkedSW->Type)->DroppodProp.Droppod_PodImage_Infantry;

		return result;
	}

	static AnimTypeClass* GetTrailer(TechnoTypeClass* pType, FootClass* pFoot, bool condition)
	{
		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
		const auto pExt = TechnoExtContainer::Instance.Find(pFoot);
		const auto defaultres = condition ? SWTypeExtContainer::Instance.Find(pExt->LinkedSW->Type)->DroppodProp.Droppod_Trailer : RulesExtData::Instance()->DropPodTrailer;

		return pTypeExt->DropPodProp.Droppod_Trailer.Get(defaultres);
	}

	static WeaponTypeClass* GetWeapon(TechnoTypeClass* pType, FootClass* pFoot, bool condition)
	{
		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
		const auto pExt = TechnoExtContainer::Instance.Find(pFoot);
		const auto defaultres = condition ? SWTypeExtContainer::Instance.Find(pExt->LinkedSW->Type)->DroppodProp.Droppod_Weapon : RulesClass::Instance->DropPodWeapon;

		return pTypeExt->DropPodProp.Droppod_Weapon.Get(defaultres);
	}

	static AnimTypeClass* GetPuff(TechnoTypeClass* pType, FootClass* pFoot, bool condition)
	{
		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
		const auto pExt = TechnoExtContainer::Instance.Find(pFoot);
		const auto defaultres = condition ? SWTypeExtContainer::Instance.Find(pExt->LinkedSW->Type)->DroppodProp.Droppod_Puff : RulesClass::Instance->DropPodPuff;

		return pTypeExt->DropPodProp.Droppod_Puff.Get(defaultres);
	}

	static AnimTypeClass* GetGroundAnim(TechnoTypeClass* pType, FootClass* pFoot,int State, bool condition)
	{
		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
		const auto pExt = TechnoExtContainer::Instance.Find(pFoot);
		const auto defaultres = condition ? make_iterator(SWTypeExtContainer::Instance.Find(pExt->LinkedSW->Type)->DroppodProp.Droppod_GroundPodAnim) : make_iterator(RulesClass::Instance->DropPod);
		const auto result = pTypeExt->DropPodProp.Droppod_GroundPodAnim.GetElements(defaultres);
		return result.empty() || result.size() < 2 ? nullptr : result[State == 0 && result.size() > 2 ? ScenarioClass::Instance->Random.RandomFromMax(result.size()-1) : State];
	}

	static AnimTypeClass* GetAtmosphereEntry(TechnoTypeClass* pType, FootClass* pFoot, bool condition)
	{
		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
		const auto pExt = TechnoExtContainer::Instance.Find(pFoot);
		const auto defaultres = condition ? SWTypeExtContainer::Instance.Find(pExt->LinkedSW->Type)->DroppodProp.Droppod_AtmosphereEntry : RulesClass::Instance->AtmosphereEntry;

		return pTypeExt->DropPodProp.Droppod_AtmosphereEntry.Get(defaultres);
	}

	// function below originated from PR #1196 by : chaserli
	// i just do some adjustment so my own code can go in
	static bool Process_(DropPodLocomotionClass* pLoco)
	{
		auto pLinked = pLoco->LinkedTo;
		auto tType = pLinked->GetTechnoType();
		CoordStruct oldLoc = pLinked->Location;
		auto pLinkedExt = TechnoExtContainer::Instance.Find(pLinked);
		const auto pLinkedSW = pLinkedExt->LinkedSW;
		const bool condition = pLinkedSW && (int)pLinkedSW->Type->Type == (int)AresNewSuperType::DropPod;
		const double angle = DroppodProperties_::GetAngle(tType, pLinked, condition);
		const auto maxspeed = DroppodProperties_::GetSpeed(tType, pLinked, condition);
		const int speed = std::max(maxspeed, pLinked->GetHeight() / 10 + 2);

		CoordStruct coords = pLinked->Location;
		coords.X += int(Math::cos(angle) * speed * (pLoco->OutOfMap ? 1 : -1));
		coords.Z -= int(Math::sin(angle) * speed);

		if (pLinked->GetHeight() > 0)
		{
			pLinked->SetLocation(coords);

			if (AnimTypeClass* pType = DroppodProperties_::GetTrailer(tType, pLinked, condition)) {
				if (Unsorted::CurrentFrame % DroppodProperties_::GetTrailerDelay(tType, pLinked, condition) == 1) {
					auto pTrail = GameCreate<AnimClass>(pType, coords, 0, 1, (AnimFlag)0x600, 0, false);
					AnimExtData::SetAnimOwnerHouseKind(pTrail,
						pLinked->Owner,
						nullptr,
						pLinked,
						false, false
					);

					if (DroppodProperties_::GetIsTrailerAttached(tType, pLinked, condition))
						pTrail->SetOwnerObject(pLinked);
				}
			}

			if (auto dWpn = DroppodProperties_::GetWeapon(tType, pLinked, condition)) {
				if (Unsorted::CurrentFrame % MaxImpl(dWpn->ROF, 3) == 0) {
					auto cell = MapClass::Instance->GetCellAt(pLoco->CoordDest);
					auto techno = cell->FindTechnoNearestTo({ 0,0 }, false);
					if (!pLinked->Owner->IsAlliedWith(techno))
					{
						// wtf are these???
						auto locnear = MapClass::GetRandomCoordsNear(pLoco->CoordDest, 85, false);

						if (int count = dWpn->Report.Count)
							VocClass::SafeImmedietelyPlayAt(count == 1 ? dWpn->Report[0] : dWpn->Report[Random2Class::Global->RandomFromMax(count - 1)], &coords);

						DamageArea::Apply(&locnear, 2 * dWpn->Damage, pLinked, dWpn->Warhead, true, pLinked->Owner);
						if (auto dmgAnim = MapClass::SelectDamageAnimation(2 * dWpn->Damage, dWpn->Warhead, LandType::Clear, locnear))
						{
							AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(dmgAnim, locnear, 0, 1, (AnimFlag)0x2600, -15, false),
								pLinked->Owner,
								nullptr,
								pLinked,
								false, false
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
			CoordStruct coord_place = pLinked->Location;
			auto pAnimType = DroppodProperties_::GetPuff(tType, pLinked, condition);
			const auto nDroppod = DroppodProperties_::GetGroundAnim(tType, pLinked, pLoco->OutOfMap, condition);

			pLoco->Release();

			if (pLinked->Unlimbo(coord_place, DirType::North))
			{
				if (pAnimType) {
					AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnimType, coord_place, 0, 1, AnimFlag(0x600), 0, 0),
						pLinked->Owner,
						nullptr,
						pLinked,
						false, false
					);
				}

				if (nDroppod) {
					AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(nDroppod, coord_place, 0, 1, AnimFlag(0x600), 0, 0),
						pLinked->Owner,
						nullptr,
						pLinked,
						false, false
					);
				}

				pLinkedExt->LaserTrails.remove_all_if([](auto const& pTrail) { return pTrail->Type->DroppodOnly; });

				pLinked->Mark(MarkType::Put);
				pLinked->SetHeight(0);
				pLinked->EnterIdleMode(false, true);
				pLinked->NextMission();
				pLinked->Scatter(CoordStruct::Empty, false, false);
			}
			else
			{
				DamageArea::Apply(&coord_place, 100, pLinked, RulesClass::Instance->C4Warhead, true, pLinked->Owner);
				if (auto dmgAnim = MapClass::SelectDamageAnimation(100, RulesClass::Instance->C4Warhead, LandType::Clear, coord_place))
				{
					AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(dmgAnim, coord_place, 0, 1, (AnimFlag)0x2600, -15, false),
						pLinked->Owner,
						nullptr,
						pLinked,
						false, false
					);
				}
			}

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
		const int height = DroppodProperties_::GetHeight(tType, pLoco->LinkedTo, condition);
		const double angle = DroppodProperties_::GetAngle(tType, pLoco->LinkedTo, condition);

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

			if (auto pAnimType = DroppodProperties_::GetAtmosphereEntry(tType, pLoco->LinkedTo, condition))
			{
				AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnimType, coord, 0, 1, AnimFlag(0x600), 0, 0),
					pLoco->Owner->Owner,
					nullptr,
					pLoco->Owner,
					false, false);
			}
		}
	}
};

/*
ASMJIT_PATCH(0x4B5BCD, DroppodLoco_Process_Speed, 0x6)
{
	GET(DropPodLocomotionClass*, pThis, EDI);
	GET(RulesClass*, pRules, EDX);

	R->EAX(DroppodProperties::GetSpeed(pThis->Owner , pRules->DropPodSpeed));

	return 0x4B5BD3;
}

ASMJIT_PATCH(0x4B5BEC, DroppodLoco_Process_Angle1, 0x6)
{
	GET(DropPodLocomotionClass*, pThis, EDI);
	GET(RulesClass*, pRules, EDX);

	R->EDX(DroppodProperties::GetAngle(pThis->Owner , pRules->DropPodAngle));

	return 0x4B5BF2;
}

ASMJIT_PATCH(0x4B5C14, DroppodLoco_Process_Angle2, 0x6)
{
	GET(DropPodLocomotionClass*, pThis, EDI);
	GET(RulesClass*, pRules, EDX);

	R->ECX(DroppodProperties::GetAngle(pThis->Owner, pRules->DropPodAngle));

	return 0x4B5C1A;
}

ASMJIT_PATCH(0x4B5C50, DroppodLoco_Process_Angle3, 0x6)
{
	GET(DropPodLocomotionClass*, pThis, EDI);
	GET(RulesClass*, pRules, EAX);

	R->EAX(DroppodProperties::GetAngle(pThis->Owner, pRules->DropPodAngle));

	return 0x4B5C56;
}

ASMJIT_PATCH(0x4B5EB0, DropPodLocomotionClass_ILocomotion_Process_Smoke, 6)
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
					VocClass::SafeImmedietelyPlayAt(ScenarioClass::Instance->Random.RandomFromMax(pWeapon->Report.Count - 1), pCoords, nullptr);
				}

				if (pWeapon->Warhead)
				{
					DamageArea::Apply(coordDest, 2 * pWeapon->Damage, pFoot, pWeapon->Warhead, pWeapon->Warhead->Tiberium, pFoot->Owner);

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
ASMJIT_PATCH(0x4B5CF1, DropPodLocomotionClass_Process_DroppodPuff, 0x5)
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
ASMJIT_PATCH(0x4B5B70, DroppodLoco_ILoco_Process, 0x5)
{
	GET_STACK(ILocomotion*, iloco, 0x4);
	R->AL(DroppodProperties_::Process_(static_cast<DropPodLocomotionClass*>(iloco)));
	return 0x4B6036;
}

// function below originated from PR #1196 by : chaserli
// i just do some adjustment so my own code can go in
ASMJIT_PATCH(0x4B6040, DroppodLoco_ILoco_MoveTo, 0x9)
{
	GET_STACK(ILocomotion*, iloco, 0x4);
	REF_STACK(CoordStruct, to, 0x8);
	DroppodProperties_::MoveTo_(static_cast<DropPodLocomotionClass*>(iloco), &to);
	return 0x4B61F5;
}

ASMJIT_PATCH(0x519168, InfantryClass_DrawIt_DroppodLinked, 0x5)
{
	GET(InfantryClass*, pThis, EBP);

	if (auto pPod = DroppodProperties_::GetPodImage(pThis))
	{
		R->EAX(pPod);
		return 0x519172;
	}

	return 0x0;
}

ASMJIT_PATCH(0x4B641D, DroppodLocomotionClass_IPiggy_EndPiggyback, 7)
{
	GET(ILocomotion*, pIloco, EAX);
	const auto pLoco = static_cast<LocomotionClass*>(pIloco);

	if (pLoco->Owner) {
		const auto pExt = TechnoExtContainer::Instance.Find(pLoco->Owner);
		if (pExt->LinkedSW && (int)pExt->LinkedSW->Type->Type == (int)AresNewSuperType::DropPod)
			pExt->LinkedSW = nullptr;
	}

	return 0x0;
}

//ASMJIT_PATCH(0x4B619F, DropPodLocomotionClass_MoveTo_AtmosphereEntry, 0x5)
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
