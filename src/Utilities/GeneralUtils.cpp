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
#include <Ext/TaskForce/Body.h>
#include <Ext/TagType/Body.h>

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
		ImageHeignt = pImage->CurrentHeader.Height;

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
	SHPCaches* fileSHP,
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

std::vector<PhobosPCXFile> GeneralUtils::GetAnimationPCX(const std::string& baseFilename)
{
	// BUGFIX: return type was std::unique_ptr<std::vector<PhobosPCXFile>>.
	//   A vector is already movable, so by-value return is free (NRVO/move) and the
	//   unique_ptr only added a heap alloc + made the storage type move-only, which
	//   deleted the owning ext class's copy ops. Return by value instead.
	std::vector<PhobosPCXFile> animationFrames;
 
	// Frame 0: the base file itself. No base file -> no animation.
	PhobosPCXFile firstPCX(baseFilename.c_str());
	if (!firstPCX.Exists())
		return animationFrames; // empty
 
	animationFrames.emplace_back(std::move(firstPCX));
	// BUGFIX: removed a second `emplace_back(baseFilename.c_str())` that ran here and
	//         added frame 0 a SECOND time whenever the base file existed.
 
	// Split "base" and ".ext".
	std::string filenameBase;
	std::string extension;
 
	const size_t lastDot = baseFilename.find_last_of('.');
	if (lastDot == std::string::npos)
	{
		filenameBase = baseFilename; // e.g. "LOADOUT"
	}
	else
	{
		filenameBase = baseFilename.substr(0, lastDot); // "LOADOUT" / "LOADOUT 0000"
		extension = baseFilename.substr(lastDot);       // ".PCX"
	}
 
	// Strip a trailing " NNNN" frame index, e.g. "LOADOUT 0000" -> "LOADOUT",
	// so the sequence search always restarts from frame 1.
	if (filenameBase.length() > 5 && filenameBase[filenameBase.length() - 5] == ' ')
	{
		const std::string frameNumberStr = filenameBase.substr(filenameBase.length() - 4);
 
		bool isNumeric = true;
		for (const char c : frameNumberStr)
		{
			// cast to unsigned char: isdigit() is UB on negative char values
			if (!std::isdigit(static_cast<unsigned char>(c)))
			{
				isNumeric = false;
				break;
			}
		}
 
		if (isNumeric)
			filenameBase = filenameBase.substr(0, filenameBase.length() - 5);
	}
 
	// Frames 1..N: stop at the first missing frame.
	for (int i = 1; i < 10000; ++i)
	{
		// original format string: "%s %04d%s"
		const std::string currentFilename = fmt::format("{} {:04d}{}", filenameBase, i, extension);
 
		PhobosPCXFile filePCX(currentFilename.c_str());
		if (!filePCX.Exists())
			break; // sequence broken -> done
 
		animationFrames.emplace_back(std::move(filePCX));
	}
 
	return animationFrames;
}

const std::vector<CellStruct>  GeneralUtils::GetFoundationCells(const BuildingClass* const pThis, CellStruct const baseCoords, bool includeOccupyHeight)
{
	const CellStruct foundationEnd = { 0x7FFF, 0x7FFF };
	CellStruct const* pFoundation = pThis->GetFoundationData(false);

	int occupyHeight = includeOccupyHeight ? pThis->Type->OccupyHeight : 1;

	if (occupyHeight <= 0)
		occupyHeight = 1;

	const CellStruct* pCellIterator = pFoundation;

	while (*pCellIterator != foundationEnd)
		++pCellIterator;

	std::vector<CellStruct> foundationCells;
	foundationCells.reserve(static_cast<int>(std::distance(pFoundation, pCellIterator + 1)) * occupyHeight);
	pCellIterator = pFoundation;

	while (*pCellIterator != foundationEnd)
	{
		auto actualCell = baseCoords + *pCellIterator;

		for (auto i = occupyHeight; i > 0; --i)
		{
			foundationCells.emplace_back(actualCell);
			--actualCell.X;
			--actualCell.Y;
		}
		++pCellIterator;
	}

	std::sort(foundationCells.begin(), foundationCells.end(),
		[](const CellStruct& lhs, const CellStruct& rhs) -> bool
	{
		return lhs.X > rhs.X || lhs.X == rhs.X && lhs.Y > rhs.Y;
	});

	auto const it = std::unique(foundationCells.begin(), foundationCells.end());
	foundationCells.erase(it, foundationCells.end());

	return foundationCells;
}

bool  GeneralUtils::IsTechnoNearCell(const TechnoClass* pTechno, const CellStruct& targetCell, int distanceCells)
{
	if (!pTechno || !pTechno->IsAlive || pTechno->Health <= 0)
		return false;

	if (const BuildingClass* pBuilding = cast_to<const BuildingClass*>(pTechno))
	{
		const std::vector<CellStruct> foundationCells = GetFoundationCells(
		pBuilding,
		pBuilding->GetCell()->MapCoords,
		false
		);

		for (const CellStruct& cell : foundationCells)
		{
			int dx = cell.X - targetCell.X;
			int dy = cell.Y - targetCell.Y;
			int distSquared = dx * dx + dy * dy;
			if (distSquared <= distanceCells * distanceCells)
				return true;
		}
		return false;
	}

	else // not building
	{
		CellStruct technoCell = CellClass::Coord2Cell(pTechno->GetCoords());
		int dx = technoCell.X - targetCell.X;
		int dy = technoCell.Y - targetCell.Y;
		int distSquared = dx * dx + dy * dy;

		return distSquared <= distanceCells * distanceCells;
	}
}

bool  GeneralUtils::IsCellInBuildingFoundation(const BuildingClass* const pBuilding, const CellStruct& cell)
{
	if (!pBuilding || !pBuilding->Type) return false;
	if (pBuilding->WhatAmI() != AbstractType::Building) return false;

	const std::vector<CellStruct> foundationCells = GetFoundationCells(
		pBuilding,
		pBuilding->GetCell()->MapCoords,
		false
	);

	auto it = std::find(foundationCells.begin(), foundationCells.end(), cell);
	return it != foundationCells.end();
}

TagClass* GeneralUtils::GetTagClassByIndex(int Index, bool forceNew)
{
	std::string tagIndex = "0" + std::to_string(Index);
	TagTypeClass* pTagType = TagTypeClass::FindByNameOrID(tagIndex.c_str());
	if (!pTagType) return nullptr;

	if (forceNew)
	{
		TagClass* pNewTag = GameCreate<TagClass>(pTagType);
		return pNewTag;
	}
	else
	{
		return TagClass::GetInstance(pTagType);
	}
}

//These map cells are what SpySat skips revealing in MP normally.
bool GeneralUtils::IsCellInvalidForReveal(CellStruct* pMapCell)
{
	const int x = pMapCell->X;
	const int y = pMapCell->Y;
	auto const& rect = MapClass::Instance->MapRect;

	if (x == 7 && y == rect.Width + 5)
		return true;

	if (x == 13 && y == rect.Width + 11)
		return true;

	if (x == rect.Height + 13 && y == rect.Width + rect.Height - 15)
		return true;

	return false;
}

bool GeneralUtils::HasZoneConnection(HouseClass* pOwner, HouseClass* pEnemy, MovementZone mz)
{
	if (mz == MovementZone::Fly)
		return true;

	auto const ownerCell = pOwner->GetBaseCenter();
	auto const enemyCell = pEnemy->GetBaseCenter();

	ZoneType ownerZone = MapClass::Instance->GetMovementZoneType(ownerCell, mz, false);
	ZoneType enemyZone = MapClass::Instance->GetMovementZoneType(enemyCell, mz, false);

	if (ownerZone < ZoneType::Core || enemyZone < ZoneType::Core)
		return false;

	return ownerZone == enemyZone;
}

bool GeneralUtils::CheckTaskForceZoneConnection(HouseClass* pOwner, HouseClass* pEnemy, TaskForceClass* pTaskForce, bool requireAll)
{
	if (!pTaskForce || pTaskForce->CountEntries <= 0)
		return true;

	int checkedCount = 0;
	int connectedCount = 0;

	for (int i = 0; i < pTaskForce->CountEntries && i < 6; ++i)
	{
		auto const pType = pTaskForce->Entries[i].Type;
		if (!pType || pTaskForce->Entries[i].Amount <= 0)
			continue;

		++checkedCount;
		auto const mz = pType->MovementZone;
		auto const connected = HasZoneConnection(pOwner, pEnemy, mz);

		if (connected)
			++connectedCount;
	}

	if (checkedCount == 0)
		return true;

	return requireAll ? (connectedCount == checkedCount) : (connectedCount > 0);
}
