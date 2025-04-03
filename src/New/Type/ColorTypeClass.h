#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>

class ColorTypeClass final : public Enumerable<ColorTypeClass>
{
public:

	Valueable<Point3D> Colors { };

	ColorTypeClass(const char* pTitle) : Enumerable<ColorTypeClass>(pTitle)
	{ }

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

	static void LoadFromINIList_New(CCINIClass* pINI, bool bDebug = false);

	COMPILETIMEEVAL OPTIONALINLINE int ToInt() const {
		auto const& nColors = Colors.Get();
		return nColors.X | nColors.Y << 8 | nColors.Z << 16;
	}

	COMPILETIMEEVAL OPTIONALINLINE ColorStruct ToColor() const {
		auto const& nColors = Colors.Get();
		return { nColors.X  , nColors.Y, nColors.Z };
	}

private:
	template <typename T>
	void Serialize(T& Stm);
};
