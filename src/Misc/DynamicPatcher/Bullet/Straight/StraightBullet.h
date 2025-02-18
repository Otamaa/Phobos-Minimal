#pragma once

#include <Utilities/TemplateDef.h>

struct StraightBullet
{
	bool Enable;
	CoordStruct SourcePos;
	CoordStruct TargetPos;
	BulletVelocity Velocity;

	StraightBullet(bool enable, CoordStruct sourcePos, CoordStruct targetPos, BulletVelocity bulletVelocity)
		:
		Enable { enable }
		, SourcePos { sourcePos }
		, TargetPos { targetPos }
		, Velocity { bulletVelocity }
	{ }

	StraightBullet()
		:
		  Enable {}
		, SourcePos {}
		, TargetPos {}
		, Velocity {}
	{ }

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ Debug::LogInfo("Loading Element From StraightBullet ! "); return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm)
	{ return Serialize(Stm); }

private:
	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(Enable)
			.Process(SourcePos)
			.Process(TargetPos)
			.Process(Velocity)
			.Success()
			;
	}
};