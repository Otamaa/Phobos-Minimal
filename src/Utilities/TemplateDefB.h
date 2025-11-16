#pragma once

#include <New/Type/ColorTypeClass.h>

#include <ParticleSystemTypeClass.h>
#include <VoxelAnimTypeClass.h>

#include "TemplateDef.h"

#include <New/Type/PaletteManager.h>

struct FoundationDef
{
	std::string_view Name;
	CellStruct Size; // { X, Y }
	uint8_t CellCount;
	std::array<CellStruct, 30> Cells;
	uint8_t OutlineCount;
	std::array<CellStruct, 30> Outline;
};

OPTIONALINLINE COMPILETIMEEVAL std::array<FoundationDef, 4> CustomFoundations = { {
		// ====================================================
		// Foundation = 5x2
		//
		// Visual layout:
		//   · · · · · · ·
		//   · # # # # # ·
		//   · # # # # # ·
		//   · · · · · · ·
		// ====================================================
		{
			"5x2",
			{5, 2},
			10,
			{{
				{0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0},
				{0, 1}, {1, 1}, {2, 1}, {3, 1}, {4, 1}
			}},
			18,
			{{
				{-1, -1}, {0, -1}, {1, -1}, {2, -1}, {3, -1}, {4, -1}, {5, -1},
				{-1, 0},  {5, 0},
				{-1, 1},  {5, 1},
				{-1, 2},  {0, 2}, {1, 2}, {2, 2}, {3, 2}, {4, 2}, {5, 2}
			}}
		},

	// ====================================================
	// Foundation = 4x1
	//
	// Visual layout:
	//   · · · · · ·
	//   · # # # # ·
	//   · · · · · ·
	// ====================================================
	{
		"4x1",
		{4, 1},
		4,
		{{
			{0, 0}, {1, 0}, {2, 0}, {3, 0}
		}},
		14,
		{{
			{-1, -1}, {0, -1}, {1, -1}, {2, -1}, {3, -1}, {4, -1},
			{-1, 0},  {4, 0},
			{-1, 1},  {0, 1}, {1, 1}, {2, 1}, {3, 1}, {4, 1}
		}}
	},

	// ====================================================
	// Foundation = 2x4
	//
	// Visual layout:
	//   · · · ·
	//   · # # ·
	//   · # # ·
	//   · # # ·
	//   · # # ·
	//   · · · ·
	// ====================================================
	{
		"2x4",
		{2, 4},
		8,
		{{
			{0, 0}, {1, 0},
			{0, 1}, {1, 1},
			{0, 2}, {1, 2},
			{0, 3}, {1, 3}
		}},
		16,
		{{
			{-1, -1}, {0, -1}, {1, -1}, {2, -1},
			{-1, 0},  {2, 0},
			{-1, 1},  {2, 1},
			{-1, 2},  {2, 2},
			{-1, 3},  {2, 3},
			{-1, 4},  {0, 4}, {1, 4}, {2, 4}
		}}
	},

	// ====================================================
	// Foundation = 5x4
	//
	// Visual layout:
	//   · · · · · · ·
	//   · # # # # # ·
	//   · # # # # # ·
	//   · # # # # # ·
	//   · # # # # # ·
	//   · · · · · · ·
	// ====================================================
	{
		"5x4",
		{5, 4},
		20,
		{{
			{0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0},
			{0, 1}, {1, 1}, {2, 1}, {3, 1}, {4, 1},
			{0, 2}, {1, 2}, {2, 2}, {3, 2}, {4, 2},
			{0, 3}, {1, 3}, {2, 3}, {3, 3}, {4, 3}
		}},
		22,
		{{
			{-1, -1}, {0, -1}, {1, -1}, {2, -1}, {3, -1}, {4, -1}, {5, -1},
			{-1, 0},  {5, 0},
			{-1, 1},  {5, 1},
			{-1, 2},  {5, 2},
			{-1, 3},  {5, 3},
			{-1, 4},  {0, 4}, {1, 4}, {2, 4}, {3, 4}, {4, 4}, {5, 4}
		}}
	}
} };

OPTIONALINLINE const FoundationDef* FindFoundation(std::string_view name) noexcept
{
	for (const auto& def : CustomFoundations)
	{
		if (def.Name == name)
			return &def;
	}

	return nullptr;
}


namespace detail
{
	template <>
	OPTIONALINLINE bool read<Vector3D<float>>(Vector3D<float>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (!parser.Read3Float(pSection, pKey, (float*)&value))
		{
			if (!parser.empty()) {
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid 3 float Values");
			}

			return false;
		}

		return true;
	}

	template<>
	OPTIONALINLINE bool read<Foundation>(Foundation& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			for (size_t i = 0; i < TechnoTypeClass::BuildingFoundationName.size(); ++i) {
				if (IS_SAME_STR_(TechnoTypeClass::BuildingFoundationName[i].Name, parser.c_str())) {
					value = TechnoTypeClass::BuildingFoundationName[i].Value;
					return true;
				}
			}

			if (IS_SAME_STR_(parser.c_str(), "Custom")) {
				value = (Foundation)127;
				return true;
			}

			if (FindFoundation(parser.c_str())) {
				return true;
			}

			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected valid Foundation value");
		}

		return false;
	}

	template<>
	OPTIONALINLINE bool read<DoTypeFacing>(DoTypeFacing& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			for (size_t i = 0; i < EnumFunctions::FacingType_to_strings.size(); ++i) {
				if (IS_SAME_STR_(EnumFunctions::FacingType_to_strings[i], parser.c_str())) {
					value = DoTypeFacing(i);
					return true;
				}
			}

			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected valid DoTypeFacing value");
		}

		return false;
	}

	template <>
	OPTIONALINLINE bool read<AffectPlayerType>(AffectPlayerType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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
	OPTIONALINLINE bool read<SpotlightFlags>(SpotlightFlags& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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
	OPTIONALINLINE bool read<ColorStruct>(ColorStruct& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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
						nResult->BaseColor.ToColorStruct(&value);
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
	OPTIONALINLINE bool read<DamageDelayTargetFlag>(DamageDelayTargetFlag& value, INI_EX& parser, const char* pSection, const char* pKey, bool bAllocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			if(GameStrings::IsBlank(parser.value()))
				return false;

			for (size_t i = 0; i < EnumFunctions::DamageDelayTargetFlag_ToStrings.size(); ++i)
			{
				if (IS_SAME_STR_(parser.value(), EnumFunctions::DamageDelayTargetFlag_ToStrings[i]))
				{
					value = (DamageDelayTargetFlag)i;
					return true;
				}
			}

			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expect valid DamageDelayTargetFlag");
		}

		return false;
	}

	template <>
	OPTIONALINLINE bool read<NewCrateType>(NewCrateType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			for (size_t i = 0; i < EnumFunctions::NewCrateType_ToStrings.size(); ++i)
			{
				if (IS_SAME_STR_(parser.value(), EnumFunctions::NewCrateType_ToStrings[i]))
				{
					value = (NewCrateType)i;
					return true;
				}
			}

			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a target zone scan type");
		}

		return false;
	}

	template <>
	OPTIONALINLINE bool read<TargetZoneScanType>(TargetZoneScanType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			if(GameStrings::IsBlank(parser.value()))
				return false;

			for (size_t i = 0; i < EnumFunctions::TargetZoneScanType_ToStrings.size(); ++i)
			{
				if (IS_SAME_STR_(parser.value(), EnumFunctions::TargetZoneScanType_ToStrings[i]))
				{
					value = (TargetZoneScanType)i;
					return true;
				}
			}

			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a target zone scan type");
		}

		return false;
	}

	template <>
	OPTIONALINLINE bool read<MouseCursor>(MouseCursor& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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
	OPTIONALINLINE bool read<MouseCursorDataStruct>(MouseCursorDataStruct& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		auto ret = false;
		std::string _key(pKey);

		for (size_t i = 0; i < EnumFunctions::MouseCursorData_ToStrings.size(); ++i) {
			ret |= read(value.OriginalData.StartFrame, parser, pSection, (_key + EnumFunctions::MouseCursorData_ToStrings[i]).c_str());
		}

		if (value.SmallFrameRate == -1)
			value.SmallFrameRate = value.OriginalData.FrameRate;

		if (parser.ReadString(pSection, (_key + ".HotSpot").c_str()))
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
	OPTIONALINLINE bool read<FacingType>(FacingType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey) && detail::getresult(value,parser.value(), pSection , pKey)) {
			return true;
		}

		return false;
	}

	template <>
	OPTIONALINLINE bool read<DirType32>(DirType32& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		int nBuffer = -1;
		if (parser.ReadInteger(pSection, pKey, &nBuffer))
		{
			if(nBuffer >= (int)DirType32::Min && nBuffer <= (int)DirType32::Max){
				value = (DirType32)nBuffer;
				return true;
			}

			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a DirType32 between 0 and 32");
		}

		return false;
	}

	template <>
	OPTIONALINLINE bool read<DirType>(DirType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		int nBuffer = -1;
		if (parser.ReadInteger(pSection, pKey, &nBuffer))
		{
			const bool IsNegative = nBuffer < 0;
			const DirType nVal = (DirType)Math::abs(nBuffer);

			if(DirType::North <= nVal && nVal <= DirType::Max){
				value = (DirType)(!IsNegative ? (int)nVal : (int)DirType::Max + 1 - IsNegative);
				return true;
			}

			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a DirType between 0 and 255");
		}

		return false;
	}

	//template <>
	//OPTIONALINLINE bool read<FacingType>(FacingType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	//{
	//	int buffer;
	//
	//	if (parser.ReadInteger(pSection, pKey, &buffer))
	//	{
	//		if (buffer <= (int)FacingType::Count && buffer >= (int)FacingType::None)
	//		{
	//			value = static_cast<FacingType>(buffer);
	//			return true;
	//		}
	//
	//		Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid FacingType (0-7 or -1).");
	//	}
	//
	//	return false;
	//}

	template <>
	OPTIONALINLINE bool read<SpotlightAttachment>(SpotlightAttachment& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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

			Debug::INIParseFailed(pSection, pKey, parser.value(),"Expect valid SpotlightAttachment");
		}

		return false;
	}

	template <>
	OPTIONALINLINE bool read<ShowTimerType>(ShowTimerType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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

			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expect valid ShowTimerType");
		}

		return false;
	}

	template <>
	inline bool read<AttachmentYSortPosition>(AttachmentYSortPosition& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			for (size_t i = 0; i < EnumFunctions::AttachmentYSortPosition_ToStrings.size(); ++i) {
				if (IS_SAME_STR_(parser.value(), EnumFunctions::AttachmentYSortPosition_ToStrings[i].second.data())) {
					value = EnumFunctions::AttachmentYSortPosition_ToStrings[i].first;
					return true;
				}
			}

			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected an attachment YSort position");
		}
		return false;
	}

	template <>
	inline bool read<AffectedTechno>(AffectedTechno& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			char* context = nullptr;
			AffectedTechno resultData = AffectedTechno::None;

			for (auto cur = strtok_s(parser.value(), Phobos::readDelims, &context);
				cur;
				cur = strtok_s(nullptr, Phobos::readDelims, &context))
			{

				size_t result = 0;
				bool found = false;
				for (const auto& pStrings : EnumFunctions::AffectedTechno_ToStrings) {
					if (IS_SAME_STR_(cur, pStrings.second.data())) {
						found = true;
						break;
					}
					++result;
				}

				if (!found) {
					if (IS_SAME_STR_(cur, "vehicle")) {
						found = true;
						result = 2;
					}
				}

				if (!found) {
					Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a AffectedTechno");
					return false;
				}
				else
				{
					switch (result)
					{
					case 0: resultData |= AffectedTechno::None; break;
					case 1: resultData |= AffectedTechno::Infantry; break;
					case 2: resultData |= AffectedTechno::Unit; break;
					case 3: resultData |= AffectedTechno::Building; break;
					case 4: resultData |= AffectedTechno::Aircraft; break;
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
	inline bool read<DisplayShowType>(DisplayShowType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			char* context = nullptr;
			DisplayShowType resultData = DisplayShowType::None;

			for (auto cur = strtok_s(parser.value(), Phobos::readDelims, &context);
				cur;
				cur = strtok_s(nullptr, Phobos::readDelims, &context))
			{

				size_t result = 0;
				bool found = false;
				for (const auto& pStrings : EnumFunctions::DisplayShowType_ToStrings) {
					if (IS_SAME_STR_(cur, pStrings)) {
						found = true;
						break;
					}
					++result;
				}

				if (!found) {
					Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a DisplayShowType");
					return false;
				}
				else
				{
					switch (result)
					{
					case 0: resultData |= DisplayShowType::None; break;
					case 1: resultData |= DisplayShowType::CursorHover; break;
					case 2: resultData |= DisplayShowType::Idle; break;
					case 3: resultData |= DisplayShowType::Selected; break;
					case 4: resultData |= DisplayShowType::Select; break;
					case 5: resultData |= DisplayShowType::All; break;
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
	OPTIONALINLINE bool read<BountyValueOption>(BountyValueOption& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			const auto pVal = parser.value();

			for (size_t i = 0; i < EnumFunctions::BountyValueOption_ToStrings.size(); ++i) {
				if (IS_SAME_STR_(pVal, EnumFunctions::BountyValueOption_ToStrings[i])) {
					value = (BountyValueOption)i;
					return true;
				}
			}

			Debug::INIParseFailed(pSection, pKey, pVal, "Expect valid BountyValueOption");
		}

		return false;
	}

	template <>
	OPTIONALINLINE bool read<BuildingSelectBracketPosition>(BuildingSelectBracketPosition& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			const auto str = parser.value();

			for (size_t i = 0; i < EnumFunctions::BuildingSelectBracketPosition_ToStrings.size(); ++i) {
				if (IS_SAME_STR_(str, EnumFunctions::BuildingSelectBracketPosition_ToStrings[i])) {
					value = BuildingSelectBracketPosition(i);
					return true;
				}
			}

			Debug::INIParseFailed(pSection, pKey, str, "Expect valid BuildingSelectBracketPosition");
		}

		return false;
	}

	template <>
	OPTIONALINLINE bool read<DisplayInfoType>(DisplayInfoType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			auto str = parser.value();

			for (size_t i = 0; i < EnumFunctions::DisplayInfoType_ToStrings.size(); ++i) {
				if (IS_SAME_STR_(str, EnumFunctions::DisplayInfoType_ToStrings[i])) {
					value = DisplayInfoType(i);
					return true;
				}
			}

			Debug::INIParseFailed(pSection, pKey, str, "Expect valid DisplayInfoType");
		}

		return false;
	}

	template <>
	OPTIONALINLINE bool read<InterpolationMode>(InterpolationMode& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocat)
	{
		if (parser.ReadString(pSection, pKey))
		{
			auto str = parser.value();

			for (auto& [pString, val] : EnumFunctions::InterpolationMode_ToStrings) {
				if (IS_SAME_STR_(pString, str)) {
					value = val;
					return true;
				}
			}

			Debug::INIParseFailed(pSection, pKey, str, "Expected an interpolation mode");
		}

		return false;
	}

	template<typename T, bool Alloc = false>
	OPTIONALINLINE void ParseVector(INI_EX& IniEx, std::vector<std::vector<T>>& nVecDest, const char* pSection, bool bDebug = true, bool bVerbose = false, const char* Delims = Phobos::readDelims, const char* message = nullptr)
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
				auto res = PhobosCRT::trim(cur);

				T buffer = nullptr;
				if COMPILETIMEEVAL (!Alloc)
					buffer = baseType::Find(res.c_str());
				else
					buffer = baseType::FindOrAllocate(res.c_str());

				if (bVerbose)
					Debug::LogInfo("ParseVector DEBUG: [{}][{}]: Verose parsing [{}]", pSection, i, res.c_str());

				if (buffer) {
					nVecDest[i].push_back(buffer);
				}else if (bDebug && !GameStrings::IsBlank(cur)) {
					Debug::LogInfo("ParseVector DEBUG: [{}][{}]: Error parsing [{}]", pSection, i, res.c_str());
				}
			}
		}
	}

	OPTIONALINLINE void ParseVector(INI_EX& IniEx, std::vector<std::vector<std::string>>& nVecDest, const char* pSection, bool bDebug = true, bool bVerbose = false, const char* Delims = Phobos::readDelims, const char* message = nullptr)
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
					nVecDest[i].emplace_back(res);

				if (bVerbose)
					Debug::LogInfo("ParseVector DEBUG: [{}][{}]: Verose parsing [{}]", pSection, i, res.c_str());
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
	OPTIONALINLINE void ParseVector(DynamicVectorClass<T>& List, INI_EX& IniEx , const char* section, const char* key , const char* message = nullptr)
	{
		if (IniEx.ReadString(section,key))
		{
			List.reset();
			char* context = nullptr;

			if COMPILETIMEEVAL (std::is_pointer<T>())
			{
				using BaseType = std::remove_pointer_t<T>;

				for (char* cur = strtok_s(IniEx.value(), Phobos::readDelims, &context); cur;
					 cur = strtok_s(nullptr, Phobos::readDelims, &context))
				{
					BaseType* buffer = nullptr;
					if COMPILETIMEEVAL (Allocate)
					{
						buffer = BaseType::FindOrAllocate(cur);
					}
					else
					{
						buffer = BaseType::Find(cur);
					}

					if (buffer)
					{
						if COMPILETIMEEVAL (!Unique) {
							List.push_back(buffer);
						} else {
							List.insert_unique(buffer);
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
						if COMPILETIMEEVAL (!Unique)
						{
							List.push_back(buffer);
						}
						else
						{
							List.insert_unique(buffer);
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