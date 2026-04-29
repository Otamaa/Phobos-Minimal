#include "TemplateDefB.h"

#include <atlstr.h>

//#include <New/Interfaces/AttachmentLocomotionClass.h>
#include <New/Interfaces/LevitateLocomotionClass.h>
#include <New/Interfaces/AdvancedDriveLocomotionClass.h>
#include <New/Interfaces/CustomRocketLocomotionClass.h>
#include <New/Interfaces/TSJumpJetLocomotionClass.h>
#include <New/Interfaces/ShiftLocomotionClass.h>

#define PARSE(who)\
if (IS_SAME_STR_(parser.value(), who ##_data.s_name)) { \
	CLSID who ##_dummy; \
	std::wstring who ##_dummy_clsid = ## who ##_data.w_CLSID; \
	## who ##_dummy_clsid.insert(0, L"{"); \
	## who ##_dummy_clsid.insert(who ##_dummy_clsid.size(), L"}"); \
	const unsigned hr = CLSIDFromString(LPCOLESTR(who ##_dummy_clsid.data()), &who ##_dummy); \
	if (SUCCEEDED(hr)) {\
		value = ## who ##_dummy; return true;\
	} else {\
		Debug::LogError("Cannot find Locomotor [{} - {}({})] err : 0x{:08X}", pSection, parser.value(), PhobosCRT::WideStringToString(## who ##_dummy_clsid), hr);\
	}\
}

template<>
bool detail::getresult<AffectedHouse>(AffectedHouse& value, const std::string& parser, const char* pSection, const char* pKey, bool allocate)
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
				Debug::INIParseFailed(pSection, pKey, pCur, "Expected a affected house");
			}
		}

		value = resultData;
		return true;
	}
	return false;
}

template <>
bool detail::getresult<TechnoTypeConvertData>(TechnoTypeConvertData& value, const std::string& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (!parser.empty())
	{

		std::string copy = parser;
		std::erase(copy, ' ');

		const auto nDelim = copy.find(":");
		if (nDelim == std::string::npos)
			return false;

		auto nFirst = copy.substr(0, nDelim);
		//second countais b:c
		auto nSecondPair = copy.substr(nDelim + 1);
		const auto nDelim2 = nSecondPair.find(":");

		if (nDelim2 != std::string::npos)
		{
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
				else if (!allocate && !GameStrings::IsNone(pCur))
					Debug::INIParseFailed(pSection, pKey, pCur, nullptr);
			}

			Parser<TechnoTypeClass*>::Parse(nSecondPair_1.c_str(), &value.To);
			detail::getresult<AffectedHouse>(value.Eligible, nSecondPair_2, pSection, pKey, allocate);

			//Debug::LogInfo("parsing[%s]%s with 3 values [%s - %s - %s]", pSection , pKey , nFirst.c_str() , nSecondPair_1.c_str() , nSecondPair_2.c_str());
		}
		else
		{
			value.From.clear();
			char* context = nullptr;
			for (auto pCur = strtok_s(nFirst.data(), Phobos::readDelims, &context);
					pCur;
					pCur = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				TechnoTypeClass* buffer = nullptr;

				if (Parser<TechnoTypeClass*>::Parse(pCur, &buffer))
					value.From.push_back(buffer);
				else if (!allocate && !GameStrings::IsNone(pCur))
					Debug::INIParseFailed(pSection, pKey, pCur, nullptr);
			}

			Parser<TechnoTypeClass*>::Parse(nSecondPair.c_str(), &value.To);
		}

		return true;
	}

	return true;
}

template <>
bool detail::getresult<TileType>(TileType& value, const std::string& parser, const char* pSection, const char* pKey, bool bAllocate)
{
	if (!parser.empty())
	{

		if (GameStrings::IsNone(parser.c_str()))
		{
			value = TileType::ClearToSandLAT;
			return true;
		}

		for (size_t i = 1; i < EnumFunctions::TileType_ToStrings.size(); ++i)
		{
			if (IS_SAME_STR_(parser.c_str(), EnumFunctions::TileType_ToStrings[i]))
			{
				value = TileType(i);
				return true;
			}
		}

		Debug::INIParseFailed(pSection, pKey, parser.c_str(), "Expect valid TileType !");
	}
	return false;
}

template <>
bool detail::getresult<LandType>(LandType& value, const std::string& parser, const char* pSection, const char* pKey, bool bAllocate)
{
	if (!parser.empty())
	{

		if (GameStrings::IsNone(parser.c_str()))
		{
			value = LandType::Clear;
			return true;
		}

		for (size_t i = 0; i < CellClass::LandTypeToStrings.size(); ++i)
		{
			if (IS_SAME_STR_(CellClass::LandTypeToStrings[i], parser.c_str()))
			{
				value = LandType(i);
				return true;
			}
		}

		Debug::INIParseFailed(pSection, pKey, parser.c_str(), "Expect Valind LandType");
	}

	return false;
}

template <>
bool detail::getresult<PhobosAbilityType>(PhobosAbilityType& value, const std::string& parser, const char* pSection, const char* pKey, bool bAllocate)
{
	if (!parser.empty())
	{
		for (size_t i = 0; i < EnumFunctions::PhobosAbilityType_ToStrings.size(); ++i)
		{
			if (IS_SAME_STR_(EnumFunctions::PhobosAbilityType_ToStrings[i], parser.c_str()))
			{
				value = PhobosAbilityType(i);
				return true;
			}
		}

		bool found = false;
		for (size_t a = 0; a < TechnoTypeClass::AbilityTypeToStrings.size(); ++a)
		{
			if (IS_SAME_STR_(TechnoTypeClass::AbilityTypeToStrings[a], parser.c_str()))
			{
				found = true;
			}
		}

		if (!found)
			Debug::INIParseFailed(pSection, pKey, parser.c_str(), "Expect Valind AbilityTypes");
	}

	return false;
}

template <>
bool detail::getresult<Rank>(Rank& value, const std::string& parser, const char* pSection, const char* pKey, bool bAllocate)
{
	if (!parser.empty())
	{
		for (size_t i = 1; i < EnumFunctions::Rank_ToStrings.size(); ++i)
		{
			const auto& [val, str] = EnumFunctions::Rank_ToStrings[i];
			if (PhobosCRT::iequals(parser.c_str(), str))
			{
				value = val;
				return true;
			}
		}

		Debug::INIParseFailed(pSection, pKey, parser.c_str(), "Expected a self Rank type");
	}

	return false;
}

template <>
bool detail::getresult<FacingType>(FacingType& value, const std::string& parser, const char* pSection, const char* pKey, bool bAllocate)
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

template <>
bool detail::read<TechnoTypeClass*>(TechnoTypeClass*& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (parser.ReadString(pSection, pKey))
	{
		const auto pValue = parser.value();
		const auto parsed = TechnoTypeClass::Find(pValue);

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
bool detail::read<SHPStruct*>(SHPStruct*& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (parser.ReadString(pSection, pKey))
	{
		std::string flag = _strlwr(parser.value());

		if (flag.size() < 4 || !std::equal(flag.end() - 4, flag.end(), ".shp", []
		(char input, char expected) { return input == expected; }))
			flag += ".shp";

		if (auto const pImage = FileSystem::LoadSHPFile(flag.c_str()))
		{
			value = pImage;
			return true;
		}
		else
		{
			Debug::Log("Failed to find file %s referenced by [%s]%s=%s\n", flag.c_str(), pSection, pKey, parser.value());
		}
	}

	return false;
}

template <>
bool detail::read<std::string>(std::string& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (parser.ReadString(pSection, pKey))
	{
		value = parser.value();
		return true;
	}

	return false;
}

template <>
bool detail::read<PartialVector2D<int>>(PartialVector2D<int>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	value.ValueCount = parser.Read2IntegerAndCount(pSection, pKey, (int*)&value);

	if (value.ValueCount > 0)
		return true;

	return false;
}

template <>
bool detail::read<PartialVector2D<double>>(PartialVector2D<double>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	value.ValueCount = parser.Read2DoubleAndCount(pSection, pKey, (double*)&value);

	if (value.ValueCount > 0)
		return true;

	return false;
}

template <>
bool detail::read<PartialVector3D<int>>(PartialVector3D<int>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	value.ValueCount = parser.Read3IntegerAndCount(pSection, pKey, (int*)&value);

	if (value.ValueCount > 0)
		return true;

	return false;
}

template <>
bool detail::read<ReversePartialVector3D<int>>(ReversePartialVector3D<int>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	// ReversePartialVector3D has Z,Y,X memory layout, so we need to read into a temp buffer
	// and then assign in the correct order
	int buffer[3] = { 0, 0, 0 };
	value.ValueCount = parser.Read3IntegerAndCount(pSection, pKey, buffer);

	if (value.ValueCount > 0)
	{
		value.X = buffer[0];
		value.Y = buffer[1];
		value.Z = buffer[2];
		return true;
	}

	return false;
}

template <>
bool detail::read<PartialVector3D<double>>(PartialVector3D<double>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	value.ValueCount = parser.Read3DoubleAndCount(pSection, pKey, (double*)&value);

	if (value.ValueCount > 0)
		return true;

	return false;
}

template <>
bool detail::read<PartialVector3D<float>>(PartialVector3D<float>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	value.ValueCount = parser.Read3FloatAndCount(pSection, pKey, (float*)&value);

	if (value.ValueCount > 0)
		return true;

	return false;
}

template <>
bool detail::read<bool>(bool& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (!parser.ReadBool(pSection, pKey, &value))
	{

		if (!parser.empty())
			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid boolean value [1, true, yes, 0, false, no]");

		return false;
	}

	return true;
}

template <>
bool detail::read<int>(int& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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
bool detail::read<unsigned int>(unsigned int& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	int buffer { 0 };
	if (parser.ReadInteger(pSection, pKey, &buffer) && buffer > 0 && (unsigned int)buffer <= MAX_VAL(unsigned int))
	{
		value = (unsigned int)buffer;
		return true;
	}

	if (!parser.empty())
		Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid unsigned int between 0 and 4294967295 inclusive");

	return false;
}

template <>
bool detail::read<unsigned short>(unsigned short& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	short buffer { 0 };
	if (parser.ReadShort(pSection, pKey, &buffer) && buffer > 0 && (unsigned short)buffer <= MAX_VAL(unsigned short))
	{
		value = (unsigned short)buffer;
		return true;
	}

	if (!parser.empty())
		Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid unsigned short between 0 and 65535 inclusive");

	return false;
}

template <>
bool detail::read<short>(short& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (!parser.ReadShort(pSection, pKey, &value))
	{
		if (!parser.empty())
		{
			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid short");
		}

		return false;
	}

	return true;
}

template <>
bool detail::read<BYTE>(BYTE& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (!parser.ReadBytes(pSection, pKey, &value))
	{

		if (!parser.empty())
			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid number");

		return false;
	}

	return true;
}

template <>
bool detail::read<float>(float& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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
bool detail::read<double>(double& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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
bool detail::read<CellStruct>(CellStruct& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (!parser.Read2Short(pSection, pKey, (short*)&value))
	{
		if (!parser.empty())
			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid CellStruct");

		return false;
	}

	return true;
}

template <>
bool detail::read<Point2D>(Point2D& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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
bool detail::read<MinMaxValue<int>>(MinMaxValue<int>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (!parser.Read2Integers(pSection, pKey, (int*)&value))
	{
		if (!parser.empty())
			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid MinMaxValue<int>");

		return false;
	}

	return true;
}

template <>
bool detail::read<MinMaxValue<float>>(MinMaxValue<float>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (!parser.Read2Float(pSection, pKey, (float*)&value))
	{
		if (!parser.empty())
			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid MinMaxValue<float>");

		return false;
	}

	return true;
}

template <>
bool detail::read<MinMaxValue<double>>(MinMaxValue<double>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (!parser.Read2Double(pSection, pKey, (double*)&value))
	{
		if (!parser.empty())
			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid MinMaxValue<double>");

		return false;
	}

	return true;
}

template <>
bool detail::read<Vector2D<int>>(Vector2D<int>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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
bool detail::read<Point3D>(Point3D& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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
bool detail::read<RectangleStruct>(RectangleStruct& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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
bool detail::read<Point2DBYTE>(Point2DBYTE& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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
bool detail::read<CoordStruct>(CoordStruct& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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
bool detail::read<Vector3D<BYTE>>(Vector3D<BYTE>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (!parser.Read3Bytes(pSection, pKey, (BYTE*)&value))
	{
		if (!parser.empty())
		{
			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid 3 BYTE Value");
		}

		return false;
	}

	return true;
}

template <>
bool detail::read<Vector3D<int>>(Vector3D<int>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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
bool detail::read<Vector2D<double>>(Vector2D<double>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (parser.Read2Double(pSection, pKey, (double*)&value))
		return true;
	else if (!parser.empty())
	{
		Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid 2 floating point Value");
	}

	return false;
}

template <>
bool detail::read<CSFText>(CSFText& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (parser.ReadString(pSection, pKey))
	{
		value = parser.value();
		return true;
	}
	return false;
}

template <>
bool detail::read<RocketStruct>(RocketStruct& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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
bool detail::read<Leptons>(Leptons& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	double buffer;
	//vanilla : return false if -1.0
	if (read(buffer, parser, pSection, pKey, allocate) && buffer != -1.0)
	{
		value = Leptons(buffer);
		return true;
	}

	return false;
}

template <>
bool detail::read<SWRange>(SWRange& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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

template <>
bool detail::read<PassiveAcquireModes>(PassiveAcquireModes& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (parser.ReadString(pSection, pKey))
	{
		for (const auto& [val, name] : EnumFunctions::PassiveAcquireModes_ToStrings)
		{
			if (PhobosCRT::iequals(parser.value(), name))
			{
				value = val;
				return true;
			}
		}

		Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a PassiveAcquireMode type");
	}

	return false;
}

template <>
bool detail::read<LaserTrailDrawType>(LaserTrailDrawType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (parser.ReadString(pSection, pKey))
	{
		for (const auto& [val, name] : EnumFunctions::LaserTrailDrawType_ToStrings)
		{
			if (PhobosCRT::iequals(parser.value(), name))
			{
				value = val;
				return true;
			}
		}

		Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a LaserTrail draw type");
	}

	return false;
}

template <>
bool detail::read<AttachedAnimPosition>(AttachedAnimPosition& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (parser.ReadString(pSection, pKey))
	{

		char* context = nullptr;
		auto resultData = AttachedAnimPosition::Default;

		for (auto cur = strtok_s(parser.value(), Phobos::readDelims, &context);
			cur;
			cur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			bool found = false;
			for (const auto& [val, name] : EnumFunctions::AttachedAnimPosition_ToStrings)
			{
				if (PhobosCRT::iequals(cur, name))
				{
					resultData |= val;
					found = true;
					break;
				}
			}

			if (!found && IS_SAME_STR_(cur, "centre"))
			{
				resultData |= AttachedAnimPosition::Center;
				found = true;
			}

			if (!found)
			{
				Debug::INIParseFailed(pSection, pKey, cur, "Expected aAttachedAnimPosition type");
				return false;
			}
		}

		value = resultData;
		return true;
	}

	return false;
}

template <>
bool detail::read<Rank>(Rank& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	return parser.ReadString(pSection, pKey)
		&& getresult<Rank>(value, parser.value(), pSection, pKey);
}

template <>
bool detail::read<Armor>(Armor& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	return parser.ReadArmor(pSection, pKey, (int*)&value);
}

template <>
bool detail::read<Edge>(Edge& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (parser.ReadString(pSection, pKey))
	{
		for (size_t i = 0; i < CellClass::EdgeToStrings.size(); ++i)
		{
			if (PhobosCRT::iequals(parser.value(), CellClass::EdgeToStrings[i]))
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
bool detail::read<TranslucencyLevel>(TranslucencyLevel& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	return value.Read(parser, pSection, pKey);
}

template <>
bool detail::read<HorizontalPosition>(HorizontalPosition& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (parser.ReadString(pSection, pKey))
	{
		for (const auto& [val, str] : EnumFunctions::HorizontalPosition_ToStrings)
		{
			if (PhobosCRT::iequals(parser.value(), str))
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
bool detail::read<BannerNumberType>(BannerNumberType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (parser.ReadString(pSection, pKey))
	{
		for (const auto& [val, str] : EnumFunctions::BannerNumberType_ToStrings)
		{
			if (PhobosCRT::iequals(parser.value(), str))
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
bool detail::read<VerticalPosition>(VerticalPosition& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (parser.ReadString(pSection, pKey))
	{
		if (IS_SAME_STR_(parser.value(), "centre"))
		{
			value = VerticalPosition::Center;
			return true;
		}

		for (const auto& [val, str] : EnumFunctions::VerticalPosition_ToStrings)
		{
			if (PhobosCRT::iequals(parser.value(), str))
			{
				value = val;
				return true;
			}
		}

		Debug::INIParseFailed(pSection, pKey, parser.value(), "Vertical Position can be either Top, Center/Centre or Bottom");

	}
	return false;
}

template <>
bool detail::read<SelfHealGainType>(SelfHealGainType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (parser.ReadString(pSection, pKey))
	{

		if (IS_SAME_STR_(parser.value(), "noheal"))
		{
			value = SelfHealGainType::None;
			return true;
		}

		for (const auto& [val, str] : EnumFunctions::SelfHealGainType_ToStrings)
		{
			if (PhobosCRT::iequals(parser.value(), str))
			{
				value = val;
				return true;
			}
		}

		Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a self heal gain type");
	}

	return false;
}

template <>
bool detail::read<SlaveReturnTo>(SlaveReturnTo& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (parser.ReadString(pSection, pKey))
	{
		for (const auto& [val, str] : EnumFunctions::SlaveReturnTo_ToStrings)
		{
			if (PhobosCRT::iequals(parser.value(), str))
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
bool detail::read<KillMethod>(KillMethod& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (parser.ReadString(pSection, pKey))
	{
		if (IS_SAME_STR_(parser.value(), "kill"))
		{
			value = KillMethod::Explode;
			return true;
		}

		for (const auto& [val, str] : EnumFunctions::KillMethod_ToStrings)
		{
			if (PhobosCRT::iequals(parser.value(), str))
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
bool detail::read<IronCurtainFlag>(IronCurtainFlag& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (parser.ReadString(pSection, pKey))
	{
		for (const auto& [val, str] : EnumFunctions::IronCurtainFlag_ToStrings)
		{
			if (PhobosCRT::iequals(parser.value(), str))
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
bool detail::read<OwnerHouseKind>(OwnerHouseKind& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (parser.ReadString(pSection, pKey))
	{
		for (size_t i = 0; i < EnumFunctions::OwnerHouseKind_ToStrings.size(); ++i)
		{
			if (PhobosCRT::iequals(parser.value(), EnumFunctions::OwnerHouseKind_ToStrings[i].second))
			{
				value = EnumFunctions::OwnerHouseKind_ToStrings[i].first;
				return true;
			}
		}

		Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a owner house kind");
	}
	return false;
}

template <>
bool detail::read<Mission>(Mission& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (parser.ReadString(pSection, pKey))
	{
		value = MissionClass::GetMissionById(parser.value());
		return true;
	}

	return false;
}

template <>
bool detail::read<SuperWeaponAITargetingMode>(SuperWeaponAITargetingMode& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (parser.ReadString(pSection, pKey))
	{
		for (const auto& [val, str] : EnumFunctions::SuperWeaponAITargetingMode_ToStrings)
		{
			if (PhobosCRT::iequals(parser.value(), str))
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
bool detail::read<AffectedTarget>(AffectedTarget& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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
			for (const auto& [pStrings, val] : EnumFunctions::AffectedTarget_ToStrings)
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
				Debug::INIParseFailed(pSection, pKey, cur, "Expected a affected target");
				return false;
			}
		}

		value = resultData;
		return true;
	}
	return false;
}

template <>
bool detail::read<ChronoSparkleDisplayPosition>(ChronoSparkleDisplayPosition& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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
			for (const auto& [pStrings, val] : EnumFunctions::ChronoSparkleDisplayPosition_ToStrings)
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
				Debug::INIParseFailed(pSection, pKey, cur, "Expected a chrono sparkle position type");
			}
		}

		value = resultData;
		return true;
	}

	return false;
}

template <>
bool detail::read<SuperWeaponTarget>(SuperWeaponTarget& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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
				Debug::INIParseFailed(pSection, pKey, cur, "Expected a SW target");
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
bool detail::read<TargetingConstraints>(TargetingConstraints& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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
			for (const auto& [pStrings, val] : EnumFunctions::TargetingConstraints_ToStrings)
			{
				if (IS_SAME_STR_(cur, pStrings))
				{
					resultData |= val;
					found = true;
					break;
				}
			}

			if (!found)
			{
				Debug::INIParseFailed(pSection, pKey, cur, "Expected a targeting constraint");
				return false;
			}
		}

		value = resultData;
		return true;
	}
	return false;
}

template <>
bool detail::read<TargetingPreference>(TargetingPreference& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (parser.ReadString(pSection, pKey))
	{
		for (const auto& [val, str] : EnumFunctions::TargetingPreference_ToStrings)
		{
			if (PhobosCRT::iequals(parser.value(), str))
			{
				value = val;
				return true;
			}
		}

		Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a targeting preference");
	}
	return false;
}

template <>
bool detail::read<DiscardCondition>(DiscardCondition& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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
				Debug::INIParseFailed(pSection, pKey, cur, "Expected a DiscardCondition");
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
				case 5: resultData |= DiscardCondition::InRange; break;
				case 6: resultData |= DiscardCondition::OutOfRange; break;
				case 7: resultData |= DiscardCondition::InvokerDeleted; break;
				case 8: resultData |= DiscardCondition::Firing; break;
				}
			}
		}

		value = resultData;
		return true;
	}
	return false;
}

template <>
bool detail::read<ExpireWeaponCondition>(ExpireWeaponCondition& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (parser.ReadString(pSection, pKey))
	{
		char* context = nullptr;
		ExpireWeaponCondition resultData = ExpireWeaponCondition::None;

		for (auto cur = strtok_s(parser.value(), Phobos::readDelims, &context);
			cur;
			cur = strtok_s(nullptr, Phobos::readDelims, &context))
		{

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
				Debug::INIParseFailed(pSection, pKey, cur, "Expected a ExpireWeaponCondition");
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
				case 5: resultData = ExpireWeaponCondition::All; break;
				}
			}
		}

		value = resultData;
		return true;
	}
	return false;
}

template <>
bool detail::read<LandType>(LandType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	return parser.ReadString(pSection, pKey)
		&& getresult<LandType>(value, parser.value(), pSection, pKey, allocate);
}

template <>
bool detail::read<AffectedHouse>(AffectedHouse& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	return parser.ReadString(pSection, pKey)
		&& getresult<AffectedHouse>(value, parser.value(), pSection, pKey, allocate);
}

template <>
bool detail::read<AttachedAnimFlag>(AttachedAnimFlag& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (parser.ReadString(pSection, pKey))
	{
		for (const auto& [pString, val] : EnumFunctions::AttachedAnimFlag_ToStrings)
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
bool detail::read<AreaFireTarget>(AreaFireTarget& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (parser.ReadString(pSection, pKey))
	{
		for (const auto& [val, str] : EnumFunctions::AreaFireTarget_ToStrings)
		{
			if (PhobosCRT::iequals(parser.value(), str))
			{
				value = val;
				return true;
			}
		}

		Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected an AreaFire target");
	}

	return false;
}

template <>
bool detail::read<TextAlign>(TextAlign& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (parser.ReadString(pSection, pKey))
	{
		for (auto const& [pString, val] : EnumFunctions::TextAlign_ToStrings)
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
bool detail::read<Layer>(Layer& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (parser.ReadString(pSection, pKey))
	{
		if (GameStrings::IsNone(parser.value()))
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
bool detail::read<AbstractType>(AbstractType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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

		Debug::INIParseFailed(pSection, pKey, parser.value(), "Expect a Valid AbstractType !");
	}

	return false;
}

template <>
bool detail::read<Locomotors>(Locomotors& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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

		Debug::INIParseFailed(pSection, pKey, parser.value(), "Expect valid Locomotor CLSID or Name");
	}

	return false;
}

template <>
bool detail::read(TileType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	return parser.ReadString(pSection, pKey)
		&& getresult<TileType>(value, parser.value(), pSection, pKey, allocate);
}

template <>
bool detail::read<LandTypeFlags>(LandTypeFlags& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (parser.ReadString(pSection, pKey))
	{
		auto parsed = LandTypeFlags::None;
		auto str = parser.value();
		char* context = nullptr;

		for (auto cur = strtok_s(str, Phobos::readDelims, &context);
			cur;
			cur = strtok_s(nullptr, Phobos::readDelims, &context))
		{

			auto const landType = GroundType::GetLandTypeFromName(parser.value());

			if (landType >= LandType::Clear && landType <= LandType::Weeds)
			{
				parsed |= (LandTypeFlags)(1 << (char)landType);
			}
			else
			{
				Debug::INIParseFailed(pSection, pKey, cur, "Expected a land type name");
				return false;
			}
		}

		value = parsed;
		return true;
	}

	return false;
}

template <>
bool detail::read<TypeList<int>>(TypeList<int>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (parser.ReadString(pSection, pKey))
	{
		value.reset();

		char* context = nullptr;
		for (auto pCur = strtok_s(parser.value(), Phobos::readDelims, &context);
			pCur;
			pCur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			int buffer = 0;
			if (Parser<int>::Parse(pCur, &buffer))
			{
				value.push_back(buffer);
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

template <>
bool detail::read<Vector3D<float>>(Vector3D<float>& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (!parser.Read3Float(pSection, pKey, (float*)&value))
	{
		if (!parser.empty())
		{
			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid 3 float Values");
		}

		return false;
	}

	return true;
}

template<>
bool detail::read<Foundation>(Foundation& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (parser.ReadString(pSection, pKey))
	{
		for (size_t i = 0; i < TechnoTypeClass::BuildingFoundationName.size(); ++i)
		{
			if (IS_SAME_STR_(TechnoTypeClass::BuildingFoundationName[i].Name, parser.c_str()))
			{
				value = TechnoTypeClass::BuildingFoundationName[i].Value;
				return true;
			}
		}

		if (IS_SAME_STR_(parser.c_str(), "Custom"))
		{
			value = (Foundation)127;
			return true;
		}

		if (FindFoundation(parser.c_str()))
		{
			return true;
		}

		Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected valid Foundation value");
	}

	return false;
}

template<>
bool detail::read<DoTypeFacing>(DoTypeFacing& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (parser.ReadString(pSection, pKey))
	{
		for (size_t i = 0; i < EnumFunctions::FacingType_to_strings.size(); ++i)
		{
			if (IS_SAME_STR_(EnumFunctions::FacingType_to_strings[i], parser.c_str()))
			{
				value = DoTypeFacing(i);
				return true;
			}
		}

		Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected valid DoTypeFacing value");
	}

	return false;
}

template <>
bool detail::read<AffectPlayerType>(AffectPlayerType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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

			if (!found)
			{
				Debug::INIParseFailed(pSection, pKey, cur, "Expected valid AffectPlayerType value");
			}
		}

		value = resultData;
		return true;
	}

	return false;
}

template <>
bool detail::read<AffectedVeterancy>(AffectedVeterancy& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (parser.ReadString(pSection, pKey))
	{
		char* context = nullptr;
		AffectedVeterancy resultData = AffectedVeterancy::None;

		for (auto cur = strtok_s(parser.value(), Phobos::readDelims, &context);
			cur;
			cur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			bool found = false;
			for (const auto& [pStrings, val] : EnumFunctions::AffectedVeterancy_ToStrings)
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
				Debug::INIParseFailed(pSection, pKey, cur, "Expected valid AffectedVeterancy value");
			}
		}

		value = resultData;
		return true;
	}

	return false;
}

template <>
bool detail::read<SpotlightFlags>(SpotlightFlags& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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
			for (const auto& [pStrings, val] : EnumFunctions::SpotlightFlags_ToStrings)
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
				Debug::INIParseFailed(pSection, pKey, cur, "Expected valid SpotlightFlags value");
			}
		}

		value = resultData;
		return true;
	}

	return false;
}

template <>
bool detail::read<ColorStruct>(ColorStruct& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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

			for (auto& [val, str] : EnumFunctions::SpotlightAttachment_ToStrings)
			{
				if (PhobosCRT::iequals(parser.value(), str))
				{
					value = Drawing::DefaultColors[(BYTE)val];
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
bool detail::read<DamageDelayTargetFlag>(DamageDelayTargetFlag& value, INI_EX& parser, const char* pSection, const char* pKey, bool bAllocate)
{
	if (parser.ReadString(pSection, pKey))
	{
		if (GameStrings::IsNone(parser.value()))
			return false;

		for (auto& [val, str] : EnumFunctions::DamageDelayTargetFlag_ToStrings)
		{
			if (PhobosCRT::iequals(parser.value(), str))
			{
				value = val;
				return true;
			}
		}

		Debug::INIParseFailed(pSection, pKey, parser.value(), "Expect valid DamageDelayTargetFlag");
	}

	return false;
}

template <>
bool detail::read<NewCrateType>(NewCrateType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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
bool detail::read<TargetZoneScanType>(TargetZoneScanType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (parser.ReadString(pSection, pKey))
	{
		if (GameStrings::IsNone(parser.value()))
			return false;

		for (auto& [val, str] : EnumFunctions::TargetZoneScanType_ToStrings)
		{
			if (PhobosCRT::iequals(parser.value(), str))
			{
				value = val;
				return true;
			}
		}

		Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a target zone scan type");
	}

	return false;
}

template <>
bool detail::read<MouseCursor>(MouseCursor& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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
bool detail::read<MouseCursorDataStruct>(MouseCursorDataStruct& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	auto ret = false;
	std::string _key(pKey);

	for (size_t i = 0; i < EnumFunctions::MouseCursorData_ToStrings.size(); ++i)
	{
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
bool detail::read<FacingType>(FacingType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (parser.ReadString(pSection, pKey) && detail::getresult(value, parser.value(), pSection, pKey))
	{
		return true;
	}

	return false;
}

template <>
bool detail::read<DirType32>(DirType32& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	int nBuffer = -1;
	if (parser.ReadInteger(pSection, pKey, &nBuffer))
	{
		if (nBuffer >= (int)DirType32::Min && nBuffer <= (int)DirType32::Max)
		{
			value = (DirType32)nBuffer;
			return true;
		}

		Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a DirType32 between 0 and 32");
	}

	return false;
}

template <>
bool detail::read<DirType>(DirType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	int nBuffer = -1;
	if (parser.ReadInteger(pSection, pKey, &nBuffer))
	{
		const bool IsNegative = nBuffer < 0;
		const DirType nVal = (DirType)Math::abs(nBuffer);

		if (DirType::North <= nVal && nVal <= DirType::Max)
		{
			value = (DirType)(!IsNegative ? (int)nVal : (int)DirType::Max + 1 - IsNegative);
			return true;
		}

		Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a DirType between 0 and 255");
	}

	return false;
}

template <>
bool detail::read<SpotlightAttachment>(SpotlightAttachment& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (parser.ReadString(pSection, pKey))
	{
		for (auto& [val, str] : EnumFunctions::SpotlightAttachment_ToStrings)
		{
			if (PhobosCRT::iequals(parser.value(), str))
			{
				value = val;
				return true;
			}
		}

		Debug::INIParseFailed(pSection, pKey, parser.value(), "Expect valid SpotlightAttachment");
	}

	return false;
}

template <>
bool detail::read<ShowTimerType>(ShowTimerType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (parser.ReadString(pSection, pKey))
	{
		for (auto& [val, str] : EnumFunctions::ShowTimerType_ToStrings)
		{
			if (PhobosCRT::iequals(parser.value(), str))
			{
				value = val;
				return true;
			}
		}

		Debug::INIParseFailed(pSection, pKey, parser.value(), "Expect valid ShowTimerType");
	}

	return false;
}

template <>
bool detail::read<AttachmentYSortPosition>(AttachmentYSortPosition& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (parser.ReadString(pSection, pKey))
	{
		for (auto& [val, str] : EnumFunctions::AttachmentYSortPosition_ToStrings)
		{
			if (PhobosCRT::iequals(parser.value(), str))
			{
				value = val;
				return true;
			}
		}

		Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected an attachment YSort position");
	}
	return false;
}

template <>
bool detail::read<AffectedTechno>(AffectedTechno& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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
			for (const auto& pStrings : EnumFunctions::AffectedTechno_ToStrings)
			{
				if (PhobosCRT::iequals(cur, pStrings.second))
				{
					found = true;
					break;
				}
				++result;
			}

			if (!found)
			{
				if (IS_SAME_STR_(cur, "vehicle"))
				{
					found = true;
					result = 2;
				}
			}

			if (!found)
			{
				Debug::INIParseFailed(pSection, pKey, cur, "Expected a AffectedTechno");
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
bool detail::read<DisplayShowType>(DisplayShowType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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
			for (const auto& pStrings : EnumFunctions::DisplayShowType_ToStrings)
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
				Debug::INIParseFailed(pSection, pKey, cur, "Expected a DisplayShowType");
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
bool detail::read<BountyValueOption>(BountyValueOption& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (parser.ReadString(pSection, pKey))
	{
		const auto pVal = parser.value();

		for (auto& [val, str] : EnumFunctions::BountyValueOption_ToStrings)
		{
			if (PhobosCRT::iequals(pVal, str))
			{
				value = val;
				return true;
			}
		}

		Debug::INIParseFailed(pSection, pKey, pVal, "Expect valid BountyValueOption");
	}

	return false;
}

template <>
bool detail::read<BuildingSelectBracketPosition>(BuildingSelectBracketPosition& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (parser.ReadString(pSection, pKey))
	{
		const auto pVal = parser.value();
		for (auto& [val, str] : EnumFunctions::BuildingSelectBracketPosition_ToStrings)
		{
			if (PhobosCRT::iequals(pVal, str))
			{
				value = val;
				return true;
			}
		}

		Debug::INIParseFailed(pSection, pKey, pVal, "Expect valid BuildingSelectBracketPosition");
	}

	return false;
}

template <>
bool detail::read<DisplayInfoType>(DisplayInfoType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (parser.ReadString(pSection, pKey))
	{
		const auto pVal = parser.value();
		for (auto& [val, str] : EnumFunctions::DisplayInfoType_ToStrings)
		{
			if (PhobosCRT::iequals(pVal, str))
			{
				value = val;
				return true;
			}
		}

		Debug::INIParseFailed(pSection, pKey, pVal, "Expect valid DisplayInfoType");
	}

	return false;
}

template <>
bool detail::read<InterpolationMode>(InterpolationMode& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocat)
{
	if (parser.ReadString(pSection, pKey))
	{
		auto str = parser.value();

		for (auto& [val, string] : EnumFunctions::InterpolationMode_ToStrings)
		{
			if (PhobosCRT::iequals(string, str))
			{
				value = val;
				return true;
			}
		}

		Debug::INIParseFailed(pSection, pKey, str, "Expected an interpolation mode");
	}

	return false;
}

template <>
bool detail::read<ParabolaFireMode>(ParabolaFireMode& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (parser.ReadString(pSection, pKey))
	{
		for (const auto& [val, str] : EnumFunctions::ParabolaFireMode_ToStrings)
		{
			if (PhobosCRT::iequals(parser.value(), str))
			{
				value = val;
				return true;
			}
		}

		Debug::INIParseFailed(pSection, pKey, parser.value(), "Parabola fire mode is invalid");
	}

	return false;
}

template <>
bool detail::read<CLSID>(CLSID& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
{
	if (!parser.ReadString(pSection, pKey))
		return false;

	// Semantic locomotor aliases
	if (parser.value()[0] != '{')
	{

		if (Phobos::Otamaa::IsAdmin)
			Debug::Log("Reading locomotor [%s] of [%s]\n", parser.value(), pSection);

		for (size_t i = 0; i < EnumFunctions::LocomotorPairs_ToStrings.size(); ++i)
		{
			if (IS_SAME_STR_(parser.value(), EnumFunctions::LocomotorPairs_ToStrings[i].first))
			{
				CLSID dummy;
				const unsigned hr = CLSIDFromString(LPCOLESTR(EnumFunctions::LocomotorPairs_ToWideStrings[i].second), &dummy);
				if (SUCCEEDED(hr))
				{
					value = dummy; return true;
				}
				else
				{
					Debug::LogError("Cannot find Locomotor [{} - {}({})] err : 0x{:08X}", pSection, parser.value(), EnumFunctions::LocomotorPairs_ToStrings[i].second, hr);
				}
			}
		}

		//PARSE(Attachment)
		//PARSE(Levitate)
		PARSE(AdvancedDrive)
		//PARSE(CustomRocket)
		//PARSE(TSJumpJet)
		PARSE(Shift)
		//AddMore loco here
		return false;
	}

	CHAR bytestr[128];
	WCHAR wcharstr[128];

	strncpy(bytestr, parser.value(), 128);
	bytestr[127] = NULL;
	CRT::strtrim(bytestr);

	if (!strlen(bytestr))
		return false;

	MultiByteToWideChar(0, 1, bytestr, -1, wcharstr, 128);
	const unsigned hr = CLSIDFromString(wcharstr, &value);

	if (!SUCCEEDED(hr))
	{
		Debug::LogError("Cannot find Locomotor [{} - {}] err : 0x{:08X}", pSection, parser.value(), hr);
		return false;
	}

	return true;
}

template <>
void detail::parse_values<LandType>(std::vector<LandType>& vector, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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
void detail::parse_values<FacingType>(std::vector<FacingType>& vector, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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
void detail::parse_values<PhobosAbilityType>(std::vector<PhobosAbilityType>& vector, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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
void detail::parse_values<TechnoTypeConvertData>(std::vector<TechnoTypeConvertData>& vector, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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
void detail::parse_values<TileType>(std::vector<TileType>& vector, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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
void detail::parse_values<Mission>(std::vector<Mission>& vector, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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
void detail::parse_values<Rank>(std::vector<Rank>& vector, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
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

void detail::ParseVector(INI_EX& IniEx, std::vector<std::vector<std::string>>& nVecDest, const char* pSection, bool bDebug, bool bVerbose, const char* Delims, const char* message)
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


#undef PARSE