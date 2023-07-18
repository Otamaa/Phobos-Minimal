#pragma once

#include <New/Type/ColorTypeClass.h>

#include "TemplateDef.h"

namespace detail
{
	template <>
	inline bool read<AffectPlayerType>(AffectPlayerType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			char* context = nullptr;
			AffectPlayerType resultData = AffectPlayerType::None;

			for (auto cur = strtok_s(parser.value(), Phobos::readDelims, &context);
				cur;
				cur = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				bool found = false;
				for (const auto& [pStrings, evalue] : EnumFunctions::AffectPlayerType_ToStrings)
				{
					if (IS_SAME_STR_(cur, pStrings))
					{
						found = true;
						resultData |= evalue;
						break;
					}
				}

				if (!found) {
					Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected valid AffectPlayerType value");
				}
			}

			value = resultData;
			return true;
		}

		return false;
	}

	template <>
	inline bool read<SpotlightFlags>(SpotlightFlags& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			char* context = nullptr;
			SpotlightFlags resultData = SpotlightFlags::None;

			for (auto cur = strtok_s(parser.value(), Phobos::readDelims, &context);
				cur;
				cur = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				bool found = false;
				for (const auto& [pStrings , val]: EnumFunctions::SpotlightFlags_ToStrings)
				{
					if (IS_SAME_STR_(cur, pStrings))
					{
						found = true;
						resultData |= val;
						break;
					}
				}

				if (!found) {
					Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected valid SpotlightFlags value");
				}
			}

			value = resultData;
			return true;
		}

		return false;
	}

	template <>
	inline bool read<ColorStruct>(ColorStruct& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (!parser.Read3Bytes(pSection, pKey, (BYTE*)(&value)))
		{
			if (!parser.empty())
			{
				if (auto const pColorClass = ColorTypeClass::Find(parser.value()))
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

				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid R,G,B color");
			}

			return false;
		}

		return true;
	}

	template <>
	inline bool read(DamageDelayTargetFlag& value, INI_EX& parser, const char* pSection, const char* pKey, bool bAllocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			for (size_t i = 0; i < EnumFunctions::DamageDelayTargetFlag_ToStrings.size(); ++i)
			{
				if (IS_SAME_STR_(parser.value(), EnumFunctions::DamageDelayTargetFlag_ToStrings[i]))
				{
					value = (DamageDelayTargetFlag)i;
					return true;
				}
			}

			if (!GameStrings::IsBlank(parser.value())) {
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expect valid DamageDelayTargetFlag");
			}
		}

		return false;
	}

	template <>
	inline bool read<TargetZoneScanType>(TargetZoneScanType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			for (size_t i = 0; i < EnumFunctions::TargetZoneScanType_ToStrings.size(); ++i)
			{
				if (IS_SAME_STR_(parser.value(), EnumFunctions::TargetZoneScanType_ToStrings[i]))
				{
					value = (TargetZoneScanType)i;
					return true;
				}
			}

			if (!GameStrings::IsBlank(parser.value()))
			{
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a target zone scan type");
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
		for (size_t i = 0; i < EnumFunctions::MouseCursorData_ToStrings.size(); ++i)
		{
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
		if (parser.ReadInteger(pSection, pKey, &nBuffer))
		{
			if (nBuffer >= (int)DirType8::Min && nBuffer <= (int)DirType8::Max)
			{
				value = (DirType8)nBuffer;
				return true;
			}

			if (!parser.empty())
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a DirType8 between 0 and 8");
		}

		return false;
	}

	template <>
	inline bool read<DirType32>(DirType32& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		int nBuffer = -1;
		if (parser.ReadInteger(pSection, pKey, &nBuffer))
		{
			if(nBuffer >= (int)DirType32::Min && nBuffer <= (int)DirType32::Max){
				value = (DirType32)nBuffer;
				return true;
			}

			if (!parser.empty())
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a DirType32 between 0 and 32");
		}

		return false;
	}

	template <>
	inline bool read<DirType>(DirType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		int nBuffer = -1;
		if (parser.ReadInteger(pSection, pKey, &nBuffer))
		{
			const bool IsNegative = nBuffer < 0;
			const DirType nVal = (DirType)abs(nBuffer);

			if(DirType::North <= nVal && nVal <= DirType::Max){
				value = (IsNegative ? (DirType)((int)DirType::Max - (int)nVal) : nVal);
				return true;
			}

			if (!parser.empty())
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a DirType between 0 and 255");
		}

		return false;
	}

	template <>
	inline bool read<FacingType>(FacingType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		int buffer;

		if (parser.ReadInteger(pSection, pKey, &buffer))
		{
			if (buffer <= (int)FacingType::Count && buffer >= (int)FacingType::None)
			{
				value = static_cast<FacingType>(buffer);
				return true;
			}

			if (!parser.empty())
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid FacingType (0-7 or -1).");
		}

		return false;
	}

	template <>
	inline bool read<SpotlightAttachment>(SpotlightAttachment& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			for (size_t i = 0; i < EnumFunctions::SpotlightAttachment_ToStrings.size(); ++i)
			{
				if (IS_SAME_STR_(parser.value(), EnumFunctions::SpotlightAttachment_ToStrings[i]))
				{
					value = (SpotlightAttachment)i;
					return true;
				}
			}

			if (!parser.empty())
				Debug::INIParseFailed(pSection, pKey, parser.value(),"Expect valid SpotlightAttachment");
		}

		return false;
	}

	template <>
	inline bool read<ShowTimerType>(ShowTimerType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			for (size_t i = 0; i < EnumFunctions::ShowTimerType_ToStrings.size(); ++i)
			{
				if (IS_SAME_STR_(parser.value(), EnumFunctions::ShowTimerType_ToStrings[i]))
				{
					value = (ShowTimerType)i;
					return true;
				}
			}

			if (!parser.empty())
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expect valid ShowTimerType");
		}

		return false;
	}

	template <>
	inline bool read<BountyValueOption>(BountyValueOption& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			const auto pVal = parser.value();

			for (size_t i = 0; i < EnumFunctions::BountyValueOption_ToSrings.size(); ++i)
			{
				if (IS_SAME_STR_(pVal, EnumFunctions::BountyValueOption_ToSrings[i]))
				{
					value = (BountyValueOption)i;
					return true;
				}
			}

			if (!parser.empty())
				Debug::INIParseFailed(pSection, pKey, pVal, "Expect valid BountyValueOption");
		}

		return false;
	}

	template <>
	inline bool read<BuildingSelectBracketPosition>(BuildingSelectBracketPosition& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			const auto str = parser.value();

			for (size_t i = 0; i < EnumFunctions::BuildingSelectBracketPosition_ToSrings.size(); ++i) {
				if (IS_SAME_STR_(str, EnumFunctions::BuildingSelectBracketPosition_ToSrings[i])) {
					value = BuildingSelectBracketPosition(i);
					return true;
				}
			}

			if (!parser.empty())
				Debug::INIParseFailed(pSection, pKey, str, "Expect valid BuildingSelectBracketPosition");
		}

		return false;
	}

	template <>
	inline bool read<DisplayInfoType>(DisplayInfoType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			auto str = parser.value();

			for (size_t i = 0; i < EnumFunctions::DisplayInfoType_ToSrings.size(); ++i) {
				if (IS_SAME_STR_(str, EnumFunctions::DisplayInfoType_ToSrings[i])) {
					value = DisplayInfoType(i);
					return true;
				}
			}

			if(!parser.empty())
				Debug::INIParseFailed(pSection, pKey, str, "Expect valid DisplayInfoType");
		}

		return false;
	}

	template<typename T, bool Alloc = false>
	inline void ParseVector(INI_EX& IniEx, std::vector<std::vector<T>>& nVecDest, const char* pSection, bool bDebug = true, bool bVerbose = false, const char* Delims = Phobos::readDelims, const char* message = nullptr)
	{
		static_assert(std::is_pointer<T>::value, "Pointer Required !");
		using baseType = std::remove_pointer_t<T>;

		if (!IniEx->GetSection(pSection))
			return;

		nVecDest.clear();
		const auto nKeyCount = IniEx->GetKeyCount(pSection);

		if (!nKeyCount)
			return;

		nVecDest.resize(nKeyCount);

		for (int i = 0; i < nKeyCount; ++i)
		{
			if (!IniEx.ReadString(pSection, IniEx->GetKeyName(pSection, i)))
				continue;

			char* context = nullptr;
			for (char* cur = strtok_s(IniEx.value(), Delims, &context);
				cur; cur = strtok_s(nullptr, Delims, &context))
			{
				const auto res = CRT::strtrim(cur);

				T buffer = nullptr;
				if constexpr (!Alloc)
					buffer = baseType::Find(res);
				else
					buffer = baseType::FindOrAllocate(res);

				if (bVerbose)
					Debug::Log("ParseVector DEBUG: [%s][%d]: Verose parsing [%s]\n", pSection, i, res);

				if (buffer) {
					nVecDest[i].push_back(buffer);
				}else if (bDebug && !GameStrings::IsBlank(cur)) {
					Debug::Log("ParseVector DEBUG: [%s][%d]: Error parsing [%s]\n", pSection, i, res);
				}
			}
		}
	}

	inline void ParseVector(INI_EX& IniEx, std::vector<std::vector<std::string>>& nVecDest, const char* pSection, bool bDebug = true, bool bVerbose = false, const char* Delims = Phobos::readDelims, const char* message = nullptr)
	{
		if (!IniEx->GetSection(pSection))
			return;

		nVecDest.clear();

		const auto nKeyCount = IniEx->GetKeyCount(pSection);
		if (!nKeyCount)
			return;

		nVecDest.resize(nKeyCount);

		for (int i = 0; i < nKeyCount; ++i)
		{
			char* context = nullptr;
			if (!IniEx.ReadString(pSection, IniEx->GetKeyName(pSection, i)))
				continue;

			for (char* cur = strtok_s(IniEx.value(), Delims, &context);
				cur;
				cur = strtok_s(nullptr, Delims, &context))
			{
				const auto res = CRT::strtrim(cur);

				if (res && res[0] && strlen(res))
					nVecDest[i].push_back(res);

				if (bVerbose)
					Debug::Log("ParseVector DEBUG: [%s][%d]: Verose parsing [%s]\n", pSection, i, cur);
			}
		}
	}

	template<typename T, bool Allocate = false, bool Unique = false>
	inline void ParseVector(DynamicVectorClass<T>& List, INI_EX& IniEx , const char* section, const char* key , const char* message = nullptr)
	{
		if (IniEx.ReadString(section,key))
		{
			List.Clear();
			char* context = nullptr;

			if constexpr (std::is_pointer<T>())
			{
				using BaseType = std::remove_pointer_t<T>;

				for (char* cur = strtok_s(IniEx.value(), Phobos::readDelims, &context); cur;
					 cur = strtok_s(nullptr, Phobos::readDelims, &context))
				{
					BaseType* buffer = nullptr;
					if constexpr (Allocate)
					{
						buffer = BaseType::FindOrAllocate(cur);
					}
					else
					{
						buffer = BaseType::Find(cur);
					}

					if (buffer)
					{
						if constexpr (!Unique)
						{
							List.AddItem(buffer);
						}
						else if(!GameStrings::IsBlank(cur))
						{
							List.AddUnique(buffer);
						}
					}
					else if(!GameStrings::IsBlank(cur))
					{
						Debug::INIParseFailed(section, key, cur, message);
					}
				}
			}
			else
			{
				for (char* cur = strtok_s(IniEx.value(), Phobos::readDelims, &context); cur;
					 cur = strtok_s(nullptr, Phobos::readDelims, &context))
				{
					T buffer = T();
					if (Parser<T>::TryParse(cur, &buffer))
					{
						if constexpr (!Unique)
						{
							List.AddItem(buffer);
						}
						else
						{
							List.AddUnique(buffer);
						}
					}
					else if (!GameStrings::IsBlank(cur))
					{
						Debug::INIParseFailed(section, key, cur, message);
					}
				}
			}
		}
	};
}