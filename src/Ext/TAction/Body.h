#pragma once

#include <TActionClass.h>
#include <TriggerClass.h>

#include <Utilities/Savegame.h>
#include <Helpers/Template.h>
#include <Utilities/VectorHelper.h>
#include <Utilities/PhobosMap.h>

class HouseClass;
class ObjectClass;
class TriggerClass;
class HouseClass;
class CellStruct;
enum class PhobosTriggerAction : unsigned int
{
	MakeAllyOneWay = 490,
	MakeEnemyOneWay = 491,
	AllAssignMission = 492,
	DeleteObject = 493,
	DisableAllyReveal = 494,
	EnableAllyReveal = 495,
	MakeElite = 496,
	DisableShortGame = 497,
	EnableShortGame = 498,
	GiveCredits = 499,

	SaveGame = 500,
	EditVariable = 501,
	GenerateRandomNumber = 502,
	PrintVariableValue = 503,
	BinaryOperation = 504,
	RunSuperWeaponAtLocation = 505,
	RunSuperWeaponAtWaypoint = 506,

	//#1144
	DumpVariables = 507,

	//#1266 , modified number
	PrintMessageRemainingTechnos = 508,

	//#844
	ToggleMCVRedeploy = 510,

	//#1164
	UndeployToWaypoint = 511,

	SetDropCrate = 600, // Only change this number if the PR is merged into develop!

	//ES
	SetTriggerTechnoVeterancy = 700,
	TransactMoneyFor = 701,
	SetAIMode = 703,
	DrawAnimWithin = 704,
	SetAllOwnedFootDestinationTo = 705,
	FlashTechnoFor = 713,
	UnInitTechno = 716,
	GameDeleteTechno = 717,
	LightningStormStrikeAtObject = 720,

	//#1549
	ResetHateValue = 606,

	//
	EditAngerNode = 607,
	ClearAngerNode = 608,
	SetForceEnemy = 609,

	CreateBannerLocal = 800, // any banner w/ local variable
	CreateBannerGlobal = 801, // any banner w/ global variable
	DeleteBanner = 802,

	//#620
	MessageForSpecifiedHouse = 9931,

	//#658
	RandomTriggerPut = 12000,
	RandomTriggerRemove = 12001,
	RandomTriggerEnable = 12002,
	ScoreCampaignText = 19000,
	ScoreCampaignTheme = 19001,
	SetNextMission = 19002 ,

	//DrawLaserBetweenWeaypoints = 9940,
	//AdjustLighting = 505,

	count
};

class TActionExtData
{
public:

	static void RecreateLightSources();
	static bool Occured(TActionClass* pThis, ActionArgs const& args , bool& bHandled);
	static bool RunSuperWeaponAt(TActionClass* pThis, int X, int Y);

#define ACTION_FUNC(name) \
	static bool name(TActionClass* pThis, HouseClass* pHouse, \
		ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)

	ACTION_FUNC(PlayAudioAtRandomWP);
	ACTION_FUNC(SaveGame);
	ACTION_FUNC(EditVariable);
	ACTION_FUNC(GenerateRandomNumber);
	ACTION_FUNC(PrintVariableValue);
	ACTION_FUNC(BinaryOperation);
	//ACTION_FUNC(AdjustLighting);

	ACTION_FUNC(RunSuperWeaponAtLocation);
	ACTION_FUNC(RunSuperWeaponAtWaypoint);

	ACTION_FUNC(DrawLaserBetweenWaypoints);

	ACTION_FUNC(RandomTriggerPut);
	ACTION_FUNC(RandomTriggerEnable);
	ACTION_FUNC(RandomTriggerRemove);

	ACTION_FUNC(ScoreCampaignText);
	ACTION_FUNC(ScoreCampaignTheme);
	ACTION_FUNC(SetNextMission);
	ACTION_FUNC(DumpVariables);
	ACTION_FUNC(ToggleMCVRedeploy);

	ACTION_FUNC(MessageForSpecifiedHouse);

	ACTION_FUNC(SetTriggerTechnoVeterancy);
	ACTION_FUNC(TransactMoneyFor);
	ACTION_FUNC(SetAIMode);
	ACTION_FUNC(DrawAnimWithin);
	ACTION_FUNC(SetAllOwnedFootDestinationTo);
	ACTION_FUNC(FlashTechnoFor);
	ACTION_FUNC(UnInitTechno);
	ACTION_FUNC(GameDeleteTechno);
	ACTION_FUNC(LightningStormStrikeAtObject);

	ACTION_FUNC(UndeployToWaypoint);

	ACTION_FUNC(PrintMessageRemainingTechnos);

	ACTION_FUNC(SetDropCrate);

	ACTION_FUNC(ResetHateValue);

	ACTION_FUNC(EditAngerNode);
	ACTION_FUNC(ClearAngerNode);
	ACTION_FUNC(SetForceEnemy);

	ACTION_FUNC(CreateBannerGlobal);
	ACTION_FUNC(CreateBannerLocal);
	ACTION_FUNC(DeleteBanner);

	ACTION_FUNC(GiveCredits);
	ACTION_FUNC(EnableShortGame);
	ACTION_FUNC(DisableShortGame);
	ACTION_FUNC(BlowupHouse);
	ACTION_FUNC(MakeElite);
	ACTION_FUNC(EnableAllyReveal);
	ACTION_FUNC(DisableAllyReveal);
	ACTION_FUNC(DeleteObject);
	ACTION_FUNC(AllAssignMission);
	ACTION_FUNC(MakeAllyOneWay);
	ACTION_FUNC(MakeEnemyOneWay);

	ACTION_FUNC(CreateBuildingAt);
	ACTION_FUNC(ChangeHouse);
	ACTION_FUNC(AllChangeHouse);
#undef ACTION_FUNC

	static PhobosMap<int, std::vector<TriggerClass*>> RandomTriggerPool;

	static void Clear() {
		RandomTriggerPool.clear();
	}

	static void InvalidatePointer(AbstractClass* ptr, bool bRemoved) {
		for (auto& nMap : RandomTriggerPool) {
			if (bRemoved) {
				fast_remove_if(nMap.second, [ptr](auto _el) { return  ptr == _el; });
			}
		}
	}

	static bool LoadGlobals(PhobosStreamReader& Stm) {
		Stm.Process(RandomTriggerPool);
		return Stm.Success();
	}

	static bool SaveGlobals(PhobosStreamWriter& Stm){
		Stm.Process(RandomTriggerPool);
		return Stm.Success();
	}
};


class NOVTABLE FakeTActionClass : public TActionClass
{
public:

	bool _OperatorBracket(HouseClass* pTargetHouse, ObjectClass* pSourceObject, TriggerClass* pTrigger, CellStruct* plocation);

}; static_assert(sizeof(FakeTActionClass) == 0x94, "Invalid Size !");