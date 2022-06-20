#pragma once

#ifdef COMPILE_PORTED_DP_FEATURES
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

	PaintballType() : Color { 0,0,0 }
		, BrightMultiplier { 1.0f }
		, Accumulate { false }
	{}

	virtual ~PaintballType() = default;

	PaintballType(const PaintballType& nData) : Color { nData.Color }
		, BrightMultiplier { nData.BrightMultiplier }
		, Accumulate { nData.Accumulate }
	{}

	PaintballType(PaintballType& nData) : Color { nData.Color }
		, BrightMultiplier { nData.BrightMultiplier }
		, Accumulate { nData.Accumulate }
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
			.Process(Color)
			.Process(BrightMultiplier)
			.Process(Accumulate)
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
			Data.reset();
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
		bool active = IsActive() && Data.get();

		if (active)
		{
			changeColor = Data->Color != ColorStruct::Empty;
			changeBright = Data->BrightMultiplier != 1.0f;
		}

		return { active  ,changeColor, changeBright };
	}

	uintptr_t GetColor();

	uintptr_t GetBright(uintptr_t bright)
	{
		double b = bright * Data->BrightMultiplier;
		return static_cast<uintptr_t>(std::clamp(static_cast<int>(b),0,2000));
	}

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
	std::unique_ptr<PaintballType> Data;

protected:
	TimerStruct timer;
public:

	template <typename T>
	bool Serialize(T& Stm)
	{
		Debug::Log("Processing Element From PaintBall ! \n");
		return Stm
			.Process(Token)
			.Process(Data)
			.Process(timer)
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