#include "HomingMissileTargetTracker.h"
#ifdef ENABLE_HOMING_MISSILE
#include <MapClass.h>
#include <ObjectClass.h>
#include <TechnoClass.h>
#include <FootClass.h>

std::vector<HomingMissileTargetTracker*> HomingMissileTargetTracker::Array;

HomingMissileTargetTracker::HomingMissileTargetTracker(AbstractClass* pTarget) :
	Target { pTarget },
	Coord { CoordStruct::Empty }
{
	Coord = pTarget->GetCoords();
	Array.push_back(this);
}

HomingMissileTargetTracker::HomingMissileTargetTracker() :
	Target { nullptr },
	Coord { CoordStruct::Empty }
{ Array.push_back(this); }

//updated multiple times to ensure accuraccy
void HomingMissileTargetTracker::AI()
{
	if (Target)
		Coord = Target->GetCoords();
}

AbstractClass* HomingMissileTargetTracker::AsAbstract() const
{
	if (Target)
		return Target;
	else if(Coord)
		return MapClass::Instance->GetCellAt(Coord);

	return nullptr;
}

void HomingMissileTargetTracker::InvalidatePointer(void* ptr, bool bDetach)
{
	AnnounceInvalidPointer(Target, ptr);
}

bool HomingMissileTargetTracker::Load(PhobosStreamReader& stm, bool registerForChange) {
	return stm >> Target >> Coord;
}

bool HomingMissileTargetTracker::Save(PhobosStreamWriter& stm) const {
	return stm << Target << Coord;
}

void HomingMissileTargetTracker::Update_All()
{
	if (Array.empty())
		return;

	for (int i = Array.size() - 1; i >= 0; --i)
	{
		HomingMissileTargetTracker* Tracker = Array[i];

		if (!Tracker)
			continue;

		Tracker->AI();
	}
}

void HomingMissileTargetTracker::Remove(HomingMissileTargetTracker* pWho)
{
	auto const Iter = std::find(Array.begin(), Array.end(), pWho);

	if (Iter != Array.end())
	{
		GameDelete(*Iter);
		Array.erase(Iter);
	}

}

void HomingMissileTargetTracker::Clear()
{
	for (auto const& pData : Array)
	{
		GameDelete(pData);
	}

	Array.clear();
}
#endif