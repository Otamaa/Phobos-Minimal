#include "GeneralUtils.h"
#include "Debug.h"
#include <ScenarioClass.h>
#include <Conversions.h>
#include <VocClass.h>

#include <Utilities/Cast.h>
#include <Utilities/CSFText.h>
#include <Misc/CSF.h>

#include <Ext/Techno/Body.h>
#include <Ext/WarheadType/Body.h>

#include <TranslateFixedPoints.h>

#include <Locomotor/CLSIDs.h>

#include "EventClass.h"

#include <Ext/Rules/Body.h>

#include <SuperClass.h>

template<>
int  GeneralUtils::GetRandomValue<true>(const Point2D point, int defVal)
{
	int min = point.X;
	int max = point.Y;

	if (min > max)
	{
		min = max;
		max = point.X;
	}

	if (max > 0)
	{
		return ScenarioClass::Instance->Random.RandomRanged(min, max);
	}

	return defVal;
}

template<>
int  GeneralUtils::GetRandomValue<false>(const Point2D point, int defVal)
{
	int min = point.X;
	int max = point.Y;

	if (min > max)
	{
		min = max;
		max = point.X;
	}

	if (max > 0)
	{
		return Random2Class::NonCriticalRandomNumber->RandomRanged(min, max);
	}

	return defVal;
}

int GeneralUtils::GetColorFromColorAdd(int colorIndex)
{
	auto const& colorAdd = FakeRulesClass::Instance()->ColorAdds;
	const int colorValue = GetColorIndexForColorAdd(colorIndex);

	if (FakeRulesClass::Instance()->ColorAddUse8BitRGB)
		return colorAdd[colorValue].ToInit();

	return GetColorFromColorAdd(colorAdd[colorValue]);
}

ColorStruct GeneralUtils::GetColorStructFromColorAdd(int colorIndex)
{
	return FakeRulesClass::Instance()->ColorAdds[GetColorIndexForColorAdd(colorIndex)];
}

int GeneralUtils::GetColorIndexForColorAdd(int colorIndex) //this one fixup the index
{
	return ((size_t)colorIndex >= FakeRulesClass::Instance()->ColorAdds.size()) ? 0 : colorIndex;
}

int GeneralUtils::GetColorFromColorAdd(ColorStruct const& colors)
{
	int colorValue = 0;
	int red = colors.R;
	int green = colors.G;
	int blue = colors.B;

	switch (Drawing::ColorMode())
	{
	case RGBMode::RGB565:
		colorValue |= blue | (32 * (green | (red << 6)));
		break;
	case RGBMode::RGB556:
		colorValue |= blue | (((32 * red) | (green >> 1)) << 6);
		break;
	default:
		colorValue |= blue | (32 * ((32 * red) | (green >> 1)));
		break;
	}

	return colorValue;
}

bool GeneralUtils::IsValidString(const char* str)
{
	if (str == nullptr || strlen(str) == 0 || GameStrings::IsNone(str))
		return false;

	if (IS_SAME_STR_(str , DEFAULT_STR) || IS_SAME_STR_(str, DEFAULT_STR2))
		return false;

	return true;
}

bool GeneralUtils::IsValidString(const wchar_t* str)
{
	return str != nullptr && wcslen(str) != 0 && !wcsstr(str, L"MISSING:");
}

void GeneralUtils::IntValidCheck(int* source, const char* section, const char* tag, int defaultValue, int min, int max)
{
	if (defaultValue < min) defaultValue = min;
	if (defaultValue > max) defaultValue = max;

	if (*source < min || *source>max) {
		//Debug::LogInfo("[Developer warning][%s]%s=%d is invalid! Reset to %d.", section, tag, *source, defaultValue);
		*source = defaultValue;
	}
}

void GeneralUtils::DoubleValidCheck(double* source, const char* section, const char* tag, double defaultValue, double min, double max)
{
	if (defaultValue < min) defaultValue = min;
	if (defaultValue > max) defaultValue = max;

	if (*source < min || *source>max) {
		//Debug::LogInfo("[Developer warning][%s]%s=%f is invalid! Reset to %f.", section, tag, *source, defaultValue);
		*source = defaultValue;
	}
}

const wchar_t* GeneralUtils::LoadStringOrDefault(const char* key, const wchar_t* defaultValue)
{
	if (!GeneralUtils::IsValidString(key))
		return defaultValue;

	return CSFLoader::FetchStringManager(key, nullptr, nullptr, -1);
}

const wchar_t* GeneralUtils::LoadStringUnlessMissing(const char* key, const wchar_t* defaultValue)
{
	if (!GeneralUtils::IsValidString(key))
		return defaultValue;

	return GeneralUtils::LoadStringUnlessMissingNoChecks(key, defaultValue);
}

const wchar_t* GeneralUtils::LoadStringUnlessMissingNoChecks(const char* key, const wchar_t* defaultValue)
{
	auto pCSF = CSFLoader::FindOrAllocateDynamicStrings(key);

	if (pCSF->IsMissingValue) {
		pCSF->Text = defaultValue;
		pCSF->IsMissingValue = false; // enforce it to false, since we dont want to do this operation again
	}

	return pCSF->Text.c_str();
}

void GeneralUtils::AdjacentCellsInRange(std::vector<CellStruct>& nCells, short range, bool clearFirst)
{
	if(clearFirst)
		nCells.clear();

	const short value = (2 * range + 1) * (2 * range + 1);

	if (value <= 0)
		return;

	nCells.reserve(value);
	for (CellSpreadEnumerator it(range); it; ++it)
		nCells.emplace_back(*it);
}

const double GeneralUtils::GetWarheadVersusArmor(WarheadTypeClass* pWH, Armor const ArmorType)
{
	const auto& verses = WarheadTypeExtContainer::Instance.Find(pWH)->GetVerses(ArmorType);
	return verses.Verses;
	//return double(FakeWarheadTypeClass::ModifyDamage(100, pWH, ArmorType, 0)) / 100.0;
}

const bool GeneralUtils::ProduceBuilding(HouseClass* pOwner, int idxBuilding)
{
	if (auto pItem = ObjectTypeClass::FetchTechnoType(AbstractType::BuildingType, idxBuilding))
	{
		if (pOwner->CanBuild(pItem, true, true) == CanBuildResult::Buildable)
		{
			if (pItem->FindFactory(true, true, true, pOwner))
			{
				const auto pBuilding = static_cast<BuildingTypeClass*>(pItem);

				if (pOwner->GetPrimaryFactory(AbstractType::Building, false, pBuilding->BuildCat))
					return false;

				EventClass vEvent { pOwner->ArrayIndex , EventType::PRODUCE , pItem->WhatAmI(), pItem->GetArrayIndex(), pItem->Naval };
				EventClass::AddEvent(&vEvent);

				return true;
			}
		}
	}

	return false;
}

AnimTypeClass* GeneralUtils::SelectRandomAnimFromVector(Iterator<AnimTypeClass*>& vec, AnimTypeClass* fallback)
{
	if (vec.empty())
		return fallback;

	if (vec.size() == 1)
		return vec[0];

	return vec[ScenarioClass::Instance->Random.RandomFromMax(vec.size() - 1)];
}

const char* GeneralUtils::GetLocomotionName(const CLSID& clsid)
{
	if (clsid == CLSIDs::Drive()) {
		return "Drive";
	}
	else if (clsid == CLSIDs::Fly()) {
		return "Fly";
	}
	else if (clsid == CLSIDs::Jumpjet()) {
		return "Jumpjet";
	}
	else if (clsid == CLSIDs::DropPod()) {
		return "DropPod";
	}
	else if (clsid == CLSIDs::Tunnel()) {
		return "Tunnel";
	}
	else if (clsid == CLSIDs::Walk()) {
		return "Walk";
	}
	else if (clsid == CLSIDs::Mech()) {
		return "Mech";
	}
	else if (clsid == CLSIDs::Teleport()) {
		return "Teleport";
	}
	else if (clsid == CLSIDs::Rocket()) {
		return "Rocket";
	}
	else if (clsid == CLSIDs::Hover()) {
		return "Hover";
	}
	else if (clsid == CLSIDs::Ship()) {
		return "Ship";
	}

	return "<unknown>";
}

#include <New/Type/TheaterTypeClass.h>

bool GeneralUtils::ApplyTheaterSuffixToString(char* str)
{
	str = _strlwr(str);

	if (auto pSuffix = strstr(str, "~~~"))
	{
		std::string pTheater = TheaterTypeClass::Array.empty() ? Theater::Get(ScenarioClass::Instance->Theater)->Extension :
			TheaterTypeClass::FindFromTheaterType(ScenarioClass::Instance->Theater)->Extension.c_str();
		PhobosCRT::lowercase(pTheater);

		pSuffix[0] = pTheater[0];
		pSuffix[1] = pTheater[1];
		pSuffix[2] = pTheater[2];
		//Debug::LogInfo("Found designated string, Replacing [%s] to [%s] ", str, pSuffix);
		return true;
	}

	return false;
}

bool GeneralUtils::ApplyTheaterExtToString(std::string& flag)
{
	const auto nPos = flag.find("~");
	if (nPos != std::string::npos)
	{
		std::string pTheater = TheaterTypeClass::Array.empty() ? Theater::Get(ScenarioClass::Instance->Theater)->Letter:
			TheaterTypeClass::FindFromTheaterType(ScenarioClass::Instance->Theater)->Letter.c_str();

		PhobosCRT::lowercase(pTheater);

		flag.replace(nPos, 1, pTheater);
		return true;
	}

	return false;
}

std::string GeneralUtils::ApplyTheaterSuffixToString(const std::string& str)
{
	std::string buffer = str;

	const auto nPos = buffer.find("~~~");
	if (nPos != std::string::npos)
	{
		std::string pTheater = TheaterTypeClass::Array.empty() ? Theater::Get(ScenarioClass::Instance->Theater)->Extension
			: TheaterTypeClass::FindFromTheaterType(ScenarioClass::Instance->Theater)->Extension.c_str()
			;

		PhobosCRT::lowercase(pTheater);

		//only set the 3 characters without the terminator string
		buffer.replace(nPos, 3, pTheater);

		//Debug::LogInfo("Found designated string at [%d] Replacing [%s] to [%s] ",
		//	nPos, str.c_str(), buffer.c_str());

	}

	return buffer;
}

#pragma region Otamaa

AnimTypeClass* GeneralUtils::GetAnimFacingFromVector(TechnoClass* pFirer, const Iterator<AnimTypeClass*> iter)
{
	return iter.get_or(GeneralUtils::GetAnimIndexFromFacing(pFirer, iter.size()),nullptr);
}

const int GeneralUtils::GetAnimIndexFromFacing(TechnoClass* pFirer, int nVectorSize)
{
	if (!pFirer || nVectorSize <= 0)
		return -1;

	int idx = 0;
	int highest = Conversions::Int2Highest(nVectorSize);

	if (highest >= 3)
	{
		unsigned int offset = 1U << (highest - 3);
		idx = TranslateFixedPoint::Normal(16, highest, static_cast<WORD>(pFirer->GetRealFacing().Raw), offset);
	}

	return idx;
}

const int GeneralUtils::GetAnimIndexFromFacing(FootClass* pFoot, int nVectorSize)
{
	if (pFoot) {
		auto highest = Conversions::Int2Highest(nVectorSize);

		// 2^highest is the frame count, 3 means 8 frames
		if (highest >= 3)
		{
			auto offset = 1u << (highest - 3);
			return TranslateFixedPoint::Normal(16, highest, static_cast<WORD>(pFoot->GetRealFacing().GetValue<16>()), offset);
		}
	}

	return 0;
}

int GeneralUtils::GetLSAnimHeightFactor(AnimTypeClass* pType, CellClass* pCell, bool checklevel)
{
	int ImageHeignt = 0;
	if (auto pImage = pType->GetImage())
		ImageHeignt = pImage->Height;

	const auto Height = int(((double)ImageHeignt) / 2);
	const auto LSFactor = (Height - 0.5) * Math::CloudHeightFactor;

	return int(checklevel ? LSFactor + (double)Unsorted::LevelHeight * (double)pCell->Level : LSFactor);
}

const DirStruct  GeneralUtils::Desired_Facing(int x1, int y1, int x2, int y2)
{
	DirStruct dir {};
	unsigned short value = static_cast<short>(int((Math::atan2(static_cast<double>(y2 - y1), static_cast<double>(x2 - x1)) - Math::deg2rad(-(360.0 / (USHRT_MAX - 1))))));
	dir.SetValue<16>(value);
	return dir;
}
#pragma endregion

// SHP & PCX drawing support in the same function
bool GeneralUtils::DrawImage(
	DSurface* pSurface,
	RectangleStruct destinationRect,
	BSurface* pPCXSurface,
	SHPStruct* fileSHP,
	ConvertClass* pPalette,
	int frameIndex,
	int zAdjust,
	BlitterFlags blitterFlags)
{
	if (!pSurface || (!pPCXSurface && !fileSHP))
		return false;

	bool painted = false;

	// Prioritize drawing the PCX file if it's provided
	if (pPCXSurface)
	{
		// This function handles stretching the PCX to fit the destinationRect
		PCXImages::Instance->BlitToSurface(&destinationRect, pSurface, pPCXSurface);
		painted = true;
	}
	// Otherwise, if an SHP is provided, draw it
	else if (fileSHP)
	{
		// SHP drawing requires a palette converter
		if (!pPalette)
		{
			Debug::Log("DrawImage Error: Attempted to draw SHP without providing a pPalette.\n");
			return false;
		}

		Point2D noLocation = { 0, 0 };

		CC_Draw_Shape(
			pSurface,
			pPalette,
			fileSHP,
			frameIndex,
			&noLocation,
			&destinationRect,
			BlitterFlags::None,
			0, zAdjust, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0
		);
		painted = true;
	}

	// Use the Phobos PCX instance to blit the image
	if (painted && blitterFlags == (BlitterFlags::Darken | BlitterFlags::bf_400))
	{
		auto black = ColorStruct { 0, 0, 0 };
		int opacity = 40;
		pSurface->Fill_Rect_Trans(&destinationRect, &black, opacity);
	}

	// Other new BlitterFlags cases should be placed here so both SHP & PCS will be affected
	return true;
}

std::unique_ptr<std::vector<PhobosPCXFile>> GeneralUtils::GetAnimationPCX(const std::string& baseFilename)
{
	//std::vector<PhobosPCXFile> animationFrames;
	auto animationFrames = std::make_unique<std::vector<PhobosPCXFile>>();

	//PhobosPCXFile firstPCX = PhobosPCXFile(_strdup(baseFilename.c_str()));
	PhobosPCXFile firstPCX(baseFilename.c_str());

	if (firstPCX.Exists())
	{
		// If it exists, move the temporary object into the vector.
		// This transfers ownership without copying
		animationFrames->emplace_back(std::move(firstPCX));
	}
	else
	{
		// If the first file doesn't exist, there's no animation
		return animationFrames;
	}

	animationFrames->emplace_back(baseFilename.c_str());

	std::string filenameBase = baseFilename;
	std::string extension;

	// Find the position of the last dot to separate the extension
	size_t lastDot = baseFilename.find_last_of('.');

	if (lastDot == std::string::npos)
	{
		// No extension found, e.g., "LOADOUT"
		filenameBase = baseFilename;
		extension = "";
	}
	else
	{
		// Standard case, e.g., "LOADOUT.PCX" or "LOADOUT 0000.PCX"
		filenameBase = baseFilename.substr(0, lastDot);
		extension = baseFilename.substr(lastDot);
	}

	// Now, check if the part before the extension was a frame number and remove it if so.
	// This ensures "LOADOUT 0000.PCX" correctly becomes "LOADOUT" for the sequence search
	if (filenameBase.length() > 5 && filenameBase[filenameBase.length() - 5] == ' ')
	{
		std::string frameNumberStr = filenameBase.substr(filenameBase.length() - 4);
		bool isNumeric = true;

		for (char c : frameNumberStr)
		{
			if (!isdigit(c))
			{
				isNumeric = false;
				break;
			}
		}
		if (isNumeric)
		{
			// It was a numbered file like "LOADOUT.0000".
			// The real base is the part before the frame number
			filenameBase = filenameBase.substr(0, filenameBase.length() - 5);
		}
	}

	// Loop to find and load the subsequent frames, ALWAYS starting from frame 1
	for (int i = 1; i < 10000; ++i)
	{
		char currentFilename[256];
		// Create the filename for the current frame, e.g., "LOADOUT 0001.PCX"
		_snprintf_s(currentFilename, sizeof(currentFilename), "%s %04d%s", filenameBase.c_str(), i, extension.c_str());

		//PhobosPCXFile filePCX = PhobosPCXFile(_strdup(currentFilename));
		PhobosPCXFile filePCX(currentFilename);
		// Check if the file for the current frame exists && add it into the vector
		if (filePCX.Exists())
			animationFrames->emplace_back(std::move(filePCX));
		else // The sequence is broken, so we stop searching more animation frames
			break;
	}

	return animationFrames;
}