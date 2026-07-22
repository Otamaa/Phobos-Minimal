#include "Body.h"

#include <SessionClass.h>
#include <MessageListClass.h>

#include <SuperWeaponTypeClass.h>
#include <SuperClass.h>

#include <Utilities/SavegameDef.h>
#include <Utilities/Macro.h>

#include <BuildingClass.h>
#include <RadSiteClass.h>
#include <LightSourceClass.h>

#include <Ext/Building/Body.h>
#include <Ext/House/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/Scenario/Body.h>
#include <Ext/Terrain/Body.h>
#include <Ext/Rules/Body.h>
#include <Ext/Script/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TEvent/Body.h>
#include <Ext/Side/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/SWType/NewSuperWeaponType/NuclearMissile.h>
#include <Ext/Tactical/Body.h>
#include <Ext/Infantry/Body.h>
#include <Ext/Sidebar/Body.h>
#include <Ext/Mouse/Body.h>
#include <Ext/Team/Body.h>
#include <Ext/TeamType/Body.h>

#include <TagTypeClass.h>
#include <WaypointPathClass.h>

#include <New/Entity/BannerClass.h>
#include <New/Type/BannerTypeClass.h>

#include <Misc/DamageArea.h>

#include <New/MessageHandler/MessageColumnClass.h>

#include <TriggerTypeClass.h>

//Static init
#include <TagClass.h>
#include <numeric>
#include <CaptureManagerClass.h>
#include <RadarEventClass.h>
#include <TActionClass.h>
#include <TeamTypeClass.h>
#include <ThemeClass.h>
#include <UI.h>
#include <IonBlastClass.h>
#include <WWKeyboardClass.h>
#include <CommandClass.h>
#include <VoxelAnimClass.h>
#include <TriggerClass.h>

PhobosMap<int, std::vector<TriggerClass*>> TActionExtData::RandomTriggerPool;

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
*/
// =============================
// container
TActionExtContainer TActionExtContainer::Instance;


//==============================
bool TActionExtData::SetFollowsIndexForVehicle(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	int followerIndex = pThis->Param3;

	if (followerIndex < 0 || followerIndex >= UnitClass::Array->Count)
		return false;

	UnitClass* pNewFollower = UnitClass::Array->Items[followerIndex];
	if (!pNewFollower)
		return false;

	for (auto const pTechno : *TechnoClass::Array)
	{

		UnitClass* pFoot = cast_to<UnitClass*>(pTechno);

		if (!pFoot)
			continue;

		if (!pFoot->AttachedTag || !pFoot->AttachedTag->ContainsTrigger(pTrigger))
			continue;

		UnitClass* pLeader = static_cast<UnitClass*>(pFoot);

		if (UnitClass* pOldFollower = pLeader->FollowerCar)
		{
			pOldFollower->IsFollowerCar = false;
			pLeader->FollowerCar = nullptr;
		}

		for (auto pOther : *UnitClass::Array)
		{
			if (pOther && pOther->FollowerCar == pNewFollower)
			{
				pOther->FollowerCar = nullptr;
				break;
			}
		}

		pLeader->FollowerCar = pNewFollower;
		pNewFollower->IsFollowerCar = true;

	}

	return true;
}

bool TActionExtData::AdjustHouseModifier(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	const int amount = pThis->Param3;

	switch (pThis->Value)
	{
	case 0:
		pHouse->FirepowerMultiplier += (double)amount / 100.0;
		break;
	case 1:
		pHouse->ArmorMultiplier += (double)amount / 100.0;
		break;
	case 2:
		pHouse->GroundspeedMultiplier += (double)amount / 100.0;
		break;
	case 3:
		pHouse->AirspeedMultiplier += (double)amount / 100.0;
		break;
	case 4:
		pHouse->ROFMultiplier += (double)amount / 100.0;
		break;
	case 5:
		pHouse->CostMultiplier += (double)amount / 100.0;
		break;
	case 6:
		pHouse->BuildTimeMultiplier += (double)amount / 100.0;
		break;
	}


	return true;
}

bool TActionExtData::AllChangeHouse(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	bool changed = false;
	if (pTrigger) {
		if (HouseClass* NewOwnerPtr = TEventExtData::ResolveHouseParam(pThis->Value, pTrigger->GetHouse())) {
			for (int i = 0; i < TechnoClass::Array->Count; ++i) {
				const auto pItem = TechnoClass::Array->Items[i];

				if (!pItem)
					continue;

				if (!pItem->IsAlive || pItem->Health <= 0 || pItem->Owner != pHouse)
					continue;

				Debug::Log("SwitchAllObjectsToHouse for [%s] from [%x] with param3 [%d] [ %s -> %s ]\n", pItem->get_ID(), pThis, pThis->Param3, pItem->Owner->get_ID(), NewOwnerPtr->get_ID());

				if (pThis->Param3 && pItem->Passengers.FirstPassenger != nullptr) {
					FootClass* pPassenger = pItem->Passengers.FirstPassenger;

					do {
						pPassenger->SetOwningHouse(NewOwnerPtr, false);
						pPassenger = flag_cast_to<FootClass*, false>(pPassenger->NextObject);
					}
					while (pPassenger != nullptr && pPassenger->Transporter == pItem);
				}

				pItem->SetOwningHouse(NewOwnerPtr, false);

				if (BuildingClass* pBuilding = cast_to<BuildingClass*, false>(pItem)) {
					if (pBuilding->Type->Powered || pBuilding->Type->PoweredSpecial) {
						pBuilding->UpdatePowerDown();
					}
				}

				changed = true;
			}
		}
	}

	return changed;
}

bool TActionExtData::ChangeHouse(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	bool changed = false;
	if (pTrigger) {
		if (HouseClass* NewOwnerPtr = TEventExtData::ResolveHouseParam(pThis->Value, pTrigger->GetHouse())) {

			for (int i = 0; i < TechnoClass::Array->Count; ++i) {
				const auto pItem = TechnoClass::Array->Items[i];

				if (!pItem)
					continue;

				if (!pItem->IsAlive || pItem->Health <= 0 || pItem->InLimbo || !pItem->IsOnMap)
					continue;

				Debug::Log("ChangeOwner for [%s] from [%x] with param3 [%d] [ %s(%x) -> %s(%x) ]\n", pItem->get_ID(), pThis, pThis->Param3, pItem->Owner->get_ID(), pItem->Owner, NewOwnerPtr->get_ID(), NewOwnerPtr);

				if (pItem->AttachedTag && pItem->AttachedTag->ContainsTrigger(pTrigger)) {
					pItem->SetOwningHouse(NewOwnerPtr, false);

					if (pThis->Param3 != 0 && pItem->Passengers.FirstPassenger) {
						FootClass* pPassenger = pItem->Passengers.FirstPassenger;

						do {
							pPassenger->SetOwningHouse(NewOwnerPtr, false);
							pPassenger = flag_cast_to<FootClass*, false>(pPassenger->NextObject);
						}
						while (pPassenger != nullptr && pPassenger->Transporter == pItem);
					}

					changed = true;
				}
			}
		}
	}

	return changed;
}

bool TActionExtData::CreateBuildingAt(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	// Bugfix: TAction 125 Build At could neither display the buildups nor be AI-repairable in singleplayer mode
	// Sep 9, 2025 - Starkku: Fixed issues with buildups potentially ending up in infinite loops etc.
	// A separate issue remains where buildup sequence will interrupt if building's house changes mid-buildup,
	// but this applies to all buildings and not just ones created through the trigger.
	// Also restored Param3 to control the buildup display, only this time it is inverted (set to >0 to disable buildups).

	if(HouseClass* NewOwnerPtr = TEventExtData::ResolveHouseParam(pThis->Param5, pHouse)){
		auto coord = CellClass::Cell2Coord(ScenarioClass::Instance->GetWaypointCoords(pThis->Waypoint));
		//const auto pCell = MapClass::Instance->GetCellAt(coord);
		const auto v8 = BuildingTypeClass::FindIndexById(pThis->Text);

		if (v8 < 0)
			return false;

		const auto pBld = BuildingTypeClass::Array->Items[v8];
		const bool playBuildup = pBld->LoadBuildup();
		bool created = false;

		if (auto pBuilding = static_cast<BuildingClass*>(pBld->CreateObject(NewOwnerPtr))) {

			// Set before unlimbo cause otherwise it will call BuildingClass::Place.
			pBuilding->QueueMission(Mission::Construction, false);
			pBuilding->NextMission();

			if (!pBuilding->ForceCreate(coord)) {
				GameDelete<true, false>(pBld);
			} else {

				// Reset mission and build state if we're not going to play buildup afterwards.
				if (!playBuildup) {
					pBuilding->BeginMode(BStateType::Idle);
					pBuilding->QueueMission(Mission::Guard, false);
					pBuilding->NextMission();
					pBuilding->Place(false); // Manually call this now.
				}

				if (SessionClass::IsCampaign() && !NewOwnerPtr->IsControlledByHuman())
					pBuilding->ShouldRebuild = pThis->Param4 > 0;

				created = true;
			}
		}

		return created;
	}

	return false;
}

bool TActionExtData::CreateBannerGlobal(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	const auto pBannerType = BannerTypeClass::Find(pThis->Text);

	if (!pBannerType)
		return true;

	auto& banners = BannerManagerClass::Instance.Array;

	bool foundAny = false;

	banners.for_each([&](BannerClass& pBanner)
 {
	 if (pBanner.ID == pThis->Param3)
	 {
		 foundAny = true;
		 pBanner.Type = pBannerType;
		 pBanner.Position = { static_cast<int>(pThis->Param4 / 100.0 * DSurface::ViewBounds->Width), static_cast<int>(pThis->Param5 / 100.0 * DSurface::ViewBounds->Height) };
		 pBanner.Variable = pThis->Param6;
		 pBanner.IsGlobalVariable = true;
		 return true;
	 }

	 return false;
	});

	if (!foundAny)
	{
		banners.emplace_back(pBannerType, pThis->Param3, Point2D { pThis->Param4, pThis->Param5 }, pThis->Param6, true);
	}
	return true;
}

bool TActionExtData::CreateBannerLocal(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	const auto pBannerType = BannerTypeClass::Find(pThis->Text);

	if (!pBannerType)
		return true;

	auto& banners = BannerManagerClass::Instance.Array;

	bool foundAny = false;

	banners.for_each([&](BannerClass& pBanner)
 {
	 if (pBanner.ID == pThis->Param3)
	 {
		 foundAny = true;
		 pBanner.Type = pBannerType;
		 pBanner.Position = { static_cast<int>(pThis->Param4 / 100.0 * DSurface::ViewBounds->Width), static_cast<int>(pThis->Param5 / 100.0 * DSurface::ViewBounds->Height) };
		 pBanner.Variable = pThis->Param6;
		 pBanner.IsGlobalVariable = false;
		 return true;
	 }

	 return false;
	});

	if (!foundAny) {
		banners.emplace_back(pBannerType, pThis->Param3, Point2D { pThis->Param4, pThis->Param5 }, pThis->Param6, false);
	}
	return true;
}

bool TActionExtData::DeleteBanner(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	BannerManagerClass::Instance.Array.remove_all_if([pThis](const BannerClass& pBanner) {
		 return pBanner.ID == pThis->Value;
	});

	return true;
}

bool TActionExtData::ResetHateValue(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	if (pThis->Value < 0) {
		for (auto pTargetHouse : *HouseClass::Array()) {
			for (auto& pAngerNode : pTargetHouse->AngerNodes)
				pAngerNode.AngerLevel = 0;
		}

	} else {
		HouseClass* pTargetHouse = TEventExtData::ResolveHouseParam(pThis->Value, nullptr);

		if (pTargetHouse && pTargetHouse->AngerNodes.Count > 0) {
			for (auto& pAngerNode : pTargetHouse->AngerNodes)
				pAngerNode.AngerLevel = 0;
		}
	}

	return true;
}

bool TActionExtData::UndeployToWaypoint(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	const auto& nCell = ScenarioExtData::Instance()->Waypoints[pThis->Waypoint];
	AbstractClass* pCell = MapClass::Instance->TryGetCellAt(nCell);

	if (!pCell) {
		return true;
	}

	bool allHouse = false;
	HouseClass* vHouse = nullptr;
	const int houseIndex = pThis->Param3;

	if (houseIndex >= 0) {
		vHouse = HouseClass::Index_IsMP(houseIndex) ?
			HouseClass::FindByIndex(houseIndex) : HouseClass::FindByCountryIndex(houseIndex);
	}
	else if (houseIndex == -1) {
		allHouse = true;
	}

	if (!allHouse && !vHouse) {
		return true;
	}

	const char* buildingName = pThis->TechnoID;
	bool allBuilding = false;
	BuildingTypeClass* pBuildingType = nullptr;

	if (strcmp(pThis->Text, GameStrings::AllStr) == 0) {
		allBuilding = true;
	} else {
		pBuildingType = BuildingTypeClass::Find(buildingName);
	}

	if (!allBuilding && !pBuildingType) {
		return true;
	}

	// Thanks to chaserli for the relevant code!
	// There should be a more perfect way to do this, but I don't know how.
	auto canUndeploy = [pThis, pTrigger, allBuilding, allHouse, pBuildingType, vHouse](BuildingClass* pBuilding)
		{
			auto const pType = pBuilding->Type;

			if (!pType->UndeploysInto ||
				(!allBuilding && pType != pBuildingType) ||
				(!allHouse && pBuilding->Owner != vHouse) ||
				!pBuilding->IsAlive || pBuilding->Health <= 0 || pBuilding->InLimbo)
			{
				return false;
			}

			if (pType->ConstructionYard)
			{
				// Conyards can't undeploy if MCVRedeploy=no
				if (!GameModeOptionsClass::Instance->MCVRedeploy)
					return false;
				// or MindControlledBy YURIX (why? for balance?)
				if (!FakeRulesClass::Instance()->AllowDeployControlledMCV && pBuilding->MindControlledBy)
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
		const auto old = std::exchange(VocClass::VoicesEnabled(), false);
		pBld->SetArchiveTarget(pCell);
		pBld->Sell(true);
		VocClass::VoicesEnabled = old;
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
				Matrix3D::MatrixMultiply(&Vec3Dresult , &TacticalClass::Instance->IsoTransformMatrix, &Vec3rot);
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

	if (!pCell)
		return true;

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
			if ((pOrigOwner == pOwner && pTech->Owner == pOrigOwner) || !pTech->CaptureManager || !pTech->CaptureManager->SetOriginalOwnerToCivilian(pTech))
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
	case PhobosTriggerAction::GiveCredits:
		ret = TActionExtData::GiveCredits(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::EnableShortGame:
		ret = TActionExtData::EnableShortGame(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::DisableShortGame:
		ret = TActionExtData::DisableShortGame(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::MakeElite:
		ret = TActionExtData::MakeElite(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::EnableAllyReveal:
		ret = TActionExtData::EnableAllyReveal(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::DisableAllyReveal:
		ret = TActionExtData::DisableAllyReveal(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::DeleteObject:
		ret = TActionExtData::DeleteObject(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::AllAssignMission:
		ret = TActionExtData::AllAssignMission(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::MakeAllyOneWay:
		ret = TActionExtData::MakeAllyOneWay(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::MakeEnemyOneWay:
		ret = TActionExtData::MakeEnemyOneWay(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
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

	case PhobosTriggerAction::AttachSoundToObjects:
	{
		if(pThis->Value >= 0) {
			for (auto& pObj : *ObjectClass::Array) {
				if (pObj && pObj->IsAlive && pObj->IsOnMap && pThis->TagType && pThis->TagType->ContainsTrigger(pTrigger->Type)) {
					pObj->AttachSound(pThis->Value);
					ret = true;
				}
			}
		}

		return true;
	}
	case PhobosTriggerAction::RemoveSoundFromObjects:
	{
		for (auto& pObj : *ObjectClass::Array) {
			if (pObj && pObj->IsAlive && pObj->IsOnMap && pThis->TagType && pThis->TagType->ContainsTrigger(pTrigger->Type)) {
					pObj->AttachSound(-1);
					ret = true;
			}
		}

		return true;
	}

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
	case PhobosTriggerAction::RandomTriggerPut:
		ret = TActionExtData::RandomTriggerPut(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::RandomTriggerEnable:
		ret = TActionExtData::RandomTriggerEnable(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::RandomTriggerRemove:
		ret = TActionExtData::RandomTriggerRemove(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
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
	case PhobosTriggerAction::SetFollowsIndexForVehicle:
		ret = TActionExtData::SetFollowsIndexForVehicle(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::PrintMessageRemainingTechnos:
		ret = TActionExtData::PrintMessageRemainingTechnos(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::SetDropCrate:
		ret = TActionExtData::SetDropCrate(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::ResetHateValue:
		ret = TActionExtData::ResetHateValue(pThis, pHouse, pObject, pTrigger, args.plocation);
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
	case PhobosTriggerAction::SetFreeRadar:
		ret = TActionExtData::SetFreeRadar(pThis, pHouse, pObject, pTrigger, args.plocation);
		break;
	case PhobosTriggerAction::SetTeamDelay:
		ret = TActionExtData::SetTeamDelay(pThis, pHouse, pObject, pTrigger, args.plocation);
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
	case PhobosTriggerAction::AdjustHouseModifier:
		ret = TActionExtData::AdjustHouseModifier(pThis, pHouse, pObject, pTrigger, args.plocation);
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
				case 4: { if (pThis->Param4 != 0) pAngerNode.AngerLevel /= pThis->Param4; break; }
				case 5: { if (pThis->Param4 != 0) pAngerNode.AngerLevel %= pThis->Param4; break; }
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
		auto audcoord = CellClass::Cell2Coord(ScenarioExtData::Instance()->DefinedAudioWaypoints
			[pScen->Random.RandomFromMax(ScenarioExtData::Instance()->DefinedAudioWaypoints.size() - 1)]);
		VocClass::SafeImmedietelyPlayAt(pThis->Value , &audcoord);

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
		if (iter->IsValid())
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

bool TActionExtData::ActivateFirestorm(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* location)
{
	if (pHouse->FirestormActive)
	{
		HouseExtData::SetFirestormState(pHouse, true);
	}

	return true;
}

bool TActionExtData::DeactivateFirestorm(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* location)
{
	if (pHouse->FirestormActive)
	{
		HouseExtData::SetFirestormState(pHouse, false);
	}
	return true;
}

bool TActionExtData::AuxiliaryPower(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* location)
{
	const auto pDecidedHouse = pAction->FindHouseByIndex(pTrigger, pAction->Value);

	if (!pDecidedHouse)
		return false;

	HouseExtContainer::Instance.Find(pDecidedHouse)->AuxPower += pAction->Value2;
	pDecidedHouse->RecheckPower = true;
	return true;
}

bool TActionExtData::KillDriversOf(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* location)
{
	auto pDecidedHouse = pAction->FindHouseByIndex(pTrigger, pAction->Value);
	if (!pDecidedHouse)
		pDecidedHouse = HouseExtData::FindSpecial();

	for (auto pUnit : *FootClass::Array)
	{
		if (pUnit->Health > 0 && pUnit->IsAlive && pUnit->IsOnMap && !pUnit->InLimbo)
		{
			if (pUnit->AttachedTag && pUnit->AttachedTag->ContainsTrigger(pTrigger))
			{
				if (!TechnoExtContainer::Instance.Find(pUnit)->Is_DriverKilled
					&& TechnoExtData::IsDriverKillable(pUnit, 1.0))
				{
					TechnoExtData::ApplyKillDriver(pUnit, nullptr, pDecidedHouse, false, Mission::Harmless);
				}
			}
		}
	}

	return true;
}

bool TActionExtData::SetEVAVoice(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* location)
{
	if (pAction->Value < (int)EVAVoices::Types.size())
	{
		VoxClass::EVAIndex = MaxImpl(pAction->Value, -1);
		return true;
	}

	return false;
}

bool TActionExtData::SetGroup(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* location)
{
	if (auto pTech = flag_cast_to<TechnoClass*>(pObject))
	{
		pTech->Group = pAction->Value;
		return true;
	}

	return false;
}

//TODO : re-eval
bool TActionExtData::LauchhNuke(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* location)
{
	const auto pFind = WeaponTypeClass::Find(GameStrings::NukePayload);
	if (!pFind)
		return false;

	const auto nLoc = ScenarioClass::Instance->GetWaypointCoords(pAction->Waypoint);
	auto nCoord = CellClass::Cell2Coord(nLoc);
	nCoord.Z = MapClass::Instance->GetCellFloorHeight(nCoord);

	if (MapClass::Instance->GetCellAt(nCoord)->ContainsBridge())
		nCoord.Z += Unsorted::BridgeHeight;

	SW_NuclearMissile::DropNukeAt(nullptr, nCoord, nullptr, pHouse, pFind);

	//if (auto pBullet = pFind->Projectile->CreateBullet(MapClass::Instance->GetCellAt(nCoord), nullptr, pFind->Damage, pFind->Warhead, 50, false))
	//{
	//	pBullet->SetWeaponType(pFind);
	//	VelocityClass nVel {};
	//
	//	double nSin = Math::sin(Math::Math::PI_BY_TWO_APPROX);
	//	double nCos = Math::cos(Math::Math::PI_BY_TWO_APPROX);
	//
	//	double nX = nCos * nCos * -100.0;
	//	double nY = nCos * nSin * -100.0;
	//	double nZ = nSin * -100.0;
	//
	//	BulletExtContainer::Instance.Find(pBullet)->Owner = pHouse;
	//	pBullet->MoveTo({ nCoord.X , nCoord.Y , nCoord.Z + 20000 }, nVel);
	//	return true;
	//}

	return false;
}

//TODO : re-eval

bool TActionExtData::LauchhChemMissile(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* location)
{
	const auto pFind = WeaponTypeClass::Find(GameStrings::ChemLauncher);
	if (!pFind)
		return false;

	auto nLoc = ScenarioClass::Instance->GetWaypointCoords(pAction->Waypoint);

	if (auto pBullet = pFind->Projectile->CreateBullet(MapClass::Instance->GetCellAt(nLoc), nullptr, pFind->Damage, pFind->Warhead, 20, false))
	{
		pBullet->SetWeaponType(pFind);
		double nSin = Math::SIN_PI_BY_TWO_ACCURATE;
		double nCos = Math::COS_DIRECTION_FIXED_MAGIC;
		BulletExtContainer::Instance.Find(pBullet)->Owner = pHouse;
		auto nCell = MapClass::Instance->Localsize_586AC0(&nLoc, false);

		pBullet->MoveTo(
			{ nCell.X + 128 , nCell.Y + 128 , 0 },
			{ nCos * nCos * 100.0  , nCos * nSin * 100.0  , nSin * 100.0 }
		);
		return true;
	}

	return false;
}

bool TActionExtData::LightstormStrike(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* location)
{
	auto nLoc = ScenarioClass::Instance->GetWaypointCoords(pAction->Waypoint);

	// get center of cell coords
	auto const pCell = MapClass::Instance->GetCellAt(nLoc);
	auto coords = pCell->GetCoordsWithBridge();

	// create a cloud animation
	if (coords.IsValid())
	{
		// select the anim
		auto const& itClouds = RulesClass::Instance->WeatherConClouds;
		auto const pAnimType = itClouds.Items[ScenarioClass::Instance->Random.RandomFromMax(itClouds.Count - 1)];

		if (pAnimType)
		{
			coords.Z += GeneralUtils::GetLSAnimHeightFactor(pAnimType, pCell, true);

			if (coords.IsValid())
			{
				// create the cloud and do some book keeping.auto const
				auto pAnim = GameCreate<AnimClass>(pAnimType, coords);
				pAnim->SetHouse(pHouse);
				LightningStorm::CloudsManifesting->push_back(pAnim);
				LightningStorm::CloudsPresent->push_back(pAnim);
			}
		}
	}

	return true;
}

bool TActionExtData::MeteorStrike(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* location)
{
	static COMPILETIMEEVAL reference<int, 0x842AFC, 5u> MeteorAddAmount {};

	const auto pSmall = AnimTypeClass::Find(GameStrings::METSMALL);
	const auto pBig = AnimTypeClass::Find(GameStrings::METLARGE);

	if (!pSmall && !pBig)
		return false;

	auto nLoc = ScenarioClass::Instance->GetWaypointCoords(pAction->Waypoint);
	CoordStruct nCoord = CellClass::Cell2Coord(nLoc);
	nCoord.Z = MapClass::Instance->GetCellFloorHeight(nCoord);

	if (MapClass::Instance->GetCellAt(nCoord)->ContainsBridge())
		nCoord.Z += Unsorted::BridgeHeight;

	const auto amount = MeteorAddAmount[pAction->Value % MeteorAddAmount.size()] + ScenarioClass::Instance->Random.Random() % 3;
	if (amount <= 0)
		return true;

	const int nTotal = 70 * amount;

	for (int i = nTotal; i > 0; --i)
	{
		auto nRandX = ScenarioClass::Instance->Random.Random() % nTotal;
		auto nRandY = ScenarioClass::Instance->Random.Random() % nTotal;
		CoordStruct nAnimLoc { nRandX + nCoord.X ,nRandY + nCoord.Y ,nCoord.Z };

		AnimTypeClass* pSelected = pBig;
		int nRandHere = Math::abs(ScenarioClass::Instance->Random.Random()) & 0x80000001;
		bool v13 = nRandHere == 0;
		if (nRandHere < 0)
		{
			v13 = ((nRandHere - 1) | 0xFFFFFFFE) == -1;
		}

		if (v13)
		{
			pSelected = pSmall;
		}

		if (pSelected)
		{
			auto pAnim = GameCreate<AnimClass>(pSelected, nAnimLoc, 0, 1, AnimFlag::AnimFlag_600, 0, 0);
			pAnim->Owner = pHouse;
		}
	}

	return true;
}

bool TActionExtData::PlayAnimAt(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* location)
{
	if (const auto pAnimType = AnimTypeClass::Array->get_or_default(pAction->Value))
	{
		auto nLoc = ScenarioClass::Instance->GetWaypointCoords(pAction->Waypoint);
		CoordStruct nCoord = CellClass::Cell2Coord(nLoc);
		nCoord.Z = MapClass::Instance->GetCellFloorHeight(nCoord);

		if (MapClass::Instance->GetCellAt(nCoord)->ContainsBridge())
			nCoord.Z += Unsorted::BridgeHeight;
		//Debug::LogInfo("Trigger %s - Tag %s PlayAnimAt at(%d %d %d) Anim[%s - %d]",
		//	pAction->TriggerType ? pAction->TriggerType->get_ID() : GameStrings::NoneStr(),
		//	pAction->TagType ? pAction->TagType->get_ID() : GameStrings::NoneStr(),
		//	nCoord.X, nCoord.Y, nCoord.Z,
		//	pAnimType->ID,
		//	pAction->Value
		//);

		auto pAnim = GameCreate<AnimClass>(pAnimType, nCoord, 0, 1, AnimFlag::AnimFlag_600, 0, 0);
		pAnim->IsPlaying = !pAction->Param3;
		pAnim->Owner = pHouse;
	}

	return true;
}

bool TActionExtData::DoExplosionAt(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* location)
{
	if (const auto pWeaponType = WeaponTypeClass::Array->get_or_default(pAction->Value))
	{
		auto nLoc = ScenarioClass::Instance->GetWaypointCoords(pAction->Waypoint);
		CoordStruct nCoord = CellClass::Cell2Coord(nLoc);
		nCoord.Z = MapClass::Instance->GetCellFloorHeight(nCoord);
		const auto pCell = MapClass::Instance->GetCellAt(nCoord);

		if (pCell->ContainsBridge())
			nCoord.Z += Unsorted::BridgeHeight;

		//Debug::LogInfo("Trigger %s - Tag %s DoExplosion at(%d %d %d) Weapon[%s] Warhead[%s]",
		//	pAction->TriggerType ? pAction->TriggerType->get_ID() : GameStrings::NoneStr() ,
		//	pAction->TagType ? pAction->TagType->get_ID() : GameStrings::NoneStr(),
		//	nCoord.X, nCoord.Y , nCoord.Z,
		//	pWeaponType->ID,
		//	pWeaponType->Warhead->ID
		//);

		if (auto pAnimType = MapClass::SelectDamageAnimation(pWeaponType->Damage, pWeaponType->Warhead, pCell->LandType, nCoord))
		{
			auto pAnim = GameCreate<AnimClass>(pAnimType, nCoord, 0, 1, AnimFlag::AnimFlag_2600, -15, 0);
			pAnim->IsPlaying = true;
			pAnim->Owner = pHouse;
		}

		MapClass::FlashbangWarheadAt(pWeaponType->Damage, pWeaponType->Warhead, nCoord);
		DamageArea::Apply(&nCoord, pWeaponType->Damage, nullptr, pWeaponType->Warhead, true, pHouse);
	}

	return true;
}

bool TActionExtData::EnableTrigger(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* location)
{
	if (pAction->TriggerType)
	{
		TriggerClass::Array->for_each([pAction](TriggerClass* pTrig) {

			if (pTrig->Type == pAction->TriggerType)
			{
				if (ScenarioClass::Instance->Difficulty1 == AIDifficulty::Easy && pTrig->Type->Difficulty[0]
					|| ScenarioClass::Instance->Difficulty1 == AIDifficulty::Normal && pTrig->Type->Difficulty[1]
					|| ScenarioClass::Instance->Difficulty1 == AIDifficulty::Hard && pTrig->Type->Difficulty[2])
				{

					pTrig->Enable();
				}
			}
		});
	}

	return true;
}

bool TActionExtData::Retint(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* location, DefaultColorList col)
{

	TintStruct copy_ = ScenarioClass::Instance->NormalLighting.Tint;
	switch (col)
	{

	case DefaultColorList::Red:
		copy_.Red = pAction->Value * 10;
		copy_.Green *= 10;
		copy_.Blue *= 10;
		ScenarioClass::RecalcLighting(copy_.Red, copy_.Green, copy_.Blue, false);
		ScenarioClass::Instance->NormalLighting.Tint.Red = pAction->Value;
		break;
	case DefaultColorList::Green:
		copy_.Green = pAction->Value * 10;
		copy_.Red *= 10;
		copy_.Blue *= 10;
		ScenarioClass::RecalcLighting(copy_.Red, copy_.Green, copy_.Blue, false);
		ScenarioClass::Instance->NormalLighting.Tint.Green = pAction->Value;

		break;
	case DefaultColorList::Blue:
		copy_.Blue = pAction->Value * 10;
		copy_.Red *= 10;
		copy_.Green *= 10;
		ScenarioClass::RecalcLighting(copy_.Red, copy_.Green, copy_.Blue, false);
		ScenarioClass::Instance->NormalLighting.Tint.Blue = pAction->Value;
		break;
	default:
		return false;
	}

	ScenarioExtData::UpdateLightSources = true;
	return true;
}

bool TActionExtData::Execute(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* location, bool& ret)
{
	switch ((AresNewTriggerAction)pAction->ActionKind)
	{
	case AresNewTriggerAction::AuxiliaryPower:
	{
		ret = AuxiliaryPower(pAction, pHouse, pObject, pTrigger, location);
		break;
	}
	case AresNewTriggerAction::KillDriversOf:
	{
		ret = KillDriversOf(pAction, pHouse, pObject, pTrigger, location);
		break;
	}
	case AresNewTriggerAction::SetEVAVoice:
	{
		ret = SetEVAVoice(pAction, pHouse, pObject, pTrigger, location);
		break;
	}
	case AresNewTriggerAction::SetGroup:
	{
		ret = SetGroup(pAction, pHouse, pObject, pTrigger, location);
		break;
	}
	default:
	{
		return false;
	}
	}

	return true;
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

		if (SuperClass* pSuper = House->Supers.get_or_default(swIdx))
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
		 TerrainExtContainer::Instance.Find(nPair)->LightSource.reset(nullptr);
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
		auto const iter = std::ranges::find_if(nPool,
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

bool TActionExtData::GiveCredits(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	if (HouseClass* hptr = (FakeHouseClass*)TEventExtData::ResolveHouseParam(pThis->Value, pTrigger->House)) {
		hptr->TransactMoney(pThis->Param3);
	}

	return true;
}

bool TActionExtData::EnableShortGame(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	GameModeOptionsClass::Instance->ShortGame = true;
	return true;
}

bool TActionExtData::DisableShortGame(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	GameModeOptionsClass::Instance->ShortGame = false;
	return true;
}

bool TActionExtData::MakeElite(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	for (int i = 0; i < TechnoClass::Array->Count; i++) {
		TechnoClass* techno = TechnoClass::Array->Items[i];

		if (techno->IsAlive && techno->IsOnMap && !techno->InLimbo) {
			if (techno->AttachedTag && techno->AttachedTag->ContainsTrigger(pTrigger)) {
				techno->Veterancy.SetElite();
			}
		}
	}

	return true;
}

bool TActionExtData::EnableAllyReveal(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	RulesClass::Instance->AllyReveal = true;
	return true;
}

bool TActionExtData::DisableAllyReveal(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	RulesClass::Instance->AllyReveal = false;
	return true;
}

bool TActionExtData::DeleteObject(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	std::set<TechnoClass*> Obj;

	for (int i = 0; i < TechnoClass::Array->Count; i++) {
		TechnoClass* techno = TechnoClass::Array->Items[i];

		if (techno->IsAlive && techno->IsOnMap && !techno->InLimbo) {
			if (techno->AttachedTag && techno->AttachedTag->ContainsTrigger(pTrigger)) {
				Obj.emplace(techno);
			}
		}
	}

	for (auto pTech : Obj) {
		pTech->UnInit();
	}

	return true;
}

bool TActionExtData::AllAssignMission(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	for (int i = 0; i < TechnoClass::Array->Count; i++) {
		TechnoClass* techno = TechnoClass::Array->Items[i];

		if (techno->IsAlive && techno->IsOnMap && !techno->InLimbo) {
			if (techno->AttachedTag && techno->AttachedTag->ContainsTrigger(pTrigger)) {
				techno->QueueMission((Mission)pThis->Value, true);
			}
		}
	}

	return true;
}

bool TActionExtData::MakeEnemyOneWay(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	if (HouseClass* hptr = (FakeHouseClass*)TEventExtData::ResolveHouseParam(pThis->Value, pTrigger->House)) {
		Unsorted::ScenarioInit++;
		pHouse->MakeEnemy(hptr,false);
		--Unsorted::ScenarioInit;
	}

	return true;
}

bool TActionExtData::MakeAllyOneWay(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	if (HouseClass* hptr = (FakeHouseClass*)TEventExtData::ResolveHouseParam(pThis->Value, pTrigger->House)) {
		Unsorted::ScenarioInit++;
		pHouse->MakeAlly(hptr, false);
		--Unsorted::ScenarioInit;
	}

	return true;
}

bool TActionExtData::RandomTriggerEnable(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	const int iPoolID = pThis->Param3;
	const bool bTakeOff = pThis->Param4;

	auto& nPools = TActionExtData::RandomTriggerPool;

	if (!nPools.contains(iPoolID))
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

	if (!nPools.contains(iPoolID))
		return true;

	auto& nPool = nPools[iPoolID];
	auto const iter = std::ranges::find_if(nPool,
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
		pHousesList->push_back(HouseClass::Array->get_or_default(param3));
	}
	else
	{
		// Pick a group of countries from [AIHousesList].
		// Any house of the same type of the listed at [AIHousesList] will be included here

		if (FakeRulesClass::Instance()->AIHousesLists.empty() || (size_t)pThis->Param4 >= FakeRulesClass::Instance()->AIHousesLists.size()) {
			Debug::LogInfo("Map action {}: [AIHousesList] is empty. This action will be skipped.", (int)pThis->ActionKind);
			return true;
		}

		std::vector<HouseTypeClass*>* housesList = &FakeRulesClass::Instance()->AIHousesLists[pThis->Param4];

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

	if ((size_t)listIdx >= FakeRulesClass::Instance()->AITargetTypesLists.size()
		|| FakeRulesClass::Instance()->AITargetTypesLists[listIdx].empty()) {
		Debug::LogInfo("Map action {}: List [AITargetTypes]({}) is empty. This action will be skipped.", (int)pThis->ActionKind, listIdx);
		return true;
	}

	std::vector<TechnoTypeClass*>* technosList = &FakeRulesClass::Instance()->AITargetTypesLists[listIdx];
	std::vector<int> technosRemaining;
	int globalRemaining = 0;

	// Count all valid instances
	for (auto const& pType : *technosList) {
		int nRemaining = 0;

		for (const auto pTechno : *TechnoClass::Array)
		{
			if (!IsUnitAvailable(pTechno, false) || GET_TECHNOTYPE(pTechno) != pType)
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

	if (!file.IsAvaible()) {
		if (!file.Create()) {
			return false;
		}
	}

	if (!file.Open1(FileAccessMode::ReadWrite)) {
		Debug::LogInfo(__FUNCTION__" Failed to Open file {} for", fileName);
		return false;
	}

	CCINIClass ini {};
	ini.ReadCCFile(&file);
	const auto variables = ScenarioExtData::GetVariables(pThis->Param3 != 0);
	std::ranges::for_each(*variables, [&](const auto& variable) {
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

bool TActionExtData::SetFreeRadar(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	if (pHouse->IsControlledByHuman())
	{
		auto const pHouseExt = HouseExtContainer::Instance.Find(pHouse);

		switch (pThis->Param3)
		{
		case 1:
			pHouseExt->FreeRadar = true;
			pHouseExt->ForceRadar = false;
			break;
		case 2:
			pHouseExt->FreeRadar = true;
			pHouseExt->ForceRadar = true;
			break;
		case 3:
			pHouseExt->FreeRadar = false;
			pHouseExt->ForceRadar = true;
			break;
		default:
			pHouseExt->FreeRadar = false;
			pHouseExt->ForceRadar = false;
			break;
		}

		pHouse->RecheckRadar = true;
	}

	return true;
}

bool TActionExtData::SetTeamDelay(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	const int timer = pThis->Param3 < 0 ? RulesClass::Instance->TeamDelays.Items[pHouse->GetAIDifficultyIndex()] : pThis->Param3;
	HouseExtContainer::Instance.Find(pHouse)->TeamDelay = timer;

	auto& Timer = pHouse->TeamDelayTimer;
	const int time = MinImpl(Timer.TimeLeft, timer);

	if (Timer.StartTime == -1 && Timer.TimeLeft != 0 && time > 0) {
		Timer.TimeLeft = time;
	} else if (Timer.InProgress() || time >= 0) {
		Timer.Start(time);
	}

	return true;
}
#pragma optimize("", off )
static NOINLINE bool _OverrideOriginalActions(TActionClass* pThis, HouseClass* pTargetHouse, ObjectClass* pSourceObject, TriggerClass* pTrigger, CellStruct* plocation, bool& ret) {

	switch (pThis->ActionKind)
	{
	case TriggerAction::Win:
	{
		auto pCurPlayer = HouseClass::CurrentPlayer();

		if (pThis->Value == pCurPlayer->Type->ParentIdx)
			HouseClass::CurrentPlayer->Win(false);
		else {
			Debug::LogInfo("TAction Win {} ParentIndex  [Value {} - Player {}] value missmatch preturn Lose", (void*)pThis, pThis->Value, pCurPlayer->Type->ParentIdx);
			HouseClass::CurrentPlayer->Lose(false);
		}

		ret = true;
		return true;
	}
	
	case TriggerAction::Lose:
	{
		
		auto pCurPlayer = HouseClass::CurrentPlayer();

		if (pThis->Value == pCurPlayer->Type->ParentIdx)
			HouseClass::CurrentPlayer->Lose(false);
		else {
			Debug::LogInfo("TAction Lose {} ParentIndex  [Value {} - Player {}] value missmatch preturn WIN", (void*)pThis, pThis->Value, pCurPlayer->Type->ParentIdx);
			HouseClass::CurrentPlayer->Win(false);
		}
		ret = true;
		return true;
	}
	
	case TriggerAction::ProductionBegins:
	{
		ret = false;
		if (auto pTrigOwner = pThis->FindHouseByIndex(pTrigger, pThis->Value)) {
			pTrigOwner->Production = true;
			ret = true;
		}

		return true;
	}
	
	case TriggerAction::CreateTeam:
	{
		++Unsorted::ScenarioInit;
		if (auto pTeam = pThis->TeamType)
			pTeam->CreateTeam(nullptr);
		--Unsorted::ScenarioInit;
		ret = true;
		return true;
	}
	
	case TriggerAction::DestroyTeam:
	{
		if (auto pTeam = pThis->TeamType)
			pTeam->DestroyAllInstances();

		ret = true;
		return true;
	}
	
	case TriggerAction::AllToHunt:
	{
		ret = false;
		if (auto pTrigOwner = pThis->FindHouseByIndex(pTrigger, pThis->Value)) {
			pTrigOwner->All_To_Hunt();
			ret = true;
		}

		return true;
	}
	
	case TriggerAction::Reinforcement:
	{
		ret = false;

		if (auto pTeam = pThis->TeamType) {
			ret = FakeTeamTypeClass::_DoReinforcement(pTeam, -1);
		}


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

		if (auto pAnim = GameCreate<AnimClass>(RulesClass::Instance->DropZoneAnim, coord))
			pAnim->IsPlaying = true;

		ret = true;
		return true;
	}
	
	case TriggerAction::FireSale:
	{
		ret = false;
		if (auto pTrigOwner = pThis->FindHouseByIndex(pTrigger, pThis->Value)) {
			pTrigOwner->AIMode = AIMode::SellAll;
			ret = true;
		}

		return true;
	}
	
	case TriggerAction::PlayMovie:
	{
		Game::UIStuffs_MenuStuffs();
		WWMouseClass::Instance->ReleaseMouse();
		ScenarioClass::ToggleDisplayMode(0);
		Game::PlayMovie(pThis->Value, -1, 1, 1, 1);
		ScenarioClass::ToggleDisplayMode(1);
		WWMouseClass::Instance->CaptureMouse();
		Game::Reset_SomeShapes_Post_Movie();
		ret = true;
		return true;
	}
	
	case TriggerAction::TextTrigger:
	{
		const auto text = std::string(pThis->Text);

		if (!text.empty()) {
			int idx = ScenarioClass::Instance->PlayerSideIndex
				? (ScenarioClass::Instance->PlayerSideIndex != 1 ? 5 : 1)
				: 2;

			if (auto pSide = SideClass::Array->get_or_default(ScenarioClass::Instance->PlayerSideIndex))
			{
				if (auto pExt = SideExtContainer::Instance.Find(pSide))
					idx = pExt->MessageTextColorIndex;
			}

			const int color = SessionClass::Instance->Game_GetLinkedColor(idx);
			const int delay = static_cast<int>(RulesClass::Instance->MessageDelay * TICKS_PER_MINUTE);
			auto pText = StringTable::FetchString(text.c_str());

			if (Phobos::Config::MessageDisplayInCenter)
				MessageColumnClass::Instance.AddMessage(nullptr, pText, delay, false);
			else
				MessageListClass::Instance->AddMessage(nullptr, 0, pText, color, TextPrintType::UseGradPal | TextPrintType::FullShadow | TextPrintType::Point6Grad, delay, false);
		}

		ret = true;
		return true;
	}

	case TriggerAction::GlobalSet:
	{
		ScenarioClass::Instance->GlobalVarChange(pThis->Value, true);
		ret = true;
		return true;
	}

	case TriggerAction::GlobalClear:
	{
		ScenarioClass::Instance->GlobalVarChange(pThis->Value, false);
		ret = true;
		return true;
	}

	case TriggerAction::AutocreateBegins:
	{
		ret = false;
		if (auto const pHouse = 
			TEventExtData::ResolveHouseParam(pThis->Value, pTrigger ? pTrigger->House : nullptr))
		{
			pHouse->AutocreateAllowed = true;
			ret = true;
		}

		return true;
	}

	case TriggerAction::PreferredTarget:
	{
		if (pTargetHouse) {
			pTargetHouse->PreferredTargetType = static_cast<QuarryType>(pThis->Value);
		}

		ret = true;
		return true;
	}

	case TriggerAction::MakeAlly:
	{
		ret = false;
		auto const pHouseB = TEventExtData::ResolveHouseParam(pThis->Value, pTrigger ? pTrigger->House : nullptr);
		if (pTargetHouse && pHouseB) {
			pTargetHouse->MakeAlly(pHouseB, false);
			pHouseB->MakeAlly(pTargetHouse, false);
			ret = true;
		}

		return true;
	}

	case TriggerAction::MakeEnemy:
	{
		ret = false;
		auto const pHouseB = TEventExtData::ResolveHouseParam(pThis->Value, pTrigger ? pTrigger->House : nullptr);
		if (pTargetHouse && pHouseB) {
			pTargetHouse->MakeEnemy(pHouseB, false);
			pHouseB->MakeEnemy(pTargetHouse, false);
			ret = true;
		}

		return true;
	}

	case TriggerAction::AutoBaseBuilding:
	{
		if (pTargetHouse)
		{
			if (LOBYTE(pThis->Value))
			{
				pTargetHouse->AutoBaseBuilding = true;

				if (pTargetHouse->ConYards.Count > 0)
				{
					CellStruct cent = CellClass::Coord2Cell(pTargetHouse->ConYards[0]->Location);
					pTargetHouse->SetBaseSpawnCell(cent);
					pTargetHouse->Base.Center = cent;
				}

				if (!pTargetHouse->Base.BaseNodes.Count)
					pTargetHouse->BuildBaseBuilding();
			}
			else
			{
				pTargetHouse->AutoBaseBuilding = false;
			}
		}

		ret = true;
		return true;
	}

	case TriggerAction::GrowShroud:
	{
		DisplayClass::Instance->EnroachShadow();
		ret = true;
		return true;
	}

	case TriggerAction::DestroyAttachedObject:
	{
		ret = false;

		// Outer restart loop — vanilla assembly 0x6E2060/0x6E20EC.
		// Restarts when any kill occurs because deaths shift vector indices.
		bool restartScan = true;
		while (restartScan)
		{
			restartScan = false;

			for (int i = 0; i < TechnoClass::Array->Count; ++i)
			{
				auto pTech = TechnoClass::Array->Items[i];

				if (pTech->Health <= 0 || !pTech->IsAlive || !pTech->IsOnMap || pTech->InLimbo)
					continue;

				auto pTag = pTech->AttachedTag;
				if (!pTag || !pTag->ContainsTrigger(pTrigger))
					continue;

				// EXTENSION: Phobos limbo-placed building kill path (no vanilla equivalent)
				bool normalKill = true;
				if (pTech->WhatAmI() == AbstractType::Building)
				{
					auto pBldExt = BuildingExtContainer::Instance.Find(static_cast<BuildingClass*>(pTech));
					if (pBldExt && pBldExt->LimboID >= 0)
					{
						BuildingExtData::LimboKill(static_cast<BuildingClass*>(pTech));
						normalKill = false;
						ret = true;
						restartScan = true;
						--i;
					}
				}

				if (normalKill)
				{
					ret = true;
					int damage = pTech->Health;
					auto state = pTech->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, 0);

					// Assembly 0x6E20D8-0x6E20DF: dec edi only on kill.
					if (state >= DamageState::NowDead)
					{
						restartScan = true;
						--i;
					}
				}
			}
		}

		if (plocation->IsValid())
		{
			int attempt = 3;
			if (!MapClass::Instance->findsoemthing_587180(plocation)) // VERIFY: method name
			{
				do { --attempt; }
				while (attempt > 0 && !MapClass::Instance->findsoemthing_587180(plocation));
			}

			// BUG FIX (from previous session): coord build is outside the !findsoemthing block.
			// Assembly 0x6E213D: always reached after retry loop regardless of outcome.
			CoordStruct coord = CellClass::Cell2Coord(*plocation, Unsorted::BridgeHeight);
			auto screenPt = TacticalClass::Instance->CoordsToClient(coord);

			RectangleStruct rect { screenPt.X - 128, screenPt.Y - 128, 256, 256 };
			TacticalClass::Instance->RegisterDirtyArea(rect, false);
		}

		// Assembly 0x6E21B4: returns anyDestroyed flag, not hardcoded true.
		return true;
	}

	case TriggerAction::AddOneTimeSuperWeapon:
	{
		if (pTargetHouse)
		{
			if (auto pSuper = pTargetHouse->Supers.get_or_default(pThis->Value))
			{
				pSuper->Grant(true, 0, 0);
				if (HouseClass::CurrentPlayer() == pTargetHouse)
					SidebarClass::Instance->AddSpecialCameo(pThis->Value);
			}
		}

		ret = true;
		return true;
	}

	case TriggerAction::AddRepeatingSuperWeapon:
	{
		if (pTargetHouse)
		{
			if (auto pSuper = pTargetHouse->Supers.get_or_default(pThis->Value))
			{
				pSuper->Grant(0, 0, 0);
				pSuper->CanHold = false;
				if (HouseClass::CurrentPlayer() == pTargetHouse)
					SidebarClass::Instance->AddSpecialCameo(pThis->Value);
			}
		}

		ret = true;
		return true;
	}

	case TriggerAction::TimerStart:
	{
		auto& timer = ScenarioClass::Instance->MissionTimer;
		if (timer.IsTicking())
		{
			ret = true;
			return true;
		}

		timer.StartTime = Unsorted::CurrentFrame();
		MapClass::Instance->RedrawSidebar(1);
		ret = true;
		return true;
	}

	case TriggerAction::TimerStop:
	{
		auto& timer = ScenarioClass::Instance->MissionTimer;
		if (!timer.IsTicking())
		{
			ret = true;
			return true;
		}

		timer.TimeLeft = timer.GetTimeLeft();
		timer.StartTime = -1;
		MapClass::Instance->RedrawSidebar(1);
		ret = true;
		return true;
	}

	case TriggerAction::TimerExtend:
	{
		ScenarioClass::Instance->MissionTimer.Add(15 * pThis->Value);
		MapClass::Instance->RedrawSidebar(1);
		ret = true;
		return true;
	}

	case TriggerAction::TimerShorten:
	{
		ScenarioClass::Instance->MissionTimer.Sub(15 * pThis->Value);
		MapClass::Instance->RedrawSidebar(1);
		ret = true;
		return true;
	}

	case TriggerAction::TimerSet:
	{
		ScenarioClass::Instance->MissionTimer.Start(15 * pThis->Value);
		ScenarioClass::Instance->MissionTimerTextCSF = nullptr;
		ScenarioClass::Instance->MissionTimerText[0] = '\0';
		MapClass::Instance->RedrawSidebar(1);
		ret = true;
		return true;
	}

	case TriggerAction::TimerText:
	{
		ScenarioClass::Instance->MissionTimerTextCSF =
			pThis->Text[0] ? const_cast<wchar_t*>(StringTable::FetchString(pThis->Text)) : nullptr;
		ret = true;
		return true;
	}

	case TriggerAction::PlaySoundEffect:
	{
		VocClass::PlayGlobal(pThis->Value, Panning::Center, 1.0f, nullptr);
		ret = true;
		return true;
	}

	case TriggerAction::PlayMusicTheme:
	{
		ThemeClass::Instance->Queue(pThis->Value);
		ret = true;
		return true;
	}

	case TriggerAction::PlaySpeech:
	{
		VoxClass::PlayIndex(pThis->Value);
		ret = true;
		return true;
	}

	case TriggerAction::PlaySoundEffectRandom:
	{
		ret = TActionExtData::PlayAudioAtRandomWP(pThis, pTargetHouse, pSourceObject, pTrigger, plocation);
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
				pObj->AttachSound(pThis->Value);
			else
				VocClass::PlayIndexAtPos(pThis->Value, CellClass::Cell2Coord(waypointCell), true);
		}

		ret = true;
		return true;
	}

	case TriggerAction::PlayIngameMovie:
	{
		Game::Play_Ingame_Movie2(pThis->Value, 0);
		ret = true;
		return true;
	}

	case TriggerAction::PlayIngameMovieAndPause:
	{
		Game::Play_Ingame_Movie2(pThis->Value, 1);
		ret = true;
		return true;
	}

	case TriggerAction::DestroyTrigger:
	{
		if (pThis->TriggerType) {
			for(int i = TriggerClass::Array->Count - 1; i >= 0; --i){
				auto pTrig = TriggerClass::Array->Items[i];
				if (pTrig && pTrig->Type == pThis->TriggerType)
					pTrig->Destroy();
			}
		}

		ret = true;
		return true;
	}

	case TriggerAction::ForceTrigger:
	{
		if (pThis->TriggerType) {
			for (int i = 0; i < TriggerClass::Array->Count; ++i) {
				auto pTrig = TriggerClass::Array->Items[i];

				if (pTrig->Type == pThis->TriggerType)
					pTrig->FireActions(nullptr, CellStruct::Empty);
			}
		}

		ret = true;
		return true;
	}

	case TriggerAction::RevealAllMap:
	{
		auto pCurrent = HouseClass::CurrentPlayer();
		if (!pCurrent->Visionary)
		{
			pCurrent->Visionary = 1;
			MapClass::Instance->CellIteratorReset();
			for (auto i = MapClass::Instance->CellIteratorNext(); i; i = MapClass::Instance->CellIteratorNext())
				RadarClass::Instance->MapCell(i->MapCoords, pCurrent);

			MapClass::Instance->RedrawSidebar(1);
		}
		ret = true;
		return true;
	}

	case TriggerAction::RevealAroundWaypoint:
	{
		if (!HouseClass::CurrentPlayer->Visionary && ScenarioClass::Instance->IsDefinedWaypoint(pThis->Waypoint))
		{
			auto const waycell = ScenarioClass::Instance->GetWaypointCoords(pThis->Waypoint);
			auto coord = CellClass::Cell2Coord(waycell);
			coord.Z = MapClass::Instance->GetCellFloorHeight(coord);
			const int radius = RulesClass::Instance->RevealTriggerRadius;
			MapClass::Instance->RevealArea2(&coord, radius, HouseClass::CurrentPlayer, false, false, false, true, 0);
			MapClass::Instance->RevealArea2(&coord, radius, HouseClass::CurrentPlayer, false, false, false, true, 1);
		}

		ret = true;
		return true;
	}

	case TriggerAction::RevealWaypointZone:
	{
		if (!HouseClass::CurrentPlayer->Visionary) {
			auto cell = ScenarioClass::Instance->GetWaypointCoords(pThis->Waypoint);
			ZoneType oldZone = MapClass::Instance->GetMovementZoneType(cell, MovementZone::Crusher, false);
			MapClass::Instance->ResizeMap();

			if (auto pCellRZ = MapClass::Instance->GetResizedCell()) {
				do {
					if (MapClass::Instance->GetMovementZoneType(pCellRZ->MapCoords, MovementZone::Crusher, false) == oldZone)
 {
						const int v10 = pCellRZ->Level / 2;
						CoordStruct coord {
							((pCellRZ->MapCoords.X - v10 / 2) << 8) + 128,
							((pCellRZ->MapCoords.Y - v10 / 2) << 8) + 128,
							v10 * Unsorted::LevelHeight
						};
						MapClass::Instance->RevealArea2(&coord, 2, HouseClass::CurrentPlayer(), 0, 0, 0, 1, 0);
						MapClass::Instance->RevealArea2(&coord, 2, HouseClass::CurrentPlayer(), 0, 0, 0, 1, 1);
					}
					pCellRZ = MapClass::Instance->GetResizedCell();
				}
				while (pCellRZ);
			}
		}

		ret = true;
		return true;
	}

	case TriggerAction::SellBuilding:
	{
		ret = false;
		// EXTENSION: LimboID skip guard added (not in vanilla). Vanilla just calls Sell_Back.
		if (pTrigger) {
			for (auto pBld : *BuildingClass::Array) {
				if (!pBld || !pBld->IsAlive || !pBld->IsOnMap || pBld->InLimbo)
					continue;
				if (!pBld->AttachedTag || !pBld->AttachedTag->ContainsTrigger(pTrigger))
					continue;
				if (BuildingExtContainer::Instance.Find(pBld)->LimboID >= 0)
					continue;
				pBld->Sell(1);
				ret = true;
			}
		}

		return true;
	}

	case TriggerAction::TurnOffBuilding:
	{
		ret = false;
		if (pTrigger) {
			for (auto pBld : *BuildingClass::Array) {
				if (!pBld || !pBld->IsAlive || !pBld->IsOnMap || pBld->InLimbo)
					continue;
				if (!pBld->HasPower)
					continue;
				if (!pBld->AttachedTag || !pBld->AttachedTag->ContainsTrigger(pTrigger))
					continue;
				pBld->HasPower = false;
				pBld->UpdatePowerDown();
				ret = true;
			}
		}

		return true;
	}

	case TriggerAction::TurnOnBuilding:
	{
		ret = false;
		if (pTrigger) {
			for (auto pBld : *BuildingClass::Array) {
				if (!pBld || !pBld->IsAlive || !pBld->IsOnMap || pBld->InLimbo)
					continue;
				if (pBld->HasPower)
					continue;
				if (!pBld->AttachedTag || !pBld->AttachedTag->ContainsTrigger(pTrigger))
					continue;
				pBld->HasPower = true;
				pBld->UpdatePowerDown();
				ret = true;
			}
		}

		return true;
	}

	case TriggerAction::Apply100Damage:
	{
		auto const cell = ScenarioClass::Instance->GetWaypointCoords(pThis->Waypoint);
		auto coord = CellClass::Cell2Coord(cell);
		coord.Z = MapClass::Instance->GetCellFloorHeight(coord);

		if (auto const pWarhead = RulesClass::Instance->C4Warhead)
		{
			constexpr int damage = 100;
			DamageArea::Apply(&coord, damage, nullptr, pWarhead, true, nullptr);

			CoordStruct offsets[4] =
			{
				{ coord.X + 85, coord.Y + 85, coord.Z },
				{ coord.X - 85, coord.Y + 85, coord.Z },
				{ coord.X + 85, coord.Y - 85, coord.Z },
				{ coord.X - 85, coord.Y - 85, coord.Z },
			};

			for (auto& off : offsets)
			{
				off.Z = MapClass::Instance->GetCellFloorHeight(off);
				DamageArea::Apply(&off, damage, nullptr, pWarhead, true, nullptr);
			}
		}

		ret = true;
		return true;
	}

	case TriggerAction::SmallLightFlash:
	{
		auto const cell = ScenarioClass::Instance->GetWaypointCoords(pThis->Waypoint);
		auto coord = CellClass::Cell2Coord(cell);
		coord.Z = MapClass::Instance->GetCellFloorHeight(coord);
		MapClass::FlashbangWarheadAt(50, RulesClass::Instance->C4Warhead, coord);
		ret = true;
		return true;
	}

	case TriggerAction::MediumLightFlash:
	{
		auto const cell = ScenarioClass::Instance->GetWaypointCoords(pThis->Waypoint);
		auto coord = CellClass::Cell2Coord(cell);
		coord.Z = MapClass::Instance->GetCellFloorHeight(coord);
		MapClass::FlashbangWarheadAt(100, RulesClass::Instance->C4Warhead, coord);
		ret = true;
		return true;
	}

	case TriggerAction::LargeLightFlash:
	{
		auto const cell = ScenarioClass::Instance->GetWaypointCoords(pThis->Waypoint);
		auto coord = CellClass::Cell2Coord(cell);
		coord.Z = MapClass::Instance->GetCellFloorHeight(coord);
		MapClass::FlashbangWarheadAt(300, RulesClass::Instance->C4Warhead, coord);
		ret = true;
		return true;
	}

	case TriggerAction::AnnounceWin:
	{
		HouseClass::CurrentPlayer->Win(true);
		ret = true;
		return true;
	}

	case TriggerAction::AnnounceLose:
	{
		HouseClass::CurrentPlayer->Lose(true);
		ret = true;
		return true;
	}

	case TriggerAction::ForceEnd:
	{
		HouseClass::CurrentPlayer->ForceEnd();
		ret = true;
		return true;
	}

	case TriggerAction::DestroyTag:
	{
		for(int i = TagClass::Array->Count - 1; i >= 0;  --i){
			auto pTag = TagClass::Array->Items[i];
			if (pTag && pTag->Type == pThis->TagType)
				CallDTOR<false>(pTag);
		}

		ret = true;
		return true;
	}

	case TriggerAction::SetAmbientStep:
	{
		RulesClass::Instance->AmbientChangeStep = pThis->Value;
		ret = true;
		return true;
	}

	case TriggerAction::SetAmbientRate:
	{
		RulesClass::Instance->AmbientChangeRate = pThis->Value;
		ret = true;
		return true;
	}

	case TriggerAction::SetAmbientLight:
	{
		ScenarioClass::Instance->AmbientOriginal = pThis->Value;
		if (!LightningStorm::IsActive())
			ScenarioClass::Instance->AmbientTarget = ScenarioClass::Instance->AmbientOriginal;

		ret = true;
		return true;
	}

	case TriggerAction::AITriggersBegin:
	{
		ret = false;
		if (pTrigger) {
			if (auto NewOwnerPtr = TEventExtData::ResolveHouseParam(pThis->Value, pTrigger->GetHouse())) {
				NewOwnerPtr->AITriggersActive = true;
				ret = true;
			}
		}


		return true;
	}

	case TriggerAction::AITriggersStop:
	{
		ret = false;
		if (pTrigger) {
			if (auto NewOwnerPtr = TEventExtData::ResolveHouseParam(pThis->Value, pTrigger->GetHouse())) {
				NewOwnerPtr->AITriggersActive = false;
				ret = true;
			}
		}

		return true;
	}

	case TriggerAction::RatioOfAITriggerTeams:
	{
		if (pTargetHouse)
			pTargetHouse->RatioAITriggerTeam = pThis->Value;
		ret = true;
		return true;
	}

	case TriggerAction::RatioOfTeamAircraft:
	{
		if (pTargetHouse)
			pTargetHouse->RatioTeamAircraft = pThis->Value;
		ret = true;
		return true;
	}

	case TriggerAction::RatioOfTeamInfantry:
	{
		if (pTargetHouse)
			pTargetHouse->RatioTeamInfantry = pThis->Value;
		ret = true;
		return true;
	}

	case TriggerAction::RatioOfTeamUnits:
	{
		if (pTargetHouse)
			pTargetHouse->RatioTeamUnits = pThis->Value;
		ret = true;
		return true;
	}

	case TriggerAction::ReinforcementAt:
	{
		ret = false;
		if (pThis->TeamType && pThis->Waypoint != -1)
			ret = FakeTeamTypeClass::_DoReinforcement(pThis->TeamType, pThis->Waypoint);

		return true;
	}

	case TriggerAction::WakeupSelf:
	{
		if (pTrigger)
		{
			for (auto pTechno : *TechnoClass::Array)
			{
				if (!pTechno || pTechno->WhatAmI() == AbstractType::Building)
					continue;
				if (!pTechno->IsAlive || !pTechno->IsOnMap || pTechno->InLimbo)
					continue;
				if (!pTechno->AttachedTag || !pTechno->AttachedTag->ContainsTrigger(pTrigger))
					continue;

				auto const mission = pTechno->CurrentMission;
				if (mission == Mission::Sleep || mission == Mission::Harmless)
				{
					pTechno->QueueMission(Mission::Guard, false);
					pTechno->NextMission();
				}
			}
		}

		ret = true;
		return true;
	}

	case TriggerAction::WakeupAllSleepers:
	{
		// Vanilla: wakes sleeping foot units belonging to houses OTHER than trigger owner.
		for (auto pFoot : *FootClass::Array)
		{
			if (!pFoot || !pFoot->IsAlive || !pFoot->IsOnMap || pFoot->InLimbo)
				continue;
			if (pFoot->Owner == pTargetHouse)
				continue;
			if (pFoot->CurrentMission == Mission::Sleep)
			{
				pFoot->QueueMission(Mission::Guard, false);
				pFoot->NextMission();
			}
		}

		ret = true;
		return true;
	}

	case TriggerAction::WakeupAllHarmless:
	{
		// Vanilla: wakes harmless foot units belonging to SAME house as trigger owner.
		for (auto pFoot : *FootClass::Array)
		{
			if (!pFoot || !pFoot->IsAlive || !pFoot->IsOnMap || pFoot->InLimbo)
				continue;
			if (pFoot->Owner != pTargetHouse)
				continue;
			if (pFoot->CurrentMission == Mission::Harmless)
			{
				pFoot->QueueMission(Mission::Guard, false);
				pFoot->NextMission();
			}
		}

		ret = true;
		return true;
	}

	case TriggerAction::WakeupGroup:
	{
		for (auto pTechno : *TechnoClass::Array)
		{
			if (!pTechno || !pTechno->IsAlive || !pTechno->IsOnMap || pTechno->InLimbo)
				continue;
			if (pTechno->Group != pThis->Value)
				continue;

			auto const mission = pTechno->CurrentMission;
			if (mission == Mission::Sleep || mission == Mission::Harmless)
			{
				pTechno->QueueMission(Mission::Guard, false);
				pTechno->NextMission();
			}
		}

		ret = true;
		return true;
	}

	case TriggerAction::VeinGrowth:
	{
		ScenarioClass::Instance->VeinGrowthEnabled = pThis->Value != 0;
		ret = true;
		return true;
	}

	case TriggerAction::TiberiumGrowth:
	{
		ScenarioClass::Instance->TiberiumGrowthEnabled = pThis->Value != 0;
		ret = true;
		return true;
	}

	case TriggerAction::IceGrowth:
	{
		ScenarioClass::Instance->IceGrowthEnabled = pThis->Value != 0;
		ret = true;
		return true;
	}

	case TriggerAction::ParticleAnim:
	{
		if (auto const pType = ParticleSystemTypeClass::Array->get_or_default(pThis->Value))
		{
			auto const cell = ScenarioClass::Instance->GetWaypointCoords(pThis->Waypoint);
			auto coord = CellClass::Cell2Coord(cell);
			coord.Z = MapClass::Instance->GetCellFloorHeight(coord);
			CoordStruct emptyCoord {};
			GameCreate<ParticleSystemClass>(pType, &coord, nullptr, nullptr, &emptyCoord, nullptr);
		}

		ret = true;
		return true;
	}

	case TriggerAction::RemoveParticleAnim:
	{
		auto const wayCell = ScenarioClass::Instance->GetWaypointCoords(pThis->Waypoint);
		auto& pss = *ParticleSystemClass::Array;
		for (int i = 0; i < pss.Count; )
		{
			auto const pPS = pss.Items[i];
			if (pPS->GetMapCoords() == wayCell)
				pPS->UnInit();
			else
				++i;
		}

		ret = true;
		return true;
	}

	case TriggerAction::GoBerzerk:
	{
		ret = false;
		if (pTrigger)
		{
			for (auto pInfantry : *InfantryClass::Array)
			{
				if (!pInfantry || !pInfantry->IsAlive || !pInfantry->IsOnMap || pInfantry->InLimbo)
					continue;
				if (!pInfantry->AttachedTag || !pInfantry->AttachedTag->ContainsTrigger(pTrigger))
					continue;

				pInfantry->PermanentBerzerk = true;
				pInfantry->GoBerzerk();
				ret = true;
			}
		}

		return true;
	}

	case TriggerAction::IonStormStart:
	{
		ret = !LightningStorm::IsActive();
		if (ret)
			LightningStorm::Start(pThis->Value, RulesClass::Instance->LightningDeferment, CellStruct::Empty, nullptr);
		return true;
	}

	case TriggerAction::IonStormStop:
	{
		ret = LightningStorm::IsActive();
		if (ret)
			LightningStorm::RequestStop();
		return true;
	}

	case TriggerAction::LockInput:
	{
		Unsorted::PendingObject = nullptr;
		Unsorted::Display_PendingObject = nullptr;
		Unsorted::Display_PendingHouse = -1;
		DisplayClass::Instance->SetActiveFoundation(nullptr);
		DisplayClass::Instance->SetRepairMode(0);
		DisplayClass::Instance->SetSellMode(0);
		Unsorted::PowerToggleMode = false;
		Unsorted::PlanningMode = false;
		Unsorted::PlaceBeaconMode = false;
		ScenarioClass::LockInput();
		ret = true;
		return true;
	}

	case TriggerAction::UnlockInput:
	{
		Game::UnlockInput(); // VERIFY: typo in original — check actual export name
		ret = true;
		return true;
	}

	case TriggerAction::MoveCameraToWaypoint:
	{
		auto cell = ScenarioClass::Instance->GetWaypointCoords(pThis->Waypoint);
		auto coord = CellClass::Cell2Coord(cell);
		coord.Z = MapClass::Instance->GetZPos(&coord);
		if (MapClass::Instance->GetCellAt(cell)->ContainsBridgeEx())
			coord.Z += Unsorted::BridgeHeight;
		TacticalClass::Instance->FocusOn(&coord, pThis->Value);
		ret = true;
		return true;
	}

	case TriggerAction::CenterCameraAtWaypoint:
	{
		auto cell = ScenarioClass::Instance->GetWaypointCoords(pThis->Waypoint);
		auto coord = CellClass::Cell2Coord(cell);
		coord.Z = MapClass::Instance->GetZPos(&coord);
		if (MapClass::Instance->GetCellAt(cell)->ContainsBridgeEx())
			coord.Z += Unsorted::BridgeHeight;
		TacticalClass::Instance->SetTacticalPosition(&coord);
		ret = true;
		return true;
	}

	case TriggerAction::JumpCameraHome:
	{
		CommandClass::JumpHomeCommand();
		ret = true;
		return true;
	}

	case TriggerAction::ZoomIn:
	{
		TacticalClass::Instance->ZoomInFactor = RulesClass::Instance->ZoomInFactor;
		Game::UserInputLocked = true;
		WWKeyboardClass::Instance->Clear();
		WWMouseClass::Instance->HideCursor();
		ScrollClass::Instance->ScrollClass::ClearDragBand();
		GScreenClass::Instance->MarkNeedsRedraw(0);
		GScreenClass::Instance->Render();
		Sleep(1000u);
		ret = true;
		return true;
	}

	case TriggerAction::ZoomOut:
	{
		TacticalClass::Instance->ZoomInFactor = 1.0;
		WWKeyboardClass::Instance->Clear();
		Game::UserInputLocked = ScenarioClass::Instance->UserInputLocked;
		WWMouseClass::Instance->ShowCursor();
		GScreenClass::Instance->MarkNeedsRedraw(0);
		GScreenClass::Instance->Render();
		ret = true;
		return true;
	}

	case TriggerAction::ReshroudMap:
	{
		MapClass::Instance->Reshroud(pTargetHouse);
		ret = true;
		return true;
	}

	case TriggerAction::ChangeLightBehavior:
	{
		ret = false;
		for (auto pBld : *BuildingClass::Array) {
			if (!pBld || !pBld->IsAlive || pBld->InLimbo)
				continue;
			if (!pBld->Type->HasSpotlight || !pBld->Spotlight)
				continue;

			if (auto pTag = pBld->AttachedTag) {
				if (pTag->ContainsTrigger(pTrigger)) {
					pBld->Spotlight->SetBehaviour(static_cast<SpotlightBehaviour>(pThis->Value));
					ret = true;
				}
			}
		}

		return true;
	}

	case TriggerAction::EnableTrigger:
	{
		ret = TActionExtData::EnableTrigger(pThis, pTargetHouse, pSourceObject, pTrigger, plocation);
		return true;
	}

	case TriggerAction::DisableTrigger:
	{
		if (pThis->TriggerType)
		{
			for (auto pTrig : *TriggerClass::Array)
			{
				if (pTrig->Type == pThis->TriggerType)
					pTrig->Disable();
			}
		}

		ret = true;
		return true;
	}

	case TriggerAction::CreateRadarEvent:
	{
		auto cell = ScenarioClass::Instance->GetWaypointCoords(pThis->Waypoint);
		RadarEventClass::Create(static_cast<RadarEventType>(pThis->Value), cell);
		ret = true;
		return true;
	}

	case TriggerAction::LocalSet:
	{
		ScenarioClass::Instance->LocalVarChange(pThis->Value, true);
		ret = true;
		return true;
	}

	case TriggerAction::LocalClear:
	{
		ScenarioClass::Instance->LocalVarChange(pThis->Value, false);
		ret = true;
		return true;
	}

	case TriggerAction::MeteorShower:
	{
		ret = TActionExtData::MeteorStrike(pThis, pTargetHouse, pSourceObject, pTrigger, plocation);
		return true;
	}

	case TriggerAction::LightningStrike:
	{
		ret = TActionExtData::LightstormStrike(pThis, pTargetHouse, pSourceObject, pTrigger, plocation);
		return true;
	}

	case TriggerAction::ActivateFirestorm:
	{
		ret = TActionExtData::ActivateFirestorm(pThis, pTargetHouse, pSourceObject, pTrigger, plocation);
		return true;
	}

	case TriggerAction::DeactivateFirestorm:
	{
		ret = TActionExtData::DeactivateFirestorm(pThis, pTargetHouse, pSourceObject, pTrigger, plocation);
		return true;
	}

	case TriggerAction::NukeStrike:
	{
		ret = TActionExtData::LauchhNuke(pThis, pTargetHouse, pSourceObject, pTrigger, plocation);
		return true;
	}

	case TriggerAction::ChemMissileStrike:
	{
		ret = TActionExtData::LauchhChemMissile(pThis, pTargetHouse, pSourceObject, pTrigger, plocation);
		return true;
	}

	case TriggerAction::DoExplosionAt:
	{
		ret = TActionExtData::DoExplosionAt(pThis, pTargetHouse, pSourceObject, pTrigger, plocation);
		return true;
	}

	case TriggerAction::PlayAnimAt:
	{
		ret = TActionExtData::PlayAnimAt(pThis, pTargetHouse, pSourceObject, pTrigger, plocation);
		return true;
	}

	case TriggerAction::IonCannonStrike:
	{
		auto cell = ScenarioClass::Instance->GetWaypointCoords(pThis->Waypoint);
		auto coord = CellClass::Cell2Coord(cell);
		coord.Z = MapClass::Instance->GetZPos(&coord);
		GameCreate<IonBlastClass>(coord);
		ret = true;
		return true;
	}

	case TriggerAction::LightningStormStrike:
	{
		auto cell = ScenarioClass::Instance->GetWaypointCoords(pThis->Waypoint);
		ret = false;

		if (!LightningStorm::IsActive()) {
			LightningStorm::Start(RulesClass::Instance->LightningStormDuration, RulesClass::Instance->LightningDeferment, cell, pTargetHouse);
			ret = true;
		}

		return true;
	}

	case TriggerAction::ToggleTrainCargo:
	{
		ScenarioClass::Instance->TrainCrate = !ScenarioClass::Instance->TrainCrate;
		ret = true;
		return true;
	}

	case TriggerAction::ReduceTiberium:
	{
		auto const waycell = ScenarioClass::Instance->GetWaypointCoords(pThis->Waypoint);
		if (auto const pCell = MapClass::Instance->GetCellAt(waycell))
			pCell->ReduceTiberiumWithinCircularArea();
		ret = true;
		return true;
	}

	case TriggerAction::ReshroudMapAtWaypoint:
	{
		if (!HouseClass::CurrentPlayer->Visionary)
		{
			auto cell = ScenarioClass::Instance->GetWaypointCoords(pThis->Waypoint);
			int level = 0;
			auto pCell = MapClass::Instance->GetCellAt(cell);

			if (pCell->ContainsBridgeEx())
				level = 4;

			level += pCell->Level;

			CoordStruct coord = CellClass::Cell2Coord(cell, level * Unsorted::LevelHeight);
			MapClass::Instance->RevealRadius(&coord, RulesClass::Instance->RevealTriggerRadius);
			MapClass::Instance->RevealFOG();
		}

		ret = true;
		return true;
	}

	case TriggerAction::ResizePlayerView:
	{
		MapClass::MapLocalSize = pThis->Bounds;
		RadarClass::Instance->RadarClass_reinit();
		MapClass::Instance->CellIteratorReset();
		for (auto i = MapClass::Instance->CellIteratorNext(); i; i = MapClass::Instance->CellIteratorNext())
			i->RecalcAttributes(-1);

		MapClass::Instance->Update_Pathfinding_1();
		MapClass::Instance->Clear_SubzoneTracking();
		MapClass::Instance->Map_AI();
		for (auto pBld : *BuildingClass::Array) {
			if (pBld && pBld->IsAlive)
				pBld->RadarTrackingUpdate(true);
		}

		ret = true;
		return true;
	}

	case TriggerAction::IronCurtain:
	{
		auto cell = ScenarioClass::Instance->GetWaypointCoords(pThis->Waypoint);

		if (cell.IsValid()) {
			auto pCell = MapClass::Instance->GetCellAt(cell);
			auto coord = pCell->GetCoords();
			GameCreate<AnimClass>(RulesClass::Instance->IronCurtainInvokeAnim, coord);

			for (int i = 0; i < 9; ++i) {
				CellStruct offs = cell + CellSpread::CellOfssets[i];
				for (auto pOcc = MapClass::Instance->GetCellAt(offs)->FirstObject; pOcc; pOcc = pOcc->NextObject) {
					if (pOcc->IsAlive)
						pOcc->IronCurtain(RulesClass::Instance->IronCurtainDuration, nullptr, false);
				}
			}
		}

		ret = true;
		return true;
	}

	case TriggerAction::SetObjectTechLevel:
	{
		for (auto& pType : *TechnoTypeClass::Array) {
			if (IS_SAME_STR_N(pType->ID, pThis->TechnoID)) {
				pType->TechLevel = pThis->Value;
				break;
			}
		}

		for (auto& pHouse : *HouseClass::Array)
			pHouse->RecheckTechTree = true;

		ret = true;
		return true;
	}

	case TriggerAction::ReinforcementByChrono:
	{
		ret = false;
		if (pThis->TeamType && pThis->Waypoint != -1)
			ret = TActionClass::Do_Chrono_Reinforcements(pThis->TeamType, pThis->Waypoint);
		return true;
	}

	case TriggerAction::CreateCrate:
	{
		const CellStruct waypointCell = ScenarioExtData::Instance()->Waypoints[pThis->Waypoint];
		ret = MapClass::Instance->Place_Crate(waypointCell, static_cast<PowerupEffects>(pThis->Value));
		return true;
	}

	case TriggerAction::PauseGame:
	{
		ScenarioClass::PauseGameFor(pThis->Value);
		ret = true;
		return true;
	}

	case TriggerAction::EvictOccupiers:
	{
		ret = false;
		// EXTENSION: vanilla only evicts from the first matching building.
		// Your version iterates all matching buildings. Intentional improvement.
		for (auto pBld : *BuildingClass::Array)
		{
			if (!pBld || !pBld->IsAlive || pBld->InLimbo)
				continue;

			auto pExt = BuildingExtContainer::Instance.Find(pBld);
			if (pExt && pExt->LimboID >= 0)
				continue;

			if (auto pTag = pBld->AttachedTag)
			{
				if (pTag->ContainsTrigger(pTrigger))
				{
					pBld->KickAllOccupants(false, false);
					ret = true;
				}
			}
		}

		return true;
	}

	case TriggerAction::MakeHouseCheer:
	{
		ret = false;
		if (pTrigger)
		{
			if (auto NewOwnerPtr = TEventExtData::ResolveHouseParam(pThis->Value, pTrigger->GetHouse()))
			{
				NewOwnerPtr->Cheer();
				ret = true;
			}
		}

		return true;
	}

	case TriggerAction::SetTabTo:
	{
		ret = false;
		if (pThis->Value >= 0 && pThis->Value < 4)
		{
			if (SidebarClass::Column[pThis->Value].BuildableCount > 0)
			{
				SidebarClass::Instance->ChangeTab(pThis->Value);
				ret = true;
			}
		}

		return true;
	}

	case TriggerAction::FlashCameo:
	{
		ret = false;
		for (auto& pType : *TechnoTypeClass::Array)
		{
			if (IS_SAME_STR_N(pType->ID, pThis->TechnoID))
			{
				SidebarClass::Instance->FlashCameo(pType, pThis->Value);
				ret = true;
			}
		}

		return true;
	}

	case TriggerAction::FlashTeam:
	{
		if (auto pTeamType = pThis->TeamType)
			pTeamType->FlashAllInstances(pThis->Value);
		ret = true;
		return true;
	}

	case TriggerAction::TalkBubble:
	{
		if (auto pTeamType = pThis->TeamType)
		{
			if (auto pTeam = pTeamType->FindFirstInstance())
				pTeam->FirstUnit->CreateTalkBubble(pThis->Value);
		}

		ret = true;
		return true;
	}

	case TriggerAction::ClearAllSmudges:
	{
		MapClass::Instance->ClearAllSmudges();
		MapClass::Instance->RedrawSidebar(1);
		ret = true;
		return true;
	}

	case TriggerAction::DestroyAll:
	{
		ret = false;
		if (pTrigger)
		{
			if (auto pHouse = static_cast<FakeHouseClass*>(TEventExtData::ResolveHouseParam(pThis->Value, pTrigger->House)))
			{
				pHouse->_BlowUpAll();
				ret = true;
			}
		}
		return true;
	}

	case TriggerAction::DestroyAllBuildings:
	{
		ret = false;
		if (pTrigger)
		{
			if (auto pHouse = static_cast<FakeHouseClass*>(TEventExtData::ResolveHouseParam(pThis->Value, pTrigger->House)))
			{
				pHouse->_BlowUpAllBuildings();
				ret = true;
			}
		}
		return true;
	}

	case TriggerAction::DestroyAllLandUnits:
	{
		ret = false;
		if (pTrigger)
		{
			if (auto pHouse = TEventExtData::ResolveHouseParam(pThis->Value, pTrigger->House))
			{
				pHouse->DestroyAllNonBuildingsNonNaval();
				ret = true;
			}
		}
		return true;
	}

	case TriggerAction::DestroyAllNavalUnits:
	{
		ret = false;
		if (pTrigger)
		{
			if (auto pHouse = TEventExtData::ResolveHouseParam(pThis->Value, pTrigger->House))
			{
				pHouse->DestroyAllNonBuildingsNaval();
				ret = true;
			}
		}
		return true;
	}

	case TriggerAction::ChangeHouse:
	{
		ret = TActionExtData::ChangeHouse(pThis, pTargetHouse, pSourceObject, pTrigger, plocation);
		return true;
	}

	case TriggerAction::AllChangeHouse:
	{
		ret = TActionExtData::AllChangeHouse(pThis, pTargetHouse, pSourceObject, pTrigger, plocation);
		return true;
	}

	case TriggerAction::CreateBuilding:
	{
		ret = TActionExtData::CreateBuildingAt(pThis, pTargetHouse, pSourceObject, pTrigger, plocation);
		return true;
	}

	case TriggerAction::MindControlBase:
	{
		ret = false;
		if (pTrigger && pTargetHouse)
		{
			if (auto NewOwnerPtr = TEventExtData::ResolveHouseParam(pThis->Value, pTrigger->GetHouse()))
			{
				pTargetHouse->MindControlBaseOf(NewOwnerPtr);
				ret = true;
				return true;
			}
		}
		return true;
	}

	case TriggerAction::RestoreMindControlledBase:
	{
		ret = false;
		if (pTrigger && pTargetHouse)
		{
			if (auto NewOwnerPtr = TEventExtData::ResolveHouseParam(pThis->Value, pTrigger->GetHouse()))
			{
				pTargetHouse->RestoreMindControlledBase(NewOwnerPtr);
				ret = true;
				return true;
			}
		}
		return true;
	}

	case TriggerAction::RestoreStartingUnits:
	{
		ret = false;
		if (pTrigger)
		{
			if (auto NewOwnerPtr = TEventExtData::ResolveHouseParam(pThis->Value, pTrigger->GetHouse()))
			{
				NewOwnerPtr->RespawnStartingForces();
				ret = true;
				return true;
			}
		}


		return true;
	}

	case TriggerAction::RestoreStartingBuildings:
	{	
		ret = false;
		if (pTrigger)
		{
			if (auto NewOwnerPtr = TEventExtData::ResolveHouseParam(pThis->Value, pTrigger->GetHouse()))
			{
				NewOwnerPtr->RespawnStartingBuildings();
				ret = true;
				return true;
			}
		}

		return true;
	}

	case TriggerAction::StartChronoScreenEffect:
	{
		ChronoScreenEffect::Start(pThis->Value);
		ret = true;
		return true;
	}

	case TriggerAction::SetSuperWeaponCharge:
	{
		ret = false;
		if (pTargetHouse)
		{
			if (auto pSW = pTargetHouse->Supers.get_or_default(pThis->Value))
			{
				if (pSW->Granted)
				{
					if (pThis->Value2 < 0 || pThis->Value2 > 100)
					{
						ret = false;
						return true;
					}

					pSW->SetCharge(pThis->Value2);
					ret = true;
					return true;
				}
			}
		}

		return true;
	}

	case TriggerAction::FlashBuildingsOfType:
	{
		ret = false;
		if (pTargetHouse)
		{
			for (auto pBld : pTargetHouse->Buildings)
			{
				if (IS_SAME_STR_N(pBld->Type->ID, pThis->TechnoID))
					pBld->Flash(pThis->Value);
			}

			ret = true;
			return true;
		}

		return true;
	}

	case TriggerAction::SuperWeaponSetRechargeTime:
	{
		ret = false;
		if (pTargetHouse)
		{
			if (auto pSW = pTargetHouse->Supers.get_or_default(pThis->Value))
			{
				pSW->CustomChargeTime = pThis->Value2;
				ret = true;
				return true;
			}
		}

		return true;
	}

	case TriggerAction::SuperWeaponResetRechargeTime:
	{
		ret = false;
		if (pTargetHouse)
		{
			if (auto pSW = pTargetHouse->Supers.get_or_default(pThis->Value))
			{
				pSW->CustomChargeTime = -1;
				ret = true;
				return true;
			}
		}

		return true;
	}

	case TriggerAction::SuperWeaponReset:
	{
		ret = false;
		if (pTargetHouse)
		{
			if (auto pSW = pTargetHouse->Supers.get_or_default(pThis->Value))
			{
				pSW->Reset();
				ret = true;
				return true;
			}
		}

		return true;
	}

	case TriggerAction::SetPreferredTargetCell:
	{
		ret = false;
		if (pTargetHouse)
		{
			auto cell = ScenarioClass::Instance->GetWaypointCoords(pThis->Waypoint);
			if (cell.IsValid())
			{
				pTargetHouse->PreferredTargetCell = cell;
				ret = true;
				return true;
			}
		}

		return true;
	}

	case TriggerAction::ClearPreferredTargetCell:
	{
		ret = false;
		if (pTargetHouse)
		{
			pTargetHouse->PreferredTargetCell = CellStruct::Empty;
			ret = true;
			return true;
		}

		return true;
	}

	case TriggerAction::SetBaseCenterCell:
	{
		ret = false;
		if (pTargetHouse)
		{
			auto cell = ScenarioClass::Instance->GetWaypointCoords(pThis->Waypoint);
			if (cell.IsValid())
			{
				pTargetHouse->BaseCenter = cell;
				ret = true;
				return true;
			}
		}

		return true;
	}

	case TriggerAction::ClearBaseCenterCell:
	{
		ret = false;
		if (pTargetHouse) {
			pTargetHouse->BaseCenter = CellStruct::Empty;
			ret = true;
			return true;
		}

		return true;
	}

	case TriggerAction::BlackoutRadar:
	{
		ret = false;
		if (pTargetHouse) {
			pTargetHouse->CreateRadarOutage(pThis->Value);
			ret = true;
			return true;
		}

		return true;
	}

	case TriggerAction::SetDefensiveTargetCell:
	{
		ret = false;
		if (pTargetHouse) {
			auto cell = ScenarioClass::Instance->GetWaypointCoords(pThis->Waypoint);
			if (cell.IsValid())
			{
				pTargetHouse->SetDefensiveTarget(cell);
				ret = true;
				return true;
			}
		}

		return true;
	}

	case TriggerAction::ClearDefensiveTargetCell:
	{
		ret = false;
		if (pTargetHouse) {
			pTargetHouse->ClearDefensiveTarget();
			ret = true;
		}

		return true;
	}

	case TriggerAction::RetintRed:
	{
		ret = TActionExtData::Retint(pThis, pTargetHouse, pSourceObject, pTrigger, plocation, DefaultColorList::Red);
		return true;
	}

	case TriggerAction::RetintGreen:
	{
		ret = TActionExtData::Retint(pThis, pTargetHouse, pSourceObject, pTrigger, plocation, DefaultColorList::Green);
		return true;
	}

	case TriggerAction::RetintBlue:
	{
		ret = TActionExtData::Retint(pThis, pTargetHouse, pSourceObject, pTrigger, plocation, DefaultColorList::Blue);
		return true;
	}

	case TriggerAction::CreateVoxelAnim:
	{
		if (auto pVxlAnim = VoxelAnimTypeClass::Array->get_or_default(pThis->Value))
		{
			auto cell = ScenarioClass::Instance->GetWaypointCoords(pThis->Waypoint);
			auto coord = CellClass::Cell2Coord(cell);
			coord.Z = MapClass::Instance->GetZPos(&coord);

			if (MapClass::Instance->GetCellAt(cell)->ContainsBridgeEx())
				coord.Z += Unsorted::BridgeHeight;

			GameCreate<VoxelAnimClass>(pVxlAnim, coord, nullptr);
		}

		ret = true;
		return true;
	}

	case TriggerAction::StopSounds:
	{
		auto cell = ScenarioClass::Instance->GetWaypointCoords(pThis->Waypoint);
		auto cellCoord = CellClass::Cell2Coord(cell);
		bool perCoord = true;

		if (auto pCell = MapClass::Instance->TryGetCellAt(cellCoord))
		{
			if (auto pBld = pCell->GetBuilding())
			{
				perCoord = false;
				pBld->AttachSound(-1);
			}
			else if (auto pTerrain = pCell->GetTerrain(false))
			{
				perCoord = false;
				pTerrain->AttachSound(-1);
			}
		}

		if (perCoord)
			VocClass::StopPlayAt(&cellCoord, 1);

		ret = true;
		return true;
	}

	case TriggerAction::TeleportAll:
	{
		ret = false;
		if (pTargetHouse)
		{
			auto cell = ScenarioClass::Instance->GetWaypointCoords(pThis->Waypoint);
			pTargetHouse->TeleportAllTo(cell);
			ret = true;
		}

		return true;
	}

	case TriggerAction::AllowWin:
	case TriggerAction::ChangeZoomLevel:
	{
		ret = true;
		return true;//vanilla
	}
	default:
		return false;
	}
}
#pragma optimize("", on )

NOINLINE std::string AresNewTriggerAction_ToString(AresNewTriggerAction action)
{
	switch (action)
	{
	case AresNewTriggerAction::AuxiliaryPower: return "AuxiliaryPower";
	case AresNewTriggerAction::KillDriversOf: return "KillDriversOf";
	case AresNewTriggerAction::SetEVAVoice: return "SetEVAVoice";
	case AresNewTriggerAction::SetGroup: return "SetGroup";
	default: return {};
	}
}

NOINLINE std::string PhobosTriggerAction_ToString(PhobosTriggerAction action)
{
	switch (action)
	{
	case PhobosTriggerAction::AllAssignMission:   return "AllAssignMission";
	case PhobosTriggerAction::DeleteObject:       return "DeleteObject";
	case PhobosTriggerAction::DisableAllyReveal:  return "DisableAllyReveal";
	case PhobosTriggerAction::EnableAllyReveal:   return "EnableAllyReveal";
	case PhobosTriggerAction::MakeElite:          return "MakeElite";
	case PhobosTriggerAction::DisableShortGame:   return "DisableShortGame";
	case PhobosTriggerAction::EnableShortGame:    return "EnableShortGame";
	case PhobosTriggerAction::GiveCredits:        return "GiveCredits";
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
	case PhobosTriggerAction::SetTeamDelay: return "SetTeamDelay";
	case PhobosTriggerAction::SetFreeRadar: return "SetFreeRadar";
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
	default: return {};
	}
}

bool FakeTActionClass::_OperatorBracket(HouseClass* pTargetHouse, ObjectClass* pSourceObject, TriggerClass* pTrigger, CellStruct* plocation)
{
	std::string_view name;

	if (name.empty())
		name = AresNewTriggerAction_ToString((AresNewTriggerAction)this->ActionKind);

	if (name.empty())
		name = PhobosTriggerAction_ToString((PhobosTriggerAction)this->ActionKind);

	if(name.empty())
		name = magic_enum::enum_name(this->ActionKind);

	Debug::LogInfo("TAction[{} - {}] triggering [{}]", (void*)this, name, (int)this->ActionKind);
	bool ret = true;

	if (pSourceObject && !pSourceObject->IsAlive)
	{
		pSourceObject = 0;
	}

	if (_OverrideOriginalActions(this, pTargetHouse, pSourceObject, pTrigger, plocation, ret))
	{
		return ret;
	}
	
	if (TActionExtData::Occured(this, { pTargetHouse,pSourceObject,pTrigger,plocation }, ret))
	{
		return ret;
	}
	
	if (TActionExtData::Execute(this, pTargetHouse, pSourceObject, pTrigger, plocation, ret))
	{
		return ret;
	}

	return false;
}

std::string FakeTActionClass::_BuildINIEntry()
{
	const int           action = (int)this->ActionKind;    // [esi+2Ch]
	const LogicNeedType needs = (LogicNeedType)TActionClass::GetMode(action);
	const int           value = this->Value;     // [esi+90h]

	// --- phase 1: classify into ParamType + resolve str/heapid ---
	// Vanilla: series of cmp+jmp before the shared sprintf block.

	ParamType   param = ParamType::None;
	const char* str = nullptr;
	int         heapid = value;   // default: Value used as numeric arg
	int         str1 = 0;       // used only by Number3Percent
	int         v17 = 0;       // used only by Number3Percent (Value2)

	switch (needs)
	{
	case LogicNeedType::BuildingNNumber:
		// 0x6DD32B: ebx=9, ebp=&String1
		param = ParamType::Str1TechLevel;
		str = this->TechnoID;             // [esi+54h]  VERIFY: field name
		break;

	case LogicNeedType::NumberNSuper:
		// 0x6DD340: ebx=11, str1=Value(ecx=[esi+90h]), v17=Value2([esi+48h])
		param = ParamType::Number3Percent;
		str1 = value;
		v17 = this->Value2;              // [esi+48h]  VERIFY: field name
		break;

	case LogicNeedType::NumberNTech:
		// 0x6DD357: ebx=9, ebp=&String1
		param = ParamType::Str1TechLevel;
		str = this->TechnoID;             // [esi+54h]  VERIFY: field name
		break;

	case LogicNeedType::BuildingNWaypoint:
		// 0x6DD369: ebx=10, ebp=&String1
		param = ParamType::Str2Waypoint;
		str = this->TechnoID;             // [esi+54h]  VERIFY: field name
		break;

	default:
	{
		// Team checks — [esi+30h]
		TeamTypeClass* team = this->TeamType;  // VERIFY: field name
		if (team)
		{
			if (needs == LogicNeedType::Team2)
			{
				// 0x6DD382: ebx=5, heapid=[edx+98h], ebp=[edx+24h]
				param = ParamType::Team2;
				heapid = team->ArrayIndex;         // [team+98h]  VERIFY: field name
				str = team->ID;    // [team+24h]  VERIFY: field name
				break;
			}

			if (needs == LogicNeedType::Team || needs == LogicNeedType::TeamNWaypoint)
			{
				// 0x6DD3A7: ebx=1, heapid=[edx+98h], ebp=[edx+24h]
				param = ParamType::AsTeam;
				heapid = team->ArrayIndex;         // [team+98h]  VERIFY: field name
				str = team->ID;    // [team+24h]  VERIFY: field name
				break;
			}
		}

		// Trigger check — [esi+50h]
		TriggerTypeClass* trigger = this->TriggerType; // VERIFY: field name
		if (trigger && needs == LogicNeedType::Trigger)
		{
			// 0x6DD3CA: ebx=2, heapid=[edx+98h], ebp=[edx+24h]
			param = ParamType::AsTrigger;
			heapid = trigger->ArrayIndex;        // [trigger+98h]  VERIFY: field name
			str = trigger->ID;     // [trigger+24h]  VERIFY: field name
			break;
		}

		// Tag check — [esi+4Ch]
		TagTypeClass* tag = this->TagType;     // VERIFY: field name
		if (tag && needs == LogicNeedType::Tag)
		{
			// 0x6DD3EA: ebx=3, heapid=[edx+98h], ebp=[edx+24h]
			param = ParamType::AsTag;
			heapid = tag->ArrayIndex;            // [tag+98h]  VERIFY: field name
			str = tag->ID;         // [tag+24h]  VERIFY: field name
			break;
		}

		// Text check — [esi+6Dh] = String2[0]
		if (needs == LogicNeedType::Text)
		{
			// 0x6DD403: ebp=&String2[0]; if (!String2[0] || !ebp) ebp="-1"
			// Note: IDA's "this == -109" is garbage — assembly is just test ebp,ebp
			str = (this->Text && this->Text[0]) ? this->Text : "-1";
			param = ParamType::AsText;
			break;
		}

		// Audio name lookups
		switch (needs)
		{
		case LogicNeedType::Speech:
			// 0x6DD422: ebx=6, call Name_From_Vox(value)
			param = ParamType::AsSpeech;
			str = VoxClass::GetName(value);  // VERIFY: signature
			break;

		case LogicNeedType::Sound:
		case LogicNeedType::SoundNWaypoint:
			// 0x6DD44F: ebx=7, call VocClass::Get_Name_Vector(value)
			param = ParamType::AsSound;
			str = VocClass::GetName(value); // VERIFY: signature
			break;

		case LogicNeedType::Theme:
			// 0x6DD443: ebx=8, call ThemeClass::Get_Name_Vector(&Theme, value)
			param = ParamType::AsTheme;
			str = ThemeClass::Instance->GetID(value); // VERIFY: signature
			break;

		default:
			break;
		}

		break;
	}
	}

	// --- phase 2: format output ---
	// Vanilla: repne scasb (inline strlen) to find end of buffer, then sprintf.
	// Replaced with fmt::format returning std::string directly.

	const int bx = this->Bounds.X;      // [esi+34h]  VERIFY: field name
	const int by = this->Bounds.Y;      // [esi+38h]  VERIFY: field name
	const int bw = this->Bounds.Width;  // [esi+3Ch]  VERIFY: field name
	const int bh = this->Bounds.Height; // [esi+40h]  VERIFY: field name

	switch (param)
	{
	case ParamType::Number3Percent:
		// Vanilla: "%d,%d,%d,%d,%d,%d,%d,%d" — action,11,str1,X,Y,W,H,Value2
		// 0x6DD497
		return fmt::format("{},{},{},{},{},{},{},{}", action, 11, str1, bx, by, bw, bh, v17);

	case ParamType::Str2Waypoint:
	{
		// Vanilla: "%d,%d,%s,%d,%d,%d,%d,%s" — action,10,str,X,Y,W,H,waypoint
		// 0x6DD4B6: Waypoint_To_String(this->Waypoint)
		const char* wp = WaypointPathClass::WaypointIdxToString(this->Waypoint); // VERIFY: signature
		return fmt::format("{},{},{},{},{},{},{},{}", action, 10, str, bx, by, bw, bh, wp);
	}

	case ParamType::Team2:
	case ParamType::Str1TechLevel:
		// Vanilla: "%d,%d,%s,%d,%d,%d,%d,%d" — action,param,str,X,Y,W,H,Value
		// 0x6DD571 (param=5,9)
		return fmt::format("{},{},{},{},{},{},{},{}", action, static_cast<int>(param), str, bx, by, bw, bh, value);

	default:
	{
		const char* wp = WaypointPathClass::WaypointIdxToString(this->Waypoint); // VERIFY: signature

		if (param != ParamType::None)
		{
			// Vanilla: "%d,%d,%s,%d,%d,%d,%d,%s" — action,param,str,X,Y,W,H,waypoint
			// 0x6DD4FE (param != 0, != 5, != 9, != 10, != 11)
			return fmt::format("{},{},{},{},{},{},{},{}", action, static_cast<int>(param), str, bx, by, bw, bh, wp);
		}

		// Vanilla: "%d,%d,%d,%d,%d,%d,%d,%s" — action,0,heapid,X,Y,W,H,waypoint
		// 0x6DD535 (param == 0)
		return fmt::format("{},{},{},{},{},{},{},{}", action, 0, heapid, bx, by, bw, bh, wp);
	}
	}
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

#ifndef _fucked
DEFINE_FUNCTION_JUMP(CALL , 0x726605, FakeTActionClass::_OperatorBracket)
DEFINE_FUNCTION_JUMP(LJMP , 0x6DD8B0, FakeTActionClass::_OperatorBracket)
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
	else if (TActionExtData::Execute(pAction, pHouse, pObject, pTrigger, pLocation, ret))
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