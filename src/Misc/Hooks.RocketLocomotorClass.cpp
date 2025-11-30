#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/Techno/Body.h>

#include <Utilities/Macro.h>
#include <Utilities/Debug.h>

#include <AircraftTrackerClass.h>
#include <AircraftClass.h>
#include <Kamikaze.h>
#include <RocketStruct.h>
#include <Locomotor/RocketLocomotionClass.h>

#include "DamageArea.h"

#ifdef NEW
/*
*	Original Backport code author : ZivDero
*	Otamaa : do some modification to adapt YRpp
*/
struct _KamikazetrackerClass
{
	static void __fastcall Add(Kamikaze* pThis, DWORD, AircraftClass* pAir, AbstractClass* pTarget)
	{
		if (!pAir->Type->MissileSpawn)
		{
			pAir->Crash(nullptr);
			return;
		}

		const auto control = GameCreate<Kamikaze::KamikazeControl>(pAir, !pTarget ?
			pAir->GetCell()->GetAdjacentCell(FacingType(pAir->PrimaryFacing.Current().GetFacing<8>())) :
			MapClass::Instance->GetCellAt(pTarget->GetCoords()));

		pAir->IsKamikaze = true;
		pAir->Ammo = 1;
		pThis->Nodes.push_back(control);
	}

	static void __fastcall AI(Kamikaze* pThis, DWORD)
	{
		if (!pThis->UpdateTimer.Expired())
			return;

		pThis->UpdateTimer.Start(30);

		for (auto& control : pThis->Nodes)
		{
			CellClass* cell = control->Cell;
			AircraftClass* aircraft = control->Item;

			aircraft->Ammo = 1;

			if (cell)
				aircraft->SetTarget(cell);
			else
				aircraft->SetTarget(aircraft->GetCell()->GetAdjacentCell(FacingType(aircraft->PrimaryFacing.Current().GetFacing<8>())));

			aircraft->QueueMission(Mission::Attack, false);
		}
	}

	static void __fastcall Detach(Kamikaze* pThis, DWORD, AbstractClass const* pTarget)
	{
		if (!pThis->Nodes.Count)
			return;

		auto removeIter = pThis->Nodes.remove_if([=](auto& item)
		{
			if (item->Item == pTarget)
			{
				GameDelete<false, false>(item);
				item = nullptr;
				return true;
			}

			if (item->Cell == pTarget)
			{
				item->Cell = MapClass::Instance->GetCellAt(pTarget->GetCoords());
			}

			return false;
		});

		pThis->Nodes.Reset(std::distance(pThis->Nodes.begin(), removeIter));

	}

	static void __fastcall Clear(Kamikaze* pThis, DWORD)
	{
		for (int i = 0; i < pThis->Nodes.Count; i++)
		{
			GameDelete<false, false>(pThis->Nodes[i]);
		}

		pThis->Nodes.Clear();
		pThis->UpdateTimer.Start(1);
	}
};

//DEFINE_FUNCTION_JUMP(LJMP, 0x54E3B0, _KamikazetrackerClass::Add));
//DEFINE_FUNCTION_JUMP(LJMP, 0x54E4D0, _KamikazetrackerClass::AI));
//DEFINE_FUNCTION_JUMP(LJMP, 0x54E590, _KamikazetrackerClass::Detach));
//DEFINE_FUNCTION_JUMP(LJMP, 0x54E6F0, _KamikazetrackerClass::Clear));


/*
*	Original Backport code author : CCHyper & ZivDero
*	Otamaa : do some modification to adapt YRpp and Ares stuffs
*/
struct _RocketLocomotionClass
{
	static NOINLINE RocketStruct* GetRocketData(TechnoTypeClass* pType)
	{
		auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

		if (pTypeExt->IsCustomMissile)
		{
			return pTypeExt->CustomMissileData.operator->();
		}

		if (pType == RulesClass::Instance->CMisl.Type)
		{
			return &RulesClass::Instance->CMisl;
		}

		if (pType == RulesClass::Instance->DMisl.Type)
		{
			return &RulesClass::Instance->DMisl;
		}

		return &RulesClass::Instance->V3Rocket;
	}

	static NOINLINE WarheadTypeClass* GetRocketWarhead(TechnoTypeClass* pType, bool IsElite)
	{
		auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

		if (pTypeExt->IsCustomMissile)
			return IsElite ? pTypeExt->CustomMissileEliteWarhead : pTypeExt->CustomMissileWarhead;

		if (pType == RulesClass::Instance->CMisl.Type)
		{
			return IsElite ? RulesClass::Instance->CMislEliteWarhead : RulesClass::Instance->CMislWarhead;
		}

		if (pType == RulesClass::Instance->DMisl.Type)
		{
			return IsElite ? RulesClass::Instance->CMislEliteWarhead : RulesClass::Instance->CMislWarhead;
		}

		return IsElite ? RulesClass::Instance->V3EliteWarhead : RulesClass::Instance->V3Warhead;
	}

	static NOINLINE bool RocketHasWeapon(FootClass* pRocket, TechnoTypeClass* pType, bool IsElite, CoordStruct coords)
	{
		auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

		if (auto const& pWeapon = IsElite ? pTypeExt->CustomMissileEliteWeapon : pTypeExt->CustomMissileWeapon)
		{
			WeaponTypeExtData::DetonateAt(pWeapon, coords, pRocket, true, pRocket ? pRocket->Owner : nullptr);
			return true;
		}

		return false;
	}

	static NOINLINE bool IsCruise(TechnoTypeClass* pType)
	{
		return pType == RulesClass::Instance->CMisl.Type;
	}

	static NOINLINE AnimTypeClass* GetTrailAnim(TechnoTypeClass* pType)
	{
		return TechnoTypeExtContainer::Instance.Find(pType)->CustomMissileTrailerAnim;
	}

	static NOINLINE AnimTypeClass* GetTakeOffAnim(TechnoTypeClass* pType)
	{
		return TechnoTypeExtContainer::Instance.Find(pType)->CustomMissileTakeoffAnim;
	}

	static NOINLINE bool CanRaise(TechnoTypeClass* pType, bool IsElite)
	{
		const auto& _opt = TechnoTypeExtContainer::Instance.Find(pType)->CustomMissileRaise;
		return IsElite ? _opt.Elite : _opt.Rookie;
	}

	static NOINLINE int GetTrailerSeparation(TechnoTypeClass* pType)
	{
		return TechnoTypeExtContainer::Instance.Find(pType)->CustomMissileTrailerSeparation;
	}

	static Matrix3D* __stdcall _Draw_Matrix(ILocomotion* pThis, Matrix3D* result, VoxelIndexKey* key)
	{
		result->MakeIdentity();

		auto pRocket = static_cast<RocketLocomotionClass*>(pThis);
		const auto pAir = pRocket->Owner;

		double angle = double((int)(pAir->PrimaryFacing.Current().GetFacing<32>() - 8) * Math::DIRECTION_FIXED_MAGIC);
		result->RotateZ((float)angle);

		if (pRocket->CurrentPitch != 0.0)
		{
			result->RotateY((-pRocket->CurrentPitch));
			const RocketStruct* rocket = GetRocketData(pAir->GetTechnoType());

			if (key)
			{
				if (pRocket->CurrentPitch == float(rocket->PitchInitial * Math::DEG90_AS_RAD))
					*(int*)(key) |= 0x20;
				else if (pRocket->CurrentPitch == float(rocket->PitchFinal * Math::DEG90_AS_RAD))
					*(int*)(key) |= 0x40;
				else
					key->Invalidate();
			}
		}

		if (key)
		{
			*(int*)(key) |= pAir->PrimaryFacing.Current().GetFacing<32>();
		}

		return result;
	}

	static void __stdcall _Move_To(ILocomotion* pThis, CoordStruct to)
	{
		auto pRocket = static_cast<RocketLocomotionClass*>(pThis);
		const auto pAir = pRocket->Owner;

		if (!pRocket->MovingDestination.IsValid())
		{
			const RocketStruct* rocket = GetRocketData(pAir->GetTechnoType());
			// set the missile very first mission status
			pRocket->MissionState = rocket->PauseFrames ? RocketMissionState::Pause : RocketMissionState::Tilt;
			pRocket->MissionTimer.Start(rocket->PauseFrames ? rocket->PauseFrames : rocket->TiltFrames);
			pRocket->CurrentPitch = (float)(rocket->PitchInitial * Math::DEG90_AS_RAD);
			auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pAir->GetTechnoType());

			/**
			*  Apply some inaccuracy to the coordinate if the rocket type specifies so.
			*/
			int accur = pTypeExt->CustomMissileInaccuracy;

			if (accur <= 0)
			{
				pRocket->MovingDestination = to;
			}
			else
			{
				const int randomx = ScenarioClass::Instance->Random.RandomRanged(-accur, accur);
				const int randomy = ScenarioClass::Instance->Random.RandomRanged(-accur, accur);
				pRocket->MovingDestination = to + Coordinate(randomx, randomy, 0);
			}
		}
	}

	static bool __stdcall _Is_Moving_Now(ILocomotion* pThis)
	{
		auto pRocket = static_cast<RocketLocomotionClass*>(pThis);
		return pRocket->MissionState >= RocketMissionState::GainingAltitude && pRocket->MissionState <= RocketMissionState::ClosingIn;
	}

	static bool __stdcall _Process(ILocomotion* pThis)
	{
		auto pRocket = static_cast<RocketLocomotionClass*>(pThis);
		const auto pAir = pRocket->Owner;
		const auto pAirType = pAir->GetTechnoType();
		const RocketStruct* rocket = GetRocketData(pAirType);
		auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pAirType);
		TechnoClass* spawn_owner = pAir->SpawnOwner;

		switch (pRocket->MissionState)
		{
		case RocketMissionState::Pause:
		{
			pRocket->CurrentSpeed = 0.0;
			pRocket->SpawnerIsElite = spawn_owner && spawn_owner->Veterancy.IsElite();

			const bool iscruise = IsCruise(pAirType);

			if (!iscruise)
			{
				pRocket->NeedToSubmit = true;
			}
			else
			{
				if (pRocket->TrailerTimer.Expired())
				{
					if (auto pTakeOff = GetTakeOffAnim(pAirType))
					{
						AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pTakeOff, pAir->Location, 2, 1, 0x600, -10),
						  pAir->Owner,
						  pAir->Target ? pAir->Target->GetOwningHouse() : nullptr,
						  pAir,
						  true);
					}

					pRocket->TrailerTimer.Start(24);
				}

				if (pRocket->NeedToSubmit)
				{
					pAir->Mark(MarkType::Remove);
					pRocket->NeedToSubmit = false;
					DisplayClass::Instance->SubmitObject(pAir);
					pAir->Mark(MarkType::Put);
				}
			}

			if (pRocket->MissionTimer.Expired())
			{
				pRocket->MissionState = !iscruise ? RocketMissionState::VerticalTakeOff : RocketMissionState::GainingAltitude;
				pRocket->MissionTimer.Start(rocket->TiltFrames);
			}
			break;
		}
		case RocketMissionState::Tilt:
		{
			pRocket->CurrentSpeed = 0.0;
			pRocket->SpawnerIsElite = spawn_owner && spawn_owner->Veterancy.IsElite();

			if (pRocket->MissionTimer.Percent_Expired() != 1.0) {

				const double pitch_initial = rocket->PitchInitial * Math::DEG90_AS_RAD;
				const double pitch_final = rocket->PitchFinal * Math::DEG90_AS_RAD;
				pRocket->CurrentPitch = float((pitch_final - pitch_initial) * pRocket->MissionTimer.Percent_Expired() + pitch_initial);

			}
			else
			{
				pRocket->CurrentPitch = float(rocket->PitchFinal * Math::DEG90_AS_RAD);
				pRocket->MissionState = RocketMissionState::GainingAltitude;
				auto lastFlight = pAir->GetLastFlightMapCoords();

				if (lastFlight == CellStruct::Empty)
					AircraftTrackerClass::Instance->Add(pAir);

				if (auto pTakeOff = GetTakeOffAnim(pAirType))
				{
					AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pTakeOff, pAir->Location, 2, 1, 0x600, -10),
					  pAir->Owner,
					  pAir->Target ? pAir->Target->GetOwningHouse() : nullptr,
					  pAir,
					  true);
				}

				VocClass::SafeImmedietelyPlayAtpAirType->AuxSound1, pAir->Location);
			}

			break;
		}
		case RocketMissionState::GainingAltitude:
		{
			if (!pRocket->NeedToSubmit)
			{
				pAir->Mark(MarkType::Remove);
				pRocket->NeedToSubmit = true;
				DisplayClass::Instance->SubmitObject(pAir);
				pAir->Mark(MarkType::Put);
			}

			pRocket->CurrentSpeed += rocket->Acceleration;
			pRocket->CurrentSpeed = std::min(pRocket->CurrentSpeed, static_cast<double>(pAirType->Speed));

			if (pAir->GetHeight() >= rocket->Altitude)
			{
				pRocket->MissionState = RocketMissionState::Flight;
				Coordinate center_coord = pAir->GetCoords();
				CellStruct center_cell = CellClass::Coord2Cell(center_coord);
				CellStruct dest_loc = CellClass::Coord2Cell(pRocket->MovingDestination);
				pRocket->ApogeeDistance = Vector2D<float>(static_cast<float>(center_cell.X - dest_loc.X), static_cast<float>(center_cell.Y - dest_loc.Y)).Length();
			}
			break;
		}
		case RocketMissionState::Flight:
		{

			if (pAir->GetHeight() <= 0)
			{
				Explode(pRocket);
				return false;
			} else {
				pRocket->CurrentSpeed += rocket->Acceleration;
				pRocket->CurrentSpeed = std::min(pRocket->CurrentSpeed, static_cast<double>(pAirType->Speed));

				if (rocket->LazyCurve && pRocket->ApogeeDistance)
				{
					if (Time_To_Explode(pRocket, rocket))
						return false;

					const Coordinate center_coord = pAir->GetCoords();
					CellStruct center_cell = CellClass::Coord2Cell(center_coord);
					CellStruct dest_loc = CellClass::Coord2Cell(pRocket->MovingDestination);
					const double dist = (center_cell - dest_loc).Length();
					const double ratio = dist / pRocket->ApogeeDistance;

					pRocket->CurrentPitch = float(rocket->PitchFinal * ratio * Math::DEG90_AS_RAD + Get_Next_Pitch(pRocket) * (1 - ratio));
				}
				else
				{

					if (pRocket->CurrentPitch > 0.0)
					{
						pRocket->CurrentPitch -= rocket->TurnRate;
						pRocket->CurrentPitch = (float)std::max((double)pRocket->CurrentPitch, 0.0);
					}

					const Coordinate center_coord = pAir->GetCoords();
					CellStruct center_cell = CellClass::Coord2Cell(center_coord);
					CellStruct dest_loc = CellClass::Coord2Cell(pRocket->MovingDestination);
					if ((center_cell - dest_loc).Length() <= pAir->Location.Z - pRocket->MovingDestination.Z)
						pRocket->MissionState = RocketMissionState::ClosingIn;
				}

				const Coordinate center_coord = pAir->GetCoords();
				const DirStruct desired { center_coord.X, pRocket->MovingDestination.Y, pRocket->MovingDestination.X, center_coord.Y };
				pAir->PrimaryFacing.Set_Desired(desired);
			}

			break;
		}
		case RocketMissionState::ClosingIn:
		{
			if (Time_To_Explode(pRocket, rocket))
				return false;

			const double pitch = Get_Next_Pitch(pRocket) - pRocket->CurrentPitch;

			if (Math::abs(pitch) > rocket->TurnRate)
				pRocket->CurrentPitch = float(pitch < 0 ? pRocket->CurrentPitch - rocket->TurnRate : pRocket->CurrentPitch + rocket->TurnRate);
			else
				pRocket->CurrentPitch += (float)pitch;

			break;
		}
		case RocketMissionState::VerticalTakeOff:
		{
			pRocket->SpawnerIsElite = spawn_owner && spawn_owner->Veterancy.IsElite();

			if (pRocket->TrailerTimer.Expired())
			{

				if (auto pTakeOff = GetTakeOffAnim(pAirType))
				{
					AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pTakeOff, pAir->Location, 2, 1, 0x600, -10),
					  pAir->Owner,
					  pAir->Target ? pAir->Target->GetOwningHouse() : nullptr,
					  pAir,
					  true);

					pRocket->TrailerTimer.Start(24);
				}
			}

			if (pRocket->MissionTimer.Percent_Expired() != 1.0)
			{
				Coordinate coord = pAir->Location;
				coord.Z += rocket->RaiseRate;
				if (MapClass::Instance->IsWithinUsableArea(CellClass::Coord2Cell(coord), true))
					pAir->SetLocation(coord);
			}
			else
			{
				pRocket->CurrentPitch = float(rocket->PitchFinal * Math::DEG90_AS_RAD);
				auto Lastflight_coord = pAir->GetLastFlightMapCoords();

				if (Lastflight_coord == CellStruct::Empty)
				{
					VocClass::SafeImmedietelyPlayAtpAirType->AuxSound1, pAir->Location);
					AircraftTrackerClass::Instance->Add(pAir);
				}
				pRocket->TrailerTimer.Start(0);
				pRocket->MissionState = RocketMissionState::GainingAltitude;
			}

			break;
		}
		default:
			break;
		}

		if (pRocket->Is_Moving_Now() && pRocket->TrailerTimer.Expired())
		{
			if (auto pAnim = GetTrailAnim(pAirType))
			{
				AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnim, pAir->Location, pTypeExt->CustomMissileTrailAppearDelay, 1, 0x600),
					pAir->Owner,
					pAir->Target ? pAir->Target->GetOwningHouse() : nullptr,
					pAir,
					true);

				pRocket->TrailerTimer.Start(GetTrailerSeparation(pAirType));
			}
		}

		if (pRocket->CurrentSpeed > 0.0)
		{

			Coordinate coord = Get_Next_Position(pRocket, pRocket->CurrentSpeed);

			if (MapClass::Instance->IsWithinUsableArea(CellClass::Coord2Cell(coord), true))
				pAir->SetLocation(coord);

			auto curCell = pAir->GetMapCoords();

			if (curCell != pAir->LastFlightMapCoords && pAir->IsAlive)
				AircraftTrackerClass::Instance->Update(pAir, pAir->LastFlightMapCoords, curCell);

			if (pAirType->Strength <= 0)
				Explode(pRocket, coord);
		}

		return pRocket->Is_Moving();
	}

public:

	static void Explode(RocketLocomotionClass* pThis, CoordStruct next = CoordStruct::Empty)
	{
		auto pLinked = pThis->LinkedTo;
		auto pRocketType = pLinked->GetTechnoType();

		const RocketStruct* rocket = GetRocketData(pRocketType);

		if (next == CoordStruct::Empty)
			next = Get_Next_Position(pThis, rocket->BodyLength);

		if (!RocketHasWeapon(pLinked, pRocketType, pThis->SpawnerIsElite, next))
		{

			CellStruct cell = CellClass::Coord2Cell(next);
			const auto pCell = MapClass::Instance->GetCellAt(next);

			const int damage = pThis->SpawnerIsElite ? rocket->EliteDamage : rocket->Damage;
			const auto pWH = GetRocketWarhead(pRocketType, pThis->SpawnerIsElite);

			if (auto pAnimType = MapClass::SelectDamageAnimation(damage, pWH, pCell->LandType, next))
			{
				AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnimType, next, 0, 1, 0x2600, -15),
					pLinked->Owner,
					pLinked->Target ? pLinked->Target->GetOwningHouse() : nullptr,
					pLinked,
					true
				);
			}

			MapClass::FlashbangWarheadAt(damage, pWH, next, false, SpotlightFlags::None);
			//setting ownership here will cause bug , better not
			DamageArea::Apply(&next, damage, nullptr, pWH, true, nullptr);
		}

		if (pLinked->IsAlive)
			TechnoExtData::HandleRemove(pLinked, nullptr, false, false);
	}

	static bool Time_To_Explode(RocketLocomotionClass* pThis, const RocketStruct* rocket)
	{
		auto pLinked = pThis->Owner;
		Coordinate coord = Get_Next_Position(pThis, rocket->BodyLength);

		if (coord.Z > pThis->MovingDestination.Z)
		{
			const CellClass* rocket_cell = pLinked->GetCell();
			if (!rocket_cell
				|| (rocket_cell->UINTFlags & 0x100) == 0
				|| pThis->MovingDestination.Z != rocket_cell->GetCoords().Z
				|| coord.Z > pThis->MovingDestination.Z + Unsorted::BridgeHeight)
			{
				if (pLinked->GetHeight() > 0)
					return false;
			}
		}

		Explode(pThis);
		return true;
	}

	static double Get_Next_Pitch(RocketLocomotionClass* pThis)
	{
		const Coordinate left_to_go = pThis->MovingDestination - pThis->Owner->Location;
		const double length = Vector2D<float> { static_cast<float>(left_to_go.X), static_cast<float>(left_to_go.Y) }.Length();

		if (length > 0)
			return std::atan(double(left_to_go.Z / length));

		return -Math::DEG90_AS_RAD;
	}

	static Coordinate Get_Next_Position(RocketLocomotionClass* pThis, double speed)
	{
		const double horizontal_speed = std::cos((double)pThis->CurrentPitch) * speed;
		const double horizontal_angle = pThis->Owner->PrimaryFacing.Current().GetRadian<65536>();

		return {
		int(pThis->Owner->Location.X + std::cos(horizontal_angle) * horizontal_speed)
		, int(pThis->Owner->Location.Y - std::sin(horizontal_angle) * horizontal_speed)
		, int(pThis->Owner->Location.Z + std::sin((double)pThis->CurrentPitch) * speed) };
	}
};

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F0B60, _RocketLocomotionClass::_Move_To));
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F0B40, _RocketLocomotionClass::_Draw_Matrix));
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F0B9C, _RocketLocomotionClass::_Is_Moving_Now));
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F0B5C, _RocketLocomotionClass::_Process));
#else


#pragma region RocketLocoHooks
//TODO : Cmisl Hardcoded shit
// 662496
// 66235D

//static DamageAreaResult FC _RocketLocomotionClass_DamageArea(
//	CoordStruct* pCoord,
//	int Damage,
//	TechnoClass* Source,
//	WarheadTypeClass* Warhead,
//	bool AffectTiberium,
//	HouseClass* SourceHouse //nullptr
//
//)
//{
//	HouseClass* pHouseOwner = Source ? Source->Owner : SourceHouse;
//	return DamageArea::Apply
//	(pCoord, Damage, Source, Warhead, Warhead->Tiberium, pHouseOwner);
//}

//DEFINE_FUNCTION_JUMP(CALL, 0x6632C7, GET_OFFSET(_RocketLocomotionClass_DamageArea));

#pragma region RocketLocoHooks

#pragma region Process


ASMJIT_PATCH(0x6622E0, RocketLocomotionClass_ILocomotion_Process_CustomMissile, 6)
{
	GET(AircraftClass* const, pThis, ECX);

	const auto pExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);

	if (pExt->IsCustomMissile)
	{
		R->EAX(pExt->CustomMissileData.operator->());
		return 0x66230A;
	}

	return 0;
}

ASMJIT_PATCH(0x66238A, RocketLocomotionClass_ILocomotion_Process_CustomMissileTakeoff1, 5)
{
	GET(ILocomotion* const, pThis, ESI);

	const auto pLocomotor = static_cast<RocketLocomotionClass* const>(pThis);
	const auto pOwner = static_cast<AircraftClass* const>(pLocomotor->LinkedTo);
	const auto pExt = TechnoTypeExtContainer::Instance.Find(pOwner->Type);

	if (AnimTypeClass* pType = pExt->CustomMissileTakeoffAnim)
	{

		AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pType, pOwner->Location, 2, 1, 0x600, -10, false),
			pOwner->Owner,
			nullptr,
			pOwner,
			true , false
		);

		return 0x6623F3;
	}

	return 0;
}

ASMJIT_PATCH(0x662512, RocketLocomotionClass_ILocomotion_Process_CustomMissileTakeoff2, 5)
{
	GET(ILocomotion* const, pThis, ESI);

	const auto pLocomotor = static_cast<RocketLocomotionClass* const>(pThis);
	const auto pOwner = static_cast<AircraftClass* const>(pLocomotor->LinkedTo);
	const auto pExt = TechnoTypeExtContainer::Instance.Find(pOwner->Type);

	if (AnimTypeClass* pType = pExt->CustomMissileTakeoffAnim)
	{
		AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pType, pOwner->Location, 2, 1, 0x600, -10, false),
			pOwner->Owner,
			nullptr,
			pOwner,
			true, false
		);

		return 0x66257B;
	}

	return 0;
}

ASMJIT_PATCH(0x6627E5, RocketLocomotionClass_ILocomotion_Process_CustomMissileTakeoff3, 5)
{
	GET(ILocomotion* const, pThis, ESI);

	const auto pLocomotor = static_cast<RocketLocomotionClass* const>(pThis);
	const auto pOwner = static_cast<AircraftClass* const>(pLocomotor->LinkedTo);
	const auto pExt = TechnoTypeExtContainer::Instance.Find(pOwner->Type);

	if (AnimTypeClass* pType = pExt->CustomMissileTakeoffAnim)
	{
		AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pType, pOwner->Location, 2, 1, 0x600, -10, false),
			pOwner->Owner,
			nullptr,
			pOwner,
			true, false
		);

		return 0x662849;
	}

	return 0;
}

ASMJIT_PATCH(0x662D85, RocketLocomotionClass_ILocomotion_Process_CustomMissileTrailer, 6)
{
	GET(ILocomotion* const, pThis, ESI);

	const auto pLocomotor = static_cast<RocketLocomotionClass* const>(pThis);

	if (pLocomotor->TrailerTimer.Expired())
	{
		const auto pOwner = static_cast<AircraftClass* const>(pLocomotor->LinkedTo);
		const auto pExt = TechnoTypeExtContainer::Instance.Find(pOwner->Type);

		pLocomotor->TrailerTimer.Start(pExt->CustomMissileTrailerSeparation);

		if (AnimTypeClass* pType = pExt->CustomMissileTrailerAnim)
		{
			AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pType, pOwner->Location),
				pOwner->Owner,
				nullptr,
				pOwner,
				true, false
			);
		}

		return 0x662E16;
	}

	return 0;
}

// new
// SpawnerOwner may die when this processed , should store the data at SpawnManagerExt or something
ASMJIT_PATCH(0x662720, RocketLocomotionClass_ILocomotion_Process_Raise, 0x6)
{
	enum { Handled = 0x6624C8, Continue = 0x0 };

	GET(RocketLocomotionClass* const, pThis, ESI);

	if (const auto pAir = static_cast<AircraftClass* const>(pThis->Owner))
	{
		const auto pExt = TechnoTypeExtContainer::Instance.Find(pAir->Type);
		if (pExt->IsCustomMissile.Get() && pAir->SpawnOwner)
		{
			if (!pExt->CustomMissileRaise.GetFromSpecificRank(pThis->SpawnerIsElite ? Rank::Elite : pAir->SpawnOwner->CurrentRanking))
				return Handled;
		}
	}

	return Continue;
}
#pragma endregion

#pragma region Explode


ASMJIT_PATCH(0x66305A, RocketLocomotionClass_Explode_CustomMissile, 6)
{
	GET(AircraftTypeClass* const, pType, ECX);
	GET(RocketLocomotionClass* const, pLocomotor, ESI);

	LEA_STACK(WarheadTypeClass**, ppWarhead, 0x10);
	LEA_STACK(RocketStruct**, ppRocketData, 0x14);

	const auto pExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pExt->IsCustomMissile)
	{
		*ppRocketData = pExt->CustomMissileData.operator->();

		const bool isElite = pLocomotor->SpawnerIsElite;
		*ppWarhead = (isElite ? pExt->CustomMissileEliteWarhead : pExt->CustomMissileWarhead);

		return 0x6630DD;
	}

	return 0;
}

ASMJIT_PATCH(0x663218, RocketLocomotionClass_Explode_CustomMissile2, 5)
{
	GET(RocketLocomotionClass* const, pThis, ESI);
	REF_STACK(CoordStruct const, coords, STACK_OFFS(0x60, 0x18));

	const auto pOwner = static_cast<AircraftClass* const>(pThis->LinkedTo);
	const auto pExt = TechnoTypeExtContainer::Instance.Find(pOwner->Type);

	if (pExt->IsCustomMissile)
	{
		if (auto const& pWeapon = pThis->SpawnerIsElite
			? pExt->CustomMissileEliteWeapon : pExt->CustomMissileWeapon)
		{
			WeaponTypeExtData::DetonateAt3(pWeapon, coords, pOwner, true, pOwner ? pOwner->Owner : nullptr);
			return 0x6632CC;
		}
	}

	GET(int, nDamage, EDI);
	GET_STACK(WarheadTypeClass* const, pWH, STACK_OFFS(0x60, 0x50));
	LEA_STACK(CellStruct* const, pCellStr, STACK_OFFS(0x60, 0x38));
	const auto pCell = MapClass::Instance->GetCellAt(pCellStr);

	if (auto pAnimType = MapClass::SelectDamageAnimation(nDamage, pWH, pCell->LandType, coords))
	{
		AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnimType, coords, 0, 1, 0x2600, -15),
			pOwner->Owner,
			pOwner->Target ? pOwner->Target->GetOwningHouse() : nullptr,
			pOwner,
			true, false
		);
	}

	//modifyng code below will cause missile to alive even after detonated
	//this need to be fixed in a different way ,..
	return 0x66328C;
}

#pragma endregion

#pragma endregion

#pragma endregion

#endif