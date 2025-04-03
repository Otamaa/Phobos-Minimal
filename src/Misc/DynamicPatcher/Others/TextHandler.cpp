#ifdef COMPILE_PORTED_DP_FEATURES_
#include "TextHandler.h"
#include "TextManager.h"
#include "DamageText.h"
#include <Utilities/TemplateDefB.h>

void PrintTextData::Read(INI_EX& reader, const char* section, const char* title)
{
	char nBuff[0x100];
	IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "%sOffset", title);
	Valueable<Point2D> Offs { Offset };
	Offs.Read(reader, section, nBuff);
	Offset = Offs.Get();

	Offs = ShadowOffset ;
	IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "%sShadowOffset", title);
	Offs.Read(reader, section, nBuff);
	ShadowOffset = Offs.Get();

	IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "%sColor", title);
	Valueable<ColorStruct> Col { Color };
	Col.Read(reader, section, nBuff);
	Color = Col.Get();

	Col = ShadowColor;
	IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "%sShadowColor", title);
	Col.Read(reader, section, nBuff);
	ShadowColor = Col.Get();

	Valueable<bool> bUseShp{ UseSHP };
	IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "%sUseSHP", title);
	bUseShp.Read(reader, section, nBuff);
	UseSHP = bUseShp.Get();

	IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "%sSHP", title);

	if (reader.ReadString(section, nBuff))
	{
		if (GeneralUtils::IsValidString(reader.value()))
		{
			if (IS_SAME_STR_(reader.value(), "pips.shp"))
			{
				CustomSHP = true;
				SHPFileName = reader.value();
			}
		}
	}

	Valueable<int> nInt { ZeroFrameIndex };
	IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "%sZeroFrameIndex", title);
	nInt.Read(reader, section, nBuff);
	ZeroFrameIndex = nInt.Get();

	Valueable<Point2D> nPoint { ImageSize };
	IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "%sImageSize", title);
	nPoint.Read(reader, section, nBuff);
	ImageSize = nPoint.Get();
}

Point2D PrintTextManager::fontSize = Point2D::Empty;
std::queue<RollingText> PrintTextManager::rollingTextQueue {};

void DamageTextData::Read(INI_EX& reader, const char* section, const char* title)
{
	char nBuff[0x100];
	PrintTextData::Read(reader, section, title);

	Valueable<bool> bUseShp { Hidden };
	IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "%sHidden", title);
	bUseShp.Read(reader, section, nBuff);
	Hidden = bUseShp.Get();

	IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "%sXOffset", title);
	Nullable<Point2D> Offs {  };
	Offs.Read(reader, section, nBuff);
	if (Offs.isset())
	{
		Point2D offset = Offs.Get();
		if (offset.X > offset.Y)
		{
			offset.X = Offs.Get().Y;
			offset.Y = Offs.Get().X;
		}

		XOffset = offset;
	}

	IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "%sYOffset", title);
	Offs.Read(reader, section, nBuff);
	if (Offs.isset())
	{
		Point2D offset = Offs.Get();
		if (offset.X > offset.Y)
		{
			offset.X = Offs.Get().Y;
			offset.Y = Offs.Get().X;
		}

		YOffset = offset;
	}

	Valueable<int> nInt { RollSpeed };
	IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "%sRollSpeed", title);
	nInt.Read(reader, section, nBuff);
	RollSpeed = nInt.Get();

	nInt = Duration;
	IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "%sDuration", title);
	nInt.Read(reader, section, nBuff);
	Duration = nInt.Get();
}

void DamageTextTypeData::Read(INI_EX& reader, const char* section, const char* title)
{
	char nBuff[0x100];
	Valueable<bool> nHidden { Hidden };
	IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "%sHidden", title);
	nHidden.Read(reader, section, nBuff);
	Hidden = nHidden.Get();

	//Damage.Read(reader, section, title);
	//Repair.Read(reader, section, title);

	IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "%sDamage.", title);
	Damage.Read(reader, section, nBuff);
	IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "%sRepair.", title);
	Repair.Read(reader, section, nBuff);

}
#endif
