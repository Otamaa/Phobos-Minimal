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

	PaintballType(const PaintballType& nData) :Color { nData.Color }
		, BrightMultiplier { nData.BrightMultiplier }
		, Accumulate { nData.Accumulate }
	{}

	void Read(INI_EX& parser, const char* pSection);

	template <typename T>
	void Serialize(T& Stm)
	{
		Debug::Log("Processing Element From PaintballType ! \n");

		Stm
			.Process(Color)
			.Process(BrightMultiplier)
			.Process(Accumulate)
			;
	}
};

class PaintBall
{
public:
	virtual void Enable(int duration, const std::string_view token, const PaintballType& data);

	virtual void Disable(const std::string_view token)
	{
		if (Token == token) {
			timer.Stop();
		}
	}

	virtual void Disable(bool bForce) {
		if ((!Token.empty()) || bForce) {
			timer.Stop();
			Token.clear();
			Data.reset();
		}
	}


	virtual bool IsActive() { return timer.InProgress(); }

	void Enable(PaintballType data) {
		Enable(-1, "null", data);
	}

	void Enable(int nDuration , PaintballType data, const std::string_view nToken)
	{
		Enable(nDuration, nToken.data(), data);
	}

	//needpaint , changeColor , changeBright
	std::tuple<bool, bool, bool> NeedPaint()
	{
		bool changeColor = false;
		bool changeBright = false;
		bool active = IsActive() && Data.has_value();

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

	std::string Token;
	std::optional<PaintballType> Data;

protected:
	TimerStruct timer;
public:

	template <typename T>
	void Serialize(T& Stm)
	{
		Debug::Log("Processing Element From PaintBall ! \n");
		Stm
			.Process(Token)
			.Process(Data)
			.Process(timer)
			;
	}
};
#endif