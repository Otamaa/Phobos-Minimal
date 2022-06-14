#pragma once
#include <string_view>
#include <tuple>
#include <optional>
#include <ColorStruct.h>
#include <BasicStructures.h>
#include <vector>
#include <memory>

enum class EffectTypeIdent : int
{
	None = 0
	, Paintball
};

struct IAEStateData { //dummy

	IAEStateData() : Type { EffectTypeIdent::None }
	{ }

	IAEStateData(EffectTypeIdent nType) : Type { nType }
	{ }

	virtual EffectTypeIdent WhatImI() const {
		return Type;
	}

protected:
	EffectTypeIdent Type;
};

class PaintballType : public IAEStateData {
public:
	ColorStruct Color;
	float BrightMultiplier;

	PaintballType(): IAEStateData { EffectTypeIdent::Paintball}
		, Color { 0,0,0 }
		, BrightMultiplier { 1.0f }
	{}

	PaintballType(const PaintballType& nData) : IAEStateData { EffectTypeIdent::Paintball }
		, Color { nData.Color }
		, BrightMultiplier { nData.BrightMultiplier }
	{}
};


template<typename T>
concept IsEffectType = std::is_base_of<IAEStateData, T>::value;

template <IsEffectType T>
class AEState
{
public:
	virtual void Enable(int duration, const std::string_view token, const T& data)
	{
		Token = token.data();
		Data = data;
		active = duration != 0;

		if (duration < 0) {
			infinite = true;
		} else {
			infinite = false;
			timer.Start(duration);
		};
		OnEnable();
	}

	virtual void OnEnable() { }
	virtual void Disable(const std::string_view token)
	{
		if (Token == token)
		{
			active = false;
			infinite = false;
			timer.Stop();
			OnDisable();
		}
	}

	virtual void OnDisable() { }
	virtual bool IsActive() { return infinite || timer.InProgress(); }

	AEState() : Token { }
		, Data { }
		, active { false }
		, infinite { false }
		, timer { }
	{ }

	void Enable(IAEStateData data) {
		Enable(-1, nullptr, data);
	}

	std::string Token;
	std::optional<T> Data;

protected:
	bool active;
	bool infinite;
	TimerStruct timer;
};

struct StateWrapper //dummy
{
	StateWrapper() : WrapperType { EffectTypeIdent::None }
	{ }

	StateWrapper(EffectTypeIdent nType) : WrapperType { nType }
	{ }

	virtual EffectTypeIdent GetWrapperType() const
	{
		return WrapperType;
	}

protected:
	EffectTypeIdent WrapperType;
};

class PaintballState :public StateWrapper,  public AEState<PaintballType>
{
public:

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

	uintptr_t GetColor() {
		return ToColor16(Data->Color).ToInit();
	}

	uintptr_t GetBright(uintptr_t bright)
	{
		double b = bright * Data->BrightMultiplier;
		if (b < 0)
		{
			b = 0;
		}
		else if (b > 2000)
		{
			b = 2000;
		}
		return static_cast<uintptr_t>(b);
	}

	PaintballState() : StateWrapper { EffectTypeIdent::Paintball }
		, AEState<PaintballType> {}
	{ }
};

class AEManagerDummy
{
public :

	std::shared_ptr<StateWrapper> PaintBallState { };
};