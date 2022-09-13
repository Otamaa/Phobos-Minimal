#pragma once

#ifdef COMPILE_PORTED_DP_FEATURES
#include <Utilities/BaseClassTemplates.h>
#include <Utilities/TemplateDef.h>

#include <ColorStruct.h>
#include <GeneralStructures.h>

#include <string_view>
#include <optional>

class TechnoClass;
class REGISTERS;

class PaintballType
{
public:
	ColorStruct Color;
	float BrightMultiplier;
	bool Accumulate;
	bool IgnoreFog;
	bool IgnoreShroud;
	bool Override;

	PaintballType() : Color { 0,0,0 }
		, BrightMultiplier { 1.0f }
		, Accumulate { false }
		, IgnoreFog { false }
		, IgnoreShroud { false }
		, Override { true }
	{}

	virtual ~PaintballType() = default;

	PaintballType(const PaintballType& nData) : Color { nData.Color }
		, BrightMultiplier { nData.BrightMultiplier }
		, Accumulate { nData.Accumulate }
		, IgnoreFog { nData.IgnoreFog }
		, IgnoreShroud { nData.IgnoreShroud }
		, Override { nData.Override }
	{}

	PaintballType(PaintballType& nData) : Color { nData.Color }
		, BrightMultiplier { nData.BrightMultiplier }
		, Accumulate { nData.Accumulate }
		, IgnoreFog { nData.IgnoreFog }
		, IgnoreShroud { nData.IgnoreShroud }
		, Override { nData.Override }
	{}

	void Read(INI_EX& parser, const char* pSection);

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm) const
	{ return const_cast<PaintballType*>(this)->Serialize(Stm); }

	template <typename T>
	bool Serialize(T& Stm)
	{
		Debug::Log("Processing Element From PaintballType ! \n");

		return Stm
			.Process(Color, false)
			.Process(BrightMultiplier, false)
			.Process(Accumulate, false)
			.Process(IgnoreFog, false)
			.Process(IgnoreShroud, false)
			.Process(Override, false)
			.Success()
			;
	}
};

class PaintBall
{
public:

	virtual ~PaintBall() = default;

	virtual void Enable(int duration, WarheadTypeClass* pAffector, const PaintballType& data);

	virtual void Disable(WarheadTypeClass* pAffector)
	{
		if (Token == pAffector) {
			timer.Stop();
		}
	}

	virtual void Disable(bool bForce) {
		if ((!Token) || bForce) {
			timer.Stop();
			Token = nullptr;
			Data.clear();
		}
	}


	virtual bool IsActive() { return timer.InProgress(); }

	void Enable(int nDuration , PaintballType data, WarheadTypeClass* pAffector)
	{
		Enable(nDuration, pAffector, data);
	}

	//needpaint , changeColor , changeBright
	std::tuple<bool, bool, bool> NeedPaint()
	{
		bool changeColor = false;
		bool changeBright = false;
		bool active = IsActive() && !Data.empty();

		if (active)
		{
			changeColor = Data.get().Color != ColorStruct::Empty;
			changeBright = Data.get().BrightMultiplier != 1.0f;
		}

		return { active  ,  changeColor, changeBright };
	}

	uintptr_t GetColor();

	uintptr_t GetBright(uintptr_t bright)
	{
		const double b = bright * Data.get().BrightMultiplier;
		return static_cast<uintptr_t>(std::clamp(static_cast<int>(b),0,2000));
	}

	void Update(TechnoClass* pThis);

	PaintBall() : Token { }
		, Data { }
		, timer { }
	{ }

	void DrawSHP_Paintball(TechnoClass* pTech, REGISTERS* R);
	void DrawSHP_Paintball_BuildAnim(TechnoClass* pTech, REGISTERS* R);
	void DrawVXL_Paintball(TechnoClass* pTech, REGISTERS* R, bool isBuilding);

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm) const
	{ return const_cast<PaintBall*>(this)->Serialize(Stm); }

	WarheadTypeClass* Token;
	OptionalStruct<PaintballType,true> Data;

private:
	TimerStruct timer;
public:

	template <typename T>
	bool Serialize(T& Stm)
	{
		Debug::Log("Processing Element From PaintBall ! \n");
		return Stm
			.Process(Token,true)
			.Process(Data, true)
			.Process(timer,false)
			.Success()
			;
	}
};


template <>
struct Savegame::ObjectFactory<PaintBall>
{
	std::unique_ptr<PaintBall> operator() (PhobosStreamReader& Stm) const {
		return std::make_unique<PaintBall>();
	}
};

#endif