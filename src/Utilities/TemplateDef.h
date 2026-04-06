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
#include <Utilities/EnumFunctions.h>
#include <Utilities/PhobosFixedString.h>

//#include <New/Type/PaletteManager.h>

#include <array>
#include <iostream>
#include <string_view>

template<typename T>
struct IndexFinder
{
	static OPTIONALINLINE bool getindex(int& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate = false)
	{
		if (parser.ReadString(pSection, pKey))
		{
			const char* val = parser.value();

			if (GameStrings::IsNone(val))
			{
				value = -1;
				return true;
			}

			int idx = value;

			if COMPILETIMEEVAL(std::is_pointer<T>::value)
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

	template <typename T>
	bool getresult(T& value, const std::string& parser, const char* pSection, const char* pKey, bool allocate = false)
	{
		//_assert(true, "Not Implemented!");
		return true;
	}

	template<>
	bool getresult<AffectedHouse>(AffectedHouse& value, const std::string& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool getresult<TechnoTypeConvertData>(TechnoTypeConvertData& value, const std::string& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool getresult<TileType>(TileType& value, const std::string& parser, const char* pSection, const char* pKey, bool bAllocate);

	template <>
	bool getresult<LandType>(LandType& value, const std::string& parser, const char* pSection, const char* pKey, bool bAllocate);

	template <>
	bool getresult<PhobosAbilityType>(PhobosAbilityType& value, const std::string& parser, const char* pSection, const char* pKey, bool bAllocate);

	template <>
	bool getresult<Rank>(Rank& value, const std::string& parser, const char* pSection, const char* pKey, bool bAllocate);

	template <>
	bool getresult<FacingType>(FacingType& value, const std::string& parser, const char* pSection, const char* pKey, bool bAllocate);


	template <typename T>
	OPTIONALINLINE bool read(T& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate = false)
	{
		if (parser.ReadString(pSection, pKey))
		{
			using base_type = std::remove_pointer_t<T>;
			const auto pValue = parser.value();
			auto const parsed = (allocate ? base_type::FindOrAllocate : base_type::Find)(pValue);

			if (parsed || GameStrings::IsNone(pValue))
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
	bool read<bool>(bool& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<int>(int& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<unsigned int>(unsigned int& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<unsigned short>(unsigned short& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<short>(short& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<BYTE>(BYTE& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<float>(float& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<double>(double& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<TechnoTypeClass*>(TechnoTypeClass*& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<SHPStruct*>(SHPStruct*& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<std::string>(std::string& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<PartialVector2D<int>>(PartialVector2D<int>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<PartialVector2D<double>>(PartialVector2D<double>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<PartialVector3D<int>>(PartialVector3D<int>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<ReversePartialVector3D<int>>(ReversePartialVector3D<int>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<PartialVector3D<double>>(PartialVector3D<double>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<PartialVector3D<float>>(PartialVector3D<float>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<CellStruct>(CellStruct& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<Point2D>(Point2D& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<MinMaxValue<int>>(MinMaxValue<int>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<MinMaxValue<float>>(MinMaxValue<float>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<MinMaxValue<double>>(MinMaxValue<double>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<Vector2D<int>>(Vector2D<int>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<Point3D>(Point3D& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<RectangleStruct>(RectangleStruct& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<Point2DBYTE>(Point2DBYTE& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<CoordStruct>(CoordStruct& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<Vector3D<BYTE>>(Vector3D<BYTE>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<Vector3D<int>>(Vector3D<int>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<Vector2D<double>>(Vector2D<double>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<CSFText>(CSFText& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<RocketStruct>(RocketStruct& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<Leptons>(Leptons& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<SWRange>(SWRange& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<PassiveAcquireModes>(PassiveAcquireModes& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<LaserTrailDrawType>(LaserTrailDrawType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<AttachedAnimPosition>(AttachedAnimPosition& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<Rank>(Rank& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<Armor>(Armor& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<Edge>(Edge& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<TranslucencyLevel>(TranslucencyLevel& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<HorizontalPosition>(HorizontalPosition& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<BannerNumberType>(BannerNumberType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<VerticalPosition>(VerticalPosition& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<SelfHealGainType>(SelfHealGainType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<SlaveReturnTo>(SlaveReturnTo& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<KillMethod>(KillMethod& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<IronCurtainFlag>(IronCurtainFlag& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<OwnerHouseKind>(OwnerHouseKind& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<Mission>(Mission& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<SuperWeaponAITargetingMode>(SuperWeaponAITargetingMode& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<AffectedTarget>(AffectedTarget& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<ChronoSparkleDisplayPosition>(ChronoSparkleDisplayPosition& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<SuperWeaponTarget>(SuperWeaponTarget& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<TargetingConstraints>(TargetingConstraints& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<TargetingPreference>(TargetingPreference& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<DiscardCondition>(DiscardCondition& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<ExpireWeaponCondition>(ExpireWeaponCondition& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<LandType>(LandType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<AffectedHouse>(AffectedHouse& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<AttachedAnimFlag>(AttachedAnimFlag& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<AreaFireTarget>(AreaFireTarget& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<TextAlign>(TextAlign& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<Layer>(Layer& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<AbstractType>(AbstractType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<Locomotors>(Locomotors& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read(TileType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<LandTypeFlags>(LandTypeFlags& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<TypeList<int>>(TypeList<int>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<Vector3D<float>>(Vector3D<float>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template<>
	bool read<Foundation>(Foundation& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template<>
	bool read<DoTypeFacing>(DoTypeFacing& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<AffectPlayerType>(AffectPlayerType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<AffectedVeterancy>(AffectedVeterancy& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<SpotlightFlags>(SpotlightFlags& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<ColorStruct>(ColorStruct& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<DamageDelayTargetFlag>(DamageDelayTargetFlag& value, INI_EX& parser, const char* pSection, const char* pKey, bool bAllocate);

	template <>
	bool read<NewCrateType>(NewCrateType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<TargetZoneScanType>(TargetZoneScanType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<MouseCursor>(MouseCursor& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<MouseCursorDataStruct>(MouseCursorDataStruct& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<FacingType>(FacingType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<DirType32>(DirType32& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<DirType>(DirType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<SpotlightAttachment>(SpotlightAttachment& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<ShowTimerType>(ShowTimerType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<AttachmentYSortPosition>(AttachmentYSortPosition& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<AffectedTechno>(AffectedTechno& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<DisplayShowType>(DisplayShowType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<BountyValueOption>(BountyValueOption& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<BuildingSelectBracketPosition>(BuildingSelectBracketPosition& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<DisplayInfoType>(DisplayInfoType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<InterpolationMode>(InterpolationMode& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocat);

	template <>
	bool read<ParabolaFireMode>(ParabolaFireMode& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	bool read<CLSID>(CLSID& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <typename T, bool clearvec = true>
	OPTIONALINLINE void parse_values(std::vector<T>& vector, INI_EX& parser, const char* pSection, const char* pKey, bool allocate = false)
	{
		if COMPILETIMEEVAL(clearvec)
			vector.clear();

		char* context = nullptr;
		for (auto pCur = strtok_s(parser.value(), Phobos::readDelims, &context);
				pCur;
				pCur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			auto buffer = T();

			if (Parser<T>::Parse(pCur, &buffer))
				vector.push_back(std::move(buffer));
			else if (!allocate && !GameStrings::IsNone(pCur))
				Debug::INIParseFailed(pSection, pKey, pCur, nullptr);
		}
	}

	template <typename T>
	OPTIONALINLINE void ReadVectors(std::vector<T>& vector, INI_EX& parser, const char* pSection, const char* pKey, bool allocate = false)
	{
		//_assert(std::is_pointer<T>::value, "Pointer Required !");

		if (parser.ReadString(pSection, pKey))
		{
			detail::parse_values(vector, parser, pSection, pKey, allocate);
		}
	}

	//WARNING : this not checking for read first , make sure before using it !
	template <typename T>
	OPTIONALINLINE void parse_Alloc_values(std::vector<T>& vector, INI_EX& parser, const char* pSection, const char* pKey, bool allocate = false)
	{
		_assert(std::is_pointer<T>::value, "Pointer Required !");
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
			else if (!GameStrings::IsNone(pCur))
				Debug::INIParseFailed(pSection, pKey, pCur, nullptr);
		}
	}

	template <typename T>
	OPTIONALINLINE void ReadVectorsAlloc(std::vector<T>& vector, INI_EX& parser, const char* pSection, const char* pKey, bool allocate = false)
	{
		_assert(std::is_pointer<T>::value, "Pointer Required !");

		if (parser.ReadString(pSection, pKey))
		{
			detail::parse_Alloc_values(vector, parser, pSection, pKey, allocate);
		}
	}

	template <>
	void parse_values<LandType>(std::vector<LandType>& vector, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	void parse_values<FacingType>(std::vector<FacingType>& vector, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	void parse_values<PhobosAbilityType>(std::vector<PhobosAbilityType>& vector, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	void parse_values<TechnoTypeConvertData>(std::vector<TechnoTypeConvertData>& vector, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	void parse_values<TileType>(std::vector<TileType>& vector, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	void parse_values<Mission>(std::vector<Mission>& vector, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

	template <>
	void parse_values<Rank>(std::vector<Rank>& vector, INI_EX& parser, const char* pSection, const char* pKey, bool allocate);

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
			if COMPILETIMEEVAL(std::is_pointer<Lookuper>::value)
			{
				using base_type = std::remove_pointer_t<Lookuper>;
				idx = base_type::FindIndexById(pCur);
			}
			else { idx = Lookuper::FindIndexById(pCur); }

			if (idx != -1 || GameStrings::IsNone(pCur))
			{
				vector.push_back(idx);
			}
			else
			{
				Debug::INIParseFailed(pSection, pKey, pCur);
			}
		}
	}

	void ParseVector(INI_EX& IniEx, std::vector<std::vector<std::string>>& nVecDest, const char* pSection, bool bDebug = true, bool bVerbose = false, const char* Delims = Phobos::readDelims, const char* message = nullptr);

	template<typename T, bool Alloc = false>
	OPTIONALINLINE void ParseVector(INI_EX& IniEx, std::vector<std::vector<T>>& nVecDest, const char* pSection, bool bDebug = true, bool bVerbose = false, const char* Delims = Phobos::readDelims, const char* message = nullptr)
	{
		//_assert(std::is_pointer<T>::value, "Pointer Required !");
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
				auto res = PhobosCRT::trim(cur);

				T buffer = nullptr;
				if COMPILETIMEEVAL(!Alloc)
					buffer = baseType::Find(res.c_str());
				else
					buffer = baseType::FindOrAllocate(res.c_str());

				if (bVerbose)
					Debug::LogInfo("ParseVector DEBUG: [{}][{}]: Verose parsing [{}]", pSection, i, res.c_str());

				if (buffer)
				{
					nVecDest[i].push_back(buffer);
				}
				else if (bDebug && !GameStrings::IsNone(cur))
				{
					Debug::LogInfo("ParseVector DEBUG: [{}][{}]: Error parsing [{}]", pSection, i, res.c_str());
				}
			}
		}
	}

	template<size_t count>
	OPTIONALINLINE void ParseVector(INI_EX& IniEx, std::vector<std::vector<PhobosFixedString<count>>>& nVecDest, const char* pSection, bool bDebug = true, bool bVerbose = false, const char* Delims = Phobos::readDelims, const char* message = nullptr)
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
				auto res = PhobosCRT::trim(cur);

				if (!res.empty())
					nVecDest[i].emplace_back(res.c_str());

				if (bVerbose)
					Debug::LogInfo("ParseVector DEBUG: [{}][{}]: Verose parsing [{}]", pSection, i, res.c_str());
			}
		}
	}

	template<typename T, bool Allocate = false, bool Unique = false>
	OPTIONALINLINE void ParseVector(DynamicVectorClass<T>& List, INI_EX& IniEx, const char* section, const char* key, const char* message = nullptr)
	{
		if (IniEx.ReadString(section, key))
		{
			List.reset();
			char* context = nullptr;

			if COMPILETIMEEVAL(std::is_pointer<T>())
			{
				using BaseType = std::remove_pointer_t<T>;

				for (char* cur = strtok_s(IniEx.value(), Phobos::readDelims, &context); cur;
					 cur = strtok_s(nullptr, Phobos::readDelims, &context))
				{
					BaseType* buffer = nullptr;
					if COMPILETIMEEVAL(Allocate)
					{
						buffer = BaseType::FindOrAllocate(cur);
					}
					else
					{
						buffer = BaseType::Find(cur);
					}

					if (buffer)
					{
						if COMPILETIMEEVAL(!Unique)
						{
							List.push_back(buffer);
						}
						else
						{
							List.insert_unique(buffer);
						}
					}
					else if (!GameStrings::IsNone(cur))
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
						if COMPILETIMEEVAL(!Unique)
						{
							List.push_back(buffer);
						}
						else
						{
							List.insert_unique(buffer);
						}
					}
					else if (!GameStrings::IsNone(cur))
					{
						Debug::INIParseFailed(section, key, cur, message);
					}
				}
			}
		}
	};


	template <typename T>
	OPTIONALINLINE T interpolate(T& first, T& second, double percentage, InterpolationMode mode)
	{
		return first;
	}

	template <>
	OPTIONALINLINE double interpolate<double>(double& first, double& second, double percentage, InterpolationMode mode)
	{
		double result = first;

		switch (mode)
		{
		case InterpolationMode::Linear:
			result = first + ((second - first) * percentage);
			break;
		default:
			break;
		}

		return result;
	}

	template <>
	OPTIONALINLINE int interpolate<int>(int& first, int& second, double percentage, InterpolationMode mode)
	{
		double firstValue = first;
		double secondValue = second;
		return (int)interpolate(firstValue, secondValue, percentage, mode);
	}

	template <>
	OPTIONALINLINE BYTE interpolate<BYTE>(BYTE& first, BYTE& second, double percentage, InterpolationMode mode)
	{
		double firstValue = first;
		double secondValue = second;
		return (BYTE)interpolate(firstValue, secondValue, percentage, mode);
	}

	template <>
	OPTIONALINLINE ColorStruct interpolate<ColorStruct>(ColorStruct& first, ColorStruct& second, double percentage, InterpolationMode mode)
	{
		BYTE r = interpolate(first.R, second.R, percentage, mode);
		BYTE g = interpolate(first.G, second.G, percentage, mode);
		BYTE b = interpolate(first.B, second.B, percentage, mode);
		return { r, g, b };
	}

	template <>
	OPTIONALINLINE TranslucencyLevel interpolate<TranslucencyLevel>(TranslucencyLevel& first, TranslucencyLevel& second, double percentage, InterpolationMode mode)
	{
		double firstValue = first.GetIntValue();
		double secondValue = second.GetIntValue();
		int value = (int)interpolate(firstValue, secondValue, percentage, mode);
		return { value };
	}

	template <typename T>
	OPTIONALINLINE bool getindex(int& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate = false)
	{
		return IndexFinder<T>::getindex(value, parser, pSection, pKey, allocate);
	}
};


// Valueable
template <typename T>
void OPTIONALINLINE Valueable<T>::Read(INI_EX& parser, const char* pSection, const char* pKey, bool Allocate)
{
	detail::read(this->Value, parser, pSection, pKey, Allocate);
}

template <typename T>
bool OPTIONALINLINE Valueable<T>::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Stm.Process(this->Value, RegisterForChange);
}

template <typename T>
bool OPTIONALINLINE Valueable<T>::Save(PhobosStreamWriter& Stm) const
{
	return Stm.Process(this->Value);
}

// ValueableIdx
template <typename Lookuper>
void OPTIONALINLINE ValueableIdx<Lookuper>::Read(INI_EX& parser, const char* pSection, const char* pKey)
{
	detail::getindex<Lookuper>(this->Value, parser, pSection, pKey);
}

// Nullable
template <typename T>
void OPTIONALINLINE Nullable<T>::Read(INI_EX& parser, const char* pSection, const char* pKey, bool Allocate)
{
	if (detail::read(this->Value, parser, pSection, pKey, Allocate))
	{
		const char* val = parser.value();

		if (strlen(val) != 0)
		{
			if (IS_SAME_STR_(val, DEFAULT_STR2))
			{
				this->Reset();
			}
			else
			{
				this->HasValue = true;
			}
		}
	}
}

template <typename T>
bool OPTIONALINLINE Nullable<T>::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->Reset();
	if (!Stm.Process(this->HasValue))
		return false;

	if (this->HasValue)
	{
		return Stm.Process(this->Value, RegisterForChange);
	}

	return true;
}

template <typename T>
bool OPTIONALINLINE Nullable<T>::Save(PhobosStreamWriter& Stm) const
{
	if (!Stm.Process(this->HasValue))
		return false;

	if (this->HasValue)
	{
		return Stm.Process(this->Value);
	}

	return true;
}

// NullableIdx
template <typename Lookuper, EnumCheckMode mode>
void OPTIONALINLINE NullableIdx<Lookuper, mode>::Read(INI_EX& parser, const char* pSection, const char* pKey)
{
	if COMPILETIMEEVAL(mode == EnumCheckMode::originalbehaviour)
	{
		if (detail::getindex<Lookuper>(this->Value, parser, pSection, pKey))
			this->HasValue = true;
	}
	else
	{
		if (parser.ReadString(pSection, pKey))
		{
			const char* val = parser.value();

			if COMPILETIMEEVAL(mode != EnumCheckMode::disable)
			{
				if (GameStrings::IsNone(val))
				{
					this->Value = -1;
					this->HasValue = true;
					return;
				}
			}

			int idx = this->Value;

			if COMPILETIMEEVAL(std::is_pointer<Lookuper>::value)
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
void OPTIONALINLINE Promotable<T>::Read(INI_EX& parser, const char* const pSection, const char* const pBaseFlag, const char* const pSingleFlag, bool allocate)
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
	if (detail::read<T>(placeholder, parser, pSection, flagbuffer, allocate))
	{
		this->SetAll(placeholder);
	}

	// read specific flags
	IMPL_SNPRNINTF(flagbuffer, sizeof(flagbuffer), pBaseFlag, EnumFunctions::Rank_ToStrings[(int)Rank::Rookie + 1].second.data());
	detail::read<T>(this->Rookie, parser, pSection, flagbuffer, allocate);

	IMPL_SNPRNINTF(flagbuffer, sizeof(flagbuffer), pBaseFlag, EnumFunctions::Rank_ToStrings[(int)Rank::Veteran + 1].second.data());
	detail::read<T>(this->Veteran, parser, pSection, flagbuffer, allocate);

	IMPL_SNPRNINTF(flagbuffer, sizeof(flagbuffer), pBaseFlag, EnumFunctions::Rank_ToStrings[(int)Rank::Elite + 1].second.data());
	detail::read<T>(this->Elite, parser, pSection, flagbuffer, allocate);
};

template <typename T>
bool OPTIONALINLINE Promotable<T>::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Stm
		.Process(this->Rookie, RegisterForChange)
		.Process(this->Veteran, RegisterForChange)
		.Process(this->Elite, RegisterForChange)
		;
}

template <typename T>
bool OPTIONALINLINE Promotable<T>::Save(PhobosStreamWriter& Stm) const
{
	return Stm
		.Process(this->Rookie)
		.Process(this->Veteran)
		.Process(this->Elite)
		;
}

// NullablePromotable
template <typename T>
void OPTIONALINLINE NullablePromotable<T>::Read(INI_EX& parser, const char* const pSection, const char* const pBaseFlag, const char* const pSingleFlag)
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
	IMPL_SNPRNINTF(flagbuffer, sizeof(flagbuffer), pBaseFlag, EnumFunctions::Rank_ToStrings[(int)Rank::Rookie + 1].second.data());
	this->Rookie.Read(parser, pSection, flagbuffer);

	IMPL_SNPRNINTF(flagbuffer, sizeof(flagbuffer), pBaseFlag, EnumFunctions::Rank_ToStrings[(int)Rank::Veteran + 1].second.data());
	this->Veteran.Read(parser, pSection, flagbuffer);

	IMPL_SNPRNINTF(flagbuffer, sizeof(flagbuffer), pBaseFlag, EnumFunctions::Rank_ToStrings[(int)Rank::Elite + 1].second.data());
	this->Elite.Read(parser, pSection, flagbuffer);
};

template <typename T>
bool OPTIONALINLINE NullablePromotable<T>::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Stm
		.Process(this->Rookie, RegisterForChange)
		.Process(this->Veteran, RegisterForChange)
		.Process(this->Elite, RegisterForChange);
}

template <typename T>
bool OPTIONALINLINE NullablePromotable<T>::Save(PhobosStreamWriter& Stm) const
{
	return Stm
		.Process(this->Rookie)
		.Process(this->Veteran)
		.Process(this->Elite);
}

// ValueableVector
template <typename T>
void OPTIONALINLINE ValueableVector<T>::Read(INI_EX& parser, const char* pSection, const char* pKey, bool bAllocate)
{
	if (parser.ReadString(pSection, pKey))
	{
		detail::parse_values<T>(*this, parser, pSection, pKey, bAllocate);
	}
}

template <>
void OPTIONALINLINE ValueableVector<std::string>::Read(INI_EX& parser, const char* pSection, const char* pKey, bool bAllocate)
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
bool OPTIONALINLINE ValueableVector<T>::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Stm.Process(this->AsVector());
}

template <typename T>
bool OPTIONALINLINE ValueableVector<T>::Save(PhobosStreamWriter& Stm) const
{
	return Stm.Process(this->AsVector());
}

template <>
bool OPTIONALINLINE ValueableVector<bool>::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Stm.Process(this->AsVector());
}

template <>
bool OPTIONALINLINE ValueableVector<bool>::Save(PhobosStreamWriter& Stm) const
{
	return Stm.Process(this->AsVector());
}

// NullableVector
template <typename T>
void OPTIONALINLINE NullableVector<T>::Read(INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (parser.ReadString(pSection, pKey))
	{
		auto const non_default = !IS_SAME_STR_(parser.value(), DEFAULT_STR2);
		this->hasValue = non_default;

		if (non_default)
		{
			detail::parse_values<T>(*this, parser, pSection, pKey, allocate);
		}
	}
}

template <typename T>
bool OPTIONALINLINE NullableVector<T>::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->clear();
	if (Stm.Process(this->hasValue, RegisterForChange))
	{
		return !this->hasValue || ValueableVector<T>::Load(Stm, RegisterForChange);
	}
	return false;
}

template <typename T>
bool OPTIONALINLINE NullableVector<T>::Save(PhobosStreamWriter& Stm) const
{
	if (Stm.Process(this->hasValue))
	{
		return !this->hasValue || ValueableVector<T>::Save(Stm);
	}

	return false;
}

// ValueableIdxVector
template <typename Lookuper>
void OPTIONALINLINE ValueableIdxVector<Lookuper>::Read(INI_EX& parser, const char* pSection, const char* pKey)
{
	if (parser.ReadString(pSection, pKey))
	{
		this->clear();
		detail::parse_indexes<Lookuper>(*this, parser, pSection, pKey);
	}
}

// NullableIdxVector
template <typename Lookuper>
void OPTIONALINLINE NullableIdxVector<Lookuper>::Read(INI_EX& parser, const char* pSection, const char* pKey)
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
void OPTIONALINLINE Damageable<T>::Read(INI_EX& parser, const char* const pSection, const char* const pBaseFlag, const char* const pSingleFlag, bool Alloc)
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

	this->BaseValue.Read(parser, pSection, flagName, Alloc);

	IMPL_SNPRNINTF(flagName, sizeof(flagName), pBaseFlag, EnumFunctions::HealthCondition_ToStrings[1].second.data());
	this->ConditionYellow.Read(parser, pSection, flagName, Alloc);

	IMPL_SNPRNINTF(flagName, sizeof(flagName), pBaseFlag, EnumFunctions::HealthCondition_ToStrings[2].second.data());
	this->ConditionRed.Read(parser, pSection, flagName, Alloc);
};

template <typename T>
bool OPTIONALINLINE Damageable<T>::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Stm
		.Process(this->BaseValue, RegisterForChange)
		.Process(this->ConditionYellow, RegisterForChange)
		.Process(this->ConditionRed, RegisterForChange);
}

template <typename T>
bool OPTIONALINLINE Damageable<T>::Save(PhobosStreamWriter& Stm) const
{
	return Stm
		.Process(this->BaseValue)
		.Process(this->ConditionYellow)
		.Process(this->ConditionRed);
}

bool OPTIONALINLINE HealthOnFireData::Read(INI_EX& parser, const char* pSection, const char* pKey)
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

bool OPTIONALINLINE HealthOnFireData::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Stm
		.Process(this->RedOnFire, RegisterForChange)
		.Process(this->GreenOnFire, RegisterForChange)
		.Process(this->YellowOnFire, RegisterForChange);
}

bool OPTIONALINLINE HealthOnFireData::Save(PhobosStreamWriter& Stm) const
{
	return Stm
		.Process(this->RedOnFire)
		.Process(this->GreenOnFire)
		.Process(this->YellowOnFire);
}

// DamageableVector
template <typename T>
void OPTIONALINLINE DamageableVector<T>::Read(INI_EX& parser, const char* const pSection, const char* const pBaseFlag, const char* const pSingleFlag)
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

	IMPL_SNPRNINTF(flagName, sizeof(flagName), pBaseFlag, EnumFunctions::HealthCondition_ToStrings[1].second.data());
	this->ConditionYellow.Read(parser, pSection, flagName);

	IMPL_SNPRNINTF(flagName, sizeof(flagName), pBaseFlag, EnumFunctions::HealthCondition_ToStrings[2].second.data());
	this->ConditionRed.Read(parser, pSection, flagName);

	IMPL_SNPRNINTF(flagName, sizeof(flagName), pBaseFlag, "MaxValue");
	this->MaxValue.Read(parser, pSection, flagName);
};

template <typename T>
bool OPTIONALINLINE DamageableVector<T>::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Stm
		.Process(this->BaseValue, RegisterForChange)
		.Process(this->ConditionYellow, RegisterForChange)
		.Process(this->ConditionRed, RegisterForChange)
		.Process(this->MaxValue, RegisterForChange);
}

template <typename T>
bool OPTIONALINLINE DamageableVector<T>::Save(PhobosStreamWriter& Stm) const
{
	return Stm
		.Process(this->BaseValue)
		.Process(this->ConditionYellow)
		.Process(this->ConditionRed)
		.Process(this->MaxValue);
}

// TimedWarheadEffect
template <typename T>
bool OPTIONALINLINE TimedWarheadValue<T>::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Stm
		.Process(this->Value, RegisterForChange)
		.Process(this->Timer, RegisterForChange)
		.Process(this->ApplyToHouses, RegisterForChange)
		.Process(this->SourceWarhead, RegisterForChange);
}

template <typename T>
bool OPTIONALINLINE TimedWarheadValue<T>::Save(PhobosStreamWriter& Stm) const
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
void OPTIONALINLINE MultiflagValueableVector<T, TExtraArgs...>::Read(INI_EX& parser, const char* const pSection, const char* const pBaseFlag, TExtraArgs&... extraArgs)
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
void OPTIONALINLINE MultiflagNullableVector<T, TExtraArgs...>::Read(INI_EX& parser, const char* const pSection, const char* const pBaseFlag, TExtraArgs&... extraArgs)
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
bool OPTIONALINLINE Animatable<TValue>::KeyframeDataEntry::Read(INI_EX& parser, const char* const pSection, const char* const pBaseFlag, absolute_length_t absoluteLength)
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
bool OPTIONALINLINE Animatable<TValue>::KeyframeDataEntry::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Stm
		.Process(this->Percentage, RegisterForChange)
		.Process(this->Value, RegisterForChange);
}

template <typename TValue>
bool OPTIONALINLINE Animatable<TValue>::KeyframeDataEntry::Save(PhobosStreamWriter& Stm) const
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
void OPTIONALINLINE Animatable<TValue>::Read(INI_EX& parser, const char* const pSection, const char* const pBaseFlag, absolute_length_t absoluteLength)
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
bool OPTIONALINLINE Animatable<TValue>::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Stm.Process(this->KeyframeData, RegisterForChange);
}

template <typename TValue>
bool OPTIONALINLINE Animatable<TValue>::Save(PhobosStreamWriter& Stm) const
{
	return Stm.Process(this->KeyframeData);
}