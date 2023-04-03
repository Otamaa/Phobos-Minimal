#pragma once

#include <New/Type/ColorTypeClass.h>
#include "TemplateDef.h"

namespace detail
{ 
	template <>
	inline bool read<ColorStruct>(ColorStruct& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		ColorStruct buffer {};

		if (parser.Read3Bytes(pSection, pKey, reinterpret_cast<BYTE*>(&buffer))) {
			value.R = std::clamp<BYTE>(buffer.R, (BYTE)0, ColorStruct::Max);
			value.G = std::clamp<BYTE>(buffer.G, (BYTE)0, ColorStruct::Max);
			value.B = std::clamp<BYTE>(buffer.B, (BYTE)0, ColorStruct::Max);

			return true;

		} else if (!parser.empty()) {

			if(auto pColorClass = ColorTypeClass::Find(parser.value())) {
				value = pColorClass->ToColor();
				return true;
			}

			if (IS_SAME_STR_(parser.value(), GameStrings::Grey()))
			{
				value = Drawing::ColorGrey;
				return true;
			}
			else if (IS_SAME_STR_(parser.value(), GameStrings::Red()))
			{
				value = Drawing::ColorRed;
				return true;
			}
			else if (IS_SAME_STR_(parser.value(), GameStrings::Green()))
			{
				value = Drawing::ColorGreen;
				return true;
			}
			else if (IS_SAME_STR_(parser.value(), "Blue"))
			{
				value = Drawing::ColorBlue;
				return true;
			}
			else if (IS_SAME_STR_(parser.value(), GameStrings::Yellow()))
			{
				value = Drawing::ColorYellow;
				return true;
			}
			else if (IS_SAME_STR_(parser.value(), "White"))
			{
				value = Drawing::ColorWhite;
				return true;
			}
			
			if (auto const& nResult = ColorScheme::Find(parser.value()))
			{
				value = nResult->BaseColor;
				return true;
			}

			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid R,G,B color ");
		}
		return false;
	}

	template <>
	inline bool read(DamageDelayTargetFlag& value, INI_EX& parser, const char* pSection, const char* pKey, bool bAllocate)
	{
		if (parser.ReadString(pSection, pKey))
		{			
			const auto cur = parser.value();

			if (IS_SAME_STR_(cur, "Cell"))
			{
				value = DamageDelayTargetFlag::Cell;
			}
			else if (IS_SAME_STR_(cur, "AttachedObject"))
			{
				value = DamageDelayTargetFlag::AttachedObject;
			}
			else if (IS_SAME_STR_(cur, "Invoker"))
			{
				value = DamageDelayTargetFlag::Invoker;
			}
			else if (!INIClass::IsBlank(cur))
			{
				Debug::INIParseFailed(pSection, pKey, cur, nullptr);
				return false;
			}

			return true;
		}

		return false;
	}

	template <>
	inline bool read<TargetZoneScanType>(TargetZoneScanType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			if (IS_SAME_STR_(parser.value(), "same"))
			{
				value = TargetZoneScanType::Same;
			}
			else if (IS_SAME_STR_(parser.value(), "any"))
			{
				value = TargetZoneScanType::Any;
			}
			else if (IS_SAME_STR_(parser.value(), "inrange"))
			{
				value = TargetZoneScanType::InRange;
			}
			else if (!INIClass::IsBlank(parser.value()))
			{
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a target zone scan type");
				return false;
			}

			return true;
		}

		return false;
	}

	template <>
	inline bool read<MouseCursor>(MouseCursor& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		auto ret = false;

		// compact way to define the cursor in one go
		if (parser.ReadString(pSection, pKey))
		{
			auto const buffer = parser.value();
			char* context = nullptr;
			if (auto const pFrame = strtok_s(buffer, Phobos::readDelims, &context))
			{
				Parser<int>::Parse(pFrame, &value.StartFrame);
			}
			if (auto const pCount = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				Parser<int>::Parse(pCount, &value.FrameCount);
			}
			if (auto const pInterval = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				Parser<int>::Parse(pInterval, &value.FrameRate);
			}
			if (auto const pFrame = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				Parser<int>::Parse(pFrame, &value.SmallFrame);
			}
			if (auto const pCount = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				Parser<int>::Parse(pCount, &value.SmallFrameCount);
			}
			if (auto const pHotX = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				MouseCursorHotSpotX::Parse(pHotX, &value.X);
			}
			if (auto const pHotY = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				MouseCursorHotSpotY::Parse(pHotY, &value.Y);
			}

			ret = true;
		}

		return ret;
	}

	template <>
	inline bool read<MouseCursorDataStruct>(MouseCursorDataStruct& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		auto ret = false;

		char pFlagName[32];
		_snprintf_s(pFlagName, 31, "%s.Frame", pKey);
		ret |= read(value.OriginalData.StartFrame, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 31, "%s.Count", pKey);
		ret |= read(value.OriginalData.FrameCount, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 31, "%s.Interval", pKey);
		ret |= read(value.OriginalData.FrameRate, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 31, "%s.MiniFrame", pKey);
		ret |= read(value.OriginalData.SmallFrame, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 31, "%s.MiniCount", pKey);
		ret |= read(value.OriginalData.SmallFrameCount, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 31, "%s.MiniInterval", pKey);
		if (!read(value.SmallFrameRate, parser, pSection, pFlagName))
			value.SmallFrameRate = value.OriginalData.FrameRate;

		_snprintf_s(pFlagName, 31, "%s.HotSpot", pKey);
		if (parser.ReadString(pSection, pFlagName))
		{
			auto const pValue = parser.value();
			char* context = nullptr;
			auto const pHotX = strtok_s(pValue, Phobos::readDelims, &context);
			MouseCursorHotSpotX::Parse(pHotX, &value.OriginalData.X);

			if (auto const pHotY = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				MouseCursorHotSpotY::Parse(pHotY, &value.OriginalData.Y);
			}

			ret = true;
		}

		return ret;
	}

	template <>
	inline bool read<DirType8>(DirType8& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		int nBuffer;
		if (parser.ReadInteger(pSection, pKey , &nBuffer) && nBuffer >= 0 && nBuffer < 8)
		{ 
			value = (DirType8)nBuffer;
			return true;
		}
		else if (!INIClass::IsBlank(parser.value()))
		{
			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a facing between 0 and 8");
		}

		return false;
	}

}