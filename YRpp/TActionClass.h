#pragma once

#include <AbstractClass.h>
#include <GeneralDefinitions.h>
#include <RectangleStruct.h>

//forward declarations
class SuperClass;
class TechnoClass;
class TagTypeClass;
class TriggerTypeClass;
class TriggerClass;
class TeamTypeClass;
class HouseClass;
class ObjectClass;

class DECLSPEC_UUID("4F0EC392-0A55-11D2-ACA7-006008055BB5")
	NOVTABLE TActionClass : public AbstractClass
{
public:
	static const AbstractType AbsID = AbstractType::Action;

	//Static
	static COMPILETIMEEVAL constant_ptr<DynamicVectorClass<TActionClass*>, 0xB0E658u> const Array{};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override R0;

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override R0;
	virtual HRESULT __stdcall Save(IStream* pStm,BOOL fClearDirty) override R0;

	//Destructor
	virtual ~TActionClass() RX;

	//AbstractClass
	virtual void PointerExpired(AbstractClass* pAbstract, bool bremoved) override RX;
	virtual AbstractType WhatAmI() const override RT(AbstractType);
	virtual int Size() const override R0;
	virtual void ComputeCRC(CRCEngine& checksum) const override RX;
	virtual int GetArrayIndex() const override R0;

	// you are responsible for doing INI::ReadString and strtok'ing it before calling
	// this func only calls strtok again, doesn't know anything about buffers
	void LoadFromINI()
		{ JMP_THIS(0x6DD5B0); }

	// you allocate the buffer for this, and save it to ini yourself after this returns
	// this func only sprintf's the stuff it needs into buffer
	void PrepareSaveToINI(char *buffer) const
		{ JMP_THIS(0x6DD300); }

	// fuck if I know what's the purpose of this, returns a bitfield of flags for trigger logic
	//  return AttachType
	static int __fastcall GetFlags(int actionKind)
		{ JMP_STD(0x6E3EE0); }

	// transforms actionKind to a number saying what to parse arguments as (team/tag/trigger id, waypoint, integer, etc)
	// return NeedType
	static int __fastcall GetMode(int actionKind)
		{ JMP_STD(0x6E3B60); }

	// main brain, returns whether succeeded (mostly, no consistency in results what so ever)
	// trigger fires all actions regardless of result of this
	bool Occured(HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)
		{ JMP_THIS(0x6DD8B0); }

	// BIG LIST OF EXECUTE'S SLAVE FUNCTIONS - feel free to use

	// NOTE: most of these are defined as separate functions AS WELL AS inlined in Execute() above.
	// Ergo, hooking into them by their address will not always override builtin handling.
	// If you need to know which are inlined, poke me.
#pragma push_macro("ACTION_FUNC")

#define ACTION_FUNC(name, addr) \
	bool name(HouseClass* pTargetHouse, ObjectClass* pSourceObject, TriggerClass* pTrigger, CellStruct* plocation) \
		{ JMP_THIS(addr); }

	ACTION_FUNC(LightningStrike, 0x6E0050)
		ACTION_FUNC(RemoveParticleAnim, 0x6E0080)
		ACTION_FUNC(ParticleAnim, 0x6E0110)
		ACTION_FUNC(WakeupSelf, 0x6E01C0)
		ACTION_FUNC(VeinGrowth, 0x6E0250)
		ACTION_FUNC(TiberiumGrowth, 0x6E0270)
		ACTION_FUNC(IceGrowth, 0x6E0290)
		ACTION_FUNC(WakeupAllSleepers, 0x6E02B0)
		ACTION_FUNC(WakeupAllHarmless, 0x6E0330)
		ACTION_FUNC(WakeupGroup, 0x6E03B0)
		ACTION_FUNC(Win, 0x6E0440)
		ACTION_FUNC(Lose, 0x6E0460)
		ACTION_FUNC(ForceEnd, 0x6E0480)
		ACTION_FUNC(Apply100Damage, 0x6E0490)
		ACTION_FUNC(SmallLightFlash, 0x6E0790)
		ACTION_FUNC(MediumLightFlash, 0x6E07F0)
		ACTION_FUNC(LargeLightFlash, 0x6E0850)
		ACTION_FUNC(SellBuilding, 0x6E08B0)
		ACTION_FUNC(GoBerzerk, 0x6E0930)
		ACTION_FUNC(TurnOffBuilding, 0x6E09A0)
		ACTION_FUNC(TurnOnBuilding, 0x6E0A20)
		ACTION_FUNC(ChangeHouse, 0x6E0AA0)
		ACTION_FUNC(AllChangeHouse, 0x6E0B60)
		ACTION_FUNC(MindControlBase, 0x6E0CA0)
		ACTION_FUNC(RestoreMindControlledBase, 0x6E0D00)
		ACTION_FUNC(TextTrigger, 0x6E0D60)
		ACTION_FUNC(MakeAlly, 0x6E0DF0)
		ACTION_FUNC(MakeEnemy, 0x6E0E60)
		ACTION_FUNC(PreferredTarget, 0x6E0ED0)
		ACTION_FUNC(AutoBaseBuilding, 0x6E0EF0)
		ACTION_FUNC(GrowShroud, 0x6E0F90)
		ACTION_FUNC(GlobalSet, 0x6E0FA0)
		ACTION_FUNC(GlobalClear, 0x6E0FC0)
		ACTION_FUNC(RevealAroundWaypoint, 0x6E0FE0)
		ACTION_FUNC(ReduceTiberium, 0x6E1180)
		ACTION_FUNC(RevealWaypointZone, 0x6E11C0)
		ACTION_FUNC(RevealAllMap, 0x6E1330)
		ACTION_FUNC(TimerStart, 0x6E13A0)
		ACTION_FUNC(TimerStop, 0x6E13E0)
		ACTION_FUNC(TimerExtend, 0x6E1440)
		ACTION_FUNC(TimerShorten, 0x6E14A0)
		ACTION_FUNC(TimerSet, 0x6E1530)
		ACTION_FUNC(TimerText, 0x6E15F0)
		ACTION_FUNC(PlayMovie, 0x6E16D0)
		ACTION_FUNC(PlayIngameMovie, 0x6E1720)
		ACTION_FUNC(PlayIngameMovieAndPause, 0x6E1740)
		ACTION_FUNC(PlaySoundEffect, 0x6E1760)
		ACTION_FUNC(PlaySoundEffectRandom, 0x6E1780)
		ACTION_FUNC(PlaySoundEffectAtWaypoint, 0x6E18B0)
		ACTION_FUNC(StopSounds, 0x6E1980)
		ACTION_FUNC(TeleportAll, 0x6E1A40)
		ACTION_FUNC(ReshroudMapAtWaypoint, 0x6E1A70)
		ACTION_FUNC(PlayMusicTheme, 0x6E1B90)
		ACTION_FUNC(PlaySpeech, 0x6E1BB0)
		ACTION_FUNC(AddOneTimeSuperWeapon, 0x6E1BD0)
		ACTION_FUNC(AddRepeatingSuperWeapon, 0x6E1C40)
		ACTION_FUNC(DropZoneFlare, 0x6E1CC0)
		ACTION_FUNC(AnnounceWin, 0x6E1DC0)
		ACTION_FUNC(AnnounceLose, 0x6E1E00)
		ACTION_FUNC(ProductionBegins, 0x6E1E40)
		ACTION_FUNC(FireSale, 0x6E1EA0)
		ACTION_FUNC(AutocreateBegins, 0x6E1F00)
		ACTION_FUNC(CreateTeam, 0x6E1F60)
		ACTION_FUNC(DestroyTeam, 0x6E1F90)
		ACTION_FUNC(Reinforcement, 0x6E1FB0)
		ACTION_FUNC(ReinforcementAt, 0x6E1FD0)
		ACTION_FUNC(AllToHunt, 0x6E1FF0)
		ACTION_FUNC(DestroyAttachedObject, 0x6E2050)
		ACTION_FUNC(ResizePlayerView, 0x6E21E0)
		ACTION_FUNC(PlayAnimAt, 0x6E2290)
		ACTION_FUNC(DoExplosionAt, 0x6E2390)
		ACTION_FUNC(CreateVoxelAnim, 0x6E2520)
		ACTION_FUNC(IonStormStart, 0x6E2600)
		ACTION_FUNC(IonStormStop, 0x6E2640)
		ACTION_FUNC(LockInput, 0x6E2660)
		ACTION_FUNC(UnlockInput, 0x6E26C0)
		ACTION_FUNC(MoveCameraToWaypoint, 0x6E26D0)
		ACTION_FUNC(CenterCameraAtWaypoint, 0x6E2790)
		ACTION_FUNC(JumpCameraHome, 0x6E2850)
		ACTION_FUNC(ZoomIn, 0x6E2860)
		ACTION_FUNC(ZoomOut, 0x6E28E0)
		ACTION_FUNC(ReshroudMap, 0x6E2950)
		ACTION_FUNC(ChangeLightBehavior, 0x6E2970)
		ACTION_FUNC(DestroyTrigger, 0x6E2A10)
		ACTION_FUNC(DestroyTag, 0x6E2A50)
		ACTION_FUNC(ForceTrigger, 0x6E2AA0)
		ACTION_FUNC(EnableTrigger, 0x6E2AF0)
		ACTION_FUNC(DisableTrigger, 0x6E2B70)
		ACTION_FUNC(CreateRadarEvent, 0x6E2BB0)
		ACTION_FUNC(LocalSet, 0x6E2BE0)
		ACTION_FUNC(LocalClear, 0x6E2C00)
		ACTION_FUNC(ClearAllSmudges, 0x6E2C20)
		ACTION_FUNC(MeteorShower, 0x6E2C40)
		ACTION_FUNC(SetAmbientStep, 0x6E2E20)
		ACTION_FUNC(SetAmbientRate, 0x6E2E40)
		ACTION_FUNC(RetintRed, 0x6E2E60)
		ACTION_FUNC(RetintGreen, 0x6E2EB0)
		ACTION_FUNC(RetintBlue, 0x6E2F00)
		ACTION_FUNC(SetAmbientLight, 0x6E2F50)
		ACTION_FUNC(StartChronoScreenEffect, 0x6E2F90)
		ACTION_FUNC(AITriggersBegin, 0x6E2FA0)
		ACTION_FUNC(AITriggersStop, 0x6E3000)
		ACTION_FUNC(MakeHouseCheer, 0x6E3060)
		ACTION_FUNC(RestoreStartingUnits, 0x6E30C0)
		ACTION_FUNC(RestoreStartingBuildings, 0x6E3120)
		ACTION_FUNC(DestroyAll, 0x6E3180)
		ACTION_FUNC(DestroyAllBuildings, 0x6E31E0)
		ACTION_FUNC(DestroyAllLandUnits, 0x6E3240)
		ACTION_FUNC(DestroyAllNavalUnits, 0x6E32A0)
		ACTION_FUNC(RatioOfAITriggerTeams, 0x6E3300)
		ACTION_FUNC(RatioOfTeamAircraft, 0x6E3320)
		ACTION_FUNC(RatioOfTeamInfantry, 0x6E3340)
		ACTION_FUNC(RatioOfTeamUnits, 0x6E3360)
		ACTION_FUNC(IonCannonStrike, 0x6E3380)
		ACTION_FUNC(NukeStrike, 0x6E3410)
		ACTION_FUNC(LightningStormStrike, 0x6E35F0)
		ACTION_FUNC(IronCurtain, 0x6E36E0)
		ACTION_FUNC(SetObjectTechLevel, 0x6E37F0)
		ACTION_FUNC(ReinforcementByChrono, 0x6E3890)
		ACTION_FUNC(CreateCrate, 0x6E38B0)
		ACTION_FUNC(ChemMissileStrike, 0x6E38F0)
		ACTION_FUNC(ToggleTrainCargo, 0x6E3B20)
		ACTION_FUNC(BlackoutRadar, 0x6E3B40)
		ACTION_FUNC(GetWaypoint, 0x6E3F70)
		ACTION_FUNC(FlashTeam, 0x6E4020)
		ACTION_FUNC(TalkBubble, 0x6E4040)
		ACTION_FUNC(PauseGame, 0x6E4080)
		ACTION_FUNC(EvictOccupiers, 0x6E4090)
		ACTION_FUNC(SetTabTo, 0x6E4100)
		ACTION_FUNC(FlashCameo, 0x6E4150)
		ACTION_FUNC(CreateBuilding, 0x6E4200)
		ACTION_FUNC(SetSuperWeaponCharge, 0x6E42D0)
		ACTION_FUNC(SuperWeaponSetRechargeTime, 0x6E4320)
		ACTION_FUNC(SuperWeaponResetRechargeTime, 0x6E4360)
		ACTION_FUNC(SuperWeaponReset, 0x6E43A0)
		ACTION_FUNC(SetPreferredTargetCell, 0x6E43E0)
		ACTION_FUNC(ClearPreferredTargetCell, 0x6E4440)
		ACTION_FUNC(SetDefensiveTargetCell, 0x6E4460)
		ACTION_FUNC(ClearDefensiveTargetCell, 0x6E44C0)
		ACTION_FUNC(SetBaseCenterCell, 0x6E44E0)
		ACTION_FUNC(ClearBaseCenterCell, 0x6E4540)
		ACTION_FUNC(FlashBuildingsOfType, 0x6E4560)


#undef ACTION_FUNC
#pragma pop_macro("ACTION_FUNC")
	// WHEEEEEW. End of slave functions.

	HouseClass* FindHouseByIndex(TriggerClass* pTrigger, int idxHouse) const
		{ JMP_THIS(0x6E45E0); }

	//Constructor
	TActionClass() noexcept
		: TActionClass(noinit_t())
	{ JMP_THIS(0x6DD000); }

protected:
	explicit __forceinline TActionClass(noinit_t) noexcept
		: AbstractClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:
	int                ArrayIndex;
	TActionClass*      NextAction;
	TriggerAction      ActionKind;
	TeamTypeClass*     TeamType;
	#pragma warning(push)
	#pragma warning (disable : 4201)
	union
	{
		RectangleStruct    Bounds; // map bounds for use with action 40
		struct
		{
			int Param3;
			int Param4;
			int Param5;
			int Param6;
		};
	}; // It's enough for calling Bounds.X, just use a union here now. - secsome
	#pragma warning(pop)
	int                Waypoint;
	int                Value2; // multipurpose
	TagTypeClass*      TagType;
	TriggerTypeClass*  TriggerType;
	char               TechnoID[0x19];
	char               Text[0x20];
	PROTECTED_PROPERTY(BYTE, align_8D[3]);
	int                Value; // multipurpose
};

static_assert(sizeof(TActionClass) == 0x94 , "Invalid Size !");

struct ActionArgs
{
	HouseClass* pHouse;
	ObjectClass* pObject;
	TriggerClass* pTrigger;
	CellStruct* plocation;
};