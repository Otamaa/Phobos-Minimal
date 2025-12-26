#include <Phobos.h>
#include <Phobos.Ext.h>

#include <Ext/Aircraft/Body.h>
#include <Ext/AITriggerType/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Ebolt/Body.h>
#include <Ext/House/Body.h>
#include <Ext/HouseType/Body.h>
#include <Ext/IsometricTileType/Body.h>
#include <Ext/RadSite/Body.h>
#include <Ext/Rules/Body.h>
#include <Ext/Scenario/Body.h>
#include <Ext/Script/Body.h>
#include <Ext/Sidebar/Body.h>
#include <Ext/Side/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/SWType/NewSuperWeaponType/SWStateMachine.h>
#include <Ext/TAction/Body.h>
#include <Ext/Team/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/TerrainType/Body.h>
#include <Ext/Tiberium/Body.h>
#include <Ext/Trigger/Body.h>
#include <Ext/TriggerType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/VoxelAnimType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/ParticleSystem/Body.h>
#include <Ext/ParticleSystemType/Body.h>

//#include <Misc/TriggerMPOwner.h>

#include <New/Type/RadTypeClass.h>
#include <New/Type/LaserTrailTypeClass.h>
#include <New/Type/PaletteManager.h>
#include <New/Type/ArmorTypeClass.h>
#include <New/Type/ColorTypeClass.h>
#include <New/Type/HoverTypeClass.h>
#include <New/Type/TheaterTypeClass.h>
#include <New/Type/CursorTypeClass.h>
#include <New/Type/BannerTypeClass.h>
#include <New/Type/TunnelTypeClass.h>
#include <New/Type/DigitalDisplayTypeClass.h>
#include <New/Type/GenericPrerequisite.h>
#include <New/Type/CrateTypeClass.h>
#include <New/Type/ImmunityTypeClass.h>
#include <New/Type/RocketTypeClass.h>
#include <New/Type/ShieldTypeClass.h>
#include <New/Type/TechTreeTypeClass.h>
#include <New/Type/ThemeTypeClass.h>
#include <New/Type/HealthBarTypeClass.h>
#include <New/Type/BarTypeClass.h>
#include <New/Type/InsigniaTypeClass.h>
#include <New/Type/SelectBoxTypeClass.h>
//#include <New/Type/AttachmentTypeClass.h>

#include <New/Entity/BannerClass.h>
//#include <New/Entity/AttachmentClass.h>

#include <New/HugeBar.h>

#pragma region OtamaaStuffs
#include <Ext/Bomb/Body.h>
#include <Ext/CaptureManager/Body.h>
#include <Ext/Infantry/Body.h>
#include <Ext/InfantryType/Body.h>
#include <Ext/SHPReference/Body.h>
#include <Ext/Cell/Body.h>
#include <Ext/DiskLaser/Body.h>
#include <Ext/Parasite/Body.h>
#include <Ext/Particle/Body.h>
#include <Ext/ParticleType/Body.h>
#include <Ext/Super/Body.h>
#include <Ext/ScriptType/Body.h>
#include <Ext/SpawnManager/Body.h>
#include <Ext/TeamType/Body.h>
#include <Ext/TEvent/Body.h>
#include <Ext/Wave/Body.h>
#include <Ext/Temporal/Body.h>
#include <Ext/Terrain/Body.h>
#include <Ext/SmudgeType/Body.h>
#include <Ext/OverlayType/Body.h>

#include <New/Entity/FlyingStrings.h>
#include <New/Entity/VerticalLaserClass.h>
#include <New/Entity/HomingMissileTargetTracker.h>
#include <New/Entity/SWFirerClass.h>

#include <Ext/Tactical/Body.h>

//#include <New/Entity/FoggedObject.h>
#include <Misc/DynamicPatcher/Trails/TrailType.h>

#include <Misc/Ares/Hooks/Header.h>
#pragma endregion

//#include <New/Entity/BannerClass.h>

#include <Commands/Commands.h>
#include <Misc/PhobosGlobal.h>

#include <utility>
#include <LoadOptionsClass.h>
#include <BeaconManagerClass.h>

HRESULT Phobos::SaveGameDataAfter(IStream* pStm)
{
	Debug::LogInfo("[Phobos] Finished saving the game");
	return S_OK;
}

HRESULT Phobos::LoadGameDataAfter(IStream* pStm)
{
	//clear the loadgame flag
	Phobos::Otamaa::DoingLoadGame = false;

	if (auto pPlayerSide = SideClass::Array->get_or_default(ScenarioClass::Instance->PlayerSideIndex)) {
		if (auto pSideMouse = SideExtContainer::Instance.Find(pPlayerSide)->MouseShape) {
			GameDelete<true, true>(std::exchange(MouseClass::ShapeData(), pSideMouse));
		}
	}

	BeaconManagerClass::Instance->LoadArt();
	Debug::LogInfo("[Phobos] Finished loading the game");
	return S_OK;
}


ASMJIT_PATCH(0x7258DE, AnnounceInvalidPointer_PhobosGlobal, 0x7)
{
	GET(AbstractClass* const, pInvalid, ECX);
	GET(bool const, removed, EDX);
	GET(AbstractType , type, EAX);

	if (Phobos::Otamaa::ExeTerminated)
		return 0;

	//if (MapClass::Instance->Cells.IsInitialized) {
	//	std::for_each(MapClass::Instance->Cells.Items, MapClass::Instance->Cells.Items + MapClass::Instance->Cells.Capacity, [removed, pInvalid](CellClass* pCell) {
	//		 if (pCell){
	//			 pCell->PointerExpired(pInvalid, removed);
	//		 }
	//	});
	//}

	TActionExtData::InvalidatePointer(pInvalid, removed);
	PhobosGlobal::PointerGotInvalid(pInvalid, removed);
	SWStateMachine::PointerGotInvalid(pInvalid, removed);

	AnnounceInvalidPointer(SWTypeExtData::TempSuper, pInvalid);
	AnnounceInvalidPointer(SWTypeExtData::LauchData, pInvalid);

	if (removed) {
		ScenarioExtData::Instance()->UndergroundTracker.erase((TechnoClass*)pInvalid);
		ScenarioExtData::Instance()->FallingDownTracker.erase((TechnoClass*)pInvalid);

		HouseExtContainer::Instance.AutoDeathObjects.erase_all_if([pInvalid](std::pair<TechnoClass*, KillMethod>& item) {
			return item.first == pInvalid;
		});

		HouseExtContainer::Instance.LimboTechno.remove((TechnoClass*)pInvalid);

		if (type == AnimClass::AbsID) {

			ShieldClass::Array.for_each([pInvalid](ShieldClass* pShield) {
				if (pShield->IdleAnim.get() == pInvalid) {
					pShield->IdleAnim.release();
				}
			});

			LightningStorm::CloudsPresent->erase((AnimClass*)pInvalid);
			LightningStorm::CloudsManifesting->erase((AnimClass*)pInvalid);;
			LightningStorm::BoltsPresent->erase((AnimClass*)pInvalid);
		}
	}

	HugeBar::InvalidatePointer(pInvalid, removed);

	return 0;
}

#include <New/Interfaces/AdvancedDriveLocomotionClass.h>
#include <New/Interfaces/LevitateLocomotionClass.h>
#include <New/Interfaces/CustomRocketLocomotionClass.h>

unsigned Phobos::GetVersionNumber() {
	unsigned version = AresGlobalData::InternalVersion + PHOBOSSAVEGAME_ID;

	version += sizeof(AnimExtData);
	version += sizeof(AnimTypeExtData);

	version += sizeof(BuildingExtData);
	version += sizeof(BuildingTypeExtData);

	version += sizeof(BulletExtData);
	version += sizeof(BulletTypeExtData);

	version += sizeof(CellExtData);

	version += sizeof(EboltExtData);

	version += sizeof(HouseExtData);
	version += sizeof(HouseTypeExtData);

	version += sizeof(InfantryExtData);
	version += sizeof(InfantryTypeExtData);

	version += sizeof(ParticleExtData);
	version += sizeof(ParticleTypeExtData);

	version += sizeof(ParticleSystemExtData);
	version += sizeof(ParticleSystemTypeExtData);

	version += sizeof(RadSiteExtData);
	version += sizeof(RulesExtData);
	version += sizeof(ScenarioExtData);
	version += sizeof(SidebarExtData);

	version += sizeof(TeamExtData);
	version += sizeof(SmudgeTypeExtData);

	version += sizeof(SuperExtData);
	version += sizeof(SWTypeExtData);

	version += sizeof(TechnoExtData);
	version += sizeof(TechnoTypeExtData);

	version += sizeof(VoxelAnimExtData);
	version += sizeof(VoxelAnimTypeExtData);

	version += sizeof(WeaponTypeExtData);
	version += sizeof(WarheadTypeExtData);
	version += sizeof(WaveExtData);

	version += sizeof(LaserTrailClass);
	version += sizeof(LauchSWData);

	version += sizeof(TActionExtData);

	version += sizeof(TriggerExtData);

	version += sizeof(ShieldClass);
	version += sizeof(SWFirerClass);

	version += sizeof(AdvancedDriveLocomotionClass);
	version += sizeof(CustomRocketLocomotionClass);
	version += sizeof(LevitateLocomotionClass);
	version += sizeof(GenericPrerequisite);
	version += sizeof(Prereqs);
	version += sizeof(PaletteManager);
	version += sizeof(HugeBar);
	version += sizeof(StaticVars);

	version += sizeof(BannerClass);
	version += sizeof(BannerManagerClass);

	version += sizeof(FlyingStrings);
	version += sizeof(FlyingStrings::ItemSize);

#define AddTypeOf(cccc) version += sizeof(cccc##TypeClass);
		AddTypeOf(Armor)
		AddTypeOf(Banner)
		AddTypeOf(Bar)
		AddTypeOf(Color)
		AddTypeOf(Crate)
		AddTypeOf(Cursor)
		AddTypeOf(DigitalDisplay)
		AddTypeOf(HealthBar)
		AddTypeOf(Hover)
		AddTypeOf(Immunity)
		AddTypeOf(Insignia)
		AddTypeOf(LaserTrail)
		AddTypeOf(Rad)
		AddTypeOf(Rocket)
		AddTypeOf(SelectBox)
		AddTypeOf(Shield)
		AddTypeOf(TechTree)
		AddTypeOf(Theater)
		AddTypeOf(Theme)
		AddTypeOf(Tunnel)
#undef AddTypeOf

	return version;
}

#include <Ext/AircraftType/Body.h>

#include <Ext/Unit/Body.h>
#include <Ext/UnitType/Body.h>

#include <New/Type/ActionTypeClass.h>

#define CLEAR_CONTAIER_CLASS_AND_TYPE(CC) 	CC##ExtContainer::Instance.Clear(); CC##TypeExtContainer::Instance.Clear()
#define CLEAR_CONTAIER_CLASS(CC) 	CC::Instance.Clear()
#define CLEAR_CLASS(CC) 	CC::Clear()
#define CLEAR_TYPE_CLASS(CC) 	CC##TypeClass::Clear()
#define CLEAR_CLASS_NONSTATIC(CC) 	CC.Clear()

// Clear static data from respective classes
// this function is executed after all game classes already cleared
ASMJIT_PATCH(0x685659, Scenario_ClearClasses_PhobosGlobal, 0xA)
{
	for (auto& hand : Handles::Array) {
		hand->detachptr();
	}

	CLEAR_CONTAIER_CLASS_AND_TYPE(Aircraft);
	CLEAR_CONTAIER_CLASS_AND_TYPE(Anim);
	CLEAR_CONTAIER_CLASS_AND_TYPE(Building);
	CLEAR_CONTAIER_CLASS_AND_TYPE(Bullet);
	CLEAR_CONTAIER_CLASS_AND_TYPE(House);
	CLEAR_CONTAIER_CLASS_AND_TYPE(Infantry);
	CLEAR_CONTAIER_CLASS_AND_TYPE(Particle);
	CLEAR_CONTAIER_CLASS_AND_TYPE(ParticleSystem);
	CLEAR_CONTAIER_CLASS_AND_TYPE(Terrain);
	CLEAR_CONTAIER_CLASS_AND_TYPE(VoxelAnim);
	CLEAR_CONTAIER_CLASS_AND_TYPE(Unit);
	CLEAR_CONTAIER_CLASS(SuperExtContainer);
	CLEAR_CONTAIER_CLASS(SWTypeExtContainer);
	CLEAR_CONTAIER_CLASS(TeamExtContainer);
	CLEAR_CONTAIER_CLASS(BombExtContainer);
	CLEAR_CONTAIER_CLASS(CellExtContainer);
	CLEAR_CONTAIER_CLASS(IsometricTileTypeExtContainer);
	CLEAR_CONTAIER_CLASS(OverlayTypeExtContainer);
	CLEAR_CONTAIER_CLASS(RadSiteExtContainer);
	CLEAR_CONTAIER_CLASS(ScriptExtContainer);
	CLEAR_CONTAIER_CLASS(SideExtContainer);
	CLEAR_CONTAIER_CLASS(SmudgeTypeExtContainer);
	CLEAR_CONTAIER_CLASS(TemporalExtContainer);
	CLEAR_CONTAIER_CLASS(TiberiumExtContainer);
	CLEAR_CONTAIER_CLASS(TEventExtContainer);
	CLEAR_CONTAIER_CLASS(TriggerExtContainer);
	CLEAR_CONTAIER_CLASS(WarheadTypeExtContainer);
	CLEAR_CONTAIER_CLASS(WeaponTypeExtContainer);
	CLEAR_CONTAIER_CLASS(WaveExtContainer);

	CLEAR_CLASS(EboltExtData);
	CLEAR_CLASS(TActionExtData);
	CLEAR_CLASS(HugeBar);

	PhobosPCXFile::LoadedMap.clear();
	Handles::Array.clear();
	PrismForwarding::Array.clear();
	ShieldClass::Array.clear();

	CLEAR_CLASS_NONSTATIC(FlyingStrings::Instance);
	CLEAR_CLASS_NONSTATIC(BannerManagerClass::Instance);
	CLEAR_CLASS_NONSTATIC(SWFirerManagerClass::Instance);

	CLEAR_TYPE_CLASS(PhobosAttachEffect);
	CLEAR_CLASS(TrailType);
	CLEAR_TYPE_CLASS(Armor);
	CLEAR_TYPE_CLASS(Action);
	CLEAR_TYPE_CLASS(Banner);
	CLEAR_TYPE_CLASS(Bar);
	CLEAR_TYPE_CLASS(Color);
	CLEAR_TYPE_CLASS(Cursor);
	CLEAR_TYPE_CLASS(DigitalDisplay);
	CLEAR_CLASS(GenericPrerequisite);
	CLEAR_TYPE_CLASS(HealthBar);
	CLEAR_TYPE_CLASS(Hover);
	CLEAR_TYPE_CLASS(Immunity);
	CLEAR_TYPE_CLASS(Insignia);
	CLEAR_TYPE_CLASS(LaserTrail);
	CLEAR_CLASS(PaletteManager);
	CLEAR_TYPE_CLASS(Rad);
	CLEAR_TYPE_CLASS(Rocket);
	CLEAR_TYPE_CLASS(SelectBox);
	CLEAR_TYPE_CLASS(Shield);
	CLEAR_TYPE_CLASS(Theme);
	CLEAR_TYPE_CLASS(Tunnel);

	CLEAR_CLASS(PhobosGlobal);
	CLEAR_CLASS(StaticVars);

	MouseClassExt::ClearCameos();
	MouseClassExt::ClearMappedAction();


	return 0;
}

#pragma endregion