#pragma once

#include <New/Type/PaletteManager.h>

#include <Utilities/PhobosPCXFile.h>
#include <Utilities/CSFText.h>
#include <Utilities/TemplateDefB.h>

class BannerTypeClass final : public Enumerable<BannerTypeClass>
{
public:

	//PCX
	PhobosPCXFile PCX;

	//SHP
	Valueable<SHPStruct*> Shape;
	CustomPalette Palette;

	//CSF
	Valueable<CSFText> CSF;
	Nullable<ColorStruct> CSF_Color;
	Valueable<bool> CSF_Background;
	Valueable<BannerNumberType> CSF_VariableFormat;

	//Duration
	Valueable<int> Duration;
	Valueable<int> Delay;
	Valueable<bool> Shape_RefreshAfterDelay;

	BannerTypeClass(const char* const pTitle) : Enumerable<BannerTypeClass>(pTitle)
		, PCX { }
		, Shape { }
		, Palette { }
		, CSF { }
		, CSF_Color { }
		, CSF_Background { false }
		, CSF_VariableFormat { BannerNumberType::None }
		, Duration { -1 }
		, Delay { -1 }
		, Shape_RefreshAfterDelay { false }
	{ }

	virtual void LoadFromINI(CCINIClass* pINI);
	virtual void LoadFromStream(PhobosStreamReader& stm);
	virtual void SaveToStream(PhobosStreamWriter& stm);

private:

	template <typename T>
	void Serialize(T& Stm);
};
