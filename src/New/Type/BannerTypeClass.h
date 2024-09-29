#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/TemplateDefB.h>
#include <Utilities/Enum.h>
#include <Utilities/PhobosFixedString.h>

enum class BannerType : int
{
	PCX = 0,
	CSF = 1,
	SHP = 2,
	VariableFormat = 3
};

class BannerTypeClass final : public Enumerable<BannerTypeClass>
{
public:
	// read from INI
	struct Content
	{
		PhobosFixedString<0x20> PCX;
		struct SHP
		{
			PhobosFixedString<0x20> _;
			PhobosFixedString<0x20> Palette;
		};
		SHP SHP;
		struct CSF
		{
			Valueable<CSFText> _;
			Nullable<ColorStruct> Color;
			Valueable<bool> DrawBackground;
		};
		CSF CSF;
		struct VariableFormat
		{
			Valueable<BannerNumberType> _;
			Valueable<CSFText> Label;
		};
		VariableFormat VariableFormat;
	};
	Content Content;
	// internal
	BannerType BannerType;
	SHPStruct* ImageSHP;
	BSurface* ImagePCX;
	ConvertClass* Palette;

	BannerTypeClass(const char* pTitle) : Enumerable<BannerTypeClass>(pTitle)
		, Content {}
		, BannerType { BannerType::CSF }
		, ImageSHP {}
		, ImagePCX {}
		, Palette {}
	{ }

	void LoadImage();

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};