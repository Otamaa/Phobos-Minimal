#pragma once
#ifdef COMPILE_PORTED_DP_FEATURES_
#include "TextHandler.h"
#include "TextManager.h"
//#include <format>

struct DamageTextData : public PrintTextData
{
	bool Hidden;
	Point2D XOffset;
	Point2D YOffset;
	int RollSpeed;
	int Duration;

	DamageTextData(bool isDamage = true) :
		PrintTextData { }
		, Hidden { false }
		, XOffset { -15, 15 }
		, YOffset { -12, 12 }
		, RollSpeed { 1 }
		, Duration { 75 }
	{
		SHPFileName = "pips.shp";
		ImageSize = { 4, 6 };
		if (isDamage)
		{
			Color = { 252, 0, 0 };
			ZeroFrameIndex = 54;
		}
		else
		{
			Color = { 0, 252, 0 };
			ZeroFrameIndex = 24;
		}
	}

	~DamageTextData() = default;

	void Read(INI_EX& reader, const char* section, const char* title);

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		//Debug::Log("Loading Element From DamageTextData ! \n"); 
		return Serialize(Stm); 
	}

	bool Save(PhobosStreamWriter& Stm)
	{ return Serialize(Stm); }

private:
	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(Offset)
			.Process(ShadowOffset)
			.Process(Color)
			.Process(ShadowColor)
			.Process(UseSHP)
			.Process(CustomSHP)
			.Process(SHPFileName)
			.Process(ZeroFrameIndex)
			.Process(ImageSize)
			.Process(Hidden)
			.Process(XOffset)
			.Process(YOffset)
			.Process(RollSpeed)
			.Process(Duration)
			.Success()
			;
	}
};

struct DamageTextTypeData
{
	bool Hidden;
	DamageTextData Damage;
	DamageTextData Repair;

	DamageTextTypeData()
		: Hidden { false }
		, Damage { true }
		, Repair { false }

	{ }

	void Read(INI_EX& reader, const char* section, const char* title);

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ 
		//Debug::Log("Loading Element From DamageTextTypeData ! \n"); 
		return Serialize(Stm);
	}

	bool Save(PhobosStreamWriter& Stm)
	{ return Serialize(Stm); }

private:
	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(Hidden)
			.Process(Damage)
			.Process(Repair)
			.Success()
			;
	}
};

inline void TechnoClass_ReceiveDamage2_DamageText(TechnoClass* pTechno, int* pRealDamage,DamageTextTypeData& nData)
{
	if (!Phobos::Otamaa::ShowHealthPercentEnabled || !pTechno)
		return;

	DamageTextData data { true };
	std::wstring text = L"";
	wchar_t Buffer[0x100] {};

	int damage = *pRealDamage;
	if (damage > 0)
	{
		data = nData.Damage;
		if (!data.Hidden)
		{
			swprintf_s(Buffer,L"-%d", damage);
		}
	}
	else if (damage < 0)
	{
		data = nData.Repair;
		if (!data.Hidden)
		{
			swprintf_s(Buffer,L"+%d", -damage);
		}
	}


	if (data.Hidden) {
		return;
	}
		text = Buffer;
	if (!text.empty())
	{
		int x = ScenarioGlobal->Random(data.XOffset.X, data.XOffset.Y);
		int y = ScenarioGlobal->Random(data.YOffset.X, data.YOffset.Y) - 15; // 离地高度
		Point2D offset { x, y };
		int length = text.size() / 2;
		if (data.UseSHP)
		{
			offset.X -= data.ImageSize.X * length;
		}
		else
		{
			offset.X -= PrintTextManager::FontSize().X * length;
		}
		CoordStruct location = pTechno->GetCoords();
		PrintTextManager::InsertRollingText(text.c_str(), location, offset, data.RollSpeed, data.Duration, data);
	}
}

#endif