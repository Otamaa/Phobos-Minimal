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
#include <Ext/Script/Body.h>
#include <Ext/Techno/Body.h>

#include <New/Entity/BannerClass.h>
#include <New/Type/BannerTypeClass.h>

#include <TriggerTypeClass.h>

//Static init
#include <TagClass.h>
#include <numeric>

std::map<int, std::vector<TriggerClass*>> TActionExtData::RandomTriggerPool;

/*
template <typename T>
void TActionExtData::ExtData::Serialize(T& Stm)
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
TActionExtData::ExtContainer TActionExtData::ExtMap;
*/

//==============================
static void CreateOrReplaceBanner(TActionClass* pTAction, bool isGlobal)
{
	const auto pBannerType = BannerTypeClass::Find(pTAction->Text);

	if (!pBannerType)
		return;

	auto& banners = BannerClass::Array;

	bool foundAny = false;

	banners.for_each([&](BannerClass& pBanner) {
		if (pBanner.ID == pTAction->Param3) {
			foundAny = true;
			pBanner.Type = pBannerType;
			pBanner.Position = { static_cast<int>(pTAction->Param4 / 100.0 * DSurface::ViewBounds->Width), static_cast<int>(pTAction->Param5 / 100.0 * DSurface::ViewBounds->Height) };
			pBanner.Variable = pTAction->Param6;
			pBanner.IsGlobalVariable = isGlobal;
			return true;
		}

		return false;
	});

	if(!foundAny) {
		banners.emplace_back(pBannerType, pTAction->Param3, Point2D { pTAction->Param4, pTAction->Param5 }, pTAction->Param6, isGlobal);
	}
}

bool TActionExtData::CreateBannerGlobal(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	CreateOrReplaceBanner(pThis, true);
	return true;
}

bool TActionExtData::CreateBannerLocal(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	CreateOrReplaceBanner(pThis, false);
	return true;
}

bool TActionExtData::DeleteBanner(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	BannerClass::Array.remove_all_if([pThis](const BannerClass& pBanner) {
		 return pBanner.ID == pThis->Value;
	});

	return true;
}

bool TActionExtData::ResetHateValue(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	if (pThis->Value >= 0) {
		HouseClass* pTargetHouse = HouseClass::Index_IsMP(pThis->Value) ?
			HouseClass::FindByIndex(pThis->Value) :
			HouseClass::FindByCountryIndex(pThis->Value);

		if (pTargetHouse && pTargetHouse->AngerNodes.Count > 0) {
			for (auto& pAngerNode : pTargetHouse->AngerNodes)
				pAngerNode.AngerLevel = 0;
		}
	} else {
		for (auto pTargetHouse : *HouseClass::Array()) {
			for (auto& pAngerNode : pTargetHouse->AngerNodes)
				pAngerNode.AngerLevel = 0;
		}
	}

	return true;
}

bool TActionExtData::UndeployToWaypoint(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	AbstractClass* pCell = MapClass::Instance->TryGetCellAt(ScenarioExtData::Instance()->Waypoints[pThis->Param5]);

	if (!pCell)
	{
		return true;
	}

	bool allHouse = false;
	HouseClass* vHouse = nullptr;

	if (pThis->Param4 >= 0)
	{
		if (HouseClass::Index_IsMP(pThis->Param4))
			vHouse = HouseClass::FindByIndex(pThis->Param4);
		else
			vHouse = HouseClass::FindByCountryIndex(pThis->Param4);
	}
	else if (pThis->Param4 == -1)
	{
		vHouse = pHouse;
	}
	else if (pThis->Param4 == -2)
	{
		allHouse = true;
	}

	if (!allHouse && !vHouse)
	{
		return true;
	}

	bool allBuilding = false;
	BuildingTypeClass* pBldType = nullptr;

	if (strcmp(pThis->Text, GameStrings::AllStr) == 0)
	{
		allBuilding = true;
	}
	else
	{
		pBldType = BuildingTypeClass::Find(pThis->Text);
		if (!pBldType)
		{
			pBldType = BuildingTypeClass::Array()->Items[atoi(pThis->Text)];
		}
	}

	if (!allBuilding && !pBldType)
	{
		return true;
	}

	// Thanks to chaserli for the relevant code!
	// There should be a more perfect way to do this, but I don't know how.
	auto canUndeploy = [pThis, pTrigger, allBuilding, allHouse, pBldType, vHouse](BuildingClass* pBld)
		{
			if (!allHouse && pBld->Owner != vHouse)
			{
				return false;
			}

			const auto pType = pBld->Type;
			if (!pType)
			{
				return false;
			}

			if (!allBuilding && pType != pBldType)
			{
				return false;
			}

			if (!pType->UndeploysInto)
			{
				return false;
			}

			if (pType->ConstructionYard)
			{
				if (!GameModeOptionsClass::Instance->MCVRedeploy || pBld->MindControlledBy || pBld->Owner->IsControlledByHuman())
					return false;
			}

			if (pThis->Param3 &&
				(!pBld->AttachedTag ||
					!pBld->AttachedTag->ContainsTrigger(pTrigger)))
			{
				return false;
			}

			return true;
		};

	for (const auto pBld : *BuildingClass::Array)
	{
		if (!canUndeploy(pBld))
			continue;

		// Why does having this allow it to undeploy?
		// Why don't vehicles move when waypoints are placed off the map?
		pBld->SetArchiveTarget(pCell);
		pBld->Sell(true);
	}

	return true;
}

#include <ExtraHeaders/StackVector.h>

bool TActionExtData::MessageForSpecifiedHouse(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	int houseIdx = 0;
	if (pThis->Param3 == -3)
	{
		// Random Human Player
		StackVector<int , 10> housesListIdx {};
		for (auto ptmpHouse : *HouseClass::Array)
		{
			if (ptmpHouse->IsControlledByHuman()
				&& !ptmpHouse->Defeated
				&& !ptmpHouse->IsObserver())
			{
				housesListIdx->push_back(ptmpHouse->ArrayIndex);
			}
		}

		if (!housesListIdx->empty())
			houseIdx = housesListIdx[(ScenarioClass::Instance->Random.RandomFromMax(housesListIdx->size() - 1))];
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
		auto pTmpHouse = HouseClass::Array->Items[i];
		if (pTmpHouse->IsControlledByHuman() && pTmpHouse == pTargetHouse)
		{
			MessageListClass::Instance->PrintMessage(StringTable::FetchString(pThis->Text), RulesClass::Instance->MessageDelay, pTmpHouse->ColorSchemeIndex);
		}
	}
	return true;
}

bool TActionExtData::SetTriggerTechnoVeterancy(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject,
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

bool TActionExtData::TransactMoneyFor(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject,
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
		pOwner->GiveMoney(Math::abs(pThis->Value));
	}
	else
	{
		pOwner->TransactMoney(pThis->Value);
	}

	return true;
}

bool TActionExtData::SetAIMode(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject,
	TriggerClass* pTrigger, CellStruct* plocation)
{
	if (!pTrigger)
		return false;

	auto pOwner = pThis->FindHouseByIndex(pTrigger, pThis->Param3);

	if (!pOwner)
		return false;

	switch (Math::abs(pThis->Value))
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

bool TActionExtData::DrawAnimWithin(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject,
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
				Vector3D<float> Vec3Dresult {};
				Vector3D<float> Vec3rot { v29 * 1.0f, nDimension * 1.0f, 0.0f };
				Matrix3D::MatrixMultiply(&Vec3Dresult , &TacticalClass::Instance->IsoTransformMatrix,&Vec3rot);
				GameCreate<AnimClass>(pAnimType, CoordStruct { (int)Vec3Dresult.X , (int)Vec3Dresult.Y , 0 });
				nDimension += nShpWidth_;
			}
			while (nDimension < v33);
		}
		v29 += nHeight;
	}
	while (v29 < nRectByt);


	return true;
}

bool TActionExtData::SetAllOwnedFootDestinationTo(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject,
	TriggerClass* pTrigger, CellStruct* plocation)
{
	if (!pTrigger)
		return false;

	auto pOwner = pThis->FindHouseByIndex(pTrigger, pThis->Param3);

	if (!pOwner)
		return false;

	CellStruct nBufer = ScenarioExtData::Instance()->Waypoints[pThis->Waypoint];

	const auto pCell = MapClass::Instance->TryGetCellAt(nBufer);

	for (auto pFoot : *FootClass::Array) {
		if (pFoot->Owner == pOwner) {
			pFoot->SetDestination(pCell, false);
		}
	}

	return true;
}

bool TActionExtData::FlashTechnoFor(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject,
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

bool TActionExtData::UnInitTechno(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject,
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

bool TActionExtData::GameDeleteTechno(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject,
	TriggerClass* pTrigger, CellStruct* plocation)
{
	if (!pTrigger)
		return false;

	for (auto pTech : *TechnoClass::Array)
	{
		if (pTech && pTech->IsAlive && pTech->IsOnMap && !pTech->InLimbo && !(pTech->IsCrashing || pTech->IsSinking))
		{
			GameDelete<true, false>(pTech);
		}
	}

	return true;
}

bool TActionExtData::LightningStormStrikeAtObject(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject,
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

static CoordStruct* GetSomething(CoordStruct* a1)
{
	const auto& MapRect = Make_Global<RectangleStruct>(0x87F8DC);
	auto v1 = 60 * MapRect.Width;
	auto v2 = 30 * MapRect.Height;
	auto vect_X = ScenarioClass::Instance->Random.RandomFromMax((60 * MapRect.Width) - v1 / 2);
	auto vect_Y = (v2 / 2 + ScenarioClass::Instance->Random.RandomFromMax(v2));
	Vector3D<float> Vec3Dresult {};
	Vector3D<float> Vec3Drot { (float)vect_X, (float)vect_Y, 0.0f };
	Matrix3D::MatrixMultiply(&Vec3Dresult, &TacticalClass::Instance->IsoTransformMatrix, &Vec3Drot);
	a1->Z = 0;
	a1->X = (int)Vec3Dresult.X;
	a1->Y = (int)Vec3Dresult.Y;
	return a1;
}

bool NOINLINE TActionExtData::Occured(TActionClass* pThis, ActionArgs const& args, bool& ret)
{
	HouseClass* pHouse = args.pHouse;
	ObjectClass* pObject = args.pObject;
	TriggerClass* pTrigger = args.pTrigger;

	// Phobos
	switch ((PhobosTriggerAction)pThis->ActionKind)
	{
	case PhobosTriggerAction::SaveGame:
		ret = TActionExtData::SaveGame(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::EditVariable:
		ret = TActionExtData::EditVariable(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::GenerateRandomNumber:
		ret = TActionExtData::GenerateRandomNumber(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::PrintVariableValue:
		ret = TActionExtData::PrintVariableValue(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::BinaryOperation:
		ret = TActionExtData::BinaryOperation(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
		//case PhobosTriggerAction::AdjustLighting:
		//	ret = TActionExtData::AdjustLighting(pThis, pHouse, pObject, pTrigger, args.plocation);
		//	break;
	case PhobosTriggerAction::RunSuperWeaponAtLocation:
		ret = TActionExtData::RunSuperWeaponAtLocation(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::RunSuperWeaponAtWaypoint:
		ret = TActionExtData::RunSuperWeaponAtWaypoint(pThis, pHouse, pObject, pTrigger, args.plocation);
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
		//case PhobosTriggerAction::RandomTriggerPut:
		//	ret = TActionExtData::RandomTriggerPut(pThis, pHouse, pObject, pTrigger, args.plocation);
		//	break;
		//case PhobosTriggerAction::RandomTriggerEnable:
		//	ret = TActionExtData::RandomTriggerEnable(pThis, pHouse, pObject, pTrigger, args.plocation);
		//	break;
		//case PhobosTriggerAction::RandomTriggerRemove:
		//	ret = TActionExtData::RandomTriggerRemove(pThis, pHouse, pObject, pTrigger, args.plocation);
		//	break;
	case PhobosTriggerAction::ScoreCampaignText:
		ret = TActionExtData::ScoreCampaignText(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::ScoreCampaignTheme:
		ret = TActionExtData::ScoreCampaignTheme(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::SetNextMission:
		ret = TActionExtData::SetNextMission(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::DumpVariables:
		return TActionExtData::DumpVariables(pThis, pHouse, pObject, pTrigger, args.plocation);
	case PhobosTriggerAction::ToggleMCVRedeploy:
		ret = TActionExtData::ToggleMCVRedeploy(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::MessageForSpecifiedHouse:
		ret = TActionExtData::MessageForSpecifiedHouse(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::UndeployToWaypoint:
		ret = TActionExtData::UndeployToWaypoint(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::PrintMessageRemainingTechnos:
		ret = TActionExtData::PrintMessageRemainingTechnos(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::SetDropCrate:
		ret = TActionExtData::SetDropCrate(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::EditAngerNode:
		ret = TActionExtData::EditAngerNode(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::ClearAngerNode:
		ret = TActionExtData::ClearAngerNode(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::SetForceEnemy:
		ret = TActionExtData::SetForceEnemy(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;

	case PhobosTriggerAction::CreateBannerGlobal:
		ret = TActionExtData::CreateBannerGlobal(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::CreateBannerLocal:
		ret = TActionExtData::CreateBannerLocal(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::DeleteBanner:
		ret = TActionExtData::DeleteBanner(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;

	default:
	{
		return false;
	}
	}

	return true;
}

bool TActionExtData::EditAngerNode(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	auto setValue = [pThis, pHouse](HouseClass* pTargetHouse)
		{
			if (!pTargetHouse || pHouse == pTargetHouse ||
				pHouse->IsAlliedWith(pTargetHouse))
				return;

			for (auto& pAngerNode : pHouse->AngerNodes)
			{
				if (pAngerNode.House != pTargetHouse)
					continue;

				switch (pThis->Param3)
				{
				case 0: { pAngerNode.AngerLevel = pThis->Param4; break; }
				case 1: { pAngerNode.AngerLevel += pThis->Param4; break; }
				case 2: { pAngerNode.AngerLevel -= pThis->Param4; break; }
				case 3: { pAngerNode.AngerLevel *= pThis->Param4; break; }
				case 4: { pAngerNode.AngerLevel /= pThis->Param4; break; }
				case 5: { pAngerNode.AngerLevel %= pThis->Param4; break; }
				case 6: { pAngerNode.AngerLevel <<= pThis->Param4; break; }
				case 7: { pAngerNode.AngerLevel >>= pThis->Param4; break; }
				case 8: { pAngerNode.AngerLevel = ~pAngerNode.AngerLevel; break; }
				case 9: { pAngerNode.AngerLevel ^= pThis->Param4; break; }
				case 10: { pAngerNode.AngerLevel |= pThis->Param4; break; }
				case 11: { pAngerNode.AngerLevel &= pThis->Param4; break; }
				default:break;
				}

				break;
			}
		};

	if (pHouse->AngerNodes.Count > 0)
	{
		if (pThis->Value >= 0)
		{
			HouseClass* pTargetHouse = HouseClass::Index_IsMP(pThis->Value) ?
				HouseClass::FindByIndex(pThis->Value) :
				HouseClass::FindByCountryIndex(pThis->Value);

			setValue(pTargetHouse);
		}
		else
		{
			for (auto pTargetHouse : *HouseClass::Array)
			{
				setValue(pTargetHouse);
			}
		}
	}

	return true;
}

bool TActionExtData::ClearAngerNode(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	if (pThis->Value >= 0)
	{
		HouseClass* pTargetHouse = HouseClass::Index_IsMP(pThis->Value) ?
			HouseClass::FindByIndex(pThis->Value) :
			HouseClass::FindByCountryIndex(pThis->Value);

		if (pTargetHouse && pTargetHouse->AngerNodes.Count > 0)
		{
			for (auto& pAngerNode : pTargetHouse->AngerNodes)
				pAngerNode.AngerLevel = 0;
		}
	}
	else
	{
		for (auto pTargetHouse :*HouseClass::Array)
		{
			if (pTargetHouse->AngerNodes.Count <= 0)
				continue;

			for (auto& pAngerNode : pTargetHouse->AngerNodes)
				pAngerNode.AngerLevel = 0;
		}
	}

	return true;
}

bool TActionExtData::SetForceEnemy(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	auto const pHouseExt = HouseExtContainer::Instance.Find(pHouse);

	if (pThis->Param3 >= 0 || pThis->Param3 == -2)
	{
		if (pThis->Param3 != -2)
		{
			HouseClass* pTargetHouse = HouseClass::Index_IsMP(pThis->Param3) ?
				HouseClass::FindByIndex(pThis->Param3) :
				HouseClass::FindByCountryIndex(pThis->Param3);

			if (pTargetHouse && !pHouse->IsAlliedWith(pTargetHouse))
			{
				pHouseExt->SetForceEnemy(pTargetHouse->GetArrayIndex());
				pHouse->EnemyHouseIndex = pTargetHouse->GetArrayIndex();
			}
		}
		else
		{
			pHouseExt->SetForceEnemy(-2);
			pHouse->EnemyHouseIndex = -1;
		}
	}
	else
	{
		pHouseExt->SetForceEnemy(-1);
		pHouse->UpdateAngerNodes(0, nullptr);
	}

	return true;
}

//========================================================================================

bool TActionExtData::SetDropCrate(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	for (auto pTechno : *TechnoClass::Array)
	{
		const auto pAttachedTag = pTechno->AttachedTag;

		if (!pAttachedTag)
			continue;

		bool foundTrigger = false;
		auto pAttachedTrigger = pAttachedTag->FirstTrigger;

		// A tag can link multiple triggers
		do
		{
			if (IS_SAME_STR_(pAttachedTrigger->Type->ID, pTrigger->Type->ID) == 0)
				foundTrigger = true;

			pAttachedTrigger = pAttachedTrigger->NextTrigger;
		}
		while (pAttachedTrigger && !foundTrigger);

		if (!foundTrigger)
			continue;

		// Overwrite the default techno's crate properties
		auto pExt = TechnoExtContainer::Instance.Find(pTechno);
		pExt->DropCrate = pThis->Value;

		if (pExt->DropCrate == 1)
			pExt->DropCrateType = static_cast<PowerupEffects>(pThis->Param3);
	}

	return true;
}

bool TActionExtData::DrawLaserBetweenWaypoints(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	//auto const pExt = TActionExtData::ExtMap.Find(pThis);
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
bool TActionExtData::PlayAudioAtRandomWP(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	ScenarioExtData::Instance()->DefinedAudioWaypoints.reserve(ScenarioExtData::Instance()->Waypoints.size());

	auto const pScen = ScenarioClass::Instance();

	if (!ScenarioExtData::Instance()->DefinedAudioWaypoints.empty())
	{
		VocClass::SafeImmedietelyPlayAt(pThis->Value,
		&CellClass::Cell2Coord(ScenarioExtData::Instance()->DefinedAudioWaypoints
			[pScen->Random.RandomFromMax(ScenarioExtData::Instance()->DefinedAudioWaypoints.size() - 1)]));
	}
	else
	{
		for (auto const& [idx, cell] : ScenarioExtData::Instance()->Waypoints)
		{
			if (pScen->IsDefinedWaypoint(idx))
				ScenarioExtData::Instance()->DefinedAudioWaypoints.push_back(cell);
		}
	}


	return true;
}

#include <LoadOptionsClass.h>

bool TActionExtData::SaveGame(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	if (SessionClass::Instance->GameMode == GameMode::Campaign || SessionClass::Instance->GameMode == GameMode::Skirmish)
	{
		auto nMessage = StringTable::FetchString(GameStrings::TXT_SAVING_GAME());
		auto pUI = UI::ShowMessageWithCancelOnly((LPARAM)nMessage, 0, 0);
		WWMouseClass::Instance->HideCursor();

		if (pUI)
		{
			UI::FocusOnWindow(pUI);
		}

		const std::string fName = "Map." + Debug::GetCurTimeA() + ".sav";
		std::wstring fDesc = SessionClass::Instance->GameMode == GameMode::Campaign ? ScenarioClass::Instance->UINameLoaded : ScenarioClass::Instance->Name;
		fDesc += L" - ";
		fDesc += StringTable::FetchString(pThis->Text);

		bool Status = ScenarioClass::Instance->SaveGame(fName.c_str(), fDesc.c_str());

		WWMouseClass::Instance->ShowCursor();

		if (pUI)
		{
			UI::EndDialog(pUI);
		}

		auto pMessage = Status ?
			StringTable::FetchString(GameStrings::TXT_GAME_WAS_SAVED) :
			StringTable::FetchString(GameStrings::TXT_ERROR_SAVING_GAME);

		GeneralUtils::PrintMessage(pMessage);
	}

	return true;
}

bool TActionExtData::EditVariable(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
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
	const auto variables = ScenarioExtData::GetVariables(pThis->Param5 != 0);

	if (auto itr = variables->tryfind(pThis->Value))
	{
		auto& nCurrentValue = itr->Value;
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

bool TActionExtData::GenerateRandomNumber(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	const auto& variables = ScenarioExtData::GetVariables(pThis->Param5 != 0);

	if (auto itr = variables->tryfind(pThis->Value))
	{
		itr->Value = ScenarioClass::Instance->Random.RandomRanged(pThis->Param3, pThis->Param4);
		if (!pThis->Param5)
			TagClass::NotifyLocalChanged(pThis->Value);
		else
			TagClass::NotifyGlobalChanged(pThis->Value);
	}

	return true;
}

bool TActionExtData::PrintVariableValue(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	const auto& variables = ScenarioExtData::GetVariables(pThis->Param3 != 0);

	if (auto itr = variables->tryfind(pThis->Value))
	{
		swprintf_s(Phobos::wideBuffer, L"%d", itr->Value);
		MessageListClass::Instance->PrintMessage(Phobos::wideBuffer);
	}

	return true;
}

bool TActionExtData::BinaryOperation(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	const auto variables1 = ScenarioExtData::GetVariables(pThis->Param5 != 0);
	auto itr1 = variables1->tryfind(pThis->Value);
	const auto variables2 = ScenarioExtData::GetVariables(pThis->Param6 != 0);
	auto itr2 = variables2->tryfind(pThis->Param4);

	if (itr1 && itr2)
	{
		auto& nCurrentValue = itr1->Value;
		auto& nOptValue = itr2->Value;
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

bool TActionExtData::RunSuperWeaponAtLocation(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	return TActionExtData::RunSuperWeaponAt(pThis, pThis->Param5, pThis->Param6);
}

bool TActionExtData::RunSuperWeaponAtWaypoint(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	const auto& waypoints = ScenarioExtData::Instance()->Waypoints;

	// Check if is a valid Waypoint
	if (auto iter = waypoints.tryfind(pThis->Param5))
	{
		if (iter->X && iter->Y)
			return TActionExtData::RunSuperWeaponAt(pThis, iter->X, iter->Y);
	}

	return true;
}

static NOINLINE HouseClass* GetPlayerAt(int param, HouseClass* const pOwnerHouse = nullptr)
{
	if (param == 8997)
	{
		return pOwnerHouse;
	}

	if (param < 0)
	{
		StackVector<HouseClass* , 10> housesListIdx {};

		switch (param)
		{
		case -1:
		{
			// Random non-neutral
			for (auto pHouse : *HouseClass::Array)
			{
				if (!pHouse->Defeated
					&& !HouseExtData::IsObserverPlayer(pHouse)
					&& !pHouse->Type->MultiplayPassive)
				{
					housesListIdx->push_back(pHouse);
				}
			}

			return housesListIdx->empty() ?
				nullptr : housesListIdx[ScenarioClass::Instance->Random.RandomFromMax(housesListIdx->size() - 1)];
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
					&& !HouseExtData::IsObserverPlayer(pHouse))
				{
					housesListIdx->push_back(pHouse);
				}
			}

			return housesListIdx->empty() ?
				nullptr :
				housesListIdx[(ScenarioClass::Instance->Random.RandomFromMax(housesListIdx->size() - 1))]
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

bool TActionExtData::RunSuperWeaponAt(TActionClass* pThis, int X, int Y)
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
				Debug::LogInfo("Failed to `RunSuperWeaponAt` after 10 retries bailout!");
				return true;
			}
		}
		while (!MapClass::Instance->IsWithinUsableArea(targetLocation, false));

		if (SuperClass* pSuper = House->Supers.GetItemOrDefault(swIdx))
		{
			if (auto const pSWExt = SWTypeExtContainer::Instance.Find(pSuper->Type))
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

void TActionExtData::RecreateLightSources()
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

		 GameDelete<true, false>(std::exchange(pRadSite->LightSource,
			 GameCreate<LightSourceClass>(coord, visibility, intensity, color)));

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
		 TerrainExtContainer::Instance.Find(nPair)->LighSource.reset(nullptr);
		 TerrainExtContainer::Instance.Find(nPair)->InitializeLightSource();
	 }
	});

}

//bool TActionExtData::AdjustLighting(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
//{
//	if (pThis->Param3 != -1)
//		ScenarioClass::Instance->NormalLighting.Tint.Red = pThis->Param3;
//	if (pThis->Param4 != -1)
//		ScenarioClass::Instance->NormalLighting.Tint.Green = pThis->Param4;
//	if (pThis->Param5 != -1)
//		ScenarioClass::Instance->NormalLighting.Tint.Blue = pThis->Param5;
//
//	const int r = ScenarioClass::Instance->NormalLighting.Tint.Red * 10;
//	const int g = ScenarioClass::Instance->NormalLighting.Tint.Green * 10;
//	const int b = ScenarioClass::Instance->NormalLighting.Tint.Blue * 10;
//
//	if (pThis->Value & 0b001) // Update Tiles
//	{
//		for (auto& pLightConvert : *LightConvertClass::Array)
//			pLightConvert->UpdateColors(r, g, b, false);
//
//		ScenarioExtData::Instance()->CurrentTint_Tiles = ScenarioClass::Instance->NormalLighting.Tint;
//	}
//
//	if (pThis->Value & 0b010) // Update Units & Buildings
//	{
//		for (auto& pScheme : *ColorScheme::Array)
//			pScheme->LightConvert->UpdateColors(r, g, b, false);
//		ScenarioExtData::Instance()->CurrentTint_Schemes = ScenarioClass::Instance->NormalLighting.Tint;
//	}
//
//	if (pThis->Value & 0b100) // Update CustomPalettes (vanilla YR LightConvertClass one, not the Ares ConvertClass only one)
//	{
//		ScenarioClass::UpdateHashPalLighting(r, g, b, false);
//		ScenarioExtData::Instance()->CurrentTint_Hashes = ScenarioClass::Instance->NormalLighting.Tint;
//	}
//
//	ScenarioClass::UpdateCellLighting();
//	MapClass::Instance->RedrawSidebar(1); // GScreenClass::Flag_To_Redraw
//
//	// #issue 429
//	if (ScenarioExtData::Instance()->AdjustLightingFix)
//		TActionExtData::RecreateLightSources();
//
//	return true;
//}

bool TActionExtData::RandomTriggerPut(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	TriggerTypeClass* pTargetType = pThis->TriggerType;

	if (!pTargetType)
		return true;

	TriggerClass* pTarget = TriggerClass::GetInstance(pTargetType);

	if (!pTarget)
		return true;

	const int iPoolID = pThis->Param3;
	auto& nPool = TActionExtData::RandomTriggerPool[iPoolID];

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

bool TActionExtData::RandomTriggerEnable(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	const int iPoolID = pThis->Param3;
	const bool bTakeOff = pThis->Param4;

	auto& nPools = TActionExtData::RandomTriggerPool;

	if (!nPools.contains(iPoolID) || !nPools.count(iPoolID))
		return true;

	auto& nPool = nPools[iPoolID];

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

bool TActionExtData::RandomTriggerRemove(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	const int iPoolID = pThis->Param3;
	TriggerTypeClass* pTriggerType = pThis->TriggerType;
	const TriggerClass* pTarget = TriggerClass::GetInstance(pTriggerType);

	auto& nPools = TActionExtData::RandomTriggerPool;

	if (!nPools.contains(iPoolID) || !nPools.count(iPoolID))
		return true;

	auto& nPool = nPools[iPoolID];
	auto const iter = std::find_if(nPool.begin(), nPool.end(),
		[&](auto const pTrigger) { return pTrigger == pTarget; });

	if (iter != nPool.end())
		nPool.erase(iter);

	return true;
}

bool TActionExtData::ScoreCampaignText(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	if (pThis->Param3 == 0)
		ScenarioExtData::Instance()->ParMessage = pThis->Text;
	else
		ScenarioExtData::Instance()->ParTitle = pThis->Text;

	return true;
}

bool TActionExtData::ScoreCampaignTheme(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	ScenarioExtData::Instance()->ScoreCampaignTheme = pThis->Text;

	return true;
}

bool TActionExtData::SetNextMission(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	ScenarioExtData::Instance()->NextMission = pThis->Text;

	return true;
}

static COMPILETIMEEVAL bool IsUnitAvailable(TechnoClass* pTechno, bool checkIfInTransportOrAbsorbed)
{
	if (!pTechno)
		return false;

	bool isAvailable = pTechno->IsAlive && pTechno->Health > 0 && !pTechno->InLimbo && pTechno->IsOnMap;

	if (checkIfInTransportOrAbsorbed)
		isAvailable &= !pTechno->Absorbed && !pTechno->Transporter;

	return isAvailable;

}

bool TActionExtData::PrintMessageRemainingTechnos(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	if (!pThis)
		return true;
	// Example:
	// ID=ActionCount,[Action1],507,4,[CSFKey],[HouseIndex],[AIHousesLists Index],[AITargetTypes Index],[MesageDelay],A,[ActionX]
	StackVector<HouseClass* , 10> pHousesList {};

	// Obtain houses
	int param3 = pThis->Param3;

	if (pThis->Param3 - HouseClass::PlayerAtA >= 0 && pThis->Param3 - HouseClass::PlayerAtA < 8997)
	{
		// Multiplayer house index (Player@A - Player@H)
		param3 = pThis->Param3 - HouseClass::PlayerAtA;
	}
	else if (pThis->Param3 - 8997 == 0)
	{
		// House specified in Trigger
		param3 = pThis->TeamType ? pThis->TeamType->Owner->ArrayIndex : pHouse->ArrayIndex;
	}
	else if (pThis->Param3 > 8997)
	{
		Debug::LogInfo("Map action {}: Invalid house index '{}'. This action will be skipped.", (int)pThis->ActionKind, pThis->Param3);
		return true;
	}

	if (param3 >= 0)
	{
		pHousesList->push_back(HouseClass::Array->GetItem(param3));
	}
	else
	{
		// Pick a group of countries from [AIHousesList].
		// Any house of the same type of the listed at [AIHousesList] will be included here

		if (RulesExtData::Instance()->AIHousesLists.empty() || (size_t)pThis->Param4 < RulesExtData::Instance()->AIHousesLists.size()) {
			Debug::LogInfo("Map action {}: [AIHousesList] is empty. This action will be skipped.", (int)pThis->ActionKind);
			return true;
		}

		std::vector<HouseTypeClass*>* housesList = &RulesExtData::Instance()->AIHousesLists[pThis->Param4];

		if (housesList->empty()) {
			Debug::LogInfo("Map action {}: List [AIHousesList]({}) is empty. This action will be skipped.", (int)pThis->ActionKind, pThis->Param4);
			return true;
		}

		for (const auto& pHouseType : *housesList) {
			for (auto pCont : *HouseClass::Array) {
				if (pCont->Type == pHouseType && !pCont->Defeated && !pCont->IsObserver())
					pHousesList->push_back(pCont);
			}
		}

		// Nothing to check
		if (pHousesList->empty())
			return true;
	}

	// Read the ID list of technos
	int listIdx = Math::abs(pThis->Param5);

	if ((size_t)listIdx < RulesExtData::Instance()->AIHousesLists.size()
		|| RulesExtData::Instance()->AITargetTypesLists[listIdx].empty()) {
		Debug::LogInfo("Map action {}: List [AITargetTypes]({}) is empty. This action will be skipped.", (int)pThis->ActionKind, listIdx);
		return true;
	}

	std::vector<TechnoTypeClass*>* technosList = &RulesExtData::Instance()->AITargetTypesLists[listIdx];
	std::vector<int> technosRemaining;
	int globalRemaining = 0;

	// Count all valid instances
	for (auto const& pType : *technosList) {
		int nRemaining = 0;

		for (const auto pTechno : *TechnoClass::Array)
		{
			if (!IsUnitAvailable(pTechno, false) || pTechno->GetTechnoType() != pType)
				continue;

			for (const auto& pCont : pHousesList.container()) {
				if (pTechno->Owner == pCont) {
					globalRemaining++;
					nRemaining++;
				}
			}
		}

		technosRemaining.push_back(nRemaining);
	}

	bool textToShow = false;
	float messageDelay = float(pThis->Param6 <= 0 ? RulesClass::Instance->MessageDelay : pThis->Param6 / 60.0); // seconds / 60 = message delay in minutes
	std::wstring _message = GeneralUtils::LoadStringUnlessMissingNoChecks(pThis->Text, L"Remaining: ");

	if (pThis->Param5 < 0) {
		if (globalRemaining > 0) {
			_message += std::to_wstring(globalRemaining);
			textToShow = true;
		}
	}
	else
	{
		_message += L"";

		for (size_t i = 0; i < technosRemaining.size(); i++) {

			if (technosRemaining[i] == 0)
				continue;

			textToShow = true;
			_message += fmt::format(L"{}: {}", (*technosList)[i]->UIName, technosRemaining[i]);
		}
	}

	if (textToShow)
		MessageListClass::Instance->PrintMessage(_message.c_str(), messageDelay, HouseClass::CurrentPlayer->ColorSchemeIndex, true);

	return true;
}

bool TActionExtData::DumpVariables(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	const auto fileName = (pThis->Param3 != 0) ? "globals.ini" : "locals.ini";
	CCFileClass file { fileName };

	if (!file.Exists()) {
		if (!file.CreateFileA()) {
			return false;
		}
	}

	if (!file.Open(FileAccessMode::ReadWrite)) {
		Debug::LogInfo(__FUNCTION__" Failed to Open file {} for", fileName);
		return false;
	}

	CCINIClass ini {};
	ini.ReadCCFile(&file);
	const auto variables = ScenarioExtData::GetVariables(pThis->Param3 != 0);
	std::for_each(variables->begin(), variables->end(), [&](const auto& variable) {
		ini.WriteInteger(ScenarioClass::Instance()->FileName, variable.second.Name, variable.second.Value, false);
	});

	ini.WriteCCFile(&file);
	return true;
}

bool TActionExtData::ToggleMCVRedeploy(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	GameModeOptionsClass::Instance->MCVRedeploy = pThis->Param3 != 0;
	return true;
}

// =============================
// container hooks
//

//ASMJIT_PATCH(0x6DD176, TActionClass_CTOR, 0x5)
//{
//	GET(TActionClass*, pItem, ESI);
//	TActionExtData::ExtMap.Allocate(pItem);
//	return 0;
//}
//
//ASMJIT_PATCH_AGAIN(0x6DD1E6, TActionClass_SDDTOR, 0x7)
//ASMJIT_PATCH(0x6E4696, TActionClass_SDDTOR, 0x7)
//{
//	GET(TActionClass*, pItem, ESI);
//	TActionExtData::ExtMap.Remove(pItem);
//	return 0;
//}
//
//ASMJIT_PATCH(0x6E3E29, TActionClass_Load_Suffix, 0x4)
//{
//	TActionExtData::ExtMap.LoadStatic();
//	return 0x0;
//}
//
//ASMJIT_PATCH(0x6E3E4A, TActionClass_Save_Suffix, 0x3)
//{
//	TActionExtData::ExtMap.SaveStatic();
//	return 0x0;
//}

//ASMJIT_PATCH_AGAIN(0x6E3E30, TActionClass_SaveLoad_Prefix, 0x8)
//ASMJIT_PATCH(0x6E3DB0, TActionClass_SaveLoad_Prefix, 0x5)
//{
//	GET_STACK(TActionClass*, pItem, 0x4);
//	GET_STACK(IStream*, pStm, 0x8);
//
//	TActionExtData::ExtMap.PrepareStream(pItem, pStm);
//
//	return 0;
//}
//
//ASMJIT_PATCH(0x6E3E19, TActionClass_Load_Suffix, 0x9)
//{
//	GET(TActionClass*, pItem, ESI);
//
//	SwizzleManagerClass::Instance->Swizzle((void**)&pItem->TriggerType);
//	TActionExtData::ExtMap.LoadStatic();
//
//	return 0x6E3E27;
//}
//
//ASMJIT_PATCH(0x6E3E44, TActionClass_Save_Suffix, 0x6)
//{
//	GET(HRESULT const, nRes, EAX);
//
//	if(SUCCEEDED(nRes)){
//		TActionExtData::ExtMap.SaveStatic();
//		return 0x6E3E48;
//	}
//
//	return 0x6E3E4A;
//}

//ASMJIT_PATCH(0x6DD2DE, TActionClass_Detach, 0x5)
//{
//	GET(TActionClass*, pThis, ECX);
//	GET(void*, target, EDX);
//	GET_STACK(bool, all, STACK_OFFS(0xC, -0x8));
//
//	if (auto pExt = TActionExtData::ExtMap.Find(pThis))
//		pExt->InvalidatePointer(target, all);
//
//	return pThis->TriggerType == target ? 0x6DD2E3 : 0x6DD2E6;
//}

#include <Utilities/Macro.h>
#include <Misc/Ares/Hooks/Header.h>
#include <Ext/Side/Body.h>

static void __fastcall UnlockImput() {
	JMP_STD(0x684290);
}

static void __fastcall UIStuffs_MenuStuffs(){
	JMP_STD(0x72DDB0);
}

static void __fastcall PlayMovie(int id, int theme, bool clrscreen_aft, bool stretch, bool clrscreen_before) {
	JMP_STD(0x5BF260);
}

static void __fastcall Reset_SomeShapes_Post_Movie() {
	JMP_STD(0x72DEF0);
}

static NOINLINE bool _OverrideOriginalActions(TActionClass* pThis, HouseClass* pTargetHouse, ObjectClass* pSourceObject, TriggerClass* pTrigger, CellStruct* plocation, bool& ret)
{
	switch (pThis->ActionKind)
	{
	case TriggerAction::Win:
	{
		//if (HouseClass::Index_IsMP(pThis->Value))
		//{
		//	const auto pHouse_ = pThis->Value == 8997 ?
		//		HouseClass::CurrentPlayer() : HouseClass::FindByIndex(pThis->Value);

		//	auto pHouseBegin = HouseClass::Array->begin();
		//	auto pHouseEnd = HouseClass::Array->end();

		//	if (HouseClass::Array->begin() != pHouseEnd)
		//	{
		//		do
		//		{
		//			auto v7 = *pHouseBegin;
		//			if (pHouse_->ArrayIndex == (*pHouseBegin)->ArrayIndex
		//				|| pHouse_->ArrayIndex != -1 && ((1 << pHouse_->ArrayIndex) & v7->Allies.data) != 0)
		//				v7->Win(false);

		//			++pHouseBegin;
		//		}
		//		while (pHouseBegin != pHouseEnd);
		//	}
		//}
		//else
		{
			if (pThis->Value == HouseClass::CurrentPlayer()->Type->ParentIdx)
				HouseClass::CurrentPlayer()->Win(false);
			else
				HouseClass::CurrentPlayer()->Lose(false);
		}
		return true;
	}
	case TriggerAction::ProductionBegins: {

		if (auto pTrigOwner =  pThis->FindHouseByIndex(pTrigger,pThis->Value)) {
			pTrigOwner->Production = true;
			ret = true;
		}

		ret = false;
		return true;
	}
	case TriggerAction::CreateTeam:{
		++Unsorted::ScenarioInit;

		if (auto pTeam = pThis->TeamType) {
			pTeam->CreateTeam(nullptr);
		}
		--Unsorted::ScenarioInit;
		return true;
	}
	case TriggerAction::DestroyTeam:{

		if (auto pTeam = pThis->TeamType) {
			pTeam->DestroyAllInstances();
		}

		return true;
	}
	case TriggerAction::AllToHunt:
	{
		if (auto pTrigOwner = pThis->FindHouseByIndex(pTrigger, pThis->Value)) {
			pTrigOwner->All_To_Hunt();
			ret = true;
		}

		ret = false;

		return true;
	}
	case TriggerAction::Reinforcement:
	{
		if (auto pTeam = pThis->TeamType) {
			ret = TeamTypeClass::DoReinforcement(pTeam, -1);
		}

		ret = false;
		return true;
	}
	case TriggerAction::DropZoneFlare:
	{
		auto cell = ScenarioClass::Instance->GetWaypointCoords(pThis->Waypoint);
		auto coord = CellClass::Cell2Coord(cell);
		coord.Z = MapClass::Instance->GetCellFloorHeight(coord);

		auto pCell = MapClass::Instance->GetCellAt(coord);

		if (pCell->ContainsBridge() || pCell->ContainsBridgeBody())
			coord.Z += CellClass::BridgeHeight;

		GameCreate<AnimClass>(RulesClass::Instance->DropZoneAnim, coord)->IsPlaying = true;
		return true;
	}
	case TriggerAction::FireSale:
	{
		if (auto pTrigOwner = pThis->FindHouseByIndex(pTrigger, pThis->Value))
		{
			pTrigOwner->AIMode = AIMode::SellAll;
			ret = true;
		}

		ret = false;

		return true;
	}
	case TriggerAction::PlayMovie:
	{
		UIStuffs_MenuStuffs();
		WWMouseClass::Instance->ReleaseMouse();
		ScenarioClass::ToggleDisplayMode(0);
		PlayMovie(pThis->Value, -1, 1, 1, 1);
		ScenarioClass::ToggleDisplayMode(1);
		WWMouseClass::Instance->CaptureMouse();
		Reset_SomeShapes_Post_Movie();
		return true;
	}
	case TriggerAction::Lose:
	{
		if (pThis->Value != HouseClass::CurrentPlayer()->Type->ParentIdx)
			HouseClass::CurrentPlayer()->Win(false);
		else
			HouseClass::CurrentPlayer()->Lose(false);

		return true;
	}
	case TriggerAction::PlaySoundEffectRandom:
	{
		ret = TActionExtData::PlayAudioAtRandomWP(pThis, pTargetHouse, pSourceObject, pTrigger, plocation);
		return true;
	}
	case TriggerAction::UnlockInput:
	{
		UnlockImput();
		return true;
	}
	case TriggerAction::PlaySpeech:
	{
		VoxClass::PlayIndex(pThis->Value);
		return true;
	}
	case TriggerAction::PlaySoundEffectAtWaypoint:
	{
		const CellStruct waypointCell = ScenarioExtData::Instance()->Waypoints[pThis->Waypoint];
		const auto pCell = MapClass::Instance->GetCellAt(waypointCell);

		if (waypointCell.IsValid() && pCell)
		{
			ObjectClass* pObj = pCell->GetSomeObject(Point2D::Empty, false);

			if (pObj && (pObj->WhatAmI() == BuildingClass::AbsID || pObj->WhatAmI() == TerrainClass::AbsID))
			{
				pObj->AttachSound(pThis->Value);
			}
			else
			{
				VocClass::PlayIndexAtPos(pThis->Value, CellClass::Cell2Coord(waypointCell), true);
			}
		}

		return true;
	}
	case TriggerAction::CreateCrate:
	{
		const CellStruct waypointCell = ScenarioExtData::Instance()->Waypoints[pThis->Waypoint];
		const auto placed = MapClass::Instance->Place_Crate(waypointCell, (PowerupEffects)pThis->Value);
		return placed;
	}
	case TriggerAction::TextTrigger:
	{
		const auto text = std::string(pThis->Text);

		if (!text.empty())
		{
			int idx = ScenarioClass::Instance->PlayerSideIndex ? (ScenarioClass::Instance->PlayerSideIndex != 1 ? 5 : 1) : 2;
			if (SideClass* pSide = SideClass::Array->GetItemOrDefault(ScenarioClass::Instance->PlayerSideIndex))
			{
				if (SideExtData* pExt = SideExtContainer::Instance.Find(pSide))
				{
					idx = pExt->MessageTextColorIndex;
				}
			}

			const int color = SessionClass::Instance->Game_GetLinkedColor(idx);
			const int delay =(int)(RulesClass::Instance->MessageDelay * TICKS_PER_MINUTE);
			auto pText = StringTable::FetchString(text.c_str());
			MessageListClass* pMessage = ScenarioExtData::Instance()->NewMessageList ?
				ScenarioExtData::Instance()->NewMessageList.get() :
				&MessageListClass::Instance();

			pMessage->AddMessage(nullptr, 0, pText, color, TextPrintType::UseGradPal | TextPrintType::FullShadow | TextPrintType::Point6Grad, delay, false);
		}

		return true;
	}
	case TriggerAction::PlayAnimAt:
	{
		ret = AresTActionExt::PlayAnimAt(pThis, pTargetHouse, pSourceObject, pTrigger, plocation);
		return true;
	}
	case TriggerAction::MeteorShower:
	{
		ret = AresTActionExt::MeteorStrike(pThis, pTargetHouse, pSourceObject, pTrigger, plocation);
		return true;
	}
	case TriggerAction::LightningStrike:
	{
		ret = AresTActionExt::LightstormStrike(pThis, pTargetHouse, pSourceObject, pTrigger, plocation);
		return true;
	}
	case TriggerAction::ActivateFirestorm:
	{
		ret = AresTActionExt::ActivateFirestorm(pThis, pTargetHouse, pSourceObject, pTrigger, plocation);
		return true;
	}
	case TriggerAction::DeactivateFirestorm:
	{
		ret = AresTActionExt::DeactivateFirestorm(pThis, pTargetHouse, pSourceObject, pTrigger, plocation);
		return true;
	}
	case TriggerAction::NukeStrike:
	{
		ret = AresTActionExt::LauchhNuke(pThis, pTargetHouse, pSourceObject, pTrigger, plocation);
		return true;
	}
	case TriggerAction::ChemMissileStrike:
	{
		ret = AresTActionExt::LauchhChemMissile(pThis, pTargetHouse, pSourceObject, pTrigger, plocation);
		return true;
	}
	case TriggerAction::DoExplosionAt:
	{
		ret = AresTActionExt::DoExplosionAt(pThis, pTargetHouse, pSourceObject, pTrigger, plocation);
		return true;
	}
	case TriggerAction::RetintRed:
	{
		ret = AresTActionExt::Retint(pThis, pTargetHouse, pSourceObject, pTrigger, plocation, DefaultColorList::Red);
		return true;
	}
	case TriggerAction::RetintGreen:
	{
		ret = AresTActionExt::Retint(pThis, pTargetHouse, pSourceObject, pTrigger, plocation, DefaultColorList::Green);
		return true;
	}
	case TriggerAction::RetintBlue:
	{
		ret = AresTActionExt::Retint(pThis, pTargetHouse, pSourceObject, pTrigger, plocation, DefaultColorList::Blue);
		return true;
	}
	default:
		return false;
	}
}

NOINLINE std::string AresNewTriggerAction_ToString(AresNewTriggerAction action)
{
	switch (action)
	{
	case AresNewTriggerAction::AuxiliaryPower: return "AuxiliaryPower";
	case AresNewTriggerAction::KillDriversOf: return "KillDriversOf";
	case AresNewTriggerAction::SetEVAVoice: return "SetEVAVoice";
	case AresNewTriggerAction::SetGroup: return "SetGroup";
	default: return "";
	}
}

NOINLINE std::string PhobosTriggerAction_ToString(PhobosTriggerAction action)
{
	switch (action)
	{
	case PhobosTriggerAction::SaveGame: return "SaveGame";
	case PhobosTriggerAction::EditVariable: return "EditVariable";
	case PhobosTriggerAction::GenerateRandomNumber: return "GenerateRandomNumber";
	case PhobosTriggerAction::PrintVariableValue: return "PrintVariableValue";
	case PhobosTriggerAction::BinaryOperation: return "BinaryOperation";
	case PhobosTriggerAction::RunSuperWeaponAtLocation: return "RunSuperWeaponAtLocation";
	case PhobosTriggerAction::RunSuperWeaponAtWaypoint: return "RunSuperWeaponAtWaypoint";
	case PhobosTriggerAction::DumpVariables: return "DumpVariables";
	case PhobosTriggerAction::PrintMessageRemainingTechnos: return "PrintMessageRemainingTechnos";
	case PhobosTriggerAction::ToggleMCVRedeploy: return "ToggleMCVRedeploy";
	case PhobosTriggerAction::UndeployToWaypoint: return "UndeployToWaypoint";
	case PhobosTriggerAction::SetDropCrate: return "SetDropCrate";
	case PhobosTriggerAction::SetTriggerTechnoVeterancy: return "SetTriggerTechnoVeterancy";
	case PhobosTriggerAction::TransactMoneyFor: return "TransactMoneyFor";
	case PhobosTriggerAction::SetAIMode: return "SetAIMode";
	case PhobosTriggerAction::DrawAnimWithin: return "DrawAnimWithin";
	case PhobosTriggerAction::SetAllOwnedFootDestinationTo: return "SetAllOwnedFootDestinationTo";
	case PhobosTriggerAction::FlashTechnoFor: return "FlashTechnoFor";
	case PhobosTriggerAction::UnInitTechno: return "UnInitTechno";
	case PhobosTriggerAction::GameDeleteTechno: return "GameDeleteTechno";
	case PhobosTriggerAction::LightningStormStrikeAtObject: return "LightningStormStrikeAtObject";
	case PhobosTriggerAction::ResetHateValue: return "ResetHateValue";
	case PhobosTriggerAction::EditAngerNode: return "EditAngerNode";
	case PhobosTriggerAction::ClearAngerNode: return "ClearAngerNode";
	case PhobosTriggerAction::SetForceEnemy: return "SetForceEnemy";
	case PhobosTriggerAction::CreateBannerGlobal: return "CreateBannerGlobal";
	case PhobosTriggerAction::CreateBannerLocal: return "CreateBannerLocal";
	case PhobosTriggerAction::DeleteBanner: return "DeleteBanner";
	case PhobosTriggerAction::MessageForSpecifiedHouse: return "MessageForSpecifiedHouse";
	case PhobosTriggerAction::RandomTriggerPut: return "RandomTriggerPut";
	case PhobosTriggerAction::RandomTriggerRemove: return "RandomTriggerRemove";
	case PhobosTriggerAction::RandomTriggerEnable: return "RandomTriggerEnable";
	case PhobosTriggerAction::ScoreCampaignText: return "ScoreCampaignText";
	case PhobosTriggerAction::ScoreCampaignTheme: return "ScoreCampaignTheme";
	case PhobosTriggerAction::SetNextMission: return "SetNextMission";
	default: return "";
	}
}

bool FakeTActionClass::_OperatorBracket(HouseClass* pTargetHouse, ObjectClass* pSourceObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	std::string_view name = magic_enum::enum_name(this->ActionKind);

	if(name.empty())
		name = AresNewTriggerAction_ToString((AresNewTriggerAction)this->ActionKind);

	if (name.empty())
		name = PhobosTriggerAction_ToString((PhobosTriggerAction)this->ActionKind);

	Debug::LogInfo("TAction[{} - {}] triggering [{}]", (void*)this, name, (int)this->ActionKind);
	bool ret = true;

	if (pSourceObject && !pSourceObject->IsAlive) {
		pSourceObject = 0;
	}

	if (_OverrideOriginalActions(this, pTargetHouse, pSourceObject, pTrigger, plocation, ret))
	{
		return ret;
	}
	else if (TActionExtData::Occured(this, { pTargetHouse,pSourceObject,pTrigger,plocation }, ret)) {
		return ret;
	}
	else if (AresTActionExt::Execute(this, pTargetHouse, pSourceObject, pTrigger, plocation, ret)) {
		return ret;
	}
	else {
		return this->ExecuteAction(this->ActionKind , pTargetHouse, pSourceObject, pTrigger, plocation);
	}
}

#ifdef _fucked
DEFINE_FUNCTION_JUMP(CALL , 0x726605, FakeTActionClass::_OperatorBracket)
#else
//DEFINE_FUNCTION_JUMP(LJMP, 0x6E1F60, FakeTActionClass::_TActionClass_Create_Team)

ASMJIT_PATCH(0x6DD8D7, TActionClass_Execute_Ares, 0xA)
{
	GET(FakeTActionClass* const, pAction, ESI);
	GET(ObjectClass* const, pObject, ECX);

	GET_STACK(HouseClass* const, pHouse, 0x254);
	GET_STACK(TriggerClass* const, pTrigger, 0x25C);
	GET_STACK(CellStruct*, pLocation, 0x260);

	enum { Handled = 0x6DFDDD, Default = 0x6DD8E7u };

	auto ret = true;

	std::string_view name = magic_enum::enum_name(pAction->ActionKind);
	std::string from = "Vanilla";

	if (name.empty()) {
		name = AresNewTriggerAction_ToString((AresNewTriggerAction)pAction->ActionKind);
		from = "Ares";
	}

	if (name.empty()){
		name = PhobosTriggerAction_ToString((PhobosTriggerAction)pAction->ActionKind);
		from = "Phobos";
	}

	Debug::LogInfo("TAction[{} - {}] triggering [{}] {}", (void*)pAction, name, (int)pAction->ActionKind , from);

	if (_OverrideOriginalActions(pAction, pHouse, pObject, pTrigger, pLocation, ret))
	{
		R->AL(ret);
		return Handled;
	}
	else if (TActionExtData::Occured(pAction, { pHouse , pObject , pTrigger , pLocation }, ret)) {
		R->AL(ret);
		return Handled;
	}
	else if (AresTActionExt::Execute(pAction, pHouse, pObject, pTrigger, pLocation, ret))
	{
		R->AL(ret);
		return Handled;
	}

	// replicate the original instructions, using underflow
	uint32_t const value = static_cast<uint32_t>(pAction->ActionKind) - 1;
	R->EDX(value);
	return (value > 144u) ? Handled : Default;
}

#endif