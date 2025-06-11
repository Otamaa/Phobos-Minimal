#pragma once

#include <GeneralStructures.h>
#include <Helpers\EnumFlags.h>

/**
 *  Timer constants.
 */
#define TIMER_SECOND        60
#define TIMER_MINUTE        (TIMER_SECOND * 60)

#define TICKS_PER_SECOND    15
#define TICKS_PER_MINUTE    (TICKS_PER_SECOND * 60)
#define TICKS_PER_HOUR      (TICKS_PER_MINUTE * 60)

#define SECONDS_TO_MILLISECONDS(x)  (x * 1000)

//Scalar Deleting Destructor Flags
#define SDDTOR_NODELETE				0x00
#define SDDTOR_DELETE				0x01

enum ProdFailType : byte
{
	OK = 0x0,
	Limit = 0x1,
	Illegal = 0x2,
	Cant = 0x3,
};

enum class MarkType : int
{
	Up = 0,
	Remove = 0,
	Down = 1,
	Put = 1,
	Change = 2,
	Redraw = 2,
	ChangeRedraw = 3,
	AddContent = 3,
	OverlapDown = 4,
	OverlapUp = 5
};

enum class InitState : int
{
	Blank = 0x0, // CTOR'd
	Constanted = 0x1, // initialization that happen after constructing class
	Ruled = 0x2, // class already reading data from rules file
	Inited = 0x3, // initialization that happen before reading from ini file
	Completed = 0x4 // value reading is completed
};

enum class PacketFieldRep : int
{
	NAM,
	SID,
	TID,
	COL,
	CTY,
	CRD,
	UNL,
	INL,
	PLL,
	BLL,
	UNB,
	INB,
	PLB,
	BLB,
	UNK,
	INK,
	PLK,
	BLK,
	BLC,
	CRA,
	HRV,
	CMP,
	IPA,
	CID,
	LCN,
	CPT,
	CPS,
	MEM,
	VID,
};

enum class RocketMissionState : int
{
	None = 0,
	Pause = 1,
	Tilt = 2,
	GainingAltitude = 3,
	Flight = 4,
	ClosingIn = 5,
	VerticalTakeOff = 6,
};

enum class PassabilityType : DWORD
{
	Passable = 0,
	Crushable = 1,
	Destroyable = 2, // Tree or wall
	Beach = 3,
	Water = 4,
	HasFreeSpots = 5,
	Impassable = 6,
	OutsideMap = 7
};

enum class EventType : unsigned char
{
	EMPTY = 0,
	POWERON = 1,
	POWEROFF = 2,
	ALLY = 3,
	MEGAMISSION = 4,
	MEGAMISSION_F = 5,
	IDLE = 6,
	SCATTER = 7,
	DESTRUCT = 8,
	DEPLOY = 9,
	DETONATE = 10,
	PLACE = 11,
	OPTIONS = 12,
	GAMESPEED = 13,
	PRODUCE = 14,
	SUSPEND = 15,
	ABANDON = 16,
	PRIMARY = 17,
	SPECIAL_PLACE = 18,
	EXIT = 19,
	ANIMATION = 20,
	REPAIR = 21,
	SELL = 22,
	SELLCELL = 23,
	SPECIAL = 24,
	FRAMESYNC = 25,
	MESSAGE = 26,
	RESPONSE_TIME = 27,
	FRAMEINFO = 28,
	SAVEGAME = 29,
	ARCHIVE = 30,
	ADDPLAYER = 31,
	TIMING = 32,
	PROCESS_TIME = 33,
	PAGEUSER = 34,
	REMOVEPLAYER = 35,
	LATENCYFUDGE = 36,
	MEGAFRAMEINFO = 37,
	PACKETTIMING = 38,
	ABOUTTOEXIT = 39,
	FALLBACKHOST = 40,
	ADDRESSCHANGE = 41,
	PLANCONNECT = 42,
	PLANCOMMIT = 43,
	PLANNODEDELETE = 44,
	ALLCHEER = 45,
	ABANDON_ALL = 46,
	LAST_EVENT = 47,
};

enum class AbandonProductionResult : int
{
	Ok = 0,
	Limit = 1,
	Illegal = 2,
	Cant = 3,

	count
};

enum class Countries : int
{
	None = -1,
	Americans,
	Alliance,
	French,
	Germans,
	British,
	Africans,
	Arabs,
	Confederation,
	Russians,
	YuriCountry
};

enum class AirAttackStatus
{
	ValidateAZ = 0,
	PickAttackLocation = 1,
	TakeOff = 2,
	FlyToPosition = 3,
	FireAtTarget = 4,
	FireAtTarget2 = 5,
	FireAtTarget2_Strafe = 6,
	FireAtTarget3_Strafe = 7,
	FireAtTarget4_Strafe = 8,
	FireAtTarget5_Strafe = 9,
	ReturnToBase = 10
};

enum class Locomotors : int
{
	Drive = 0,
	Jumpjet = 1,
	Hover = 2,
	Rocket = 3,
	Tunnel = 4,
	Walk = 5,
	DropPod = 6,
	Fly = 7,
	Teleport = 8,
	Mech = 9,
	Ship = 10,
};

enum class Panning : int
{
	Left = 0x0000,
	Rignt = 0x4000,
	Center = 0x2000
};

enum class ZoneType : int
{
	None = -1,
	Core = 0,
	North = 1,
	East = 2,
	South = 3,
	West = 4
};

enum class RGBMode : unsigned int
{
	RGB555 = 0,
	RGB556 = 1,
	RGB565 = 2,
	RGB655 = 3,
	Invalid = 0x0FFFFFFFF
};

enum class NavalTargetingType : int
{
	Underwater_never = 0x0,
	Underwater_secondary = 0x1,
	Underwater_only = 0x2,
	Organic_secondary = 0x3,
	Seal_special = 0x4,
	Naval_all = 0x5,
	Naval_none = 0x6,
	Naval_primary = 0x7,
};

enum class LandTargetingType : int
{
	Land_okay = 0x0,
	Land_not_okay = 0x1,
	Land_secondary = 0x2,
};

enum class ThreatType : int
{
	Normal = 0x0,
	Range = 0x1,
	Area = 0x2,
	Air = 0x4,
	Infantry = 0x8,
	Vehicles = 0x10,
	Buildings = 0x20,
	Tiberium = 0x40,
	Boats = 0x80,
	Civilians = 0x100,
	Capture = 0x200,
	FakeBuildings = 0x400,
	PowerFacilties = 0x800,
	Factories = 0x1000,
	Base_defenses = 0x2000,
	Threattype_4000 = 0x4000,
	OccupiableBuildings = 0x8000,
	TechBuildings = 0x10000,
};
MAKE_ENUM_FLAGS(ThreatType);

enum class SellCheckType :int
{
	CheckMissionDeconst = -1,
	CheckIfNotMissionDeconst = 0,
	PlayGenericClick
};

enum class RepairState : int
{
	Off = 0,
	On = 1,
	Toggle = -1
};

enum class PCPType :int
{
	Rotation = 0,
	During = 1,
	End = 2
};

enum class AbstractFlags : unsigned int {
	None = 0x0,
	Techno = 0x1,
	Object = 0x2,
	Foot = 0x4
};

MAKE_ENUM_FLAGS(AbstractFlags)

enum class TileType : int
{
	Unk = 0,
	Tunnel,
	Water,
	Blank,
	Ramp,
	Cliff,
	Shore,
	Wet,
	MiscPave,
	Pave,
	DirtRoad,
	PavedRoad,
	PavedRoadEnd,
	PavedRoadSlope,
	Median,
	Bridge,
	WoodBridge,
	ClearToSandLAT,
	Green,
	NotWater,
	DestroyableCliff
};

enum class AbstractType : unsigned int {
	None = 0,
	Unit = 1,
	Aircraft = 2,
	AircraftType = 3,
	Anim = 4,
	AnimType = 5,
	Building = 6,
	BuildingType = 7,
	Bullet = 8,
	BulletType = 9,
	Campaign = 10,
	Cell = 11,
	Factory = 12,
	House = 13,
	HouseType = 14,
	Infantry = 15,
	InfantryType = 16,
	Isotile = 17,
	IsotileType = 18,
	BuildingLight = 19,
	Overlay = 20,
	OverlayType = 21,
	Particle = 22,
	ParticleType = 23,
	ParticleSystem = 24,
	ParticleSystemType = 25,
	Script = 26,
	ScriptType = 27,
	Side = 28,
	Smudge = 29,
	SmudgeType = 30,
	Special = 31,
	SuperWeaponType = 32,
	TaskForce = 33,
	Team = 34,
	TeamType = 35,
	Terrain = 36,
	TerrainType = 37,
	Trigger = 38,
	TriggerType = 39,
	UnitType = 40,
	VoxelAnim = 41,
	VoxelAnimType = 42,
	Wave = 43,
	Tag = 44,
	TagType = 45,
	Tiberium = 46,
	Action = 47,
	Event = 48,
	WeaponType = 49,
	WarheadType = 50,
	Waypoint = 51,
	Abstract = 52,
	Tube = 53,
	LightSource = 54,
	EMPulse = 55,
	TacticalMap = 56,
	Super = 57,
	AITrigger = 58,
	AITriggerType = 59,
	Neuron = 60,
	FoggedObject = 61,
	AlphaShape = 62,
	VeinholeMonster = 63,
	NavyType = 64,
	SpawnManager = 65,
	CaptureManager = 66,
	Parasite = 67,
	Bomb = 68,
	RadSite = 69,
	Temporal = 70,
	Airstrike = 71,
	SlaveManager = 72,
	DiskLaser = 73
};

enum class RecordFlag : unsigned int
{
	Write = 0x1, // I am recording the game now
	Read = 0x2, // I am playing a recorded game now
	Attract = 0x4 // Recording enabled by commmand line
};

enum class Action : unsigned int {
	None = 0,
	Move = 1,
	NoMove = 2,
	Enter = 3,
	Self_Deploy = 4,
	Attack = 5,
	Harvest = 6,
	Select = 7,
	ToggleSelect = 8,
	Capture = 9,
	Eaten = 10,
	Repair = 11,
	Sell = 12,
	SellUnit = 13,
	NoSell = 14,
	NoRepair = 15,
	Sabotage = 16,
	Tote = 17,
	DontUse2 = 18,
	DontUse3 = 19,
	Nuke = 20,
	DontUse4 = 21,
	DontUse5 = 22,
	DontUse6 = 23,
	DontUse7 = 24,
	DontUse8 = 25,
	GuardArea = 26,
	Heal = 27,
	Damage = 28,
	GRepair = 29,
	NoDeploy = 30,
	NoEnter = 31,
	NoGRepair = 32,
	TogglePower = 33,
	NoTogglePower = 34,
	EnterTunnel = 35,
	NoEnterTunnel = 36,
	IronCurtain = 37,
	LightningStorm = 38,
	ChronoSphere = 39,
	ChronoWarp = 40,
	ParaDrop = 41,
	PlaceWaypoint = 42,
	TibSunBug = 43,
	EnterWaypointMode = 44,
	FollowWaypoint = 45,
	SelectWaypoint = 46,
	LoopWaypointPath = 47,
	DragWaypoint = 48,
	AttackWaypoint = 49,
	EnterWaypoint = 50,
	PatrolWaypoint = 51,
	AreaAttack = 52,
	IvanBomb = 53,
	NoIvanBomb = 54,
	Detonate = 55,
	DetonateAll = 56,
	DisarmBomb = 57,
	SelectNode = 58,
	AttackSupport = 59,
	PlaceBeacon = 60,
	SelectBeacon = 61,
	AttackMoveNav = 62,
	AttackMoveTar = 63,
	Demolish = 64,
	AmerParaDrop = 65,
	PsychicDominator = 66,
	SpyPlane = 67,
	GeneticConverter = 68,
	ForceShield = 69,
	NoForceShield = 70,
	Airstrike = 71,
	PsychicReveal = 72,

	count
};

enum class AbilityType : int {
	Faster = 0,
	Stronger = 1,
	Firepower = 2,
	Scatter = 3,
	ROF = 4,
	Sight = 5,
	Cloak = 6,
	TiberiumProof = 7,
	VeinProof = 8,
	SelfHeal = 9,
	Explodes = 10,
	RadarInvisible = 11,
	Sensors = 12,
	Fearless = 13,
	C4 = 14,
	TiberiumHeal = 15,
	GuardArea = 16,
	Crusher = 17
};

enum class AIDifficulty : unsigned int {
	Hard = 0,
	Normal = 1,
	Easy = 2
};

enum class TextPrintType : int
{
	LASTPOINT = 0x0, //*
	LASTSHADOW = 0x0, //*
	Point6 = 0x1, //*
	Point8 = 0x2,
	Point3 = 0x3, //*
	Led = 0x4, //*
	Vcr = 0x5, //*
	Point6Grad = 0x6,
	Map = 0x7, //*
	Metal12 = 0x8,
	Efnt = 0x9, //*
	Type = 0xA, //*
	Score = 0xB, //*
	Fonts = 0xF, //*
	NoShadow = 0x10,
	DropShadow = 0x20,
	FullShadow = 0x40,
	LightShadow = 0x80,
	Center = 0x100,
	Right = 0x200,
	Background = 0x400,
	MediumColor = 0x1000,
	BrightColor = 0x2000,
	UseGradPal = 0x4000,
	UnknownColor = 0x8000,
	GradAll = 0xF000,
};

MAKE_ENUM_FLAGS(TextPrintType)

enum class TriggerPersistence : unsigned int {
	Volatile = 0, // trigger for the first object whose events fired, then disable
	SemiPersistant = 1, // trigger after all object's events fired, then disable
	Persistent = 2 // trigger every time events fire for any object, never disable
};

enum class TriggerEvent : unsigned int {
	None = 0x0,
	EnteredBy = 0x1,
	SpiedBy = 0x2,
	ThievedBy = 0x3,
	DiscoveredByPlayer = 0x4,
	HouseDiscovered = 0x5,
	AttackedByAnybody = 0x6,
	DestroyedByAnybody = 0x7,
	AnyEvent = 0x8,
	DestroyedUnitsAll = 0x9,
	DestroyedBuildingsAll = 0xA,
	DestroyedAll = 0xB,
	CreditsExceed = 0xC,
	ElapsedTime = 0xD,
	MissionTimerExpired = 0xE,
	DestroyedBuildingsNum = 0xF,
	DestroyedUnitsNum = 0x10,
	NoFactoriesLeft = 0x11,
	CiviliansEvacuated = 0x12,
	BuildBuildingType = 0x13,
	BuildUnitType = 0x14,
	BuildInfantryType = 0x15,
	BuildAircraftType = 0x16,
	TeamLeavesMap = 0x17,
	ZoneEntryBy = 0x18,
	CrossesHorizontalLine = 0x19,
	CrossesVerticalLine = 0x1A,
	GlobalSet = 0x1B,
	GlobalCleared = 0x1C,
	DestroyedFakesAll = 0x1D,
	LowPower = 0x1E,
	AllBridgesDestroyed = 0x1F,
	BuildingExists = 0x20,
	SelectedByPlayer = 0x21,
	ComesNearWaypoint = 0x22,
	EnemyInSpotlight = 0x23,
	LocalSet = 0x24,
	LocalCleared = 0x25,
	FirstDamaged_combatonly = 0x26,
	HalfHealth_combatonly = 0x27,
	QuarterHealth_combatonly = 0x28,
	FirstDamaged_anysource = 0x29,
	HalfHealth_anysource = 0x2A,
	QuarterHealth_anysource = 0x2B,
	AttackedByHouse = 0x2C,
	AmbientLightBelow = 0x2D,
	AmbientLightAbove = 0x2E,
	ElapsedScenarioTime = 0x2F,
	DestroyedByAnything = 0x30,
	PickupCrate = 0x31,
	PickupCrate_any = 0x32,
	RandomDelay = 0x33,
	CreditsBelow = 0x34,
	SpyAsHouse = 0x35,
	SpyAsInfantry = 0x36,
	DestroyedUnitsNaval = 0x37,
	DestroyedUnitsLand = 0x38,
	BuildingDoesNotExist = 0x39,
	PowerFull = 0x3A,
	EnteredOrOverflownBy = 0x3B,
	TechTypeExists = 0x3C,
	TechTypeDoesntExist = 0x3D,

	count
};

enum class TriggerAction : unsigned int {
	None = 0x0,
	Win = 0x1,
	Lose = 0x2,
	ProductionBegins = 0x3,
	CreateTeam = 0x4,
	DestroyTeam = 0x5,
	AllToHunt = 0x6,
	Reinforcement = 0x7,
	DropZoneFlare = 0x8,
	FireSale = 0x9,
	PlayMovie = 0xA,
	TextTrigger = 0xB,
	DestroyTrigger = 0xC,
	AutocreateBegins = 0xD,
	ChangeHouse = 0xE,
	AllowWin = 0xF,
	RevealAllMap = 0x10,
	RevealAroundWaypoint = 0x11,
	RevealWaypointZone = 0x12,
	PlaySoundEffect = 0x13,
	PlayMusicTheme = 0x14,
	PlaySpeech = 0x15,
	ForceTrigger = 0x16,
	TimerStart = 0x17,
	TimerStop = 0x18,
	TimerExtend = 0x19,
	TimerShorten = 0x1A,
	TimerSet = 0x1B,
	GlobalSet = 0x1C,
	GlobalClear = 0x1D,
	AutoBaseBuilding = 0x1E,
	GrowShroud = 0x1F,
	DestroyAttachedObject = 0x20,
	AddOneTimeSuperWeapon = 0x21,
	AddRepeatingSuperWeapon = 0x22,
	PreferredTarget = 0x23,
	AllChangeHouse = 0x24,
	MakeAlly = 0x25,
	MakeEnemy = 0x26,
	ChangeZoomLevel = 0x27,
	ResizePlayerView = 0x28,
	PlayAnimAt = 0x29,
	DoExplosionAt = 0x2A,
	CreateVoxelAnim = 0x2B,
	IonStormStart = 0x2C,
	IonStormStop = 0x2D,
	LockInput = 0x2E,
	UnlockInput = 0x2F,
	MoveCameraToWaypoint = 0x30,
	ZoomIn = 0x31,
	ZoomOut = 0x32,
	ReshroudMap = 0x33,
	ChangeLightBehavior = 0x34,
	EnableTrigger = 0x35,
	DisableTrigger = 0x36,
	CreateRadarEvent = 0x37,
	LocalSet = 0x38,
	LocalClear = 0x39,
	MeteorShower = 0x3A,
	ReduceTiberium = 0x3B,
	SellBuilding = 0x3C,
	TurnOffBuilding = 0x3D,
	TurnOnBuilding = 0x3E,
	Apply100Damage = 0x3F,
	SmallLightFlash = 0x40,
	MediumLightFlash = 0x41,
	LargeLightFlash = 0x42,
	AnnounceWin = 0x43,
	AnnounceLose = 0x44,
	ForceEnd = 0x45,
	DestroyTag = 0x46,
	SetAmbientStep = 0x47,
	SetAmbientRate = 0x48,
	SetAmbientLight = 0x49,
	AITriggersBegin = 0x4A,
	AITriggersStop = 0x4B,
	RatioOfAITriggerTeams = 0x4C,
	RatioOfTeamAircraft = 0x4D,
	RatioOfTeamInfantry = 0x4E,
	RatioOfTeamUnits = 0x4F,
	ReinforcementAt = 0x50,
	WakeupSelf = 0x51,
	WakeupAllSleepers = 0x52,
	WakeupAllHarmless = 0x53,
	WakeupGroup = 0x54,
	VeinGrowth = 0x55,
	TiberiumGrowth = 0x56,
	IceGrowth = 0x57,
	ParticleAnim = 0x58,
	RemoveParticleAnim = 0x59,
	LightningStrike = 0x5A,
	GoBerzerk = 0x5B,
	ActivateFirestorm = 0x5C,
	DeactivateFirestorm = 0x5D,
	IonCannonStrike = 0x5E,
	NukeStrike = 0x5F,
	ChemMissileStrike = 0x60,
	ToggleTrainCargo = 0x61,
	PlaySoundEffectRandom = 0x62,
	PlaySoundEffectAtWaypoint = 0x63,
	PlayIngameMovie = 0x64,
	ReshroudMapAtWaypoint = 0x65,
	LightningStormStrike = 0x66,
	TimerText = 0x67,
	FlashTeam = 0x68,
	TalkBubble = 0x69,
	SetObjectTechLevel = 0x6A,
	ReinforcementByChrono = 0x6B,
	CreateCrate = 0x6C,
	IronCurtain = 0x6D,
	PauseGame = 0x6E,
	EvictOccupiers = 0x6F,
	CenterCameraAtWaypoint = 0x70,
	MakeHouseCheer = 0x71,
	SetTabTo = 0x72,
	FlashCameo = 0x73,
	StopSounds = 0x74,
	PlayIngameMovieAndPause = 0x75,
	ClearAllSmudges = 0x76,
	DestroyAll = 0x77,
	DestroyAllBuildings = 0x78,
	DestroyAllLandUnits = 0x79,
	DestroyAllNavalUnits = 0x7A,
	MindControlBase = 0x7B,
	RestoreMindControlledBase = 0x7C,
	CreateBuilding = 0x7D,
	RestoreStartingUnits = 0x7E,
	StartChronoScreenEffect = 0x7F,
	TeleportAll = 0x80,
	SetSuperWeaponCharge = 0x81,
	RestoreStartingBuildings = 0x82,
	FlashBuildingsOfType = 0x83,
	SuperWeaponSetRechargeTime = 0x84,
	SuperWeaponResetRechargeTime = 0x85,
	SuperWeaponReset = 0x86,
	SetPreferredTargetCell = 0x87,
	ClearPreferredTargetCell = 0x88,
	SetBaseCenterCell = 0x89,
	ClearBaseCenterCell = 0x8A,
	BlackoutRadar = 0x8B,
	SetDefensiveTargetCell = 0x8C,
	ClearDefensiveTargetCell = 0x8D,
	RetintRed = 0x8E,
	RetintGreen = 0x8F,
	RetintBlue = 0x90,
	JumpCameraHome = 0x91,

	count
};

enum class TriggerAttachType : int {
	None = 0x0,
	Global = 0x1,
	Object = 0x2,
	Map = 0x4,
	House = 0x8,
	Logic = 0x10
};
MAKE_ENUM_FLAGS(TriggerAttachType);

enum class LogicNeedType : int {
	None = 0,
	Theme,
	Movie,
	Sound,
	Speech,
	Infantry,
	Unit,
	Aircraft,
	Structure,
	Waypoint,
	Number,
	Trigger,
	Team,
	House,
	Time,
	Quarry,
	Bool,
	Special,
	Mission,
	Cell,
	Script,
	Rectangle,
	Global,
	AnimNWaypoint,
	WaypointNWeapon,
	LightBehavior,
	Tag,
	MeteorSize,
	RadarEventNWaypoint,
	Local,
	VelocityNWaypoint,
	ShowerNWaypoint,
	Velocity,
	Float,
	TeamNWaypoint,
	ParticleNWaypoint,
	BuildingNProperty,
	SplitType,
	SoundNWaypoint,
	Text,
	Team2,
	Anim,
	SpeechBubble,
	NumberNTech,
	CrateNWaypoint,
	BuildingNWaypoint,
	NumberNSuper,
	BuildingNNumber,
};

enum class UrgencyType : int
{
	None,           // No action on this matter is needed or desired.
	Low,            // Minimal attention requested.
	Medium,         // Normal attention requested.
	Hight,           // High attention requested.
	Critical,       // This matter must be addressed immediately.

	Count,
	First = 0
};
MAKE_ENUM_FLAGS(UrgencyType);

enum class StrategyType : int
{
	FireSale,         // Situation hopeless, sell and attack.
	RaiseMoney,       // Money is low, emergency raise cash.

	Count,
	First = 0
};
MAKE_ENUM_FLAGS(StrategyType);

enum class AIMode : int {
	General = 0,
	LowOnCash = 1, // if HouseClass::AvailableMoney < 25 ! stupidly low value
	BuildBase = 3,
	SellAll = 4,
};

enum class AITriggerCondition : int {
	Pool = -1,
	AIOwns = 0,
	EnemyOwns = 1,
	EnemyYellowPower = 2,
	EnemyRedPower = 3,
	EnemyCashExceeds = 4,
	IronCharged = 5,
	ChronoCharged = 6,
	NeutralOwns = 7
};

enum class AITriggerHouseType : int {
	None = 0,
	Single = 1,
	Any = 2
};

enum class Armor : unsigned int {
	None = 0,
	Flak = 1,
	Plate = 2,
	Light = 3,
	Medium = 4,
	Heavy = 5,
	Wood = 6,
	Steel = 7,
	Concrete = 8,
	Special_1 = 9,
	Special_2 = 10
};

//spotlight behaviour
enum class SpotlightBehaviour : unsigned int {
	None = 0,
	Sweep = 1,
	Circle = 2,
	Follow = 3
};

enum class SpotlightFlags : unsigned int {
	None = 0x0,
	NoColor = 0x1,
	NoRed = 0x2,
	NoGreen = 0x4,
	NoBlue = 0x8
};
MAKE_ENUM_FLAGS(SpotlightFlags)

enum class ParticleSystemTypeBehavesLike : int {
	None = -1,
	Smoke = 0,
	Gas = 1,
	Fire = 2,
	Spark = 3,
	Railgun = 4
};

//ParticleSystemTypeClass->HoldsWhat, almost, but not entirely, unlike eBehavesLike above
enum class ParticleTypeBehavesLike : int {
	None = -1,
	Gas = 0,
	Smoke = 1,
	Fire = 2,
	Spark = 3,
	Railgun = 4
};

enum class BuildCat : unsigned int {
	DontCare = 0,
	Tech = 1,
	Resoure = 2,
	Power = 3,
	Infrastructure = 4,
	Combat = 5
};

enum class BuildingAnimSlot : int {
	All = -2,
	None = -1,
	Upgrade1 = 0,
	Upgrade2 = 1,
	Upgrade3 = 2,
	Active = 3,
	ActiveTwo = 4,
	ActiveThree = 5,
	ActiveFour = 6,
	PreProduction = 7,
	Production = 8,
	Turret = 9,
	Special = 10,
	SpecialTwo = 11,
	SpecialThree = 12,
	SpecialFour = 13,
	Super = 14,
	SuperTwo = 15,
	SuperThree = 16,
	SuperFour = 17,
	Idle = 18,
	LowPower = 19,
	SuperLowPower = 20
};

enum class Category : int {
	Invalid = -1,
	Soldier = 0,
	Civilian = 1,
	VIP = 2,
	Recon = 3,
	AFV = 4,
	IFV = 5,
	LRFS = 6,
	Support = 7,
	Transport = 8,
	AirPower = 9,
	AirLift = 10
};

enum class CellFlags : unsigned int
{
	Empty = 0x0,
	CenterRevealed = 0x1,
	EdgeRevealed = 0x2,
	IsWaypoint = 0x4,
	Explored = 0x8, // this means no shroud
	FlagPresent = 0x10,
	FlagToShroud = 0x20,
	IsPlot = 0x40,
	BridgeOwner = 0x80, // where the repair hut should be put, see 571FEB
	BridgeHead = 0x100, // Bridge head, not sure if its just the owner, see 570254
	Unknown_200 = 0x200,
	BridgeBody = 0x400,
	BridgeDir = 0x800, // see 570288 and 57036D
	PixelFX = 0x1000, // see 6D7A91
	Unknown_2000 = 0x2000,
	Unknown_4000 = 0x4000,
	Veinhole = 0x8000, // see 74E4A3
	DrawDarkenIfInAir = 0x10000, // see 51936F and 73D084
	AnimAttached = 0x20000,
	Tube = 0x40000, // see 42B5B8
	EMPPresent = 0x80000,
	HorizontalLineEventTag = 0x100000,
	VerticalLineEventTag = 0x200000,
	Fogged = 0x400000,

	Revealed = CenterRevealed | EdgeRevealed,
	Bridge = BridgeHead,
	BridgeWithBody = BridgeHead | BridgeBody,
};

MAKE_ENUM_FLAGS(CellFlags)

enum class AltCellFlags : unsigned int
{
	Unknown_1 = 0x1, // 487720 check_obstacle?
	ContainsBuilding = 0x2,
	Unknown_4 = 0x4, // 47EED4 PlaceShape related
	Mapped = 0x8,
	NoFog = 0x10,
	Unknown_20 = 0x20,
	Unknown_40 = 0x40,
	Unknown_80 = 0x80,
	Unknown_100 = 0x100,

	Clear = Mapped | NoFog
};

MAKE_ENUM_FLAGS(AltCellFlags)


enum class CloakState : int {
	Uncloaked = 0,
	Cloaking = 1,
	Cloaked = 2,
	Uncloaking = 3
};

enum class DamageState : int {
	Unaffected = 0,
	Unchanged = 1,
	NowYellow = 2,
	NowRed = 3,
	NowDead = 4,
	PostMortem = 5
};

enum class DamageAreaResult : int {
	Hit = 0,
	Missed = 1,
	Nullified = 2
};

enum class KickOutResult : int {
	Failed = 0,
	Busy = 1,
	Succeeded = 2
};

enum class CanBuildResult : int {
	TemporarilyUnbuildable = -1, // black out cameo
	Unbuildable = 0, // permanently; remove cameo
	Buildable = 1, // can build
};

// this is how game's enums are to be defined from now on
enum class FireError : int {
	NONE = -1, // no valid value
	OK = 0, // no problem, can fire
	AMMO = 1, // no ammo
	FACING = 2, // bad facing
	REARM = 3, // still reloading
	ROTATING = 4, // busy rotating
	ILLEGAL = 5, // can't fire
	CANT = 6, // I'm sorry Dave, I can't do that
	MOVING = 7, // moving, can't fire
	RANGE = 8, // out of range
	CLOAKED = 9, // need to decloak
	BUSY = 10, // busy, please hold
	MUST_DEPLOY = 11 // deploy first!
};

enum class HealthState : unsigned int {
	Red = 0,
	Yellow = 1,
	Green = 2
};

enum class Foundation : int
{
	_1x1 = 0,
	_2x1 = 1,
	_1x2 = 2,
	_2x2 = 3,
	_2x3 = 4,
	_3x2 = 5,
	_3x3 = 6,
	_3x5 = 7,
	_4x2 = 8,
	_3x3Refinery = 9,
	_1x3 = 10,
	_3x1 = 11,
	_4x3 = 12,
	_1x4 = 13,
	_1x5 = 14,
	_2x6 = 15,
	_2x5 = 16,
	_5x3 = 17,
	_4x4 = 18,
	_3x4 = 19,
	_6x4 = 20,
	_0x0 = 21,

	count = 22
};

enum class GameMode : unsigned int {
	Campaign = 0x0,
	LAN = 0x3,
	Internet = 0x4,
	Skirmish = 0x5,
};

enum class InfDeath : unsigned int {
	None = 0,
	Die1 = 1,
	Die2 = 2,
	Explode = 3,
	Flames = 4,
	Electro = 5,
	HeadPop = 6,
	Nuked = 7,
	Virus = 8,
	Mutate = 9,
	Brute = 10
};

enum class LandType : int {
	Clear = 0,
	Road = 1,
	Water = 2,
	Rock = 3,
	Wall = 4,
	Tiberium = 5,
	Beach = 6,
	Rough = 7,
	Ice = 8,
	Railroad = 9,
	Tunnel = 10,
	Weeds = 11
};

/*
 *
 * Some helpers
 *  let W = a result of creepy maths = 104 (leptons)
 *  let GH = this->GetHeight()
 *
 *  ObjectClass::InWhichLayer, used by practically all ObjectClass derivates except Foot, returns
 *   Ground if GH < 2*W leptons
 *   Air if it returns < Rules->CruiseHeight leptons
 *   Top otherwise
 *
 * FootClass::InWhichLayer, used by Infantry, Unit and Aircraft, returns results depending on the locomotor
 *  Drive         : Ground
 *  DropPod       : Air
 *  Fly           : Ground if GH <= 0, Top otherwise
 *  Hover         : Ground
 *  Jumpjet       : Ground if GH <= 2*W, Air if it's not at the height of its flight (rising/landing), Top otherwise
 *  Mech          : Ground
 *  Rocket        : Air
 *  Ship          : Ground
 *  Subterrannean : Underground if really underground, Ground if on ground, diving or elevating
 *  Walk          : Ground
 *
 */
enum class Layer : int {
	None = -1,
	Underground = 0,
	Surface = 1,
	Ground = 2,
	Air = 3,
	Top = 4
};

enum class MouseHotSpotX : int {
	Left = 0,
	Center = 12345,
	Right = 54321
};

enum class MouseHotSpotY : int {
	Top = 0,
	Middle = 12345,
	Bottom = 54321
};

enum class Mission : int {
	None = -1,
	Sleep = 0,
	Attack = 1,
	Move = 2,
	QMove = 3,
	Retreat = 4,
	Guard = 5,
	Sticky = 6,
	Enter = 7,
	Capture = 8,
	Eaten = 9,
	Harvest = 10,
	Area_Guard = 11,
	Return = 12,
	Stop = 13,
	Ambush = 14,
	Hunt = 15,
	Unload = 16,
	Sabotage = 17,
	Construction = 18,
	Selling = 19,
	Repair = 20,
	Rescue = 21,
	Missile = 22,
	Harmless = 23,
	Open = 24,
	Patrol = 25,
	ParadropApproach = 26,
	ParadropOverfly = 27,
	Wait = 28,
	AttackMove = 29,
	SpyplaneApproach = 30,
	SpyplaneOverfly = 31,

	count
};

enum class MovementZone : int {
	None = -1,
	Normal = 0,
	Crusher = 1,
	Destroyer = 2,
	AmphibiousDestroyer = 3,
	AmphibiousCrusher = 4,
	Amphibious = 5,
	Subterrannean = 6,
	Infantry = 7,
	InfantryDestroyer = 8,
	Fly = 9,
	Water = 10,
	WaterBeach = 11,
	CrusherAll = 12
};

enum class PipIndex : unsigned int {
	Empty = 0,
	Green = 1,
	Yellow = 2,
	White = 3,
	Red = 4,
	Blue = 5,
	PersonEmpty = 6,
	PersonGreen = 7,
	PersonYellow = 8,
	PersonWhite = 9,
	PersonRed = 10,
	PersonBlue = 11,
	PersonPurple = 12
};

enum class PipScale : unsigned int {
	None = 0,
	Ammo = 1,
	Tiberium = 2,
	Passengers = 3,
	Power = 4,
	MindControl = 5
};

enum class PowerupEffects : unsigned char
 {
	Money = 0,
	Unit = 1,
	HealBase = 2,
	Cloak = 3,
	Explosion = 4,
	Napalm = 5,
	Squad = 6,
	Darkness = 7,
	Reveal = 8,
	Armor = 9,
	Speed = 10,
	Firepower = 11,
	ICBM = 12,
	Invulnerability = 13,
	Veteran = 14,
	IonStorm = 15,
	Gas = 16,
	Tiberium = 17,
	Pod = 18
};

enum class Powerup : unsigned int {
	Money = 0,
	Unit = 1,
	HealBase = 2,
	Cloak = 3,
	Explosion = 4,
	Napalm = 5,
	Squad = 6,
	Darkness = 7,
	Reveal = 8,
	Armor = 9,
	Speed = 10,
	Firepower = 11,
	ICBM = 12,
	Invulnerability = 13,
	Veteran = 14,
	IonStorm = 15,
	Gas = 16,
	Tiberium = 17,
	Pod = 18
};

enum class Prerequisites : int
{
	Proc = -6,
	Tech = -5,
	Radar = -4,
	Barracks = -3,
	Factory = -2,
	Power = -1
};

//typedef enum DirTypes
//{
//	DIR_N = 0,          // 0
//	DIR_NE = 1 << 5,    // 32
//	DIR_E = 2 << 5,     // 64
//	DIR_SE = 3 << 5,    // 96
//	DIR_S = 4 << 5,     // 128
//	DIR_SW = 5 << 5,    // 160
//	DIR_W = 6 << 5,     // 192
//	DIR_NW = 7 << 5,    // 224
//
//	DIR_MIN = 0,
//	DIR_MAX = 255,
//
//	DIR_SW_X1 = DirTypes((5 << 5) - 8),  // 152      // Direction of harvester while unloading.
//	DIR_SW_X2 = DirTypes((5 << 5) - 16), // 144      // Direction of harvester while unloading.
//} DirTypes;
//DEFINE_ENUMERATION_OPERATORS(DirTypes);

enum class PrismChargeState : int {
	Idle = 0,
	Master = 1,
	Slave = 2
};

enum class RadarEventType : int {
	None = -1,
	Combat = 0,
	Noncombat = 1,
	DropZone = 2,
	BaseAttacked = 3,
	HarvesterAttacked = 4,
	EnemySensed = 5,
	UnitProduced = 6,
	UnitLost = 7,
	UnitRepaired = 8,
	BuildingInfiltrated = 9,
	BuildingCaptured = 10,
	BeaconPlaced = 11,
	SuperweaponDetected = 12,
	SuperweaponActivated = 13,
	BridgeRepaired = 14,
	GarrisonAbandoned = 15,
	AllyBaseAttacked = 16,
	Count
};

enum class PsychicDominatorStatus : unsigned int {
	Inactive = 0,
	FirstAnim = 1,
	Fire = 2,
	SecondAnim = 3,
	Reset = 4,
	Over = 5
};

enum class NukeFlashStatus : unsigned int {
	Inactive = 0,
	FadeIn = 1,
	FadeOut = 2
};

enum class ChargeDrainState : int {
	None = -1,
	Charging = 0,
	Ready = 1,
	Draining = 2
};

enum class SuperWeaponType : int {
	Invalid = -1,
	Nuke = 0,
	IronCurtain = 1,
	LightningStorm = 2,
	ChronoSphere = 3,
	ChronoWarp = 4,
	ParaDrop = 5,
	AmerParaDrop = 6,
	PsychicDominator = 7,
	SpyPlane = 8,
	GeneticMutator = 9,
	ForceShield = 10,
	PsychicReveal = 11,

	count
};

enum class MouseCursorType : unsigned int {
	Default = 0x0,
	Move_N = 0x1,
	Move_NE = 0x2,
	Move_E = 0x3,
	Move_SE = 0x4,
	Move_S = 0x5,
	Move_SW = 0x6,
	Move_W = 0x7,
	Move_NW = 0x8,
	NoMove_N = 0x9,
	NoMove_NE = 0xA,
	NoMove_E = 0xB,
	NoMove_SE = 0xC,
	NoMove_S = 0xD,
	NoMove_SW = 0xE,
	NoMove_W = 0xF,
	NoMove_NW = 0x10,
	Select = 0x11,
	Move = 0x12,
	NoMove = 0x13,
	Attack = 0x14,
	AttackOutOfRange = 0x15,
	Protect = 0x16,
	DesolatorDeploy = 0x17,
	Cursor_18 = 0x18,
	Enter = 0x19,
	NoEnter = 0x1A,
	Deploy = 0x1B,
	NoDeploy = 0x1C,
	Cursor_1D = 0x1D,
	Sell = 0x1E,
	SellUnit = 0x1F,
	NoSell = 0x20,
	Repair = 0x21,
	EngineerRepair = 0x22,
	NoRepair = 0x23,
	Waypoint = 0x24,
	Disguise = 0x25,
	IvanBomb = 0x26,
	MindControl = 0x27,
	RemoveSquid = 0x28,
	Crush = 0x29,
	SpyTech = 0x2A,
	SpyPower = 0x2B,
	Cursor_2C = 0x2C,
	GIDeploy = 0x2D,
	Cursor_2E = 0x2E,
	ParaDrop = 0x2F,
	Cursor_30 = 0x30, // RallyPoint
	CloseWaypoint = 0x31, // ???
	LightningStorm = 0x32,
	Detonate = 0x33,
	Demolish = 0x34,
	Nuke = 0x35,
	Cursor_36 = 0x36, // BlueMove
	Power = 0x37,
	Cursor_38 = 0x38, // NoBlueMove
	IronCurtain = 0x39,
	Chronosphere = 0x3A,
	Disarm = 0x3B,
	Disallowed = 0x3C,
	Scroll = 0x3D,
	Scroll_ESW = 0x3E,
	Scroll_SW = 0x3F,
	Scroll_NSW = 0x40,
	Scroll_NW = 0x41,
	Scroll_NEW = 0x42,
	Scroll_NE = 0x43,
	Scroll_NES = 0x44,
	Scroll_ES = 0x45,
	Protect2 = 0x46,
	AttackOutOfRange2 = 0x47,
	Cursor_48 = 0x48, // LeaveBuilding
	InfantryAbsorb = 0x49,
	NoMindControl = 0x4A,
	Cursor_4B = 0x4B, // NoRallyPoint
	Cursor_4C = 0x4C,
	Cursor_4D = 0x4D,
	Beacon = 0x4E,
	ForceShield = 0x4F,
	NoForceShield = 0x50,
	GeneticMutator = 0x51,
	AirStrike = 0x52,
	PsychicDominator = 0x53,
	PsychicReveal = 0x54,
	SpyPlane = 0x55 ,

	count
};
MAKE_ENUM_FLAGS(MouseCursorType)

enum class RadBeamType : int {
	Temporal = 0,
	RadBeam = 1,
	Eruption = 2 // this sets the beam color to MagnaBeamColor! There probably was no reason for that whatsoever.
};

enum class Rank : int {
	Invalid = -1,
	Elite = 0,
	Veteran = 1,
	Rookie = 2,
};

enum class DefaultColorList : int
{
	Grey = 0,
	Red = 1,
	Green = 2,
	Blue = 3,
	Yellow = 4,
	White = 5,
	AresPCXTransparent = 6,
	Black = 7,
	count
};

enum class DoType : int {
	Ready = 0,
	Guard = 1,
	Prone = 2,
	Walk = 3,
	FireUp = 4,
	Down = 5,
	Crawl = 6,
	Up = 7,
	FireProne = 8,
	Idle1 = 9,
	Idle2 = 10,
	Die1 = 11,
	Die2 = 12,
	Die3 = 13,
	Die4 = 14,
	Die5 = 15,
	Tread = 16,
	Swim = 17,
	WetIdle1 = 18,
	WetIdle2 = 19,
	WetDie1 = 20,
	WetDie2 = 21,
	WetAttack = 22,
	Hover = 23,
	Fly = 24,
	Tumble = 25,
	FireFly = 26,
	Deploy = 27,
	Deployed = 28,
	DeployedFire = 29,
	DeployedIdle = 30,
	Undeploy = 31,
	Cheer = 32,
	Paradrop = 33,
	AirDeathStart = 34,
	AirDeathFalling = 35,
	AirDeathFinish = 36,
	Panic = 37,
	Shovel = 38,
	Carry = 39,
	SecondaryFire = 40,
	SecondaryProne = 41
};

//MAKE_ENUM_FLAGS(DoType);

enum class DoTypeFacing : int {
	None = -1,
	N = 0,
	NE = 1,
	E = 2,
	SE = 3,
	S = 4,
	SW = 5,
	W = 6,
	NW = 7
};

enum class SpeedType : int {
	None = -1,
	Foot = 0,
	Track = 1,
	Wheel = 2,
	Hover = 3,
	Winged = 4,
	Float = 5,
	Amphibious = 6,
	FloatBeach = 7
};

enum class TheaterType : int {
	None = -1,
	Temperate = 0,
	Snow = 1,
	Urban = 2,
	Desert = 3,
	NewUrban = 4,
	Lunar = 5,
};

//typedef int eVisualType;
enum class VisualType : unsigned int {
	Normal = 0,
	Indistinct = 1,
	Darken = 2,
	Shadowy = 3,
	Ripple = 4,
	Hidden = 5
};

enum class RadioCommand : int {
	AnswerInvalid = 0, // static (no message)
	AnswerPositive = 1, // Roger.
	RequestLink = 2, // Come in.
	NotifyUnlink = 3, // Over and out.
	unknown_4 = 4, // Requesting transport.
	unknown_5 = 5, // Attach to transport.
	unknown_6 = 6, // I've got a delivery for you.
	NotifyBeginLoad = 7, // I'm performing load/unload maneuver. Be careful.
	NotifyUnloaded = 8, // I'm clear.
	RequestUnload = 9, // You are clear to unload. Driving away now.
	AnswerNegative = 10, // Am unable to comply.
	RequestBeginProduction = 11, // I'm starting construction now... act busy.
	RequestEndProduction = 12, // I've finished construction. You are free.
	RequestRedraw = 13, // We bumped, redraw yourself please.
	RequestLoading = 14, // I'm trying to load up now.
	AnswerLoading = 14, // Loading up now.
	QueryCanEnter = 15, // May I become a passenger?
	QueryCanUnload = 16, // Are you ready to receive shipment?
	QueryWantEnter = 17, // Are you trying to become a passenger?
	RequestMoveTo = 18, // Move to location X.
	QueryMoving = 19, // Do you need to move?
	AnswerAwaiting = 20, // All right already. Now what?
	RequestCompleteEnter = 21, // I'm a passenger now.
	RequestDockRefinery = 22, // Backup into refinery now.
	AnswerLeave = 23, // Run away!
	NotifyLeave = 23, // Running away.
	RequestTether = 24, // Tether established.
	RequestUntether = 25, // Tether broken.
	RequestAlternativeTether = 26, // Alternative tether established.
	RequestAlternativeUntether = 27, // Alternative tether broken.
	RequestRepair = 28, // Repair one step.
	QueryReadiness = 29, // Are you prepared to fight?
	RequestAttack = 30, // Attack this target please.
	RequestReload = 31, // Reload one step.
	AnswerBlocked = 32, // Circumstances prevent success.
	QueryDone = 33, // All done with the request?
	AnswerDone = 33, // All done with the request.
	QueryNeedRepair = 34, // Do you need service depot work?
	QueryOnBuilding = 35, // Are you located on top of me?
	QueryCanTote = 36, // Want ride

	count,
};

enum class NetworkEventType : unsigned char {
	Empty = 0x0,
	PowerOn = 0x1,
	PowerOff = 0x2,
	Ally = 0x3,
	MegaMission = 0x4,
	MegaMissionF = 0x5,
	Idle = 0x6,
	Scatter = 0x7,
	KillFootClass = 0x8,
	Deploy = 0x9,
	Detonate = 0xA,
	Place = 0xB,
	Options = 0xC,
	GameSpeed = 0xD,
	Produce = 0xE,
	Suspend = 0xF,
	Abandon = 0x10,
	Primary = 0x11,
	SpecialPlace = 0x12,
	Exit = 0x13,
	Animation = 0x14,
	Repair = 0x15,
	Sell = 0x16,
	SellCell = 0x17,
	Special = 0x18,
	FrameSync = 0x19,
	Message = 0x1A,
	ResponseTime = 0x1B,
	FrameInfo = 0x1C,
	SaveGame = 0x1D,
	Archive = 0x1E,
	AddPlayer = 0x1F,
	Timing = 0x20,
	ProcessTime = 0x21,
	PageUser = 0x22,
	RemovePlayer = 0x23,
	LatencyFudge = 0x24,
	MegaFrameInfo = 0x25,
	PacketTiming = 0x26,
	AboutToExit = 0x27,
	FallbackHost = 0x28,
	AddressChange = 0x29,
	PlanConnect = 0x2A,
	PlanCommit = 0x2B,
	PlanNodeDelete = 0x2C,
	AllCheer = 0x2D,
	AbandonAll = 0x2E,

	count
};

// Sound specific
enum class SoundPriority : int {
	Lowest = 0,
	Low = 1,
	Normal = 2,
	High = 3,
	Critical = 4
};

enum class SoundType : unsigned int {
	Normal = 0x0,
	Violent = 0x1,
	Movement = 0x2,
	Quiet = 0x4,
	Loud = 0x8,
	Global = 0x10,
	Screen = 0x20,
	Local = 0x40,
	Player = 0x80,
	NoiseShy = 0x100,
	GunShy = 0x200,
	Unshroud = 0x400,
	Shroud = 0x800,
	Ambient = 0x1000
};

enum class SoundControl : unsigned int {
	None = 0x0,
	Loop = 0x1,
	Random = 0x2,
	All = 0x4,
	Predelay = 0x8,
	Interrupt = 0x10,
	Attack = 0x20,
	Decay = 0x40,
	Ambient = 0x80
};

enum class VoxType : int {
	none = -1,
	Standard = 0,
	Queue = 1,
	Interrupt = 2,
	QueuedInterrupt = 3
};

enum class VoxPriority : int {
	none = -1,
	Low = 0,
	Normal = 1,
	Important = 2,
	Critical = 3
};

enum class WaveType : int {
	Sonic = 0,
	BigLaser = 1,
	Laser = 2,
	Magnetron = 3
};

enum class QuarryType : unsigned int
{
	None,
	Anything,						// Attack any enemy (same as "hunt").
	Buildings,						// Attack buildings (in general).
	Harvesters,						// Attack harvesters or refineries.
	Infantry,						// Attack infantry.
	Vehicles,						// Attack combat vehicles.
	Factories,						// Attack factories (all types).
	Defenses,						// Attack base defense buildings.
	Threats,						// Attack enemies near friendly base.
	Power,							// Attack power facilities.
	OccupiableBuildings,			// Prefer to attack occupiable buildings.
	TechBuildings					// Prefer to attack tech buildings.
};


enum class BlitterFlags : unsigned int {
	None = 0x0,
	Darken = 0x1,
	TransLucent25 = 0x2,
	TransLucent50 = 0x4,
	TransLucent75 = 0x6,
	Warp = 0x8,
	ZRemap = 0x10,
	Plain = 0x20,
	bf_040 = 0x40,
	bf_080 = 0x80,
	MultiPass = 0x100,
	Centered = 0x200,
	bf_400 = 0x400,
	Alpha = 0x800,
	bf_1000 = 0x1000,
	Flat = 0x2000,
	ZRead = 0x3000,
	ZReadWrite = 0x4000,
	bf_8000 = 0x8000,
	Zero = 0x10000,
	Nonzero = 0x20000
};

MAKE_ENUM_FLAGS(BlitterFlags)
// UI
enum class MouseEvent : unsigned char {
	None = 0x0,
	LeftDown = 0x1,
	LeftHeld = 0x2,
	LeftUp = 0x4,
	Move = 0x8,
	RightDown = 0x10,
	RightHeld = 0x20,
	RightUp = 0x40
};

MAKE_ENUM_FLAGS(MouseEvent)

//control key flags
typedef DWORD eControlKeyFlags;

#define ckf_SHIFT 0x01
#define ckf_CTRL 0x02
#define ckf_ALT 0x04


enum class Edge : int {
	None = -1,
	North = 0,
	East = 1,
	South = 2,
	West = 3,
	Air = 4
};

enum class Move : int {
	OK = 0,
	Cloak = 1,
	MovingBlock = 2,
	ClosedGate = 3,
	FriendlyDestroyable = 4,
	Destroyable = 5,
	Temp = 6,
	No = 7
};

enum class ZGradient : int {
	None = -1,
	Ground = 0,
	Deg45 = 1,
	Deg90 = 2,
	Deg135 = 3
};

enum class ParasiteState : int {
	Start = 0, // creates grab animation
	Grab = 1, // wait for the grab anim
	PushLeft = 2, // push the victim, variant A
	PushRight = 3, // push the victim, variant B
	Damage = 4 // wait until rocking stops; deliver damage
};

enum class WWKey : int {
	Shift = 0x100,
	Ctrl = 0x200,
	Alt = 0x400,
	Release = 0x800,
	VirtualKey = 0x1000,
	DoubleClick = 0x2000,
	Button = 0x8000,
};
MAKE_ENUM_FLAGS(WWKey)

enum class TeamMissionType : int
{
	Attack = 0,
	Att_waypt = 1,
	Go_bezerk = 2,//ts
	Move = 3,
	Movecell = 4,
	Guard = 5,
	Loop = 6,
	Player_wins = 7,//ts
	Unload = 8,
	Deploy = 9,
	Hound_dog = 10,
	Do = 11,
	Set_global = 12,
	Idle_anim = 13,//ts
	Load = 14,
	Spy = 15,
	Patrol = 16,
	Change_script = 17,//ts
	Change_team = 18,//ts
	Panic = 19,//ts
	Change_house = 20,//ts
	Scatter = 21,//ts
	Goto_nearby_shroud = 22,//ts
	Player_loses = 23,//ts
	Play_speech = 24,//ts
	Play_sound = 25,//ts
	Play_movie = 26,//ts
	Play_music = 27,//ts
	Reduce_tiberium = 28,//ts
	Begin_production = 29,//ts
	Fire_sale = 30,//ts
	Self_destruct = 31,//ts
	Ion_storm_start_in = 32,//ts
	Ion_storn_end = 33,//ts
	Center_view_on_team = 34,//ts
	Reshroud_map = 35,//ts
	Reveal_map = 36,//ts
	Delete_team_members = 37,//ts
	Clear_global = 38,//ts
	Set_local = 39,//ts
	Clear_local = 40,//ts
	Unpanic = 41,//ts
	Force_facing = 42,//ts
	Wait_till_fully_loaded = 43,//ts
	Truck_unload = 44,//ts
	Truck_load = 45,//ts
	Attack_enemy_building = 46,//ts
	Moveto_enemy_building = 47,//ts
	Scout = 48,//ts
	Success = 49,//ts
	Flash = 50,//ts
	Play_anim = 51,//ts
	Talk_bubble = 52,//ts
	Gather_at_enemy = 53,//ra2
	Gather_at_base = 54,//ra2
	Iron_curtain_me = 55,//ra2
	Chrono_prep_for_abwp = 56,//ra2
	Chrono_prep_for_aq = 57,//ra2
	Move_to_own_building = 58,//ra2
	Attack_building_at_waypoint = 59, //yr
	Enter_grinder = 60, //yr
	Occupy_tank_bunker = 61, //yr
	Enter_bio_reactor = 62, //yr
	Occupy_battle_bunker = 63, //yr
	Garrison_building = 64, //yr

	count,
	none = -1,
};

//Westwood custom messages (e.g. for SendMessage)
#define	WW_SLIDER_GETVALUE			0x400

#define	WW_SLIDER_SETVALUE			0x405
#define	WW_SLIDER_SETRANGE			0x406

#define	WW_INITDIALOG				0x497
#define	WW_SETCOLOR					0x498

#define WW_BLITDDS					0x4A8 //lParam is some structure

#define	WW_STATIC_SETTEXT			0x4B2

#define WW_CB_GETITEMINDEX			0x4BE //wParam = 0; lParam = wchar_t* text;

#define	WW_CB_ADDITEM				0x4C2 //wParam = int index; lParam = 0

#define	WW_LB_ADDITEM				0x4CD

#define		COLOR_BLACK  0x0000
#define		COLOR_WHITE  0xFFFF

#define		COLOR_RED    0xF800
#define		COLOR_GREEN  0x07E0
#define		COLOR_BLUE   0x001F

#define		COLOR_PURPLE (COLOR_RED | COLOR_BLUE)
#define		COLOR_PURPLE (COLOR_RED | COLOR_BLUE)
#define		COLOR_YELLOW (COLOR_RED | COLOR_GREEN)
#define		COLOR_CYAN   (COLOR_BLUE | COLOR_GREEN)