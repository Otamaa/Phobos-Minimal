#include <AircraftClass.h>
#include <AircraftTypeClass.h>

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

struct DummyTechnoTypeExt{

	struct Data {
		bool ManualDocks { false };
		int FetchAmmotime { -1 };
	};

	static std::unordered_map<const TechnoTypeClass*, Data*> Container;

	static Data* Find(TechnoTypeClass* pKey) {
		const auto find = Container.find(pKey);
		if (find == Container.end())
			return nullptr;

		return find->second;
	}

};
std::unordered_map<const TechnoTypeClass*, DummyTechnoTypeExt::Data*> DummyTechnoTypeExt::Container {};

struct DummyTechnoExt {

	struct Data {

	};

	static std::unordered_map<const TechnoClass*, Data*> Container;

	static Data* Find(TechnoClass* pKey)	{
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
DEFINE_HOOK(0x417996, AircraftClass_AssumeTaskComplete_SkipAutoDocking1,9)
{
	GET(AircraftClass*, pThis, ESI);

	if (pThis->Type->Spawned || pThis->Type->Carryall)
		return 0x0;

	const auto pTypeExt = DummyTechnoTypeExt::Find(pThis->Type);

	if (pTypeExt->ManualDocks || (pThis->Ammo && !pThis->Destination)) {
		pThis->SetDestination(nullptr, true);
		return 0x417B69;
	}

	if (pTypeExt->FetchAmmotime == -1)
		return 0x0;

	some_bldType_Arr_.clear();

	for (auto pBld : *BuildingClass::Array) {
		if (pBld && !pBld->InLimbo && pBld->IsAlive && pBld->Owner == pThis->Owner) {
			for (auto& pDock : pThis->Type->Dock) {
				if (pDock == pBld->Type) {
					some_bldType_Arr_.push_back(pBld);
					break;
				}
			}
		}
	}

	BuildingClass* Closest = nullptr;
	for (auto pBldGet : some_bldType_Arr_) {
		if (Closest) {
			if (Closest->DistanceFrom(pThis) > pBldGet->DistanceFrom(pThis)) {
				Closest = pBldGet;
			}
		} else {
			Closest = pBldGet;
		}
	}

	pThis->SetDestination(Closest , true);
	pThis->SetTarget(nullptr);
	pThis->QueueMission(Mission::Move , true);
	some_bldType_Arr_.clear();
	return 0x417B69;
}

DEFINE_HOOK(0x417ad4, AircraftClass_AssumeTaskComplete_SkipAutoDocking3,6)
{
}