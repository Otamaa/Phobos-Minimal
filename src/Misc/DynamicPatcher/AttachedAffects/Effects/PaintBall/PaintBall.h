#pragma once

#include <Utilities/OptionalStruct.h>

#include <ColorStruct.h>
#include <GeneralStructures.h>

#include <string_view>
#include <optional>

class TechnoClass;
class REGISTERS;
class INI_EX;
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
		//Debug::LogInfo("Processing Element From PaintballType ! ");

		return Stm
			.Process(Color, false)
			.Process(BrightMultiplier, false)
			.Process(Accumulate, false)
			.Process(IgnoreFog, false)
			.Process(IgnoreShroud, false)
			.Process(Override, false)
			.Success()
			&& Stm.RegisterChange(this)
			; // announce this type
	}
};

class PaintBall
{
public:

	bool IsActive() { return timer.GetTimeLeft() > 0; }
	void Init();
	void SetData(PaintballType& type) {
		this->Data = &type;
	}

	bool AllowDraw(TechnoClass* pWho);

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm) const
	{ return const_cast<PaintBall*>(this)->Serialize(Stm); }


	CDTimerClass timer;
	PaintballType* Data;
	OptionalStruct<uintptr_t, true> Color;
public:

	template <typename T>
	bool Serialize(T& Stm)
	{
		//Debug::LogInfo("Processing Element From PaintBall ! ");
		return Stm
			.Process(timer,true)
			.Process(Data, true)
			.Process(Color)
			.Success()
			;
	}
};


