#pragma once
#ifdef ENABLE_HOMING_MISSILE
#include <CoordStruct.h>
#include <Utilities/SavegameDef.h>

class AbstractClass;
class HomingMissileTargetTracker final
{
public:

	AbstractClass* Target;
	CoordStruct Coord;

	HomingMissileTargetTracker(AbstractClass* pTarget);
	HomingMissileTargetTracker();
	~HomingMissileTargetTracker() = default;

	void AI();
	AbstractClass* AsAbstract() const;

	void InvalidatePointer(void* ptr, bool bDetach);

	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

	static std::vector<HomingMissileTargetTracker*> Array;

	static void Update_All();
	static void Clear();
	static void Remove(HomingMissileTargetTracker* pWho);

	bool LoadGlobals(PhobosStreamReader& Stm)
	{
		return Stm
			.Process(Array)
			.Success();
	}

	bool SaveGlobals(PhobosStreamWriter& Stm)
	{
		return Stm
			.Process(Array)
			.Success();
	}

};
#endif