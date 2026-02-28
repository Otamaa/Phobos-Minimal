#pragma once
#include <string>
#include <vector>

#include <GeneralStructures.h>

#include <Misc/Kratos/Common/INI/INIConfig.h>

#include <Misc/Kratos/Ext/Common/CommonStatus.h>

class AirstrikeData : public INIConfig
{
public:

	int AirstrikeTargetLaser = -1;
	ColorStruct AirstrikeLineColor = ColorStruct::Red;

	bool AirstrikeDisableLine = false;
	bool AirstrikeDisableColor = false;
	bool AirstrikeDisableBlink = false;

	CoordStruct AirstrikePutOffset = CoordStruct::Empty;

	virtual void Read(INIBufferReader* reader) override
	{
		// 读全局
		INIBufferReader* avReader = INI::GetSection(INI::Rules, INI::SectionAudioVisual);
		AirstrikeTargetLaser = avReader->Get("AirstrikeTargetLaser", AirstrikeTargetLaser);
		if (AirstrikeTargetLaser >= 0)
		{
			AirstrikeLineColor = Drawing::Int_To_RGB(Add2RGB565(RulesClass::Instance->ColorAdd[AirstrikeTargetLaser]));
		}
		else
		{
			AirstrikeLineColor = Drawing::Int_To_RGB(Add2RGB565(RulesClass::Instance->ColorAdd[RulesClass::Instance->LaserTargetColor]));
		}
		AirstrikeDisableLine = reader->Get("AirstrikeDisableLine", AirstrikeDisableLine);
		AirstrikeDisableColor = reader->Get("AirstrikeDisableColor", AirstrikeDisableColor);
		AirstrikeDisableBlink = reader->Get("AirstrikeDisableBlink", AirstrikeDisableBlink);
		AirstrikePutOffset = reader->Get("AirstrikePutOffset", AirstrikePutOffset);

		// 读个体
		AirstrikeTargetLaser = reader->Get("AirstrikeTargetLaser", AirstrikeTargetLaser);
		if (AirstrikeTargetLaser >= 0)
		{
			AirstrikeLineColor = Drawing::Int_To_RGB(Add2RGB565(RulesClass::Instance->ColorAdd[AirstrikeTargetLaser]));
		}
		else
		{
			AirstrikeLineColor = Drawing::Int_To_RGB(Add2RGB565(RulesClass::Instance->ColorAdd[RulesClass::Instance->LaserTargetColor]));
		}
		AirstrikeDisableLine = reader->Get("AirstrikeDisableLine", AirstrikeDisableLine);
		AirstrikeDisableColor = reader->Get("AirstrikeDisableColor", AirstrikeDisableColor);
		AirstrikeDisableBlink = reader->Get("AirstrikeDisableBlink", AirstrikeDisableBlink);
		AirstrikePutOffset = reader->Get("AirstrikePutOffset", AirstrikePutOffset);
	}
};
