#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/TemplateDefB.h>

class LaserTrailTypeClass final : public Enumerable<LaserTrailTypeClass>
{
public:
	static COMPILETIMEEVAL const char* MainSection = "LaserTrailTypes";
	static COMPILETIMEEVAL const char* ClassName = "LaserTrailTypeClass";

public:
	Valueable<bool> IsHouseColor;
	Valueable<ColorStruct> Color;
	Nullable<int> FadeDuration;
	Valueable<int> Thickness;
	Valueable<int> SegmentLength;
	Valueable<bool> IgnoreVertical;
	Valueable<bool> IsIntense;
	Valueable<int> InitialDelay;
	Valueable<bool> CloakVisible;
	Valueable<bool> CloakVisible_Houses;
	Valueable<bool> DroppodOnly;
	Valueable<bool> Permanent;

	Valueable<LaserTrailDrawType> DrawType;
	Valueable<bool> IsAlternateColor;
	Nullable<ColorStruct> Bolt_Color[3];
	Valueable<bool> Bolt_Disable[3];
	Valueable<int> Bolt_Arcs;
	Nullable<ColorStruct> Beam_Color;
	Valueable<double> Beam_Amplitude;
	Valueable<bool> CanBeHidden;

	LaserTrailTypeClass(const char* pTitle) : Enumerable<LaserTrailTypeClass> { pTitle }
		, IsHouseColor { false }
		, Color { Drawing::DefaultColors[(int)DefaultColorList::Red] }
		, FadeDuration { }
		, Thickness { 4 }
		, SegmentLength { 128 }
		, IgnoreVertical { false }
		, IsIntense { false }
		, InitialDelay {0}
		, CloakVisible { false }
		, CloakVisible_Houses { false }
		, DroppodOnly { false }
		, Permanent { false }

		, DrawType { LaserTrailDrawType::Laser }
		, IsAlternateColor { false }
		, Bolt_Color {}
		, Bolt_Disable { }
		, Bolt_Arcs { 8 }
		, Beam_Color {}
		, Beam_Amplitude { 40.0 }
		, CanBeHidden { true }
	{ }

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};