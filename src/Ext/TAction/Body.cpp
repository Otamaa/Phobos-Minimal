#include "Body.h"

#include <SessionClass.h>
#include <MessageListClass.h>
#include <HouseClass.h>
#include <CRT.h>
#include <SuperWeaponTypeClass.h>
#include <SuperClass.h>
#include <Ext/SWType/Body.h>
#include <Utilities/SavegameDef.h>

#include <BuildingClass.h>
#include <RadSiteClass.h>
#include <LightSourceClass.h>

#include <Ext/Scenario/Body.h>
#include <Ext/Terrain/Body.h>

//Static init
#include <TagClass.h>

TActionExt::ExtContainer TActionExt::ExtMap;

template <typename T>
void TActionExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Value1)
		.Process(this->Value2)
		.Process(this->Parm3)
		.Process(this->Parm4)
		.Process(this->Parm5)
		.Process(this->Parm6)
		;
}

void TActionExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<TActionClass>::Serialize(Stm);
	this->Serialize(Stm);
}

void TActionExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<TActionClass>::Serialize(Stm);
	this->Serialize(Stm);
}

bool TActionExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool TActionExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

bool TActionExt::DrawLaserBetweenWaypoints(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	TActionExt::ExtData* pExt = TActionExt::ExtMap.Find(pThis);
	int duration = atoi(pExt->Value2.c_str());
	int idx1 = pThis->Param3;
	int idx2 = pThis->Param4;
	ColorStruct innerColor = Drawing::RGB888_HEX(pExt->Parm5.c_str());
	ColorStruct outerColor = Drawing::RGB888_HEX(pExt->Parm6.c_str());
	CellStruct srcCell = ScenarioClass::Instance->GetWaypointCoords(idx1);
	CellStruct destCell = ScenarioClass::Instance->GetWaypointCoords(idx2);
	CoordStruct src = CellClass::Cell2Coord(srcCell, 100);
	CoordStruct dest = CellClass::Cell2Coord(destCell, 100);
	LaserDrawClass* pLaser = GameCreate<LaserDrawClass>(src, dest, innerColor, outerColor, outerColor, duration);
	pLaser->IsHouseColor = true;
	pLaser->Thickness = 7;

	return true;
}

// =============================
// container

TActionExt::ExtContainer::ExtContainer() : Container("TActionClass") { };
TActionExt::ExtContainer::~ExtContainer() = default;

static bool something_700(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject,
	TriggerClass* pTrigger, CellStruct const& location)
{
	if (!pTrigger)
		return false;

	auto nVal = pThis->Value;
	if (nVal <= 2) {
		if (nVal < 0)
			nVal = 0;
	} else {
		nVal = 2;
	}

	bool IsEligible = false;
	for (auto pTech : *TechnoClass::Array)
	{
		if (pTech && pTech->IsAlive && pTech->IsOnMap && !pTech->InLimbo && !(pTech->IsCrashing || pTech->IsSinking))
		{
			if (!pTech->AttachedTag || !pTech->AttachedTag->ContainsTrigger(pTrigger))
			{
				IsEligible = false;
			}
			else
			{
				IsEligible = true;
				pTech->Veterancy.Veterancy = (nVal * 1.0f);
			}
		}
	}

	return IsEligible;

}

static bool something_701(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject,
	TriggerClass* pTrigger, CellStruct const& location)
{
	if (!pTrigger)
		return false;

	auto pOwner = pThis->FindHouseByIndex(pTrigger, pThis->Param4);

	if (!pOwner)
		return false;

	if (pThis->Param3) {
		auto nAmount = pOwner->Available_Money();
		pOwner->TakeMoney(nAmount);
		pOwner->GiveMoney(abs(pThis->Value));
	} else {
		pOwner->TransactMoney(pThis->Value);
	}

	return true;
}

static bool something_702(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject,
	TriggerClass* pTrigger, CellStruct const& location)
{
	if (!pTrigger)
		return false;

	auto pOwner = pThis->FindHouseByIndex(pTrigger, pThis->Param3);

	if (!pOwner)
		return false;

	auto pSuperType = SuperWeaponTypeClass::FindIndex(pThis->Text);

	if (pSuperType == -1 || pThis->Waypoint == -1)
		return false;

	CellStruct nBufer { };
	ScenarioGlobal->GetWaypointCoords(&nBufer, pThis->Waypoint);

	if(const auto pSuper = pOwner->Supers[pSuperType]) {
		pSuper->IsCharged = true;
		pSuper->Launch(nBufer, pOwner == HouseClass::CurrentPlayer());
	}

	return true;
}

static bool something_703(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject,
	TriggerClass* pTrigger, CellStruct const& location)
{
	if (!pTrigger)
		return false;

	auto pOwner = pThis->FindHouseByIndex(pTrigger, pThis->Param3);

	if (!pOwner)
		return false;

	switch (abs(pThis->Value))
	{
	case 0 :
		pOwner->AIMode = AIMode::General;
		break;
	case 1 :
	case 2 :
		pOwner->AIMode = AIMode::LowOnCash;
		break;
	case 3 :
		pOwner->AIMode = AIMode::BuildBase;
		break;
	default:
		pOwner->AIMode = AIMode::SellAll;
		break;
	}

	return true;
}

static bool something_704(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject,
	TriggerClass* pTrigger, CellStruct const& location)
{
	if (!pTrigger)
		return false;

	auto pAnimType  = AnimTypeClass::Find(pThis->Text);

	if (!pAnimType || !pAnimType->Image)
		return false;

	auto pImage = pAnimType->Image;
	const auto MapRect = &Make_Global<RectangleStruct>(0x87F8DC);

	/*
	87F8D4 = X
	87F8D8 = Y
	87F8DC = Width
	87F8E0 = Height
	*/

	int nShpWidth = pImage->Width;
	int nHeight = pImage->Height;
	auto nShpWidth_ = nShpWidth;
	auto nRectByt = 30 * MapRect->Width;
	auto v29 = nHeight / 2 - 30 * MapRect->Width;

	if (v29 >= 30 * MapRect->Width)
		return true;

	auto nDimension = (15 * MapRect->Height + nShpWidth / 2);
	auto v33 = 45 * MapRect->Height;

	do
	{
		if (nDimension < v33)
		{
			do
			{
				Vector3D<float> vect { v29 * 1.0f, nDimension * 1.0f  , 0.0f};
				Vector3D<float> Vec3Dresult { };
				Matrix3D::MatrixMultiply(&Vec3Dresult, &TacticalGlobal->IsoTransformMatrix, &vect);
				auto nCoord = CoordStruct { (int)Vec3Dresult.X , (int)Vec3Dresult.Y , 0 };
				GameCreate<AnimClass>(pAnimType, nCoord);
				nDimension += nShpWidth_;
			}
			while (nDimension < v33);
		}
		v29 += nHeight;
	}
	while (v29 < nRectByt);


	return true;
}

static bool something_705(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject,
	TriggerClass* pTrigger, CellStruct const& location)
{
	if (!pTrigger)
		return false;

	auto pOwner = pThis->FindHouseByIndex(pTrigger, pThis->Param3);

	if (!pOwner)
		return false;

	CellStruct nBufer { };
	ScenarioGlobal->GetWaypointCoords(&nBufer, pThis->Waypoint);
	const auto pCell = Map.TryGetCellAt(nBufer);

	for (auto pFoot : *FootClass::Array)
	{
		if (pFoot->Owner == pOwner)
		{
			pFoot->SetDestination(pCell, false);
		}
	}

	return true;
}

static bool something_713(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject,
	TriggerClass* pTrigger, CellStruct const& location)
{
	if (!pTrigger)
		return false;

	for (auto pTech : *TechnoClass::Array) {
		if (pTech && pTech->IsAlive && pTech->IsOnMap && !pTech->InLimbo && !(pTech->IsCrashing || pTech->IsSinking))
		{
			if (pTech->AttachedTag && pTech->AttachedTag->ContainsTrigger(pTrigger))
				pTech->Flash(pThis->Value);
		}
	}

	return true;
}

static bool something_716(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject,
	TriggerClass* pTrigger, CellStruct const& location)
{
	if (!pTrigger)
		return false;

	auto pOwner = pThis->FindHouseByIndex(pTrigger, pThis->Value);

	if (!pOwner)
		return false;

	for (auto pTech : *TechnoClass::Array)
	{
		if (pTech && pTech->IsAlive && pTech->IsOnMap && !pTech->InLimbo && !(pTech->IsCrashing || pTech->IsSinking))
		{
			auto pOrigOwner = pTech->GetOriginalOwner();
			if ((pOrigOwner == pOwner && pTech->Owner == pOrigOwner) || !pTech->CaptureManager || !pTech->CaptureManager->SetOriginalOwnerToCivilian())
			{
				if (auto pTemp = pTech->TemporalTargetingMe)
					pTemp->JustLetGo();

				pTech->UnInit();
			}
		}
	}

	return true;
}

static bool something_717(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject,
	TriggerClass* pTrigger, CellStruct const& location)
{
	if (!pTrigger)
		return false;

	for (auto pTech : *TechnoClass::Array) {
		if (pTech && pTech->IsAlive && pTech->IsOnMap && !pTech->InLimbo && !(pTech->IsCrashing || pTech->IsSinking)) {
			GameDelete(pTech);
		}
	}

	return true;
}

CoordStruct* GetSomething(CoordStruct* a1)
{
	auto MapRect = Make_Global<RectangleStruct>(0x87F8DC);
	auto v1 = 60 * MapRect.Width;
	auto v2 = 30 * MapRect.Height;
	auto vect_X = ScenarioGlobal->Random.RandomFromMax((60 * MapRect.Width) - v1 / 2);
	auto vect_Y = (v2 / 2 + ScenarioGlobal->Random.RandomFromMax(v2));
	Vector3D<float> vect { (float)vect_X , (float)vect_Y , 0.0f};
	Vector3D<float> Vec3Dresult { };
	Matrix3D::MatrixMultiply(&Vec3Dresult, &TacticalGlobal->IsoTransformMatrix, &vect);
	a1->Z = 0;
	a1->X = (int)Vec3Dresult.X;
	a1->Y = (int)Vec3Dresult.Y;
	return a1;
}

static bool something_720(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject,
	TriggerClass* pTrigger, CellStruct const& location)
{
	if (pThis->Value <= 0 || !pObject)
		return false;

	for (int i = 0; i < pThis->Value; ++i) {
		auto ncell = pObject->GetMapCoords();
		LightningStorm::Strike(ncell);
	}

	return true;
}


bool TActionExt::Execute(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject,
	TriggerClass* pTrigger, CellStruct const& location, bool& bHandled)
{
	bHandled = true;

	// Vanilla
	switch (pThis->ActionKind)
	{
	case TriggerAction::PlaySoundEffectRandom:
		return TActionExt::PlayAudioAtRandomWP(pThis, pHouse, pObject, pTrigger, location);
	default:
		break;
	};

	// Phobos
	switch (static_cast<PhobosTriggerAction>(pThis->ActionKind))
	{
	case PhobosTriggerAction::SaveGame:
		return TActionExt::SaveGame(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::EditVariable:
		return TActionExt::EditVariable(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::GenerateRandomNumber:
		return TActionExt::GenerateRandomNumber(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::PrintVariableValue:
		return TActionExt::PrintVariableValue(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::BinaryOperation:
		return TActionExt::BinaryOperation(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::AdjustLighting:
		return TActionExt::AdjustLighting(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::RunSuperWeaponAtLocation:
		return TActionExt::RunSuperWeaponAtLocation(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::RunSuperWeaponAtWaypoint:
		return TActionExt::RunSuperWeaponAtWaypoint(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::Something_700:
		return something_700(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::Something_701:
		return something_701(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::LauchSWAtWaypoint:
		return something_702(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::AISetMode:
		return something_703(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::Something_704:
		return something_704(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::Something_705:
		return something_705(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::DoFlash:
		return something_713(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::Something_716:
		return something_716(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::Something_717:
		return something_717(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::DoLighningStormStrike:
		return something_720(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::DrawLaserBetweenWeaypoints:
		return TActionExt::DrawLaserBetweenWaypoints(pThis, pHouse, pObject, pTrigger, location);
	default:
		bHandled = false;
		return true;
	}
}

bool TActionExt::PlayAudioAtRandomWP(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	std::vector<int> waypoints;
	waypoints.reserve(ScenarioExt::Global()->Waypoints.size());

	auto const pScen = ScenarioClass::Instance();

	for (auto pair : ScenarioExt::Global()->Waypoints)
		if (pScen->IsDefinedWaypoint(pair.first))
			waypoints.push_back(pair.first);

	if (waypoints.size() > 0)
	{
		auto const index = pScen->Random.RandomRanged(0, waypoints.size() - 1);
		auto const luckyWP = waypoints[index];
		auto const cell = pScen->GetWaypointCoords(luckyWP);
		auto const coords = CellClass::Cell2Coord(cell);
		VocClass::PlayIndexAtPos(pThis->Value, coords);
	}

	return true;
}

bool TActionExt::SaveGame(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	if (SessionClass::Instance->GameMode == GameMode::Campaign || SessionClass::Instance->GameMode == GameMode::Skirmish)
	{
		auto PrintMessage = [](const wchar_t* pMessage)
		{
			MessageListClass::Instance->PrintMessage(
				pMessage,
				RulesClass::Instance->MessageDelay,
				HouseClass::CurrentPlayer->ColorSchemeIndex,
				true
			);
		};

		char fName[0x80];

		SYSTEMTIME time;
		GetLocalTime(&time);

		_snprintf_s(fName, 0x7F, "Map.%04u%02u%02u-%02u%02u%02u-%05u.sav",
			time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);

		PrintMessage(StringTable::LoadString("TXT_SAVING_GAME"));

		wchar_t fDescription[0x80] = { 0 };
		wcscpy_s(fDescription, ScenarioClass::Instance->UINameLoaded);
		wcscat_s(fDescription, L" - ");
		wcscat_s(fDescription, StringTable::LoadString(pThis->Text));

		if (ScenarioClass::Instance->SaveGame(fName, fDescription))
			PrintMessage(StringTable::LoadString("TXT_GAME_WAS_SAVED"));
		else
			PrintMessage(StringTable::LoadString("TXT_ERROR_SAVING_GAME"));
	}

	return true;
}

bool TActionExt::EditVariable(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	// Variable Index
	// holds by pThis->Value

	// Operations:
	// 0 : set value - operator=
	// 1 : add value - operator+
	// 2 : minus value - operator-
	// 3 : multiply value - operator*
	// 4 : divide value - operator/
	// 5 : mod value - operator%
	// 6 : <<
	// 7 : >>
	// 8 : ~ (no second param being used)
	// 9 : ^
	// 10 : |
	// 11 : &
	// holds by pThis->Param3

	// Params:
	// The second value
	// holds by pThis->Param4

	// Global Variable or Local
	// 0 for local and 1 for global
	// holds by pThis->Param5

	// uses !pThis->Param5 to ensure Param5 is 0 or 1
	auto& variables = ScenarioExt::Global()->Variables[pThis->Param5 != 0];
	auto itr = variables.find(pThis->Value);
	if (itr != variables.end())
	{
		auto& nCurrentValue = itr->second.Value;
		// variable being found
		switch (pThis->Param3)
		{
		case 0: { nCurrentValue = pThis->Param4; break; }
		case 1: { nCurrentValue += pThis->Param4; break; }
		case 2: { nCurrentValue -= pThis->Param4; break; }
		case 3: { nCurrentValue *= pThis->Param4; break; }
		case 4: { nCurrentValue /= pThis->Param4; break; }
		case 5: { nCurrentValue %= pThis->Param4; break; }
		case 6: { nCurrentValue <<= pThis->Param4; break; }
		case 7: { nCurrentValue >>= pThis->Param4; break; }
		case 8: { nCurrentValue = ~nCurrentValue; break; }
		case 9: { nCurrentValue ^= pThis->Param4; break; }
		case 10: { nCurrentValue |= pThis->Param4; break; }
		case 11: { nCurrentValue &= pThis->Param4; break; }
		default:
			return true;
		}

		if (!pThis->Param5)
			TagClass::NotifyLocalChanged(pThis->Value);
		else
			TagClass::NotifyGlobalChanged(pThis->Value);
	}
	return true;
}

bool TActionExt::GenerateRandomNumber(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	auto& variables = ScenarioExt::Global()->Variables[pThis->Param5 != 0];
	auto itr = variables.find(pThis->Value);
	if (itr != variables.end())
	{
		itr->second.Value = ScenarioClass::Instance->Random.RandomRanged(pThis->Param3, pThis->Param4);
		if (!pThis->Param5)
			TagClass::NotifyLocalChanged(pThis->Value);
		else
			TagClass::NotifyGlobalChanged(pThis->Value);
	}

	return true;
}

bool TActionExt::PrintVariableValue(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	auto& variables = ScenarioExt::Global()->Variables[pThis->Param3 != 0];
	auto itr = variables.find(pThis->Value);
	if (itr != variables.end())
	{
		CRT::swprintf(Phobos::wideBuffer, L"%d", itr->second.Value);
		MessageListClass::Instance->PrintMessage(Phobos::wideBuffer);
	}

	return true;
}

bool TActionExt::BinaryOperation(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	auto& variables1 = ScenarioExt::Global()->Variables[pThis->Param5 != 0];
	auto itr1 = variables1.find(pThis->Value);
	auto& variables2 = ScenarioExt::Global()->Variables[pThis->Param6 != 0];
	auto itr2 = variables2.find(pThis->Param4);

	if (itr1 != variables1.end() && itr2 != variables2.end())
	{
		auto& nCurrentValue = itr1->second.Value;
		auto& nOptValue = itr2->second.Value;
		switch (pThis->Param3)
		{
		case 0: { nCurrentValue = nOptValue; break; }
		case 1: { nCurrentValue += nOptValue; break; }
		case 2: { nCurrentValue -= nOptValue; break; }
		case 3: { nCurrentValue *= nOptValue; break; }
		case 4: { nCurrentValue /= nOptValue; break; }
		case 5: { nCurrentValue %= nOptValue; break; }
		case 6: { nCurrentValue <<= nOptValue; break; }
		case 7: { nCurrentValue >>= nOptValue; break; }
		case 8: { nCurrentValue = nOptValue; break; }
		case 9: { nCurrentValue ^= nOptValue; break; }
		case 10: { nCurrentValue |= nOptValue; break; }
		case 11: { nCurrentValue &= nOptValue; break; }
		default:
			return true;
		}

		if (!pThis->Param5)
			TagClass::NotifyLocalChanged(pThis->Value);
		else
			TagClass::NotifyGlobalChanged(pThis->Value);
	}
	return true;
}

bool TActionExt::RunSuperWeaponAtLocation(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	if (!pThis)
		return true;

	TActionExt::RunSuperWeaponAt(pThis, pThis->Param5, pThis->Param6);

	return true;
}

bool TActionExt::RunSuperWeaponAtWaypoint(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	if (!pThis)
		return true;

	auto& waypoints = ScenarioExt::Global()->Waypoints;
	int nWaypoint = pThis->Param5;

	// Check if is a valid Waypoint
	if (nWaypoint >= 0 && waypoints.find(nWaypoint) != waypoints.end() && waypoints[nWaypoint].X && waypoints[nWaypoint].Y)
	{
		auto const selectedWP = waypoints[nWaypoint];
		TActionExt::RunSuperWeaponAt(pThis, selectedWP.X, selectedWP.Y);
	}

	return true;
}

bool TActionExt::RunSuperWeaponAt(TActionClass* pThis, int X, int Y)
{
	if (SuperWeaponTypeClass::Array->Count > 0)
	{
		int swIdx = pThis->Param3;
		int houseIdx = -1;
		std::vector<int> housesListIdx;
		CellStruct targetLocation = { (short)X, (short)Y };

		do
		{
			if (X < 0)
				targetLocation.X = ScenarioClass::Instance->Random.RandomRangedSpecific<short>(0, (short)MapClass::Instance->MapCoordBounds.Right);

			if (Y < 0)
				targetLocation.Y = ScenarioClass::Instance->Random.RandomRangedSpecific<short>(0, (short)MapClass::Instance->MapCoordBounds.Bottom);
		} while (!MapClass::Instance->IsWithinUsableArea(targetLocation, false));

		// Only valid House indexes
		if ((pThis->Param4 >= HouseClass::Array->Count
			&& pThis->Param4 < HouseClass::PlayerAtA)
			|| pThis->Param4 > (HouseClass::PlayerAtA + HouseClass::Array->Count - 3))
		{
			return true;
		}

		switch (pThis->Param4)
		{
		case HouseClass::PlayerAtA:
			houseIdx = 0;
			break;

		case HouseClass::PlayerAtB:
			houseIdx = 1;
			break;

		case HouseClass::PlayerAtC:
			houseIdx = 2;
			break;

		case HouseClass::PlayerAtD:
			houseIdx = 3;
			break;

		case HouseClass::PlayerAtE:
			houseIdx = 4;
			break;

		case HouseClass::PlayerAtF:
			houseIdx = 5;
			break;

		case HouseClass::PlayerAtG:
			houseIdx = 6;
			break;

		case HouseClass::PlayerAtH:
			houseIdx = 7;
			break;

		case -1:
			// Random non-neutral
			for (auto pHouse : *HouseClass::Array)
			{
				if (!pHouse->Defeated
					&& !pHouse->IsObserver()
					&& !pHouse->Type->MultiplayPassive)
				{
					housesListIdx.push_back(pHouse->ArrayIndex);
				}
			}

			if (housesListIdx.size() > 0)
				houseIdx = housesListIdx.at(ScenarioClass::Instance->Random.RandomRanged(0, housesListIdx.size() - 1));
			else
				return true;

			break;

		case -2:
			// Find first Neutral
			for (auto pHouseNeutral : *HouseClass::Array)
			{
				if (pHouseNeutral->IsNeutral())
				{
					houseIdx = pHouseNeutral->ArrayIndex;
					break;
				}
			}

			if (houseIdx < 0)
				return true;

			break;

		case -3:
			// Random Human Player
			for (auto pHouse : *HouseClass::Array)
			{
				if (pHouse->IsControlledByHuman()
					&& !pHouse->Defeated
					&& !pHouse->IsObserver())
				{
					housesListIdx.push_back(pHouse->ArrayIndex);
				}
			}

			if (housesListIdx.size() > 0)
				houseIdx = housesListIdx.at(ScenarioClass::Instance->Random.RandomRanged(0, housesListIdx.size() - 1));
			else
				return true;

			break;

		default:
			if (pThis->Param4 >= 0)
				houseIdx = pThis->Param4;
			else
				return true;

			break;
		}

		HouseClass* pHouse = HouseClass::Array->GetItem(houseIdx);
		SuperWeaponTypeClass* pSuperType = SuperWeaponTypeClass::Array->GetItem(swIdx);
		SuperClass* pSuper = GameCreate<SuperClass>(pSuperType, pHouse);

		if (auto const pSWExt = SWTypeExt::ExtMap.Find(pSuperType))
		{
			pSuper->SetReadiness(true);
			pSuper->Launch(targetLocation, false);
		}
	}

	return true;
}

void TActionExt::RecreateLightSources()
{
	// Yeah, we just simply recreating these lightsource...
	// Stupid but works fine.

	std::for_each(BuildingClass::Array->begin(), BuildingClass::Array->end(), [](BuildingClass* const pBld)
 {
	 if (pBld->LightSource)
	 {
		 bool activated = pBld->LightSource->Activated;

		 CallDTOR<false>(pBld->LightSource);

		 if (pBld->Type->LightIntensity)
		 {
			 TintStruct color { pBld->Type->LightRedTint, pBld->Type->LightGreenTint, pBld->Type->LightBlueTint };

			 pBld->LightSource = GameCreate<LightSourceClass>(pBld->GetCoords(),
				 pBld->Type->LightVisibility, pBld->Type->LightIntensity, color);

			 if (activated)
				 pBld->LightSource->Activate();
			 else
				 pBld->LightSource->Deactivate();
		 }
	 }
	});

	std::for_each(RadSiteClass::Array->begin(), RadSiteClass::Array->end(), [](RadSiteClass* const pRadSite)
	{
		if (pRadSite->LightSource)
		{
			bool activated = pRadSite->LightSource->Activated;
			auto coord = pRadSite->LightSource->Location;
			auto color = pRadSite->LightSource->LightTint;
			auto intensity = pRadSite->LightSource->LightIntensity;
			auto visibility = pRadSite->LightSource->LightVisibility;

			GameDelete<true>(pRadSite->LightSource);

			pRadSite->LightSource = GameCreate<LightSourceClass>(coord,
				visibility, intensity, color);

			if (activated)
				pRadSite->LightSource->Activate();
			else
				pRadSite->LightSource->Deactivate();
		}
	 });

	std::for_each(TerrainExt::ExtMap.begin(), TerrainExt::ExtMap.end(), [](auto const& nPair)
 {
	 if (nPair.second && !nPair.first->InLimbo)
	 {
		 nPair.second->ClearLightSource();
		 nPair.second->InitializeLightSource();
	 }
	});

}

bool TActionExt::AdjustLighting(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	if (pThis->Param3 != -1)
		ScenarioClass::Instance->NormalLighting.Tint.Red = pThis->Param3;
	if (pThis->Param4 != -1)
		ScenarioClass::Instance->NormalLighting.Tint.Green = pThis->Param4;
	if (pThis->Param5 != -1)
		ScenarioClass::Instance->NormalLighting.Tint.Blue = pThis->Param5;

	const int r = ScenarioClass::Instance->NormalLighting.Tint.Red * 10;
	const int g = ScenarioClass::Instance->NormalLighting.Tint.Green * 10;
	const int b = ScenarioClass::Instance->NormalLighting.Tint.Blue * 10;

	if (pThis->Value & 0b001) // Update Tiles
	{
		for (auto& pLightConvert : *LightConvertClass::Array)
			pLightConvert->UpdateColors(r, g, b, false);

		ScenarioExt::Global()->CurrentTint_Tiles = ScenarioClass::Instance->NormalLighting.Tint;
	}

	if (pThis->Value & 0b010) // Update Units & Buildings
	{
		for (auto& pScheme : *ColorScheme::Array)
			pScheme->LightConvert->UpdateColors(r, g, b, false);
		ScenarioExt::Global()->CurrentTint_Schemes = ScenarioClass::Instance->NormalLighting.Tint;
	}

	if (pThis->Value & 0b100) // Update CustomPalettes (vanilla YR LightConvertClass one, not the Ares ConvertClass only one)
	{
		ScenarioClass::UpdateHashPalLighting(r, g, b, false);
		ScenarioExt::Global()->CurrentTint_Hashes = ScenarioClass::Instance->NormalLighting.Tint;
	}

	ScenarioClass::UpdateCellLighting();
	MapClass::Instance->RedrawSidebar(1); // GScreenClass::Flag_To_Redraw

	// #issue 429
	TActionExt::RecreateLightSources();

	return true;
}
// =============================
// container hooks

DEFINE_HOOK(0x6DD176, TActionClass_CTOR, 0x5)
{
	GET(TActionClass*, pItem, ESI);
#ifdef ENABLE_NEWHOOKS
	TActionExt::ExtMap.JustAllocate(pItem, pItem, "Trying To Allocate from nullptr !");
#else
	TActionExt::ExtMap.FindOrAllocate(pItem);
#endif
	return 0;
}

DEFINE_HOOK(0x6E4696, TActionClass_SDDTOR, 0x7)
{
	GET(TActionClass*, pItem, ESI);
	TActionExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x6E3E30, TActionClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x6E3DB0, TActionClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(TActionClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	TActionExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x6E3E29, TActionClass_Load_Suffix, 0x4)
{
	TActionExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x6E3E4A, TActionClass_Save_Suffix, 0x3)
{
	TActionExt::ExtMap.SaveStatic();
	return 0;
}

/*
DEFINE_HOOK(0x6DD2DE, TActionClass_Detach, 0x5)
{
	GET(TActionClass*, pThis, ECX);
	GET(void*, target, EDX);
	GET_STACK(bool, all, STACK_OFFS(0xC, -0x8));

	if (auto pExt = TActionExt::ExtMap.Find(pThis))
		pExt->InvalidatePointer(target, all);

	return pThis->TriggerType == target ? 0x6DD2E3 : 0x6DD2E6;
}*/
