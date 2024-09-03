#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/HouseType/Body.h>
#include <Ext/Side/Body.h>

#include <New/Type/GenericPrerequisite.h>

#include "Header.h"

static constexpr int ObserverBackgroundWidth = 121;
static constexpr int ObserverBackgroundHeight = 96;

static constexpr int ObserverFlagPCXX = 70;
static constexpr int ObserverFlagPCXY = 70;
static constexpr int ObserverFlagPCXWidth = 45;
static constexpr int ObserverFlagPCXHeight = 21;

DEFINE_HOOK(0x6AA0CA, StripClass_Draw_DrawObserverBackground, 6)
{
	enum { DrawSHP = 0x6AA0ED, DontDraw = 0x6AA159 };

	GET(HouseTypeClass*, pCountry, EAX);

	const auto pData = HouseTypeExtContainer::Instance.Find(pCountry);

	if (pData->ObserverBackgroundSHP)
	{
		R->EAX<SHPStruct*>(pData->ObserverBackgroundSHP);
		return DrawSHP;
	}
	else if (auto PCXSurface = pData->ObserverBackground.GetSurface())
	{
		GET(int, TLX, EDI);
		GET(int, TLY, EBX);
		RectangleStruct bounds = { TLX, TLY, ObserverBackgroundWidth, ObserverBackgroundHeight };
		PCX::Instance->BlitToSurface(&bounds, DSurface::Sidebar, PCXSurface, Drawing::ColorStructToWordRGB(Drawing::DefaultColors[6]));
	}

	return DontDraw;
}

DEFINE_HOOK(0x6AA164, StripClass_Draw_DrawObserverFlag, 6)
{
	enum { IDontKnowYou = 0x6AA16D, DrawSHP = 0x6AA1DB, DontDraw = 0x6AA2CE };

	GET(HouseTypeClass*, pCountry, EAX);

	const auto idx = pCountry->ArrayIndex2;

	//special cases
	if (idx == -2) {
		R->EAX(idx);
		return 0x6AA1CD;
	}

	if (idx == -3) {
		R->EAX(idx);
		return 0x6AA17D;
	}

	const auto pData = HouseTypeExtContainer::Instance.Find(pCountry);

	if (pData->ObserverFlagSHP)
	{
		R->ESI<SHPStruct*>(pData->ObserverFlagSHP);
		R->EAX<int>(pData->ObserverFlagYuriPAL ? 9 : 0);
		return DrawSHP;
	}
	else if (auto PCXSurface = pData->ObserverFlag.GetSurface())
	{
		GET(int, TLX, EDI);
		GET(int, TLY, EBX);
		RectangleStruct bounds = { TLX + ObserverFlagPCXX , TLY + ObserverFlagPCXY,
				ObserverFlagPCXWidth, ObserverFlagPCXHeight
		};

		PCX::Instance->BlitToSurface(&bounds, DSurface::Sidebar, PCXSurface, Drawing::ColorStructToWordRGB(Drawing::DefaultColors[6]));
	}

	return DontDraw;
}

DEFINE_HOOK(0x4E3560, Game_GetFlagSurface, 5)
{
	GET(int, n, ECX);
	//GET_STACK(DWORD, caller, 0x0);

	//if (Phobos::Otamaa::IsAdmin)
	//	Debug::Log(__FUNCTION__" Called From [0x%x] with idx [%d]\n", caller, n);

	if (n == -2)
	{
		//rani.pcx
		R->EAX(PCX::Instance->GetSurface(reinterpret_cast<const char*>(0x844AE8))); // special index
		return 0x4E3686;
	}

	if (n == -3)
	{
		//obsi.pcx
		R->EAX(PCX::Instance->GetSurface(reinterpret_cast<const char*>(0x844AEC))); // special index
		return 0x4E3686;
	}

	if (auto pHouse = HouseTypeClass::Array->GetItemOrDefault(n)) {
		if (auto pSurface = HouseTypeExtContainer::Instance.Find(pHouse)->FlagFile.GetSurface()) {
			R->EAX(pSurface);
			return 0x4E3686; //override result
		}
	}

	return 0x0; // handle check
}

DEFINE_HOOK(0x4E38A0, LoadPlayerCountryString, 5)
{
	GET(int, n, ECX);

	enum { NextCompare = 0x4E38BC, Neg2Result = 0x4E38A5 , RetResult = 0x4E39F1 };

	if(auto pHouse = HouseTypeClass::Array->GetItemOrDefault(n)) {
		R->EAX(HouseTypeExtContainer::Instance.Find(pHouse)->StatusText->Text);
		return RetResult; //replaced
	}

	return n == -2 ? Neg2Result : NextCompare; /// overriden
}

DEFINE_HOOK(0x553412, LoadProgressMgr_Draw_LSFile, 9)
{
	GET(int, n, EBX);
	enum { SwitchStatement = 0x553421, DefaultResult = 0x553416, RetResult = 0x55342C };

	if(auto pHouse = HouseTypeClass::Array->GetItemOrDefault(n)) {
		R->EDX(HouseTypeExtContainer::Instance.Find(pHouse)->LoadScreenBackground.data());
		return RetResult;//replaced
	}

	if (n == 0) {
		return DefaultResult; //USA
	}

	return SwitchStatement; //switch
}

DEFINE_HOOK(0x5536da, LoadProgressMgr_Draw_LSName, 9)
{
	GET(int, n, EBX);
	enum { SwitchStatement = 0x5536FB, DefaultResult = 0x5536DE, RetResult = 0x553820 };

	if(auto pHouse = HouseTypeClass::Array->GetItemOrDefault(n)){
		R->EDI(HouseTypeExtContainer::Instance.Find(pHouse)->LoadScreenName->Text);
		return RetResult;//replaced
	}

	if (n == 0) {
		return DefaultResult; //USA
	}

	return SwitchStatement;//switch
}

DEFINE_HOOK(0x553a05, LoadProgressMgr_Draw_LSSpecialName, 6)
{
	GET_STACK(int, n, 0x38);
	enum { SwitchStatement = 0x553A28, DefaultResult = 0x553A0D, RetResult = 0x553B3B };

	// any valid index will be override
	if (auto pHouse = HouseTypeClass::Array->GetItemOrDefault(n)) {
		R->EAX(HouseTypeExtContainer::Instance.Find(pHouse)->LoadScreenSpecialName->Text);
		return RetResult;
	}

	if(n == 0) // Index 0 is USA
		return DefaultResult;

	R->EAX(n);
	return SwitchStatement; //the default for switch statement is `null`
}

DEFINE_HOOK(0x553d06, LoadProgressMgr_Draw_LSBrief, 6)
{
	GET_STACK(int, n, 0x38);
	enum { SwitchStatement = 0x553D2B, DefaultResult = 0x553D0E, RetResult = 0x553E54 };

	// any valid index will be override
	if (auto pHouse = HouseTypeClass::Array->GetItemOrDefault(n)) {
		R->ESI(HouseTypeExtContainer::Instance.Find(pHouse)->LoadScreenBrief->Text);
		return RetResult;
	}

	if(n == 0) // Index 0 is USA
		return DefaultResult;

	R->EAX(n);
	return SwitchStatement; //the default for switch statement is `null`
}

DEFINE_HOOK(0x69B774, HTExt_PickRandom_Human, 5)
{
	R->EAX(HouseTypeExtData::PickRandomCountry());
	return 0x69B788;
}

DEFINE_HOOK(0x69B670, HTExt_PickRandom_AI, 5)
{
	R->EAX(HouseTypeExtData::PickRandomCountry());
	return 0x69B684;
}

bool KeepThisAlive(HouseClass* pHouse, TechnoClass* pTech, AbstractType what, uint8_t keep)
{
	const auto pType = pTech->GetTechnoType();

	const Nullable<bool>* result = &TechnoTypeExtContainer::Instance.Find(pType)->KeepAlive;

	bool ret = true;
	bool defaultKeppAlive = false;

	if (pType->Insignificant || pType->DontScore) {
		ret = false;
	} else {
		ret = true;
		defaultKeppAlive = what == BuildingClass::AbsID;
	}

	if (result->Get(defaultKeppAlive)) {
		const int add = 2 * keep - 1;
		HouseExtContainer::Instance.Find(pHouse)->KeepAliveCount += add;
		if (what == BuildingClass::AbsID)
			HouseExtContainer::Instance.Find(pHouse)->KeepAliveBuildingCount += add;
	}

	return ret;
}

// break short game ?
DEFINE_HOOK(0x4ff563, HouseClass_RegisterTechnoLoss_StatCounters_KeepAlive, 6)
{
	GET(TechnoClass*, pTech, ESI);
	GET(HouseClass*, pThis, EDI);
	const auto what = pTech->WhatAmI();
	const bool Keep = KeepThisAlive(pThis, pTech, what, 0);
	R->EAX(what);
	return Keep ? 0x4FF596 : 0x4FF6CE;
}

DEFINE_HOOK(0x4ff71b, HouseClass_RegisterTechnoGain_StatCounters_KeepAlive, 6)
{
	GET(TechnoClass*, pTech, ESI);
	GET(HouseClass*, pThis, EDI);
	const auto what = pTech->WhatAmI();
	const bool Keep = KeepThisAlive(pThis, pTech, what, 1u);
	R->EAX(what);
	return Keep ? 0x4FF748 : 0x4FF8C6;
}

DEFINE_HOOK(0x506306, HouseClass_FindPlaceToBuild_Evaluate, 6)
{
	GET(BuildingTypeClass*, pBuilding, EDX);
	auto pEXt = BuildingTypeExtContainer::Instance.Find(pBuilding);
	R->CL(pEXt->AIInnerBase.Get(pBuilding->CloakGenerator));
	return 0x50630C;
}

DEFINE_HOOK(0x4F94A5, HouseClass_BuildingUnderAttack, 6)
{
	GET(BuildingClass*, pSource, ESI);

	if (auto pWh = std::exchange(BuildingExtContainer::Instance.Find(pSource)->ReceiveDamageWarhead, nullptr))
	{
		if (!WarheadTypeExtContainer::Instance.Find(pWh)->Malicious)
		{
			return 0x4F95D4;
		}
	}

	return 0;
}

// drain affecting only the drained power plant
DEFINE_HOOK(0x508D32, HouseClass_UpdatePower_LocalDrain1, 5)
{
	GET(HouseClass*, pThis, ESI);
	GET(BuildingClass*, pBld, EDI);

	bool fullDrain = true;

	auto output = pBld->GetPowerOutput();

	if (output > 0)
	{
		auto pBldTypeExt = TechnoTypeExtContainer::Instance.Find(pBld->Type);
		auto pDrainTypeExt = TechnoTypeExtContainer::Instance.Find(pBld->DrainingMe->GetTechnoType());

		// local, if any of the participants in the drain is local
		if (pBldTypeExt->Drain_Local || pDrainTypeExt->Drain_Local)
		{
			fullDrain = false;

			// use the sign to select min or max.
			// 0 means no change (maximum of 0 and a positive value)
			auto limit = [](int value, int limit)
				{
					if (limit <= 0)
					{
						return MaxImpl(value, -limit);
					}
					else
					{
						return MinImpl(value, limit);
					}
				};

			// drains the entire output of this building by default
			// (the local output). building has the last word though.
			auto drain = limit(output, pDrainTypeExt->Drain_Amount);
			drain = limit(drain, pBldTypeExt->Drain_Amount);

			if (drain > 0)
			{
				pThis->PowerOutput -= drain;
			}
		}
	}

	return fullDrain ? 0 : 0x508D37;
}

// replaced the entire function, to have one centralized implementation
DEFINE_HOOK(0x5051E0, HouseClass_FirstBuildableFromArray, 5)
{
	GET(HouseClass*, pThis, ECX);
	GET_STACK(const DynamicVectorClass<BuildingTypeClass*>*const, pList, 0x4);
	R->EAX(HouseExtData::FindBuildable(pThis, pThis->Type->FindParentCountryIndex(), make_iterator(*pList)));
	return 0x505300;
}

DEFINE_HOOK(0x688B37, MPGameModeClass_CreateStartingUnits_B, 5)
{
	enum { hasBaseUnit = 0x688B75, hasNoBaseUnit = 0x688C09 };

	GET_STACK(HouseClass *, pHouse, 0x10);

	const int idxParent = pHouse->Type->FindParentCountryIndex();

	if(const auto Item =
		HouseExtData::FindOwned(pHouse , idxParent , make_iterator(RulesClass::Instance->BaseUnit))) {
		R->ESI<UnitClass*>((UnitClass*)Item->CreateObject(pHouse));
		R->EBP(0);
		R->EDI<HouseClass*>(pHouse);
		return hasBaseUnit;
	}

	Debug::Log(__FUNCTION__" House of country [%s] cannot build anything from [General]BaseUnit=.\n", pHouse->Type->ID);
	return hasNoBaseUnit;
}

// DEFINE_HOOK(0x5D721A, MPGameMode_CreateStartingUnits, 5)
// {
// 	GET_STACK(int, UnitCount, 0x40);
// 	GET_STACK(HouseClass*, pHouse, 0x4C);
//
// 	if(!UnitCount) {
// 		Debug::Log(__FUNCTION__" House of country [%s] cannot build anything from [General]BaseUnit=.\n", pHouse->Type->ID);
// 	}
//
// 	return 0;
// }

// DEFINE_HOOK(0x4F8440, HouseClass_Update_TogglePower, 5)
// {
// 	GET(HouseClass* const, pThis, ECX);
// 	AresHouseExt::UpdateTogglePower(pThis);
// 	return 0;
// }

DEFINE_HOOK(0x52267D, InfantryClass_GetDisguise_Disguise, 6)
{
	GET(HouseClass*, pHouse, EAX);

	if (auto pDisguiose = HouseExtData::GetDisguise(pHouse))
	{
		R->EAX<InfantryTypeClass*>(pDisguiose);
		return 0x5226B7;
	}
	else
	{
		return 0;
	}
}

DEFINE_HOOK_AGAIN(0x6F422F, Sides_Disguise, 6) // TechnoClass_Init
DEFINE_HOOK(0x5227A3, Sides_Disguise, 6) // InfantryClass_SetDefaultDisguise
{
	GET(HouseClass*, pHouse, EAX);
	InfantryClass* pThis = nullptr;
	DWORD dwReturnAddress = 0;

	if (R->Origin() == 0x5227A3)
	{
		pThis = R->ECX<InfantryClass*>();
		dwReturnAddress = 0x5227EC;
	}
	else if(R->Origin() == 0x6F422F)
	{
		GET(TechnoClass* , pTech , ESI);

		if(pTech->WhatAmI() != InfantryClass::AbsID) {
			return 0x0;
		}

		pThis = (InfantryClass*)pTech;
		dwReturnAddress = 0x6F4277;
	}

	if (pThis) {
		if (auto pDisguise = HouseExtData::GetDisguise(pHouse)) {
			pThis->Disguise = pDisguise;
			return dwReturnAddress;
		} else if (const auto pDefaultDisguiseType = TechnoTypeExtContainer::Instance.Find(pThis->Type)->DefaultDisguise.Get()){
			pThis->Disguise = pDefaultDisguiseType;
			return dwReturnAddress;
		}
		}

	return 0;
}

DEFINE_HOOK(0x4F8B08, HouseClass_Update_DamageDelay, 6)
{
	GET(HouseClass* const, pThis, ESI);

	// timer used unconditionally to trigger checks
	if (pThis->DamageDelayTimer.Completed())
	{
		auto const pType = pThis->Type;
		auto const pExt = HouseExtContainer::Instance.Find(pThis);
		auto const pTypeExt = HouseTypeExtContainer::Instance.Find(pType);
		auto const degrades = pTypeExt->Degrades.Get(pExt->Degrades.Get(!pType->MultiplayPassive));

		auto const pRules = RulesClass::Instance();
		pThis->DamageDelayTimer.Start(static_cast<int>(pRules->DamageDelay * 900));

		// damage is only applied conditionally
		auto const pRulesExt = RulesExtData::Instance();
		if (degrades && pRulesExt->DegradeEnabled && pThis->HasLowPower())
		{
			auto const defaultPercentage = pRulesExt->DegradePercentage.Get(pRules->ConditionYellow);

			for (auto const& pBld : pThis->Buildings)
			{
				auto const pBldType = pBld->Type;
				auto const pBldTypeExt = BuildingTypeExtContainer::Instance.Find(pBldType);

				// get the default amount for this building
				auto const& defaultAmount = pBldType->PowerDrain ?
					pRulesExt->DegradeAmountConsumer : pRulesExt->DegradeAmountNormal;

				// get the damage that applies to this building
				auto damage = pBldTypeExt->DegradeAmount.Get(defaultAmount);
				auto const percentage = pBldTypeExt->DegradePercentage.Get(defaultPercentage);

				if (damage > 0 && pBld->GetHealthPercentage() > percentage)
				{
					pBld->ReceiveDamage(&damage, 0, pRules->C4Warhead, nullptr, false, false, nullptr);
				}
			}
		}
	}

	// recreate the replaced instructions
	return pThis->IsCurrentPlayer() ? 0x4F8B14u : 0x4F8DB1u;
}

DEFINE_HOOK(0x508EBC, HouseClass_Radar_Update_CheckEligible, 6)
{
	enum { Eligible = 0, Jammed = 0x508F08 };

	GET(BuildingClass*, Radar, EAX);

	if(BuildingExtContainer::Instance.Find(Radar)->RegisteredJammers.empty() &&
			Radar->EMPLockRemaining <= 0) {
		return Eligible;
	}

	return Jammed;
}

static std::vector<BuildingTypeClass*> Eligible;

DEFINE_HOOK(0x4FE782, HouseClass_AI_BaseConstructionUpdate_PickPowerplant, 6)
{
	GET(HouseClass* const, pThis, EBP);
	auto const pExt = HouseTypeExtContainer::Instance.Find(pThis->Type);
	Eligible.clear();

	const auto it = pExt->GetPowerplants();

	for (auto const& pPower : it) {
		if (HouseExtData::PrereqValidate(pThis, pPower, false, true) == CanBuildResult::Buildable
			&& HouseExtData::PrerequisitesMet(pThis, pPower)
		) {
			Eligible.push_back(pPower);
		}
	}

	BuildingTypeClass* pResult = nullptr;
	if (!Eligible.empty()) {

		if ((int)Eligible.size() > 1)
			pResult = Eligible[ScenarioClass::Instance->Random.RandomFromMax((int)Eligible.size() - 1)];
		else
			pResult = Eligible[0];

	}
	else if (!it.empty())
	{
		pResult = it.at(0);
		Debug::Log("Country [%s] does not meet prerequisites for any possible power plant."
					"Fall back to the first one (%s).\n", pThis->Type->ID, pResult->ID);
	}
	else
	{
		Debug::FatalErrorAndExit(
			"Country [%s] did not find any powerplants it could construct!\n", pThis->Type->ID);
	}

	R->EDI(pResult);
	return 0x4FE893;
}

DEFINE_HOOK(0x4F8EBD, HouseClass_Update_HasBeenDefeated, 5)
{
	GET(HouseClass*, pThis, ESI);

	if (HouseExtContainer::Instance.Find(pThis)->KeepAliveCount) {
		return 0x4F8F87;
	}

	auto IsEligible = [pThis](FootClass* pFoot)
		{
			if (pFoot->Owner != pThis)
			{
				return false;
			}
			if (!pFoot->InLimbo)
			{
				return true;
			}
			return pFoot->ParasiteImUsing != nullptr;
		};

	if (GameModeOptionsClass::Instance->ShortGame)
	{
		for (auto pBaseUnit : RulesClass::Instance->BaseUnit)
		{
			if (pThis->OwnedUnitTypes[pBaseUnit->ArrayIndex])
			{
				return 0x4F8F87;
			}
		}
	}
	else
	{
		if (pThis->ActiveUnitTypes.Total)
		{
			for (auto pTechno : *UnitClass::Array)
			{
				if (IsEligible(pTechno))
				{
					return 0x4F8F87;
				}
			}
		}

		if (pThis->ActiveInfantryTypes.Total)
		{
			for (auto pTechno : *InfantryClass::Array)
			{
				if (IsEligible(pTechno))
				{
					return 0x4F8F87;
				}
			}
		}

		if (pThis->ActiveAircraftTypes.Total)
		{
			for (auto pTechno : *AircraftClass::Array)
			{
				if (IsEligible(pTechno))
				{
					return 0x4F8F87;
				}
			}
		}
	}

	pThis->DestroyAll();
	pThis->AcceptDefeat();

	return 0x4F8F87;
}

// this is checked right before the TeamClass is instantiated -
// it does not mean the AI will abandon this team if another team wants BuildLimit'ed units at the same time
DEFINE_HOOK(0x50965E, HouseClass_CanInstantiateTeam, 5)
{
	GET(DWORD, ptrTask, EAX);
	GET(DWORD, ptrOffset, ECX);

	ptrTask += (ptrOffset - 4); // pointer math!
	TaskForceEntryStruct* ptrEntry = reinterpret_cast<TaskForceEntryStruct*>(ptrTask); // evil! but works, don't ask me why

	GET(HouseClass*, Owner, EBP);
	enum { BuildLimitAllows = 0x5096BD, TryToRecruit = 0x509671, NoWay = 0x5096F1 } CanBuild = NoWay;

	if (TechnoTypeClass* Type = ptrEntry->Type)
	{
		if (Type->FindFactory(true, true, false, Owner))
		{
			if (RulesExtData::Instance()->AllowBypassBuildLimit[Owner->GetAIDifficultyIndex()])
			{
				CanBuild = BuildLimitAllows;
			} else {

				CanBuild = HouseExtData::BuildLimitRemaining(Owner, Type) >= ptrEntry->Amount ?
					BuildLimitAllows : TryToRecruit;
			}
		}
		else
		{
			CanBuild = TryToRecruit;
		}
	}
	return CanBuild;
}

DEFINE_HOOK_AGAIN(0x507DBA, HouseClass_BaseDefenses, 6) // HouseClass_PickAntiArmorDefense
DEFINE_HOOK_AGAIN(0x507FAA, HouseClass_BaseDefenses, 6) // HouseClass_PickAntiInfantryDefense
DEFINE_HOOK(0x507BCA, HouseClass_BaseDefenses, 6) // HouseClass_PickAntiAirDefense
{
	GET(HouseTypeClass*, pCountry, EAX);
	static DynamicVectorClass<BuildingTypeClass*> dummy;

	if (SideClass* pSide = SideClass::Array->GetItemOrDefault(pCountry->SideIndex))
	{
		auto it = SideExtContainer::Instance.Find(pSide)->GetBaseDefenses();
		dummy.Items = const_cast<BuildingTypeClass**>(it.begin());
		dummy.Count = dummy.Capacity = it.size();

		R->EBX(&dummy);
		return R->Origin() + 0x36;
	}

	return 0;
}

DEFINE_HOOK(0x505C95, HouseClass_GenerateAIBuildList_CountExtra, 7)
{
	GET(HouseClass* const, pThis, EBX);
	GET_STACK(int const, idxSide, 0x80);
	REF_STACK(DynamicVectorClass<BuildingTypeClass*>, BuildList, STACK_OFFS(0xA4, 0x78));

	auto const idxDifficulty = pThis->GetAIDifficultyIndex();
	auto& Random = ScenarioClass::Instance->Random;

	// optionally add the same buildings more than once, but ignore the
	// construction yard at index 0
	for (auto i = 1; i < BuildList.Count; ++i)
	{
		auto const pItem = BuildList[i];

		// only handle if occurs for the first time, otherwise we have an
		// escalating probability of desaster.
		auto const handled = make_iterator(BuildList.begin(), i);

		if (!handled.contains(pItem))
		{
			auto const pExt = BuildingTypeExtContainer::Instance.Find(pItem);
			if (pExt->AIBuildCounts.isset())
			{
				// fixed number of buildings, one minimum (exists already)
				auto count = MaxImpl(pExt->AIBuildCounts->at(idxDifficulty), 1);

				// random optional building counts
				if (pExt->AIExtraCounts.isset()) {
					auto const& max = pExt->AIExtraCounts->at(idxDifficulty);
					count += Random.RandomFromMax(MaxImpl(max, 0));
				}

				// account for the one that already exists
				for (auto j = 1; j < count; ++j)
				{
					auto const idx = Random.RandomRanged(
						i + 1, BuildList.Count);
					BuildList.AddItem(pItem);
					std::rotate(BuildList.begin() + idx, BuildList.end() - 1,
						BuildList.end());
				}
			}
		}
	}

	if (auto const pSide = SideClass::Array->GetItemOrDefault(idxSide))
	{
		auto const it = SideExtContainer::Instance.Find(pSide)->GetBaseDefenseCounts();

		if (idxDifficulty < it.size()) {
			R->EAX(it[idxDifficulty]);
			return 0x505CE9;
		} else {
			Debug::Log("WTF! vector has %u items, requested item #%u\n",
				it.size(), idxDifficulty);
		}
	}

	return 0;
}

// #917 - validate build list before it needs to be generated
DEFINE_HOOK(0x5054B0, HouseClass_GenerateAIBuildList_EnsureSanity, 6)
{
	GET(HouseClass* const, pThis, ECX);

	AresHouseExt::CheckBasePlanSanity(pThis);

	// allow the list to be generated even if it will crash the game - sanity
	// check will log potential problems and thou shalt RTFLog
	return 0;
}

DEFINE_HOOK(0x505360, HouseClass_PrerequisitesForTechnoTypeAreListed, 5)
{
	//GET(HouseClass *, pHouse, ECX);

	GET_STACK(TechnoTypeClass*, pItem, 0x4);
	GET_STACK(DynamicVectorClass<BuildingTypeClass*> *, pBuildingsToCheck, 0x8);
	GET_STACK(int, pListCount, 0xC);

	R->EAX(Prereqs::PrerequisitesListed(pBuildingsToCheck->Items, pListCount , pItem));

	return 0x505486;
}

DEFINE_HOOK(0x5F7900, ObjectTypeClass_FindFactory, 5)
{
	GET(TechnoTypeClass*, pThis, ECX);
	GET_STACK(HouseClass*, pHouse, 0x10);
	GET_STACK(bool, bSkipAircraft, 0x4);
	GET_STACK(bool, bRequirePower, 0x8);
	GET_STACK(bool, bCheckCanBuild, 0xC);

	const auto nBuffer = HouseExtData::HasFactory(
	pHouse,
	pThis,
	bSkipAircraft,
	bRequirePower,
	bCheckCanBuild,
	false);

	R->EAX(nBuffer.first >= NewFactoryState::Available_Alternative ?
		nBuffer.second : nullptr);

	return 0x5F7A89;
}

DEFINE_HOOK(0x6AB312, SidebarClass_ProcessCameoClick_Power, 6)
{
	GET(TechnoClass*, pFactoryObject, ESI);

	const auto nBuffer = HouseExtData::HasFactory(
		pFactoryObject->GetOwningHouse(), pFactoryObject->GetTechnoType(), false, true, false, true);

	if (nBuffer.first == NewFactoryState::Unpowered)
		return 0x6AB95A;

	R->EAX(nBuffer.second);
	return 0x6AB320;
}

DEFINE_HOOK(0x4F7870, HouseClass_CanBuild, 7)
{
	// int (TechnoTypeClass *item, bool BuildLimitOnly, bool includeQueued)
/* return
	 1 - cameo shown
	 0 - cameo not shown
	-1 - cameo greyed out
 */

	GET(HouseClass* const, pThis, ECX);
	GET_STACK(TechnoTypeClass* const, pItem, 0x4);
	GET_STACK(bool , buildLimitOnly, 0x8);
	GET_STACK(bool , includeInProduction, 0xC);
	//GET_STACK(DWORD , caller , 0x0);
	auto validationResult = HouseExtData::PrereqValidate(pThis, pItem, buildLimitOnly, includeInProduction);

	if(validationResult == CanBuildResult::Buildable) {
		validationResult = HouseExtData::BuildLimitGroupCheck(pThis, pItem, buildLimitOnly, includeInProduction);

		if (HouseExtData::ReachedBuildLimit(pThis, pItem, true))
			validationResult = CanBuildResult::TemporarilyUnbuildable;

	}

	R->EAX(validationResult);
	return 0x4F8361;
}

DEFINE_HOOK(0x50B370, HouseClass_ShouldDisableCameo, 5)
{
	GET(HouseClass*, pThis, ECX);
	GET_STACK(TechnoTypeClass*, pType, 0x4);

	bool result = HouseExtData::ShouldDisableCameo(pThis, pType);

	if(!result && HouseExtData::ReachedBuildLimit(pThis, pType, false)) {
		result = true;
	}

	R->EAX(result);
	return 0x50B669;
}

// HouseClass_Update_Factories_Queues_SkipBrokenDTOR
DEFINE_JUMP(LJMP, 0x50928C, 0x5092A3);

// reject negative indexes. if the index is the result of the function above, this
// catches all invalid cells. otherwise, the game can write of of bounds, which can
// set a field that is supposed to be a pointer, and crash when calling a virtual
// method on it. in worst case, this goes unnoticed.
DEFINE_HOOK(0x4FA2E0, HouseClass_SetThreat_Bounds, 0x7)
{
	//GET(HouseClass*, pThis, ESI);
	GET_STACK(int, index, 0x4);
	//GET_STACK(int, threat, 0x8);

	return index < 0 ? 0x4FA347u : 0;
}

DEFINE_HOOK(0x504796, HouseClass_AddAnger_MultiplayPassive, 0x6)
{
	GET_STACK(HouseClass*, pOtherHouse, 0x10);
	GET(HouseClass*, pThis, ECX);

	R->ECX(SessionClass::Instance->GameMode != GameMode::Campaign
		&& pOtherHouse->Type->MultiplayPassive ? 0x0 : pThis->AngerNodes.Count);

	return 0x50479C;
}

DEFINE_HOOK(0x509303, HouseClass_AllyWith_unused, 0x6)
{
	GET(HouseClass*, pThis, ESI);
	GET(HouseClass*, pThat, EAX);

	pThis->RadarVisibleTo.Add(pThat);
	return 0x509319;
}

// don't crash if you can't find a base unit
// I imagine we'll have a pile of hooks like this sooner or later
DEFINE_HOOK(0x4F65BF, HouseClass_CanAffordBase, 0x6)
{
	GET(UnitTypeClass*, pBaseUnit, ECX);

	if (pBaseUnit)
	{
		return 0;
	}
	//GET(HouseClass *, pHouse, ESI);
	//Debug::Log("AI House of country [%s] cannot build anything from [General]BaseUnit=.\n", pHouse->Type->ID);
	return 0x4F65DA;
}

DEFINE_HOOK(0x50067C, HouseClass_ClearFactoryCreatedManually, 0x6)
{
	GET(HouseClass*, pThis, ECX);
	pThis->InfantryType_53D1 = false;
	return 0x5006C0;
}

DEFINE_HOOK(0x5005CC, HouseClass_SetFactoryCreatedManually, 0x6)
{
	GET(HouseClass*, pThis, ECX);
	pThis->InfantryType_53D1 = true;
	return 0x500612;
}

DEFINE_HOOK(0x5007BE, HouseClass_SetFactoryCreatedManually2, 0x6)
{
	GET(HouseClass*, pThis, ECX);
	pThis->InfantryType_53D1 = R->EDX<bool>();
	return 0x50080D;
}

DEFINE_HOOK(0x455E4C, HouseClass_FindRepairBay, 0x9)
{
	GET(UnitClass* const, pUnit, ECX);
	GET(BuildingClass* const, pBay, ESI);

	auto const pType = pBay->Type;
	auto const pUnitType = pUnit->Type;

	const bool isNotAcceptable = (pUnitType->BalloonHover
		|| pType->Naval != pUnitType->Naval
		|| pType->Factory == AbstractType::AircraftType
		|| pType->Helipad
		|| pType->HoverPad && !RulesClass::Instance->SeparateAircraft);

	if (isNotAcceptable)
	{
		return 0x455EDE;
	}

	auto const response = pUnit->SendCommand(
		RadioCommand::QueryCanEnter, pBay);

	// original game accepted any valid answer as a positive one
	return response != RadioCommand::AnswerPositive ? 0x455EDEu : 0x455E5Du;
}

// fixes SWs not being available in campaigns if they have been turned off in a
// multiplayer mode
DEFINE_HOOK(0x5055D8, HouseClass_GenerateAIBuildList_SWAllowed, 0x5)
{
	auto const allowed = SessionClass::Instance->GameMode == GameMode::Campaign
		|| GameModeOptionsClass::Instance->SWAllowed;

	R->EAX(allowed);
	return 0x5055DD;
}

// #917 - stupid copying logic
/**
 * v2[0] = v1[0];
 * v2[1] = v1[1];
 * v2[2] = v1[2];
 * for(int i = 3; i < v1.Count; ++i) {
 *  v2[i] = v1[i];
 * }
 * care to guess what happens when v1.Count is < 3?
 *
 * fixed old fix, which was quite broken itself...
 */

DEFINE_HOOK(0x505B58, HouseClass_GenerateAIBuildList_SkipManualCopy, 0x6)
{
	REF_STACK(DynamicVectorClass<BuildingTypeClass*>, PlannedBase1, STACK_OFFS(0xA4, 0x90));
	REF_STACK(DynamicVectorClass<BuildingTypeClass*>, PlannedBase2, STACK_OFFS(0xA4, 0x78));
	PlannedBase2.SetCapacity(PlannedBase1.Capacity, nullptr);
	return 0x505C2C;
}

DEFINE_HOOK(0x505C34, HouseClass_GenerateAIBuildList_FullAutoCopy, 0x5)
{
	R->EDI(0);
	return 0x505C39;
}

// I am crying all inside
DEFINE_HOOK(0x505CF1, HouseClass_GenerateAIBuildList_PadWithN1, 0x5)
{
	REF_STACK(DynamicVectorClass<BuildingTypeClass*>, PlannedBase2, STACK_OFFS(0xA4, 0x78));
	GET(int, DefenseCount, EAX);
	while (PlannedBase2.Count <= 3)
	{
		PlannedBase2.AddItem(reinterpret_cast<BuildingTypeClass*>(-1));
		--DefenseCount;
	}
	R->EDI(DefenseCount);
	R->EBX(-1);
	return (DefenseCount > 0) ? 0x505CF6u : 0x505D8Du;
}

// #1369308: if still charged it hasn't fired.
// more efficient place would be 4FAEC9, but this is global
DEFINE_HOOK(0x4FAF2A, HouseClass_SWDefendAgainst_Aborted, 0x8)
{
	GET(SuperClass*, pSW, EAX);
	return (pSW && !pSW->IsCharged) ? 0x4FAF32 : 0x4FB0CF;
}

// restored from TS
DEFINE_HOOK(0x4F9610, HouseClass_GiveTiberium_Storage, 0xA)
{
	GET(HouseClass* const, pThis, ECX);
	GET_STACK(float, amount, 0x4);
	GET_STACK(int const, idxType, 0x8);

	pThis->SiloMoney += int(amount * 5.0);

	if (SessionClass::Instance->GameMode == GameMode::Campaign || pThis->IsHumanPlayer)
	{
		// don't change, old values are needed for silo update
		const double lastStorage = (HouseExtContainer::Instance.Find(pThis)->TiberiumStorage.GetAmounts());
		const auto lastTotalStorage = pThis->TotalStorage;
		const auto curStorage = (double)lastTotalStorage - lastStorage;
		double rest = 0.0;

		// this is the upper limit for stored tiberium
		if (amount > curStorage) {
			rest = amount - curStorage;
			amount = float(curStorage);
		}

		// go through all buildings and fill them up until all is in there
		for (auto const& pBuilding : pThis->Buildings)
		{
			if (amount <= 0.0) {
				break;
			}

			auto const storage = pBuilding->Type->Storage;
			if (pBuilding->IsOnMap && storage > 0)
			{
				auto storage_ = &TechnoExtContainer::Instance.Find(pBuilding)->TiberiumStorage;
				// put as much tiberium into this silo
				double freeSpace = (double)storage - storage_->GetAmounts();

				if (freeSpace > 0.0) {
					if (freeSpace > amount) {
						freeSpace = amount;
					}

					storage_->IncreaseAmount((float)freeSpace, idxType);
					HouseExtContainer::Instance.Find(pThis)->TiberiumStorage.IncreaseAmount((float)freeSpace, idxType);
					amount -= (float)freeSpace;
				}
			}
		}

		amount += (float)rest;

		//no free space , just give the money ,..
		if(amount > 0.0) {
			auto const pTib = TiberiumClass::Array->Items[idxType];
			pThis->Balance += int(amount * pTib->Value * pThis->Type->IncomeMult);
		}
		// redraw silos
		pThis->UpdateAllSilos((int)lastStorage, lastTotalStorage);
	}
	else
	{
		// just add the money. this is the only original YR logic
		auto const pTib = TiberiumClass::Array->Items[idxType];
		pThis->Balance += int(amount * pTib->Value * pThis->Type->IncomeMult);
	}

	return 0x4F9664;
}

DEFINE_HOOK(0x4F62FF, HouseClass_CTOR_FixNameOverflow, 6)
{
	GET(HouseClass*, H, EBP);
	GET_STACK(HouseTypeClass*, Country, 0x48);

	PhobosCRT::wstrCopy(H->UIName, Country->UIName);

	return 0x4F6312;
}

DEFINE_HOOK(0x4F645F, HouseClass_CTOR_FixSideIndices, 5)
{
	GET(HouseClass*, pHouse, EBP);
	if (HouseTypeClass* pCountry = pHouse->Type)
	{
		if (strcmp(pCountry->ID, GameStrings::Neutral()) &&
			strcmp(pCountry->ID, GameStrings::Special()))
		{
			pHouse->SideIndex = pCountry->SideIndex;
		}
	}
	return 0x4F6490;
}

DEFINE_HOOK(0x50BEB0, HouseClass_GetCostMult, 6)
{
	GET(HouseClass*, pThis, ECX);
	GET_STACK(TechnoTypeClass*, pType, 0x4);

	double nVal = 1.0;
	double nDefVal = 1.0;

	switch (pType->WhatAmI())
	{
	case AbstractType::AircraftType:
	{
		nVal = 1.0 - pThis->CostAircraftMult;
		break;
	}
	case AbstractType::BuildingType:
	{
		nVal = 1.0 - (static_cast<BuildingTypeClass*>(pType)->BuildCat == BuildCat::Combat ? pThis->CostDefensesMult : pThis->CostBuildingsMult);
		break;
	}
	case AbstractType::InfantryType:
	{
		nVal = 1.0 - pThis->CostInfantryMult;
		break;
	}
	case AbstractType::UnitType:
	{
		nVal = 1.0 - (static_cast<UnitTypeClass*>(pType)->ConsideredAircraft ? pThis->CostAircraftMult : pThis->CostUnitsMult);
		break;
	}
	}

	auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	auto nResult = (nDefVal - nVal * pTypeExt->FactoryPlant_Multiplier.Get());

	__asm { fld nResult };
	return 0x50BF1E;
}

#include <CCToolTip.h>

DEFINE_HOOK(0x509140, HouseClass_Update_Factories_Queues, 5)
{
	GET(HouseClass*, H, ECX);
	GET_STACK(AbstractType, nWhat, 0x4);
	GET_STACK(bool, bIsNaval, 0x8);
	GET_STACK(BuildCat, nBuildCat, 0xC);

	if (nWhat == AbstractType::BuildingType && nBuildCat != BuildCat::DontCare)
		H->Update_FactoriesQueues(nWhat, bIsNaval, nBuildCat);

	CCToolTip::Bound = 1;
	return 0;
}

DEFINE_HOOK(0x508C7F, HouseClass_UpdatePower_Auxiliary, 6)
{
	GET(HouseClass*, pThis, ESI);

	const auto curAux = HouseExtContainer::Instance.Find(pThis)->AuxPower;

	int AuxPowerOutput = 0;
	if (curAux >= 0)
		AuxPowerOutput = curAux;
	pThis->PowerOutput = AuxPowerOutput;

	int AuxPowerDrain = 0;
	if (AuxPowerDrain <= 0)
		AuxPowerDrain = -curAux;
	pThis->PowerDrain = AuxPowerDrain;

	return 0x508C8B;
}

// #917 - handle the case of no shipyard gracefully
DEFINE_HOOK(0x50610E, HouseClass_FindPositionForBuilding_FixShipyard, 7)
{
	GET(BuildingTypeClass*, pShipyard, EAX);

	if (pShipyard)
	{
		R->ESI<int>(pShipyard->GetFoundationWidth() + 2);
		R->EAX<int>(pShipyard->GetFoundationHeight(false));
		return 0x506134;
	}

	return 0x5060CE;
}

DEFINE_HOOK(0x4FC731, HouseClass_DestroyAll_ReturnStructures, 7)
{
	GET_STACK(HouseClass*, pThis, STACK_OFFS(0x18, 0x8));
	GET(TechnoClass*, pTechno, ESI);

	// do not return structures in campaigns
	if (SessionClass::Instance->IsCampaign()) {
		return 0;
	}

	// check whether this is a building
	if (auto pBld = specific_cast<BuildingClass*>(pTechno))
	{
		auto pInitialOwner = pBld->InitialOwner;

		// was the building owned by a neutral country?
		if (!pInitialOwner || pInitialOwner->Type->MultiplayPassive)
		{
			auto pExt = BuildingTypeExtContainer::Instance.Find(pBld->Type);

			auto occupants = pBld->GetOccupantCount();
			auto canReturn = (pInitialOwner != pThis) || occupants > 0;

			if (canReturn && pExt->Returnable.Get(RulesExtData::Instance()->ReturnStructures))
			{
				// this may change owner
				if (occupants) {
					pBld->KillOccupants(nullptr);
				}

				// don't do this when killing occupants already changed owner
				if (pBld->GetOwningHouse() == pThis)
				{

					// fallback to first civilian side house, same logic SlaveManager uses
					if (!pInitialOwner)
					{
						pInitialOwner = HouseClass::FindCivilianSide();
					}

					// give to other house and disable
					if (pInitialOwner && pBld->SetOwningHouse(pInitialOwner, false))
					{
						pBld->Guard();

						if (pBld->Type->NeedsEngineer)
						{
							pBld->HasEngineer = false;
							pBld->DisableStuff();
						}

						return 0x4FC770;
					}
				}
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x4FB2FD, HouseClass_UnitFromFactory_BuildingSlam, 6)
{
	GET(BuildingClass*, pThis, ESI);

	VocClass::PlayGlobal(BuildingTypeExtContainer::Instance.Find(pThis->Type)->SlamSound.
		Get(RulesClass::Instance->BuildingSlam), Panning::Center, 1.0, 0);

	return 0x4FB319;
}

//0x4F8F54
DEFINE_HOOK(0x4F8F54, HouseClass_Update_SlaveMinerCheck, 6)
{
	GET(HouseClass*, pThis, ESI);
	GET(int, n, EDI);

	for (auto const& ref : RulesClass::Instance->BuildRefinery) {
		//new sane way to find a slave miner
		if (ref && ref->SlavesNumber > 0)
			n += pThis->ActiveBuildingTypes.GetItemCount(ref->ArrayIndex);
	}

	R->EDI(n);
	return 0x4F8F75;
}

//0x4F8C97
DEFINE_HOOK(0x4F8C97, HouseClass_Update_BuildConst, 6)
{
	GET(HouseClass*, pThis, ESI);

	enum { NotifyLowPower = 0x4F8D02, Skip = 0x4F8DB1 };

	// disable FSW on low power
	AresHouseExt::SetFirestormState(pThis, false);

	// should play low power EVA for more than three BuildConst items
	for (auto const& pItem : RulesClass::Instance->BuildConst) {
		if (pItem && pThis->ActiveBuildingTypes.GetItemCount(pItem->ArrayIndex) > 0) {
			return NotifyLowPower;
		}
	}

	return Skip;
}

// there is actually an SpeakDelay
// dunno atm
//DEFINE_HOOK(0x4F8B3C, HouseClass_Update_Annouces, 0x6) {
//	GET(HouseClass* const, pThis, ESI);
//
//		pThis->Buildings.for_each([pThis](BuildingClass* pBld) {
//		if(pBld->Type->Radar && pThis->SpeakMaxedDelayTimer.Expired()){
//			if (!BuildingExtContainer::Instance.Find(pBld)->RegisteredJammers.empty()) {
//				VoxClass::Play("EVA_RadarJammed");
//				const int time = GameOptionsClass::Instance->GetAnimSpeed(
//						int(RulesClass::Instance->SpeakDelay * 900.0));
//				pThis->SpeakMaxedDelayTimer.Start(time);
//			}
//		}
//	});
//
//	return 0x0;
//}

// play this annoying message every now and then
DEFINE_HOOK(0x4F8C23, HouseClass_Update_SilosNeededEVA, 5)
{
	GET(HouseClass* const, pThis, ESI);

	VoxClass::Play("EVA_SilosNeeded");

	if (const CSFText& Message = RulesExtData::Instance()->MessageSilosNeeded) {
		Message.PrintAsMessage(pThis->ColorSchemeIndex);
	}

	return 0;
}

DEFINE_HOOK(0x500CC5, HouseClass_InitFromINI_FixBufferLimits, 6)
{
	GET(HouseClass*, H, EBX);

	if (H->UINameString[0])
	{
		const wchar_t* str = StringTable::LoadString(H->UINameString);
		PhobosCRT::wstrCopy(H->UIName, str);
	}
	else
	{
		PhobosCRT::wstrCopy(H->UIName, H->Type->UIName);
	}

	//dropping this here, should be fine
	const auto pParent = H->Type->FindParentCountry();
	HouseExtContainer::Instance.Find(H)->FactoryOwners_GatheredPlansOf.insert(pParent ? pParent : H->Type);

	return 0x500D0D;
}