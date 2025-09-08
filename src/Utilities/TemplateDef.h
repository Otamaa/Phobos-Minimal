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
#include <VelocityClass.h>
#include <SWRange.h>
#include <CoordStruct.h>
#include <RocketStruct.h>

#include <FileFormats/_Loader.h>
#include <Helpers/Enumerators.h>

#include <Utilities/TechnoTypeConvertData.h>
#include <Utilities/CSFText.h>

//#include <New/Type/PaletteManager.h>

#include <array>
#include <iostream>
#include <string_view>

#include "Enumparser.h"

template<typename T>
struct IndexFinder
{
	static OPTIONALINLINE bool getindex(int& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate = false)
	{
		if (parser.ReadString(pSection, pKey))
		{
			const char* val = parser.value();

			if (GameStrings::IsBlank(val))
			{
				value = -1;
				return true;
			}

			int idx = value;

			if COMPILETIMEEVAL (std::is_pointer<T>::value)
			{
				using base_type = std::remove_pointer_t<T>;
				idx = base_type::FindIndexById(val);
			}
			else
			{
				idx = T::FindIndexById(val);
			}

			if (idx != -1)
			{
				value = idx;
				return true;
			}

			Debug::INIParseFailed(pSection, pKey, val);
		}

		return false;
	}
};

namespace detail
{
#pragma region getresult
	template <typename T>
	OPTIONALINLINE bool getresult(T& value, const std::string& parser, const char* pSection, const char* pKey, bool allocate = false) {
		static_assert(true, "Not Implemented!");
		return true;
	}

	template <>
	OPTIONALINLINE bool getresult<AffectedHouse>(AffectedHouse& value, const std::string& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (!parser.empty())
		{
			AffectedHouse resultData = AffectedHouse::None;
			char* context = nullptr;
			std::string copy = parser;

			for (auto pCur = strtok_s(copy.data(),
				Phobos::readDelims, &context);
				pCur;
				pCur = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				bool found = false;
				for (auto const& [pString, val] : EnumFunctions::AffectedHouse_ToStrings)
				{
					if (IS_SAME_STR_(pCur, pString))
					{
						found = true;
						resultData |= val;
						break;
					}
				}

				if (!found)
				{
					Debug::INIParseFailed(pSection, pKey, parser.c_str(), "Expected a affected house");
				}
			}

			value = resultData;
			return true;
		}
		return false;
	}

	template <>
	OPTIONALINLINE bool getresult<TechnoTypeConvertData>(TechnoTypeConvertData& value, const std::string& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (!parser.empty()) {

			std::string copy = parser;
			std::erase(copy, ' ');

			const auto nDelim = copy.find(":");
			if (nDelim == std::string::npos)
				return false;

			auto nFirst = copy.substr(0, nDelim);
			//second countais b:c
			auto nSecondPair = copy.substr(nDelim + 1);
			const auto nDelim2 = nSecondPair.find(":");

			if (nDelim2 != std::string::npos) {
				auto nSecondPair_1 = nSecondPair.substr(0, nDelim2);
				auto nSecondPair_2 = nSecondPair.substr(nDelim2 + 1);

				value.From.clear();
				char* context = nullptr;
				for (auto pCur = strtok_s(nFirst.data(), Phobos::readDelims, &context);
						pCur;
						pCur = strtok_s(nullptr, Phobos::readDelims, &context))
				{
					TechnoTypeClass* buffer = nullptr;

					if (Parser<TechnoTypeClass*>::Parse(pCur, &buffer))
						value.From.push_back(buffer);
					else if (!allocate && !GameStrings::IsBlank(pCur))
						Debug::INIParseFailed(pSection, pKey, pCur, nullptr);
				}

				Parser<TechnoTypeClass*>::Parse(nSecondPair_1.c_str(), &value.To);
				detail::getresult<AffectedHouse>(value.Eligible, nSecondPair_2, pSection, pKey, allocate);

				//Debug::LogInfo("parsing[%s]%s with 3 values [%s - %s - %s]", pSection , pKey , nFirst.c_str() , nSecondPair_1.c_str() , nSecondPair_2.c_str());
			} else {
				value.From.clear();
				char* context = nullptr;
				for (auto pCur = strtok_s(nFirst.data(), Phobos::readDelims, &context);
						pCur;
						pCur = strtok_s(nullptr, Phobos::readDelims, &context))
				{
					TechnoTypeClass* buffer = nullptr;

					if (Parser<TechnoTypeClass*>::Parse(pCur, &buffer))
						value.From.push_back(buffer);
					else if (!allocate && !GameStrings::IsBlank(pCur))
						Debug::INIParseFailed(pSection, pKey, pCur, nullptr);
				}

				Parser<TechnoTypeClass*>::Parse(nSecondPair.c_str(), &value.To);
			}

			return true;
		}

		return true;
	}

	template <>
	OPTIONALINLINE bool getresult<TileType>(TileType& value, const std::string& parser, const char* pSection, const char* pKey, bool bAllocate)
	{
		if (!parser.empty()) {

			if (GameStrings::IsBlank(parser.c_str())) {
				value = TileType::ClearToSandLAT;
				return true;
			}

			for (size_t i = 1; i < EnumFunctions::TileType_ToStrings.size(); ++i) {
				if (IS_SAME_STR_(parser.c_str(), EnumFunctions::TileType_ToStrings[i])) {
					value = TileType(i);
					return true;
				}
			}

			Debug::INIParseFailed(pSection, pKey, parser.c_str(), "Expect valid TileType !");
		}
		return false;
	}

	template <>
	OPTIONALINLINE bool getresult<LandType>(LandType& value, const std::string& parser, const char* pSection, const char* pKey, bool bAllocate)
	{
		if (!parser.empty()) {

			if (GameStrings::IsBlank(parser.c_str())) {
				value = LandType::Clear;
				return true;
			}

			for (size_t i = 0; i < CellClass::LandTypeToStrings.size(); ++i) {
				if (IS_SAME_STR_(CellClass::LandTypeToStrings[i], parser.c_str())) {
					value = LandType(i);
					return true;
				}
			}

			Debug::INIParseFailed(pSection, pKey, parser.c_str(), "Expect Valind LandType");
		}

		return false;
	}

	template <>
	OPTIONALINLINE bool getresult<PhobosAbilityType>(PhobosAbilityType& value, const std::string& parser, const char* pSection, const char* pKey, bool bAllocate)
	{
		if (!parser.empty())
		{
			for (size_t i = 0; i < EnumFunctions::PhobosAbilityType_ToStrings.size(); ++i) {
				if (IS_SAME_STR_(EnumFunctions::PhobosAbilityType_ToStrings[i], parser.c_str())) {
					value = PhobosAbilityType(i);
					return true;
				}
			}

			bool found = false;
			for (size_t a = 0; a < TechnoTypeClass::AbilityTypeToStrings.c_size(); ++a) {
				if (IS_SAME_STR_(TechnoTypeClass::AbilityTypeToStrings[a], parser.c_str())) {
					found = true;
				}
			}

			if(!found)
				Debug::INIParseFailed(pSection, pKey, parser.c_str(), "Expect Valind AbilityTypes");
		}

		return false;
	}

	template <>
	OPTIONALINLINE bool getresult<Rank>(Rank& value, const std::string& parser, const char* pSection, const char* pKey, bool bAllocate)
	{
		if (!parser.empty())
		{
			for (size_t i = 0; i < EnumFunctions::Rank_ToStrings.size(); ++i)
			{
				if (IS_SAME_STR_(parser.c_str(), EnumFunctions::Rank_ToStrings[i]))
				{
					value = Rank(i);
					return true;
				}
			}

			Debug::INIParseFailed(pSection, pKey, parser.c_str(), "Expected a self Rank type");
		}

		return false;
	}

	template <>
	OPTIONALINLINE bool getresult<FacingType>(FacingType& value, const std::string& parser, const char* pSection, const char* pKey, bool bAllocate)
	{
		if (!parser.empty())
		{
			int nBuffer = std::stoi(parser);
			if (nBuffer >= (int)FacingType::Min && nBuffer < (int)FacingType::Max)
			{
				value = (FacingType)nBuffer;
				return true;
			}

			Debug::INIParseFailed(pSection, pKey, parser.c_str(), "Expected a FacingType between 0 and 8");
		}

		return false;
	}

#pragma endregion

	template <typename T>
	OPTIONALINLINE bool read(T& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate = false)
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

	// template <>
	// OPTIONALINLINE bool read<PaletteManager*>(PaletteManager*& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	// {
	// 	if (parser.ReadString(pSection, pKey))
	// 	{
	// 		if (!GeneralUtils::IsValidString(parser.value()))
	// 			return false;
	//
	// 		std::string flag = _strlwr(parser.value());
	// 		if (flag.find(".pal") == std::string::npos) {
	// 			flag += ".pal";
	// 		}
	//
	// 		if (const auto nResult = PaletteManager::FindOrAllocate(flag.c_str()))
	// 		{
	// 			if (!nResult->Palette)
	// 				return false;

	// 			value = nResult;
	// 			return true;
	// 		}
	// 		else
	// 		{
	// 			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a Palette Name");
	// 		}
	// 	}

	// 	return false;
	// }

	template <>
	OPTIONALINLINE bool read<TechnoTypeClass*>(TechnoTypeClass*& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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
	OPTIONALINLINE bool read<SHPStruct*>(SHPStruct*& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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

	template <>
	OPTIONALINLINE bool read<std::string> (std::string& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate) {
		if (parser.ReadString(pSection, pKey)) {
			value = parser.value();
			return true;
		}

		return false;
	}

#pragma endregion

#pragma region PartialVector
	template <>
	OPTIONALINLINE bool read<PartialVector2D<int>>(PartialVector2D<int>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		value.ValueCount = parser.Read2IntegerAndCount(pSection, pKey, (int*)&value);

		if (value.ValueCount > 0)
			return true;

		return false;
	}

	template <>
	OPTIONALINLINE bool read<PartialVector2D<double>>(PartialVector2D<double>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		value.ValueCount = parser.Read2DoubleAndCount(pSection, pKey, (double*)&value);

		if (value.ValueCount > 0)
			return true;

		return false;
	}

	template <>
	OPTIONALINLINE bool read<PartialVector3D<int>>(PartialVector3D<int>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		value.ValueCount = parser.Read3IntegerAndCount(pSection, pKey, (int*)&value);

		if (value.ValueCount > 0)
			return true;

		return false;
	}

	template <>
	OPTIONALINLINE bool read<ReversePartialVector3D<int>>(ReversePartialVector3D<int>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		value.ValueCount = parser.Read3IntegerAndCount(pSection, pKey, (int*)&value);

		if (value.ValueCount > 0)
			return true;

		return false;
	}

	template <>
	OPTIONALINLINE bool read<PartialVector3D<double>>(PartialVector3D<double>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		value.ValueCount = parser.Read3DoubleAndCount(pSection, pKey, (double*)&value);

		if (value.ValueCount > 0)
			return true;

		return false;
	}

	template <>
	OPTIONALINLINE bool read<PartialVector3D<float>>(PartialVector3D<float>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		value.ValueCount = parser.Read3Float(pSection, pKey, (float*)&value);

		if (value.ValueCount > 0)
			return true;

		return false;
	}
#pragma endregion

#pragma region structandval
	template <>
	OPTIONALINLINE bool read<bool>(bool& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (!parser.ReadBool(pSection, pKey, &value)) {

			if (!parser.empty())
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid boolean value [1, true, yes, 0, false, no]");

			return false;
		}

		return true;
	}

	template <>
	OPTIONALINLINE bool read<int>(int& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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
	OPTIONALINLINE bool read<unsigned int>(unsigned int& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		int buffer { 0 };
		if (parser.ReadInteger(pSection, pKey, &buffer) && buffer > 0 && (unsigned int)buffer <= MAX_VAL(unsigned int)) {
			value = (unsigned int)buffer;
			return true;
		}

		if (!parser.empty())
			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid unsigned int between 0 and 4294967295 inclusive");

		return false;
	}

	template <>
	OPTIONALINLINE bool read<unsigned short>(unsigned short& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		short buffer { 0 };
		if (parser.ReadShort(pSection, pKey, &buffer) && buffer > 0 && (unsigned short)buffer <= MAX_VAL(unsigned short)) {
			value = (unsigned short)buffer;
			return true;
		}

		if(!parser.empty())
			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid unsigned short between 0 and 65535 inclusive");

		return false;
	}

	template <>
	OPTIONALINLINE bool read<short>(short& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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
	OPTIONALINLINE bool read<BYTE>(BYTE& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (!parser.ReadBytes(pSection, pKey, &value)) {

			if(!parser.empty())
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid number");

			return false;
		}

		return true;
	}

	template <>
	OPTIONALINLINE bool read<float>(float& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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
	OPTIONALINLINE bool read<double>(double& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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
	OPTIONALINLINE bool read<CellStruct>(CellStruct& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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
	OPTIONALINLINE bool read<Point2D>(Point2D& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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
	OPTIONALINLINE bool read<Vector2D<int>>(Vector2D<int>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (!parser.Read2Integers(pSection, pKey, (int*)&value))
		{
			if (!parser.empty())
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid Vector2D");

			return false;
		}

		return true;
	}

	template <>
	OPTIONALINLINE bool read<Point3D>(Point3D& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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
	OPTIONALINLINE bool read<RectangleStruct>(RectangleStruct& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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
	OPTIONALINLINE bool read<Point2DBYTE>(Point2DBYTE& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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
	OPTIONALINLINE bool read<CoordStruct>(CoordStruct& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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
	OPTIONALINLINE bool read<Vector3D<BYTE>>(Vector3D<BYTE>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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
	OPTIONALINLINE bool read<Vector3D<int>>(Vector3D<int>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (!parser.Read3Integers(pSection, pKey, (int*)&value))
		{
			if (!parser.empty())
			{
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid 3 int Value");
			}

			return false;
		}

		return true;
	}

	template <>
	OPTIONALINLINE bool read<Vector2D<double>>(Vector2D<double>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if(parser.Read2Double(pSection, pKey, (double*)&value))
			return true;
		else if (!parser.empty()) {
			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid 2 floating point Value");
		}

		return false;
	}

	template <>
	OPTIONALINLINE bool read<CSFText>(CSFText& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			value = parser.value();
			return true;
		}
		return false;
	}

	template <>
	OPTIONALINLINE bool read<RocketStruct>(RocketStruct& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		auto ret = false;
		std::string _buffer(pKey);

		ret |= read(value.PauseFrames, parser, pSection, (_buffer + ".PauseFrames").c_str());
		ret |= read(value.TiltFrames, parser, pSection, (_buffer + ".TiltFrames").c_str());
		ret |= read(value.PitchInitial, parser, pSection, (_buffer + ".PitchInitial").c_str());
		ret |= read(value.PitchFinal, parser, pSection, (_buffer + ".PitchFinal").c_str());
		ret |= read(value.TurnRate, parser, pSection, (_buffer + ".TurnRate").c_str());

		// sic! integer read like a float.
		float buffer = 0.0f;
		if (read(buffer, parser, pSection, (_buffer + ".RaiseRate").c_str()))
		{
			value.RaiseRate = int(buffer);
			ret = true;
		}

		ret |= read(value.Acceleration, parser, pSection, (_buffer + ".Acceleration").c_str());
		ret |= read(value.Altitude, parser, pSection, (_buffer + ".Altitude").c_str());
		ret |= read(value.Damage, parser, pSection, (_buffer + ".Damage").c_str());
		ret |= read(value.EliteDamage, parser, pSection, (_buffer + ".EliteDamage").c_str());
		ret |= read(value.BodyLength, parser, pSection, (_buffer + ".BodyLength").c_str());
		ret |= read(value.LazyCurve, parser, pSection, (_buffer + ".LazyCurve").c_str());
		ret |= read(value.Type, parser, pSection, (_buffer + ".Type").c_str());

		return ret;
	}

	template <>
	OPTIONALINLINE bool read<Leptons>(Leptons& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		double buffer;
		//vanilla : return false if -1.0
		if (read(buffer,parser ,pSection ,pKey ,allocate ) && buffer != -1.0) {
			value = Leptons(buffer);
			return true;
		}

		return false;
	}

	template <>
	OPTIONALINLINE bool read<SWRange>(SWRange& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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
	inline bool read<LaserTrailDrawType>(LaserTrailDrawType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			for (const auto& [val, name] : EnumFunctions::LaserTrailDrawType_ToStrings) {
				if (IS_SAME_STR_(parser.value(), name.data())) {
					value = val;
					return true;
				}
			}

			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a LaserTrail draw type");
		}

		return false;
	}

	template <>
	inline bool read<AttachedAnimPosition>(AttachedAnimPosition& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey)) {
			for (const auto& [val, name] : EnumFunctions::AttachedAnimPosition_ToStrings) {
				if (IS_SAME_STR_(parser.value(), name.data())) {
					value |= val;
					return true;
				}
			}

			if(IS_SAME_STR_(parser.value(), "centre")) {
				value |= AttachedAnimPosition::Center;
				return true;
			}

			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a AttachedAnimPosition type");
		}

		return false;
	}

	template <>
	OPTIONALINLINE bool read<Rank>(Rank& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		return parser.ReadString(pSection, pKey)
			&& getresult<Rank>(value, parser.value(), pSection, pKey);
	}

	template <>
	OPTIONALINLINE bool read<Armor>(Armor& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		return parser.ReadArmor(pSection, pKey, (int*)&value);
	}

	template <>
	OPTIONALINLINE bool read<Edge>(Edge& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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

			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expect valid map Edge strings");
		}

		return false;
	}

	template <>
	OPTIONALINLINE bool read<TranslucencyLevel>(TranslucencyLevel& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		return value.Read(parser, pSection, pKey);
	}

	template <>
	OPTIONALINLINE bool read<HorizontalPosition>(HorizontalPosition& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			for (auto const& [pString , val] : EnumFunctions::HorizontalPosition_ToStrings)
			{
				if (IS_SAME_STR_(parser.value(), pString))
				{
					value = val;
					return true;
				}
			}

			if (IS_SAME_STR_(parser.value(), "centre"))
			{
				value = HorizontalPosition::Center;
				return true;
			}

			Debug::INIParseFailed(pSection, pKey, parser.value(), "Horizontal Position can be either Left, Center/Centre or Right");
		}

		return false;
	}

	template <>
	OPTIONALINLINE bool read<BannerNumberType>(BannerNumberType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			for (const auto& [pString, val] : EnumFunctions::BannerNumberType_ToStrings)
			{
				if (IS_SAME_STR_(parser.value(), pString))
				{
					value = val;
					return true;
				}
			}

			Debug::INIParseFailed(pSection, pKey, parser.value(), "Content.VariableFormat can be either none, prefixed, suffixed or fraction");
		}

		return false;
	}

	template <>
	OPTIONALINLINE bool read<VerticalPosition>(VerticalPosition& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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

			if (IS_SAME_STR_(parser.value(), "centre"))
			{
				value = VerticalPosition::Center;
				return true;
			}

			Debug::INIParseFailed(pSection, pKey, parser.value(), "Vertical Position can be either Top, Center/Centre or Bottom");

		}
		return false;
	}

	template <>
	OPTIONALINLINE bool read<SelfHealGainType>(SelfHealGainType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			for (size_t i = 0; i < EnumFunctions::SelfHealGainType_ToStrings.size(); ++i) {
				if (IS_SAME_STR_(parser.value(), EnumFunctions::SelfHealGainType_ToStrings[i])) {
					value = SelfHealGainType(i);
					return true;
				}
			}

			if (IS_SAME_STR_(parser.value(), "noheal")){
				value = SelfHealGainType::None;
				return true;
			}

			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a self heal gain type");
		}

		return false;
	}

	template <>
	OPTIONALINLINE bool read<SlaveReturnTo>(SlaveReturnTo& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			for (const auto& [pString , val] : EnumFunctions::SlaveReturnTo_ToStrings)
			{
				if (IS_SAME_STR_(parser.value(), pString))
				{
					value = val;
					return true;
				}
			}

			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a free-slave option, default to killer");
		}

		return false;
	}

	template <>
	OPTIONALINLINE bool read<KillMethod>(KillMethod& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			for (const auto& [pString, val] : EnumFunctions::KillMethod_ToStrings)
			{
				if (IS_SAME_STR_(parser.value(), pString))
				{
					value = val;
					return true;
				}
			}

			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a kill method, default disabled");
		}
		return false;
	}

	template <>
	OPTIONALINLINE bool read<IronCurtainFlag>(IronCurtainFlag& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			for (auto const& [pStrings ,val]: EnumFunctions::IronCurtainFlag_ToStrings)
			{
				if (IS_SAME_STR_(parser.value(), pStrings))
				{
					value = val;
					return true;
				}
			}

			Debug::INIParseFailed(pSection, pKey, parser.value(), "IronCurtainFlag can be either kill, invulnerable, ignore or random");
		}

		return false;
	}

	template <>
	OPTIONALINLINE bool read<OwnerHouseKind>(OwnerHouseKind& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey)) {
			for (size_t i = 0; i < EnumFunctions::OwnerHouseKind_ToStrings.size(); ++i) {
				if (IS_SAME_STR_(parser.value(), EnumFunctions::OwnerHouseKind_ToStrings[i].second.data())) {
					value = EnumFunctions::OwnerHouseKind_ToStrings[i].first;
					return true;
				}
			}

			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a owner house kind");
		}
		return false;
	}

	template <>
	OPTIONALINLINE bool read<Mission>(Mission& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			value = MissionClass::GetMissionById(parser.value());
			return true;
		}

		return false;
	}

	template <>
	OPTIONALINLINE bool read<SuperWeaponAITargetingMode>(SuperWeaponAITargetingMode& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			for (const auto& [pStrings, val] : EnumFunctions::SuperWeaponAITargetingMode_ToStrings)
			{
				if (IS_SAME_STR_(parser.value(), pStrings))
				{
					value = val;
					return true;
				}
			}

			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a AItargetingmode");
		}
		return false;
	}

	template <>
	OPTIONALINLINE bool read<AffectedTarget>(AffectedTarget& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			char* context = nullptr;
			AffectedTarget resultData = AffectedTarget::None;

			for (auto cur = strtok_s(parser.value(), Phobos::readDelims, &context);
				cur;
				cur = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				bool found = false;
				for (const auto& [pStrings , val] : EnumFunctions::AffectedTarget_ToStrings)
				{
					if (IS_SAME_STR_(cur, pStrings))
					{
						found = true;
						resultData |= val;
						break;
					}
				}

				if (!found)
				{
					Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a affected target");
					return false;
				}
			}

			value = resultData;
			return true;
		}
		return false;
	}

	template <>
	OPTIONALINLINE bool read<ChronoSparkleDisplayPosition>(ChronoSparkleDisplayPosition& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			char* context = nullptr;
			auto resultData = ChronoSparkleDisplayPosition::None;

			for (auto cur = strtok_s(parser.value(), Phobos::readDelims, &context);
				cur;
				cur = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				bool found = false;
				for (const auto& [pStrings , val] : EnumFunctions::ChronoSparkleDisplayPosition_ToStrings)
				{
					if (IS_SAME_STR_(cur, pStrings))
					{
						found = true;
						resultData |= val;
						break;
					}
				}

				if (!found) {
					Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a chrono sparkle position type");
				}
			}

			value = resultData;
			return true;
		}

		return false;
	}

	template <>
	OPTIONALINLINE bool read<SuperWeaponTarget>(SuperWeaponTarget& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			char* context = nullptr;
			SuperWeaponTarget resultData = SuperWeaponTarget::None;

			for (auto cur = strtok_s(parser.value(), Phobos::readDelims, &context);
				cur;
				cur = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				size_t result = 0;
				bool found = false;
				for (const auto& pStrings : EnumFunctions::AffectedTarget_ToStrings)
				{
					if (IS_SAME_STR_(cur, pStrings.first))
					{
						found = true;
						break;
					}
					++result;
				}

				if (!found || result == 9)
				{
					Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a SW target");
					return false;
				}
				else
				{
					switch (result)
					{
					case 0: resultData |= SuperWeaponTarget::None; break;
					case 1: resultData |= SuperWeaponTarget::Land; break;
					case 2: resultData |= SuperWeaponTarget::Water; break;

					case 14:
					case 3: resultData |= SuperWeaponTarget::Empty; break;

					case 4: resultData |= SuperWeaponTarget::Infantry; break;

					case 5:
					case 6: resultData |= SuperWeaponTarget::Unit; break;

					case 7:
					case 8: resultData |= SuperWeaponTarget::Building; break;

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
	OPTIONALINLINE bool read<TargetingConstraints>(TargetingConstraints& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			char* context = nullptr;
			auto resultData = TargetingConstraints::None;

			for (auto cur = strtok_s(parser.value(), Phobos::readDelims, &context);
				cur;
				cur = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				bool found = false;
				for (const auto& [pStrings , val] : EnumFunctions::TargetingConstraints_ToStrings) {
					if (IS_SAME_STR_(cur, pStrings)) {
						resultData |= val;
						found = true;
						break;
					}
				}

				if (!found) {
					Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a targeting constraint");
					return false;
				}
			}

			value = resultData;
			return true;
		}
		return false;
	}

	template <>
	OPTIONALINLINE bool read<TargetingPreference>(TargetingPreference& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			for (size_t i = 0; i < EnumFunctions::TargetingPreference_ToStrings.size(); ++i) {
				if(IS_SAME_STR_(parser.value(), EnumFunctions::TargetingPreference_ToStrings[i])){
					value = TargetingPreference(i);
					return true;
				}
			}

			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a targeting preference");
		}
		return false;
	}

	template <>
	OPTIONALINLINE bool read<DiscardCondition>(DiscardCondition& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			char* context = nullptr;
			DiscardCondition resultData = DiscardCondition::None;

			for (auto cur = strtok_s(parser.value(), Phobos::readDelims, &context);
				cur;
				cur = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				size_t result = 0;
				bool found = false;
				for (const auto& pStrings : EnumFunctions::DiscardCondition_to_strings)
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
					Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a DiscardCondition");
					return false;
				}
				else
				{
					switch (result)
					{
					case 0: resultData |= DiscardCondition::None; break;
					case 1: resultData |= DiscardCondition::Entry; break;
					case 2: resultData |= DiscardCondition::Move; break;
					case 3: resultData |= DiscardCondition::Stationary; break;
					case 4: resultData |= DiscardCondition::Drain; break;
					}
				}
			}

			value = resultData;
			return true;
		}
		return false;
	}

	template <>
	OPTIONALINLINE bool read<ExpireWeaponCondition>(ExpireWeaponCondition& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			char* context = nullptr;
			ExpireWeaponCondition resultData = ExpireWeaponCondition::None;

			for (auto cur = strtok_s(parser.value(), Phobos::readDelims, &context);
				cur;
				cur = strtok_s(nullptr, Phobos::readDelims, &context)) {

				size_t result = 0;
				bool found = false;
				for (const auto& pStrings : EnumFunctions::ExpireWeaponCondition_to_strings)
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
					Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a ExpireWeaponCondition");
					return false;
				}
				else
				{
					switch (result)
					{
					case 0: resultData |= ExpireWeaponCondition::None; break;
					case 1: resultData |= ExpireWeaponCondition::Expire; break;
					case 2: resultData |= ExpireWeaponCondition::Remove; break;
					case 3: resultData |= ExpireWeaponCondition::Death; break;
					case 4: resultData |= ExpireWeaponCondition::Discard; break;
					case 5: resultData = ExpireWeaponCondition::All;
						break;//switch break
						break;//loop break
					}
				}
			}

			value = resultData;
			return true;
		}
		return false;
	}

	template <>
	OPTIONALINLINE bool read<LandType>(LandType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		return parser.ReadString(pSection, pKey)
			&& getresult<LandType>(value, parser.value(), pSection, pKey, allocate);
	}

	template <>
	OPTIONALINLINE bool read<AffectedHouse>(AffectedHouse& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		return parser.ReadString(pSection, pKey)
			&& getresult<AffectedHouse>(value, parser.value(), pSection, pKey, allocate);
	}

	template <>
	OPTIONALINLINE bool read<AttachedAnimFlag>(AttachedAnimFlag& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			for (const auto& [pString , val] : EnumFunctions::AttachedAnimFlag_ToStrings)
			{
				if (IS_SAME_STR_(parser.value(), pString))
				{
					value = val;
					return true;
				}
			}

			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a AttachedAnimFlag");
		}

		return false;
	}

	template <>
	OPTIONALINLINE bool read<AreaFireTarget>(AreaFireTarget& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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
	OPTIONALINLINE bool read<TextAlign>(TextAlign& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			for (auto const& [pString , val]: EnumFunctions::TextAlign_ToStrings)
			{
				if (IS_SAME_STR_(parser.value(), pString))
				{
					value = val;
					return true;
				}
			}

			if (IS_SAME_STR_(parser.value(), "centre"))
			{
				value = TextAlign::Center;
				return true;
			}

			Debug::INIParseFailed(pSection, pKey, parser.value(), "Text Alignment can be either Left, Center/Centre or Right");
		}

		return false;
	}

	template <>
	OPTIONALINLINE bool read<Layer>(Layer& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			if(GameStrings::IsBlank(parser.value()))
				return false;

			for (size_t i = 0; i < CellClass::LayerToStrings.size(); ++i)
			{
				if (IS_SAME_STR_(parser.value(), CellClass::LayerToStrings[i]))
				{
					value = Layer(i);
					return true;
				}
			}

			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expect a Valid Layer !");
		}

		return false;
	}

	template <>
	OPTIONALINLINE bool read<AbstractType>(AbstractType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			if (IS_SAME_STR_(parser.value(), GameStrings::NoneStrb())) {
				value = AbstractType::None;
				return true;
			}

			for (size_t i = 0; i < AbstractClass::RTTIToString.size(); ++i) {
				if (IS_SAME_STR_(parser.value(), AbstractClass::RTTIToString[i].Name)) {
					value = AbstractType(i);
					return true;
				}
			}

			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expect a Valid AbstractType !");
		}

		return false;
	}

	template <>
	OPTIONALINLINE bool read<Locomotors>(Locomotors& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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

			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expect valid Locomotor CLSID or Name");
		}

		return false;
	}

	template <>
	OPTIONALINLINE bool read(TileType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		return parser.ReadString(pSection, pKey)
			&& getresult<TileType>(value, parser.value(), pSection, pKey, allocate);
	}

	template <>
	OPTIONALINLINE bool read<LandTypeFlags>(LandTypeFlags& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			auto parsed = LandTypeFlags::None;
			auto str = parser.value();
			char* context = nullptr;

			for (auto cur = strtok_s(str, Phobos::readDelims, &context);
				cur;
				cur = strtok_s(nullptr, Phobos::readDelims, &context)) {

				auto const landType = GroundType::GetLandTypeFromName(parser.value());

				if (landType >= LandType::Clear && landType <= LandType::Weeds) {
					parsed |= (LandTypeFlags)(1 << (char)landType);
				}
				else {
					Debug::INIParseFailed(pSection, pKey, cur, "Expected a land type name");
					return false;
				}
			}

			value = parsed;
			return true;
		}

		return false;
	}

#pragma endregion

#pragma region Vectorstuffs
	template <>
	OPTIONALINLINE bool read<TypeList<int>>(TypeList<int>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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

	//WARNING : this not checking for read first , make sure before using it !
	template <typename T , bool clearvec = true>
	OPTIONALINLINE void parse_values(std::vector<T>& vector, INI_EX& parser, const char* pSection, const char* pKey, bool allocate = false)
	{
		if COMPILETIMEEVAL (clearvec)
			vector.clear();

		char* context = nullptr;
		for (auto pCur = strtok_s(parser.value(), Phobos::readDelims, &context);
				pCur;
				pCur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			auto buffer = T();

			if (Parser<T>::Parse(pCur, &buffer))
				vector.push_back(std::move(buffer));
			else if (!allocate && !GameStrings::IsBlank(pCur))
				Debug::INIParseFailed(pSection, pKey, pCur, nullptr);
		}
	}

	template <typename T>
	OPTIONALINLINE void ReadVectors(std::vector<T>& vector, INI_EX& parser, const char* pSection, const char* pKey, bool allocate = false)
	{
		//static_assert(std::is_pointer<T>::value, "Pointer Required !");

		if (parser.ReadString(pSection, pKey))
		{
			detail::parse_values(vector, parser, pSection, pKey, allocate);
		}
	}

	template <typename T>
	OPTIONALINLINE void ReadVectorsAlloc(std::vector<T>& vector, INI_EX& parser, const char* pSection, const char* pKey, bool allocate = false)
	{
		static_assert(std::is_pointer<T>::value, "Pointer Required !");

		if (parser.ReadString(pSection, pKey))
		{
			detail::parse_Alloc_values(vector, parser, pSection, pKey, allocate);
		}
	}

	//WARNING : this not checking for read first , make sure before using it !
	template <typename T>
	OPTIONALINLINE void parse_Alloc_values(std::vector<T>& vector, INI_EX& parser, const char* pSection, const char* pKey, bool allocate = false)
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

			if (parseSucceeded)
				vector.push_back(buffer);
			else if(!GameStrings::IsBlank(pCur))
				Debug::INIParseFailed(pSection, pKey, pCur, nullptr);
		}
	}

	//WARNING : this not checking for read first , make sure before using it !
	template <>
	OPTIONALINLINE void parse_values<LandType>(std::vector<LandType>& vector, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		vector.clear();
		char* context = nullptr;
		for (auto cur = strtok_s(parser.value(), Phobos::readDelims, &context);
			cur;
			cur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			LandType buffer;
			if (getresult<LandType>(buffer, cur, pSection, pKey, allocate))
				vector.push_back(buffer);
		}
	}

	template <>
	OPTIONALINLINE void parse_values<FacingType>(std::vector<FacingType>& vector, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		vector.clear();
		char* context = nullptr;
		for (auto cur = strtok_s(parser.value(), Phobos::readDelims, &context);
			cur;
			cur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			FacingType buffer;
			if (getresult<FacingType>(buffer, cur, pSection, pKey, allocate))
				vector.push_back(buffer);
		}
	}

	template <>
	OPTIONALINLINE void parse_values<PhobosAbilityType>(std::vector<PhobosAbilityType>& vector, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		vector.clear();
		char* context = nullptr;
		for (auto cur = strtok_s(parser.value(), Phobos::readDelims, &context);
			cur;
			cur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			PhobosAbilityType buffer;
			if (getresult<PhobosAbilityType>(buffer, cur, pSection, pKey, allocate))
				vector.push_back(buffer);
		}
	}

	template <>
	OPTIONALINLINE void parse_values<TechnoTypeConvertData>(std::vector<TechnoTypeConvertData>& vector, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		vector.clear();
		char* context = nullptr;
		for (auto cur = strtok_s(parser.value(), Phobos::readDelims, &context);
			cur;
			cur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			TechnoTypeConvertData buffer;
			if (getresult<TechnoTypeConvertData>(buffer, cur, pSection, pKey, allocate))
				vector.push_back(buffer);
		}
	}

	template <>
	OPTIONALINLINE void parse_values<TileType>(std::vector<TileType>& vector, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		vector.clear();
		char* context = nullptr;
		for (auto cur = strtok_s(parser.value(),
			Phobos::readDelims, &context);
			cur;
			cur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			TileType buffer;
			if (getresult<TileType>(buffer, cur, pSection, pKey, allocate))
				vector.push_back(buffer);
		}
	}

	template <>
	OPTIONALINLINE void parse_values<Mission>(std::vector<Mission>& vector, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		vector.clear();
		char* context = nullptr;
		for (auto cur = strtok_s(parser.value(),
			Phobos::readDelims, &context);
			cur;
			cur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			Mission buffer = MissionClass::GetMissionById(cur);
			if (buffer != Mission::None)
				vector.push_back(buffer);
		}
	}

	template <>
	OPTIONALINLINE void parse_values<Rank>(std::vector<Rank>& vector, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		vector.clear();
		char* context = nullptr;
		for (auto cur = strtok_s(parser.value(),
			Phobos::readDelims, &context);
			cur;
			cur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			Rank buffer;
			if (getresult<Rank>(buffer, cur, pSection, pKey, allocate))
				vector.push_back(buffer);
		}
	}

	template <typename Lookuper, typename T>
	OPTIONALINLINE void parse_indexes(std::vector<T>& vector, INI_EX& parser, const char* pSection, const char* pKey)
	{
		vector.clear();
		char* context = nullptr;
		for (auto pCur = strtok_s(parser.value(), Phobos::readDelims, &context);
			pCur;
			pCur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			int idx = -1;
			if COMPILETIMEEVAL (std::is_pointer<Lookuper>::value) {
				using base_type = std::remove_pointer_t<Lookuper>;
				idx = base_type::FindIndexById(pCur);
			} else { idx = Lookuper::FindIndexById(pCur); }

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

	template <typename T>
	OPTIONALINLINE bool getindex(int& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate = false) {
		return IndexFinder<T>::getindex(value , parser , pSection , pKey , allocate);
	}
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
	return Stm.Process(this->Value, RegisterForChange);
}

template <typename T>
bool Valueable<T>::Save(PhobosStreamWriter& Stm) const
{
	return Stm.Process(this->Value);
}

// ValueableIdx
template <typename Lookuper>
void NOINLINE ValueableIdx<Lookuper>::Read(INI_EX& parser, const char* pSection, const char* pKey)
{
	detail::getindex<Lookuper>(this->Value, parser, pSection, pKey);
}

// Nullable
template <typename T>
void NOINLINE Nullable<T>::Read(INI_EX& parser, const char* pSection, const char* pKey, bool Allocate)
{
	if (detail::read(this->Value, parser, pSection, pKey, Allocate))
	{
		const char* val = parser.value();

		if(strlen(val) != 0) {
			if (IS_SAME_STR_(val, DEFAULT_STR2)) {
				this->Reset();
			} else {
				this->HasValue = true;
			}
		}
	}
}

template <typename T>
bool Nullable<T>::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->Reset();
	if (!Stm.Load(this->HasValue))
		return false;

	if (this->HasValue) {
		return Stm.Process(this->Value, RegisterForChange);
	}

	return true;
}

template <typename T>
bool Nullable<T>::Save(PhobosStreamWriter& Stm) const
{
	if (!Stm.Save(this->HasValue))
		return false;

	if (this->HasValue) {
		return Stm.Process(this->Value);
	}

	return true;
}

// NullableIdx
template <typename Lookuper, EnumCheckMode mode>
void NOINLINE NullableIdx<Lookuper, mode>::Read(INI_EX& parser, const char* pSection, const char* pKey)
{
	if COMPILETIMEEVAL (mode == EnumCheckMode::default) {
		if (detail::getindex<Lookuper>(this->Value, parser, pSection, pKey))
			this->HasValue = true;
	}
	else
	{
		if (parser.ReadString(pSection, pKey))
		{
			const char* val = parser.value();

			if COMPILETIMEEVAL (mode != EnumCheckMode::disable){
				if (GameStrings::IsBlank(val)) {
					this->Value = -1;
					this->HasValue = true;
					return;
				}
			}

			int idx = this->Value;

			if COMPILETIMEEVAL (std::is_pointer<Lookuper>::value)
			{
				using base_type = std::remove_pointer_t<Lookuper>;
				idx = base_type::FindIndexById(val);
			}
			else
			{
				idx = Lookuper::FindIndexById(val);
			}

			if (idx != -1)
			{
				this->Value = idx;
				this->HasValue = true;
				return;
			}

			Debug::INIParseFailed(pSection, pKey, val);
		}
	}
}

// Promotable
template <typename T>
void NOINLINE Promotable<T>::Read(INI_EX& parser, const char* const pSection, const char* const pBaseFlag, const char* const pSingleFlag, bool allocate)
{

	// read the common flag, with the trailing dot being stripped
	char flagbuffer[0x80];
	const auto res = IMPL_SNPRNINTF(flagbuffer, sizeof(flagbuffer), pSingleFlag ? pSingleFlag : pBaseFlag, Phobos::readDefval); //remove the formatting

	if (res > 0)
	{
		if (flagbuffer[res - 1] == '.')
			flagbuffer[res - 1] = '\0';

		if (flagbuffer[0] == '.')
			strcpy_s(flagbuffer, flagbuffer + 1);

	}

	T placeholder {};
	if (detail::read(placeholder, parser, pSection, flagbuffer, allocate)) {
		this->SetAll(placeholder);
	}

	// read specific flags
	IMPL_SNPRNINTF(flagbuffer, sizeof(flagbuffer), pBaseFlag, EnumFunctions::Rank_ToStrings[(int)Rank::Rookie]);
	detail::read(this->Rookie, parser, pSection, flagbuffer, allocate);

	IMPL_SNPRNINTF(flagbuffer, sizeof(flagbuffer), pBaseFlag, EnumFunctions::Rank_ToStrings[(int)Rank::Veteran]);
	detail::read(this->Veteran, parser, pSection, flagbuffer, allocate);

	IMPL_SNPRNINTF(flagbuffer, sizeof(flagbuffer), pBaseFlag, EnumFunctions::Rank_ToStrings[(int)Rank::Elite]);
	detail::read(this->Elite, parser, pSection, flagbuffer, allocate);
};

template <typename T>
bool Promotable<T>::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Stm
		.Process(this->Rookie, RegisterForChange)
		.Process(this->Veteran, RegisterForChange)
		.Process(this->Elite, RegisterForChange)
		;
}

template <typename T>
bool Promotable<T>::Save(PhobosStreamWriter& Stm) const
{
	return Stm
		.Process(this->Rookie)
		.Process(this->Veteran)
		.Process(this->Elite)
		;
}

// NullablePromotable
template <typename T>
void NOINLINE NullablePromotable<T>::Read(INI_EX& parser, const char* const pSection, const char* const pBaseFlag, const char* const pSingleFlag)
{
	// read the common flag, with the trailing dot being stripped
	char flagbuffer[0x80];
	const auto res = IMPL_SNPRNINTF(flagbuffer, sizeof(flagbuffer), pSingleFlag ? pSingleFlag : pBaseFlag, Phobos::readDefval); //remove the formatting

	if (res > 0)
	{
		if (flagbuffer[res - 1] == '.')
			flagbuffer[res - 1] = '\0';

		if (flagbuffer[0] == '.')
			strcpy_s(flagbuffer, flagbuffer + 1);


	}

	T placeholder {};
	if (detail::read(placeholder, parser, pSection, flagbuffer))
	{
		this->SetAll(placeholder);
	}

	// read specific flags
	IMPL_SNPRNINTF(flagbuffer, sizeof(flagbuffer), pBaseFlag, EnumFunctions::Rank_ToStrings[(int)Rank::Rookie]);
	this->Rookie.Read(parser, pSection, flagbuffer);

	IMPL_SNPRNINTF(flagbuffer, sizeof(flagbuffer), pBaseFlag, EnumFunctions::Rank_ToStrings[(int)Rank::Veteran]);
	this->Veteran.Read(parser, pSection, flagbuffer);

	IMPL_SNPRNINTF(flagbuffer, sizeof(flagbuffer), pBaseFlag, EnumFunctions::Rank_ToStrings[(int)Rank::Elite]);
	this->Elite.Read(parser, pSection, flagbuffer);
};

template <typename T>
bool NullablePromotable<T>::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Stm
		.Process(this->Rookie, RegisterForChange)
		.Process(this->Veteran, RegisterForChange)
		.Process(this->Elite, RegisterForChange);
}

template <typename T>
bool NullablePromotable<T>::Save(PhobosStreamWriter& Stm) const
{
	return Stm
		.Process(this->Rookie)
		.Process(this->Veteran)
		.Process(this->Elite);
}

// ValueableVector
template <typename T>
void NOINLINE ValueableVector<T>::Read(INI_EX& parser, const char* pSection, const char* pKey, bool bAllocate)
{
	if (parser.ReadString(pSection, pKey))
	{
		detail::parse_values<T>(*this, parser, pSection, pKey, bAllocate);
	}
}

template <>
void NOINLINE ValueableVector<std::string>::Read(INI_EX& parser, const char* pSection, const char* pKey, bool bAllocate)
{
	if (parser.ReadString(pSection, pKey))
	{
		this->clear();
		char* context = nullptr;
		for (auto pCur = strtok_s(parser.value(), Phobos::readDelims, &context);
				pCur;
				pCur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			this->push_back(PhobosCRT::trim(pCur));
		}
	}
}

template <typename T>
bool ValueableVector<T>::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Stm.Process(this->AsVector());
}

template <typename T>
bool ValueableVector<T>::Save(PhobosStreamWriter& Stm) const
{
	return Stm.Process(this->AsVector());
}

template <>
bool ValueableVector<bool>::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Stm.Process(this->AsVector());
}

template <>
bool ValueableVector<bool>::Save(PhobosStreamWriter& Stm) const
{
	return Stm.Process(this->AsVector());
}

// NullableVector
template <typename T>
void NOINLINE NullableVector<T>::Read(INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (parser.ReadString(pSection, pKey))
	{
		auto const non_default = !IS_SAME_STR_(parser.value(), DEFAULT_STR2);
		this->hasValue = non_default;

		if (non_default)
		{
			detail::parse_values<T>(*this, parser, pSection, pKey , allocate);
		}
	}
}

template <typename T>
bool NullableVector<T>::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->clear();
	if (Stm.Process(this->hasValue, RegisterForChange))
	{
		return !this->hasValue || ValueableVector<T>::Load(Stm, RegisterForChange);
	}
	return false;
}

template <typename T>
bool NullableVector<T>::Save(PhobosStreamWriter& Stm) const
{
	if (Stm.Process(this->hasValue)) {
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

	if (res > 0) {
		if (flagName[res - 1] == '.') //dot in the end
			flagName[res - 1] = '\0';
		else if (flagName[0] == '.') //dot in the first
			flagName[0] = '\0';
	}

	this->BaseValue.Read(parser, pSection, flagName , Alloc);

	IMPL_SNPRNINTF(flagName, sizeof(flagName), pBaseFlag, EnumFunctions::HealthCondition_ToStrings[1]);
	this->ConditionYellow.Read(parser, pSection, flagName, Alloc);

	IMPL_SNPRNINTF(flagName, sizeof(flagName), pBaseFlag, EnumFunctions::HealthCondition_ToStrings[2]);
	this->ConditionRed.Read(parser, pSection, flagName, Alloc);
};

template <typename T>
bool Damageable<T>::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Stm
		.Process(this->BaseValue, RegisterForChange)
		.Process(this->ConditionYellow, RegisterForChange)
		.Process(this->ConditionRed, RegisterForChange);
}

template <typename T>
bool Damageable<T>::Save(PhobosStreamWriter& Stm) const
{
	return Stm
		.Process(this->BaseValue)
		.Process(this->ConditionYellow)
		.Process(this->ConditionRed);
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
	return Stm
		.Process(this->RedOnFire, RegisterForChange)
		.Process(this->GreenOnFire, RegisterForChange)
		.Process(this->YellowOnFire, RegisterForChange);
}

bool HealthOnFireData::Save(PhobosStreamWriter& Stm) const
{
	return Stm
		.Process(this->RedOnFire)
		.Process(this->GreenOnFire)
		.Process(this->YellowOnFire);
}

// DamageableVector
template <typename T>
void NOINLINE DamageableVector<T>::Read(INI_EX& parser, const char* const pSection, const char* const pBaseFlag, const char* const pSingleFlag)
{
	// read the common flag, with the trailing dot being stripped
	char flagName[0x80];
	auto const pSingleFormat = pSingleFlag ? pSingleFlag : pBaseFlag;
	auto res = IMPL_SNPRNINTF(flagName, sizeof(flagName), pSingleFormat, Phobos::readDefval);

	if (res > 0)
	{
		if (flagName[res - 1] == '.') //dot in the end
			flagName[res - 1] = '\0';
		else if (flagName[0] == '.') //dot in the first
			flagName[0] = '\0';
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
	return Stm
		.Process(this->BaseValue, RegisterForChange)
		.Process(this->ConditionYellow, RegisterForChange)
		.Process(this->ConditionRed, RegisterForChange)
		.Process(this->MaxValue, RegisterForChange);
}

template <typename T>
bool DamageableVector<T>::Save(PhobosStreamWriter& Stm) const
{
	return Stm
		.Process(this->BaseValue)
		.Process(this->ConditionYellow)
		.Process(this->ConditionRed)
		.Process(this->MaxValue);
}

/*
// PromotableVector
template <typename T>
void NOINLINE PromotableVector<T>::Read(INI_EX& parser, const char* const pSection, const char* const pBaseFlag, const char* const pSingleFlag)
{
	// read the common flag, with the trailing dot being stripped
	char flagName[0x80];
	auto const pSingleFormat = pSingleFlag ? pSingleFlag : pBaseFlag;
	auto res = IMPL_SNPRNINTF(flagName, sizeof(flagName), pSingleFormat, Phobos::readDefval);

	if (res > 0)
	{
		if (flagName[res - 1] == '.')
			flagName[res - 1] = '\0';

		if (flagName[0] == '.')
			strcpy_s(flagName, flagName + 1);

		this->Base.Read(parser, pSection, flagName);
	}

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

	char flag[0x40];

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

		this->Base.push_back(value.Get());
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
*/

// TimedWarheadEffect
template <typename T>
bool TimedWarheadValue<T>::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Stm
		.Process(this->Value, RegisterForChange)
		.Process(this->Timer, RegisterForChange)
		.Process(this->ApplyToHouses, RegisterForChange)
		.Process(this->SourceWarhead, RegisterForChange);
}

template <typename T>
bool TimedWarheadValue<T>::Save(PhobosStreamWriter& Stm) const
{
	return Stm
		.Process(this->Value)
		.Process(this->Timer)
		.Process(this->ApplyToHouses)
		.Process(this->SourceWarhead);
}

// MultiflagValueableVector

template<typename T, typename... TExtraArgs>
	requires MultiflagReadable<T, TExtraArgs...>
void NOINLINE MultiflagValueableVector<T, TExtraArgs...>::Read(INI_EX& parser, const char* const pSection, const char* const pBaseFlag, TExtraArgs&... extraArgs)
{
	char flagName[0x40];
	for (size_t i = 0; ; ++i)
	{
		T dataEntry {};

		// we expect %d for array number then %s for the subflag name, so we replace %s with itself (but escaped)
		IMPL_SNPRNINTF(flagName, sizeof(flagName), pBaseFlag, i, "%s");

		if (!dataEntry.Read(parser, pSection, flagName, extraArgs...))
		{
			if (i < this->size())
				continue;
			else
				break;
		}

		if (this->size() > i)
			this->operator[](i) = dataEntry;
		else
			this->push_back(dataEntry);
	}
}

// MultiflagNullableVector

template<typename T, typename... TExtraArgs>
	requires MultiflagReadable<T, TExtraArgs...>
void NOINLINE MultiflagNullableVector<T, TExtraArgs...>::Read(INI_EX& parser, const char* const pSection, const char* const pBaseFlag, TExtraArgs&... extraArgs)
{
	char flagName[0x40];
	for (size_t i = 0; ; ++i)
	{
		T dataEntry {};

		// we expect %d for array number then %s for the subflag name, so we replace %s with itself (but escaped)
		IMPL_SNPRNINTF(flagName, sizeof(flagName), pBaseFlag, i, "%s");

		if (!dataEntry.Read(parser, pSection, flagName, extraArgs...))
		{
			if (i < this->size())
				continue;
			else
				break;
		}

		if (this->size() > i)
			this->operator[](i) = dataEntry;
		else
			this->push_back(dataEntry);

		this->hasValue = true;
	}
}

// Animatable

// Animatable::KeyframeDataEntry

template <typename TValue>
bool NOINLINE Animatable<TValue>::KeyframeDataEntry::Read(INI_EX& parser, const char* const pSection, const char* const pBaseFlag, absolute_length_t absoluteLength)
{
	char flagName[0x40];

	Nullable<double> percentageTemp {};
	Nullable<absolute_length_t> absoluteTemp {};

	IMPL_SNPRNINTF(flagName, sizeof(flagName), pBaseFlag, "Percentage");
	percentageTemp.Read(parser, pSection, flagName);

	IMPL_SNPRNINTF(flagName, sizeof(flagName), pBaseFlag, "Absolute");
	absoluteTemp.Read(parser, pSection, flagName);

	if (!percentageTemp.isset() && !absoluteTemp.isset())
		return false;

	if (absoluteTemp.isset())
		this->Percentage = (double)absoluteTemp / absoluteLength;
	else
		this->Percentage = percentageTemp;

	IMPL_SNPRNINTF(flagName, sizeof(flagName), pBaseFlag, "Value");
	this->Value.Read(parser, pSection, flagName);

	return true;
};

template <typename TValue>
bool Animatable<TValue>::KeyframeDataEntry::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Stm
		.Process(this->Percentage, RegisterForChange)
		.Process(this->Value, RegisterForChange);
}

template <typename TValue>
bool Animatable<TValue>::KeyframeDataEntry::Save(PhobosStreamWriter& Stm) const
{
	return Stm
		.Process(this->Percentage)
		.Process(this->Value);
}

template <typename TValue>
COMPILETIMEEVAL TValue Animatable<TValue>::Get(double const percentage) const noexcept
{
	// This currently assumes the keyframes are ordered and there are no duplicates for same frame/percentage.
	// Thing is still far from lightweight as searching for the correct items requires going through the vector.

	TValue match {};

	if (!this->KeyframeData.size())
		return match;

	double startPercentage = 0.0;
	int i = this->KeyframeData.size() - 1;

	for (; i >= 0; i--)
	{
		auto const value = this->KeyframeData[i];

		if (percentage >= value.Percentage)
		{
			startPercentage = value.Percentage;
			match = value.Value;
			break;
		}
	}

	// Only interpolate if an interpolation mode is enabled and there's keyframes remaining.
	if (this->InterpolationMode != InterpolationMode::None && i >= 0 && (size_t)(i + 1) < this->KeyframeData.size())
	{
		auto const value = this->KeyframeData[i + 1];
		TValue nextValue = value.Value;
		double progressPercentage = (percentage - startPercentage) / (value.Percentage - startPercentage);
		return detail::interpolate(match, nextValue, progressPercentage, this->InterpolationMode);
	}

	return match;
}

template <typename TValue>
void NOINLINE Animatable<TValue>::Read(INI_EX& parser, const char* const pSection, const char* const pBaseFlag, absolute_length_t absoluteLength)
{
	char flagName[0x40];

	// we expect "BaseFlagName.%s" here
	IMPL_SNPRNINTF(flagName, sizeof(flagName), pBaseFlag, "Keyframe%d.%s");
	this->KeyframeData.Read(parser, pSection, flagName, absoluteLength);

	IMPL_SNPRNINTF(flagName, sizeof(flagName), pBaseFlag, "Interpolation");
	detail::read(this->InterpolationMode, parser, pSection, flagName);

	// Error handling
	bool foundError = false;
	double lastPercentage = -DBL_MAX;
	std::unordered_set<double> percentages {};

	for (size_t i = 0; i < this->KeyframeData.size(); i++)
	{
		auto const& value = this->KeyframeData[i];
		IMPL_SNPRNINTF(flagName, sizeof(flagName), pBaseFlag, "Keyframe%d");
		IMPL_SNPRNINTF(flagName, sizeof(flagName), flagName, i);

		if (percentages.contains(value.Percentage))
		{
			Debug::LogInfo("[Developer warning] [{}] {} has duplicated keyframe {}.", pSection, flagName, value.Percentage);
			foundError = true;
		}

		if (lastPercentage > value.Percentage)
		{
			Debug::LogInfo("[Developer warning] [{}] {} has keyframe out of order ({} after previous keyframe of {}).", pSection, flagName, value.Percentage, lastPercentage);
			foundError = true;
		}

		percentages.insert(value.Percentage);
		lastPercentage = value.Percentage;
	}

	if (foundError)
	{
		IMPL_SNPRNINTF(flagName, sizeof(flagName), pBaseFlag, "%s");
		int len = strlen(pBaseFlag);

		if (len >= 4)
			flagName[len - 3] = '\0';

		Debug::FatalErrorAndExit("[%s] %s has invalid keyframe data defined. Check debug log for more details.", pSection, flagName);
	}
};

template <typename TValue>
bool Animatable<TValue>::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Stm.Process(this->KeyframeData, RegisterForChange);
}

template <typename TValue>
bool Animatable<TValue>::Save(PhobosStreamWriter& Stm) const
{
	return Stm.Process(this->KeyframeData);
}