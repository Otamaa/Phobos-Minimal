#pragma region Ares Copyrights
/*
 *Copyright (c) 2008+, All Ares Contributors
 *All rights reserved.
 *
 *Redistribution and use in source and binary forms, with or without
 *modification, are permitted provided that the following conditions are met:
 *1. Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *2. Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *3. All advertising materials mentioning features or use of this software
 *   must display the following acknowledgement:
 *   This product includes software developed by the Ares Contributors.
 *4. Neither the name of Ares nor the
 *   names of its contributors may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 *THIS SOFTWARE IS PROVIDED BY ITS CONTRIBUTORS ''AS IS'' AND ANY
 *EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *DISCLAIMED. IN NO EVENT SHALL THE ARES CONTRIBUTORS BE LIABLE FOR ANY
 *DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#pragma endregion

#pragma once

#include <Windows.h>

#include "Template.h"

#include "INIParser.h"
#include "Enum.h"
#include "Constructs.h"
#include "SavegameDef.h"
#include "TranslucencyLevel.h"
#include "GeneralUtils.h"

#include <InfantryTypeClass.h>
#include <AircraftTypeClass.h>
#include <UnitTypeClass.h>
#include <BuildingTypeClass.h>
#include <FootClass.h>
#include <VocClass.h>
#include <VoxClass.h>

#include <FileFormats/_Loader.h>
#include <Helpers/Enumerators.h>

//TODO : Replace more strings with game define strings !

namespace detail
{
	template <typename T>
	inline bool read(T& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate = false)
	{
		if (parser.ReadString(pSection, pKey))
		{
			using base_type = std::remove_pointer_t<T>;

			auto const pValue = parser.value();
			auto const parsed = (allocate ? base_type::FindOrAllocate : base_type::Find)(pValue);

			if (parsed || INIClass::IsBlank(pValue))
			{
				value = parsed;
				return true;
			}
			else
			{
				Debug::INIParseFailed(pSection, pKey, pValue, nullptr);
			}
		}
		return false;
	}

	/*template <>
	inline bool read<WarheadTypeClass*>(WarheadTypeClass*& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			auto const pValue = parser.value();
			auto const parsed = (allocate ? parser.GetINI()->WarheadTypeClass_FindOrMake(pSection, pKey, nullptr) :
									WarheadTypeClass::Find(pValue));

			if (parsed || INIClass::IsBlank(pValue))
			{
				value = parsed;
				return true;
			}
			else
			{
				Debug::INIParseFailed(pSection, pKey, pValue, nullptr);
			}
		}
		return false;
	}

	template <>
	inline bool read<AircraftTypeClass*>(AircraftTypeClass*& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			auto const pValue = parser.value();
			auto const parsed = (allocate ? parser.GetINI()->AircraftTypeClass_FindOrMake(pSection, pKey, nullptr) :
									AircraftTypeClass::Find(pValue));

			if (parsed || INIClass::IsBlank(pValue))
			{
				value = parsed;
				return true;
			}
			else
			{
				Debug::INIParseFailed(pSection, pKey, pValue, nullptr);
			}
		}
		return false;
	}

	template <>
	inline bool read<InfantryTypeClass*>(InfantryTypeClass*& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			auto const pValue = parser.value();
			auto const parsed = (allocate ? parser.GetINI()->InfantryTypeClass_FindOrMake(pSection, pKey, nullptr) :
									InfantryTypeClass::Find(pValue));

			if (parsed || INIClass::IsBlank(pValue))
			{
				value = parsed;
				return true;
			}
			else
			{
				Debug::INIParseFailed(pSection, pKey, pValue, nullptr);
			}
		}
		return false;
	}*/

	template <>
	inline bool read<PartialVector2D<int>>(PartialVector2D<int>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		value.ValueCount = parser.Read2IntegerAndCount(pSection, pKey, (int*)&value);

		if (value.ValueCount > 0)
			return true;

		return false;
	}

	template <>
	inline bool read<PartialVector2D<double>>(PartialVector2D<double>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		value.ValueCount = parser.Read2DoubleAndCount(pSection, pKey, (double*)&value);

		if (value.ValueCount > 0)
			return true;

		return false;
	}

	template <>
	inline bool read<PartialVector3D<int>>(PartialVector3D<int>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		value.ValueCount = parser.Read3IntegerAndCount(pSection, pKey, (int*)&value);

		if (value.ValueCount > 0)
			return true;

		return false;
	}

	template <>
	inline bool read<PartialVector3D<double>>(PartialVector3D<double>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		value.ValueCount = parser.Read3DoubleAndCount(pSection, pKey, (double*)&value);

		if (value.ValueCount > 0)
			return true;

		return false;
	}


	template <>
	inline bool read(TechnoTypeClass*& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			auto const pValue = parser.value();
			auto const parsed = !allocate ? TechnoTypeClass::Find(pValue) : nullptr;
			if (parsed || INIClass::IsBlank(pValue))
			{
				value = parsed;
				return true;
			}
			else
			{
				Debug::INIParseFailed(pSection, pKey, pValue, nullptr);
			}
		}
		return false;
	}

	template <>
	inline bool read<TranslucencyLevel>(TranslucencyLevel& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		return value.Read(parser, pSection, pKey);
	}

	template <>
	inline bool read<HorizontalPosition>(HorizontalPosition& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			if (IS_SAME_STR_(parser.value(), "left"))
			{
				value = HorizontalPosition::Left;
			}
			else if (IS_SAME_STR_(parser.value(), "center") || IS_SAME_STR_(parser.value(), "centre"))
			{
				value = HorizontalPosition::Center;
			}
			else if (IS_SAME_STR_(parser.value(), "right"))
			{
				value = HorizontalPosition::Right;
			}
			else
			{
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Horizontal Position can be either Left, Center/Centre or Right");
				return false;
			}
			return true;
		}
		return false;
	}

	template <>
	inline bool read<BannerNumberType>(BannerNumberType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			auto parsed = BannerNumberType::None;
			auto str = parser.value();
			if (IS_SAME_STR_(str, "variable"))
			{
				parsed = BannerNumberType::Variable;
			}
			else if (IS_SAME_STR_(str, "prefixed"))
			{
				parsed = BannerNumberType::Prefixed;
			}
			else if (IS_SAME_STR_(str, "suffixed"))
			{
				parsed = BannerNumberType::Suffixed;
			}
			else if (IS_SAME_STR_(str, "fraction"))
			{
				parsed = BannerNumberType::Fraction;
			}
			else if (IS_SAME_STR_(str, NONE_STR2))
			{
				Debug::INIParseFailed(pSection, pKey, parser.value(),
					"Content.VariableFormat can be either none, prefixed, suffixed or fraction");
				return false;
			}
			if (parsed != BannerNumberType::None)
				value = parsed;
			return true;
		}
		return false;
	}

	template <>
	inline bool read<VerticalPosition>(VerticalPosition& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			if (IS_SAME_STR_(parser.value(), "top"))
			{
				value = VerticalPosition::Top;
			}
			else if (IS_SAME_STR_(parser.value(), "center") || IS_SAME_STR_(parser.value(), "centre"))
			{
				value = VerticalPosition::Center;
			}
			else if (IS_SAME_STR_(parser.value(), "bottom"))
			{
				value = VerticalPosition::Bottom;
			}
			else
			{
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Vertical Position can be either Top, Center/Centre or Bottom");
				return false;
			}
			return true;
		}
		return false;
	}

	template <>
	inline bool read<bool>(bool& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		bool buffer;
		if (parser.ReadBool(pSection, pKey, &buffer))
		{
			value = buffer;
			return true;
		}
		else if (!parser.empty())
		{
			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid boolean value [1, true, yes, 0, false, no]");
		}
		return false;
	}

	template <>
	inline bool read<int>(int& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		int buffer;
		if (parser.ReadInteger(pSection, pKey, &buffer))
		{
			value = buffer;
			return true;

		} else if (!parser.empty()) {
			bool bufferb;
			if(!parser.ReadBool(pSection, pKey, &bufferb)) {
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid number");
			} else {
				Debug::Log("Reading [%s] %s as boolean ! \n", pSection, pKey);
				value = (int)bufferb;
				return true;
			}
		}

		return false;
	}

	template <>
	inline bool read<unsigned short>(unsigned short& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		int buffer;
		if (parser.ReadInteger(pSection, pKey, &buffer))
		{
			buffer = abs(buffer);
			value = static_cast<unsigned short>(buffer);
			return true;
		}
		return false;
	}

	template <>
	inline bool read<short>(short& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		int buffer;
		if (parser.ReadInteger(pSection, pKey, &buffer))
		{
			value = static_cast<short>(buffer);
			return true;
		}
		return false;
	}

	template <>
	inline bool read<BYTE>(BYTE& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		int buffer;
		if (parser.ReadInteger(pSection, pKey, &buffer))
		{
			buffer = abs(buffer);
			if (buffer <= 255 && buffer >= 0)
			{
				value = static_cast<BYTE>(buffer); // shut up shut up shut up C4244
				return true;
			}
			else
			{
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid number between 0 and 255 inclusive.");
			}
		}
		else if (!parser.empty())
		{
			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid number");
		}
		return false;
	}

	template <>
	inline bool read<float>(float& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		float buffer;
		if (parser.ReadFloat(pSection, pKey, &buffer))
		{
			value = (buffer);
			return true;
		}
		else if (!parser.empty())
		{
			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid floating point number");
		}
		return false;
	}

	template <>
	inline bool read<double>(double& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		double buffer;
		if (parser.ReadDouble(pSection, pKey, &buffer))
		{
			value = buffer;
			return true;
		}
		else if (!parser.empty())
		{
			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid floating point number");
		}
		return false;
	}

	template <>
	inline bool read<CellStruct>(CellStruct& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		Point2D pBuffer {};
		if (parser.Read2Integers(pSection, pKey, (int*)&pBuffer))
		{
			value.X = (short)pBuffer.X;
			value.Y = (short)pBuffer.Y;
			return true;
		}
		return false;
	}

	template <>
	inline bool read<Point2D>(Point2D& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.Read2Integers(pSection, pKey, (int*)&value))
		{
			return true;
		}
		return false;
	}

	template <>
	inline bool read<Point3D>(Point3D& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.Read3Integers(pSection, pKey, (int*)&value))
		{
			return true;
		}
		return false;
	}

	template <>
	inline bool read<RectangleStruct>(RectangleStruct& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.Read4Integers(pSection, pKey, (int*)&value))
		{
			return true;
		}
		return false;
	}

	template <>
	inline bool read<Point2DBYTE>(Point2DBYTE& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.Read2Bytes(pSection, pKey, (BYTE*)&value))
		{
			return true;
		}
		return false;
	}

	template <>
	inline bool read<CoordStruct>(CoordStruct& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.Read3Integers(pSection, pKey, (int*)&value))
		{
			return true;
		}
		return false;
	}

	template <>
	inline bool read<Vector3D<BYTE>>(Vector3D<BYTE>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		Vector3D<BYTE> buffer {};
		if (parser.Read3Bytes(pSection, pKey, reinterpret_cast<BYTE*>(&buffer)))
		{
			value = buffer;
			return true;
		}
		else if (!parser.empty())
		{
			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid 3 BYTE Value");
		}
		return false;
	}

	template <>
	inline bool read<Vector2D<double>>(Vector2D<double>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.Read2Double(pSection, pKey, (double*)&value))
		{
			return true;
		}

		return false;
	}

	template <>
	inline bool read<CSFText>(CSFText& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			value = parser.value();
			return true;
		}
		return false;
	}

	template <>
	inline bool read<Theater_SHPStruct*>(Theater_SHPStruct*& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			auto const pValue = parser.value();
			GeneralUtils::ApplyTheaterSuffixToString(pValue);

			std::string Result = pValue;

			if (Result.find(".shp") == std::string::npos)
			{
				Result += ".shp";
			}

			if (auto const pImage = FileSystem::LoadSHPFile(Result.c_str()))
			{
				value = reinterpret_cast<Theater_SHPStruct*>(pImage);
				return true;
			}
			else
			{
				Debug::Log("Failed to find file %s referenced by [%s]%s=%s\n", Result.c_str(), pSection, pKey, pValue);
			}
		}
		return false;
	}

	template <>
	inline bool read<SHPStruct*>(SHPStruct*& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			auto const pValue = _strlwr(parser.value());

			if (GeneralUtils::IsValidString(pValue))
			{
				std::string flag = pValue;

				if (flag.find(".shp") == std::string::npos)
				{
					flag += ".shp";
				}

				if (flag.find("~") != std::string::npos)
				{
					flag.replace(flag.begin() + flag.find("~"), flag.end(), Theater::GetTheater(ScenarioClass::Instance->Theater).Letter);
				}

				if (auto const pImage = FileSystem::LoadSHPFile(flag.c_str()))
				{
					value = pImage;
					return true;
				}
			}
		}

		return false;
	}

	template <>
	inline bool read<SelfHealGainType>(SelfHealGainType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			if (IS_SAME_STR_(parser.value(), GameStrings::NoneStrb()))
			{
				value = SelfHealGainType::None;
			}
			else if (IS_SAME_STR_(parser.value(), GameStrings::Infantry()))
			{
				value = SelfHealGainType::Infantry;
			}
			else if (IS_SAME_STR_(parser.value(), GameStrings::Units()))
			{
				value = SelfHealGainType::Units;
			}
			else
			{
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a self heal gain type");
				return false;
			}
			return true;
		}
		return false;
	}

	template <>
	inline bool read<SlaveReturnTo>(SlaveReturnTo& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			if (IS_SAME_STR_(parser.value(), GameStrings::Suicide()) || IS_SAME_STR_(parser.value(), "kill") || IS_SAME_STR_(parser.value(), "explode"))
			{
				value = SlaveReturnTo::Suicide;
				return true;
			}
			else if (IS_SAME_STR_(parser.value(), "master"))
			{
				value = SlaveReturnTo::Master;
				return true;
			}
			else if (IS_SAME_STR_(parser.value(), "killer"))
			{
				value = SlaveReturnTo::Killer;
				return true;
			}
			else if (IS_SAME_STR_(parser.value(), GameStrings::Neutral()))
			{
				value = SlaveReturnTo::Neutral;
				return true;
			}
			else if (IS_SAME_STR_(parser.value(), GameStrings::Civilian()))
			{
				value = SlaveReturnTo::Civilian;
				return true;
			}
			else if (IS_SAME_STR_(parser.value(), GameStrings::Special()))
			{
				value = SlaveReturnTo::Special;
				return true;
			}
			else if (IS_SAME_STR_(parser.value(), "random"))
			{
				value = SlaveReturnTo::Random;
				return true;
			}
			else
			{
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a free-slave option, default killer");
			}		
		}
		return false;
	}

	template <>
	inline bool read<KillMethod>(KillMethod& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			auto const pValue = parser.value();
			if (IS_SAME_STR_(pValue, "sell"))
			{
				value = KillMethod::Sell;
				return true;
			}
			else if (IS_SAME_STR_(pValue, "vanish"))
			{
				value = KillMethod::Vanish;
				return true;
			}
			else if (IS_SAME_STR_(pValue, "kill") || IS_SAME_STR_(pValue, "explode"))
			{
				value = KillMethod::Explode;
				return true;
			}
			else if (IS_SAME_STR_(pValue, "random"))
			{
				value = KillMethod::Random;
				return true;
			}
			else if(!parser.empty()) //parser not empty but it doesnt match any condition
			{
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a kill method, default disabled");
			}
		}
		return false;
	}

	template <>
	inline bool read<IronCurtainFlag>(IronCurtainFlag& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			auto str = parser.value();

			if (IS_SAME_STR_(str, "invulnerable"))
			{
				value = IronCurtainFlag::Invulnerable;
				return true;
			}
			else if (IS_SAME_STR_(str, "ignore"))
			{
				value = IronCurtainFlag::Ignore;
				return true;
			}
			else if (IS_SAME_STR_(str, "random"))
			{
				value = IronCurtainFlag::Random;
				return true;
			}
			else if (IS_SAME_STR_(str, DEFAULT_STR) || IS_SAME_STR_(str, DEFAULT_STR2))
			{
				value = IronCurtainFlag::Default;
				return true;
			}
			else if (IS_SAME_STR_(str, "kill"))
			{
				value = IronCurtainFlag::Kill;
				return true;
			}
			else
			{
				Debug::INIParseFailed(pSection, pKey, str, "IronCurtainFlag can be either kill, invulnerable, ignore or random");
			}
		}

		return false;
	}

	/*
		template <>
		inline bool read<ShapeHandlerEnumerator*>(ShapeHandlerEnumerator*& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
		{
			if (parser.ReadString(pSection, pKey))
			{
				auto const pValue = parser.value();

				if (CCINIClass::IsBlank(pValue))
				{
					value = nullptr;
					return false;
				}

				if (auto const parsed = ShapeHandlerEnumerator::FindOrAllocate(pValue))
				{
					parsed->FetchSHP();
					value = parsed;
					return true;
				}
				else
				{
					Debug::INIParseFailed(pSection, pKey, pValue);
				}
			}
			return false;
		}
	*/

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
				Parser<int>::Parse(pFrame, &value.Frame);
			}
			if (auto const pCount = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				Parser<int>::Parse(pCount, &value.Count);
			}
			if (auto const pInterval = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				Parser<int>::Parse(pInterval, &value.Interval);
			}
			if (auto const pFrame = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				Parser<int>::Parse(pFrame, &value.MiniFrame);
			}
			if (auto const pCount = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				Parser<int>::Parse(pCount, &value.MiniCount);
			}
			if (auto const pHotX = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				MouseCursorHotSpotX::Parse(pHotX, &value.HotX);
			}
			if (auto const pHotY = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				MouseCursorHotSpotY::Parse(pHotY, &value.HotY);
			}

			ret = true;
		}

		char pFlagName[32];
		_snprintf_s(pFlagName, 31, "%s.Frame", pKey);
		ret |= read(value.Frame, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 31, "%s.Count", pKey);
		ret |= read(value.Count, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 31, "%s.Interval", pKey);
		ret |= read(value.Interval, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 31, "%s.MiniFrame", pKey);
		ret |= read(value.MiniFrame, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 31, "%s.MiniCount", pKey);
		ret |= read(value.MiniCount, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 31, "%s.HotSpot", pKey);
		if (parser.ReadString(pSection, pFlagName))
		{
			auto const pValue = parser.value();
			char* context = nullptr;
			auto const pHotX = strtok_s(pValue, Phobos::readDelims, &context);
			MouseCursorHotSpotX::Parse(pHotX, &value.HotX);

			if (auto const pHotY = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				MouseCursorHotSpotY::Parse(pHotY, &value.HotY);
			}

			ret = true;
		}

		return ret;
	}

	template<>
	inline bool read<WeaponStruct>(WeaponStruct& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		auto ret = false;

		char Buffer[0x100];
		constexpr auto sizebuffer = sizeof(Buffer);
		ret |= read(value.WeaponType, parser, pSection, pKey, allocate);
		_snprintf_s(Buffer, sizebuffer, "%s.FLH", pKey);
		ret |= read(value.FLH, parser, pSection, Buffer);
		_snprintf_s(Buffer, sizebuffer, "%s.BarrelLength", pKey);
		ret |= read(value.BarrelLength, parser, pSection, Buffer);
		_snprintf_s(Buffer, sizebuffer, "%s.BarrelThickness", pKey);
		ret |= read(value.BarrelThickness, parser, pSection, Buffer);
		_snprintf_s(Buffer, sizebuffer, "%s.TurretLocked", pKey);
		ret |= read(value.TurretLocked, parser, pSection, Buffer);

		return ret;
	}

	template <>
	inline bool read<RocketStruct>(RocketStruct& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		auto ret = false;

		char pFlagName[0x40];
		_snprintf_s(pFlagName, 0x3F, "%s.PauseFrames", pKey);
		ret |= read(value.PauseFrames, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 0x3F, "%s.TiltFrames", pKey);
		ret |= read(value.TiltFrames, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 0x3F, "%s.PitchInitial", pKey);
		ret |= read(value.PitchInitial, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 0x3F, "%s.PitchFinal", pKey);
		ret |= read(value.PitchFinal, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 0x3F, "%s.TurnRate", pKey);
		ret |= read(value.TurnRate, parser, pSection, pFlagName);

		// sic! integer read like a float.
		_snprintf_s(pFlagName, 0x3F, "%s.RaiseRate", pKey);
		float buffer;
		if (read(buffer, parser, pSection, pFlagName))
		{
			value.RaiseRate = Game::F2I(buffer);
			ret = true;
		}

		_snprintf_s(pFlagName, 0x3F, "%s.Acceleration", pKey);
		ret |= read(value.Acceleration, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 0x3F, "%s.Altitude", pKey);
		ret |= read(value.Altitude, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 0x3F, "%s.Damage", pKey);
		ret |= read(value.Damage, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 0x3F, "%s.EliteDamage", pKey);
		ret |= read(value.EliteDamage, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 0x3F, "%s.BodyLength", pKey);
		ret |= read(value.BodyLength, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 0x3F, "%s.LazyCurve", pKey);
		ret |= read(value.LazyCurve, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 0x3F, "%s.Type", pKey);
		ret |= read(value.Type, parser, pSection, pFlagName);

		return ret;
	}

	template <>
	inline bool read<Leptons>(Leptons& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		double buffer;
		if (parser.ReadDouble(pSection, pKey, &buffer))
		{
			value = Leptons(buffer);
			return true;
		}
		else if (!parser.empty())
		{
			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid floating point number");
		}
		return false;
	}

	template <>
	inline bool read<OwnerHouseKind>(OwnerHouseKind& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			if (IS_SAME_STR_(parser.value(), "default"))
			{
				value = OwnerHouseKind::Default;
			}
			else if (IS_SAME_STR_(parser.value(), "invoker"))
			{
				value = OwnerHouseKind::Invoker;
			}
			else if (IS_SAME_STR_(parser.value(), "killer"))
			{
				value = OwnerHouseKind::Killer;
			}
			else if (IS_SAME_STR_(parser.value(), "victim"))
			{
				value = OwnerHouseKind::Victim;
			}
			else if (IS_SAME_STR_(parser.value(), GameStrings::Civilian()))
			{
				value = OwnerHouseKind::Civilian;
			}
			else if (IS_SAME_STR_(parser.value(), GameStrings::Special()))
			{
				value = OwnerHouseKind::Special;
			}
			else if (IS_SAME_STR_(parser.value(), GameStrings::Neutral()))
			{
				value = OwnerHouseKind::Neutral;
			}
			else if (IS_SAME_STR_(parser.value(), "random"))
			{
				value = OwnerHouseKind::Random;
			}
			else
			{
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a owner house kind");
				return false;
			}
			return true;
		}
		return false;
	}

	template <>
	inline bool read<Mission>(Mission& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			auto const mission = MissionControlClass::FindIndexById(parser.value());
			if (mission != Mission::None)
			{
				value = mission;
				return true;
			}
			else if (!parser.empty())
			{
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Invalid Mission name");
			}
		}
		return false;
	}

	template <>
	inline bool read<SuperWeaponAITargetingMode>(SuperWeaponAITargetingMode& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			static const auto Modes = {
				GameStrings::NoneStrb(), "nuke", "lightningstorm", "psychicdominator", "paradrop",
				"geneticmutator", "forceshield", "notarget", "offensive", "stealth",
				"self", "base", "multimissile", "hunterseeker", "enemybase" };

			auto it = Modes.begin();
			for (auto i = 0u; i < Modes.size(); ++i)
			{
				if (IS_SAME_STR_(parser.value(), *it++))
				{
					value = static_cast<SuperWeaponAITargetingMode>(i);
					return true;
				}
			}

			if (CRT::strcmpi(parser.value(), GameStrings::NoneStrb())){
				value = SuperWeaponAITargetingMode::NoTarget;
				return true;
			}

			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a targeting mode");
		}
		return false;
	}

	template <>
	inline bool read<AffectedTarget>(AffectedTarget& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			auto parsed = AffectedTarget::None;

			auto str = parser.value();
			char* context = nullptr;
			for (auto cur = strtok_s(str, Phobos::readDelims, &context); cur; cur = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				if (IS_SAME_STR_(cur, "land"))
				{
					parsed |= AffectedTarget::Land;
				}
				else if (IS_SAME_STR_(cur, "water"))
				{
					parsed |= AffectedTarget::Water;
				}
				else if (IS_SAME_STR_(cur, "empty"))
				{
					parsed |= AffectedTarget::NoContent;
				}
				else if (IS_SAME_STR_(cur, "infantry"))
				{
					parsed |= AffectedTarget::Infantry;
				}
				else if (IS_SAME_STR_(cur, "units"))
				{
					parsed |= AffectedTarget::Unit;
				}
				else if (IS_SAME_STR_(cur, "buildings"))
				{
					parsed |= AffectedTarget::Building;
				}
				else if (IS_SAME_STR_(cur, "aircraft"))
				{
					parsed |= AffectedTarget::Aircraft;
				}
				else if (IS_SAME_STR_(cur, "all"))
				{
					parsed |= AffectedTarget::All;
				}
				else if (CRT::strcmpi(cur, NONE_STR2))
				{
					Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a affected target");
					return false;
				}
			}
			value = parsed;
			return true;
		}
		return false;
	}

	template <>
	inline bool read<LandType>(LandType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			auto parsed = LandType::Clear;

			auto str = parser.value();
			char* context = nullptr;
			for (auto cur = strtok_s(str, Phobos::readDelims, &context); cur; cur = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				if (IS_SAME_STR_(cur, "Road"))
				{
					parsed = LandType::Road;
				}
				else if (IS_SAME_STR_(cur, "Water"))
				{
					parsed = LandType::Water;
				}
				else if (IS_SAME_STR_(cur, "Rock"))
				{
					parsed = LandType::Rock;
				}
				else if (IS_SAME_STR_(cur, "Wall"))
				{
					parsed = LandType::Wall;
				}
				else if (IS_SAME_STR_(cur, "Tiberium"))
				{
					parsed = LandType::Tiberium;
				}
				else if (IS_SAME_STR_(cur, "Beach"))
				{
					parsed = LandType::Beach;
				}
				else if (IS_SAME_STR_(cur, "Rough"))
				{
					parsed = LandType::Rough;
				}
				else if (IS_SAME_STR_(cur, "Ice"))
				{
					parsed = LandType::Ice;
				}
				else if (IS_SAME_STR_(cur, "Railroad"))
				{
					parsed = LandType::Railroad;
				}
				else if (IS_SAME_STR_(cur, "Tunnel"))
				{
					parsed = LandType::Tunnel;
				}
				else if (IS_SAME_STR_(cur, "Weeds"))
				{
					parsed = LandType::Weeds;
				}
				else if (CRT::strcmpi(cur, NONE_STR2))
				{
					Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a land type");
					return false;
				}
			}
			value = parsed;
			return true;
		}
		return false;
	}

	template <>
	inline bool read<AffectedHouse>(AffectedHouse& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			auto parsed = AffectedHouse::None;

			auto str = parser.value();
			char* context = nullptr;
			for (auto cur = strtok_s(str, Phobos::readDelims, &context); cur; cur = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				if (IS_SAME_STR_(cur, "owner") || IS_SAME_STR_(cur, "self"))
				{
					parsed |= AffectedHouse::Owner;
				}
				else if (IS_SAME_STR_(cur, "allies") || IS_SAME_STR_(cur, "ally"))
				{
					parsed |= AffectedHouse::Allies;
				}
				else if (IS_SAME_STR_(cur, "enemies") || IS_SAME_STR_(cur, "enemy"))
				{
					parsed |= AffectedHouse::Enemies;
				}
				else if (IS_SAME_STR_(cur, "team"))
				{
					parsed |= AffectedHouse::Team;
				}
				else if (IS_SAME_STR_(cur, "others"))
				{
					parsed |= AffectedHouse::NotOwner;
				}
				else if (IS_SAME_STR_(cur, "all"))
				{
					parsed |= AffectedHouse::All;
				}
				else if (CRT::strcmpi(cur, NONE_STR2))
				{
					Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a affected house");
					return false;
				}
			}
			value = parsed;
			return true;
		}
		return false;
	}

	template <>
	inline bool read<AttachedAnimFlag>(AttachedAnimFlag& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			auto parsed = AttachedAnimFlag::None;

			auto str = parser.value();

			if (IS_SAME_STR_(str, "hides"))
			{
				parsed = AttachedAnimFlag::Hides;
			}
			else if (IS_SAME_STR_(str, "temporal"))
			{
				parsed = AttachedAnimFlag::Temporal;
			}
			else if (IS_SAME_STR_(str, "paused"))
			{
				parsed = AttachedAnimFlag::Paused;
			}
			else if (IS_SAME_STR_(str, "pausedtemporal"))
			{
				parsed = AttachedAnimFlag::PausedTemporal;
			}
			else if (CRT::strcmpi(str, NONE_STR2))
			{
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a AttachedAnimFlag");
				return false;
			}
			value = parsed;
			return true;
		}
		return false;
	}

	template <>
	inline bool read<AreaFireTarget>(AreaFireTarget& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			if (IS_SAME_STR_(parser.value(), "base"))
			{
				value = AreaFireTarget::Base;
			}
			else if (IS_SAME_STR_(parser.value(), "self"))
			{
				value = AreaFireTarget::Self;
			}
			else if (IS_SAME_STR_(parser.value(), "random"))
			{
				value = AreaFireTarget::Random;
			}
			else
			{
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected an area fire target");
				return false;
			}
			return true;
		}
		return false;
	}

	template <>
	inline bool read<TextAlign>(TextAlign& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			auto parsed = TextAlign::None;
			auto str = parser.value();
			if (IS_SAME_STR_(str, "left"))
			{
				parsed = TextAlign::Left;
			}
			else if (IS_SAME_STR_(str, "center"))
			{
				parsed = TextAlign::Center;
			}
			else if (IS_SAME_STR_(str, "centre"))
			{
				parsed = TextAlign::Center;
			}
			else if (IS_SAME_STR_(str, "right"))
			{
				parsed = TextAlign::Right;
			}
			else if (CRT::strcmpi(str, NONE_STR2))
			{
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Text Alignment can be either Left, Center/Centre or Right");
				return false;
			}
			if (parsed != TextAlign::None)
				value = parsed;
			return true;
		}
		return false;
	}

	template <>
	inline bool read<Layer>(Layer& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			Layer parsed = Layer::None;
			auto const str = parser.value();

			if (str != nullptr
				&& CRT::strlen(str) != 0)
			{
				if (IS_SAME_STR_(str, "underground"))
				{
					parsed = Layer::Underground;
				}
				else if (IS_SAME_STR_(str, "surface"))
				{
					parsed = Layer::Surface;
				}
				else if (IS_SAME_STR_(str, "ground"))
				{
					parsed = Layer::Ground;
				}
				else if (IS_SAME_STR_(str, "air"))
				{
					parsed = Layer::Air;
				}
				else if (IS_SAME_STR_(str, "top"))
				{
					parsed = Layer::Top;
				}
				else
				{
					Debug::INIParseFailed(pSection, pKey, parser.value(), "Expect a Valid Layer !");
				}
			}

			value = parsed;
			return true;
		}

		return false;
	}

	template <>
	inline bool read<AbstractType>(AbstractType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			AbstractType parsed = AbstractType::None;
			auto const str = parser.value();

			if (str != nullptr
				&& CRT::strlen(str) != 0)
			{
				if (IS_SAME_STR_(str, "Infantry"))
				{
					parsed = AbstractType::Infantry;
				}
				else if (IS_SAME_STR_(str, "Unit"))
				{
					parsed = AbstractType::Unit;
				}
				else if (IS_SAME_STR_(str, "Aircraft"))
				{
					parsed = AbstractType::Aircraft;
				}
				else if (IS_SAME_STR_(str, "Building"))
				{
					parsed = AbstractType::Building;
				}
				else
				{
					Debug::INIParseFailed(pSection, pKey, parser.value(), "Expect a Valid AbstractType !");
				}

				value = parsed;
				return true;
			}
		}

		return false;
	}

	template <>
	inline bool read<Locomotors>(Locomotors& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			auto const str = parser.value();

			if (IS_SAME_STR_(str, "Drive") || IS_SAME_STR_(str, "{4A582741-9839-11d1-B709-00A024DDAFD1}"))
			{
				value = Locomotors::Drive;
			}
			else if (IS_SAME_STR_(str, "Jumpjet") || IS_SAME_STR_(str, "{92612C46-F71F-11d1-AC9F-006008055BB5}"))
			{
				value = Locomotors::Jumpjet;
			}
			else if (IS_SAME_STR_(str, "Hover") || IS_SAME_STR_(str, "{4A582742-9839-11d1-B709-00A024DDAFD1}"))
			{
				value = Locomotors::Hover;
			}
			else if (((str, "Rocket") == 0) || IS_SAME_STR_(str, "{B7B49766-E576-11d3-9BD9-00104B972FE8}"))
			{
				value = Locomotors::Rocket;
			}
			else if (IS_SAME_STR_(str, "Tunnel") || IS_SAME_STR_(str, "{4A582743-9839-11d1-B709-00A024DDAFD1}"))
			{
				value = Locomotors::Tunnel;
			}
			else if (IS_SAME_STR_(str, "Walk") || IS_SAME_STR_(str, "{4A582744-9839-11d1-B709-00A024DDAFD1}"))
			{
				value = Locomotors::Walk;
			}
			else if (IS_SAME_STR_(str, "Droppod") || IS_SAME_STR_(str, "{4A582745-9839-11d1-B709-00A024DDAFD1}"))
			{
				value = Locomotors::Droppod;
			}
			else if (IS_SAME_STR_(str, "Fly") || IS_SAME_STR_(str, "{4A582746-9839-11d1-B709-00A024DDAFD1}"))
			{
				value = Locomotors::Fly;
			}
			else if (IS_SAME_STR_(str, "Teleport") || IS_SAME_STR_(str, "{4A582747-9839-11d1-B709-00A024DDAFD1}"))
			{
				value = Locomotors::Teleport;
			}
			else if (IS_SAME_STR_(str, "Mech") || IS_SAME_STR_(str, "{55D141B8-DB94-11d1-AC98-006008055BB5}"))
			{
				value = Locomotors::Mech;
			}
			else if (IS_SAME_STR_(str, "Ship") || IS_SAME_STR_(str, "{2BEA74E1-7CCA-11d3-BE14-00104B62A16C}"))
			{
				value = Locomotors::Ship;
			}
			else
			{
				return false;
			}

			return true;
		}
		return false;
	}

	template <>
	inline bool read(TileType& value, INI_EX& parser, const char* pSection, const char* pKey, bool bAllocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			auto parsed = TileType::Unk;
			auto cur = parser.value();

			if (IS_SAME_STR_(cur, "Tunnel"))
			{
				parsed = TileType::Tunnel;
			}
			else if (IS_SAME_STR_(cur, "Water"))
			{
				parsed = TileType::Water;
			}
			else if (IS_SAME_STR_(cur, "Ramp"))
			{
				parsed = TileType::Ramp;
			}
			else if (IS_SAME_STR_(cur, "Blank"))
			{
				parsed = TileType::Blank;
			}
			else if (IS_SAME_STR_(cur, "Shore"))
			{
				parsed = TileType::Shore;
			}
			else if (IS_SAME_STR_(cur, "Wet"))
			{
				parsed = TileType::Wet;
			}
			else if (IS_SAME_STR_(cur, "MiscPave"))
			{
				parsed = TileType::MiscPave;
			}
			else if (IS_SAME_STR_(cur, "Pave"))
			{
				parsed = TileType::Pave;
			}
			else if (IS_SAME_STR_(cur, "DirtRoad"))
			{
				parsed = TileType::DirtRoad;
			}
			else if (IS_SAME_STR_(cur, "PavedRoad"))
			{
				parsed = TileType::PavedRoad;
			}
			else if (IS_SAME_STR_(cur, "PavedRoadEnd"))
			{
				parsed = TileType::PavedRoadEnd;
			}
			else if (IS_SAME_STR_(cur, "PavedRoadSlope"))
			{
				parsed = TileType::PavedRoadSlope;
			}
			else if (IS_SAME_STR_(cur, "Median"))
			{
				parsed = TileType::Median;
			}
			else if (IS_SAME_STR_(cur, "Bridge"))
			{
				parsed = TileType::Bridge;
			}
			else if (IS_SAME_STR_(cur, "WoodBridge"))
			{
				parsed = TileType::WoodBridge;
			}
			else if (IS_SAME_STR_(cur, "ClearToSandLAT"))
			{
				parsed = TileType::ClearToSandLAT;
			}
			else if (IS_SAME_STR_(cur, "Green"))
			{
				parsed = TileType::Green;
			}
			else if (IS_SAME_STR_(cur, "NotWater"))
			{
				parsed = TileType::NotWater;
			}
			else if (IS_SAME_STR_(cur, "DestroyableCliff"))
			{
				parsed = TileType::DestroyableCliff;
			}
			else if (!INIClass::IsBlank(cur) && !bAllocate)
			{
				Debug::INIParseFailed(pSection, pKey, cur, nullptr);
				return false;
			}

			if (parsed != TileType::Unk)
			{
				value = parsed;
				return true;
			}
		}
		return false;
	}

	template <>
	inline bool read<TypeList<int>>(TypeList<int>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			value.Clear();
			char* context = nullptr;
			for (auto pCur = strtok_s(parser.value(), Phobos::readDelims, &context); pCur; pCur = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				int buffer = 0;
				if (Parser<int>::Parse(pCur, &buffer))
				{
					value.AddItem(buffer);
					return true;
				}
				else if (!INIClass::IsBlank(pCur))
				{
					Debug::INIParseFailed(pSection, pKey, pCur, nullptr);
				}
			}
		}
		return false;
	}

	template <typename T>
	void parse_values(std::vector<T>& vector, INI_EX& parser, const char* pSection, const char* pKey, bool bAllocate = false)
	{
		char* context = nullptr;
		for (auto pCur = strtok_s(parser.value(), Phobos::readDelims, &context); pCur; pCur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			auto buffer = T();
			if (Parser<T>::Parse(pCur, &buffer))
				vector.push_back(buffer);
			else if (!INIClass::IsBlank(pCur))
				Debug::INIParseFailed(pSection, pKey, pCur, nullptr);
		}
	}

	template <typename T>
	void ReadVectorsAlloc(std::vector<T>& vector, INI_EX& parser, const char* pSection, const char* pKey, bool bAllocate = false)
	{
		static_assert(std::is_pointer<T>::value, "Pointer Required !");

		if (parser.ReadString(pSection, pKey))
		{
			vector.clear();
			detail::parse_Alloc_values(vector, parser, pSection, pKey, bAllocate);
		}
	}

	template <typename T>
	void parse_Alloc_values(std::vector<T>& vector, INI_EX& parser, const char* pSection, const char* pKey, bool bAllocate = false)
	{
		static_assert(std::is_pointer<T>::value, "Pointer Required !");

		using base_type = std::remove_pointer_t<T>;

		char* context = nullptr;
		for (auto pCur = strtok_s(parser.value(), Phobos::readDelims, &context); pCur; pCur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			auto buffer = base_type::FindOrAllocate(pCur);
			bool parseSucceeded = buffer != nullptr;

			if (parseSucceeded)
				vector.push_back(buffer);
			else if (!INIClass::IsBlank(pCur))
				Debug::INIParseFailed(pSection, pKey, pCur, nullptr);
		}
	}

	template <>
	inline void parse_values(std::vector<LandType>& vector, INI_EX& parser, const char* pSection, const char* pKey, bool bAllocate)
	{
		char* context = nullptr;
		for (auto cur = strtok_s(parser.value(), Phobos::readDelims, &context); cur; cur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			auto parsed = LandType::Clear;
			if (IS_SAME_STR_(cur, "Road"))
			{
				parsed = LandType::Road;
			}
			else if (IS_SAME_STR_(cur, "Water"))
			{
				parsed = LandType::Water;
			}
			else if (IS_SAME_STR_(cur, "Rock"))
			{
				parsed = LandType::Rock;
			}
			else if (IS_SAME_STR_(cur, "Wall"))
			{
				parsed = LandType::Wall;
			}
			else if (IS_SAME_STR_(cur, "Tiberium"))
			{
				parsed = LandType::Tiberium;
			}
			else if (IS_SAME_STR_(cur, "Beach"))
			{
				parsed = LandType::Beach;
			}
			else if (IS_SAME_STR_(cur, "Rough"))
			{
				parsed = LandType::Rough;
			}
			else if (IS_SAME_STR_(cur, "Ice"))
			{
				parsed = LandType::Ice;
			}
			else if (IS_SAME_STR_(cur, "Railroad"))
			{
				parsed = LandType::Railroad;
			}
			else if (IS_SAME_STR_(cur, "Tunnel"))
			{
				parsed = LandType::Tunnel;
			}
			else if (IS_SAME_STR_(cur, "Weeds"))
			{
				parsed = LandType::Weeds;
			}
			else if (!INIClass::IsBlank(cur) && !bAllocate)
				Debug::INIParseFailed(pSection, pKey, cur, nullptr);

			vector.push_back(parsed);
		}
	}

	template <>
	inline void parse_values(std::vector<TileType>& vector, INI_EX& parser, const char* pSection, const char* pKey, bool bAllocate)
	{
		char* context = nullptr;
		for (auto cur = strtok_s(parser.value(), Phobos::readDelims, &context); cur; cur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			auto parsed = TileType::Unk;

			if (IS_SAME_STR_(cur, "Tunnel"))
			{
				parsed = TileType::Tunnel;
			}
			else if (IS_SAME_STR_(cur, "Water"))
			{
				parsed = TileType::Water;
			}
			else if (IS_SAME_STR_(cur, "Ramp"))
			{
				parsed = TileType::Ramp;
			}
			else if (IS_SAME_STR_(cur, "Blank"))
			{
				parsed = TileType::Blank;
			}
			else if (IS_SAME_STR_(cur, "Shore"))
			{
				parsed = TileType::Shore;
			}
			else if (IS_SAME_STR_(cur, "Wet"))
			{
				parsed = TileType::Wet;
			}
			else if (IS_SAME_STR_(cur, "MiscPave"))
			{
				parsed = TileType::MiscPave;
			}
			else if (IS_SAME_STR_(cur, "Pave"))
			{
				parsed = TileType::Pave;
			}
			else if (IS_SAME_STR_(cur, "DirtRoad"))
			{
				parsed = TileType::DirtRoad;
			}
			else if (IS_SAME_STR_(cur, "PavedRoad"))
			{
				parsed = TileType::PavedRoad;
			}
			else if (IS_SAME_STR_(cur, "PavedRoadEnd"))
			{
				parsed = TileType::PavedRoadEnd;
			}
			else if (IS_SAME_STR_(cur, "PavedRoadSlope"))
			{
				parsed = TileType::PavedRoadSlope;
			}
			else if (IS_SAME_STR_(cur, "Median"))
			{
				parsed = TileType::Median;
			}
			else if (IS_SAME_STR_(cur, "Bridge"))
			{
				parsed = TileType::Bridge;
			}
			else if (IS_SAME_STR_(cur, "WoodBridge"))
			{
				parsed = TileType::WoodBridge;
			}
			else if (IS_SAME_STR_(cur, "ClearToSandLAT"))
			{
				parsed = TileType::ClearToSandLAT;
			}
			else if (IS_SAME_STR_(cur, "Green"))
			{
				parsed = TileType::Green;
			}
			else if (IS_SAME_STR_(cur, "NotWater"))
			{
				parsed = TileType::NotWater;
			}
			else if (IS_SAME_STR_(cur, "DestroyableCliff"))
			{
				parsed = TileType::DestroyableCliff;
			}
			else if (!INIClass::IsBlank(cur) && !bAllocate)
				Debug::INIParseFailed(pSection, pKey, cur, nullptr);

			vector.push_back(parsed);
		}
	}

	template <typename Lookuper, typename T>
	void parse_indexes(std::vector<T>& vector, INI_EX& parser, const char* pSection, const char* pKey)
	{
		char* context = nullptr;
		for (auto pCur = strtok_s(parser.value(), Phobos::readDelims, &context); pCur; pCur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			int idx = Lookuper::FindIndexById(pCur);
			if (idx != -1)
			{
				vector.push_back(idx);
			}
			else if (!INIClass::IsBlank(pCur))
			{
				Debug::INIParseFailed(pSection, pKey, pCur);
			}
		}
	}
}


// Valueable

template <typename T>
void __declspec(noinline) Valueable<T>::Read(INI_EX& parser, const char* pSection, const char* pKey, bool Allocate)
{
	detail::read(this->Value, parser, pSection, pKey, Allocate);
}

template <typename T>
bool Valueable<T>::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Savegame::ReadPhobosStream(Stm, this->Value, RegisterForChange);
}

template <typename T>
bool Valueable<T>::Save(PhobosStreamWriter& Stm) const
{
	return Savegame::WritePhobosStream(Stm, this->Value);
}


// ValueableIdx

template <typename Lookuper>
void __declspec(noinline) ValueableIdx<Lookuper>::Read(INI_EX& parser, const char* pSection, const char* pKey)
{
	if (parser.ReadString(pSection, pKey))
	{
		const char* val = parser.value();
		int idx = Lookuper::FindIndexById(val);
		if (idx != -1 || INIClass::IsBlank(val))
		{
			this->Value = idx;
		}
		else
		{
			Debug::INIParseFailed(pSection, pKey, val);
		}
	}
}


// Nullable

template <typename T>
void __declspec(noinline) Nullable<T>::Read(INI_EX& parser, const char* pSection, const char* pKey, bool Allocate)
{
	if (detail::read(this->Value, parser, pSection, pKey, Allocate))
	{
		this->HasValue = true;
	}
}

template <typename T>
bool Nullable<T>::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->Reset();
	auto ret = Savegame::ReadPhobosStream(Stm, this->HasValue);
	if (ret && this->HasValue)
	{
		ret = Savegame::ReadPhobosStream(Stm, this->Value, RegisterForChange);
	}
	return ret;
}

template <typename T>
bool Nullable<T>::Save(PhobosStreamWriter& Stm) const
{
	auto ret = Savegame::WritePhobosStream(Stm, this->HasValue);
	if (this->HasValue)
	{
		ret = Savegame::WritePhobosStream(Stm, this->Value);
	}
	return ret;
}


// NullableIdx

template <typename Lookuper>
void __declspec(noinline) NullableIdx<Lookuper>::Read(INI_EX& parser, const char* pSection, const char* pKey)
{
	if (parser.ReadString(pSection, pKey))
	{
		const char* val = parser.value();
		int idx = Lookuper::FindIndexById(val);
		if (idx != -1 || INIClass::IsBlank(val))
		{
			this->Value = idx;
			this->HasValue = true;
		}
		else
		{
			Debug::INIParseFailed(pSection, pKey, val, nullptr);
		}
	}
}


// Promotable

template <typename T>
void __declspec(noinline) Promotable<T>::Read(INI_EX& parser, const char* const pSection, const char* const pBaseFlag, const char* const pSingleFlag)
{

	// read the common flag, with the trailing dot being stripped
	char flagName[0x40];
	auto const pSingleFormat = pSingleFlag ? pSingleFlag : pBaseFlag;
	auto res = _snprintf_s(flagName, _TRUNCATE, pSingleFormat, "");
	if (res > 0 && flagName[res - 1] == '.')
	{
		flagName[res - 1] = '\0';
	}

	T placeholder {};
	if (detail::read(placeholder, parser, pSection, flagName))
	{
		this->SetAll(placeholder);
	}

	// read specific flags
	_snprintf_s(flagName, _TRUNCATE, pBaseFlag, "Rookie");
	detail::read(this->Rookie, parser, pSection, flagName);

	_snprintf_s(flagName, _TRUNCATE, pBaseFlag, "Veteran");
	detail::read(this->Veteran, parser, pSection, flagName);

	_snprintf_s(flagName, _TRUNCATE, pBaseFlag, "Elite");
	detail::read(this->Elite, parser, pSection, flagName);
};

template <typename T>
bool Promotable<T>::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Savegame::ReadPhobosStream(Stm, this->Rookie, RegisterForChange)
		&& Savegame::ReadPhobosStream(Stm, this->Veteran, RegisterForChange)
		&& Savegame::ReadPhobosStream(Stm, this->Elite, RegisterForChange);
}

template <typename T>
bool Promotable<T>::Save(PhobosStreamWriter& Stm) const
{
	return Savegame::WritePhobosStream(Stm, this->Rookie)
		&& Savegame::WritePhobosStream(Stm, this->Veteran)
		&& Savegame::WritePhobosStream(Stm, this->Elite);
}


// ValueableVector

template <typename T>
void __declspec(noinline) ValueableVector<T>::Read(INI_EX& parser, const char* pSection, const char* pKey, bool bAllocate)
{
	if (parser.ReadString(pSection, pKey))
	{
		this->clear();
		detail::parse_values<T>(*this, parser, pSection, pKey, bAllocate);
	}
}

template <typename T>
bool ValueableVector<T>::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	size_t size = 0;
	if (Savegame::ReadPhobosStream(Stm, size, RegisterForChange))
	{

		if (!size)
			return true;

		this->clear();
		this->reserve(size);

		for (size_t i = 0; i < size; ++i)
		{
			value_type buffer = value_type();
			Savegame::ReadPhobosStream(Stm, buffer, false);
			this->push_back(std::move(buffer));

			if (RegisterForChange)
			{
				Swizzle swizzle(this->back());
			}
		}
		return true;
	}
	return false;
}

template <typename T>
bool ValueableVector<T>::Save(PhobosStreamWriter& Stm) const
{
	auto size = this->size();
	if (Savegame::WritePhobosStream(Stm, size))
	{
		for (auto const& item : *this)
		{
			if (!Savegame::WritePhobosStream(Stm, item))
			{
				return false;
			}
		}
		return true;
	}
	return false;
}

// NullableVector

template <typename T>
void __declspec(noinline) NullableVector<T>::Read(INI_EX& parser, const char* pSection, const char* pKey)
{
	if (parser.ReadString(pSection, pKey))
	{
		this->clear();

		auto const non_default = CRT::strcmpi(parser.value(), DEFAULT_STR2) != 0;
		this->hasValue = non_default;

		if (non_default)
		{
			detail::parse_values<T>(*this, parser, pSection, pKey);
		}
	}
}

template <typename T>
bool NullableVector<T>::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->clear();
	if (Savegame::ReadPhobosStream(Stm, this->hasValue, RegisterForChange))
	{
		return !this->hasValue || ValueableVector<T>::Load(Stm, RegisterForChange);
	}
	return false;
}

template <typename T>
bool NullableVector<T>::Save(PhobosStreamWriter& Stm) const
{
	if (Savegame::WritePhobosStream(Stm, this->hasValue))
	{
		return !this->hasValue || ValueableVector<T>::Save(Stm);
	}
	return false;
}


// ValueableIdxVector

template <typename Lookuper>
void __declspec(noinline) ValueableIdxVector<Lookuper>::Read(INI_EX& parser, const char* pSection, const char* pKey)
{
	if (parser.ReadString(pSection, pKey))
	{
		this->clear();
		detail::parse_indexes<Lookuper>(*this, parser, pSection, pKey);
	}
}


// NullableIdxVector

template <typename Lookuper>
void __declspec(noinline) NullableIdxVector<Lookuper>::Read(INI_EX& parser, const char* pSection, const char* pKey)
{
	if (parser.ReadString(pSection, pKey))
	{
		this->clear();

		auto const non_default = CRT::strcmpi(parser.value(), DEFAULT_STR2) != 0;
		this->hasValue = non_default;

		if (non_default)
		{
			detail::parse_indexes<Lookuper>(*this, parser, pSection, pKey);
		}
	}
}

// Damageable
template <typename T>
void __declspec(noinline) Damageable<T>::Read(INI_EX& parser, const char* const pSection, const char* const pBaseFlag, const char* const pSingleFlag)
{
	// read the common flag, with the trailing dot being stripped
	char flagName[0x40];
	auto const pSingleFormat = pSingleFlag ? pSingleFlag : pBaseFlag;
	auto res = _snprintf_s(flagName, _TRUNCATE, pSingleFormat, "");

	if (res > 0 && flagName[res - 1] == '.')
		flagName[res - 1] = '\0';

	this->BaseValue.Read(parser, pSection, flagName);

	_snprintf_s(flagName, _TRUNCATE, pBaseFlag, "ConditionYellow");
	this->ConditionYellow.Read(parser, pSection, flagName);

	_snprintf_s(flagName, _TRUNCATE, pBaseFlag, "ConditionRed");
	this->ConditionRed.Read(parser, pSection, flagName);
};

template <typename T>
bool Damageable<T>::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Savegame::ReadPhobosStream(Stm, this->BaseValue, RegisterForChange)
		&& Savegame::ReadPhobosStream(Stm, this->ConditionYellow, RegisterForChange)
		&& Savegame::ReadPhobosStream(Stm, this->ConditionRed, RegisterForChange);
}

template <typename T>
bool Damageable<T>::Save(PhobosStreamWriter& Stm) const
{
	return Savegame::WritePhobosStream(Stm, this->BaseValue)
		&& Savegame::WritePhobosStream(Stm, this->ConditionYellow)
		&& Savegame::WritePhobosStream(Stm, this->ConditionRed);
}

bool __declspec(noinline) HealthOnFireData::Read(INI_EX& parser, const char* pSection, const char* pKey)
{
	if (pSection)
	{
		if (parser.ReadString(pSection, pKey))
		{
			auto value = this;
			auto const buffer = parser.value();
			char* context = nullptr;

			if (auto const nRedOnFire = strtok_s(buffer, Phobos::readDelims, &context))
			{
				Parser<bool>::Parse(nRedOnFire, &value->RedOnFire);
			}

			if (auto const nGreenOnFire = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				Parser<bool>::Parse(nGreenOnFire, &value->GreenOnFire);
			}

			if (auto const nYellowOnFire = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				Parser<bool>::Parse(nYellowOnFire, &value->YellowOnFire);
			}

			return true;
		}
	}

	return false;
};

bool HealthOnFireData::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Savegame::ReadPhobosStream(Stm, this->RedOnFire, RegisterForChange)
		&& Savegame::ReadPhobosStream(Stm, this->GreenOnFire, RegisterForChange)
		&& Savegame::ReadPhobosStream(Stm, this->YellowOnFire, RegisterForChange);
}

bool HealthOnFireData::Save(PhobosStreamWriter& Stm) const
{
	return Savegame::WritePhobosStream(Stm, this->RedOnFire)
		&& Savegame::WritePhobosStream(Stm, this->GreenOnFire)
		&& Savegame::WritePhobosStream(Stm, this->YellowOnFire);
}

// DamageableVector

template <typename T>
void __declspec(noinline) DamageableVector<T>::Read(INI_EX& parser, const char* const pSection, const char* const pBaseFlag, const char* const pSingleFlag)
{
	// read the common flag, with the trailing dot being stripped
	char flagName[0x40];
	auto const pSingleFormat = pSingleFlag ? pSingleFlag : pBaseFlag;
	auto res = _snprintf_s(flagName, _TRUNCATE, pSingleFormat, "");

	if (res > 0 && flagName[res - 1] == '.')
	{
		flagName[res - 1] = '\0';
	}

	this->BaseValue.Read(parser, pSection, flagName);

	_snprintf_s(flagName, _TRUNCATE, pBaseFlag, "ConditionYellow");
	this->ConditionYellow.Read(parser, pSection, flagName);

	_snprintf_s(flagName, _TRUNCATE, pBaseFlag, "ConditionRed");
	this->ConditionRed.Read(parser, pSection, flagName);

	_snprintf_s(flagName, _TRUNCATE, pBaseFlag, "MaxValue");
	this->MaxValue.Read(parser, pSection, flagName);
};

template <typename T>
bool DamageableVector<T>::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Savegame::ReadPhobosStream(Stm, this->BaseValue, RegisterForChange)
		&& Savegame::ReadPhobosStream(Stm, this->ConditionYellow, RegisterForChange)
		&& Savegame::ReadPhobosStream(Stm, this->ConditionRed, RegisterForChange)
		&& Savegame::ReadPhobosStream(Stm, this->MaxValue, RegisterForChange);
}

template <typename T>
bool DamageableVector<T>::Save(PhobosStreamWriter& Stm) const
{
	return Savegame::WritePhobosStream(Stm, this->BaseValue)
		&& Savegame::WritePhobosStream(Stm, this->ConditionYellow)
		&& Savegame::WritePhobosStream(Stm, this->ConditionRed)
		&& Savegame::WritePhobosStream(Stm, this->MaxValue);
}

// PromotableVector

template <typename T>
void __declspec(noinline) PromotableVector<T>::Read(INI_EX& parser, const char* const pSection, const char* const pBaseFlag, const char* const pSingleFlag)
{
	// read the common flag, with the trailing dot being stripped
	char flagName[0x40];
	auto const pSingleFormat = pSingleFlag ? pSingleFlag : pBaseFlag;
	auto res = _snprintf_s(flagName, _TRUNCATE, pSingleFormat, "");

	if (res > 0 && flagName[res - 1] == '.')
	{
		flagName[res - 1] = '\0';
	}

	this->Base.Read(parser, pSection, flagName);

	NullableVector<T> veteran;
	_snprintf_s(flagName, _TRUNCATE, pBaseFlag, "Veteran");
	veteran.Read(parser, pSection, flagName);

	NullableVector<T> elite;
	_snprintf_s(flagName, _TRUNCATE, pBaseFlag, "Elite");
	elite.Read(parser, pSection, flagName);

	if (veteran.HasValue())
	{
		for (size_t i = 0; i < veteran.size(); i++)
		{
			this->Veteran[i] = veteran[i];
		}
	}

	if (elite.HasValue())
	{
		for (size_t i = 0; i < elite.size(); i++)
		{
			this->Elite[i] = elite[i];
		}
	}
}

template <typename T>
void __declspec(noinline) PromotableVector<T>::ReadList(INI_EX& parser, const char* pSection, const char* pFlag, bool allocate)
{
	bool numFirst = false;
	int flagLength = CRT::strlen(pFlag);

	for (int i = 1; i < flagLength; i++)
	{
		if (pFlag[i - 1] == '%')
		{
			if (pFlag[i] == 'd')
			{
				numFirst = true;
			}
			else
			{
				break;
			}
		}
	}

	char flag[0x40] = { '\0' };

	for (int i = 0;; i++)
	{
		Nullable<T> value;
		int res = 0;

		if (numFirst)
			res = _snprintf_s(flag, _TRUNCATE, pFlag, i, "");
		else
			res = _snprintf_s(flag, _TRUNCATE, pFlag, "", i);

		if (res > 0 && flag[res - 1] == '.')
			flag[res - 1] = '\0';

		if (flag[0] == '.')
			strcpy_s(flag, flag + 1);

		value.Read(parser, pSection, flag, allocate);

		if (!value.isset())
			break;

		this->Base.emplace_back(value.Get());
	}

	int size = static_cast<int>(this->Base.size());

	for (int i = 0; i < size; i++)
	{
		Nullable<T> veteran;
		Nullable<T> elite;

		if (numFirst)
			_snprintf_s(flag, _TRUNCATE, pFlag, i, "Veteran");
		else
			_snprintf_s(flag, _TRUNCATE, pFlag, "Veteran", i);

		veteran.Read(parser, pSection, flag, allocate);

		if (numFirst)
			_snprintf_s(flag, _TRUNCATE, pFlag, i, "Elite");
		else
			_snprintf_s(flag, _TRUNCATE, pFlag, "Elite", i);

		elite.Read(parser, pSection, flag, allocate);

		if (veteran.isset())
			this->Veteran.emplace(i, veteran.Get());

		if (elite.isset())
			this->Elite.emplace(i, elite.Get());
	}
}

template <typename T>
bool PromotableVector<T>::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return Savegame::ReadPhobosStream(stm, this->Base, registerForChange)
		&& Savegame::ReadPhobosStream(stm, this->Veteran, registerForChange)
		&& Savegame::ReadPhobosStream(stm, this->Elite, registerForChange);
}

template <typename T>
bool PromotableVector<T>::Save(PhobosStreamWriter& stm) const
{
	return Savegame::WritePhobosStream(stm, this->Base)
		&& Savegame::WritePhobosStream(stm, this->Veteran)
		&& Savegame::WritePhobosStream(stm, this->Elite);
}

// TimedWarheadEffect

template <typename T>
bool TimedWarheadValue<T>::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Savegame::ReadPhobosStream(Stm, this->Value, RegisterForChange) &&
		Savegame::ReadPhobosStream(Stm, this->Timer, RegisterForChange) &&
		Savegame::ReadPhobosStream(Stm, this->ApplyToHouses, RegisterForChange) &&
		Savegame::ReadPhobosStream(Stm, this->SourceWarhead, RegisterForChange);
}

template <typename T>
bool TimedWarheadValue<T>::Save(PhobosStreamWriter& Stm) const
{
	return Savegame::WritePhobosStream(Stm, this->Value) &&
		Savegame::WritePhobosStream(Stm, this->Timer) &&
		Savegame::WritePhobosStream(Stm, this->ApplyToHouses) &&
		Savegame::WritePhobosStream(Stm, this->SourceWarhead);
}