#include "Body.h"

#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/Terrain/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/Side/Body.h>

#include <TerrainTypeClass.h>
#include <New/Type/ArmorTypeClass.h>
//#include <Lib/gcem/gcem.hpp>

#include <Notifications.h>

#include <TEventClass.h>
#include <TActionClass.h>
}

ASMJIT_PATCH(0x41E893, AITriggerTypeClass_ConditionMet_SideIndex, 0xA)
{
	GET(HouseClass*, House, EDI);
	GET(int, triggerSide, EAX);

	enum { Eligible = 0x41E8D7, NotEligible = 0x41E8A1 };

	if (!triggerSide) {
		return Eligible;
	}

	return((triggerSide - 1) == House->SideIndex)
		? Eligible
		: NotEligible
		;
}

#include <Ext/TEvent/Body.h>

TriggerAttachType __fastcall FakeTActionClass::AttachesTo(int type)
{
	//TODO : Phobos TAction has no proper AttachesTo set 
	// 
	//0x6E3EE0, TActionClass_GetFlags, 5
	std::pair<TriggerAttachType, bool> _result = TActionExtData::GetTriggetAttach((AresNewTriggerAction)type);

	if (_result.second) {
		return (_result.first);
	}

	switch ((TriggerAction)type)
	{
		
	case TriggerAction::ChangeHouse:
	case TriggerAction::DestroyAttachedObject:
	case TriggerAction::SellBuilding:
	case TriggerAction::TurnOffBuilding:
	case TriggerAction::TurnOnBuilding:
	case TriggerAction::GoBerzerk:
	case TriggerAction::EvictOccupiers:
		return TriggerAttachType::Object;
	default:
		return TriggerAttachType::None;
	}
}

DEFINE_FUNCTION_JUMP(LJMP, 0x6E3EE0 , FakeTActionClass::AttachesTo)

LogicNeedType __fastcall  FakeTActionClass::ActionNeeds(int type)
{
	//TODO : Phobos TAction has no proper GetLogicNeed set 

	std::pair<LogicNeedType, bool> _result = TActionExtData::GetLogicNeed((AresNewTriggerAction)type);

	if (_result.second) {
		return (_result.first);
	}

	switch (TriggerAction(type))
	{
		// --- LogicNeedType::House ---
	case TriggerAction::Win:
	case TriggerAction::Lose:
	case TriggerAction::AllToHunt:
	case TriggerAction::FireSale:
	case TriggerAction::AutocreateBegins:
	case TriggerAction::ChangeHouse:
	case TriggerAction::AllChangeHouse:
	case TriggerAction::MakeAlly:
	case TriggerAction::MakeEnemy:
	case TriggerAction::AITriggersBegin:
	case TriggerAction::AITriggersStop:
	case TriggerAction::MakeHouseCheer:
	case TriggerAction::DestroyAll:
	case TriggerAction::DestroyAllBuildings:
	case TriggerAction::DestroyAllLandUnits:
	case TriggerAction::DestroyAllNavalUnits:
	case TriggerAction::MindControlBase:
	case TriggerAction::RestoreMindControlledBase:
	case TriggerAction::RestoreStartingUnits:
	case TriggerAction::RestoreStartingBuildings:
		return LogicNeedType::House;

		// --- LogicNeedType::Team ---
	case TriggerAction::CreateTeam:
	case TriggerAction::DestroyTeam:
	case TriggerAction::Reinforcement:
	case TriggerAction::TalkBubble:
		return LogicNeedType::Team;

		// --- LogicNeedType::Waypoint ---
	case TriggerAction::DropZoneFlare:
	case TriggerAction::RevealAroundWaypoint:
	case TriggerAction::RevealWaypointZone:
	case TriggerAction::ReduceTiberium:
	case TriggerAction::Apply100Damage:
	case TriggerAction::SmallLightFlash:
	case TriggerAction::MediumLightFlash:
	case TriggerAction::LargeLightFlash:
	case TriggerAction::RemoveParticleAnim:
	case TriggerAction::LightningStrike:
	case TriggerAction::IonCannonStrike:
	case TriggerAction::NukeStrike:
	case TriggerAction::ChemMissileStrike:
	case TriggerAction::ReshroudMapAtWaypoint:
	case TriggerAction::LightningStormStrike:
	case TriggerAction::IronCurtain:
	case TriggerAction::CenterCameraAtWaypoint:
	case TriggerAction::StopSounds:
	case TriggerAction::TeleportAll:
	case TriggerAction::SetPreferredTargetCell:
	case TriggerAction::SetBaseCenterCell:
	case TriggerAction::SetDefensiveTargetCell:
		return LogicNeedType::Waypoint;

		// --- LogicNeedType::Movie ---
	case TriggerAction::PlayMovie:
	case TriggerAction::PlayIngameMovie:
	case TriggerAction::PlayIngameMovieAndPause:
		return LogicNeedType::Movie;

		// --- LogicNeedType::Text ---
	case TriggerAction::TextTrigger:
	case TriggerAction::TimerText:
		return LogicNeedType::Text;

		// --- LogicNeedType::Trigger ---
	case TriggerAction::DestroyTrigger:
	case TriggerAction::ForceTrigger:
	case TriggerAction::EnableTrigger:
	case TriggerAction::DisableTrigger:
		return LogicNeedType::Trigger;

		// --- LogicNeedType::Sound ---
	case TriggerAction::PlaySoundEffect:
	case TriggerAction::PlaySoundEffectRandom:
		return LogicNeedType::Sound;

		// --- LogicNeedType::Theme ---
	case TriggerAction::PlayMusicTheme:
		return LogicNeedType::Theme;

		// --- LogicNeedType::Speech ---
	case TriggerAction::PlaySpeech:
		return LogicNeedType::Speech;

		// --- LogicNeedType::Number ---
	case TriggerAction::ChangeZoomLevel:
	case TriggerAction::IonStormStart:
	case TriggerAction::SetAmbientLight:
	case TriggerAction::RatioOfAITriggerTeams:
	case TriggerAction::RatioOfTeamAircraft:
	case TriggerAction::RatioOfTeamInfantry:
	case TriggerAction::RatioOfTeamUnits:
	case TriggerAction::WakeupGroup:
	case TriggerAction::PauseGame:
	case TriggerAction::SetTabTo:
	case TriggerAction::StartChronoScreenEffect:
	case TriggerAction::BlackoutRadar:
	case TriggerAction::RetintRed:
	case TriggerAction::RetintGreen:
	case TriggerAction::RetintBlue:
		return LogicNeedType::Number;

		// --- LogicNeedType::Global ---
	case TriggerAction::GlobalSet:
	case TriggerAction::GlobalClear:
		return LogicNeedType::Global;

		// --- LogicNeedType::Bool ---
	case TriggerAction::AutoBaseBuilding:
	case TriggerAction::VeinGrowth:
	case TriggerAction::TiberiumGrowth:
	case TriggerAction::IceGrowth:
		return LogicNeedType::Bool;

		// --- LogicNeedType::Special ---
	case TriggerAction::AddOneTimeSuperWeapon:         // VERIFY: 0x21
	case TriggerAction::AddRepeatingSuperWeapon:   // VERIFY: 0x22
	case TriggerAction::SuperWeaponReset:
		return LogicNeedType::Special;

		// --- LogicNeedType::Quarry ---
	case TriggerAction::PreferredTarget:
		return LogicNeedType::Quarry;

		// --- LogicNeedType::Rectangle ---
	case TriggerAction::ResizePlayerView:
		return LogicNeedType::Rectangle;

		// --- LogicNeedType::AnimNWaypoint ---
	case TriggerAction::PlayAnimAt:
		return LogicNeedType::AnimNWaypoint;

		// --- LogicNeedType::WaypointNWeapon ---
	case TriggerAction::DoExplosionAt:
		return LogicNeedType::WaypointNWeapon;

		// --- LogicNeedType::MeteorSize ---
	case TriggerAction::CreateVoxelAnim:           // 0x2B — VERIFY: maps to MeteorSize in byte table
		return LogicNeedType::MeteorSize;

		// --- LogicNeedType::VelocityNWaypoint ---
	case TriggerAction::MoveCameraToWaypoint:      // 0x30
		return LogicNeedType::VelocityNWaypoint;

		// --- LogicNeedType::LightBehavior ---
	case TriggerAction::ChangeLightBehavior:
		return LogicNeedType::LightBehavior;

		// --- LogicNeedType::RadarEventNWaypoint ---
	case TriggerAction::CreateRadarEvent:
		return LogicNeedType::RadarEventNWaypoint;

		// --- LogicNeedType::Local ---
	case TriggerAction::LocalSet:
	case TriggerAction::LocalClear:
		return LogicNeedType::Local;

		// --- LogicNeedType::Tag ---
	case TriggerAction::DestroyTag:
		return LogicNeedType::Tag;

		// --- LogicNeedType::ShowerNWaypoint ---
	case TriggerAction::MeteorShower:
		return LogicNeedType::ShowerNWaypoint;

		// --- LogicNeedType::TeamNWaypoint ---
	case TriggerAction::ReinforcementAt:
	case TriggerAction::ReinforcementByChrono:
		return LogicNeedType::TeamNWaypoint;

		// --- LogicNeedType::Float ---
	case TriggerAction::SetAmbientStep:
	case TriggerAction::SetAmbientRate:
		return LogicNeedType::Float;

		// --- LogicNeedType::SoundNWaypoint ---
	case TriggerAction::PlaySoundEffectAtWaypoint:
		return LogicNeedType::SoundNWaypoint;

		// --- LogicNeedType::ParticleNWaypoint ---
	case TriggerAction::ParticleAnim:
		return LogicNeedType::ParticleNWaypoint;

		// --- LogicNeedType::Team2 ---
	case TriggerAction::FlashTeam:
		return LogicNeedType::Team2;

		// --- LogicNeedType::NumberNTech ---
	case TriggerAction::SetObjectTechLevel:
	case TriggerAction::FlashCameo:
		return LogicNeedType::NumberNTech;

		// --- LogicNeedType::CrateNWaypoint ---
	case TriggerAction::CreateCrate:
		return LogicNeedType::CrateNWaypoint;

		// --- LogicNeedType::BuildingNWaypoint ---
	case TriggerAction::CreateBuilding:
		return LogicNeedType::BuildingNWaypoint;

		// --- LogicNeedType::NumberNSuper ---
	case TriggerAction::SetSuperWeaponCharge:
	case TriggerAction::SuperWeaponSetRechargeTime:
	case TriggerAction::SuperWeaponResetRechargeTime:
		return LogicNeedType::NumberNSuper;

		// --- LogicNeedType::BuildingNNumber ---
	case TriggerAction::FlashBuildingsOfType:
		return LogicNeedType::BuildingNNumber;

		// --- LogicNeedType::None ---
		// Covers: unassigned slots (0x22 in byte table), actions > 0x90,
		// and actions with no parameter need:
		// AllowWin, RevealAllMap, TimerStart/Stop/Extend/Shorten,
		// GrowShroud, DestroyAttachedObject, IonStormStop, LockInput,
		// UnlockInput, ZoomIn/Out, SellBuilding, TurnOff/OnBuilding,
		// AnnounceWin/Lose, ForceEnd, WakeupSelf/AllSleepers/AllHarmless,
		// GoBerzerk, Activate/DeactivateFirestorm, ToggleTrainCargo,
		// EvictOccupiers, ClearAllSmudges, ClearPreferredTargetCell,
		// ClearBaseCenterCell, ClearDefensiveTargetCell, JumpCameraHome
	default:
		return LogicNeedType::None;
	}
}

DEFINE_FUNCTION_JUMP(LJMP, 0x6E3B60, FakeTActionClass::ActionNeeds)
