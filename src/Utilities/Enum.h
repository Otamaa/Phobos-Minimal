#pragma region Ares Copyrights
/*
 *Copyright (c) 2008+, All Ares Contributors
 *All rights reserved.
 *
 *Redistribution and use in source and binary forms, with or without
 *modification, are permitted provided that the following conditions are met:
 *1. Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *2. Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *3. All advertising materials mentioning features or use of this software
 *   must display the following acknowledgement:
 *   This product includes software developed by the Ares Contributors.
 *4. Neither the name of Ares nor the
 *   names of its contributors may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 *THIS SOFTWARE IS PROVIDED BY ITS CONTRIBUTORS ''AS IS'' AND ANY
 *EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *DISCLAIMED. IN NO EVENT SHALL THE ARES CONTRIBUTORS BE LIABLE FOR ANY
 *DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#pragma endregion

#pragma once

#include <GeneralDefinitions.h>

enum class SpotlightAttachment
{
	Body, Turret, Barrel
};

enum class SuperWeaponTarget : unsigned char
{
	None = 0x0,
	Land = 0x1,
	Water = 0x2,
	Empty = 0x4,
	Infantry = 0x8,
	Unit = 0x10,
	Building = 0x20,

	All = 0xFF,
	AllCells = Land | Water,
	AllTechnos = Infantry | Unit | Building,
	AllContents = Empty | AllTechnos
};
MAKE_ENUM_FLAGS(SuperWeaponTarget);

enum class TargetingConstraint : int
{
	None = 0x0,
	OffensiveCellClear = 0x1,
	DefensifeCellClear = 0x2,
	Enemy = 0x4,
	LighningStormInactive = 0x8,
	DominatorInactive = 0x10,
	Attacked = 0x20,
	LowPower = 0x40,
	OffensiveCellSet = 0x80,
	DefensiveCellSet = 0x0100,
};
MAKE_ENUM_FLAGS(TargetingConstraint);

enum class TargetingPreference : char
{
	None = 0, Offensive, Devensive
};

//TODO HelpText
enum class AresNewTriggerAction : int
{
	AuxiliaryPower = 146,
	KillDriversOf = 147,
	SetEVAVoice = 148,
	SetGroup = 149
};

enum class AresTriggerEvents : int
{
	UnderEMP = 62,
	UnderEMP_ByHouse = 63 ,
	RemoveEMP = 64 ,
	RemoveEMP_ByHouse = 65 ,
	EnemyInSpotlightNow = 66,
	DriverKiller = 67 ,
	DriverKilled_ByHouse = 68 ,
	VehicleTaken = 69 ,
	VehicleTaken_ByHouse = 70 , 
	Abducted = 71 , 
	Abducted_ByHouse = 72 ,
	AbductSomething = 73,
	AbductSomething_OfHouse = 74,
	SuperActivated = 75 , 
	SuperDeactivated = 76 ,
	SuperNearWaypoint = 77 ,
	ReverseEngineered = 78 ,
	ReverseEngineerAnything = 79 ,
	ReverseEngineerType = 80 ,
	HouseOwnTechnoType = 81 , 
	HouseDoesntOwnTechnoType = 82 ,
	AttackedOrDestroyedByAnybody = 83 , 
	AttackedOrDestroyedByHouse = 84 , 
	DestroyedByHouse = 85 ,
	TechnoTypeDoesntExistMoreThan = 86 ,
	AllKeepAlivesDestroyed = 87,
	AllKeppAlivesBuildingDestroyed = 88 ,
};

enum class NewMouseCursorType : unsigned int
{
	Tote = 0,
	EngineerDamage,
	TogglePower,
	NoTogglePower,
	InfantryHeal,
	UnitRepair,
	TakeVehicle,
	Sabotage,
	RepairTrench,
	count,
};

enum class IronCurtainFlag : int
{
	Default = -1,
	Kill = 0,
	Invulnerable = 1,
	Ignore = 2,
	Random = 3,
};

enum class AreaFireReturnFlag : char
{
	Continue = 0,
	ContinueAndReturn,
	DoNotFire ,
	SkipSetTarget,
};

enum class PhobosAbilityType : int
{
	None = -1,

	Interceptor = 0,
	ChronoDelayDamageImmune = 1,
	CritImmune = 2,
	PsionicsImmune = 3,
	CullingImmune = 4,
	EmpImmune = 5,
	RadImmune = 6,
	Protected_Driver = 7,
	Unwarpable = 8,
	PoisonImmune = 9,
	PsionicsWeaponImmune = 10,
	BerzerkImmune = 11,
	AbductorImmune = 12,
	Assaulter = 13,

	count
};

enum class TargetZoneScanType : int
{
	Same = 0,
	Any = 1,
	InRange = 2,

	count
};

enum class DamageDelayTargetFlag : int
{
	Cell = 0,
	AttachedObject = 1,
	Invoker = 2,
	
	count
};

enum class FullMapDetonateResult : int
{
	TargetNotDamageable,
	TargetNotEligible,
	TargetHouseNotEligible,
	TargetRestricted,
	TargetValid,

	count
};

enum class TransactValueType : int
{
	Experience = 0,
	// Other....
};

enum class AttachedAnimFlag : int {
	None = 0x0,
	Hides = 0x1,
	Temporal = 0x2,
	Paused = 0x4,

	PausedTemporal = Paused | Temporal
};

MAKE_ENUM_FLAGS(AttachedAnimFlag);

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

enum class AirAttackStatusIDB : int
{
	AIR_ATT_VALIDATE_AZ = 0x0,
	AIR_ATT_PICK_ATTACK_LOCATION = 0x1,
	AIR_ATT_TAKE_OFF = 0x2,
	AIR_ATT_FLY_TO_POSITION = 0x3,
	AIR_ATT_FIRE_AT_TARGET0 = 0x4,
	AIR_ATT_FIRE_AT_TARGET1 = 0x5,
	AIR_ATT_FIRE_AT_TARGET2 = 0x6,
	AIR_ATT_FIRE_AT_TARGET3 = 0x7,
	AIR_ATT_FIRE_AT_TARGET4 = 0x8,
	AIR_ATT_FIRE_AT_TARGET5 = 0x9,
	AIR_ATT_RETURN_TO_BASE = 0xA,
};

enum class SuperWeaponAITargetingMode
{
	None = 0,
	Nuke = 1,
	LightningStorm = 2,
	PsychicDominator = 3,
	ParaDrop = 4,
	GeneticMutator = 5,
	ForceShield = 6,
	NoTarget = 7,
	Offensive = 8,
	Stealth = 9,
	Self = 10,
	Base = 11,
	MultiMissile = 12,
	HunterSeeker = 13,
	EnemyBase = 14,
	IronCurtain = 15,
	Attack = 16,
	LowPower = 17,
	LowPowerAttack = 18,
	Droppod = 19,
	LighningRandom = 20
};

enum class AffectedTarget : unsigned char {
	None = 0x0,
	Land = 0x1,
	Water = 0x2,
	NoContent = 0x4,
	Infantry = 0x8,
	Unit = 0x10,
	Building = 0x20,
	Aircraft = 0x40,

	All = 0xFF,
	AllCells = Land | Water,
	AllTechnos = Infantry | Unit | Building | Aircraft,
	AllContents = NoContent | AllTechnos
};

MAKE_ENUM_FLAGS(AffectedTarget);

enum class AffectedHouse : unsigned char {
	None = 0x0,
	Owner = 0x1,
	Allies = 0x2,
	Enemies = 0x4,

	Team = Owner | Allies,
	NotAllies = Owner | Enemies,
	NotOwner = Allies | Enemies,
	All = 0xA
};

MAKE_ENUM_FLAGS(AffectedHouse);

enum class OwnerHouseKind : int {
	Default = 0,
	Invoker = 1,
	Killer = 2,
	Victim = 3,
	Civilian = 4,
	Special = 5,
	Neutral = 6,
	Random = 7
};

enum class SuperWeaponFlags : unsigned short {
	None = 0x0,
	NoAnim = 0x1,
	NoSound = 0x2,
	NoEvent = 0x4,
	NoEVA = 0x8,
	NoMoney = 0x10,
	NoCleanup = 0x20,
	NoMessage = 0x40,
	PreClick = 0x80,
	PostClick = 0x100
};

enum class AreaFireTarget
{
	Base = 0,
	Self = 1,
	Random = 2
};

enum class PhobosAction {
	None = 0,
	Hijack = 1,
	Drive = 2
};

enum class SelfHealGainType
{
	None = 0,
	Infantry = 1,
	Units = 2
};

enum class TextAlign : int
{
	None = 0xFFF,
	Left = 0x000,
	Center = 0x100,
	Right = 0x200,
};

enum class HorizontalPosition
{
	Left = 0,
	Center = 1,
	Right = 2
};

enum class VerticalPosition
{
	Top = 0,
	Center = 1,
	Bottom = 2
};

enum class FeedBackType
{
	WeaponFire = 0 ,
	HealthLevel = 1,
	ReceiveDamage = 2
};

enum class InterceptedStatus : int
{
	None = 0,
	Targeted = 1,
	Intercepted = 2
};

enum class SlaveReturnTo : int
{
	Killer = 0, //default
	Master = 1,
	Suicide = 2,
	Neutral = 3,
	Civilian = 4,
	Special = 5,
	Random = 6
};

enum class KillMethod : int
{
	None = -1 ,
	Explode = 0,     //default death option
	Vanish = 1,
	Sell = 2,     // buildings only
	Random = 3
};

enum class BannerNumberType : int
{
	None = 0,
	Variable = 1,
	Prefixed = 2,
	Suffixed = 3,
	Fraction = 4
};
#pragma region Otamaa

enum class AircraftFireMode : int
{
	FireAt = 0,
	Strafe2 = 1,
	Strafe3 = 2,
	Strafe4	= 3,
	Strafe5 = 4
};

#pragma endregion