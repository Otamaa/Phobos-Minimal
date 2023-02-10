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

			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid R,G,B color");
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
				return true;
			}
			else if (IS_SAME_STR_(cur, "AttachedObject"))
			{
				value = DamageDelayTargetFlag::AttachedObject;
				return true;
			}
			else if (IS_SAME_STR_(cur, "Invoker"))
			{
				value = DamageDelayTargetFlag::Invoker;
				return true;
			}
			else if (!INIClass::IsBlank(cur))
			{
				Debug::INIParseFailed(pSection, pKey, cur, nullptr);
				return false;
			}

		}

		return false;
	}
}