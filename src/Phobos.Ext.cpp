#include <Phobos.h>
#include <Phobos.Ext.h>

#include <Ext/Aircraft/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/House/Body.h>
#include <Ext/RadSite/Body.h>
#include <Ext/Rules/Body.h>
#include <Ext/Scenario/Body.h>
#include <Ext/Script/Body.h>
#include <Ext/Side/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/TAction/Body.h>
#include <Ext/Team/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/TerrainType/Body.h>
#include <Ext/Tiberium/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/VoxelAnimType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>

#include <New/Type/RadTypeClass.h>
#include <New/Type/LaserTrailTypeClass.h>

#pragma region OtamaaStuffs
//#include <Ext/Bomb/Body.h>
//#include <Ext/Parasite/Body.h>
#include <Ext/Particle/Body.h>
#include <Ext/ParticleType/Body.h>
#include <Ext/ScriptType/Body.h>
#include <Ext/TeamType/Body.h>
#include <Ext/Terrain/Body.h>
#include <Ext/SmudgeType/Body.h>
#include <New/Type/ArmorTypeClass.h>
#include <New/Type/HoverTypeClass.h>
#include <New/Entity/FlyingStrings.h>
//#include <New/Entity/FoggedObject.h>
#ifdef COMPILE_PORTED_DP_FEATURES
#include <Misc/DynamicPatcher/Trails/TrailType.h>
#endif
#pragma endregion

#include <utility>

// Add more class names as you like
auto MassActions = MassAction <
#pragma region OtamaaStuffs
	//BombExt,
	ParticleExt,
	ParticleTypeExt,
	ScriptTypeExt,
	ArmorTypeClass,
	TeamTypeExt,
	TerrainExt,
	SmudgeTypeExt,
#ifdef COMPILE_PORTED_DP_FEATURES
	TrailType,
#endif
#pragma endregion
	// Ext classes
	AircraftExt,
	AnimTypeExt,
	AnimExt,
	BuildingExt,
	BuildingTypeExt,
	BulletExt,
	BulletTypeExt,
	HouseExt,
	RadSiteExt,
	RulesExt,
	ScenarioExt,
	ScriptExt,
	SideExt,
	SWTypeExt,
	TActionExt,
	TeamExt,
	TechnoExt,
	TechnoTypeExt,
	TerrainTypeExt,
	TiberiumExt,
	VoxelAnimExt,
	VoxelAnimTypeExt,
	WarheadTypeExt,
	WeaponTypeExt,
	// New classes
	ShieldTypeClass,
	LaserTrailTypeClass,
	RadTypeClass,
	HoverTypeClass
	// other classes
> ();

DEFINE_HOOK(0x7258D0, AnnounceInvalidPointer, 0x6)
{
	GET(AbstractClass* const, pInvalid, ECX);
	GET(bool const, removed, EDX);

	Phobos::PointerGotInvalid(pInvalid, removed);

	return 0;
}

DEFINE_HOOK(0x685659, Scenario_ClearClasses, 0xa)
{
	Phobos::Clear();
	return 0;
}

void Phobos::Clear()
{
	MassActions.Clear();
	//AnimExt::Invokers.clear();
	//AnimExt::AnimCellUpdater::Clear();
	FlyingStrings::Clear();
#ifdef COMPILE_PORTED_DP_FEATURES
	ElectricBoltManager::Clear_All();
#endif
}

void Phobos::PointerGotInvalid(AbstractClass* const pInvalid, bool const removed)
{
	MassActions.InvalidPointer(pInvalid, removed);
}

HRESULT Phobos::SaveGameData(IStream* pStm)
{
	Debug::Log("Saving global Phobos data\n");

	if (!MassActions.Save(pStm))
		return E_FAIL;

	Debug::Log("Finished saving the game\n");

	return S_OK;
}

void Phobos::LoadGameData(IStream* pStm)
{
	Debug::Log("Loading global Phobos data\n");

	if (!MassActions.Load(pStm))
		Debug::Log("Error loading the game\n");
	else
		Debug::Log("Finished loading the game\n");
}
