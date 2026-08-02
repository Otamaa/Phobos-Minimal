#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/TemplateDefB.h>

class LaserTrailTypeClass final : public Enumerable<LaserTrailTypeClass>
{
public:
	static COMPILETIMEEVAL const char* MainSection = "LaserTrailTypes";
	static COMPILETIMEEVAL const char* ClassName = "LaserTrailTypeClass";

public:
	Valueable<bool> IsHouseColor { false };
	Valueable<ColorStruct> Color { Drawing::DefaultColors[(int)DefaultColorList::Red] };
	Nullable<int> FadeDuration {};                         // value‑initialised
	Valueable<int> Thickness { 4 };
	Valueable<int> SegmentLength { 128 };
	Valueable<bool> IgnoreVertical { false };
	Valueable<bool> IsIntense { false };
	Valueable<int> InitialDelay { 0 };
	Valueable<bool> CloakVisible { false };
	Valueable<bool> CloakVisible_Houses { false };
	Valueable<bool> DroppodOnly { false };
	Valueable<bool> Permanent { false };

	Valueable<LaserTrailDrawType> DrawType { LaserTrailDrawType::Laser };
	Valueable<bool> IsAlternateColor { false };
	Nullable<ColorStruct> Bolt_Color[3] {};               // zero‑initialises each element
	Valueable<bool> Bolt_Disable[3] {};                   // value‑initialises each element
	Valueable<int> Bolt_Arcs { 8 };
	Nullable<ColorStruct> Beam_Color {};
	Valueable<double> Beam_Amplitude { 40.0 };
	Valueable<bool> CanBeHidden { true };

	Valueable<int> Bolt_ZAdjust { 0 };
	Valueable<int> Laser_ZAdjust { 0 };

	LaserTrailTypeClass(const char* pTitle) : Enumerable<LaserTrailTypeClass> { pTitle } { }
	virtual ~LaserTrailTypeClass() = default;

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};