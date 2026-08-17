#pragma once

#include <TActionClass.h>
#include <TriggerClass.h>

#include <Helpers/Template.h>

#include <Utilities/Savegame.h>
#include <Utilities/VectorHelper.h>
#include <Utilities/PhobosMap.h>
#include <Utilities/Enum.h>
#include <Utilities/Container.h>

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

	//
	AdjustHouseModifier = 509,

	ToggleMCVRedeploy = 510,
	UndeployToWaypoint = 511,

	SetFollowsIndexForVehicle = 512,
	AttachSoundToObjects = 513,
	RemoveSoundFromObjects = 514,

	SetWaypointTextBoxByType = 549,
	SetWaypointTextBoxByData = 550,

	ClearWaypointTextBox = 551,
	ClearAllWaypointTextBoxs = 552,

	BindAllTeamMemberToTag = 553,
	BindOwnerTeamMemberToTag = 554,
	BindAllTechnoTypeToTag = 555,
	BindOwnerTechnoTypeToTag = 556,

	GiveHouseMoney = 557,
	TakeHouseMoney = 558,
	SetHouseMoney = 559,

	AddBaseNodeForHouseAtWaypoint = 560,
	RemoveAllBaseNodeForHouseAtWaypoint = 561,
	RemoveBaseNodesOfBuildingTypeForHouse = 562,
	DestroyAllTagByTagTypeSafely = 563,
	BindTagToTechnoTypeAtWaypoint = 564,
	BindTagToTechnoTypeOfHouseAtWaypoint = 565,
	BindTagToSpecificTechnoTypeWithinWaypointRange = 566,
	BindTagToSpecificTechnoTypeOfSpecificOwnerWithinWaypointRange = 567,
	BindTagToAllTechnoTypesWithinWaypointRange = 568,
	BindTagToAllTechnoTypesOfSpecificOwnerWithinWaypointRange = 569,
	UnifyAllInstancesOfSameTagType = 570,
	SetRecruitableForFoot = 571,
	BindTagsToAllTechTypesInWaypointRangeExceptSpecified = 572,
	BindTagsToAllTechTypesOfTriggerOwnerInWaypointRangeExceptSpecified = 573,
	UpdateAllBuildingAnims = 574,
	UpdateAssociatedBuildingsAnims = 575,
	UpdateOwnerBuildingsAnimations = 576,
	CreateTeamConsideringLimits = 577,
	RecruitNearbyFootToTeam = 578,
	SetUnitTextBoxByTriggerType = 579,
	SetUnitTextBoxByTriggerData = 580,
	SetUnitTextBoxByTeamType = 581,
	SetUnitTextBoxByTeamData = 582,
	ClearUnitTextBoxByType = 583,
	ClearUnitTextBoxByTag = 584,
	ClearUnitTextBoxByTechType = 585,
	ClearUnitTextBoxByHouseAndType = 586,
	ClearUnitTextBoxByTeam = 587,
	ClearAllUnitTextBoxs = 588,
	ClearAllTextBoxs = 589,

	SetWaypointChoiceBox = 590,
	SetScreenChoiceBox = 591,
	ClearChoiceBoxByID = 592,
	ClearAllChoiceBoxs = 593,

	SetDropCrate = 600,

	ClearScript = 650,
	CopyScript = 651,

	ModifyScriptByParam = 652,
	ModifyScriptByLocalVar = 653,
	ModifyScriptByGlobalVar = 654,

	RebindTeamTypeScript = 655,
	ResetTeamTypeScript = 656,
	ResetAllTeamTypeScripts = 657,
	RestoreScriptContent = 658,
	RestoreAllScriptContents = 659,
	SeekTeamTypeScript = 660,
	SetTeamTypeMaxValue = 661,

	SetOverParTitle = 665,
	SetOverParMessage = 666,
	SetUnderParTitle = 667,
	SetUnderParMessage = 668,

	ClearTaskForce = 670,
	CopyTaskForce = 671,
	ModifyTaskForceEntry = 672,
	RebindTeamTypeTaskForce = 673,
	RestoreTaskForce = 674,
	RestoreAllTaskForces = 675,
	ResetTeamTypeTaskForce = 676,
	ResetAllTeamTypeTaskForces = 677,
	RecruitGroupToTeam = 678,
	UndeployHouseUnits = 679,
	SetParTimeEasy = 669,
	SetParTimeMedium = 680,
	SetParTimeDifficult = 681,

	//#1549
	ResetHateValue = 605,

	//
	EditAngerNode = 606,
	ClearAngerNode = 607,
	SetForceEnemy = 608,
	SetFreeRadar = 609,
	SetTeamDelay = 610,
	SetNextScanario = 611,

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

	CreateBannerLocal = 800, // any banner w/ local variable
	CreateBannerGlobal = 801, // any banner w/ global variable
	DeleteBanner = 802,

	//#2270
	OpenDropshipLoadoutWindow = 900,
	CreateDropshipLoadoutTransport = 901,

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
	
	// PR #1932 , adjusted +2
	WinByID = 19003,
	LoseByID = 19004,
	ProductionBeginsByID = 19005,
	AllToHuntByID = 19006,
	PlayMovieByID = 19007,
	FireSaleByID = 19008,
	AutocreateBeginsByID = 19009,
	ChangeHouseByID = 19010,
	PlayMusicThemeByID = 19011,
	AddOneTimeSuperWeaponByID = 19012,
	AddRepeatingSuperWeaponByID = 19013,
	AllChangeHouseByID = 19014,
	MakeAllyByID = 19015,
	MakeEnemyByID = 19016,
	PlayAnimAtByID = 19017,
	DoExplosionAtByID = 19018,
	CreateVoxelAnimByID = 19019,
	AITriggersBeginByID = 19020,
	AITriggersStopByID = 19021,
	ParticleAnimByID = 19022,
	MakeHouseCheerByID = 19023,
	DestroyAllByID = 19024,
	DestroyAllBuildingsByID = 19025,
	DestroyAllLandUnitsByID = 19026,
	DestroyAllNavalUnitsByID = 19027,
	MindControlBaseByID = 19028,
	RestoreMindControlledBaseByID = 19029,
	RestoreStartingUnitsByID = 19030,
	RestoreStartingBuildingsByID = 19031,

	count
};

class TActionExtData
{
public:
	using base_type = TActionClass;
	static COMPILETIMEEVAL const char* ClassName = "TActionExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "TActionClass";
	static COMPILETIMEEVAL DWORD Canary = 0x0F4E8D87;

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
	
	ACTION_FUNC(SetFollowsIndexForVehicle);

	ACTION_FUNC(PrintMessageRemainingTechnos);

	ACTION_FUNC(SetDropCrate);

	ACTION_FUNC(ResetHateValue);

	ACTION_FUNC(EditAngerNode);
	ACTION_FUNC(ClearAngerNode);
	ACTION_FUNC(SetForceEnemy);
	ACTION_FUNC(SetFreeRadar);
	ACTION_FUNC(SetTeamDelay);
	ACTION_FUNC(SetNextScanario);

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

	ACTION_FUNC(AdjustHouseModifier);

	ACTION_FUNC(ActivateFirestorm);
	ACTION_FUNC(DeactivateFirestorm);
	ACTION_FUNC(AuxiliaryPower);
	ACTION_FUNC(KillDriversOf);
	ACTION_FUNC(SetEVAVoice);
	ACTION_FUNC(SetGroup);

	//TODO : re-eval
	ACTION_FUNC(LauchhNuke);

	//TODO : re-eval
	ACTION_FUNC(LauchhChemMissile);
	ACTION_FUNC(LightstormStrike);
	ACTION_FUNC(MeteorStrike);
	ACTION_FUNC(PlayAnimAt);
	ACTION_FUNC(DoExplosionAt);
	ACTION_FUNC(EnableTrigger);

	ACTION_FUNC(CreateDropshipLoadoutTransport);
	ACTION_FUNC(OpenDropshipLoadoutWindow);

	ACTION_FUNC(SetWaypointTextBoxByType);
	ACTION_FUNC(SetWaypointTextBoxByData);
	ACTION_FUNC(ClearWaypointTextBox);
	ACTION_FUNC(ClearAllWaypointTextBoxs);
	ACTION_FUNC(BindAllTeamMemberToTag);
	ACTION_FUNC(BindOwnerTeamMemberToTag);
	ACTION_FUNC(BindAllTechnoTypeToTag);
	ACTION_FUNC(BindOwnerTechnoTypeToTag);
	ACTION_FUNC(GiveHouseMoney);
	ACTION_FUNC(TakeHouseMoney);
	ACTION_FUNC(SetHouseMoney);
	ACTION_FUNC(AddBaseNodeForHouseAtWaypoint);
	ACTION_FUNC(RemoveAllBaseNodeForHouseAtWaypoint);
	ACTION_FUNC(RemoveBaseNodesOfBuildingTypeForHouse);
	ACTION_FUNC(DestroyAllTagByTagTypeSafely);
	ACTION_FUNC(BindTagToTechnoTypeAtWaypoint);
	ACTION_FUNC(BindTagToTechnoTypeOfHouseAtWaypoint);
	ACTION_FUNC(BindTagToSpecificTechnoTypeWithinWaypointRange);
	ACTION_FUNC(BindTagToSpecificTechnoTypeOfSpecificOwnerWithinWaypointRange);
	ACTION_FUNC(BindTagToAllTechnoTypesWithinWaypointRange);
	ACTION_FUNC(BindTagToAllTechnoTypesOfSpecificOwnerWithinWaypointRange);
	ACTION_FUNC(UnifyAllInstancesOfSameTagType);
	ACTION_FUNC(SetRecruitableForFoot);
	ACTION_FUNC(BindTagsToAllTechTypesInWaypointRangeExceptSpecified);
	ACTION_FUNC(BindTagsToAllTechTypesOfTriggerOwnerInWaypointRangeExceptSpecified);
	ACTION_FUNC(UpdateAllBuildingAnims);
	ACTION_FUNC(UpdateAssociatedBuildingsAnims);
	ACTION_FUNC(UpdateOwnerBuildingsAnimations);
	ACTION_FUNC(CreateTeamConsideringLimits);
	ACTION_FUNC(RecruitNearbyFootToTeam);
	ACTION_FUNC(SetUnitTextBoxByTriggerType);
	ACTION_FUNC(SetUnitTextBoxByTriggerData);
	ACTION_FUNC(SetUnitTextBoxByTeamType);
	ACTION_FUNC(SetUnitTextBoxByTeamData);
	ACTION_FUNC(ClearUnitTextBoxByType);
	ACTION_FUNC(ClearUnitTextBoxByTag);
	ACTION_FUNC(ClearUnitTextBoxByTechType);
	ACTION_FUNC(ClearUnitTextBoxByHouseAndType);
	ACTION_FUNC(ClearUnitTextBoxByTeam);
	ACTION_FUNC(ClearAllUnitTextBoxs);
	ACTION_FUNC(ClearAllTextBoxs);

	ACTION_FUNC(SetWaypointChoiceBox);
	ACTION_FUNC(SetScreenChoiceBox);
	ACTION_FUNC(ClearChoiceBoxByID);
	ACTION_FUNC(ClearAllChoiceBoxs);

	ACTION_FUNC(ClearScript);
	ACTION_FUNC(CopyScript);
	ACTION_FUNC(ModifyScriptByParam);
	ACTION_FUNC(ModifyScriptByLocalVar);
	ACTION_FUNC(ModifyScriptByGlobalVar);
	ACTION_FUNC(RebindTeamTypeScript);
	ACTION_FUNC(ResetTeamTypeScript);
	ACTION_FUNC(ResetAllTeamTypeScripts);
	ACTION_FUNC(RestoreScriptContent);
	ACTION_FUNC(RestoreAllScriptContents);
	ACTION_FUNC(SeekTeamTypeScript);
	ACTION_FUNC(SetTeamTypeMaxValue);

	ACTION_FUNC(ClearTaskForce);
	ACTION_FUNC(CopyTaskForce);
	ACTION_FUNC(ModifyTaskForceEntry);
	ACTION_FUNC(RebindTeamTypeTaskForce);
	ACTION_FUNC(RestoreTaskForce);
	ACTION_FUNC(RestoreAllTaskForces);
	ACTION_FUNC(ResetTeamTypeTaskForce);
	ACTION_FUNC(ResetAllTeamTypeTaskForces);
	ACTION_FUNC(RecruitGroupToTeam);
	ACTION_FUNC(UndeployHouseUnits);

	ACTION_FUNC(SetOverParTitle);
	ACTION_FUNC(SetOverParMessage);
	ACTION_FUNC(SetUnderParTitle);
	ACTION_FUNC(SetUnderParMessage);
	ACTION_FUNC(SetParTimeEasy);
	ACTION_FUNC(SetParTimeMedium);
	ACTION_FUNC(SetParTimeDifficult);

#undef ACTION_FUNC

	static bool Retint(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* location, DefaultColorList col);
	static bool Execute(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* location, bool& ret);
	static PhobosMap<int, std::vector<TriggerClass*>> RandomTriggerPool;
};

class TActionExtContainer final //: 
	//public Container<TActionExtData>, 
	//public ContainerSaveLoad<TActionExtContainer, TActionExtData>
{
public:
	static COMPILETIMEEVAL const char* ClassName = "TActionExtContainer";

	bool LoadGlobal(PhobosStreamReader& Stm)
	{
		return Stm.Process(TActionExtData::RandomTriggerPool);
	}

	bool SaveGlobal(PhobosStreamWriter& Stm)
	{
		return Stm.Process(TActionExtData::RandomTriggerPool);
	}

	void InvalidatePointer(AbstractClass* ptr, bool bRemoved)
	{
		for (auto& nMap : TActionExtData::RandomTriggerPool) {
			if (bRemoved) {
				fast_remove_if(nMap.second, [ptr](auto _el) { return  ptr == _el; });
			}
		}
	}

	void Clear() {
		TActionExtData::RandomTriggerPool.clear();
	}

public:
	static TActionExtContainer Instance;
};

class NOVTABLE FakeTActionClass : public TActionClass
{
public:

	bool _OperatorBracket(HouseClass* pTargetHouse, ObjectClass* pSourceObject, TriggerClass* pTrigger, CellStruct* plocation);
	std::string _BuildINIEntry();

	static TriggerAttachType __fastcall AttachesTo(int type);
	static LogicNeedType __fastcall ActionNeeds(int type);

}; static_assert(sizeof(FakeTActionClass) == 0x94, "Invalid Size !");