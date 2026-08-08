#pragma once

#include <New/Type/PaletteManager.h>

#include <Utilities/PhobosPCXFile.h>
#include <Utilities/CSFText.h>
#include <Utilities/TemplateDefB.h>

class BannerTypeClass final : public Enumerable<BannerTypeClass>
{
public:
	static COMPILETIMEEVAL const char* MainSection = "BannerTypes";
	static COMPILETIMEEVAL const char* ClassName = "BannerTypeClass";

public:

	//PCX
	PhobosPCXFile PCX {};

	//SHP
	Valueable<SHPCaches*> Shape {};
	CustomPalette Palette {};

	//CSF
	Valueable<CSFText> CSF {};
	Nullable<ColorStruct> CSF_Color {};
	Valueable<bool> CSF_Background {};
	Valueable<BannerNumberType> CSF_VariableFormat { BannerNumberType::None };

	//Duration
	Valueable<int> Duration { -1 };
	Valueable<int> Delay { -1 };
	Valueable<bool> Shape_RefreshAfterDelay {};

	Valueable<bool> ClampToScreen {};

	BannerTypeClass(const char* const pTitle) : Enumerable<BannerTypeClass>(pTitle)	{ }
	virtual ~BannerTypeClass() = default;

	virtual void LoadFromINI(CCINIClass* pINI);
	virtual void LoadFromStream(PhobosStreamReader& stm);
	virtual void SaveToStream(PhobosStreamWriter& stm);

private:

	template <typename T>
	void Serialize(T& Stm);
};
