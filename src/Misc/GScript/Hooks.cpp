#include <AircraftClass.h>
#include <AircraftTypeClass.h>

#include <Utilities/TemplateDefB.h>

#include <Phobos.h>

/*
		An experimental code  , recovered from GScript.ext (dll) from the `TC2` mod
		Since it based on Phobos , they suppose to share the dll source code as per License says
		instead they hiding it behind new names , and re-using Phobos member codes
		without following the GPL license

		so  , i guess i need to brakeforce like what i did with ares :p

		This codes is here in purpose to understand the internal working of the
		`TC2` bootleg `Phobos.dll`.
*/

const char* CustomTypestr[] =
{
"None"
,"MRT"
,"PAC"
, "Mastodon"
,"EradiDummy"
,"WidowDummy"
, "RepairDrone"
,"ScrinRepairDrone"
,"Stratofighter"
, "ShadeGuardian"
,"MS_Camera"
, "MS_CameraExec"
, "Crate"
, "TiberiumCrystal"
, "Eradicator"
,"Devourer"
, "DevourerCharged"
, "ReaperAnnihilator"
,"ReaperAnnihilatorCharged"
, "MS_FakeAirport"
, "UpgradeStructure"
, "VertigoI"
, "Wreckage"
, "AnnihilatorHand"
, "SensorDummy"
, "FirehawkSpawned"
, "Shatterer"
,"SW_RadarJammer"
,"SW_Eradication"
, "SW_ShadeGuardian"
, "SW_DroneRepairer"
, "SW_Stratofighter"
, "SW_FirehawkSpecial"
, "Buzzer"
, "OutpostConstructor"
, "AI_FlyingMCV"
, "Juggernaught"
, "Invader"
,"PACDummy"
,"MindController"
, "SecondarySkill"
, "Basilisk"
, "Mastermind"
,"Prodigy"
,"TertiarySkill"
,"FirehawkII"
, "Mine"
, "ORCA"
, "FighterHelper"
, "BeamCannon"
, "Obelisk"
,"AirStrafer"
,"GCommandoMissile"
, "CyborgCommando"
, "IonLaser"
, "BuzzerHelper"
, "BuzzerHive"
,"Kodiak"
,"KodiakMachgun"
, "Rhino"
, "RhinoDeployed"
, "FactoryRepairDrone"
,"Reckoner"
,"ReckonerUP"
,"Specter"
, "SpecterDeployed"
,"Genesis"
,"FastUpgradeStructure"
,"BoostedCrawler"
,"MCV"
,"V_Base"
,"SubTowerDummy"
,"TransportChecker"
,"Cobra"
,"Redeemer"
};

enum class CustomType : int
{
	None = 0
	, MRT = 1
	, PAC = 2
	, Mastodon = 3
	, EradiDummy = 4
	, WidowDummy = 5
	, RepairDrone = 6
	, ScrinRepairDrone = 7
	, Stratofighter = 8
	, ShadeGuardian = 9
	, MS_Camera = 10
	, MS_CameraExec = 11
	, Crate = 12
	, TiberiumCrystal = 13
	, Eradicator = 14
	, Devourer = 15
	, DevourerCharged = 16
	, ReaperAnnihilator = 17
	, ReaperAnnihilatorCharged = 18
	, MS_FakeAirport = 19
	, UpgradeStructure = 20
	, VertigoI = 21
	, Wreckage = 22
	, AnnihilatorHand = 23
	, SensorDummy = 24
	, FirehawkSpawned = 25
	, Shatterer = 26
	, SW_RadarJammer = 27
	, SW_Eradication = 28
	, SW_ShadeGuardian = 29
	, SW_DroneRepairer = 30
	, SW_Stratofighter = 31
	, SW_FirehawkSpecial = 32
	, Buzzer = 33
	, OutpostConstructor = 34
	, AI_FlyingMCV = 35
	, Juggernaught = 36
	, Invader = 37
	, PACDummy = 38
	, MindController = 39
	, SecondarySkill = 40
	, Basilisk = 41
	, Mastermind = 42
	, Prodigy = 43
	, TertiarySkill = 44
	, FirehawkII = 45
	, Mine = 46
	, ORCA = 47
	, FighterHelper = 48
	, BeamCannon = 49
	, Obelisk = 50
	, AirStrafer = 51
	, GCommandoMissile = 52
	, CyborgCommando = 53
	, IonLaser = 54
	, BuzzerHelper = 55
	, BuzzerHive = 56
	, Kodiak = 57
	, KodiakMachgun = 58
	, Rhino = 59
	, RhinoDeployed = 60
	, FactoryRepairDrone = 61
	, Reckoner = 62
	, ReckonerUP = 63
	, Specter = 64
	, SpecterDeployed = 65
	, Genesis = 66
	, FastUpgradeStructure = 67
	, BoostedCrawler = 68
	, MCV = 69
	, V_Base = 70
	, SubTowerDummy = 71
	, TransportChecker = 72
	, Cobra = 73
	, Redeemer = 74
};

struct BulletAttachType
{
	enum class Types : BYTE{
		none,
		flash ,
		orangeflash ,
		gmissile,
		nmissile,
		disfiremisl
	};

	struct Resource {
		ConvertClass* Convert;
		bool ConvertClearable;
		SHPStruct* Shape;
		bool ShapeClearable;
	};
	static std::array<Resource, 5u> Resources;

	static constexpr std::array<std::pair<Types, const char*>, 6u> to_strings {
	{
		{ Types::none , "none" } ,
		{ Types::flash , "flash" } ,
		{ Types::orangeflash , "orangeflash" } ,
		{ Types::gmissile , "gmissile" } ,
		{ Types::nmissile , "nmissile" } ,
		{ Types::disfiremisl , "disfiremisl" } ,
	}
	};

	static void InitGlobalResources() {
		Resources[0].Convert = FileSystem::LoadPALFile("mislflashwh.pal", DSurface::Temp);
		Resources[0].ConvertClearable = true;
		Resources[0].Shape = FileSystem::LoadSHPFile("missileflash.shp");
		Resources[0].ShapeClearable = true;

		Resources[1].Convert = FileSystem::LoadPALFile("misflashorange.pal", DSurface::Temp);
		Resources[1].ConvertClearable = true;
		Resources[1].Shape = FileSystem::LoadSHPFile("missileflash.shp");
		Resources[1].ShapeClearable = true;

		Resources[2].Convert = FileSystem::UNITx_PAL();
		Resources[2].Shape = FileSystem::LoadSHPFile("gmissile.shp");
		Resources[2].ShapeClearable = true;

		Resources[3].Convert = FileSystem::PALETTE_PAL();
		Resources[3].Shape = FileSystem::LoadSHPFile("nmissile.shp");
		Resources[3].ShapeClearable = true;

		Resources[4].Convert = FileSystem::PALETTE_PAL();
		Resources[4].Shape = FileSystem::LoadSHPFile("disfiremisl.shp");
		Resources[4].ShapeClearable = true;
	}

	static void ClearGlobalResources(){
		for (auto& datas : Resources) {
			if (datas.ConvertClearable && datas.Convert) {
				GameDelete(datas.Convert);
			}

			if (datas.ShapeClearable && datas.Shape) {
				GameDelete(datas.Shape);
			}

			datas.Convert = nullptr;
			datas.Shape = nullptr;
		}
	}

	void Read(INI_EX& read, const char* pSection, const char* pKey) const {
		if (read.ReadString(pSection, pKey) >= 4) {
			for (auto& [result, str_] : to_strings) {
				if (IS_SAME_STR_(str_, read.value())){
					this->Val = result;
					break;
				}
			}
		}
	}

	void Draw(BulletClass* pBullet) {

		ConvertClass* pConvert = nullptr;
		SHPStruct* pAttachment = nullptr;
		BlitterFlags flags = BlitterFlags::None;

		switch (this->Val)
		{
		case Types::flash:
			pConvert = Resources[0].Convert;
			pAttachment = Resources[0].Shape;
			flags = BlitterFlags(0x2E06u);
			break;
		case Types::orangeflash:
			pConvert = Resources[1].Convert;
			pAttachment = Resources[1].Shape;
			flags = BlitterFlags(0x2E06u);
			break;
		case Types::gmissile:
			pConvert = Resources[2].Convert;
			pAttachment = Resources[2].Shape;
			flags = BlitterFlags(0x2E00u);
			break;
		case Types::nmissile:
			pConvert = Resources[3].Convert;
			pAttachment = Resources[3].Shape;
			flags = BlitterFlags(0x2E00u);
			break;
		case Types::disfiremisl:
			pConvert = Resources[4].Convert;
			pAttachment = Resources[4].Shape;
			flags = BlitterFlags(0x2E00u);
			break;
		default:
			return;
		}

		if (!pAttachment || !pConvert)
			return;

		int height = pBullet->GetHeight();
		int z_adj = -31 - Game::AdjustHeight(height);
		Point2D Client {};
		TacticalClass::Instance->CoordsToClient(pBullet->Location, &Client);
		auto rect = DSurface::Temp->Get_Rect_WithoutBottomBar();

		DSurface::Temp->DrawSHP(pConvert, pAttachment, 0, &Client, &rect, flags, 0, z_adj, 0, 1000, 0, nullptr, 0, 0, 0);
	}

protected :
	mutable Types Val;
};

std::array<BulletAttachType::Resource, 5u> BulletAttachType::Resources = {{
	{ } ,
	{ } ,
	{ } ,
	{ } ,
	{ } ,
}};

struct DummyTechnoTypeExt
{

	struct Data
	{
		bool ManualDocks { false };
		int FetchAmmotime { -1 };
		CustomType ThisCustomType { CustomType::None };
	};

	static std::unordered_map<const TechnoTypeClass*, Data*> Container;

	static Data* Find(TechnoTypeClass* pKey)
	{
		const auto find = Container.find(pKey);
		if (find == Container.end())
			return nullptr;

		return find->second;
	}

};
std::unordered_map<const TechnoTypeClass*, DummyTechnoTypeExt::Data*> DummyTechnoTypeExt::Container {};

struct DummyTechnoExt
{

	struct Data
	{

	};

	static std::unordered_map<const TechnoClass*, Data*> Container;

	static Data* Find(TechnoClass* pKey)
	{
		const auto find = Container.find(pKey);
		if (find == Container.end())
			return nullptr;

		return find->second;
	}

};
std::unordered_map<const TechnoClass*, DummyTechnoExt::Data*> DummyTechnoExt::Container {};

DEFINE_HOOK(0x4179f7, AircraftClass_AssumeTaskComplete_DontCrash, 0x6)
{
	GET(AircraftClass*, pThis, ESI);

	if (pThis->Type->Spawned || pThis->Type->Carryall)
		return 0x0;

	pThis->SetDestination(nullptr, true);
	return 0x417B69;
}

static std::vector<BuildingClass*> some_bldType_Arr_ {};
DEFINE_HOOK_AGAIN(0x417b72, AircraftClass_AssumeTaskComplete_SkipAutoDocking2, 6)
DEFINE_HOOK(0x417996, AircraftClass_AssumeTaskComplete_SkipAutoDocking1, 9)
{
	GET(AircraftClass*, pThis, ESI);

	if (pThis->Type->Spawned || pThis->Type->Carryall)
		return 0x0;

	const auto pTypeExt = DummyTechnoTypeExt::Find(pThis->Type);

	if (pTypeExt->ManualDocks || (pThis->Ammo && !pThis->Destination))
	{
		pThis->SetDestination(nullptr, true);
		return 0x417B69;
	}

	if (pTypeExt->FetchAmmotime == -1)
		return 0x0;

	some_bldType_Arr_.clear();

	for (auto pBld : *BuildingClass::Array)
	{
		if (pBld && !pBld->InLimbo && pBld->IsAlive && pBld->Owner == pThis->Owner)
		{
			for (auto& pDock : pThis->Type->Dock)
			{
				if (pDock == pBld->Type)
				{
					some_bldType_Arr_.push_back(pBld);
					break;
				}
			}
		}
	}

	BuildingClass* Closest = nullptr;
	for (auto pBldGet : some_bldType_Arr_)
	{
		if (Closest)
		{
			if (Closest->DistanceFrom(pThis) > pBldGet->DistanceFrom(pThis))
			{
				Closest = pBldGet;
			}
		}
		else
		{
			Closest = pBldGet;
		}
	}

	pThis->SetDestination(Closest, true);
	pThis->SetTarget(nullptr);
	pThis->QueueMission(Mission::Move, true);
	some_bldType_Arr_.clear();
	return 0x417B69;
}

DEFINE_HOOK(0x417ad4, AircraftClass_AssumeTaskComplete_SkipAutoDocking3, 6)
{
	GET(AircraftClass*, pThis, ESI);

	if (pThis->Type->Spawned || pThis->Type->Carryall)
		return 0x0;

	const auto pTypeExt = DummyTechnoTypeExt::Find(pThis->Type);

	if (pTypeExt->ManualDocks && pThis->HasAnyLink())
		return 0x417B34;

	pThis->SetDestination(nullptr, true);
	return 0x417B69;
}

DEFINE_HOOK(0x41cb62, AircraftClass_CanAttackMove, 14)
{
	R->AL(true);
	return 0;
}

#include <Utilities/VectorHelper.h>

static HelperedVector<HouseClass*> Detectors {};

DEFINE_HOOK(0x4149b6, AircraftClass_Draw_BuzzerHelper, 5)
{
	GET(AircraftClass*, pThis, EBP);

	const auto pTypeExt = DummyTechnoTypeExt::Find(pThis->Type);


	return 0;
}