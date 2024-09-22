#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/TemplateDefB.h>

class LaserTrailTypeClass final : public Enumerable<LaserTrailTypeClass>
{
public:
	Valueable<bool> IsHouseColor;
	Valueable<ColorStruct> Color;
	Valueable<int> FadeDuration;
	Valueable<int> Thickness;
	Valueable<int> SegmentLength;
	Valueable<bool> IgnoreVertical;
	Valueable<bool> IsIntense;
	Valueable<int> InitialDelay;
	Valueable<bool> CloakVisible;
	Valueable<bool> CloakVisible_Houses;
	Valueable<bool> DroppodOnly;

	LaserTrailTypeClass(const char* pTitle) : Enumerable<LaserTrailTypeClass> { pTitle }
		, IsHouseColor { false }
		, Color { Drawing::DefaultColors[(int)DefaultColorList::Red] }
		, FadeDuration { 64 }
		, Thickness { 4 }
		, SegmentLength { 128 }
		, IgnoreVertical { false }
		, IsIntense { false }
		, InitialDelay {0}
		, CloakVisible { false }
		, CloakVisible_Houses { false }
		, DroppodOnly { false }
	{ }

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};