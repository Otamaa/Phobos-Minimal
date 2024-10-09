#pragma once

//Rockets
class AircraftTypeClass;
struct RocketStruct
{
	int PauseFrames;
	int TiltFrames;
	float PitchInitial;
	float PitchFinal;
	float TurnRate;
	int RaiseRate; //shouldn't this be a float? prolly a mistake by WW...
	float Acceleration;
	int Altitude;
	int Damage;
	int EliteDamage;
	int BodyLength;
	bool LazyCurve;
	AircraftTypeClass* Type;

	constexpr RocketStruct() noexcept :
		PauseFrames(0),
		TiltFrames(0),
		PitchInitial(0.0f),
		PitchFinal(0.0f),
		TurnRate(0.0f),
		RaiseRate(0),
		Acceleration(0.0f),
		Altitude(0),
		Damage(0),
		EliteDamage(0),
		BodyLength(0),
		LazyCurve(false),
		Type(nullptr)
	{
	}

	constexpr explicit RocketStruct(noinit_t) noexcept
	{ }

	constexpr ~RocketStruct() = default;
};