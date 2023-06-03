#pragma once
#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/Enum.h>
#include "../Helpers/EffectHelpers.h"

enum class TrailMode : int
{
	NONE = 0,
	LASER = 1,
	ELECTIRIC = 2,
	BEAM = 3,
	PARTICLE = 4,
	ANIM = 5
};

class TrailType final : public Enumerable<TrailType>
{
public:

	TrailMode Mode;
	Valueable<int> Distance;
	Valueable<bool> IgnoreVertical;
	Valueable<int> InitialDelay;
	Valueable<bool> HideWhenCloak;

	//Mode : Anim
	struct AnimTrailType
	{
		Valueable<AnimTypeClass*> StartDrivingAnim;
		Valueable<AnimTypeClass*> WhileDrivingAnim;
		Valueable<AnimTypeClass*> StopDrivingAnim;

		template <typename T>
		bool Serialize(T& Stm)
		{
			return Stm
				.Process(StartDrivingAnim)
				.Process(WhileDrivingAnim)
				.Process(StopDrivingAnim)
				.Success()
				;
		}

	} AnimTrailType;

	//Mode : Beam
	BeamType BeamTrailType;

	//Mode : Ebolt
	BoltType BoltTrailType;

	//Mode : Laser
	LaserType LaserTrailType;

	//Mode : Particle
	struct ParticleTrailType
	{
		ParticleTrailType()
			:ParticleSystem { nullptr }
		{ };

		~ParticleTrailType() = default;

		Valueable<ParticleSystemTypeClass*> ParticleSystem;

		bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
		{ return Serialize(Stm); }

		bool Save(PhobosStreamWriter& Stm)
		{ return  Serialize(Stm); }

	private:
		template <typename T>
		bool Serialize(T& Stm)
		{
			return Stm
				.Process(ParticleSystem)
				.Success()
				;
		}
	}ParticleTrailType ;

	TrailType(const char* const pTitle) : Enumerable<TrailType>(pTitle)
		, Mode { TrailMode::LASER }
		, Distance { 64 }
		, IgnoreVertical { false }
		, InitialDelay { 0 }
		, HideWhenCloak { true }
		//
		, AnimTrailType { }
		, BeamTrailType { RadBeamType::Eruption }
		, BoltTrailType { }
		, LaserTrailType { true }

		//
		, ParticleTrailType { }


	{ }

	virtual ~TrailType() override = default;
	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm) override;
	virtual void SaveToStream(PhobosStreamWriter& Stm) override;

private:
	template <typename T>
	void Serialize(T& Stm)
	{
		Stm
			.Process(Mode)
			.Process(Distance)
			.Process(IgnoreVertical)
			.Process(InitialDelay)
			;

		AnimTrailType.Serialize(Stm);
		BeamTrailType.Serialize(Stm);
		BoltTrailType.Serialize(Stm);
		LaserTrailType.Serialize(Stm);

		Stm
			.Process(ParticleTrailType)
			.Process(HideWhenCloak)
			;

	}
};
