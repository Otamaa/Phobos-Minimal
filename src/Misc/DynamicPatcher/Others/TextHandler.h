#pragma once
#ifdef COMPILE_PORTED_DP_FEATURES_

#include <Utilities/TemplateDef.h>
#include <CoordStruct.h>
#include <RectangleStruct.h>
#include <GeneralStructures.h>
#include <TacticalClass.h>
#include <Surface.h>
#include <MapClass.h>

struct PrintTextData
{
	Point2D Offset;
	Point2D ShadowOffset;
	ColorStruct Color;
	ColorStruct ShadowColor;
	bool UseSHP;
	bool CustomSHP;
	PhobosFixedString<0x100> SHPFileName;
	int ZeroFrameIndex;
	Point2D ImageSize;

	PrintTextData() :
		Offset { 0,0 }
		, ShadowOffset { 1, 1 }
		, Color { 252, 252, 252 }
		, ShadowColor { 82, 85, 82 }
		, UseSHP { false }
		, CustomSHP { false }
		, SHPFileName {  }
		, ZeroFrameIndex { 0 }
		, ImageSize { 5,8 }
	{ SHPFileName = "pipsnum.shp"; }

	~PrintTextData() = default;

	void Read(INI_EX& reader, const char* section, const char* title);

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ Debug::LogInfo("Loading Element From PrintTextData ! "); return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm)
	{ return Serialize(Stm); }

private:
	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(Offset)
			.Process(ShadowOffset)
			.Process(Color)
			.Process(ShadowColor)
			.Process(UseSHP)
			.Process(CustomSHP)
			.Process(SHPFileName)
			.Process(ZeroFrameIndex)
			.Process(ImageSize)
			.Success()
			;
	}
};

struct PrintText
{
	wchar_t Text[0x100];
	CoordStruct Location;
	Point2D Offset;
	int Duration;
	CDTimerClass LifeTimer;
	PrintTextData Data;

	PrintText(const wchar_t* text, CoordStruct location, Point2D offset, int duration, PrintTextData data) :
		Text { }
		, Location { location }
		, Offset { offset }
		, Duration { duration }
		, LifeTimer { duration }
		, Data { data }
	{ PhobosCRT::wstrCopy(Text, text); }

	virtual bool CanPrint(Point2D& offset, Point2D& pos, RectangleStruct& bound)
	{
		offset = Offset;
		pos = Point2D::Empty;
		bound = { 0,0,0,0 };
		if (LifeTimer.InProgress() && !LocationOutOfViewOrHiddenInFog(pos, bound))
		{
			return true;
		}
		return false;
	}

	bool LocationOutOfViewOrHiddenInFog(Point2D& pos, RectangleStruct& bound)
	{
		pos = TacticalClass::Instance->CoordsToView(Location);
		bound = DSurface::Composite->Get_Rect();
		if (pos.X < 0 || pos.Y < 0 || pos.X > bound.Width || pos.Y > bound.Height - 32)
			return true;

		if (auto pCell = MapClass::Instance->TryGetCellAt(Location))
			return ((pCell->Flags & CellFlags::Revealed) == CellFlags::Empty);

		return false;
	}

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ Debug::LogInfo("Loading Element From PrintText ! "); return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm)
	{ return Serialize(Stm); }

private:
	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(Text)
			.Process(Location)
			.Process(Offset)
			.Process(Duration)
			.Process(LifeTimer)
			.Process(Data)
			.Success()
			;
	}
};

struct RollingText : public PrintText
{
	int RollSpeed;

	RollingText(const wchar_t* text, CoordStruct location, Point2D offset, int rollSpeed, int duration, PrintTextData data) :
		PrintText(text, location, offset, duration, data)
		, RollSpeed { rollSpeed }
	{ }

	bool CanPrint(Point2D& offset, Point2D& pos, RectangleStruct& bound) override
	{
		if (PrintText::CanPrint(offset, pos, bound))
		{
			Offset -= { 0 , RollSpeed};
			return true;
		}
		return false;
	}

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ Debug::LogInfo("Loading Element From RollingText ! "); return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm)
	{ return Serialize(Stm); }

private:
	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(Text)
			.Process(Location)
			.Process(Offset)
			.Process(Duration)
			.Process(LifeTimer)
			.Process(Data)
			.Process(RollSpeed)
			.Success()
			;
	}
};
#endif