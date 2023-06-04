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
#include <Utilities/EnumFunctions.h>

#include <New/Type/PaletteManager.h>

#include <array>
#include <iostream>
#include <string_view>

namespace detail
{
	template <typename T>
	inline bool read(T& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate = false)
	{
		if (parser.ReadString(pSection, pKey))
		{
			using base_type = std::remove_pointer_t<T>;
			const auto pValue = parser.value();
			auto const parsed = (allocate ? base_type::FindOrAllocate : base_type::Find)(pValue);

			if (parsed || GameStrings::IsBlank(pValue))
			{
				value = parsed;
				return true;
			}
			else {
				Debug::INIParseFailed(pSection, pKey, pValue, nullptr);
			}
		}
		return false;
	}

#pragma region Pointers
	template <>
	inline bool read<PaletteManager*>(PaletteManager*& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			if (!GeneralUtils::IsValidString(parser.value()))
				return false;

			std::string flag = _strlwr(parser.value());
			if (flag.find(".pal") == std::string::npos)
			{
				flag += ".pal";
			}

			if (const auto nResult = PaletteManager::FindOrAllocate(flag.c_str()))
			{
				if (!nResult->Palette)
					return false;

				value = nResult;
				return true;
			}
			else
			{
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a Palette Name");
			}
		}

		return false;
	}

	template <>
	inline bool read<TechnoTypeClass*>(TechnoTypeClass*& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			const auto pValue = parser.value();
			const auto parsed = TechnoTypeClass::Find(pValue);

			if (parsed || GameStrings::IsBlank(pValue))
			{
				value = parsed;
				return true;
			}
			else {
				Debug::INIParseFailed(pSection, pKey, pValue, nullptr);
			}
		}
		return false;
	}

	template <>
	inline bool read<Theater_SHPStruct*>(Theater_SHPStruct*& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			std::string Result = GeneralUtils::ApplyTheaterSuffixToString(parser.c_str());

			if (Result.find(".shp") == std::string::npos)
			{
				Result += ".shp";
			}

			if (auto const pImage = FileSystem::LoadSHPFile(Result.c_str()))
			{
				value = reinterpret_cast<Theater_SHPStruct*>(pImage);
				return true;
			}

			Debug::Log("[Phobos] Failed to find file %s referenced by [%s]%s=%s\n", Result.c_str(), pSection, pKey, parser.value());
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

				GeneralUtils::ApplyTheaterExtToString(flag);

				if (auto const pImage = FileSystem::LoadSHPFile(flag.c_str()))
				{
					value = pImage;
					return true;
				}
			}
		}

		return false;
	}
#pragma endregion

#pragma region PartialVector
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
	inline bool read<ReversePartialVector3D<int>>(ReversePartialVector3D<int>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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
#pragma endregion

#pragma region structandval
	template <>
	inline bool read<bool>(bool& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (!parser.ReadBool(pSection, pKey, &value)) {

			if (!parser.empty())
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid boolean value [1, true, yes, 0, false, no]");

			return false;
		}

		return true;
	}

	template <>
	inline bool read<int>(int& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (!parser.ReadInteger(pSection, pKey, &value))
		{
			if (!parser.empty())
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid number");

			return false;

		}

		return true;
	}

	template <>
	inline bool read<unsigned short>(unsigned short& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		short buffer;
		if (parser.ReadShort(pSection, pKey, &buffer))
		{
			if (buffer < 0){ 
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid unsigned short between 0 and 65535 inclusive");
				return false;
			} else{
				value = static_cast<unsigned short>(buffer);
				return true;
			}

		}else if (!parser.empty()){ 
			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid unsigned short");
		}

		return false;
	}

	template <>
	inline bool read<short>(short& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (!parser.ReadShort(pSection, pKey, &value))
		{
			if (!parser.empty()) {
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid short");
			}

			return false;
		}

		return true;
	}

	template <>
	inline bool read<BYTE>(BYTE& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (!parser.ReadBytes(pSection, pKey, &value)) {

			if(!parser.empty())
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid number");

			return false;
		}

		return true;
	}

	template <>
	inline bool read<float>(float& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (!parser.ReadFloat(pSection, pKey, &value))
		{
			if (!parser.empty())
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid floating point number");

			return false;
		}

		return true;
	}

	template <>
	inline bool read<double>(double& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (!parser.ReadDouble(pSection, pKey, &value))
		{
			if (!parser.empty())
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid floating point number");

			return false;
		}

		return true;
	}

	template <>
	inline bool read<CellStruct>(CellStruct& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (!parser.Read2Short(pSection, pKey, (short*)&value))
		{
			if(!parser.empty())
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid CellStruct");

			return false;
		}

		return true;
	}

	template <>
	inline bool read<Point2D>(Point2D& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (!parser.Read2Integers(pSection, pKey, (int*)&value))
		{
			if (!parser.empty())
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid Point2D");

			return false;
		}

		return true;
	}

	template <>
	inline bool read<Point3D>(Point3D& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (!parser.Read3Integers(pSection, pKey, (int*)&value))
		{
			if (!parser.empty())
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid Point3D");

			return false;
		}

		return true;
	}

	template <>
	inline bool read<RectangleStruct>(RectangleStruct& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (!parser.Read4Integers(pSection, pKey, (int*)&value))
		{
			if (!parser.empty())
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid RectangleStruct");

			return false;
		}

		return true;
	}

	template <>
	inline bool read<Point2DBYTE>(Point2DBYTE& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (!parser.Read2Bytes(pSection, pKey, (BYTE*)&value))
		{
			if (!parser.empty())
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid 3 BYTE Value");

			return false;
		}

		return true;
	}

	template <>
	inline bool read<CoordStruct>(CoordStruct& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (!parser.Read3Integers(pSection, pKey, (int*)&value))
		{
			if (!parser.empty())
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid CoordStruct");

			return false;
		}
		return true;
	}

	template <>
	inline bool read<Vector3D<BYTE>>(Vector3D<BYTE>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (!parser.Read3Bytes(pSection, pKey, (BYTE*)&value))
		{
			if (!parser.empty()) {
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid 3 BYTE Value");
			}

			return false;
		}
		 
		return true;
	}

	template <>
	inline bool read<Vector2D<double>>(Vector2D<double>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (!parser.Read2Double(pSection, pKey, (double*)&value))
		{
			if (!parser.empty()) {
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid 2 floating point Value");
			}

			return false;
		}

		return true;
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
	inline bool read<RocketStruct>(RocketStruct& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		auto ret = false;

		char pFlagName[0x40];
		IMPL_SNPRNINTF(pFlagName, sizeof(pFlagName), "%s.PauseFrames", pKey);
		ret |= read(value.PauseFrames, parser, pSection, pFlagName);

		IMPL_SNPRNINTF(pFlagName, sizeof(pFlagName), "%s.TiltFrames", pKey);
		ret |= read(value.TiltFrames, parser, pSection, pFlagName);

		IMPL_SNPRNINTF(pFlagName, sizeof(pFlagName), "%s.PitchInitial", pKey);
		ret |= read(value.PitchInitial, parser, pSection, pFlagName);

		IMPL_SNPRNINTF(pFlagName, sizeof(pFlagName), "%s.PitchFinal", pKey);
		ret |= read(value.PitchFinal, parser, pSection, pFlagName);

		IMPL_SNPRNINTF(pFlagName, sizeof(pFlagName), "%s.TurnRate", pKey);
		ret |= read(value.TurnRate, parser, pSection, pFlagName);

		// sic! integer read like a float.
		IMPL_SNPRNINTF(pFlagName, sizeof(pFlagName), "%s.RaiseRate", pKey);
		float buffer = 0.0f;
		if (read(buffer, parser, pSection, pFlagName))
		{
			value.RaiseRate = int(buffer);
			ret = true;
		}

		IMPL_SNPRNINTF(pFlagName, sizeof(pFlagName), "%s.Acceleration", pKey);
		ret |= read(value.Acceleration, parser, pSection, pFlagName);

		IMPL_SNPRNINTF(pFlagName, sizeof(pFlagName), "%s.Altitude", pKey);
		ret |= read(value.Altitude, parser, pSection, pFlagName);

		IMPL_SNPRNINTF(pFlagName, sizeof(pFlagName), "%s.Damage", pKey);
		ret |= read(value.Damage, parser, pSection, pFlagName);

		IMPL_SNPRNINTF(pFlagName, sizeof(pFlagName), "%s.EliteDamage", pKey);
		ret |= read(value.EliteDamage, parser, pSection, pFlagName);

		IMPL_SNPRNINTF(pFlagName, sizeof(pFlagName), "%s.BodyLength", pKey);
		ret |= read(value.BodyLength, parser, pSection, pFlagName);

		IMPL_SNPRNINTF(pFlagName, sizeof(pFlagName), "%s.LazyCurve", pKey);
		ret |= read(value.LazyCurve, parser, pSection, pFlagName);

		IMPL_SNPRNINTF(pFlagName, sizeof(pFlagName), "%s.Type", pKey);
		ret |= read(value.Type, parser, pSection, pFlagName);

		return ret;
	}

	template <>
	inline bool read<Leptons>(Leptons& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		double buffer;
		if (parser.ReadDouble(pSection, pKey, &buffer))
		{
			if (buffer == -1.0) { //vanilla
				return false;
			} else {
				value = Leptons(buffer);
				return true;
			}
		}

		if (!parser.empty())
			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid floating point number");

		return false;
	}

	template <>
	inline bool read<SWRange>(SWRange& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			char* context = nullptr;
			char* p = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context);
			if (p && *p)
			{
				value.WidthOrRange = float(atof(p));
				value.Height = 0;

				p = strtok_s(nullptr, Phobos::readDelims, &context);
				if (p && *p)
				{
					value.Height = atoi(p);
				}
			}

			return true;
		}

		return false;
	}
#pragma endregion

#pragma region Enumstuffs

	template <>
	inline bool read<Rank>(Rank& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey)) {
			for (size_t i = 0; i < EnumFunctions::Rank_ToStrings.size(); ++i) {
				if (IS_SAME_STR_(parser.value(), EnumFunctions::Rank_ToStrings[i]))
				{
					value = Rank(i);
					return true;
				}
			}

			if (!parser.empty())
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a self Rank type");
		}

		return false;
	}

	template <>
	inline bool read<Armor>(Armor& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		return parser.ReadArmor(pSection, pKey, (int*)&value);
	}

	template <>
	inline bool read<Edge>(Edge& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			for (size_t i = 0; i < CellClass::EdgeToStrings.size(); ++i)
			{
				if (IS_SAME_STR_(parser.value(), CellClass::EdgeToStrings[i]))
				{
					value = (Edge)i;
					return true;
				}
			}

			if (!parser.empty())
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expect valid map Edge strings");
			
			return false;
		}
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
			size_t result = 0;
			bool found = false;
			for (auto const& pString : EnumFunctions::TextAlign_ToStrings)
			{
				if (IS_SAME_STR_(parser.value(), pString))
				{
					found = true;
					break;
				}
				++result;
			}

			if (!found)
			{
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Horizontal Position can be either Left, Center/Centre or Right");
			}
			else
			{
				switch (result)
				{
				case 1: value = HorizontalPosition::Left; return true;
				case 2: value = HorizontalPosition::Center; return true;
				case 3: value = HorizontalPosition::Right; return true;
				}
			}
		}
		return false;
	}

	template <>
	inline bool read<BannerNumberType>(BannerNumberType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			size_t result = 0;
			bool found = false;
			for (const auto& pString : EnumFunctions::BannerNumberType_ToStrings)
			{
				if (IS_SAME_STR_(parser.value(), pString))
				{
					found = true;
					break;
				}
				++result;
			}

			if (result == 0 || !found)
			{
				Debug::INIParseFailed(pSection, pKey, parser.value(),
				"Content.VariableFormat can be either none, prefixed, suffixed or fraction");
				return false;
			}
			else
			{
				value = BannerNumberType(result);
				return true;
			}
		}
		return false;
	}

	template <>
	inline bool read<VerticalPosition>(VerticalPosition& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			for (size_t i = 0; i < EnumFunctions::VerticalPosition_ToStrings.size(); ++i)
			{
				if (IS_SAME_STR_(parser.value(), EnumFunctions::VerticalPosition_ToStrings[i]))
				{
					value = VerticalPosition(i);
					return true;
				}
			}

			if (!parser.empty())
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Vertical Position can be either Top, Center/Centre or Bottom");

		}
		return false;
	}

	template <>
	inline bool read<SelfHealGainType>(SelfHealGainType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			for (size_t i = 0; i < EnumFunctions::SelfHealGainType_ToStrings.size(); ++i) {
				if (IS_SAME_STR_(parser.value(), EnumFunctions::SelfHealGainType_ToStrings[i])) {
					value = SelfHealGainType(i);
					return true;
				}
			}

			if(!parser.empty())
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a self heal gain type");
		}

		return false;
	}

	template <>
	inline bool read<SlaveReturnTo>(SlaveReturnTo& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			size_t result = 0;
			bool found = false;
			for (const auto& pString : EnumFunctions::SlaveReturnTo_ToStrings)
			{
				if (IS_SAME_STR_(parser.value(), pString))
				{
					found = true;
					break;
				}
				++result;
			}

			if (!found)
			{
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a free-slave option, default to killer");
			}
			else
			{
				switch (result)
				{
				case 0: value = SlaveReturnTo::Killer; return true;
				case 1: value = SlaveReturnTo::Master; return true;
				case 2:
				case 3:
				case 4: value = SlaveReturnTo::Suicide; return true;
				case 5: value = SlaveReturnTo::Neutral; return true;
				case 6: value = SlaveReturnTo::Civilian; return true;
				case 7: value = SlaveReturnTo::Special; return true;
				case 8: value = SlaveReturnTo::Random; return true;
				}
			}
		}
		return false;
	}

	template <>
	inline bool read<KillMethod>(KillMethod& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			size_t result = 0;
			bool found = false;
			for (const auto& pString : EnumFunctions::KillMethod_ToStrings)
			{
				if (IS_SAME_STR_(parser.value(), pString))
				{
					found = true;
					break;
				}
				++result;
			}

			if (!found)
			{
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a kill method, default disabled");
			}
			else
			{
				switch (result)
				{
				case 0: value = KillMethod::None; return true;
				case 1:
				case 2: value = KillMethod::Explode; return true;
				case 3: value = KillMethod::Vanish; return true;
				case 4: value = KillMethod::Sell; return true;
				case 5: value = KillMethod::Random; return true;

				}
			}
		}
		return false;
	}

	template <>
	inline bool read<IronCurtainFlag>(IronCurtainFlag& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			size_t result = 0;
			bool found = false;
			for (auto const& pStrings : EnumFunctions::IronCurtainFlag_ToStrings)
			{
				if (IS_SAME_STR_(parser.value(), pStrings))
				{
					found = true;
					break;
				}
				++result;
			}

			if (!found)
			{
				if (GameStrings::IsBlank(parser.value()))
				{
					value = IronCurtainFlag::Default;
					return true;
				}

				Debug::INIParseFailed(pSection, pKey, parser.value(), "IronCurtainFlag can be either kill, invulnerable, ignore or random");

			}
			else
			{
				switch (result)
				{
				case 0: value = IronCurtainFlag::Default; return true;
				case 1: value = IronCurtainFlag::Kill; return true;
				case 2: value = IronCurtainFlag::Invulnerable; return true;
				case 3: value = IronCurtainFlag::Ignore; return true;
				case 4: value = IronCurtainFlag::Random; return true;
				}
			}
		}

		return false;
	}

	template <>
	inline bool read<OwnerHouseKind>(OwnerHouseKind& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			for (size_t i = 0; i < EnumFunctions::OwnerHouseKind_ToStrings.size(); ++i)
			{
				if (IS_SAME_STR_(parser.value(), EnumFunctions::OwnerHouseKind_ToStrings[i]))
				{
					value = OwnerHouseKind(i);
					return true;
				}
			}

			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a owner house kind");
		}
		return false;
	}

	template <>
	inline bool read<Mission>(Mission& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			value = MissionClass::GetMissionById(parser.value());
			return true;
		}

		return false;
	}

	template <>
	inline bool read<SuperWeaponAITargetingMode>(SuperWeaponAITargetingMode& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			size_t result = 0;
			bool found = false;
			for (const auto& pStrings : EnumFunctions::SuperWeaponAITargetingMode_ToStrings)
			{
				if (IS_SAME_STR_(parser.value(), pStrings))
				{
					found = true;
					break;
				}
				++result;
			}

			if (!found)
			{
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a targeting mode");
			}
			else
			{
				if (result == 0)
				{
					value = SuperWeaponAITargetingMode::NoTarget; return true;
				}
				else
				{
					value = static_cast<SuperWeaponAITargetingMode>(result);
					return true;
				}
			}
		}
		return false;
	}

	template <>
	inline bool read<AffectedTarget>(AffectedTarget& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			char* context = nullptr;
			auto resultData = AffectedTarget::None;

			for (auto cur = strtok_s(parser.value(), Phobos::readDelims, &context);
				cur;
				cur = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				size_t result = 0;
				bool found = false;
				for (const auto& pStrings : EnumFunctions::AffectedTarget_ToStrings)
				{
					if (IS_SAME_STR_(cur, pStrings))
					{
						found = true;
						break;
					}
					++result;
				}

				if (!found)
				{
					Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a affected target");
					return false;
				}
				else
				{
					switch (result)
					{
					case 0: resultData |= AffectedTarget::None; break;
					case 1: resultData |= AffectedTarget::Land; break;
					case 2: resultData |= AffectedTarget::Water; break;
					case 14:
					case 3: resultData |= AffectedTarget::NoContent; break;
					case 4: resultData |= AffectedTarget::Infantry; break;
					case 5:
					case 6: resultData |= AffectedTarget::Unit; break;
					case 7:
					case 8: resultData |= AffectedTarget::Building; break;
					case 9: resultData |= AffectedTarget::Aircraft; break;
					case 10: resultData |= AffectedTarget::All; break;
					case 11: resultData |= AffectedTarget::AllCells; break;
					case 12: resultData |= AffectedTarget::AllTechnos; break;
					case 13: resultData |= AffectedTarget::AllContents; break;
					}
				}
			}

			value = resultData;
			return true;
		}
		return false;
	}

	template <>
	inline bool read<ChronoSparkleDisplayPosition>(ChronoSparkleDisplayPosition& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			char* context = nullptr;
			auto resultData = ChronoSparkleDisplayPosition::None;

			for (auto cur = strtok_s(parser.value(), Phobos::readDelims, &context);
				cur;
				cur = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				size_t result = 0;
				bool found = false;
				for (const auto& pStrings : EnumFunctions::ChronoSparkleDisplayPosition_ToStrings)
				{
					if (IS_SAME_STR_(cur, pStrings))
					{
						found = true;
						break;
					}
					++result;
				}

				if (!found || result == (size_t)ChronoSparkleDisplayPosition::None)
				{
					Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a chrono sparkle position type");
					return false;
				}
				else
				{
					switch (result)
					{
					case 1: resultData |= ChronoSparkleDisplayPosition::Building; break;
					case 2: resultData |= ChronoSparkleDisplayPosition::Occupants; break;
					case 3: resultData |= ChronoSparkleDisplayPosition::OccupantSlots; break;
					case 4: resultData |= ChronoSparkleDisplayPosition::All; break;
					}
				}
			}

			value = resultData;
			return true;
		}

		return false;
	}

	template <>
	inline bool read<SuperWeaponTarget>(SuperWeaponTarget& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			char* context = nullptr;
			auto resultData = SuperWeaponTarget::None;

			for (auto cur = strtok_s(parser.value(), Phobos::readDelims, &context);
				cur;
				cur = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				size_t result = 0;
				bool found = false;
				for (const auto& pStrings : EnumFunctions::AffectedTarget_ToStrings)
				{
					if (IS_SAME_STR_(cur, pStrings))
					{
						found = true;
						break;
					}
					++result;
				}

				if (!found)
				{
					err :
					Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a SW target");
					return false;
				}
				else
				{
					switch (result)
					{
					case 0: resultData |= SuperWeaponTarget::None; break;
					case 1: goto err;
					case 2: resultData |= SuperWeaponTarget::Water; break;
					case 14:
					case 3: resultData |= SuperWeaponTarget::Empty; break;
					case 4: resultData |= SuperWeaponTarget::Infantry; break;
					case 5:
					case 6: resultData |= SuperWeaponTarget::Unit; break;
					case 7:
					case 8: resultData |= SuperWeaponTarget::Building; break;
					case 9: goto err;
					case 10: resultData |= SuperWeaponTarget::All; break;
					case 11: resultData |= SuperWeaponTarget::AllCells; break;
					case 12: resultData |= SuperWeaponTarget::AllTechnos; break;
					case 13: resultData |= SuperWeaponTarget::AllContents; break;
					}
				}
			}

			value = resultData;
			return true;
		}
		return false;
	}

	template <>
	inline bool read<TargetingConstraint>(TargetingConstraint& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			char* context = nullptr;
			auto resultData = TargetingConstraint::None;

			for (auto cur = strtok_s(parser.value(), Phobos::readDelims, &context);
				cur;
				cur = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				size_t result = 0;
				bool found = false;
				for (const auto& pStrings : EnumFunctions::TargetingConstraint_ToStrings)
				{
					if (IS_SAME_STR_(cur, pStrings))
					{
						found = true;
						break;
					}
					++result;
				}

				if (!found)
				{
				err:
					Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a targeting constraint");
					return false;
				}
				else
				{
					switch (result)
					{
					case 0: resultData |= TargetingConstraint::None; break;
					case 1: goto err; break;
					case 2: resultData |= TargetingConstraint::DefensifeCellClear; break;
					case 3: resultData |= TargetingConstraint::Enemy; break;
					case 4: resultData |= TargetingConstraint::LighningStormInactive; break;
					case 5: resultData |= TargetingConstraint::DominatorInactive; break;
					case 6: resultData |= TargetingConstraint::Attacked; break;
					case 7: resultData |= TargetingConstraint::OffensiveCellSet; break;
					case 8: resultData |= TargetingConstraint::DefensiveCellSet; break;
					}
				}
			}

			value = resultData;
			return true;
		}
		return false;
	}

	template <>
	inline bool read<TargetingPreference>(TargetingPreference& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			size_t i = 0;
			while (IMPL_STRCMPI(parser.value(), EnumFunctions::TargetingPreference_ToStrings[i]))
			{
				if (i >= EnumFunctions::TargetingPreference_ToStrings.size()) {
					Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a targeting preference");
					return false;
				}
			}

			value = TargetingPreference(i);
			return true;
		}
		return false;
	}

	template <>
	inline bool read<LandType>(LandType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			for (size_t i = 0; i < CellClass::LandTypeToStrings.size(); ++i) {
				if (IS_SAME_STR_(parser.value(), CellClass::LandTypeToStrings[i])) {
					value = LandType(i);
					return true;
				}
			}

			if (GameStrings::IsBlank(parser.value()))
			{
				value = LandType::Clear;
				return true;

			}

			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expect Valind LandType");
		}

		return false;
	}

	template <>
	inline bool read<AffectedHouse>(AffectedHouse& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			auto resultData = AffectedHouse::None;
			char* context = nullptr;
			for (auto pCur = strtok_s(parser.value(),
				Phobos::readDelims, &context);
				pCur;
				pCur = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				size_t result = 0;
				bool found = false;
				for (auto const& pString : EnumFunctions::AffectedHouse_ToStrings)
				{
					if (IS_SAME_STR_(pCur, pString))
					{
						found = true;
						break;
					}
					++result;
				}

				if (!found || result == (size_t)AffectedHouse::None)
				{
					Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a affected house");
					return false;
				}
				else
				{
					switch (result)
					{
					case 1:
					case 2: resultData |= AffectedHouse::Owner; break;
					case 3:
					case 4: resultData |= AffectedHouse::Allies; break;
					case 5:
					case 6: resultData |= AffectedHouse::Enemies; break;
					case 7: resultData |= AffectedHouse::Team; break;
					case 8: resultData |= AffectedHouse::NotOwner; break;
					case 9: resultData |= AffectedHouse::All; break;
					case 10: resultData |= AffectedHouse::NotAllies;  break;
					default:
						break;
					}
				}
			}

			value = resultData;
			return true;
		}
		return false;
	}

	template <>
	inline bool read<AttachedAnimFlag>(AttachedAnimFlag& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			size_t result = 0;
			bool found = false;
			for (const auto& pString : EnumFunctions::AttachedAnimFlag_ToStrings)
			{
				if (IS_SAME_STR_(parser.value(), pString))
				{
					found = true;
					break;
				}
				++result;
			}

			if (!found)
			{
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a AttachedAnimFlag");
			}
			else
			{
				switch (result)
				{
				case 1: value = AttachedAnimFlag::Hides; return true;
				case 2: value = AttachedAnimFlag::Temporal; return true;
				case 3:	value = AttachedAnimFlag::Paused; return true;
				case 4: value = AttachedAnimFlag::PausedTemporal; return true;
				}
			}
		}
		return false;
	}

	template <>
	inline bool read<AreaFireTarget>(AreaFireTarget& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			for (size_t i = 0; i < EnumFunctions::AreaFireTarget_ToStrings.size(); ++i)
			{
				if (IS_SAME_STR_(parser.value(), EnumFunctions::AreaFireTarget_ToStrings[i]))
				{
					value = AreaFireTarget(i);
					return true;
				}
			}

			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected an AreaFire target");

		}
		return false;
	}

	template <>
	inline bool read<TextAlign>(TextAlign& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			size_t result = 0;
			bool found = false;
			for (auto const& pString : EnumFunctions::TextAlign_ToStrings)
			{
				if (IS_SAME_STR_(parser.value(), pString))
				{
					found = true;
					break;
				}
				++result;
			}

			if (!found)
			{
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Text Alignment can be either Left, Center/Centre or Right");
			}
			else
			{
				switch (result)
				{
				case 1: value = TextAlign::Left; return true;
				case 2: value = TextAlign::Center; return true;
				case 3: value = TextAlign::Right; return true;
				}
			}
		}

		return false;
	}

	template <>
	inline bool read<Layer>(Layer& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			for (size_t i = 0; i < CellClass::LayerToStrings.size(); ++i)
			{
				if (IS_SAME_STR_(parser.value(), CellClass::LayerToStrings[i]))
				{
					value = Layer(i);
					return true;
				}
			}

			if (!GameStrings::IsBlank(parser.value()) && !allocate)
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expect a Valid Layer !");

		}

		return false;
	}

	template <>
	inline bool read<AbstractType>(AbstractType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			if (IS_SAME_STR_(parser.value(), GameStrings::NoneStrb()))
			{
				value = AbstractType::None;
				return true;
			}

			for (size_t i = 0; i < AbstractClass::RTTIToString.size(); ++i)
			{
				if (IS_SAME_STR_(parser.value(), AbstractClass::RTTIToString[i].Name))
				{
					value = AbstractType(i);
					return true;
				}
			}

			if (!allocate)
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expect a Valid AbstractType !");
		}

		return false;
	}

	template <>
	inline bool read<Locomotors>(Locomotors& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			for (size_t i = 0; i < EnumFunctions::LocomotorPairs_ToStrings.size(); ++i)
			{
				const auto& [name, ID] = EnumFunctions::LocomotorPairs_ToStrings[i];
				if (IS_SAME_STR_(parser.value(), name) || IS_SAME_STR_(parser.value(), ID)) {
					value = Locomotors(i);
					return true;
				}
			}

			Debug::INIParseFailed(pSection, pKey, parser.value(), nullptr);
		}

		return false;
	}

	template <>
	inline bool read(TileType& value, INI_EX& parser, const char* pSection, const char* pKey, bool bAllocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			for (size_t i = 0; i < EnumFunctions::TileType_ToStrings.size(); ++i)
			{
				if (IS_SAME_STR_(parser.value(), EnumFunctions::TileType_ToStrings[i]))
				{
					value = TileType(i);
					return true;
				}
			}

			if (GameStrings::IsBlank(parser.value())) {
				value = TileType::ClearToSandLAT;
				return true;
			}

			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expect valid TileType !");
		}
		return false;
	}
#pragma endregion

#pragma region Vectorstuffs
	template <>
	inline bool read<TypeList<int>>(TypeList<int>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			value.Clear();
			char* context = nullptr;
			for (auto pCur = strtok_s(parser.value(), Phobos::readDelims, &context);
				pCur;
				pCur = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				int buffer = 0;
				if (Parser<int>::Parse(pCur, &buffer))
				{
					value.AddItem(buffer);
					return true;
				}
				else
				{
					Debug::INIParseFailed(pSection, pKey, pCur, nullptr);
				}
			}
		}
		return false;
	}

	template <typename T>
	inline void parse_values(std::vector<T>& vector, INI_EX& parser, const char* pSection, const char* pKey, bool bAllocate = false)
	{
		vector.clear();
		char* context = nullptr;
		for (auto pCur = strtok_s(parser.value(), Phobos::readDelims, &context);
			pCur;
			pCur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			auto buffer = T();
			if (Parser<T>::Parse(pCur, &buffer))
				vector.push_back(buffer);
			else if (!GameStrings::IsBlank(pCur))
				Debug::INIParseFailed(pSection, pKey, pCur, nullptr);
		}
	}

	template <typename T>
	inline void ReadVectorsAlloc(std::vector<T>& vector, INI_EX& parser, const char* pSection, const char* pKey, bool bAllocate = false)
	{
		static_assert(std::is_pointer<T>::value, "Pointer Required !");

		if (parser.ReadString(pSection, pKey))
		{	
			detail::parse_Alloc_values(vector, parser, pSection, pKey, bAllocate);
		}
	}

	template <typename T>
	inline void parse_Alloc_values(std::vector<T>& vector, INI_EX& parser, const char* pSection, const char* pKey, bool bAllocate = false)
	{
		static_assert(std::is_pointer<T>::value, "Pointer Required !");
		using base_type = std::remove_pointer_t<T>;
		vector.clear();

		char* context = nullptr;
		for (auto pCur = strtok_s(parser.value(), Phobos::readDelims, &context);
			pCur;
			pCur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			auto buffer = base_type::FindOrAllocate(pCur);
			bool parseSucceeded = buffer != nullptr;

			if (parseSucceeded || GameStrings::IsBlank(pCur))
				vector.push_back(buffer);
			else
				Debug::INIParseFailed(pSection, pKey, pCur, nullptr);
		}
	}

	template <>
	inline void parse_values(std::vector<LandType>& vector, INI_EX& parser, const char* pSection, const char* pKey, bool bAllocate)
	{
		vector.clear();
		char* context = nullptr;
		for (auto cur = strtok_s(parser.value(), Phobos::readDelims, &context);
			cur;
			cur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			LandType buffer;
			if (read<LandType>(buffer, parser, pSection, pKey, bAllocate))
				vector.push_back(buffer);
		}
	}

	template <>
	inline void parse_values(std::vector<TileType>& vector, INI_EX& parser, const char* pSection, const char* pKey, bool bAllocate)
	{
		vector.clear();
		char* context = nullptr;
		for (auto cur = strtok_s(parser.value(),
			Phobos::readDelims, &context);
			cur;
			cur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			TileType buffer;
			if (read<TileType>(buffer, parser, pSection, pKey, bAllocate))
				vector.push_back(buffer);
		}
	}

	template <typename Lookuper, typename T>
	inline void parse_indexes(std::vector<T>& vector, INI_EX& parser, const char* pSection, const char* pKey)
	{
		vector.clear();
		char* context = nullptr;
		for (auto pCur = strtok_s(parser.value(), Phobos::readDelims, &context);
			pCur;
			pCur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			int idx = Lookuper::FindIndexById(pCur);
			if (idx != -1 || GameStrings::IsBlank(pCur))
			{
				vector.push_back(idx);
			}
			else
			{
				Debug::INIParseFailed(pSection, pKey, pCur);
			}
		}
	}
#pragma endregion

}

// Valueable
template <typename T>
void NOINLINE Valueable<T>::Read(INI_EX& parser, const char* pSection, const char* pKey, bool Allocate)
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
void NOINLINE ValueableIdx<Lookuper>::Read(INI_EX& parser, const char* pSection, const char* pKey)
{
	if (parser.ReadString(pSection, pKey))
	{
		const char* val = parser.value();
		int idx = -1;

		if constexpr (std::is_pointer<Lookuper>::value)
		{
			using base_type = std::remove_pointer_t<Lookuper>;
			idx = base_type::FindIndexById(val);
		}
		else
			idx = Lookuper::FindIndexById(val);

		if (idx != -1 || GameStrings::IsBlank(val))
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
void NOINLINE Nullable<T>::Read(INI_EX& parser, const char* pSection, const char* pKey, bool Allocate)
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
void NOINLINE NullableIdx<Lookuper>::Read(INI_EX& parser, const char* pSection, const char* pKey)
{
	if (parser.ReadString(pSection, pKey))
	{
		const char* val = parser.value();
		int idx = Lookuper::FindIndexById(val);
		if (idx != -1 || GameStrings::IsBlank(val)) //if it is blank , count as read , but return -1
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
void NOINLINE Promotable<T>::Read(INI_EX& parser, const char* const pSection, const char* const pBaseFlag, const char* const pSingleFlag, bool allocate)
{

	// read the common flag, with the trailing dot being stripped
	char flagName[0x80];
	auto const pSingleFormat = pSingleFlag ? pSingleFlag : pBaseFlag;
	auto res = IMPL_SNPRNINTF(flagName, sizeof(flagName), pSingleFormat, Phobos::readDefval);
	if (res > 0 && flagName[res - 1] == '.')
	{
		flagName[res - 1] = '\0';
	}

	T placeholder {};
	if (detail::read(placeholder, parser, pSection, flagName , allocate))
	{
		this->SetAll(placeholder);
	}

	// read specific flags
	IMPL_SNPRNINTF(flagName, sizeof(flagName), pBaseFlag, EnumFunctions::Rank_ToStrings[(int)Rank::Rookie]);
	detail::read(this->Rookie, parser, pSection, flagName, allocate);

	IMPL_SNPRNINTF(flagName, sizeof(flagName), pBaseFlag, EnumFunctions::Rank_ToStrings[(int)Rank::Veteran]);
	detail::read(this->Veteran, parser, pSection, flagName, allocate);

	IMPL_SNPRNINTF(flagName, sizeof(flagName), pBaseFlag, EnumFunctions::Rank_ToStrings[(int)Rank::Elite]);
	detail::read(this->Elite, parser, pSection, flagName, allocate);
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

// NullablePromotable
template <typename T>
void NOINLINE NullablePromotable<T>::Read(INI_EX& parser, const char* const pSection, const char* const pBaseFlag, const char* const pSingleFlag)
{

	// read the common flag, with the trailing dot being stripped
	char flagName[0x100];
	auto const pSingleFormat = pSingleFlag ? pSingleFlag : pBaseFlag;
	auto res = IMPL_SNPRNINTF(flagName, sizeof(flagName), pSingleFormat, Phobos::readDefval);
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
	IMPL_SNPRNINTF(flagName, sizeof(flagName), pBaseFlag, EnumFunctions::Rank_ToStrings[(int)Rank::Rookie]);
	this->Rookie.Read(parser, pSection, flagName);

	IMPL_SNPRNINTF(flagName, sizeof(flagName), pBaseFlag, EnumFunctions::Rank_ToStrings[(int)Rank::Veteran]);
	this->Veteran.Read(parser, pSection, flagName);

	IMPL_SNPRNINTF(flagName, sizeof(flagName), pBaseFlag, EnumFunctions::Rank_ToStrings[(int)Rank::Elite]);
	this->Elite.Read(parser, pSection, flagName);
};

template <typename T>
bool NullablePromotable<T>::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Savegame::ReadPhobosStream(Stm, this->Rookie, RegisterForChange)
		&& Savegame::ReadPhobosStream(Stm, this->Veteran, RegisterForChange)
		&& Savegame::ReadPhobosStream(Stm, this->Elite, RegisterForChange);
}

template <typename T>
bool NullablePromotable<T>::Save(PhobosStreamWriter& Stm) const
{
	return Savegame::WritePhobosStream(Stm, this->Rookie)
		&& Savegame::WritePhobosStream(Stm, this->Veteran)
		&& Savegame::WritePhobosStream(Stm, this->Elite);
}

// ValueableVector
template <typename T>
void NOINLINE ValueableVector<T>::Read(INI_EX& parser, const char* pSection, const char* pKey, bool bAllocate)
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
void NOINLINE NullableVector<T>::Read(INI_EX& parser, const char* pSection, const char* pKey)
{
	if (parser.ReadString(pSection, pKey))
	{
		this->clear();

		auto const non_default = !IS_SAME_STR_(parser.value(), DEFAULT_STR2);
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
void NOINLINE ValueableIdxVector<Lookuper>::Read(INI_EX& parser, const char* pSection, const char* pKey)
{
	if (parser.ReadString(pSection, pKey))
	{
		this->clear();
		detail::parse_indexes<Lookuper>(*this, parser, pSection, pKey);
	}
}

// NullableIdxVector
template <typename Lookuper>
void NOINLINE NullableIdxVector<Lookuper>::Read(INI_EX& parser, const char* pSection, const char* pKey)
{
	if (parser.ReadString(pSection, pKey))
	{
		this->clear();

		auto const non_default = !IS_SAME_STR_(parser.value(), DEFAULT_STR2);
		this->hasValue = non_default;

		if (non_default)
		{
			detail::parse_indexes<Lookuper>(*this, parser, pSection, pKey);
		}
	}
}

// Damageable
template <typename T>
void NOINLINE Damageable<T>::Read(INI_EX& parser, const char* const pSection, const char* const pBaseFlag, const char* const pSingleFlag, bool Alloc)
{
	// read the common flag, with the trailing dot being stripped
	char flagName[0x80];
	auto const pSingleFormat = pSingleFlag ? pSingleFlag : pBaseFlag;
	auto res = IMPL_SNPRNINTF(flagName, sizeof(flagName), pSingleFormat, Phobos::readDefval);

	if (res > 0 && flagName[res - 1] == '.')
		flagName[res - 1] = '\0';

	this->BaseValue.Read(parser, pSection, flagName , Alloc);

	IMPL_SNPRNINTF(flagName, sizeof(flagName), pBaseFlag, EnumFunctions::HealthCondition_ToStrings[1]);
	this->ConditionYellow.Read(parser, pSection, flagName, Alloc);

	IMPL_SNPRNINTF(flagName, sizeof(flagName), pBaseFlag, EnumFunctions::HealthCondition_ToStrings[2]);
	this->ConditionRed.Read(parser, pSection, flagName, Alloc);
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

bool NOINLINE HealthOnFireData::Read(INI_EX& parser, const char* pSection, const char* pKey)
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
void NOINLINE DamageableVector<T>::Read(INI_EX& parser, const char* const pSection, const char* const pBaseFlag, const char* const pSingleFlag)
{
	// read the common flag, with the trailing dot being stripped
	char flagName[0x80];
	auto const pSingleFormat = pSingleFlag ? pSingleFlag : pBaseFlag;
	auto res = IMPL_SNPRNINTF(flagName, sizeof(flagName), pSingleFormat, Phobos::readDefval);

	if (res > 0 && flagName[res - 1] == '.')
	{
		flagName[res - 1] = '\0';
	}

	this->BaseValue.Read(parser, pSection, flagName);

	IMPL_SNPRNINTF(flagName, sizeof(flagName), pBaseFlag, EnumFunctions::HealthCondition_ToStrings[1]);
	this->ConditionYellow.Read(parser, pSection, flagName);

	IMPL_SNPRNINTF(flagName, sizeof(flagName), pBaseFlag, EnumFunctions::HealthCondition_ToStrings[2]);
	this->ConditionRed.Read(parser, pSection, flagName);

	IMPL_SNPRNINTF(flagName, sizeof(flagName), pBaseFlag, "MaxValue");
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
void NOINLINE PromotableVector<T>::Read(INI_EX& parser, const char* const pSection, const char* const pBaseFlag, const char* const pSingleFlag)
{
	// read the common flag, with the trailing dot being stripped
	char flagName[0x80];
	auto const pSingleFormat = pSingleFlag ? pSingleFlag : pBaseFlag;
	auto res = IMPL_SNPRNINTF(flagName, sizeof(flagName), pSingleFormat, Phobos::readDefval);

	if (res > 0 && flagName[res - 1] == '.')
	{
		flagName[res - 1] = '\0';
	}

	this->Base.Read(parser, pSection, flagName);

	NullableVector<T> veteran;
	IMPL_SNPRNINTF(flagName, sizeof(flagName), pBaseFlag, EnumFunctions::Rank_ToStrings[(int)Rank::Veteran]);
	veteran.Read(parser, pSection, flagName);

	NullableVector<T> elite;
	IMPL_SNPRNINTF(flagName, sizeof(flagName), pBaseFlag, EnumFunctions::Rank_ToStrings[(int)Rank::Elite]);
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
void NOINLINE PromotableVector<T>::ReadList(INI_EX& parser, const char* pSection, const char* pFlag, bool allocate)
{
	bool numFirst = false;
	int flagLength = strlen(pFlag);

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
			res = IMPL_SNPRNINTF(flag, sizeof(flag), pFlag, i, Phobos::readDefval);
		else
			res = IMPL_SNPRNINTF(flag, sizeof(flag), pFlag, Phobos::readDefval, i);

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
			IMPL_SNPRNINTF(flag, sizeof(flag), pFlag, i, EnumFunctions::Rank_ToStrings[(int)Rank::Veteran]);
		else
			IMPL_SNPRNINTF(flag, sizeof(flag), pFlag, EnumFunctions::Rank_ToStrings[(int)Rank::Veteran], i);

		veteran.Read(parser, pSection, flag, allocate);

		if (numFirst)
			IMPL_SNPRNINTF(flag, sizeof(flag), pFlag, i, EnumFunctions::Rank_ToStrings[(int)Rank::Elite]);
		else
			IMPL_SNPRNINTF(flag, sizeof(flag), pFlag, EnumFunctions::Rank_ToStrings[(int)Rank::Elite], i);

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