#pragma once

#include <CoordStruct.h>
#include <Utilities/SavegameDef.h>

class WeaponTypeClass;
class VerticalLaserClass
{
	bool Expired;
public:
	int count;
	int angle;
	int cur_frames;
	int frames_max;
	double radius;
	WeaponTypeClass* Weapon;
	CoordStruct From;
	int Height;
	int radius_decrement;

	void Reset();

	VerticalLaserClass();
	VerticalLaserClass(WeaponTypeClass* Weapon ,CoordStruct From , int Height);
	~VerticalLaserClass() = default;

	void AI();
	CoordStruct GetCoords(int start , int i , int increase);
	void Draw(const CoordStruct& from, const CoordStruct& to);
	void DealDamage(const CoordStruct& to);
	void AI(int start, int count);
	bool Load(PhobosStreamReader& stm, bool registerForChange) { return true; }
	bool Save(PhobosStreamWriter& stm) const { return true; }

	static std::vector<VerticalLaserClass*> Array;

	static void Draw_All();
	static void Clear();

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