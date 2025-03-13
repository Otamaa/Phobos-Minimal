
// TODO :
class BulletClass_patch : public BulletClass
{
	void _AI()
	{

	}

	COMPILETIMEEVAL double GetFloaterGravity()
	{
		return BulletTypeExtContainer::Instance.Find(this->Type)->Gravity.Get(RulesClass::Instance->Gravity) * 0.5;
	}

	DirStruct _Motion(CoordStruct& intialCoord, VelocityClass& InitialVelocity, Coordinate& targetCoord, int InitialDir)
	{
		DirStruct inital { this->CourseLock ? InitialDir : 0 };
		//const bool IsTargetFlying = this->Target && this->Target->WhatAmI() == AbstractType::Aircraft;
		//const bool IsAirburst = this->Type->Airburst;
		//const bool IsVeryHeigh = this->Type->VeryHigh;
		//const bool IsLevel = this->Type->Level;

		if (targetCoord.IsEmpty())
		{
			double length_XY = InitialVelocity.LengthXY();
			DirStruct XY { (double)InitialVelocity.Z, length_XY };
			DirStruct Initial_ { 0x2000 };
			DirStruct _dummy {};

			if (XY.CompareToTwoDir(Initial_, inital))
			{
				XY.Raw = Initial_.Raw;
			}
			else if ((_dummy = (Initial_ - XY)).Raw >= 0)
			{
				XY += inital;
			}
			else
			{
				XY -= inital;
			}

			InitialVelocity.GetDirectionFromXY(&_dummy);

			double angle = double(((int16_t)_dummy.Raw - 0x3FFF) * -0.00009587672516830327);
			//	double length_XYZ = InitialVelocity.Length();

			if (angle != 0.0)
			{
				InitialVelocity.X /= Math::cos(angle);
				InitialVelocity.Y /= Math::sin(angle);
			}

			double angle_XY = double(((int16_t)XY.Raw - 0x3FFF) * -0.00009587672516830327);

			InitialVelocity.X *= Math::cos(angle_XY);
			InitialVelocity.Y *= Math::cos(angle_XY);
			InitialVelocity.Z *= Math::sin(angle_XY);

			const CoordStruct Vel {
				(int)InitialVelocity.X , (int)InitialVelocity.Y , (int)InitialVelocity.Z
			};

			intialCoord -= Vel;
			return DirStruct { (int)intialCoord.Length() };
		}

		const CoordStruct Vel {
			(int)InitialVelocity.X , (int)InitialVelocity.Y , (int)InitialVelocity.Z
		};

		intialCoord -= Vel;
		CoordStruct distance_ = intialCoord - targetCoord;
		double length_intial = distance_.Length();
		DirStruct length_Dir { int(length_intial) };
		Point2D targetCoord_XY { targetCoord.X , targetCoord.Y };
		Point2D initialCoord_XY { intialCoord.X , intialCoord.Y };
		Point2D distance_XY = targetCoord_XY - initialCoord_XY;
		//double length_Xt = distance_XY.Length();
		VelocityClass distance_vel { double(distance_.X), double(distance_.Y), double(distance_.Z) };
		// the variable around here is messed up , idk
		double angle = Math::atan2((double)-inital.Raw, (double)inital.Raw);
		double rad = angle - Math::DEG90_AS_RAD;
		DirStruct distance_Dir { (int)rad * Math::BINARY_ANGLE_MAGIC };
		DirStruct distance_Dir_ { -distance_vel.Y , distance_vel.Y };
		DirStruct _dummy {};

		if (distance_Dir.CompareToTwoDir(distance_Dir_, inital))
		{
			distance_Dir.Raw = distance_Dir_.Raw;
		}
		else if ((_dummy = (distance_Dir_ - distance_Dir)).Raw >= 0)
		{
			distance_Dir.Raw += inital.Raw;
		}
		else
		{
			distance_Dir.Raw -= inital.Raw;
		}

	}
};
