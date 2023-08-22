#include "Body.h"

#include <SessionClass.h>
#include <MessageListClass.h>
#include <Ext/House/Body.h>
#include <SuperWeaponTypeClass.h>
#include <SuperClass.h>
#include <Ext/SWType/Body.h>
#include <Utilities/SavegameDef.h>

#include <BuildingClass.h>
#include <RadSiteClass.h>
#include <LightSourceClass.h>

#include <Ext/Scenario/Body.h>
#include <Ext/Terrain/Body.h>
#include <Ext/Rules/Body.h>

//Static init
#include <TagClass.h>
#include <numeric>

std::map<int, std::vector<TriggerClass*>> TActionExt::RandomTriggerPool;

template <typename T>
void TActionExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)
		//.Process(this->Value1)
		//.Process(this->Value2)
		//.Process(this->Parm3)
		//.Process(this->Parm4)
		//.Process(this->Parm5)
		//.Process(this->Parm6)
		;
}

// =============================
// container
TActionExt::ExtContainer TActionExt::ExtMap;

TActionExt::ExtContainer::ExtContainer() : Container("TActionClass") { };
TActionExt::ExtContainer::~ExtContainer() = default;

//==============================

bool TActionExt::MessageForSpecifiedHouse(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	int houseIdx = 0;
	if (pThis->Param3 == -3)
	{
		// Random Human Player
		std::vector<int> housesListIdx;
		for (auto ptmpHouse : *HouseClass::Array)
		{
			if (ptmpHouse->IsControlledByHuman_()
				&& !ptmpHouse->Defeated
				&& !ptmpHouse->IsObserver())
			{
				housesListIdx.push_back(ptmpHouse->ArrayIndex);
			}
		}

		if (housesListIdx.size() > 0)
			houseIdx = housesListIdx.at(ScenarioClass::Instance->Random.RandomRanged(0, housesListIdx.size() - 1));
		else
			return true;
	}
	else
	{
		houseIdx = pThis->Param3;
	}

	const HouseClass* pTargetHouse = HouseClass::Index_IsMP(houseIdx) ? HouseClass::FindByIndex(houseIdx) : HouseClass::FindByCountryIndex(houseIdx);
	if (!pTargetHouse)
		return true;

	for (int i = 0; i < HouseClass::Array->Count; i++)
	{
		auto pTmpHouse = HouseClass::Array->GetItem(i);
		if (pTmpHouse->ControlledByPlayer() && pTmpHouse == pTargetHouse)
		{
			MessageListClass::Instance->PrintMessage(StringTable::LoadStringA(pThis->Text), RulesClass::Instance->MessageDelay, pTmpHouse->ColorSchemeIndex);
		}
	}
	return true;
}

bool TActionExt::SetTriggerTechnoVeterancy(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject,
	TriggerClass* pTrigger, CellStruct* plocation)
{
	if (!pTrigger)
		return false;

	auto nVal = pThis->Value;
	if (nVal <= 2)
	{
		if (nVal < 0)
			nVal = 0;
	}
	else
	{
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

bool TActionExt::TransactMoneyFor(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject,
	TriggerClass* pTrigger, CellStruct* plocation)
{
	if (!pTrigger)
		return false;

	auto pOwner = pThis->FindHouseByIndex(pTrigger, pThis->Param4);

	if (!pOwner)
		return false;

	if (pThis->Param3)
	{
		auto nAmount = pOwner->Available_Money();
		pOwner->TakeMoney(nAmount);
		pOwner->GiveMoney(abs(pThis->Value));
	}
	else
	{
		pOwner->TransactMoney(pThis->Value);
	}

	return true;
}

bool TActionExt::SetAIMode(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject,
	TriggerClass* pTrigger, CellStruct* plocation)
{
	if (!pTrigger)
		return false;

	auto pOwner = pThis->FindHouseByIndex(pTrigger, pThis->Param3);

	if (!pOwner)
		return false;

	switch (abs(pThis->Value))
	{
	case 0:
		pOwner->AIMode = AIMode::General;
		break;
	case 1:
	case 2:
		pOwner->AIMode = AIMode::LowOnCash;
		break;
	case 3:
		pOwner->AIMode = AIMode::BuildBase;
		break;
	default:
		pOwner->AIMode = AIMode::SellAll;
		break;
	}

	return true;
}

bool TActionExt::DrawAnimWithin(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject,
	TriggerClass* pTrigger, CellStruct* plocation)
{
	if (!pTrigger)
		return false;

	auto pAnimType = AnimTypeClass::Find(pThis->Text);

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
				Vector3D<float> vect { v29 * 1.0f, nDimension * 1.0f, 0.0f };
				Vector3D<float> Vec3Dresult {};
				Matrix3D::MatrixMultiply(Vec3Dresult, &TacticalClass::Instance->IsoTransformMatrix, vect);
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

bool TActionExt::SetAllOwnedFootDestinationTo(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject,
	TriggerClass* pTrigger, CellStruct* plocation)
{
	if (!pTrigger)
		return false;

	auto pOwner = pThis->FindHouseByIndex(pTrigger, pThis->Param3);

	if (!pOwner)
		return false;

	CellStruct nBufer { };
	ScenarioClass::Instance->GetWaypointCoords(&nBufer, pThis->Waypoint);
	const auto pCell = MapClass::Instance->TryGetCellAt(nBufer);

	for (auto pFoot : *FootClass::Array)
	{
		if (pFoot->Owner == pOwner)
		{
			pFoot->SetDestination(pCell, false);
		}
	}

	return true;
}

bool TActionExt::FlashTechnoFor(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject,
	TriggerClass* pTrigger, CellStruct* plocation)
{
	if (!pTrigger)
		return false;

	for (auto pTech : *TechnoClass::Array)
	{
		if (pTech && pTech->IsAlive && pTech->IsOnMap && !pTech->InLimbo && !(pTech->IsCrashing || pTech->IsSinking))
		{
			if (pTech->AttachedTag && pTech->AttachedTag->ContainsTrigger(pTrigger))
				pTech->Flash(pThis->Value);
		}
	}

	return true;
}

bool TActionExt::UnInitTechno(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject,
	TriggerClass* pTrigger, CellStruct* plocation)
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

bool TActionExt::GameDeleteTechno(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject,
	TriggerClass* pTrigger, CellStruct* plocation)
{
	if (!pTrigger)
		return false;

	for (auto pTech : *TechnoClass::Array)
	{
		if (pTech && pTech->IsAlive && pTech->IsOnMap && !pTech->InLimbo && !(pTech->IsCrashing || pTech->IsSinking))
		{
			GameDelete<true,false>(pTech);
		}
	}

	return true;
}

bool TActionExt::LightningStormStrikeAtObject(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject,
	TriggerClass* pTrigger, CellStruct* plocation)
{
	if (pThis->Value <= 0 || !pObject)
		return false;

	for (int i = 0; i < pThis->Value; ++i)
	{
		auto ncell = pObject->InlineMapCoords();
		LightningStorm::Strike(ncell);
	}

	return true;
}

CoordStruct* GetSomething(CoordStruct* a1)
{
	const auto& MapRect = Make_Global<RectangleStruct>(0x87F8DC);
	auto v1 = 60 * MapRect.Width;
	auto v2 = 30 * MapRect.Height;
	auto vect_X = ScenarioClass::Instance->Random.RandomFromMax((60 * MapRect.Width) - v1 / 2);
	auto vect_Y = (v2 / 2 + ScenarioClass::Instance->Random.RandomFromMax(v2));
	Vector3D<float> vect { (float)vect_X, (float)vect_Y, 0.0f };
	Vector3D<float> Vec3Dresult { };
	Matrix3D::MatrixMultiply(Vec3Dresult, &TacticalClass::Instance->IsoTransformMatrix, vect);
	a1->Z = 0;
	a1->X = (int)Vec3Dresult.X;
	a1->Y = (int)Vec3Dresult.Y;
	return a1;
}

bool TActionExt::Occured(TActionClass* pThis, ActionArgs const& args, bool& ret)
{
	HouseClass* pHouse = args.pHouse;
	ObjectClass* pObject = args.pObject;
	TriggerClass* pTrigger = args.pTrigger;

	// Vanilla overriden
	switch (pThis->ActionKind)
	{
	case TriggerAction::PlaySoundEffectRandom:
	{
		ret = TActionExt::PlayAudioAtRandomWP(pThis, pHouse, pObject, pTrigger, args.plocation);
		return true;
	}
	};

	//quick way out
	if ((PhobosTriggerAction)pThis->ActionKind < PhobosTriggerAction::count ||
		(AresNewTriggerAction)pThis->ActionKind < AresNewTriggerAction::count)
		return false;

	// Phobos
	switch ((PhobosTriggerAction)pThis->ActionKind)
	{
	case PhobosTriggerAction::SaveGame:
		ret = TActionExt::SaveGame(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::EditVariable:
		ret = TActionExt::EditVariable(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::GenerateRandomNumber:
		ret = TActionExt::GenerateRandomNumber(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::PrintVariableValue:
		ret = TActionExt::PrintVariableValue(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::BinaryOperation:
		ret = TActionExt::BinaryOperation(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
		//case PhobosTriggerAction::AdjustLighting:
		//	ret = TActionExt::AdjustLighting(pThis, pHouse, pObject, pTrigger, args.plocation);
		//	break;
	case PhobosTriggerAction::RunSuperWeaponAtLocation:
		ret = TActionExt::RunSuperWeaponAtLocation(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::RunSuperWeaponAtWaypoint:
		ret = TActionExt::RunSuperWeaponAtWaypoint(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::SetTriggerTechnoVeterancy:
		ret = SetTriggerTechnoVeterancy(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::TransactMoneyFor:
		ret = TransactMoneyFor(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::SetAIMode:
		ret = SetAIMode(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::DrawAnimWithin:
		ret = DrawAnimWithin(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::SetAllOwnedFootDestinationTo:
		ret = SetAllOwnedFootDestinationTo(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::FlashTechnoFor:
		ret = FlashTechnoFor(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::UnInitTechno:
		ret = UnInitTechno(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::GameDeleteTechno:
		ret = GameDeleteTechno(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::LightningStormStrikeAtObject:
		ret = LightningStormStrikeAtObject(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::RandomTriggerPut:
		ret = TActionExt::RandomTriggerPut(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::RandomTriggerEnable:
		ret = TActionExt::RandomTriggerEnable(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::RandomTriggerRemove:
		ret = TActionExt::RandomTriggerRemove(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::ScoreCampaignText:
		ret = TActionExt::ScoreCampaignText(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::ScoreCampaignTheme:
		ret = TActionExt::ScoreCampaignTheme(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::SetNextMission:
		ret = TActionExt::SetNextMission(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;;
	case PhobosTriggerAction::ToggleMCVRedeploy:
		ret = TActionExt::ToggleMCVRedeploy(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::MessageForSpecifiedHouse:
		ret = TActionExt::MessageForSpecifiedHouse(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	default:
		return false;
	}

	return true;
}

//========================================================================================

bool TActionExt::DrawLaserBetweenWaypoints(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	//auto const pExt = TActionExt::ExtMap.Find(pThis);
	//const int duration = pThis->Value2;

	//const ColorStruct innerColor = Drawing::RGB888_HEX((char)pThis->Param5);
	//const ColorStruct outerColor = Drawing::RGB888_HEX(pThis->Param6);

	//auto const& pScen = ScenarioClass::Instance;
	//const CellStruct srcCell = pScen->GetWaypointCoords(pThis->Param3);
	//const CellStruct destCell = pScen->GetWaypointCoords(pThis->Param4);
	//const CoordStruct src = CellClass::Cell2Coord(srcCell, 100);
	//const CoordStruct dest = CellClass::Cell2Coord(destCell, 100);

	//if (LaserDrawClass* pLaser = GameCreate<LaserDrawClass>(src, dest, innerColor, outerColor, outerColor, duration))
	//{
	//	pLaser->IsHouseColor = true;
	//	pLaser->Thickness = 7;
	//}

	return true;
}

// #1004906: support more than 100 waypoints
bool TActionExt::PlayAudioAtRandomWP(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	std::vector<CellStruct> waypoints {};
	waypoints.reserve(ScenarioExt::Global()->Waypoints.size());

	auto const pScen = ScenarioClass::Instance();

	for (auto const& [idx, cell] : ScenarioExt::Global()->Waypoints)
	{
		if (pScen->IsDefinedWaypoint(idx))
			waypoints.emplace_back(cell);
	}

	if (!waypoints.empty())
	{
		VocClass::PlayIndexAtPos(pThis->Value,
		CellClass::Cell2Coord(waypoints[pScen->Random.RandomFromMax(waypoints.size() - 1)]));
	}

	return true;
}

#include <LoadOptionsClass.h>

bool TActionExt::SaveGame(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	if (SessionClass::Instance->GameMode == GameMode::Campaign || SessionClass::Instance->GameMode == GameMode::Skirmish)
	{
		auto nMessage = StringTable::LoadString(GameStrings::TXT_SAVING_GAME());
		auto pUI = UI::sub_623230((LPARAM)nMessage, 0, 0);
		WWMouseClass::Instance->HideCursor();

		if (pUI)
		{
			UI::FocusOnWindow(pUI);
		}

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

		wchar_t fDescription[0x80] = { 0 };
		wcscpy_s(fDescription, ScenarioClass::Instance->UINameLoaded);
		wcscat_s(fDescription, L" - ");
		wcscat_s(fDescription, StringTable::LoadString(pThis->Text));

		bool Status = ScenarioClass::Instance->SaveGame(fName, fDescription);

		WWMouseClass::Instance->ShowCursor();

		if (pUI)
		{
			UI::EndDialog(pUI);
		}

		if (Status)
			PrintMessage(StringTable::LoadString(GameStrings::TXT_GAME_WAS_SAVED));
		else
			PrintMessage(StringTable::LoadString(GameStrings::TXT_ERROR_SAVING_GAME));
	}

	return true;
}

bool TActionExt::EditVariable(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
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
	const auto variables = ScenarioExt::GetVariables(pThis->Param5 != 0);
	auto const& itr = variables->find(pThis->Value);

	if (itr != variables->end())
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

bool TActionExt::GenerateRandomNumber(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	const auto& variables = ScenarioExt::GetVariables(pThis->Param5 != 0);
	const auto& itr = variables->find(pThis->Value);

	if (itr != variables->end())
	{
		itr->second.Value = ScenarioClass::Instance->Random.RandomRanged(pThis->Param3, pThis->Param4);
		if (!pThis->Param5)
			TagClass::NotifyLocalChanged(pThis->Value);
		else
			TagClass::NotifyGlobalChanged(pThis->Value);
	}

	return true;
}

bool TActionExt::PrintVariableValue(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	const auto& variables = ScenarioExt::GetVariables(pThis->Param3 != 0);
	const auto& itr = variables->find(pThis->Value);

	if (itr != variables->end())
	{
		swprintf_s(Phobos::wideBuffer, L"%d", itr->second.Value);
		MessageListClass::Instance->PrintMessage(Phobos::wideBuffer);
	}

	return true;
}

bool TActionExt::BinaryOperation(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	const auto& variables1 = ScenarioExt::GetVariables(pThis->Param5 != 0);
	auto const& itr1 = variables1->find(pThis->Value);
	const auto& variables2 = ScenarioExt::GetVariables(pThis->Param6 != 0);
	auto const& itr2 = variables2->find(pThis->Param4);

	if (itr1 != variables1->end() && itr2 != variables2->end())
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

bool TActionExt::RunSuperWeaponAtLocation(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	return TActionExt::RunSuperWeaponAt(pThis, pThis->Param5, pThis->Param6);
}

bool TActionExt::RunSuperWeaponAtWaypoint(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	const auto& waypoints = ScenarioExt::Global()->Waypoints;
	int nWaypoint = pThis->Param5;

	// Check if is a valid Waypoint
	if (nWaypoint >= 0 && waypoints.find(nWaypoint) != waypoints.end())
	{
		auto const& selectedWP = waypoints.at(nWaypoint);
		if (selectedWP.X && selectedWP.Y)
			return TActionExt::RunSuperWeaponAt(pThis, selectedWP.X, selectedWP.Y);
	}

	return true;
}

NOINLINE HouseClass* GetPlayerAt(int param, HouseClass* const pOwnerHouse = nullptr)
{
	if (param == 8997)
	{
		return pOwnerHouse;
	}

	if (param < 0)
	{
		std::vector<HouseClass*> housesListIdx {};

		switch (param)
		{
		case -1:
		{
			// Random non-neutral
			for (auto pHouse : *HouseClass::Array)
			{
				if (!pHouse->Defeated
					&& !HouseExt::IsObserverPlayer(pHouse)
					&& !pHouse->Type->MultiplayPassive)
				{
					housesListIdx.emplace_back(pHouse);
				}
			}

			return housesListIdx.empty() ?
				nullptr : housesListIdx[ScenarioClass::Instance->Random.RandomFromMax(housesListIdx.size() - 1)];
		}
		case -2:
		{
			// Find first Neutral
			for (auto pHouseNeutral : *HouseClass::Array)
			{
				if (pHouseNeutral->IsNeutral())
				{
					return pHouseNeutral;
				}
			}

			return nullptr;
		}
		case -3:
		{
			// Random Human Player
			for (auto pHouse : *HouseClass::Array)
			{
				if (pHouse->IsControlledByHuman()
					&& !pHouse->Defeated
					&& !HouseExt::IsObserverPlayer(pHouse))
				{
					housesListIdx.emplace_back(pHouse);
				}
			}

			return housesListIdx.empty() ?
				nullptr :
				housesListIdx[(ScenarioClass::Instance->Random.RandomFromMax(housesListIdx.size() - 1))]
				;
		}
		default:
			return nullptr;
		}
	}

	if (HouseClass::Index_IsMP(param))
	{
		return HouseClass::FindByIndex(param);
	}

	return HouseClass::FindByCountryIndex(param);
}

bool TActionExt::RunSuperWeaponAt(TActionClass* pThis, int X, int Y)
{
	if (SuperWeaponTypeClass::Array->Count > 0)
	{
		auto const House = GetPlayerAt(pThis->Param4);

		if (!House || !House->Supers.Count)
			return true;

		int swIdx = pThis->Param3;

		CellStruct targetLocation = { (short)X, (short)Y };
		int retry = 0;

		do
		{
			if (X < 0)
				targetLocation.X += ScenarioClass::Instance->Random.RandomRangedSpecific<short>(0, (short)MapClass::Instance->MapCoordBounds.Right);

			if (Y < 0)
				targetLocation.Y += ScenarioClass::Instance->Random.RandomRangedSpecific<short>(0, (short)MapClass::Instance->MapCoordBounds.Bottom);

			if (++retry >= 10)
			{
				Debug::Log("Failed to `RunSuperWeaponAt` after 10 retries bailout!\n");
				return true;
			}
		}
		while (!MapClass::Instance->IsWithinUsableArea(targetLocation, false));

		if (SuperClass* pSuper = House->Supers.GetItemOrDefault(swIdx))
		{
			if (auto const pSWExt = SWTypeExt::ExtMap.Find(pSuper->Type))
			{
				const int oldstart = pSuper->RechargeTimer.StartTime;
				const int oldleft = pSuper->RechargeTimer.TimeLeft;
				pSuper->SetReadiness(true);
				pSuper->Launch(targetLocation, House->IsCurrentPlayer());
				pSuper->Reset();
				pSuper->RechargeTimer.StartTime = oldstart;
				pSuper->RechargeTimer.TimeLeft = oldleft;
			}
		}
	}

	return true;
}

void TActionExt::RecreateLightSources()
{
	// Yeah, we just simply recreating these lightsource...
	// Stupid but works fine.

	BuildingClass::Array->for_each([](BuildingClass* const pBld)
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

	RadSiteClass::Array->for_each([](RadSiteClass* const pRadSite)
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

	TerrainClass::Array->for_each([](auto const& nPair)
 {
	 if (nPair->IsAlive && !nPair->InLimbo)
	 {
		 const  auto pExt = TerrainExt::ExtMap.Find(nPair);
		 pExt->ClearLightSource();
		 pExt->InitializeLightSource();
	 }
	});

}

bool TActionExt::AdjustLighting(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
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
	if (ScenarioExt::Global()->AdjustLightingFix)
		TActionExt::RecreateLightSources();

	return true;
}

bool TActionExt::RandomTriggerPut(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	TriggerTypeClass* pTargetType = pThis->TriggerType;

	if (!pTargetType)
		return true;

	TriggerClass* pTarget = TriggerClass::GetInstance(pTargetType);

	if (!pTarget)
		return true;

	const int iPoolID = pThis->Param3;
	auto& nPool = TActionExt::RandomTriggerPool[iPoolID];

	if (!nPool.empty())
	{

		auto const iter = std::find_if(nPool.begin(), nPool.end(),
			[&](auto const pTrigger) { return pTrigger == pTarget; });

		if (iter == nPool.end())
			nPool.push_back(pTarget);

	}
	else
	{
		nPool.push_back(pTarget);
	}

	return true;
}

bool TActionExt::RandomTriggerEnable(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	const int iPoolID = pThis->Param3;
	const bool bTakeOff = pThis->Param4;

	auto& nPools = TActionExt::RandomTriggerPool;

	if (!nPools.contains(iPoolID) || !nPools.count(iPoolID))
		return true;

	auto& nPool = nPools.at(iPoolID);

	if (nPool.empty())
		return true;

	const int idx = ScenarioClass::Instance->Random.RandomFromMax(static_cast<int>(nPool.size()) - 1);

	TriggerClass* pTarget = nPool[idx];
	pTarget->Enable();

	if (bTakeOff)
	{
		nPool.erase(nPool.begin() + idx);

		if (nPool.empty())
			nPools.erase(iPoolID);
	}

	return true;
}

bool TActionExt::RandomTriggerRemove(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	const int iPoolID = pThis->Param3;
	TriggerTypeClass* pTriggerType = pThis->TriggerType;
	const TriggerClass* pTarget = TriggerClass::GetInstance(pTriggerType);

	auto& nPools = TActionExt::RandomTriggerPool;

	if (!nPools.contains(iPoolID) || !nPools.count(iPoolID))
		return true;

	auto& nPool = nPools.at(iPoolID);
	auto const iter = std::find_if(nPool.begin(), nPool.end(),
		[&](auto const pTrigger) { return pTrigger == pTarget; });

	if (iter != nPool.end())
		nPool.erase(iter);

	return true;
}

bool TActionExt::ScoreCampaignText(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	if (pThis->Param3 == 0)
		ScenarioExt::Global()->ParMessage = pThis->Text;
	else
		ScenarioExt::Global()->ParTitle = pThis->Text;

	return true;
}

bool TActionExt::ScoreCampaignTheme(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	ScenarioExt::Global()->ScoreCampaignTheme = pThis->Text;

	return true;
}

bool TActionExt::SetNextMission(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	ScenarioExt::Global()->NextMission = pThis->Text;

	return true;
}

bool TActionExt::ToggleMCVRedeploy(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	GameModeOptionsClass::Instance->MCVRedeploy = pThis->Param3 != 0;
	return true;
}

// =============================
// container hooks
//

//DEFINE_HOOK(0x6DD176, TActionClass_CTOR, 0x5)
//{
//	GET(TActionClass*, pItem, ESI);
//	TActionExt::ExtMap.Allocate(pItem);
//	return 0;
//}
//
//DEFINE_HOOK_AGAIN(0x6DD1E6, TActionClass_SDDTOR, 0x7)
//DEFINE_HOOK(0x6E4696, TActionClass_SDDTOR, 0x7)
//{
//	GET(TActionClass*, pItem, ESI);
//	TActionExt::ExtMap.Remove(pItem);
//	return 0;
//}
//
//DEFINE_HOOK(0x6E3E29, TActionClass_Load_Suffix, 0x4)
//{
//	TActionExt::ExtMap.LoadStatic();
//	return 0x0;
//}
//
//DEFINE_HOOK(0x6E3E4A, TActionClass_Save_Suffix, 0x3)
//{
//	TActionExt::ExtMap.SaveStatic();
//	return 0x0;
//}

//DEFINE_HOOK_AGAIN(0x6E3E30, TActionClass_SaveLoad_Prefix, 0x8)
//DEFINE_HOOK(0x6E3DB0, TActionClass_SaveLoad_Prefix, 0x5)
//{
//	GET_STACK(TActionClass*, pItem, 0x4);
//	GET_STACK(IStream*, pStm, 0x8);
//
//	TActionExt::ExtMap.PrepareStream(pItem, pStm);
//
//	return 0;
//}
//
//DEFINE_HOOK(0x6E3E19, TActionClass_Load_Suffix, 0x9)
//{
//	GET(TActionClass*, pItem, ESI);
//
//	SwizzleManagerClass::Instance->Swizzle((void**)&pItem->TriggerType);
//	TActionExt::ExtMap.LoadStatic();
//
//	return 0x6E3E27;
//}
//
//DEFINE_HOOK(0x6E3E44, TActionClass_Save_Suffix, 0x6)
//{
//	GET(HRESULT const, nRes, EAX);
//
//	if(SUCCEEDED(nRes)){
//		TActionExt::ExtMap.SaveStatic();
//		return 0x6E3E48;
//	}
//
//	return 0x6E3E4A;
//}

//DEFINE_HOOK(0x6DD2DE, TActionClass_Detach, 0x5)
//{
//	GET(TActionClass*, pThis, ECX);
//	GET(void*, target, EDX);
//	GET_STACK(bool, all, STACK_OFFS(0xC, -0x8));
//
//	if (auto pExt = TActionExt::ExtMap.Find(pThis))
//		pExt->InvalidatePointer(target, all);
//
//	return pThis->TriggerType == target ? 0x6DD2E3 : 0x6DD2E6;
//}
