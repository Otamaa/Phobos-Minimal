#pragma once

#include <New/Type/ColorTypeClass.h>

#include "TemplateDef.h"

namespace detail
{
	template <>
	inline bool read<ColorStruct>(ColorStruct& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (!parser.Read3Bytes(pSection, pKey, (BYTE*)(&value)))
		{
			if (!parser.empty())
			{

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

			if (!GameStrings::IsBlank(parser.value()))
			{
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
		if (parser.ReadInteger(pSection, pKey, &nBuffer) && nBuffer >= 0 && nBuffer <= 8)
		{
			value = (DirType8)nBuffer;
			return true;
		}
		else
		{
			if (!parser.empty())
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a facing between 0 and 8");
		}

		return false;
	}

	template <>
	inline bool read<DirType32>(DirType32& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		int nBuffer = -1;
		if (parser.ReadInteger(pSection, pKey, &nBuffer) && nBuffer >= 0 && nBuffer <= 32)
		{
			value = (DirType32)nBuffer;
			return true;
		}
		else
		{
			if (!parser.empty())
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a facing between 0 and 32");
		}

		return false;
	}

	template <>
	inline bool read<DirType>(DirType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		int nBuffer = -1;
		if (parser.ReadInteger(pSection, pKey, &nBuffer))
		{
			if (nBuffer == -1) //uhh ROTE using -1 , so it will generate bunch of debug log
				return false;

			if(nBuffer >= 0 && nBuffer <= 255){
				value = (DirType)nBuffer;
				return true;
			}
		}
		else
		{
			if (!parser.empty())
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a facing between 0 and 255");
		}

		return false;
	}

	template <>
	inline bool read<SpotlightAttachment>(SpotlightAttachment& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			if (IS_SAME_STR_(parser.value(), "body"))
			{
				value = SpotlightAttachment::Body;
				return true;
			}
			else if (IS_SAME_STR_(parser.value(), "turret"))
			{
				value = SpotlightAttachment::Turret;
				return true;
			}
			else if (IS_SAME_STR_(parser.value(), "barrel"))
			{
				value = SpotlightAttachment::Barrel;
				return true;
			}
			else
			{
				Debug::INIParseFailed(pSection, pKey, parser.value());
			}
		}

		return false;
	}

	template<typename T, bool Alloc = false>
	inline void ParseVector(INI_EX& IniEx, std::vector<std::vector<T>>& nVecDest, const char* pSection, bool bDebug = true, bool bVerbose = false, const char* Delims = Phobos::readDelims, const char* message = nullptr)
	{
		static_assert(std::is_pointer<T>::value, "Pointer Required !");
		using baseType = std::remove_pointer_t<T>;
		nVecDest.clear();

		if (!IniEx->GetSection(pSection))
			return;

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
				cur; cur = strtok_s(nullptr, Delims, &context))
			{
				const auto res = CRT::strtrim(cur);

				T buffer = nullptr;
				if constexpr (!Alloc)
					buffer = baseType::Find(res);
				else
					buffer = baseType::FindOrAllocate(res);

				if (buffer)
				{
					nVecDest[i].push_back(buffer);

					if (bVerbose)
						Debug::Log("ParseVector DEBUG: [%s][%d]: Verose parsing [%s]\n", pSection, nVecDest.size(), res);

					return;
				}

				if (bDebug && !GameStrings::IsBlank(cur)) {
					Debug::Log("ParseVector DEBUG: [%s][%d]: Error parsing [%s]\n", pSection, nVecDest.size(), res);
				}
			}
		}
	}

	inline void ParseVector(INI_EX& IniEx, std::vector<std::vector<std::string>>& nVecDest, const char* pSection, bool bDebug = true, bool bVerbose = false, const char* Delims = Phobos::readDelims, const char* message = nullptr)
	{
		nVecDest.clear();

		if (!IniEx->GetSection(pSection))
			return;

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
					Debug::Log("ParseVector DEBUG: [%s][%d]: Verose parsing [%s]\n", pSection, nVecDest.size(), cur);
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

	// designated format = key1 : key2 , ....
	template<typename Tkey1, typename TKey2>
	inline void ParseVectorOfPair(std::vector<std::pair<Tkey1, TKey2>>& v_pairs, INI_EX& parser, const char* section, const char* key)
	{
		if (parser.ReadString(section, key))
		{
			v_pairs.clear();
			char* context = nullptr;
			for (auto cur = strtok_s(parser.value(), Phobos::readDelims, &context);
			cur;
			cur = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				std::string pairs(cur);
				if (!pairs.empty())
				{
					std::erase(pairs, ' ');

					const auto nDelim = pairs.find(":");
					if (nDelim == std::string::npos)
						continue;

					auto nFirst = pairs.substr(0, nDelim);
					auto nSecond = pairs.substr(nDelim + 1);

					std::pair<Tkey1, TKey2 > buffer;
					Parser<Tkey1>::Parse(nFirst.c_str(), &buffer.first);
					Parser<TKey2>::Parse(nSecond.c_str(), &buffer.second);

					v_pairs.push_back(buffer);
				}
			}
		}
	}

}