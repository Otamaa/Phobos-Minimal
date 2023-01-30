#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>

class ColorTypeClass final : public Enumerable<ColorTypeClass>
{
public:

	Valueable<Point3D> Colors { };

	ColorTypeClass(const char* const pTitle) : Enumerable<ColorTypeClass>(pTitle)
	{ }

	virtual ~ColorTypeClass() override = default;

	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm);
	virtual void SaveToStream(PhobosStreamWriter& Stm);

	static void LoadFromINIList_New(CCINIClass* pINI, bool bDebug = false);

	int ToInt() const {
		auto const& nColors = Colors.Get();
		return nColors.X | nColors.Y << 8 | nColors.Z << 16;
	}

	ColorStruct ToColor() const { 
		auto const& nColors = Colors.Get(); 
		return { nColors.X  , nColors.Y, nColors.Z };
	}

private:
	template <typename T>
	void Serialize(T& Stm);
};