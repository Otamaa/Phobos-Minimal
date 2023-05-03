#pragma once

#include <New/Type/ColorTypeClass.h>

#include "TemplateDef.h"

namespace detail
{ 
	template <>
	inline bool read<ColorStruct>(ColorStruct& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (!parser.Read3Bytes(pSection, pKey, (BYTE*)(&value))) {
			if (!parser.empty()) {

				if (auto const& pColorClass = ColorTypeClass::Find(parser.value()))
				{
					value = pColorClass->ToColor();
					return true;
				}

				for (size_t i = 0; i < EnumFunctions::DefaultGameColor_ToStrings.size(); ++i)
				{
					if (IS_SAME_STR_(parser.value(), EnumFunctions::DefaultGameColor_ToStrings[i]))
					{
						value = Drawing::DefaultColors[i];
						return true;
					}
				}

				if (ColorScheme::Array->Count)
				{
					if (auto const& nResult = ColorScheme::Find(parser.value()))
					{
						value = nResult->BaseColor;
						return true;
					}
				}

				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid R,G,B color ");
				return false;
			}
		}

		return true;
	}

	template <>
	inline bool read(DamageDelayTargetFlag& value, INI_EX& parser, const char* pSection, const char* pKey, bool bAllocate)
	{
		if (parser.ReadString(pSection, pKey))
		{			
			for (size_t i = 0; i < EnumFunctions::DamageDelayTargetFlag_ToStrings.size(); ++i) {
				if (IS_SAME_STR_(parser.value(), EnumFunctions::DamageDelayTargetFlag_ToStrings[i])) {
					value = (DamageDelayTargetFlag)i;
					return true;
				}
			}

			if (!INIClass::IsBlank(parser.value())) {
				Debug::INIParseFailed(pSection, pKey, parser.value(), nullptr);
			}
		}

		return false;
	}

	template <>
	inline bool read<TargetZoneScanType>(TargetZoneScanType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			for (size_t i = 0; i < EnumFunctions::TargetZoneScanType_ToStrings.size(); ++i) {
				if (IS_SAME_STR_(parser.value(), EnumFunctions::TargetZoneScanType_ToStrings[i])) {
					value = (TargetZoneScanType)i;
					return true;
				}
			}

			if (!INIClass::IsBlank(parser.value())) {
				Debug::INIParseFailed(pSection, pKey, parser.value(), "[Phobos] Expected a target zone scan type");
			}		
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

		char pFlagName[0x40];
		for (size_t i = 0; i < EnumFunctions::MouseCursorData_ToStrings.size(); ++i) {
			IMPL_SNPRNINTF(pFlagName, sizeof(pFlagName), EnumFunctions::MouseCursorData_ToStrings[i], pKey);
			ret |= read(value.OriginalData.StartFrame, parser, pSection, pFlagName);
		}

		if (value.SmallFrameRate == -1)
			value.SmallFrameRate = value.OriginalData.FrameRate;

		IMPL_SNPRNINTF(pFlagName, sizeof(pFlagName), "%s.HotSpot", pKey);
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
		int nBuffer = -1;
		if (parser.ReadInteger(pSection, pKey , &nBuffer) && nBuffer >= 0 && nBuffer < 8)
		{
			value = (DirType8)nBuffer;
			return true;
		}
		else
		{
			if(strlen(parser.value()))
				Debug::INIParseFailed(pSection, pKey, parser.value(), "[Phobos] Expected a facing between 0 and 8");
		}

		return false;
	}

	template <>
	inline bool read<DirType32>(DirType32& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		int nBuffer = -1;
		if (parser.ReadInteger(pSection, pKey, &nBuffer) && nBuffer >= 0 && nBuffer < 32)
		{
			value = (DirType32)nBuffer;
			return true;
		}
		else
		{
			if (strlen(parser.value()))
				Debug::INIParseFailed(pSection, pKey, parser.value(), "[Phobos] Expected a facing between 0 and 32");
		}

		return false;
	}

}