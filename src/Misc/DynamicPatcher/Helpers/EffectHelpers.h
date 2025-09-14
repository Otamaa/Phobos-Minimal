#pragma once

#include <CoordStruct.h>
#include <Utilities/SavegameDef.h>
#include <Utilities/Constructs.h>
#include <Utilities/TemplateDefB.h>

#include <LaserDrawClass.h>
#include <RadBeam.h>
#include <New/Entity/ElectricBoltClass.h>

#include <ParticleSystemTypeClass.h>
#include <ParticleSystemClass.h>

// ToDo : proper bolt colors
struct LaserType
{
public:

	Valueable<ColorStruct> InnerColor;
	Valueable<ColorStruct> OuterColor;
	Valueable<ColorStruct> OuterSpread;
	Valueable<int> Duration;
	Valueable<int> Thickness;
	Valueable<bool> IsHouseColor;
	Valueable<bool> IsSupported;
	Valueable<bool> Fade;

	LaserType() = default;

	LaserType(bool def)
		:InnerColor { ColorStruct::Empty }
		, OuterColor { ColorStruct::Empty }
		, OuterSpread { ColorStruct::Empty }
		, Duration { 15 }
		, Thickness { 2 }
		, IsHouseColor { false }
		, IsSupported { false }
		, Fade { true }
	{
		if (def)
		{
			InnerColor = ColorStruct(204, 64, 6);
			OuterColor = ColorStruct(102, 32, 3);
		}
	};

	~LaserType() = default;

	LaserType(const LaserType& other) = default;
	LaserType& operator=(const LaserType& other) = default;

	void SetInnerColor(int R, int G, int B)
	{ InnerColor = ColorStruct((byte)R, (byte)G, (byte)B); }

	void SetOuterColor(int R, int G, int B)
	{ OuterColor = ColorStruct((byte)R, (byte)G, (byte)B); }

	void SetOuterSpread(int R, int G, int B)
	{ OuterSpread = ColorStruct((byte)R, (byte)G, (byte)B); }

	bool FORCEDINLINE Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return this->Serialize(Stm);
	}

	bool FORCEDINLINE Save(PhobosStreamWriter& Stm) const
	{
		return const_cast<LaserType*>(this)->Serialize(Stm);
	}

private:

	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(InnerColor)
			.Process(OuterColor)
			.Process(OuterSpread)
			.Process(Duration)
			.Process(Thickness)
			.Process(IsHouseColor)
			.Process(IsSupported)
			.Process(Fade)
			.Success()
			//&& Stm.RegisterChange(this)
		;
	}
};

struct BeamType
{
public:

	RadBeamType TBeamType;
	Valueable<ColorStruct> BeamColor;
	Valueable<int> Period;
	Valueable<double> Amplitude;

	BeamType() = default;

	BeamType(RadBeamType radBeamType) :
		Period { 15 }
		, Amplitude { 40.0 }
	{
		SetBeamType(radBeamType);
	};

	~BeamType() = default;

	BeamType(const BeamType& other) = default;
	BeamType& operator=(const BeamType& other) = default;

	void SetBeamType(RadBeamType radBeamType)
	{
		TBeamType = radBeamType;
		switch (radBeamType)
		{
		case RadBeamType::Temporal:
			BeamColor = ColorStruct(128, 200, 255);
			break;
		default:
			BeamColor = ColorStruct(0, 255, 0);
			break;
		}
	}

	bool FORCEDINLINE Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return this->Serialize(Stm);
	}

	bool FORCEDINLINE Save(PhobosStreamWriter& Stm) const
	{
		return const_cast<BeamType*>(this)->Serialize(Stm);
	}

private:

	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(TBeamType)
			.Process(BeamColor)
			.Process(Period)
			.Process(Amplitude)
			.Success()
			//&& Stm.RegisterChange(this)
		;
	}
};

struct BoltType
{
public:

	Valueable<ColorStruct> Color1;
	Valueable<ColorStruct> Color2;
	Valueable<ColorStruct> Color3;
	Valueable<bool> IsAlternateColor;
	Valueable<bool> ParticleSystem_coordFlip;
	Valueable<ParticleSystemTypeClass*> ParticleSystem;

	Valueable<bool> Color1_disable;
	Valueable<bool> Color2_disable;
	Valueable<bool> Color3_disable;

	BoltType() :
		IsAlternateColor { false }
		, ParticleSystem_coordFlip { false }
		, ParticleSystem { nullptr }
		, Color1_disable {}
		, Color2_disable {}
		, Color3_disable {}
	{
		auto const nDraw = FileSystem::PALETTE_PAL();
		Color1 = ColorStruct((int)nDraw->inline_02(IsAlternateColor ? 5 : 10));
		Color2 = ColorStruct((int)nDraw->inline_02(IsAlternateColor ? 5 : 10));
		Color3 = ColorStruct((int)nDraw->inline_02(15));
	};

	~BoltType() = default;

	BoltType(const BoltType& other) = default;
	BoltType& operator=(const BoltType& other) = default;

	bool FORCEDINLINE Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return this->Serialize(Stm);
	}

	bool FORCEDINLINE Save(PhobosStreamWriter& Stm) const
	{
		return const_cast<BoltType*>(this)->Serialize(Stm);
	}

private:

	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(IsAlternateColor)
			.Process(ParticleSystem_coordFlip)
			.Process(ParticleSystem)
			.Process(Color1)
			.Process(Color2)
			.Process(Color3)
			.Process(Color1_disable)
			.Process(Color2_disable)
			.Process(Color3_disable)
			.Success()
		//	&& Stm.RegisterChange(this)
		;
	}
};

struct EffectHelpers
{
private:
	NO_CONSTRUCT_CLASS(EffectHelpers)
public:

	static void RedCrosshair(CoordStruct sourcePos, int length, int thickness = 1, int duration = 1)
	{
		Crosshair(sourcePos, length, { 255, 0, 0 }, ColorStruct::Empty, thickness, duration);
	}

	static void RedCell(CoordStruct sourcePos, int length, int thickness = 1, int duration = 1, bool crosshair = false)
	{
		if (crosshair)
		{
			RedCrosshair(sourcePos, length, thickness, duration);
		}
		Cell(sourcePos, length, { 255, 0, 0 }, ColorStruct::Empty, thickness, duration);
	}

	static void GreenCrosshair(CoordStruct sourcePos, int length, int thickness = 1, int duration = 1)
	{
		Crosshair(sourcePos, length, { 0, 255, 0 }, ColorStruct::Empty, thickness, duration);
	}

	static void GreenCell(CoordStruct sourcePos, int length, int thickness = 1, int duration = 1, bool crosshair = false)
	{
		if (crosshair)
		{
			GreenCrosshair(sourcePos, length, thickness, duration);
		}
		Cell(sourcePos, length, { 0, 255, 0 }, ColorStruct::Empty, thickness, duration);
	}

	static void BlueCrosshair(CoordStruct sourcePos, int length, int thickness = 1, int duration = 1)
	{
		Crosshair(sourcePos, length, { 0, 0, 255 }, ColorStruct::Empty, thickness, duration);
	}

	static void BlueCell(CoordStruct sourcePos, int length, int thickness = 1, int duration = 1, bool crosshair = false)
	{
		if (crosshair)
		{
			BlueCrosshair(sourcePos, length, thickness, duration);
		}
		Cell(sourcePos, length, { 0, 0, 255 }, ColorStruct::Empty, thickness, duration);
	}

	static void Crosshair(CoordStruct sourcePos, int length, ColorStruct lineColor, ColorStruct outerColor = ColorStruct::Empty, int thickness = 1, int duration = 1)
	{
		DrawLine(sourcePos, sourcePos + CoordStruct { length, 0, 0 }, lineColor, outerColor, thickness, duration);
		DrawLine(sourcePos, sourcePos + CoordStruct { -length, 0, 0 }, lineColor, outerColor, thickness, duration);
		DrawLine(sourcePos, sourcePos + CoordStruct { 0, -length, 0 }, lineColor, outerColor, thickness, duration);
		DrawLine(sourcePos, sourcePos + CoordStruct { 0, length, 0 }, lineColor, outerColor, thickness, duration);
	}

	static void Cell(CoordStruct sourcePos, int length, ColorStruct lineColor, ColorStruct outerColor = ColorStruct::Empty, int thickness = 1, int duration = 1)
	{
		CoordStruct p1 = sourcePos + CoordStruct { length, length, 0 };
		CoordStruct p2 = sourcePos + CoordStruct { -length, length, 0 };
		CoordStruct p3 = sourcePos + CoordStruct { -length, -length, 0 };
		CoordStruct p4 = sourcePos + CoordStruct { length, -length, 0 };
		DrawLine(p1, p2, lineColor, outerColor, thickness, duration);
		DrawLine(p2, p3, lineColor, outerColor, thickness, duration);
		DrawLine(p3, p4, lineColor, outerColor, thickness, duration);
		DrawLine(p4, p1, lineColor, outerColor, thickness, duration);
	}

	static void RedLineZ(CoordStruct sourcePos, int length, int thickness = 1, int duration = 1)
	{
		RedLine(sourcePos, sourcePos + CoordStruct { 0, 0, length }, thickness, duration);
	}

	static void RedLine(CoordStruct sourcePos, CoordStruct targetPos, int thickness = 1, int duration = 1)
	{
		DrawLine(sourcePos, targetPos, { 255, 0, 0 }, ColorStruct::Empty, thickness, duration);
	}

	static void GreenLineZ(CoordStruct sourcePos, int length, int thickness = 1, int duration = 1)
	{
		GreenLine(sourcePos, sourcePos + CoordStruct { 0, 0, length }, thickness, duration);
	}

	static void GreenLine(CoordStruct sourcePos, CoordStruct targetPos, int thickness = 1, int duration = 1)
	{
		DrawLine(sourcePos, targetPos, { 0, 255, 0 }, ColorStruct::Empty, thickness, duration);
	}

	static void BlueLineZ(CoordStruct sourcePos, int length, int thickness = 1, int duration = 1)
	{
		BlueLine(sourcePos, sourcePos + CoordStruct { 0, 0, length }, thickness, duration);
	}

	static void BlueLine(CoordStruct sourcePos, CoordStruct targetPos, int thickness = 1, int duration = 1)
	{
		DrawLine(sourcePos, targetPos, { 0, 0, 255 }, ColorStruct::Empty, thickness, duration);
	}

	static void DrawLine(CoordStruct sourcePos, CoordStruct targetPos, ColorStruct innerColor, ColorStruct outerColor = ColorStruct::Empty, int thickness = 2, int duration = 15)
	{

		LaserType type = LaserType(true);
		type.InnerColor = innerColor;
		type.OuterColor = outerColor;
		type.Thickness = thickness;
		type.Duration = duration;
		DrawLine(sourcePos, targetPos, type, ColorStruct::Empty);
	}

	static void DrawLine(CoordStruct& sourcePos, CoordStruct& targetPos, LaserType& type, const ColorStruct& houseColor)
	{
		ColorStruct innerColor = type.InnerColor;
		ColorStruct outerColor = type.OuterColor;
		ColorStruct outerSpread = type.OuterSpread;
		if (houseColor != ColorStruct::Empty)
		{
			innerColor = houseColor;
			outerColor = ColorStruct::Empty;
		}

		auto pLaser = GameCreate<LaserDrawClass>(sourcePos, targetPos, innerColor, outerColor, outerSpread, type.Duration);
		pLaser->Thickness = type.Thickness;
		pLaser->IsHouseColor = type.Fade ? type.Fade : type.IsHouseColor;
		pLaser->IsSupported = type.IsSupported ? type.IsSupported : (type.Thickness > 5 && !type.Fade);
	}

	static void DrawBeam(CoordStruct& sourcePos, CoordStruct& targetPos, BeamType& type, const ColorStruct& customColor)
	{
		ColorStruct beamColor = type.BeamColor;

		if (customColor != ColorStruct::Empty)
			beamColor = customColor;

		if (auto const pRadBeam = RadBeam::Allocate(type.TBeamType))
		{
			pRadBeam->SetCoordsSource(sourcePos);
			pRadBeam->SetCoordsTarget(targetPos);
			pRadBeam->Color = beamColor;
			pRadBeam->Period = type.Period.Get();
			pRadBeam->Amplitude = type.Amplitude.Get();
		}
	}

	static void DrawBolt(CoordStruct sourcePos, CoordStruct targetPos, WeaponTypeClass* pWeapon);

	static void DrawBolt(CoordStruct sourcePos, CoordStruct targetPos, bool alternate = false)
	{
		BoltType type = BoltType();
		type.IsAlternateColor = alternate;
		DrawBolt(sourcePos, targetPos, type);
	}

	static void DrawBolt(CoordStruct& sourcePos, CoordStruct& targetPos, BoltType& type)
	{
#ifdef _Enable
		ElectricBoltClass::Create(sourcePos, targetPos, type.Color1 , type.Color2 , type.Color3,
		type.Color1_disable, type.Color2_disable, type.Color3_disable ,
		0, type.ParticleSystem.Get(), type.ParticleSystem_coordFlip.Get());
#endif
	}

	static void DrawBolt(TechnoClass* pShooter, AbstractClass* pTarget, WeaponTypeClass* pWeapon, CoordStruct& sourcePos)
	{
		pShooter->CreateEbolt(pTarget, pWeapon, sourcePos);
	}

	static void DrawParticle(HouseClass* pOwner, CoordStruct& sourcePos, CoordStruct& targetPos, ParticleSystemTypeClass* psType)
	{
		if (psType)
			DrawParticle(psType, sourcePos, targetPos, pOwner);
	}

	static void DrawParticle(ParticleSystemTypeClass* psType, CoordStruct& sourcePos, CoordStruct& targetPos, HouseClass* pOwner)
	{
		DrawParticle(psType, sourcePos, nullptr, targetPos, pOwner);
	}

	static void DrawParticle(ParticleSystemTypeClass* psType, CoordStruct& sourcePos, TechnoClass* pOwner, CoordStruct& targetPos, HouseClass* pOwnerHouse)
	{
		DrawParticle(psType, sourcePos, nullptr, pOwner, targetPos, pOwnerHouse);
	}

	static void DrawParticle(ParticleSystemTypeClass* psType, CoordStruct& sourcePos, AbstractClass* pTarget, TechnoClass* pOwner, CoordStruct& targetPos, HouseClass* pOwnerHouse)
	{
		if(psType->BehavesLike == ParticleSystemTypeBehavesLike::Smoke)
			sourcePos.Z += 100;

		GameCreate<ParticleSystemClass>(psType, sourcePos, pTarget, pOwner, targetPos, pOwnerHouse);
	}

};