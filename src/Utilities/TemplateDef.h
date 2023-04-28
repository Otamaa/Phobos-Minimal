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
	inline bool read<PaletteManager*>(PaletteManager*& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			if (!GeneralUtils::IsValidString(parser.value()))
				return false;

			std::string flag = _strlwr(parser.value());
			if (flag.find(".pal") == std::string::npos) {
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
				Debug::INIParseFailed(pSection, pKey, parser.value(), "[Phobos] Expected a Palette Name");
			}
		}

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
				goto error;

			switch (result)
			{
			case 1: value = HorizontalPosition::Left; return true;
			case 2: value = HorizontalPosition::Center; return true;
			case 3: value = HorizontalPosition::Right; return true;
			default:
			error:
				Debug::INIParseFailed(pSection, pKey, parser.value(), "[Phobos] Horizontal Position can be either Left, Center/Centre or Right");
				break;
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
				"[Phobos] Content.VariableFormat can be either none, prefixed, suffixed or fraction");
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

			Debug::INIParseFailed(pSection, pKey, parser.value(), "[Phobos] Vertical Position can be either Top, Center/Centre or Bottom");

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
			Debug::INIParseFailed(pSection, pKey, parser.value(), "[Phobos] Expected a valid boolean value [1, true, yes, 0, false, no]");
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

		}
		else if (!parser.empty())
		{
			Debug::INIParseFailed(pSection, pKey, parser.value(), "[Phobos] Expected a valid number");
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
				Debug::INIParseFailed(pSection, pKey, parser.value(), "[Phobos] Expected a valid number between 0 and 255 inclusive.");
			}
		}
		else if (!parser.empty())
		{
			Debug::INIParseFailed(pSection, pKey, parser.value(), "[Phobos] Expected a valid number");
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
			Debug::INIParseFailed(pSection, pKey, parser.value(), "[Phobos] Expected a valid floating point number");
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
			Debug::INIParseFailed(pSection, pKey, parser.value(), "[Phobos] Expected a valid floating point number");
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
			Debug::INIParseFailed(pSection, pKey, parser.value(), "[Phobos] Expected a valid 3 BYTE Value");
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
				Debug::Log("[Phobos] Failed to find file %s referenced by [%s]%s=%s\n", Result.c_str(), pSection, pKey, pValue);
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

				const auto nPos = flag.find("~");
				if (nPos != std::string::npos)
				{
					std::string pTheater =
						Theater::Get(ScenarioClass::Instance->Theater)->Letter;
					pTheater = GeneralUtils::lowercase(pTheater);

					flag.replace(nPos, 1, pTheater);
					Debug::Log("Found designated string at [%d] Replacing [%s] to [%s] \n",
					nPos, pValue, flag.c_str());
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
				Debug::INIParseFailed(pSection, pKey, parser.value(), "[Phobos] Expected a self heal gain type");
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
				Debug::INIParseFailed(pSection, pKey, parser.value(), "[Phobos] Expected a free-slave option, default to killer");
				return false;
			}

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
				Debug::INIParseFailed(pSection, pKey, parser.value(), "[Phobos] Expected a kill method, default disabled");
				return false;
			}

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
				if (INIClass::IsBlank(parser.value()))
				{
					value = IronCurtainFlag::Default;
					return true;
				}

				Debug::INIParseFailed(pSection, pKey, parser.value(), "[Phobos] IronCurtainFlag can be either kill, invulnerable, ignore or random");

			}

			switch (result)
			{
			case 0: value = IronCurtainFlag::Default; return true;
			case 1: value = IronCurtainFlag::Kill; return true;
			case 2: value = IronCurtainFlag::Invulnerable; return true;
			case 3: value = IronCurtainFlag::Ignore; return true;
			case 4: value = IronCurtainFlag::Random; return true;
			}
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
		float buffer;
		if (read(buffer, parser, pSection, pFlagName))
		{
			value.RaiseRate = Game::F2I(buffer);
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
			value = Leptons(buffer);
			return true;
		}
		else if (!parser.empty())
		{
			Debug::INIParseFailed(pSection, pKey, parser.value(), "[Phobos] Expected a valid floating point number");
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
					value = OwnerHouseKind::Default;
					return true;
				}
			}

			Debug::INIParseFailed(pSection, pKey, parser.value(), "[Phobos] Expected a owner house kind");
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

			if (!found)// no match ever found 
			{
				Debug::INIParseFailed(pSection, pKey, parser.value(), "[Phobos] Expected a targeting mode");
				return false;
			}

			switch (result)
			{
			case 0: value = SuperWeaponAITargetingMode::NoTarget; return true;
			default: value = static_cast<SuperWeaponAITargetingMode>(result); return true;
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

				if (!found) // no match ever found
					goto error;

				switch (result)
				{
				case 0: value |= AffectedTarget::None; break;
				case 1: value |= AffectedTarget::Land; break;
				case 2: value |= AffectedTarget::Water; break;
				case 14:
				case 3: value |= AffectedTarget::NoContent; break;
				case 4: value |= AffectedTarget::Infantry; break;
				case 5:
				case 6: value |= AffectedTarget::Unit; break;
				case 7:
				case 8: value |= AffectedTarget::Building; break;
				case 9: value |= AffectedTarget::Aircraft; break;
				case 10: value |= AffectedTarget::All; break;
				case 11: value |= AffectedTarget::AllCells; break;
				case 12: value |= AffectedTarget::AllTechnos; break;
				case 13: value |= AffectedTarget::AllContents; break;
				default:
				error:
					Debug::INIParseFailed(pSection, pKey, parser.value(), "[Phobos] Expected a affected target");
					break;
				}

			}
		}
		return false;
	}

	template <>
	inline bool read<LandType>(LandType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			for (size_t i = 0; i < EnumFunctions::LandType_ToStrings.size(); ++i)
			{
				if (IS_SAME_STR_(parser.value(), EnumFunctions::LandType_ToStrings[i]))
				{
					value = LandType(i);
					return true;
				}
			}

			if (!INIClass::IsBlank(parser.value()) && !allocate)
				Debug::INIParseFailed(pSection, pKey, parser.value(), nullptr);
		}

		return false;
	}

	template <>
	inline bool read<AffectedHouse>(AffectedHouse& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
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

				if (!found)
					goto error;

				switch (result)
				{
				case 1:
				case 2: value |= AffectedHouse::Owner; break;
				case 3:
				case 4: value |= AffectedHouse::Allies; break;
				case 5:
				case 6: value |= AffectedHouse::Enemies; break;
				case 7: value |= AffectedHouse::Team; break;
				case 8: value |= AffectedHouse::NotOwner; break;
				case 9: value |= AffectedHouse::All; break;
				default:
				error:
					Debug::INIParseFailed(pSection, pKey, parser.value(), "[Phobos] Expected a affected house");
					break;
				}
			}
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
				goto error;

			switch (result)
			{
			case 1: value = AttachedAnimFlag::Hides; return true;
			case 2: value = AttachedAnimFlag::Temporal; return true;
			case 3:	value = AttachedAnimFlag::Paused; return true;
			case 4: value = AttachedAnimFlag::PausedTemporal; return true;
			default:
			error:
				Debug::INIParseFailed(pSection, pKey, parser.value(), "[Phobos] Expected a AttachedAnimFlag");
				break;
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

			Debug::INIParseFailed(pSection, pKey, parser.value(), "[Phobos] Expected an area fire target");

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
				goto error;

			switch (result)
			{
			case 1: value = TextAlign::Left; return true;
			case 2: value = TextAlign::Center; return true;
			case 3: value = TextAlign::Right; return true;
			default:
			error:
				Debug::INIParseFailed(pSection, pKey, parser.value(), "[Phobos] Text Alignment can be either Left, Center/Centre or Right");
				break;
			}
		}

		return false;
	}

	template <>
	inline bool read<Layer>(Layer& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			for (size_t i = 0; i < EnumFunctions::LayerType_ToStrings.size(); ++i)
			{
				if (IS_SAME_STR_(parser.value(), EnumFunctions::LayerType_ToStrings[i]))
				{
					value = Layer(i);
					return true;
				}
			}

			if (!INIClass::IsBlank(parser.value()) && !allocate)
				Debug::INIParseFailed(pSection, pKey, parser.value(), "[Phobos] Expect a Valid Layer !");

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
				Debug::INIParseFailed(pSection, pKey, parser.value(), "[Phobos] Expect a Valid AbstractType !");
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
				if (IS_SAME_STR_(parser.value(), name) || IS_SAME_STR_(parser.value(), ID))
				{
					value = Locomotors(i);
					return true;
				}
			}

			if (!INIClass::IsBlank(parser.value()) && !allocate)
			{
				Debug::INIParseFailed(pSection, pKey, parser.value(), nullptr);
			}
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

			if (!INIClass::IsBlank(parser.value()) && !bAllocate)
			{
				Debug::INIParseFailed(pSection, pKey, parser.value(), nullptr);
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
		for (auto pCur = strtok_s(parser.value(), Phobos::readDelims, &context);
			pCur;
			pCur = strtok_s(nullptr, Phobos::readDelims, &context))
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
		for (auto pCur = strtok_s(parser.value(), Phobos::readDelims, &context);
			pCur;
			pCur = strtok_s(nullptr, Phobos::readDelims, &context))
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
	void parse_indexes(std::vector<T>& vector, INI_EX& parser, const char* pSection, const char* pKey)
	{
		char* context = nullptr;
		for (auto pCur = strtok_s(parser.value(), Phobos::readDelims, &context);
			pCur;
			pCur = strtok_s(nullptr, Phobos::readDelims, &context))
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
void NOINLINE Promotable<T>::Read(INI_EX& parser, const char* const pSection, const char* const pBaseFlag, const char* const pSingleFlag)
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
	if (detail::read(placeholder, parser, pSection, flagName))
	{
		this->SetAll(placeholder);
	}

	// read specific flags
	IMPL_SNPRNINTF(flagName, sizeof(flagName), pBaseFlag, EnumFunctions::Rank_ToStrings[(int)Rank::Rookie]);
	detail::read(this->Rookie, parser, pSection, flagName);

	IMPL_SNPRNINTF(flagName, sizeof(flagName), pBaseFlag, EnumFunctions::Rank_ToStrings[(int)Rank::Veteran]);
	detail::read(this->Veteran, parser, pSection, flagName);

	IMPL_SNPRNINTF(flagName, sizeof(flagName), pBaseFlag, EnumFunctions::Rank_ToStrings[(int)Rank::Elite]);
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
	if (detail::read(placeholder, parser, pSection, flagName)) {
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
void NOINLINE Damageable<T>::Read(INI_EX& parser, const char* const pSection, const char* const pBaseFlag, const char* const pSingleFlag)
{
	// read the common flag, with the trailing dot being stripped
	char flagName[0x80];
	auto const pSingleFormat = pSingleFlag ? pSingleFlag : pBaseFlag;
	auto res = IMPL_SNPRNINTF(flagName, sizeof(flagName), pSingleFormat, Phobos::readDefval);

	if (res > 0 && flagName[res - 1] == '.')
		flagName[res - 1] = '\0';

	this->BaseValue.Read(parser, pSection, flagName);

	IMPL_SNPRNINTF(flagName, sizeof(flagName), pBaseFlag, EnumFunctions::HealthCondition_ToStrings[1]);
	this->ConditionYellow.Read(parser, pSection, flagName);

	IMPL_SNPRNINTF(flagName, sizeof(flagName), pBaseFlag, EnumFunctions::HealthCondition_ToStrings[2]);
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