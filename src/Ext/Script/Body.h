#pragma once

#include <ScriptClass.h>

#include <Utilities/Container.h>

enum class MoveMissionEndModes : int
{
	Fetch = -1,
	TeamLeaderInRange = 0,
	AnyMemberInRange = 1,
	AllMemberInRange = 2,

	count
};

enum class BuildingWithProperty : unsigned int
{
	LeastThreat = 0,
	HighestThreat = 65536,
	Nearest = 131072,
	Farthest = 196608
};

class HouseClass;
class TeamClass;
enum class PhobosScripts : int
{
	////https://github.com/FrozenFog/Phobos/commit/449074b15fc846b8545911447aea57fc2964c377
	ChangeTeamGroup = 114,
	DistributedLoading = 115,
	FollowFriendlyByGroup = 116,
	RallyUnitWithSameGroup = 117,

	ForceGlobalOnlyTargetHouseEnemy = 150,

	// Range 10000-10999 are team (aka ingame) actions
	// Sub-range 10000-10049 is for "attack" actions
	RepeatAttackCloser = 10000,
	SingleAttackCloser = 10001,
	RepeatAttackTypeCloser = 10002,
	SingleAttackTypeCloser = 10003,
	RandomAttackTypeCloser = 10004,
	RepeatAttackFarther = 10005,
	SingleAttackFarther = 10006,
	RepeatAttackTypeFarther = 10007,
	SingleAttackTypeFarther = 10008,
	RandomAttackTypeFarther = 10009,
	RepeatAttackCloserThreat = 10010,
	SingleAttackCloserThreat = 10011,
	RepeatAttackTypeCloserThreat = 10012,
	SingleAttackTypeCloserThreat = 10013,
	RepeatAttackFartherThreat = 10014,
	SingleAttackFartherThreat = 10015,
	RepeatAttackTypeFartherThreat = 10016,
	SingleAttackTypeFartherThreat = 10017,
	// Sub-range 10050-10099 is for "Move to" actions
	MoveToEnemyCloser = 10050,
	MoveToTypeEnemyCloser = 10051,
	RandomMoveToTypeEnemyCloser = 10052,
	MoveToFriendlyCloser = 10053,
	MoveToTypeFriendlyCloser = 10054,
	RandomMoveToTypeFriendlyCloser = 10055,
	MoveToEnemyFarther = 10056,
	MoveToTypeEnemyFarther = 10057,
	RandomMoveToTypeEnemyFarther = 10058,
	MoveToFriendlyFarther = 10059,
	MoveToTypeFriendlyFarther = 10060,
	RandomMoveToTypeFriendlyFarther = 10061,
	// Sub-range 10100-10999 is for "general purpose" actions
	TimedAreaGuard = 10100,
	WaitUntilFullAmmo = 10101,
	GatherAroundLeader = 10102,
	LoadIntoTransports = 10103,
	ChronoshiftToEnemyBase = 10104,
	RepairDestroyedBridge = 10105,

	// Range 12000-12999 are suplementary/setup pre-actions
	WaitIfNoTarget = 12000,
	ModifyTargetDistance = 12001,
	SetMoveMissionEndMode = 12002,

	// Range 14000-14999 are utility actions (angernodes manipulation, Team manipulation, etc)
	TeamWeightReward = 14000,
	IncreaseCurrentAITriggerWeight = 14001,
	DecreaseCurrentAITriggerWeight = 14002,
	UnregisterGreatSuccess = 14003,


	OverrideOnlyTargetHouseEnemy = 14005,
	SetHouseAngerModifier = 14006,
	ModifyHateHouseIndex = 14007,
	ModifyHateHousesList = 14008,
	ModifyHateHousesList1Random = 14009,
	SetTheMostHatedHouseMinorNoRandom = 14010,
	SetTheMostHatedHouseMajorNoRandom = 14011,
	SetTheMostHatedHouseRandom = 14012,
	ResetAngerAgainstHouses = 14013,
	AggroHouse = 14014,

	// Range 16000-16999 are flow control actions (jumps, change script, loops, breaks, etc)
	SameLineForceJumpCountdown = 16000,
	NextLineForceJumpCountdown = 16001,
	StopForceJumpCountdown = 16002,
	RandomSkipNextAction = 16003,
	PickRandomScript = 16004,
	JumpBackToPreviousScript = 16005,

	SetSideIdxForManagingTriggers = 16006,
	SetHouseIdxForManagingTriggers = 16007,
	ManageAllAITriggers = 16008,
	EnableTriggersFromList = 16009,
	DisableTriggersFromList = 16010,
	DisableTriggersWithObjects = 16011,
	EnableTriggersWithObjects = 16012,
	
	ConditionalJumpResetVariables = 16013,
	ConditionalJumpManageResetIfJump = 16014,
	AbortActionAfterSuccessKill = 16015,
	ConditionalJumpManageKillsCounter = 16016,
	ConditionalJumpSetCounter = 16017,
	ConditionalJumpSetComparatorMode = 16018,
	ConditionalJumpSetComparatorValue = 16019,
	ConditionalJumpSetIndex = 16020,
	ConditionalJumpIfFalse = 16021,
	ConditionalJumpIfTrue = 16022,
	ConditionalJumpKillEvaluation = 16023,
	ConditionalJumpCheckCount = 16024,
	ConditionalJumpCheckAliveHumans = 16025,
	ConditionalJumpCheckObjects = 16026,
	ConditionalJumpCheckHumanIsMostHated = 16027,
	
	// Range 18000-18999 are variable actions
	LocalVariableSet = 18000,
	LocalVariableAdd = 18001,
	LocalVariableMinus = 18002,
	LocalVariableMultiply = 18003,
	LocalVariableDivide = 18004,
	LocalVariableMod = 18005,
	LocalVariableLeftShift = 18006,
	LocalVariableRightShift = 18007,
	LocalVariableReverse = 18008,
	LocalVariableXor = 18009,
	LocalVariableOr = 18010,
	LocalVariableAnd = 18011,
	GlobalVariableSet = 18012,
	GlobalVariableAdd = 18013,
	GlobalVariableMinus = 18014,
	GlobalVariableMultiply = 18015,
	GlobalVariableDivide = 18016,
	GlobalVariableMod = 18017,
	GlobalVariableLeftShift = 18018,
	GlobalVariableRightShift = 18019,
	GlobalVariableReverse = 18020,
	GlobalVariableXor = 18021,
	GlobalVariableOr = 18022,
	GlobalVariableAnd = 18023,
	LocalVariableSetByLocal = 18024,
	LocalVariableAddByLocal = 18025,
	LocalVariableMinusByLocal = 18026,
	LocalVariableMultiplyByLocal = 18027,
	LocalVariableDivideByLocal = 18028,
	LocalVariableModByLocal = 18029,
	LocalVariableLeftShiftByLocal = 18030,
	LocalVariableRightShiftByLocal = 18031,
	LocalVariableReverseByLocal = 18032,
	LocalVariableXorByLocal = 18033,
	LocalVariableOrByLocal = 18034,
	LocalVariableAndByLocal = 18035,
	GlobalVariableSetByLocal = 18036,
	GlobalVariableAddByLocal = 18037,
	GlobalVariableMinusByLocal = 18038,
	GlobalVariableMultiplyByLocal = 18039,
	GlobalVariableDivideByLocal = 18040,
	GlobalVariableModByLocal = 18041,
	GlobalVariableLeftShiftByLocal = 18042,
	GlobalVariableRightShiftByLocal = 18043,
	GlobalVariableReverseByLocal = 18044,
	GlobalVariableXorByLocal = 18045,
	GlobalVariableOrByLocal = 18046,
	GlobalVariableAndByLocal = 18047,
	LocalVariableSetByGlobal = 18048,
	LocalVariableAddByGlobal = 18049,
	LocalVariableMinusByGlobal = 18050,
	LocalVariableMultiplyByGlobal = 18051,
	LocalVariableDivideByGlobal = 18052,
	LocalVariableModByGlobal = 18053,
	LocalVariableLeftShiftByGlobal = 18054,
	LocalVariableRightShiftByGlobal = 18055,
	LocalVariableReverseByGlobal = 18056,
	LocalVariableXorByGlobal = 18057,
	LocalVariableOrByGlobal = 18058,
	LocalVariableAndByGlobal = 18059,
	GlobalVariableSetByGlobal = 18060,
	GlobalVariableAddByGlobal = 18061,
	GlobalVariableMinusByGlobal = 18062,
	GlobalVariableMultiplyByGlobal = 18063,
	GlobalVariableDivideByGlobal = 18064,
	GlobalVariableModByGlobal = 18065,
	GlobalVariableLeftShiftByGlobal = 18066,
	GlobalVariableRightShiftByGlobal = 18067,
	GlobalVariableReverseByGlobal = 18068,
	GlobalVariableXorByGlobal = 18069,
	GlobalVariableOrByGlobal = 18070,
	GlobalVariableAndByGlobal = 18071,

	SimpleDeployerDeploy = 19000,

	ChangeToScriptByID = 19017,
	ChangeToTeamTypeByID = 19018,
	ChangeToHouseByID = 19020,
	PlaySpeechByID = 19024,
	PlaySoundByID = 19025,
	PlayMovieByID = 19026,
	PlayThemeByID = 19027,
	AttackEnemyStructureByID = 19046,
	MoveToEnemyStructureByID = 19047,
	PlayAnimationByID = 19051,
	ChronoshiftTaskForceToStructureByID = 19056,
	MoveToFriendlyStructureByID = 19058,

	count
};

enum class DistanceMode : int
{
	idkZero = 0,
	idkOne = 1,
	Closest = 2,
	Furtherst = 3
};

class ScriptExtData final
{
public:
	using base_type = ScriptClass;

public:
	// Nothing yet

	static bool ProcessScriptActions(TeamClass* pTeam, ScriptActionNode* pTeamMission, bool bThirdArd);
	static void ExecuteTimedAreaGuardAction(TeamClass* pTeam);
	static void LoadIntoTransports(TeamClass* pTeam);
	static void WaitUntilFullAmmoAction(TeamClass* pTeam);
	static void Mission_Gather_NearTheLeader(TeamClass* pTeam, int countdown);
	static void DecreaseCurrentTriggerWeight(TeamClass* pTeam, bool forceJumpLine, double modifier);
	static void IncreaseCurrentTriggerWeight(TeamClass* pTeam, bool forceJumpLine, double modifier);
	static void WaitIfNoTarget(TeamClass* pTeam, int attempts);
	static void TeamWeightReward(TeamClass* pTeam, double award);
	static void PickRandomScript(TeamClass* pTeam, int idxScriptsList);
	static void UnregisterGreatSuccess(TeamClass* pTeam);
	static void SetCloseEnoughDistance(TeamClass* pTeam, double distance);
	static void SetMoveMissionEndMode(TeamClass* pTeam, int mode);
	static void SkipNextAction(TeamClass* pTeam, int successPercentage);
	static void Set_ForceJump_Countdown(TeamClass* pTeam, bool repeatLine, int count);
	static void Stop_ForceJump_Countdown(TeamClass* pTeam);
	static void ChronoshiftToEnemyBase(TeamClass* pTeam, int extraDistance);
	static void ForceGlobalOnlyTargetHouseEnemy(TeamClass* pTeam, int mode);

	static bool IsExtVariableAction(int action);
	static bool VariablesHandler(TeamClass* pTeam, PhobosScripts eAction, int nArg);

	template<bool IsGlobal, class _Pr>
	static void VariableOperationHandler(TeamClass* pTeam, int nVariable, int Number);
	template<bool IsSrcGlobal, bool IsGlobal, class _Pr>
	static void VariableBinaryOperationHandler(TeamClass* pTeam, int nVariable, int nVarToOperate);
	static bool IsUnitAvailable(TechnoClass* pTechno, bool checkIfInTransportOrAbsorbed);

	// Mission_Attack.cpp
	static void Mission_Attack(TeamClass* pTeam, bool repeatAction, DistanceMode calcThreatMode, int attackAITargetType, int IdxAITargetTypeItem);
	static TechnoClass* GreatestThreat(TechnoClass* pTechno, int method, DistanceMode calcThreatMode, HouseClass* onlyTargetThisHouseEnemy, int attackAITargetType, int idxAITargetTypeItem, bool agentMode);
	static bool EvaluateObjectWithMask(TechnoClass* pTechno, int mask, int attackAITargetType, int idxAITargetTypeItem, TechnoClass* pTeamLeader);
	static void Mission_Attack_List(TeamClass* pTeam, bool repeatAction, DistanceMode calcThreatMode, int attackAITargetType);
	static void Mission_Attack_List1Random(TeamClass* pTeam, bool repeatAction, DistanceMode calcThreatMode, int attackAITargetType);
	static void CheckUnitTargetingCapabilities(TechnoClass* pTechno, bool& hasAntiGround, bool& hasAntiAir, bool agentMode);
	static bool IsUnitArmed(TechnoClass* pTechno);
	static bool IsUnitMindControlledFriendly(HouseClass* pHouse, TechnoClass* pTechno);

	// Mission_Move.cpp
	static void Mission_Move(TeamClass* pTeam, DistanceMode calcThreatMode, bool pickAllies, int attackAITargetType, int idxAITargetTypeItem);
	static TechnoClass* FindBestObject(TechnoClass* pTechno, int method, DistanceMode calcThreatMode, bool pickAllies, int attackAITargetType, int idxAITargetTypeItem);
	static void Mission_Move_List(TeamClass* pTeam, DistanceMode calcThreatMode, bool pickAllies, int attackAITargetType);
	static void Mission_Move_List1Random(TeamClass* pTeam, DistanceMode calcThreatMode, bool pickAllies, int attackAITargetType, int idxAITargetTypeItem);

	//Conditional Jumps
	static void ConditionalJumpIfTrue(TeamClass* pTeam, int newScriptLine);
	static void ConditionalJumpIfFalse(TeamClass* pTeam, int newScriptLine);
	static void ConditionalJump_KillEvaluation(TeamClass* pTeam);
	static void ConditionalJump_ManageKillsCounter(TeamClass* pTeam, int enable);
	static void ConditionalJump_SetIndex(TeamClass* pTeam, int index);
	static void ConditionalJump_SetComparatorValue(TeamClass* pTeam, int value);
	static void ConditionalJump_SetComparatorMode(TeamClass* pTeam, int value);
	static void ConditionalJump_SetCounter(TeamClass* pTeam, int value);
	static void ConditionalJump_ResetVariables(TeamClass* pTeam);
	static void ConditionalJump_CheckHumanIsMostHated(TeamClass* pTeam);
	static void ConditionalJump_CheckAliveHumans(TeamClass* pTeam, int mode);
	static void ConditionalJump_CheckObjects(TeamClass* pTeam);
	static void ConditionalJump_CheckCount(TeamClass* pTeam, int modifier);
	static void ConditionalJump_ManageResetIfJump(TeamClass* pTeam, int enable);
	static bool ConditionalJump_MakeEvaluation(int comparatorMode, int studiedValue, int comparatorValue);
	static void SetAbortActionAfterSuccessKill(TeamClass* pTeam, int enable);

	//ManagingTriggers
	static void ManageTriggersFromList(TeamClass* pTeam, int idxAITriggerType, bool isEnabled);
	static void ManageAllTriggersFromHouse(TeamClass* pTeam, HouseClass* pHouse, int sideIdx, int houseIdx, bool isEnabled);
	static void ManageAITriggers(TeamClass* pTeam, int enabled);
	static void ManageTriggersWithObjects(TeamClass* pTeam, int idxAITargetType, bool isEnabled);
	static void SetSideIdxForManagingTriggers(TeamClass* pTeam, int sideIdx);
	static void SetHouseIdxForManagingTriggers(TeamClass* pTeam, int houseIdx);

	//AngerNodes
	static void ResetAngerAgainstHouses(TeamClass* pTeam);
	static void SetHouseAngerModifier(TeamClass* pTeam, int modifier);
	static void ModifyHateHouses_List(TeamClass* pTeam, int idxHousesList);
	static void ModifyHateHouses_List1Random(TeamClass* pTeam, int idxHousesList);
	static void ModifyHateHouse_Index(TeamClass* pTeam, int idxHouse);
	static void SetTheMostHatedHouse(TeamClass* pTeam, int mask, int mode, bool random);
	static void OverrideOnlyTargetHouseEnemy(TeamClass* pTeam, int mode);
	static void AggroHouse(TeamClass* pTeam, int index);
	static HouseClass* GetTheMostHatedHouse(TeamClass* pTeam, int mask, int mode);
	static void DebugAngerNodesData();
	static void UpdateEnemyHouseIndex(HouseClass* pHouse);

	//
	static void JumpBackToPreviousScript(TeamClass* pTeam);
	static void RepairDestroyedBridge(TeamClass* pTeam, int mode);
	static std::pair<WeaponTypeClass*, WeaponTypeClass*> GetWeapon(TechnoClass* pTechno);

	// SimpleDeployer deploy action
	static void SimpleDeployerDeploy(TeamClass* pTeam, int mode = -1);

	static void PlaySpeech(TeamClass* pTeam);

	COMPILETIMEEVAL FORCEDINLINE static size_t size_Of()
	{
		return sizeof(ScriptExtData) -
			(4u //AttachedToObject
			 );
	}
private:
	static void ModifyCurrentTriggerWeight(TeamClass* pTeam, bool forceJumpLine, double modifier);
	static bool MoveMissionEndStatus(TeamClass* pTeam, TechnoClass* pFocus, FootClass* pLeader, int mode);
	static void ChronoshiftTeamToTarget(TeamClass* pTeam, TechnoClass* pTeamLeader, AbstractClass* pTarget);
	static ScriptActionNode GetSpecificAction(ScriptClass* pScript, int nIdx);

private:
	template <typename T>
	void Serialize(T& Stm)
	{
	}
};

class ScriptExtContainer final : public Container<ScriptExtData>
{
public:
	static COMPILETIMEEVAL const char* ClassName = "ScriptExtContainer";

public:
	static ScriptExtContainer Instance;

	virtual bool LoadAll(const json& root) { return true;  };
	virtual bool SaveAll(json& root) { return true;  };

};
