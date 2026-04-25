#include <Locomotor/DropPodLocomotionClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/SWType/NewSuperWeaponType/SWTypeHandler.h>
#include <Ext/Super/Body.h>

#include <Misc/DamageArea.h>

#include <Utilities/Patch.h>
#include <Utilities/Macro.h>

#include <InfantryClass.h>

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
		const auto pTypeExt = GET_TECHNOTYPEEXT(pFoot);
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
		if (result.empty() || result.size() < 2)
			return nullptr;
		const size_t index = (State == 0 && result.size() > 2) ? ScenarioClass::Instance->Random.RandomFromMax(result.size()-1) : static_cast<size_t>(MinImpl(State, static_cast<int>(result.size()) - 1));
		return result[index];
	}

	static AnimTypeClass* GetAtmosphereEntry(TechnoTypeClass* pType, FootClass* pFoot, bool condition)
	{
		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
		const auto pExt = TechnoExtContainer::Instance.Find(pFoot);
		const auto defaultres = condition ? SWTypeExtContainer::Instance.Find(pExt->LinkedSW->Type)->DroppodProp.Droppod_AtmosphereEntry : RulesClass::Instance->AtmosphereEntry;

		return pTypeExt->DropPodProp.Droppod_AtmosphereEntry.Get(defaultres);
	}


};

struct _DropPodLocomotionClass
{
	// function below originated from PR #1196 by : chaserli
// i just do some adjustment so my own code can go in
	static bool __stdcall _Process(ILocomotion* pILoco)
	{
		auto pLoco = static_cast<DropPodLocomotionClass*>(pILoco);
		auto pLinked = pLoco->LinkedTo;
		auto tType = GET_TECHNOTYPE(pLinked);
		CoordStruct oldLoc = pLinked->Location;
		auto pLinkedExt = TechnoExtContainer::Instance.Find(pLinked);
		const auto pLinkedSW = pLinkedExt->LinkedSW;
		const bool condition = pLinkedSW && (int)pLinkedSW->Type->Type == (int)NewSuperType::DropPod;
		const double angle = DroppodProperties_::GetAngle(tType, pLinked, condition);
		const auto maxspeed = DroppodProperties_::GetSpeed(tType, pLinked, condition);
		const int speed = MaxImpl(maxspeed, pLinked->GetHeight() / 10 + 2);

		CoordStruct coords = pLinked->Location;
		coords.X += int(Math::cos(angle) * speed * (pLoco->OutOfMap ? 1 : -1));
		coords.Z -= int(Math::sin(angle) * speed);

		if (pLinked->GetHeight() > 0)
		{
			pLinked->SetLocation(coords);

			if (AnimTypeClass* pType = DroppodProperties_::GetTrailer(tType, pLinked, condition))
			{
				if (Unsorted::CurrentFrame.get() % DroppodProperties_::GetTrailerDelay(tType, pLinked, condition) == 1)
				{
					auto pTrail = GameCreate<AnimClass>(pType, coords, 0, 1, AnimFlag::AnimFlag_600, 0, false);
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

			if (auto dWpn = DroppodProperties_::GetWeapon(tType, pLinked, condition))
			{
				if (Unsorted::CurrentFrame.get() % MaxImpl(dWpn->ROF, 3) == 0)
				{
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
							AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(dmgAnim, locnear, 0, 1, AnimFlag::AnimFlag_2600, -15, false),
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
			auto pAnimType = DroppodProperties_::GetPuff(tType, pLinked, condition);
			const auto nDroppod = DroppodProperties_::GetGroundAnim(tType, pLinked, pLoco->OutOfMap, condition);

			pLinked->SetHeight(0);
			pLinked->Limbo();
			pLoco->AddRef();
			pLoco->End_Piggyback(&pLinked->Locomotor);
			CoordStruct coord_place = pLinked->Location;
			pLoco->Release();

			if (pLinked->Unlimbo(coord_place, DirType::North))
			{
				if (pAnimType)
				{
					AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnimType, coord_place, 0, 1, AnimFlag::AnimFlag_600, 0, 0),
						pLinked->Owner,
						nullptr,
						pLinked,
						false, false
					);
				}

				if (nDroppod)
				{
					AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(nDroppod, coord_place, 0, 1, AnimFlag::AnimFlag_600, 0, 0),
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
					AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(dmgAnim, coord_place, 0, 1, AnimFlag::AnimFlag_2600, -15, false),
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
	static void __stdcall _Move_To(ILocomotion* pILoco, CoordStruct pCoord)
	{
		auto pLoco = static_cast<DropPodLocomotionClass*>(pILoco);

		if (pLoco->CoordDest.IsValid())
			return;

		pLoco->CoordDest = pCoord;
		pLoco->CoordDest.Z = MapClass::Instance->GetCellFloorHeight(pCoord);
		auto tType = GET_TECHNOTYPE(pLoco->LinkedTo);
		const auto pLinkedSW = TechnoExtContainer::Instance.Find(pLoco->LinkedTo)->LinkedSW;
		const bool condition = pLinkedSW && (int)pLinkedSW->Type->Type == (int)NewSuperType::DropPod;
		const int height = DroppodProperties_::GetHeight(tType, pLoco->LinkedTo, condition);
		const double angle = DroppodProperties_::GetAngle(tType, pLoco->LinkedTo, condition);

		CoordStruct coord = pCoord;
		coord.Z += height;
		if (!MapClass::Instance->IsWithinUsableArea(coord))
		{
			pLoco->OutOfMap = true;
			coord.X -= int(height / std::tan(angle));
		}
		else
			coord.X += int(height / std::tan(angle));

		pLoco->LinkedTo->SetLocation(coord);

		if (pLoco->LinkedTo->Unlimbo(coord, DirType::South))
		{
			pLoco->LinkedTo->PrimaryFacing.Set_Current(DirStruct { DirType::South });

			if (auto pAnimType = DroppodProperties_::GetAtmosphereEntry(tType, pLoco->LinkedTo, condition))
			{
				AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnimType, coord, 0, 1, AnimFlag::AnimFlag_600, 0, 0),
					pLoco->Owner->Owner,
					nullptr,
					pLoco->Owner,
					false, false);
			}
		}
	}
};

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E82B8, _DropPodLocomotionClass::_Process);
DEFINE_FUNCTION_JUMP(LJMP, 0x4B5B70, _DropPodLocomotionClass::_Process);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E82BC, _DropPodLocomotionClass::_Move_To);
DEFINE_FUNCTION_JUMP(LJMP, 0x4B6040, _DropPodLocomotionClass::_Move_To);

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
		if (pExt->LinkedSW && (int)pExt->LinkedSW->Type->Type == (int)NewSuperType::DropPod)
			pExt->LinkedSW = nullptr;
	}

	return 0x0;
}