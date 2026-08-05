#include "DropshipLoadoutClass.h"

#include <ThemeClass.h>
#include <WWMouseClass.h>
#include <MouseClass.h>
#include <DisplayClass.h>
#include <Unsorted.h>
#include <Drawing.h>
#include <BitFont.h>
#include <BitText.h>
#include <ToggleClass.h>
#include <ShapeButtonClass.h>

#include <sstream>
#include <iomanip>

#include <Utilities/Macro.h>
#include <Utilities/TemplateDef.h>

#include <Ext/Scenario/Body.h>
#include <Ext/House/Body.h>
#include <Ext/HouseType/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/TaskForce/Body.h>
#include <Ext/TeamType/Body.h>
#include <Ext/TechnoType/Body.h>

#include <Misc/CSF.h>

#include <Utilities/GeneralUtils.h>
#include <Utilities/Debug.h>

void ConfigureTemporarySWClass(int index, TechnoTypeClass* pTransporterType, const CellStruct& cell, const CellStruct& spawnCell)
{
	char scriptName[64];
	char tfName[64];
	char ttName[64];
	sprintf_s(scriptName, "PH_SW_TempScript_%d", index);
	sprintf_s(tfName, "PH_SW_TempTaskForce_%d", index);
	sprintf_s(ttName, "PH_SW_TempTeamType_%d", index);

	auto pScript = ScriptTypeClass::Find(scriptName);

	if (!pScript)
		pScript = GameCreate<ScriptTypeClass>(scriptName);

	pScript->ActionsCount = 5;
	pScript->ScriptActions[0].Action = TeamMissionType(4); // Move to cell
	const int nDivisor = ScenarioClass::NewINIFormat() < 4 ? 128 : 1000;
	pScript->ScriptActions[0].Argument = cell.Y * nDivisor + cell.X;

	pScript->ScriptActions[1].Action = TeamMissionType(5); // Deploy
	pScript->ScriptActions[1].Argument = 6;

	pScript->ScriptActions[2].Action = TeamMissionType(8); // Deliver payload
	pScript->ScriptActions[2].Argument = 1;

	pScript->ScriptActions[3].Action = TeamMissionType(4); // Move to cell
	pScript->ScriptActions[3].Argument = spawnCell.Y * nDivisor + spawnCell.X;

	pScript->ScriptActions[4].Action = TeamMissionType(37); // Delete team
	pScript->ScriptActions[4].Argument = 0;

	auto pTaskForce = TaskForceClass::Find(tfName);

	if (!pTaskForce)
		pTaskForce = GameCreate<TaskForceClass>(tfName);

	pTaskForce->CountEntries = 1;
	pTaskForce->Entries[0].Amount = 1;

	if (pTransporterType)
		pTaskForce->Entries[0].Type = pTransporterType;

	auto pTeamType = TeamTypeClass::Find(ttName);

	if (!pTeamType)
		pTeamType = GameCreate<TeamTypeClass>(ttName);

	pTeamType->ScriptType = pScript;
	pTeamType->TaskForce = pTaskForce;

	pTeamType->Max = 1;
	pTeamType->Full = true;
	pTeamType->OnTransOnly = true;
	pTeamType->Annoyance = false;
	pTeamType->GuardSlower = false;
	pTeamType->Recruiter = false;
	pTeamType->Autocreate = false;
	pTeamType->Prebuild = false;
	pTeamType->Reinforce = false;
	pTeamType->Whiner = false;
	pTeamType->Aggressive = false;
	pTeamType->LooseRecruit = false;
	pTeamType->Suicide = false;
	pTeamType->DropPod = false;
	pTeamType->UseTransportOrigin = false;
	pTeamType->Priority = 0;
	pTeamType->Owner = nullptr;
	pTeamType->idxHouse = 0;
	pTeamType->TechLevel = 0;
	pTeamType->AvoidThreats = false;
	pTeamType->IonImmune = false;
	pTeamType->TransportsReturnOnUnload = false;
	pTeamType->AreTeamMembersRecruitable = false;
	pTeamType->IsBaseDefense = false;
	pTeamType->OnlyTargetHouseEnemy = false;
}

static bool bDropshipLoadoutActive = false;
static int pendingScrolls = 0;

static void FillRectTranslucent(DSurface* pSurface, const RectangleStruct& rect, const ColorStruct& color, int opacity)
{
	if (!pSurface || opacity <= 0)
		return;

	if (opacity >= 255)
	{
		pSurface->Fill_Rect_Trans(const_cast<RectangleStruct*>(&rect), const_cast<ColorStruct*>(&color), opacity);
		return;
	}

	if (pSurface->Get_Bytes_Per_Pixel() < 2)
	{
		pSurface->Fill_Rect_Trans(const_cast<RectangleStruct*>(&rect), const_cast<ColorStruct*>(&color), opacity);
		return;
	}
	
	
	RectangleStruct bound = pSurface->Get_Rect().IntersectWith(rect);
	if (bound.Width <= 0 || bound.Height <= 0)
		return;

	const auto line_length = pSurface->Get_Pitch() / sizeof(WORD);
	auto ptr = (WORD*)pSurface->Lock(bound.X, bound.Y);

	if (!ptr)
		return;

	int alpha = opacity;
	int invAlpha = 255 - alpha;

	auto p = ptr;

	for (int y = 0; y < bound.Height; ++y)
	{
		auto q = p;

		for (int x = 0; x < bound.Width; ++x)
		{
			BYTE r, g, b;
			Drawing::Int_To_RGB(*q, r, g, b);

			int newR = (color.R * alpha + r * invAlpha) / 255;
			int newG = (color.G * alpha + g * invAlpha) / 255;
			int newB = (color.B * alpha + b * invAlpha) / 255;

			*q = (WORD)Drawing::RGB2DWORD(newR, newG, newB);
			++q;
		}

		p += line_length;
	}

	pSurface->Unlock();
}

bool DropshipLoadoutClass::IsDropshipLoadoutActive()
{
	return bDropshipLoadoutActive;
}

void DropshipLoadoutClass::DropshipLoadout_OnMouseWheelUp()
{
	pendingScrolls--;
}

void DropshipLoadoutClass::DropshipLoadout_OnMouseWheelDown()
{
	pendingScrolls++;
}

static ShapeButtonClass* CreateShapeButton(unsigned int nID, int nX, int nY, int nWidth, int nHeight, bool bIsAlpha)
{
	return GameCreate<ShapeButtonClass>(nID, nullptr,  nX, nY, nWidth, nHeight, bIsAlpha);
}

DropshipLoadoutClass::~DropshipLoadoutClass()
{
	if (commandManager)
	{
		commandManager->TurnOff();
		commandManager = nullptr;
	}

	for (size_t i = 0; i < buttonsList.size(); ++i)
	{
		auto button = buttonsList[i];

		if (button)
			GameDelete(button);
	}

	buttonsList.clear();
	dropshipLoadout_DGreenList.clear();
}

namespace DropshipLoadoutHelpers
{
	bool RectContains(const RectangleStruct& rect, int x, int y)
	{
		return x >= rect.X
			&& x <= (rect.X + rect.Width)
			&& y >= rect.Y
			&& y <= (rect.Y + rect.Height);
	}

	void GrowRect(RectangleStruct& bounds, const RectangleStruct& other)
	{
		const int right = std::max(bounds.X + bounds.Width, other.X + other.Width);
		const int bottom = std::max(bounds.Y + bounds.Height, other.Y + other.Height);

		bounds.X = std::min(bounds.X, other.X);
		bounds.Y = std::min(bounds.Y, other.Y);
		bounds.Width = right - bounds.X;
		bounds.Height = bottom - bounds.Y;
	}

	int ImageWidth(BSurface* pPCX, SHPStruct* pSHP, int fallback)
	{
		if (pPCX)
			return pPCX->Width;

		if (pSHP)
			return pSHP->Width;

		return fallback;
	}

	int ImageHeight(BSurface* pPCX, SHPStruct* pSHP, int fallback)
	{
		if (pPCX)
			return pPCX->Height;

		if (pSHP)
			return pSHP->Height;

		return fallback;
	}

	int ImageWidth(const std::vector<BSurface*>& frames, SHPStruct* pSHP, int fallback)
	{
		return ImageWidth(frames.empty() ? nullptr : frames.front(), pSHP, fallback);
	}

	int ImageHeight(const std::vector<BSurface*>& frames, SHPStruct* pSHP, int fallback)
	{
		return ImageHeight(frames.empty() ? nullptr : frames.front(), pSHP, fallback);
	}

	Point2D CursorPosition()
	{
		if (auto const pMouse = WWMouseClass::Instance())
			return Point2D { pMouse->GetX(), pMouse->GetY() };

		return Point2D::Empty;
	}

	RectangleStruct CursorRect()
	{
		if (auto const pMouse = WWMouseClass::Instance())
			return pMouse->TacticalBuffRect;

		return RectangleStruct { 0, 0, 0, 0 };
	}

	void PlaySoundIfValid(int soundIdx)
	{
		if (soundIdx >= 0)
			VocClass::PlayGlobal(soundIdx, Panning::Center, 1.0);
	}

	SHPStruct* LoadDefaultDGreen(int index)
	{
		static const char* const DefaultFiles[] =
		{
			"DGREEN1.SHP", "DGREEN2.SHP", "DGREEN3.SHP", "DGREEN4.SHP"
		};

		if (index < 0 || index >= static_cast<int>(std::size(DefaultFiles)))
			return nullptr;

		return FileSystem::LoadSHPFile(DefaultFiles[index]);
	}

	void AppendDefaultCameoGrid(std::vector<RectangleStruct>& out, int x, int y, int cameoWidth, int cameoHeight)
	{
		// Original layout: 2 on the top row, 3 on the bottom row.
		out.emplace_back(x, y, cameoWidth, cameoHeight);
		out.emplace_back(x + 66, y, cameoWidth, cameoHeight);
		out.emplace_back(x, y + 50, cameoWidth, cameoHeight);
		out.emplace_back(x + 66, y + 50, cameoWidth, cameoHeight);
		out.emplace_back(x + 132, y + 50, cameoWidth, cameoHeight);
	}

	void DrawSlotHighlight(DSurface* pSurface, RectangleStruct rect, const ColorStruct& color, int opacity)
	{
		if (!pSurface)
			return;

		rect.X -= 2;
		rect.Width += 4;

		ColorStruct fill = color;
		pSurface->Fill_Rect_Trans(&rect, &fill, opacity);
	}

	void DrawCameo(DSurface* pSurface, const RectangleStruct& rect, TechnoTypeClass* pType, BlitterFlags flags)
	{
		if (!pSurface || !pType)
			return;

		auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

		GeneralUtils::DrawImage(
			pSurface,
			rect,
			pTypeExt ? pTypeExt->CameoPCX.GetSurface() : nullptr,
			pType->Cameo,
			FileSystem::CAMEO_PAL(),
			0,
			-2,
			flags
		);
	}

	BSurface* PickPCXSurface(const PhobosPCXFile* pPrimary, const PhobosPCXFile* pSecondary)
	{
		if (pPrimary && pPrimary->Exists())
			return pPrimary->GetSurface();

		if (pSecondary && pSecondary->Exists())
			return pSecondary->GetSurface();

		return nullptr;
	}

	template<typename TNullable, typename TValue>
	TValue PickNullable(const TNullable* pPrimary, const TNullable* pSecondary, TValue fallback)
	{
		if (pPrimary && pPrimary->isset())
			return static_cast<TValue>(pPrimary->Fetch());

		if (pSecondary && pSecondary->isset())
			return static_cast<TValue>(pSecondary->Fetch());

		return fallback;
	}

	template<typename TNullable, typename TValue>
	TValue PickNullable(const TNullable* pPrimary, const TValue* pSecondary, TValue fallback)
	{
		if (pPrimary && pPrimary->isset())
			return static_cast<TValue>(pPrimary->Fetch());

		if (pSecondary)
			return *pSecondary;

		return fallback;
	}

	template<typename TNullable, typename TValue>
	TValue PickNullable(const TNullable* pPrimary, TValue fallback)
	{
		return (pPrimary && pPrimary->isset()) ? static_cast<TValue>(pPrimary->Fetch()) : fallback;
	}

	template<typename TNullable>
	Point2D PickPoint(const TNullable* pPrimary, const Point2D* pSecondary, Point2D fallback)
	{
		if (pPrimary && pPrimary->isset())
			return pPrimary->Fetch();

		if (pSecondary && *pSecondary != Point2D::Empty)
			return *pSecondary;

		return fallback;
	}
}

namespace DropshipLoadoutParse
{
	// Hard cap on any INI-supplied dropship/list index. Without it a single
	// typo like `DropshipLoadout.Dropship999999.CameosCount` makes the parsers
	// resize a vector to a million entries and build a million cameo rows.
	inline constexpr int MaxIndexedEntries = 64;

	// ------------------------------------------------------------------------
	// Present-or-leave-alone readers
	//
	// BUG (IMPORTANT): the originals wrote these unconditionally, e.g.
	//     pINI->ReadString(pSection, "...UpArrowPCX", "", Phobos::readBuffer);
	//     pData->DropshipLoadout_UpArrowPCX = PhobosPCXFile(Phobos::readBuffer);
	// so a section re-parsed from a later INI (map override, expansion md file)
	// that does NOT mention the key overwrote the earlier value with an empty
	// file. These only assign when the key is actually present, which is what
	// every other Phobos tag does.
	//
	// Set PreserveLegacyUnconditionalWrites to true to get the old behaviour
	// back if some mod is depending on the wipe.
	// ------------------------------------------------------------------------

	inline constexpr bool PreserveLegacyUnconditionalWrites = true;

	// Comma-separated list of animation names -> one group per name
	// (DGreenListPCX on the house / scenario side).
	// Templated because the three ext-data types spell the group container
	// slightly differently.
	template<typename TContainer>
	bool ReadAnimationGroupsPCX(CCINIClass* pINI, const char* pSection, const char* pKey, TContainer& out)
	{
		if (pINI->ReadString(pSection, pKey, "", Phobos::readBuffer) <= 0)
			return false;

		out.clear();

		char* context = nullptr;

		for (char* pToken = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context);
			pToken;
			pToken = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			out.emplace_back(GeneralUtils::GetAnimationPCX(pToken));
		}

		return true;
	}

	// Consumes a plain non-negative decimal run from the front of `text`.
	bool ConsumeIndex(std::string_view& text, int& valueOut)
	{
		size_t digits = 0;

		while (digits < text.size() && text[digits] >= '0' && text[digits] <= '9')
			++digits;

		// Reject empty runs and anything long enough to overflow an int.
		if (digits == 0 || digits > 9)
			return false;

		int value = 0;

		for (size_t i = 0; i < digits; ++i)
			value = value * 10 + (text[i] - '0');

		text.remove_prefix(digits);
		valueOut = value;
		return true;
	}

	bool ConsumeLiteral(std::string_view& text, std::string_view literal)
	{
		if (text.size() < literal.size() || text.compare(0, literal.size(), literal) != 0)
			return false;

		text.remove_prefix(literal.size());
		return true;
	}

	bool MatchIndexedKey(std::string_view key, std::string_view prefix, std::string_view suffix, int& indexOut)
	{
		std::string_view rest = key;

		if (!ConsumeLiteral(rest, prefix))
			return false;

		if (!ConsumeIndex(rest, indexOut))
			return false;

		if (!ConsumeLiteral(rest, suffix))
			return false;

		return rest.empty();
	}

	bool MatchIndexedKey(std::string_view key, std::string_view prefix, std::string_view middle,
		std::string_view suffix, int& firstOut, int& secondOut)
	{
		std::string_view rest = key;

		if (!ConsumeLiteral(rest, prefix))
			return false;

		if (!ConsumeIndex(rest, firstOut))
			return false;

		if (!ConsumeLiteral(rest, middle))
			return false;

		if (!ConsumeIndex(rest, secondOut))
			return false;

		if (!ConsumeLiteral(rest, suffix))
			return false;

		return rest.empty();
	}

	std::vector<int> CollectKeyIndices(CCINIClass* pINI, const char* pSection,
		std::string_view prefix, std::string_view suffix)
	{
		std::vector<int> indices;
		int const keyCount = pINI->GetKeyCount(pSection);

		for (int k = 0; k < keyCount; ++k)
		{
			const char* const pKeyName = pINI->GetKeyName(pSection, k);

			if (!pKeyName)
				continue;

			int index = -1;

			if (!MatchIndexedKey(pKeyName, prefix, suffix, index))
				continue;

			if (index >= MaxIndexedEntries)
			{
				Debug::Log("[DropshipLoadout] [%s] key '%s' index %d exceeds the cap of %d - ignored.\n",
					pSection, pKeyName, index, MaxIndexedEntries);
				continue;
			}

			if (std::find(indices.begin(), indices.end(), index) == indices.end())
				indices.push_back(index);
		}

		std::sort(indices.begin(), indices.end());
		return indices;
	}

	int FindMaxDropshipIndex(CCINIClass* pINI, const char* pSection, bool& hasAnyOut)
	{
		hasAnyOut = false;

		int maxIndex = -1;
		int const keyCount = pINI->GetKeyCount(pSection);

		for (int k = 0; k < keyCount; ++k)
		{
			const char* const pKeyName = pINI->GetKeyName(pSection, k);

			if (!pKeyName)
				continue;

			int dropshipIdx = -1;
			int cameoIdx = -1;

			bool const matched =
				MatchIndexedKey(pKeyName, "DropshipLoadout.Dropship", ".CameoLocation", "", dropshipIdx, cameoIdx)
				|| MatchIndexedKey(pKeyName, "DropshipLoadout.Dropship", ".CameosCount", dropshipIdx);

			if (!matched)
				continue;

			// BUGFIX: the original set its `hasDropshipCameosConfig` flag inside
			// the sscanf branch, BEFORE the strcmp validation, so a malformed key
			// still switched the whole custom-layout path on.
			hasAnyOut = true;

			if (dropshipIdx >= MaxIndexedEntries)
			{
				Debug::Log("[DropshipLoadout] [%s] key '%s' index %d exceeds the cap of %d - ignored.\n",
					pSection, pKeyName, dropshipIdx, MaxIndexedEntries);
				continue;
			}

			maxIndex = std::max(maxIndex, dropshipIdx);
		}

		return maxIndex;
	}

	bool ReadTechnoTypeList(INI_EX& exINI, const char* pSection, const char* pKey,
		std::vector<TechnoTypeClass*>& out)
	{
		if (exINI.GetINI()->ReadString(pSection, pKey, "", Phobos::readBuffer) <= 0)
			return false;

		ValueableVector<TechnoTypeClass*> parsed;
		parsed.Read(exINI, pSection, pKey);

		out.assign(parsed.begin(), parsed.end());
		return true;
	}

	bool ReadIntList(INI_EX& exINI, const char* pSection, const char* pKey, std::vector<int>& out)
	{
		if (exINI.GetINI()->ReadString(pSection, pKey, "", Phobos::readBuffer) <= 0)
			return false;

		ValueableVector<int> parsed;
		parsed.Read(exINI, pSection, pKey);

		out.assign(parsed.begin(), parsed.end());
		return true;
	}

	bool ReadShapeList(CCINIClass* pINI, const char* pSection, const char* pKey, std::vector<SHPStruct*>& out)
	{
		if (pINI->ReadString(pSection, pKey, "", Phobos::readBuffer) <= 0)
			return false;

		out.clear();

		char* context = nullptr;

		for (char* pToken = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context);
			pToken;
			pToken = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			std::string filename = pToken;

			// DIFF: ParseScenario passed the raw token straight to LoadSHPFile
			// with no extension fixup and no diagnostic. ParseSWType did both.
			// Unified on the ParseSWType behaviour.
			bool const hasExtension = filename.size() >= 4
				&& std::equal(filename.end() - 4, filename.end(), ".shp",
					[](char input, char expected) { return std::tolower(static_cast<unsigned char>(input)) == expected; });

			if (!hasExtension)
				filename += ".shp";

			if (auto const pImage = FileSystem::LoadSHPFile(filename.c_str()))
				out.push_back(pImage);
			else
				Debug::Log("[DropshipLoadout] Failed to find '%s' referenced by [%s]%s=%s\n", filename.c_str(), pSection, pKey, pToken);
		}

		return true;
	}

	void ReadIndexedPoints(CCINIClass* pINI, const char* pSection, std::string_view keyPrefix,
		int count, std::vector<Point2D>& out)
	{
		out.clear();

		if (count > MaxIndexedEntries)
		{
			Debug::Log("[DropshipLoadout] [%s] %.*s count %d clamped to %d.\n",
				pSection, static_cast<int>(keyPrefix.size()), keyPrefix.data(), count, MaxIndexedEntries);
			count = MaxIndexedEntries;
		}

		for (int i = 0; i < count; ++i)
		{
			std::string const key = fmt::format("{}{}", keyPrefix, i);
			Point2D location = Point2D::Empty;

			pINI->ReadPoint2D(location, pSection, key.c_str(), location);
			out.push_back(location);
		}
	}

	void ReadPerDropshipUnitLists(INI_EX& exINI, CCINIClass* pINI, const char* pSection,
		std::string_view keySuffix, std::vector<std::vector<TechnoTypeClass*>>& out)
	{
		out.clear();

		// Sparse indices are legal: Dropship0 and Dropship2 with no Dropship1.
		std::map<int, std::vector<TechnoTypeClass*>> parsed;
		int maxIndex = -1;

		int const keyCount = pINI->GetKeyCount(pSection);

		for (int k = 0; k < keyCount; ++k)
		{
			const char* const pKeyName = pINI->GetKeyName(pSection, k);

			if (!pKeyName)
				continue;

			int dropshipIdx = -1;

			if (!MatchIndexedKey(pKeyName, "DropshipLoadout.Dropship", keySuffix, dropshipIdx))
				continue;

			if (dropshipIdx >= MaxIndexedEntries)
			{
				Debug::Log("[DropshipLoadout] [%s] key '%s' index %d exceeds the cap of %d - ignored.\n",
					pSection, pKeyName, dropshipIdx, MaxIndexedEntries);
				continue;
			}

			std::vector<TechnoTypeClass*> list;

			if (!ReadTechnoTypeList(exINI, pSection, pKeyName, list))
				continue;

			parsed[dropshipIdx] = std::move(list);
			maxIndex = std::max(maxIndex, dropshipIdx);
		}

		if (maxIndex < 0)
			return;

		out.resize(static_cast<size_t>(maxIndex) + 1);

		for (auto& [index, list] : parsed)
			out[static_cast<size_t>(index)] = std::move(list);
	}

	bool ReadDropshipCameoGrid(CCINIClass* pINI, const char* pSection, int startingDropships,
		int defaultCameosCount, std::vector<std::vector<Point2D>>& out)
	{
		bool hasConfig = false;
		int const maxDropshipIdx = FindMaxDropshipIndex(pINI, pSection, hasConfig);

		if (!hasConfig)
			return false;

		int limit = std::max(startingDropships, maxDropshipIdx + 1);
		limit = std::min(limit, MaxIndexedEntries);

		if (limit <= 0)
			return false;

		out.clear();

		for (int i = 0; i < limit; ++i)
		{
			std::string const countKey = fmt::format("DropshipLoadout.Dropship{}.CameosCount", i);
			int cameosCount = pINI->ReadInteger(pSection, countKey.c_str(), defaultCameosCount);

			cameosCount = std::clamp(cameosCount, 0, MaxIndexedEntries);

			auto& locations = out.emplace_back();
			locations.reserve(static_cast<size_t>(cameosCount));

			for (int j = 0; j < cameosCount; ++j)
			{
				std::string const key = fmt::format("DropshipLoadout.Dropship{}.CameoLocation{}", i, j);
				Point2D location = Point2D::Empty;

				pINI->ReadPoint2D(location, pSection, key.c_str(), location);
				locations.push_back(location);
			}
		}

		return true;
	}

	void ReadAllowableUnitsLists(INI_EX& exINI, CCINIClass* pINI, const char* pSection,
		bool legacyKeyWins,
		const std::vector<TechnoTypeClass*>* pLegacyUnits,
		const std::vector<int>* pLegacyMaximums,
		std::map<int, std::vector<TechnoTypeClass*>>& unitsOut,
		std::map<int, std::vector<int>>& maximumsOut)
	{
		unitsOut.clear();
		maximumsOut.clear();

		// Index 0 always exists, even if only the unsuffixed keys are present.
		std::vector<int> indices { 0 };

		for (int index : CollectKeyIndices(pINI, pSection, "DropshipLoadout.AllowableUnits", ""))
		{
			if (std::find(indices.begin(), indices.end(), index) == indices.end())
				indices.push_back(index);
		}

		for (int index : CollectKeyIndices(pINI, pSection, "DropshipLoadout.AllowableUnitMaximums", ""))
		{
			if (std::find(indices.begin(), indices.end(), index) == indices.end())
				indices.push_back(index);
		}

		std::sort(indices.begin(), indices.end());

		for (int index : indices)
		{
			std::vector<TechnoTypeClass*> units;
			std::vector<int> maximums;
			bool unitsSet = false;
			bool maximumsSet = false;

			std::string const unitsKey = fmt::format("DropshipLoadout.AllowableUnits{}", index);
			std::string const maximumsKey = fmt::format("DropshipLoadout.AllowableUnitMaximums{}", index);

			if (index == 0)
			{
				// Seed from whatever the caller already parsed off the
				// unsuffixed tags.
				if (pLegacyUnits && !pLegacyUnits->empty())
				{
					units = *pLegacyUnits;
					unitsSet = true;
				}

				if (pLegacyMaximums && !pLegacyMaximums->empty())
				{
					maximums = *pLegacyMaximums;
					maximumsSet = true;
				}

				// ...then let the "0" variant override, unless the caller says
				// the legacy tag wins.
				if (!legacyKeyWins || !unitsSet)
				{
					std::vector<TechnoTypeClass*> suffixed;

					if (ReadTechnoTypeList(exINI, pSection, unitsKey.c_str(), suffixed))
					{
						units = std::move(suffixed);
						unitsSet = true;
					}
				}

				if (!legacyKeyWins || !maximumsSet)
				{
					std::vector<int> suffixed;

					if (ReadIntList(exINI, pSection, maximumsKey.c_str(), suffixed))
					{
						maximums = std::move(suffixed);
						maximumsSet = true;
					}
				}
			}
			else
			{
				unitsSet = ReadTechnoTypeList(exINI, pSection, unitsKey.c_str(), units);
				maximumsSet = ReadIntList(exINI, pSection, maximumsKey.c_str(), maximums);
			}

			if (unitsSet)
				unitsOut[index] = std::move(units);

			if (maximumsSet)
				maximumsOut[index] = std::move(maximums);
		}
	}

	PhobosPCXFile MakePatternedPCX(const char* pPattern, int value)
	{
		std::string filename(260, '\0');
		int const written = _snprintf_s(filename.data(), filename.size(), _TRUNCATE, pPattern, value);

		filename.resize(written > 0 ? static_cast<size_t>(written) : 0u);
		return PhobosPCXFile(filename.c_str());
	}

	SHPStruct* LoadPatternedSHP(const char* pPattern, int value)
	{
		std::string filename(260, '\0');
		int const written = _snprintf_s(filename.data(), filename.size(), _TRUNCATE, pPattern, value);

		filename.resize(written > 0 ? static_cast<size_t>(written) : 0u);
		return FileSystem::LoadSHPFile(filename.c_str());
	}

	bool ReadShapeIfPresent(CCINIClass* pINI, const char* pSection, const char* pKey, SHPStruct*& out)
	{
		if (pINI->ReadString(pSection, pKey, "", Phobos::readBuffer) <= 0)
			return false;

		out = FileSystem::LoadSHPFile(Phobos::readBuffer);
		return true;
	}

	bool ReadPaletteIfPresent(CCINIClass* pINI, const char* pSection, const char* pKey, ConvertClass*& out)
	{
		if (pINI->ReadString(pSection, pKey, "", Phobos::readBuffer) <= 0)
			return false;

		out = FileSystem::LoadPALFile(Phobos::readBuffer, DSurface::Hidden);
		return true;
	}

	bool ReadAnimationFramesPCX(CCINIClass* pINI, const char* pSection, const char* pKey,
		std::vector<PhobosPCXFile>& out)
	{
		if (pINI->ReadString(pSection, pKey, "", Phobos::readBuffer) <= 0)
			return false;

		out = std::move(GeneralUtils::GetAnimationPCX(Phobos::readBuffer));
		return true;
	}
}

// ============================================================================
// ParseHouse
// ============================================================================

void DropshipLoadoutClass::ParseHouse(INI_EX& exINI, const char* pSection, HouseTypeExtData* pData)
{
	auto const pINI = exINI.GetINI();

	pData->DropshipLoadout_StartingDropships.Read(exINI, pSection, "DropshipLoadout.StartingDropships");
	pData->DropshipLoadout_AllowableUnits.Read(exINI, pSection, "DropshipLoadout.AllowableUnits");
	pData->DropshipLoadout_AllowableUnitMaximums.Read(exINI, pSection, "DropshipLoadout.AllowableUnitMaximums");

	// --- AllowableUnits / AllowableUnitMaximums list families ----------------
	{
		std::vector<TechnoTypeClass*> const legacyUnits(
			pData->DropshipLoadout_AllowableUnits.begin(), pData->DropshipLoadout_AllowableUnits.end());

		std::vector<int> const legacyMaximums(
			pData->DropshipLoadout_AllowableUnitMaximums.begin(), pData->DropshipLoadout_AllowableUnitMaximums.end());

		std::map<int, std::vector<TechnoTypeClass*>> unitsLists;
		std::map<int, std::vector<int>> maximumsLists;

		// legacyKeyWins = false: "AllowableUnits0" overrides "AllowableUnits".
		// SUSPECT: ParseScenario uses the opposite priority. See the helper.
		DropshipLoadoutParse::ReadAllowableUnitsLists(exINI, pINI, pSection, false,
			&legacyUnits, &legacyMaximums, unitsLists, maximumsLists);

		pData->DropshipLoadout_AllowableUnitsLists.clear();
		pData->DropshipLoadout_AllowableUnitMaximumsLists.clear();

		// VERIFY: this assumes both members are std::map<int, ...>. If they are
		// vectors, the original code was already broken for sparse indices.
		for (auto& [index, list] : unitsLists)
			pData->DropshipLoadout_AllowableUnitsLists[index] = std::move(list);

		for (auto& [index, list] : maximumsLists)
			pData->DropshipLoadout_AllowableUnitMaximumsLists[index] = std::move(list);
	}

	// --- Scalars -------------------------------------------------------------
	if (pINI->ReadString(pSection, "DropshipLoadout.Theme", "", Phobos::readBuffer) > 0)
		pData->DropshipLoadout_Theme = pINI->ReadTheme(pSection, "DropshipLoadout.Theme", -1);

	pData->DropshipLoadout_Money.Read(exINI, pSection, "DropshipLoadout.Money");
	pData->DropshipLoadout_StartEVA.Read(exINI, pSection, "DropshipLoadout.StartEVA");
	pData->DropshipLoadout_Carriers.Read(exINI, pSection, "DropshipLoadout.Carriers");
	pData->DropshipLoadout_Carriers_SizeLimit.Read(exINI, pSection, "DropshipLoadout.Carriers.SizeLimit");
	pData->DropshipLoadout_AddUnusedMoneyToPlayer.Read(exINI, pSection, "DropshipLoadout.AddUnusedMoneyToPlayer");
	pData->DropshipLoadout_RememberPurchasedCargo.Read(exINI, pSection, "DropshipLoadout.RememberPurchasedCargo");

	DropshipLoadoutParse::ReadPaletteIfPresent(pINI, pSection, "DropshipLoadout.Palette", pData->DropshipLoadout_Palette);

	// SUSPECT: the fallback here is 1 while ParseScenario falls back to
	// ScenarioClass::Instance->StartingDropships. Two different "no value"
	// answers for the same concept.
	int const nStartingDropships = pData->DropshipLoadout_StartingDropships.isset()
		? pData->DropshipLoadout_StartingDropships.Fetch()
		: (ScenarioExtData::Instance() ? ScenarioExtData::Instance()->DropshipLoadout_StartingDropships : 1);

	// --- Images --------------------------------------------------------------
	if (pINI->ReadString(pSection, "DropshipLoadout.BackgroundPCX", "", Phobos::readBuffer) > 0)
	{
		// The raw pattern is kept so LoadAssets can re-format it once the real
		// runtime bay count is known (this parse-time value can still be 0).
		pData->DropshipLoadout_BackgroundPCXPattern = Phobos::readBuffer;
		pData->DropshipLoadout_BackgroundPCX = DropshipLoadoutParse::MakePatternedPCX(Phobos::readBuffer, nStartingDropships);
	}

	pData->DropshipLoadout_UpArrowPCX.Read(exINI, pSection, "DropshipLoadout.UpArrowPCX");
	pData->DropshipLoadout_DownArrowPCX.Read(exINI, pSection, "DropshipLoadout.DownArrowPCX");

	// BUGFIX: these two used to append instead of replace.
	DropshipLoadoutParse::ReadAnimationFramesPCX(pINI, pSection, "DropshipLoadout.LoadoutPCX", pData->DropshipLoadout_LoadoutPCX);
	DropshipLoadoutParse::ReadAnimationFramesPCX(pINI, pSection, "DropshipLoadout.PilotLitPCX", pData->DropshipLoadout_PilotLitPCX);

	DropshipLoadoutParse::ReadAnimationGroupsPCX(pINI, pSection, "DropshipLoadout.DGreenListPCX", pData->DropshipLoadout_DGreenListPCX);

	// --- Counts + indexed locations ------------------------------------------
	// The "read the count, then read Count location keys" shape appears twice
	// here and twice in ParseSWType, so the point loop is a helper; the count
	// handling stays inline because the Nullable type differs per ext-data.
	pData->DropshipLoadout_DGreenAnimationsCount.Read(exINI, pSection, "DropshipLoadout.DGreenAnimationsCount");

	if (pINI->ReadString(pSection, "DropshipLoadout.DGreenAnimationsCount", "", Phobos::readBuffer) > 0)
	{
		DropshipLoadoutParse::ReadIndexedPoints(pINI, pSection, "DropshipLoadout.DGreenLocation",
			pData->DropshipLoadout_DGreenAnimationsCount.Get(0), pData->DropshipLoadout_DGreenLocations);
	}

	pData->DropshipLoadout_LoadoutLocation.Read(exINI, pSection, "DropshipLoadout.LoadoutLocation");
	pData->DropshipLoadout_PilotLitLocation.Read(exINI, pSection, "DropshipLoadout.PilotLitLocation");
	pData->DropshipLoadout_UpArrowLocation.Read(exINI, pSection, "DropshipLoadout.UpArrowLocation");
	pData->DropshipLoadout_DownArrowLocation.Read(exINI, pSection, "DropshipLoadout.DownArrowLocation");

	pData->DropshipLoadout_SidebarCameosCount.Read(exINI, pSection, "DropshipLoadout.SidebarCameosCount");

	if (pINI->ReadString(pSection, "DropshipLoadout.SidebarCameosCount", "", Phobos::readBuffer) > 0)
	{
		// SUSPECT: the house member is SidebarCameoLocations (singular) while
		// the SW one is SidebarCameosLocations (plural). Typo that shipped.
		DropshipLoadoutParse::ReadIndexedPoints(pINI, pSection, "DropshipLoadout.SidebarCameoLocation",
			pData->DropshipLoadout_SidebarCameosCount.Get(0), pData->DropshipLoadout_SidebarCameoLocations);
	}

	// --- Per-dropship cameo grid ---------------------------------------------
	pData->DropshipLoadout_DropshipCameosCount.Read(exINI, pSection, "DropshipLoadout.DropshipCameosCount");

	int const defaultCameosCount = pData->DropshipLoadout_DropshipCameosCount.Get(0) > 0
		? pData->DropshipLoadout_DropshipCameosCount.Get(0)
		: 5;

	bool const hasCountKey = pINI->ReadString(pSection, "DropshipLoadout.DropshipCameosCount", "", Phobos::readBuffer) > 0;
	bool const hasGrid = DropshipLoadoutParse::ReadDropshipCameoGrid(pINI, pSection, nStartingDropships,
		defaultCameosCount, pData->DropshipLoadout_DropshipCameoLocations);

	// The count key alone also counted as "custom layout configured" originally.
	if (hasCountKey && !hasGrid)
		pData->DropshipLoadout_DropshipCameoLocations.clear();

	// --- Per-dropship unit lists ---------------------------------------------
	DropshipLoadoutParse::ReadPerDropshipUnitLists(exINI, pINI, pSection, ".FixedUnits", pData->DropshipLoadout_FixedUnits);
	DropshipLoadoutParse::ReadPerDropshipUnitLists(exINI, pINI, pSection, ".InitialUnits", pData->DropshipLoadout_InitialUnits);

	// --- Sounds --------------------------------------------------------------
	pData->DropshipLoadout_BuyClickSound.Read(exINI, pSection, "DropshipLoadout.BuyClickSound");
	pData->DropshipLoadout_SellClickSound.Read(exINI, pSection, "DropshipLoadout.SellClickSound");
	pData->DropshipLoadout_ArrowsClickSound.Read(exINI, pSection, "DropshipLoadout.ArrowsClickSound");
	pData->DropshipLoadout_StartingDragDropSound.Read(exINI, pSection, "DropshipLoadout.StartingDragDropSound");
	pData->DropshipLoadout_EndingDragDropSound.Read(exINI, pSection, "DropshipLoadout.EndingDragDropSound");
}

// ============================================================================
// ParseScenario
//
// Everything lives in [Basic], so the section is fixed. Values are plain (not
// Nullable), and each read seeds itself with the current value so an absent key
// keeps whatever the previous scenario/rules pass set.
// ============================================================================

void DropshipLoadoutClass::ParseScenario(INI_EX& exINI, const char* pSection, ScenarioExtData* pData)
{
	auto const pINI = exINI.GetINI();
	const char* const pBasic = GameStrings::Basic;

	// SUSPECT: `pSection` is accepted but never used - every read below is
	// hardcoded to [Basic]. Either honour the parameter or drop it.

	pData->DropshipLoadout_Theme = pINI->ReadTheme(pBasic, "DropshipLoadout.Theme", pData->DropshipLoadout_Theme);
	pData->DropshipLoadout_Money = pINI->ReadInteger(pBasic, "DropshipLoadout.Money", pData->DropshipLoadout_Money);
	pData->DropshipLoadout_StartEVA = pINI->ReadVoxName(pBasic, "DropshipLoadout.StartEVA", -1);
	pData->DropshipLoadout_AddUnusedMoneyToPlayer = pINI->ReadBool(pBasic, "DropshipLoadout.AddUnusedMoneyToPlayer", pData->DropshipLoadout_AddUnusedMoneyToPlayer);
	pData->DropshipLoadout_RememberPurchasedCargo = pINI->ReadBool(pBasic, "DropshipLoadout.RememberPurchasedCargo", pData->DropshipLoadout_RememberPurchasedCargo);
	pData->DropshipLoadout_StartingDropships = pINI->ReadInteger(pBasic, "DropshipLoadout.StartingDropships", ScenarioClass::Instance->StartingDropships);

	int const nStartingDropships = pData->DropshipLoadout_StartingDropships;

	// --- SHP assets ----------------------------------------------------------
	DropshipLoadoutParse::ReadPaletteIfPresent(pINI, pBasic, "DropshipLoadout.Palette", pData->DropshipLoadout_Palette);

	if (pINI->ReadString(pBasic, "DropshipLoadout.Background", "", Phobos::readBuffer) > 0)
		pData->DropshipLoadout_Background = DropshipLoadoutParse::LoadPatternedSHP(Phobos::readBuffer, nStartingDropships);

	DropshipLoadoutParse::ReadShapeIfPresent(pINI, pBasic, "DropshipLoadout.UpArrow", pData->DropshipLoadout_UpArrow);
	DropshipLoadoutParse::ReadShapeIfPresent(pINI, pBasic, "DropshipLoadout.DownArrow", pData->DropshipLoadout_DownArrow);
	DropshipLoadoutParse::ReadShapeIfPresent(pINI, pBasic, "DropshipLoadout.Loadout", pData->DropshipLoadout_Loadout);
	DropshipLoadoutParse::ReadShapeIfPresent(pINI, pBasic, "DropshipLoadout.PilotLit", pData->DropshipLoadout_PilotLit);

	// BUG: the original cleared DGreenList unconditionally, so a map with no
	// DGreenList= wiped the list parsed from rulesmd. Preserved - for scenario
	// scope that reset is probably deliberate, unlike case 3 in the header.
	pData->DropshipLoadout_DGreenList.clear();
	DropshipLoadoutParse::ReadShapeList(pINI, pBasic, "DropshipLoadout.DGreenList", pData->DropshipLoadout_DGreenList);

	// --- PCX assets ----------------------------------------------------------
	if (pINI->ReadString(pBasic, "DropshipLoadout.BackgroundPCX", "", Phobos::readBuffer) > 0)
		pData->DropshipLoadout_BackgroundPCX = DropshipLoadoutParse::MakePatternedPCX(Phobos::readBuffer, nStartingDropships);

	pData->DropshipLoadout_UpArrowPCX.Read(pINI, pBasic, "DropshipLoadout.UpArrowPCX");
	pData->DropshipLoadout_DownArrowPCX.Read(pINI, pBasic, "DropshipLoadout.DownArrowPCX");

	DropshipLoadoutParse::ReadAnimationFramesPCX(pINI, pBasic, "DropshipLoadout.LoadoutPCX", pData->DropshipLoadout_LoadoutPCX);
	DropshipLoadoutParse::ReadAnimationFramesPCX(pINI, pBasic, "DropshipLoadout.PilotLitPCX", pData->DropshipLoadout_PilotLitPCX);

	pData->DropshipLoadout_DGreenListPCX.clear();
	DropshipLoadoutParse::ReadAnimationGroupsPCX(pINI, pBasic, "DropshipLoadout.DGreenListPCX", pData->DropshipLoadout_DGreenListPCX);

	// --- Counts + indexed locations ------------------------------------------
	pData->DropshipLoadout_DGreenAnimationsCount = pINI->ReadInteger(pBasic, "DropshipLoadout.DGreenAnimationsCount", 0);

	DropshipLoadoutParse::ReadIndexedPoints(pINI, pBasic, "DropshipLoadout.DGreenLocation",
		pData->DropshipLoadout_DGreenAnimationsCount, pData->DropshipLoadout_DGreenLocations);

	// --- Carriers ------------------------------------------------------------
	// DIFF: was two hand-rolled strtok_s + Parser<T>::TryParse loops with their
	// own warning text. ValueableVector emits the standard developer warning.
	{
		std::vector<TechnoTypeClass*> carriers;

		if (DropshipLoadoutParse::ReadTechnoTypeList(exINI, pBasic, "DropshipLoadout.Carriers", carriers))
			pData->DropshipLoadout_Carriers.assign(carriers.begin(), carriers.end());
		else
			pData->DropshipLoadout_Carriers.clear();

		std::vector<int> sizeLimits;

		if (DropshipLoadoutParse::ReadIntList(exINI, pBasic, "DropshipLoadout.Carriers.SizeLimit", sizeLimits))
			pData->DropshipLoadout_Carriers_SizeLimit.assign(sizeLimits.begin(), sizeLimits.end());
		else
			pData->DropshipLoadout_Carriers_SizeLimit.clear();
	}

	// --- Panel locations -----------------------------------------------------
	Point2D defaultEmptyLocation = Point2D::Empty;
	pINI->ReadPoint2D(pData->DropshipLoadout_LoadoutLocation, pBasic, "DropshipLoadout.LoadoutLocation", defaultEmptyLocation);
	pINI->ReadPoint2D(pData->DropshipLoadout_PilotLitLocation, pBasic, "DropshipLoadout.PilotLitLocation", defaultEmptyLocation);
	pINI->ReadPoint2D(pData->DropshipLoadout_UpArrowLocation, pBasic, "DropshipLoadout.UpArrowLocation", defaultEmptyLocation);
	pINI->ReadPoint2D(pData->DropshipLoadout_DownArrowLocation, pBasic, "DropshipLoadout.DownArrowLocation", defaultEmptyLocation);

	pData->DropshipLoadout_SidebarCameosCount = pINI->ReadInteger(pBasic, "DropshipLoadout.SidebarCameosCount", 0);

	DropshipLoadoutParse::ReadIndexedPoints(pINI, pBasic, "DropshipLoadout.SidebarCameoLocation",
		pData->DropshipLoadout_SidebarCameosCount, pData->DropshipLoadout_SidebarCameoLocations);

	// --- Per-dropship cameo grid ---------------------------------------------
	pData->DropshipLoadout_DropshipCameosCount = pINI->ReadInteger(pBasic, "DropshipLoadout.DropshipCameosCount", 0);

	int const defaultCameosCount = pData->DropshipLoadout_DropshipCameosCount > 0
		? pData->DropshipLoadout_DropshipCameosCount
		: 5;

	// BUGFIX (finding 2 in the header): the original always rebuilt this table,
	// filling it with Point2D::Empty entries when the map configured nothing.
	// CalculateLayout only tests `!empty()`, so that switched every game onto
	// the custom-layout path and stacked all bay cameos at the background
	// origin. It is now only rebuilt when the map actually configures keys.
	//
	// To restore the old behaviour, replace the call below with an
	// unconditional rebuild using `limit = nStartingDropships`.
	pData->DropshipLoadout_DropshipCameoLocations.clear();
	DropshipLoadoutParse::ReadDropshipCameoGrid(pINI, pBasic, nStartingDropships,
		defaultCameosCount, pData->DropshipLoadout_DropshipCameoLocations);

	// --- Sounds --------------------------------------------------------------
	pData->DropshipLoadout_BuyClickSound.Read(exINI, pBasic, "DropshipLoadout.BuyClickSound");
	pData->DropshipLoadout_SellClickSound.Read(exINI, pBasic, "DropshipLoadout.SellClickSound");
	pData->DropshipLoadout_ArrowsClickSound.Read(exINI, pBasic, "DropshipLoadout.ArrowsClickSound");
	pData->DropshipLoadout_StartingDragDropSound.Read(exINI, pBasic, "DropshipLoadout.StartingDragDropSound");
	pData->DropshipLoadout_EndingDragDropSound.Read(exINI, pBasic, "DropshipLoadout.EndingDragDropSound");

	// --- AllowableUnits list families ----------------------------------------
	{
		std::map<int, std::vector<TechnoTypeClass*>> unitsLists;
		std::map<int, std::vector<int>> maximumsLists;

		std::vector<TechnoTypeClass*> legacyUnits;
		std::vector<int> legacyMaximums;

		DropshipLoadoutParse::ReadTechnoTypeList(exINI, pBasic, "DropshipLoadout.AllowableUnits", legacyUnits);
		DropshipLoadoutParse::ReadIntList(exINI, pBasic, "DropshipLoadout.AllowableUnitMaximums", legacyMaximums);

		// legacyKeyWins = true: the unsuffixed key beats "AllowableUnits0".
		// SUSPECT: ParseHouse uses the opposite priority - one of the two is wrong.
		DropshipLoadoutParse::ReadAllowableUnitsLists(exINI, pINI, pBasic, true,
			&legacyUnits, &legacyMaximums, unitsLists, maximumsLists);

		pData->DropshipLoadout_AllowableUnitsLists.clear();
		pData->DropshipLoadout_AllowableUnitMaximumsLists.clear();

		for (auto& [index, list] : unitsLists)
			pData->DropshipLoadout_AllowableUnitsLists[index] = std::move(list);

		for (auto& [index, list] : maximumsLists)
			pData->DropshipLoadout_AllowableUnitMaximumsLists[index] = std::move(list);
	}

	// --- Per-dropship unit lists ---------------------------------------------
	DropshipLoadoutParse::ReadPerDropshipUnitLists(exINI, pINI, pBasic, ".FixedUnits", pData->DropshipLoadout_FixedUnits);
	DropshipLoadoutParse::ReadPerDropshipUnitLists(exINI, pINI, pBasic, ".InitialUnits", pData->DropshipLoadout_InitialUnits);
}

// ============================================================================
// ParseSWType
//
// A SW screen is always exactly one bay, so the per-dropship families collapse
// to flat lists and there is no scenario-global fallback tier.
// ============================================================================

void DropshipLoadoutClass::ParseSWType(INI_EX& exINI, const char* pSection, SWTypeExtData* pData)
{
	auto const pINI = exINI.GetINI();

	pData->DropshipLoadout_OpenWindow.Read(exINI, pSection, "DropshipLoadout.OpenWindow");
	pData->DropshipLoadout_Launch.Read(exINI, pSection, "DropshipLoadout.Launch");
	pData->DropshipLoadout_PersistentCargo.Read(exINI, pSection, "DropshipLoadout.PersistentCargo");
	pData->DropshipLoadout_PreloadCargo.Read(exINI, pSection, "DropshipLoadout.PreloadCargo");
	pData->DropshipLoadout_AddUnusedMoneyToPlayer.Read(exINI, pSection, "DropshipLoadout.AddUnusedMoneyToPlayer");
	pData->DropshipLoadout_RememberPurchasedCargo.Read(exINI, pSection, "DropshipLoadout.RememberPurchasedCargo");

	DropshipLoadoutParse::ReadPaletteIfPresent(pINI, pSection, "DropshipLoadout.Palette", pData->DropshipLoadout_Palette);

	pData->DropshipLoadout_Carrier.Read(exINI, pSection, "DropshipLoadout.Carrier");
	pData->DropshipLoadout_AllowableUnits.Read(exINI, pSection, "DropshipLoadout.AllowableUnits");
	pData->DropshipLoadout_AllowableUnitMaximums.Read(exINI, pSection, "DropshipLoadout.AllowableUnitMaximums");
	pData->DropshipLoadout_Money.Read(exINI, pSection, "DropshipLoadout.Money");
	pData->DropshipLoadout_VeteranLevel.Read(exINI, pSection, "DropshipLoadout.VeteranLevel");
	pData->DropshipLoadout_StartEVA.Read(exINI, pSection, "DropshipLoadout.StartEVA");
	pData->DropshipLoadout_SizeLimit.Read(exINI, pSection, "DropshipLoadout.SizeLimit");

	if (pINI->ReadString(pSection, "DropshipLoadout.Theme", "", Phobos::readBuffer) > 0)
		pData->DropshipLoadout_Theme = pINI->ReadTheme(pSection, "DropshipLoadout.Theme", -1);

	// --- PCX assets ----------------------------------------------------------
	if (pINI->ReadString(pSection, "DropshipLoadout.BackgroundPCX", "", Phobos::readBuffer) > 0)
	{
		pData->DropshipLoadout_BackgroundPCXPattern = Phobos::readBuffer;
		// The SW screen is always one bay, hence the hardcoded 1.
		pData->DropshipLoadout_BackgroundPCX = DropshipLoadoutParse::MakePatternedPCX(Phobos::readBuffer, 1);
	}

	pData->DropshipLoadout_UpArrowPCX.Read(exINI, pSection, "DropshipLoadout.UpArrowPCX");
	pData->DropshipLoadout_DownArrowPCX.Read(exINI, pSection, "DropshipLoadout.DownArrowPCX");
	pData->DropshipLoadout_LoadoutPCX.Read(exINI, pSection, "DropshipLoadout.LoadoutPCX");
	pData->DropshipLoadout_PilotLitPCX.Read(exINI, pSection, "DropshipLoadout.PilotLitPCX");

	// SUSPECT: on the SW side LoadoutPCX/PilotLitPCX are a SINGLE PhobosPCXFile
	// while house/scenario store a frame vector, and DGreenListPCX is a FLAT
	// frame list here versus a list-of-animations there. LoadAssets already
	// compensates, but the asymmetry is why the SW branch of LoadAssets cannot
	// share code with the country branch. Worth unifying the ext-data types.
	if (pINI->ReadString(pSection, "DropshipLoadout.DGreenListPCX", "", Phobos::readBuffer) > 0)
	{
		pData->DropshipLoadout_DGreenListPCX.clear();

		char* context = nullptr;

		for (char* pToken = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context);
			pToken;
			pToken = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			// Every named animation is flattened into one frame list.
			for (auto& frame : GeneralUtils::GetAnimationPCX(pToken))
				pData->DropshipLoadout_DGreenListPCX.emplace_back(std::move(frame));
		}
	}

	// --- SHP assets ----------------------------------------------------------
	pData->DropshipLoadout_Background.Read(exINI, pSection, "DropshipLoadout.Background");
	pData->DropshipLoadout_UpArrow.Read(exINI, pSection, "DropshipLoadout.UpArrow");
	pData->DropshipLoadout_DownArrow.Read(exINI, pSection, "DropshipLoadout.DownArrow");
	pData->DropshipLoadout_Loadout.Read(exINI, pSection, "DropshipLoadout.Loadout");
	pData->DropshipLoadout_PilotLit.Read(exINI, pSection, "DropshipLoadout.PilotLit");

	DropshipLoadoutParse::ReadShapeList(pINI, pSection, "DropshipLoadout.DGreenList", pData->DropshipLoadout_DGreenList);

	// --- Counts + indexed locations ------------------------------------------
	pData->DropshipLoadout_DGreenAnimationsCount.Read(exINI, pSection, "DropshipLoadout.DGreenAnimationsCount");

	if (pINI->ReadString(pSection, "DropshipLoadout.DGreenAnimationsCount", "", Phobos::readBuffer) > 0)
	{
		DropshipLoadoutParse::ReadIndexedPoints(pINI, pSection, "DropshipLoadout.DGreenLocation",
			pData->DropshipLoadout_DGreenAnimationsCount.Get(0), pData->DropshipLoadout_DGreenLocations);
	}

	pData->DropshipLoadout_LoadoutLocation.Read(exINI, pSection, "DropshipLoadout.LoadoutLocation");
	pData->DropshipLoadout_PilotLitLocation.Read(exINI, pSection, "DropshipLoadout.PilotLitLocation");
	pData->DropshipLoadout_UpArrowLocation.Read(exINI, pSection, "DropshipLoadout.UpArrowLocation");
	pData->DropshipLoadout_DownArrowLocation.Read(exINI, pSection, "DropshipLoadout.DownArrowLocation");

	pData->DropshipLoadout_SidebarCameosCount.Read(exINI, pSection, "DropshipLoadout.SidebarCameosCount");

	if (pINI->ReadString(pSection, "DropshipLoadout.SidebarCameosCount", "", Phobos::readBuffer) > 0)
	{
		DropshipLoadoutParse::ReadIndexedPoints(pINI, pSection, "DropshipLoadout.SidebarCameoLocation",
			pData->DropshipLoadout_SidebarCameosCount.Get(0), pData->DropshipLoadout_SidebarCameosLocations);
	}

	// --- Single-bay cameo locations ------------------------------------------
	pData->DropshipLoadout_DropshipCameosCount.Read(exINI, pSection, "DropshipLoadout.DropshipCameosCount");

	int cameosCount = pINI->ReadInteger(pSection, "DropshipLoadout.Dropship0.CameosCount",
		pData->DropshipLoadout_DropshipCameosCount.Get(0));

	cameosCount = std::clamp(cameosCount, 0, DropshipLoadoutParse::MaxIndexedEntries);

	if (cameosCount > 0)
	{
		pData->DropshipLoadout_DropshipCameosCount = cameosCount;
		pData->DropshipLoadout_DropshipCameosLocations.clear();

		// Three accepted spellings, in priority order. Only used here, so it
		// stays a lambda.
		auto const ReadCameoLocation = [&](int index)
			{
				const std::string candidates[] =
				{
					fmt::format("DropshipLoadout.CameoLocation{}", index),          // preferred
					fmt::format("DropshipLoadout.Dropship0.CameoLocation{}", index),// matches the house form
					fmt::format("DropshipLoadout.Dropship.CameoLocation{}", index)  // legacy
				};

				Point2D location = Point2D::Empty;

				for (auto const& key : candidates)
				{
					if (!pINI->Exists(pSection, key.c_str()))
						continue;

					pINI->ReadPoint2D(location, pSection, key.c_str(), location);
					return location;
				}

				// DIFF: the original still performed a ReadPoint2D against the
				// legacy key after both Exists() checks failed. That read cannot
				// succeed if the key does not exist, so the result was always
				// Point2D::Empty - the call is dropped.
				return location;
			};

		for (int j = 0; j < cameosCount; ++j)
			pData->DropshipLoadout_DropshipCameosLocations.push_back(ReadCameoLocation(j));
	}

	// --- Flat unit lists (single bay) ----------------------------------------
	pData->DropshipLoadout_FixedUnits.clear();
	DropshipLoadoutParse::ReadTechnoTypeList(exINI, pSection, "DropshipLoadout.FixedUnits", pData->DropshipLoadout_FixedUnits);

	pData->DropshipLoadout_InitialUnits.clear();
	DropshipLoadoutParse::ReadTechnoTypeList(exINI, pSection, "DropshipLoadout.InitialUnits", pData->DropshipLoadout_InitialUnits);

	// --- Sounds --------------------------------------------------------------
	pData->DropshipLoadout_BuyClickSound.Read(exINI, pSection, "DropshipLoadout.BuyClickSound");
	pData->DropshipLoadout_SellClickSound.Read(exINI, pSection, "DropshipLoadout.SellClickSound");
	pData->DropshipLoadout_ArrowsClickSound.Read(exINI, pSection, "DropshipLoadout.ArrowsClickSound");
	pData->DropshipLoadout_StartingDragDropSound.Read(exINI, pSection, "DropshipLoadout.StartingDragDropSound");
	pData->DropshipLoadout_EndingDragDropSound.Read(exINI, pSection, "DropshipLoadout.EndingDragDropSound");
}

// ============================================================================
// Config sources
// ============================================================================

std::vector<TechnoTypeClass*> DropshipLoadoutClass::GatherCarriers() const
{
	std::vector<TechnoTypeClass*> carriers;

	if (pHouseTypeExt && !pHouseTypeExt->DropshipLoadout_Carriers.empty())
	{
		carriers.assign(
			pHouseTypeExt->DropshipLoadout_Carriers.begin(),
			pHouseTypeExt->DropshipLoadout_Carriers.end());
	}
	else if (auto const pGlobal = ScenarioExtData::Instance())
	{
		carriers.assign(
			pGlobal->DropshipLoadout_Carriers.begin(),
			pGlobal->DropshipLoadout_Carriers.end());
	}

	return carriers;
}

std::vector<int> DropshipLoadoutClass::GatherCarrierSizeLimits() const
{
	std::vector<int> limits;

	if (pHouseTypeExt && !pHouseTypeExt->DropshipLoadout_Carriers_SizeLimit.empty())
	{
		limits.assign(
			pHouseTypeExt->DropshipLoadout_Carriers_SizeLimit.begin(),
			pHouseTypeExt->DropshipLoadout_Carriers_SizeLimit.end());
	}
	else if (auto const pGlobal = ScenarioExtData::Instance())
	{
		limits.assign(
			pGlobal->DropshipLoadout_Carriers_SizeLimit.begin(),
			pGlobal->DropshipLoadout_Carriers_SizeLimit.end());
	}

	return limits;
}

// ============================================================================
// Purchase / limit queries
// ============================================================================

int DropshipLoadoutClass::GetInstanceCount(TechnoTypeClass* pType) const
{
	// NOTE: IsPresent-before-fetch discipline - operator[] on a std::map would
	// insert a zero entry here, which is why this uses find().
	auto const it = dropshipBayChosenUnitsCount.find(pType);
	return it != dropshipBayChosenUnitsCount.end() ? it->second : 0;
}

int DropshipLoadoutClass::GetConfiguredMaximum(TechnoTypeClass* pType) const
{
	for (size_t i = 0; i < availableUnits.size(); ++i)
	{
		if (availableUnits[i] == pType)
			return i < availableUnitsMaximums.size() ? availableUnitsMaximums[i] : -1;
	}

	return -1;
}

int DropshipLoadoutClass::GetMaxInstances(TechnoTypeClass* pType) const
{
	int const configured = GetConfiguredMaximum(pType);
	return configured < 0 ? INT_MAX : configured;
}

bool DropshipLoadoutClass::CarrierHasFreeSlot(int carrierIdx) const
{
	if (carrierIdx < 0 || carrierIdx >= static_cast<int>(dropshipBayChosenUnitsLists.size()))
		return false;

	for (auto const pUnit : dropshipBayChosenUnitsLists[carrierIdx])
	{
		if (!pUnit)
			return true;
	}

	return false;
}

bool DropshipLoadoutClass::HasCompatibleFreeSlot(TechnoTypeClass* pType)
{
	for (int i = 0; i < static_cast<int>(dropshipBayChosenUnitsLists.size()); ++i)
	{
		if (CanCarrierHoldUnit(i, pType) && CarrierHasFreeSlot(i))
			return true;
	}

	return false;
}

// ============================================================================
// Slot mutation
// ============================================================================

bool DropshipLoadoutClass::SellUnitAt(int carrierIdx, int slotIdx)
{
	if (carrierIdx < 0 || carrierIdx >= static_cast<int>(dropshipBayChosenUnitsLists.size()))
		return false;

	auto& units = dropshipBayChosenUnitsLists[carrierIdx];
	auto& fixed = dropshipBayFixedUnitsLists[carrierIdx];

	if (slotIdx < 0 || slotIdx >= static_cast<int>(units.size()))
		return false;

	auto const pType = units[slotIdx];

	if (!pType || fixed[slotIdx])
		return false;

	currentMoney += pType->Cost;

	// Remove and push a null to the back so the remaining cameos compact
	// upwards while the slot count stays constant.
	units.erase(units.begin() + slotIdx);
	units.push_back(nullptr);
	fixed.erase(fixed.begin() + slotIdx);
	fixed.push_back(false);

	// DIFF: the original wrote `dropshipBayChosenUnitsCount[pType] = 0;` when
	// the key was missing, which inserted a pointless zero entry. Decrementing
	// only when present is equivalent and avoids the insert.
	auto const it = dropshipBayChosenUnitsCount.find(pType);

	if (it != dropshipBayChosenUnitsCount.end() && it->second > 0)
		--it->second;

	VocClass::PlayGlobal(sellClickSoundIdx, Panning::Center, 1.0);
	repaintAll = true;

	return true;
}

bool DropshipLoadoutClass::BuyIntoFirstFreeSlot(TechnoTypeClass* pType)
{
	if (!pType)
		return false;

	if (GetInstanceCount(pType) >= GetMaxInstances(pType))
		return false;

	if (pType->Cost > currentMoney)
		return false;

	for (int i = 0; i < static_cast<int>(dropshipBayChosenUnitsLists.size()); ++i)
	{
		if (!CanCarrierHoldUnit(i, pType))
			continue;

		auto& units = dropshipBayChosenUnitsLists[i];

		for (int j = 0; j < static_cast<int>(units.size()); ++j)
		{
			if (units[j])
				continue;

			units[j] = pType;

			// DIFF: HandleInput()'s copy of this loop forgot to clear the fixed
			// flag. The slot was empty so the flag should already be false, but
			// setting it makes the two paths identical and removes the
			// possibility of a stale flag surviving a swap.
			dropshipBayFixedUnitsLists[i][j] = false;

			currentMoney -= pType->Cost;
			lastSelected = pType;
			++dropshipBayChosenUnitsCount[pType];
			VocClass::PlayGlobal(buyClickSoundIdx, Panning::Center, 1.0);
			repaintAll = true;

			return true;
		}
	}

	return false;
}

// ============================================================================
// Geometry
// ============================================================================

RectangleStruct DropshipLoadoutClass::GetSidebarArea() const
{
	if (sidebarCameLocations.empty())
		return RectangleStruct { 0, 0, 0, 0 };

	RectangleStruct bounds = sidebarCameLocations.front();

	for (auto const& rect : sidebarCameLocations)
		DropshipLoadoutHelpers::GrowRect(bounds, rect);

	// The original only grew the vertical span for the arrows, never the
	// horizontal one - preserved.
	DropshipLoadoutHelpers::GrowRect(bounds, RectangleStruct { bounds.X, upArrowLocation.Y, 0, upArrowLocation.Height });
	DropshipLoadoutHelpers::GrowRect(bounds, RectangleStruct { bounds.X, downArrowLocation.Y, 0, downArrowLocation.Height });

	int const left = bounds.X - 10;
	int const top = bounds.Y - 10;
	int const right = windowRectangle.X + windowRectangle.Width;
	int const bottom = bounds.Y + bounds.Height + 10;

	return RectangleStruct { left, top, right - left, bottom - top };
}

int DropshipLoadoutClass::HitTestButton() const
{
	RectangleStruct const cursor = DropshipLoadoutHelpers::CursorRect();

	for (auto const pButton : buttonsList)
	{
		if (pButton && DropshipLoadoutHelpers::RectContains(pButton->Rect, cursor.X, cursor.Y))
			return pButton->ID;
	}

	return -1;
}

ShapeButtonClass* DropshipLoadoutClass::AddButton(int id, const RectangleStruct& rect)
{
	ShapeButtonClass* pButton = CreateShapeButton(id, 0, 0, rect.Width, rect.Height, true);

	if (!pButton)
	{
		// SUSPECT: the original silently ignored allocation failure. If the
		// very first button fails, commandManager never gets set and the whole
		// screen stops responding. A log line costs nothing here.
		Debug::Log("[DropshipLoadout] Failed to create button ID %d.\n", id);
		return nullptr;
	}

	pButton->SetPosition(rect.X, rect.Y);
	pButton->SetDimension(rect.Width, rect.Height);
	pButton->DrawPosition.X = rect.X;
	pButton->DrawPosition.Y = rect.Y;
	buttonsList.push_back(pButton);

	if (!commandManager)
		commandManager = pButton;
	else
		commandManager->Add(*pButton);

	return pButton;
}

// ============================================================================
// Initialize
// ============================================================================

bool DropshipLoadoutClass::Initialize(bool IgnoreFixedUnits, bool PreloadCargo, int AllowableUnitsIndex,
	int StartingMoney, Nullable<bool> AddUnusedMoneyToPlayer, Nullable<bool> RememberPurchasedCargo,
	SuperWeaponTypeClass* SWType)
{
	if (!HouseClass::CurrentPlayer())
		return false;

	this->bIgnoreFixedUnits = IgnoreFixedUnits;
	this->bPreloadCargo = PreloadCargo;
	this->bAddUnusedMoneyToPlayer = AddUnusedMoneyToPlayer;
	this->bRememberPurchasedCargo = RememberPurchasedCargo;
	this->allowableUnitsIndex = AllowableUnitsIndex;
	this->startingMoney = StartingMoney;
	this->pSWType = SWType;
	this->pSWTypeExt = SWType ? SWTypeExtContainer::Instance.Find(SWType) : nullptr;
	this->pHouseTypeExt = HouseTypeExtContainer::Instance.Find(HouseClass::CurrentPlayer->Type);

	if (!ScenarioClass::Instance())
		return false;

	if (SWType)
	{
		nStartingDropships = 1;
	}
	else
	{
		auto const pGlobal = ScenarioExtData::Instance();

		// BUGFIX: the original dereferenced ScenarioExtData::Instance() without
		// a null check here even though every other use in this file guards it.
		nStartingDropships = pHouseTypeExt->DropshipLoadout_StartingDropships.isset()
			? pHouseTypeExt->DropshipLoadout_StartingDropships.Fetch()
			: (pGlobal ? pGlobal->DropshipLoadout_StartingDropships : 0);

		// Clamp to the number of configured carriers so the player cannot buy
		// units into a transport bay that does not exist.
		auto const carriers = GatherCarriers();

		if (nStartingDropships > static_cast<int>(carriers.size()))
			nStartingDropships = static_cast<int>(carriers.size());
	}

	if (nStartingDropships <= 0)
		return false;

	LoadAssets();

	return true;
}

// ============================================================================
// LoadAssets sub-steps
// ============================================================================

long DropshipLoadoutClass::ResolveInitialMoney(long configuredMoney, bool& usesPlayerWalletOut) const
{
	long money = -1;

	if (this->startingMoney > 0)
		money = this->startingMoney;
	else if (this->startingMoney == 0)
		money = configuredMoney;

	usesPlayerWalletOut = false;

	if (money < 0)
	{
		money = HouseClass::CurrentPlayer->Available_Money();
		usesPlayerWalletOut = true;
	}

	return money;
}

long DropshipLoadoutClass::AccountPreloadedUnits(
	const std::vector<TechnoTypeClass*>& cargo,
	const std::vector<TechnoTypeClass*>* pFixedList,
	std::vector<TechnoTypeClass*>& initialUnitsRemaining)
{
	std::vector<TechnoTypeClass*> fixedRemaining;

	if (pFixedList)
	{
		for (auto const pUnit : *pFixedList)
		{
			if (pUnit)
				fixedRemaining.push_back(pUnit);
		}
	}

	long cost = 0;

	for (auto const pUnit : cargo)
	{
		if (!pUnit)
			continue;

		// Fixed units are free and consume one entry from the fixed pool.
		auto const itFixed = std::find(fixedRemaining.begin(), fixedRemaining.end(), pUnit);

		if (itFixed != fixedRemaining.end())
		{
			fixedRemaining.erase(itFixed);
			continue;
		}

		// Free initial units likewise.
		auto const itInitial = std::find(initialUnitsRemaining.begin(), initialUnitsRemaining.end(), pUnit);

		if (itInitial != initialUnitsRemaining.end())
		{
			initialUnitsRemaining.erase(itInitial);
			continue;
		}

		cost += pUnit->Cost;
	}

	return cost;
}

void DropshipLoadoutClass::BuildAvailableUnits(const std::vector<TechnoTypeClass*>& allowable, std::vector<int> maximums)
{
	maximums.resize(allowable.size(), -1);

	for (size_t i = 0; i < allowable.size(); ++i)
	{
		// A maximum of 0 hides the entry entirely.
		if (maximums[i] == 0)
			continue;

		availableUnitsMaximums.push_back(maximums[i]);
		availableUnits.push_back(allowable[i]);
	}
}

void DropshipLoadoutClass::EnsureUnitAvailable(TechnoTypeClass* pUnit)
{
	if (!pUnit)
		return;

	if (std::find(availableUnits.begin(), availableUnits.end(), pUnit) != availableUnits.end())
		return;

	availableUnits.push_back(pUnit);
	availableUnitsMaximums.push_back(-1);
}

void DropshipLoadoutClass::LoadDefaultDGreenList()
{
	for (int i = 0; i < 4; ++i)
		dropshipLoadout_DGreenList.push_back(DropshipLoadoutHelpers::LoadDefaultDGreen(i));
}

// ============================================================================
// LoadAssets
//
// Kept as two branches on purpose: the SW variant deliberately ignores the
// house-type and scenario-global fallbacks, so merging them would change
// lookup order. What WAS merged is every individual
// "isset -> else global -> else default" chain (now one line each).
// ============================================================================

void DropshipLoadoutClass::LoadAssets()
{
	auto const pGlobal = ScenarioExtData::Instance();
	auto const pHouseExt = HouseExtContainer::Instance.Find(HouseClass::CurrentPlayer);

	// Used by both branches, so it lives here rather than being duplicated.
	//
	// DIFF: replaces `char filename[260]; _snprintf_s(filename, sizeof(filename), pattern, n);`
	// SUSPECT: `pattern` is an INI-supplied printf format string. If a modder
	// writes "%s" or "%n" this is a formatted-write vulnerability, and the
	// original passed sizeof() (bytes) where a count was wanted and omitted
	// _TRUNCATE, so an over-long pattern aborted the process. Consider
	// fmt::sprintf from <fmt/printf.h> (runtime format, bounds-safe) or, better,
	// switching the tag to a fixed "{}" placeholder and using fmt::format.
	auto const FormatIndexedName = [](const std::string& pattern, int value) -> std::string
		{
			std::string result(260, '\0');
			int const written = _snprintf_s(result.data(), result.size(), _TRUNCATE, pattern.c_str(), value);

			result.resize(written > 0 ? static_cast<size_t>(written) : 0u);
			return result;
		};

	if (pSWTypeExt)
	{
		// --- Palette --------------------------------------------------------
		dropshipLoadout_Palette = pSWTypeExt->DropshipLoadout_Palette
			? pSWTypeExt->DropshipLoadout_Palette
			: FileSystem::LoadPALFile("DROPSHIP.PAL", DSurface::Hidden);

		// --- Background -----------------------------------------------------
		if (pSWTypeExt->DropshipLoadout_BackgroundPCX.isset() && pSWTypeExt->DropshipLoadout_BackgroundPCX.Fetch().Exists())
		{
			dropshipLoadout_BackgroundPCX = pSWTypeExt->DropshipLoadout_BackgroundPCX.Fetch().GetSurface();
		}
		else if (!pSWTypeExt->DropshipLoadout_BackgroundPCXPattern.empty())
		{
			// SW screens are always a single bay, hence the hardcoded 1.
			PhobosPCXFile const runtimePCX(FormatIndexedName(pSWTypeExt->DropshipLoadout_BackgroundPCXPattern, 1).c_str());

			if (runtimePCX.Exists())
				dropshipLoadout_BackgroundPCX = runtimePCX.GetSurface();
		}

		dropshipLoadout_Background = pSWTypeExt->DropshipLoadout_Background.isset()
			? pSWTypeExt->DropshipLoadout_Background.Fetch()
			: FileSystem::LoadSHPFile("DROP0001.SHP");

		// --- Loadout / PilotLit ---------------------------------------------
		if (pSWTypeExt->DropshipLoadout_LoadoutPCX.isset() && pSWTypeExt->DropshipLoadout_LoadoutPCX.Fetch().Exists())
			dropshipLoadout_LoadoutPCX.push_back(pSWTypeExt->DropshipLoadout_LoadoutPCX.Fetch().GetSurface());

		dropshipLoadout_Loadout = pSWTypeExt->DropshipLoadout_Loadout.isset()
			? pSWTypeExt->DropshipLoadout_Loadout.Fetch()
			: FileSystem::LoadSHPFile("LOADOUT.SHP");

		if (pSWTypeExt->DropshipLoadout_PilotLitPCX.isset() && pSWTypeExt->DropshipLoadout_PilotLitPCX.Fetch().Exists())
			dropshipLoadout_PilotLitPCX.push_back(pSWTypeExt->DropshipLoadout_PilotLitPCX.Fetch().GetSurface());

		dropshipLoadout_PilotLit = pSWTypeExt->DropshipLoadout_PilotLit.isset()
			? pSWTypeExt->DropshipLoadout_PilotLit.Fetch()
			: FileSystem::LoadSHPFile("PILOTLIT.SHP");

		// --- Arrows ---------------------------------------------------------
		if (pSWTypeExt->DropshipLoadout_UpArrowPCX.isset() && pSWTypeExt->DropshipLoadout_UpArrowPCX.Fetch().Exists())
			dropshipLoadout_UpArrowPCX = pSWTypeExt->DropshipLoadout_UpArrowPCX.Fetch().GetSurface();

		dropshipLoadout_UpArrow = pSWTypeExt->DropshipLoadout_UpArrow.isset()
			? pSWTypeExt->DropshipLoadout_UpArrow.Fetch()
			: FileSystem::LoadSHPFile("DROPUP.SHP");

		if (pSWTypeExt->DropshipLoadout_DownArrowPCX.isset() && pSWTypeExt->DropshipLoadout_DownArrowPCX.Fetch().Exists())
			dropshipLoadout_DownArrowPCX = pSWTypeExt->DropshipLoadout_DownArrowPCX.Fetch().GetSurface();

		dropshipLoadout_DownArrow = pSWTypeExt->DropshipLoadout_DownArrow.isset()
			? pSWTypeExt->DropshipLoadout_DownArrow.Fetch()
			: FileSystem::LoadSHPFile("DROPDOWN.SHP");

		// --- DGreen row animations ------------------------------------------
		if (!pSWTypeExt->DropshipLoadout_DGreenListPCX.empty())
		{
			std::vector<BSurface*> rowAnimFrames;

			for (auto const& frame : pSWTypeExt->DropshipLoadout_DGreenListPCX)
			{
				if (frame.Exists())
					rowAnimFrames.push_back(frame.GetSurface());
			}

			if (!rowAnimFrames.empty())
				dropshipLoadout_DGreenListPCX.push_back(std::move(rowAnimFrames));
		}

		if (!pSWTypeExt->DropshipLoadout_DGreenList.empty())
		{
			dropshipLoadout_DGreenList.assign(
				pSWTypeExt->DropshipLoadout_DGreenList.begin(),
				pSWTypeExt->DropshipLoadout_DGreenList.end());
		}
		else
		{
			LoadDefaultDGreenList();
		}

		// --- Sounds ---------------------------------------------------------
		buyClickSoundIdx = DropshipLoadoutHelpers::PickNullable(&pSWTypeExt->DropshipLoadout_BuyClickSound, RulesClass::Instance->GenericClick);
		sellClickSoundIdx = DropshipLoadoutHelpers::PickNullable(&pSWTypeExt->DropshipLoadout_SellClickSound, RulesClass::Instance->SellSound);
		arrowsClickSoundIdx = DropshipLoadoutHelpers::PickNullable(&pSWTypeExt->DropshipLoadout_ArrowsClickSound, RulesClass::Instance->GUITabSound);
		startingDragDropSoundIdx = DropshipLoadoutHelpers::PickNullable(&pSWTypeExt->DropshipLoadout_StartingDragDropSound, -1);
		endingDragDropSoundIdx = DropshipLoadoutHelpers::PickNullable(&pSWTypeExt->DropshipLoadout_EndingDragDropSound, -1);

		// --- Money ----------------------------------------------------------
		bool usesPlayerWallet = false;
		long const configuredMoney = pSWTypeExt->DropshipLoadout_Money.isset()
			? static_cast<long>(pSWTypeExt->DropshipLoadout_Money.Fetch())
			: -1;

		this->initialMoney = ResolveInitialMoney(configuredMoney, usesPlayerWallet);
		this->currentMoney = this->initialMoney;

		// SUSPECT: the SW branch reads DropshipLoadout_RememberPurchasedCargo
		// straight off the SW type and completely ignores this->bRememberPurchasedCargo,
		// the Nullable passed into Initialize(). The country branch below DOES
		// honour it. Either the map-action override is meant to apply to SW
		// screens too (then this is a bug) or the parameter should not be
		// accepted for SW invocations at all.
		bool const rememberPurchasedCargo = pSWTypeExt->DropshipLoadout_RememberPurchasedCargo.Get();

		if (!pHouseExt->DropshipLoadout_SWInitialUnitsSet)
		{
			pHouseExt->DropshipLoadout_SWInitialUnits = pSWTypeExt->DropshipLoadout_InitialUnits;
			pHouseExt->DropshipLoadout_SWInitialUnitsSet = true;
		}

		std::vector<TechnoTypeClass*> initialUnitsRemaining;

		if (!bIgnoreFixedUnits)
		{
			for (auto const pUnit : pHouseExt->DropshipLoadout_SWInitialUnits)
			{
				if (pUnit)
					initialUnitsRemaining.push_back(pUnit);
			}
		}

		// --- Preload cargo --------------------------------------------------
		bool canPreload = false;
		long totalPreloadedCost = 0;

		if (this->bPreloadCargo && !pHouseExt->DropshipLoadout_SWCargo.empty())
		{
			const std::vector<TechnoTypeClass*>* pFixedList = nullptr;

			if (!this->bIgnoreFixedUnits && !pSWTypeExt->DropshipLoadout_FixedUnits.empty())
				pFixedList = &pSWTypeExt->DropshipLoadout_FixedUnits;

			totalPreloadedCost = AccountPreloadedUnits(pHouseExt->DropshipLoadout_SWCargo, pFixedList, initialUnitsRemaining);

			canPreload = usesPlayerWallet || rememberPurchasedCargo || currentMoney >= totalPreloadedCost;
		}

		if (canPreload)
		{
			if (!usesPlayerWallet && !rememberPurchasedCargo)
				currentMoney -= totalPreloadedCost;
		}
		else
		{
			this->bPreloadCargo = false;
		}

		// --- Available units ------------------------------------------------
		std::vector<TechnoTypeClass*> allowableUnits;
		std::vector<int> allowableUnitMaximums;

		if (!pSWTypeExt->DropshipLoadout_AllowableUnits.empty())
		{
			for (auto const pUnit : pSWTypeExt->DropshipLoadout_AllowableUnits)
			{
				if (pUnit)
					allowableUnits.push_back(pUnit);
			}

			allowableUnitMaximums.assign(
				pSWTypeExt->DropshipLoadout_AllowableUnitMaximums.begin(),
				pSWTypeExt->DropshipLoadout_AllowableUnitMaximums.end());
		}

		BuildAvailableUnits(allowableUnits, std::move(allowableUnitMaximums));

		// Initial units must always be buyable back after being removed.
		for (auto const pUnit : pSWTypeExt->DropshipLoadout_InitialUnits)
			EnsureUnitAvailable(pUnit);

		return;
	}

	// ========================= country / scenario branch =====================

	dropshipLoadout_Palette = pHouseTypeExt->DropshipLoadout_Palette;

	if (!dropshipLoadout_Palette && pGlobal)
		dropshipLoadout_Palette = pGlobal->DropshipLoadout_Palette;

	if (!dropshipLoadout_Palette)
		dropshipLoadout_Palette = FileSystem::LoadPALFile("DROPSHIP.PAL", DSurface::Hidden);

	// --- Background ---------------------------------------------------------
	if (pHouseTypeExt->DropshipLoadout_BackgroundPCX.isset() && pHouseTypeExt->DropshipLoadout_BackgroundPCX.Fetch().Exists())
	{
		dropshipLoadout_BackgroundPCX = pHouseTypeExt->DropshipLoadout_BackgroundPCX.Fetch().GetSurface();
	}
	else if (!pHouseTypeExt->DropshipLoadout_BackgroundPCXPattern.empty())
	{
		// Re-format with the runtime bay count; the parse-time value may still
		// have been 0 when the pattern was read.
		PhobosPCXFile const runtimePCX(FormatIndexedName(pHouseTypeExt->DropshipLoadout_BackgroundPCXPattern, nStartingDropships).c_str());

		if (runtimePCX.Exists())
			dropshipLoadout_BackgroundPCX = runtimePCX.GetSurface();
		else if (pGlobal && pGlobal->DropshipLoadout_BackgroundPCX.Exists())
			dropshipLoadout_BackgroundPCX = pGlobal->DropshipLoadout_BackgroundPCX.GetSurface();
	}
	else if (pGlobal && pGlobal->DropshipLoadout_BackgroundPCX.Exists())
	{
		dropshipLoadout_BackgroundPCX = pGlobal->DropshipLoadout_BackgroundPCX.GetSurface();
	}

	if (pGlobal && pGlobal->DropshipLoadout_Background)
	{
		dropshipLoadout_Background = pGlobal->DropshipLoadout_Background;
	}
	else
	{
		// DIFF: was `char tempFilenameBuffer[32]; _snprintf_s(...); LoadSHPFile(_strdup(buf));`
		// The _strdup leaked one allocation per call.
		// SUSPECT: verify LoadSHPFile copies the name rather than retaining the
		// pointer. If it retains, this std::string must outlive the call - the
		// _strdup may have been a deliberate (leaky) workaround for exactly that.
		std::string const backgroundName = fmt::format("DROP{:04d}.SHP", nStartingDropships);
		dropshipLoadout_Background = FileSystem::LoadSHPFile(backgroundName.c_str());
	}

	// --- Loadout / PilotLit -------------------------------------------------
	auto const AppendSurfaces = [](std::vector<BSurface*>& out, auto const& files)
		{
			for (auto const& file : files)
				out.push_back(file.GetSurface());
		};

	if (!pHouseTypeExt->DropshipLoadout_LoadoutPCX.empty())
		AppendSurfaces(dropshipLoadout_LoadoutPCX, pHouseTypeExt->DropshipLoadout_LoadoutPCX);
	else if (pGlobal && !pGlobal->DropshipLoadout_LoadoutPCX.empty())
		AppendSurfaces(dropshipLoadout_LoadoutPCX, pGlobal->DropshipLoadout_LoadoutPCX);

	dropshipLoadout_Loadout = (pGlobal && pGlobal->DropshipLoadout_Loadout)
		? pGlobal->DropshipLoadout_Loadout
		: FileSystem::LoadSHPFile("LOADOUT.SHP");

	if (!pHouseTypeExt->DropshipLoadout_PilotLitPCX.empty())
		AppendSurfaces(dropshipLoadout_PilotLitPCX, pHouseTypeExt->DropshipLoadout_PilotLitPCX);
	else if (pGlobal && !pGlobal->DropshipLoadout_PilotLitPCX.empty())
		AppendSurfaces(dropshipLoadout_PilotLitPCX, pGlobal->DropshipLoadout_PilotLitPCX);

	dropshipLoadout_PilotLit = (pGlobal && pGlobal->DropshipLoadout_PilotLit)
		? pGlobal->DropshipLoadout_PilotLit
		: FileSystem::LoadSHPFile("PILOTLIT.SHP");

	// --- Arrows -------------------------------------------------------------
	dropshipLoadout_UpArrowPCX = DropshipLoadoutHelpers::PickPCXSurface(
		pHouseTypeExt->DropshipLoadout_UpArrowPCX.isset() ? &pHouseTypeExt->DropshipLoadout_UpArrowPCX.Fetch() : nullptr,
		pGlobal ? &pGlobal->DropshipLoadout_UpArrowPCX : nullptr);

	dropshipLoadout_UpArrow = (pGlobal && pGlobal->DropshipLoadout_UpArrow)
		? pGlobal->DropshipLoadout_UpArrow
		: FileSystem::LoadSHPFile("DROPUP.SHP");

	dropshipLoadout_DownArrowPCX = DropshipLoadoutHelpers::PickPCXSurface(
		pHouseTypeExt->DropshipLoadout_DownArrowPCX.isset() ? &pHouseTypeExt->DropshipLoadout_DownArrowPCX.Fetch() : nullptr,
		pGlobal ? &pGlobal->DropshipLoadout_DownArrowPCX : nullptr);

	dropshipLoadout_DownArrow = (pGlobal && pGlobal->DropshipLoadout_DownArrow)
		? pGlobal->DropshipLoadout_DownArrow
		: FileSystem::LoadSHPFile("DROPDOWN.SHP");

	// --- DGreen row animations ----------------------------------------------
	auto const AppendAnimationGroups = [this](std::vector<std::vector<PhobosPCXFile>>& groups)
		{
			for (auto const& pGroup : groups)
			{
				auto& rowAnimFrames = dropshipLoadout_DGreenListPCX.emplace_back();

				for (auto const& frame : pGroup)
					rowAnimFrames.push_back(frame.GetSurface());
			}
		};

	if (!pHouseTypeExt->DropshipLoadout_DGreenListPCX.empty())
	{
		AppendAnimationGroups(pHouseTypeExt->DropshipLoadout_DGreenListPCX);
	}
	else if (pGlobal && !pGlobal->DropshipLoadout_DGreenListPCX.empty())
	{
		AppendAnimationGroups(pGlobal->DropshipLoadout_DGreenListPCX);

		// Pad to the 4 rows the default layout expects.
		// DIFF: the original loop condition was `i < 4 && size() < 4`, where the
		// counter was redundant - the size check alone terminates it.
		while (dropshipLoadout_DGreenListPCX.size() < 4)
			dropshipLoadout_DGreenListPCX.emplace_back();
	}

	for (int i = 0; i < 4; ++i)
	{
		bool const globalHasRow = pGlobal
			&& static_cast<int>(pGlobal->DropshipLoadout_DGreenList.size()) >= 4
			&& pGlobal->DropshipLoadout_DGreenList[i] != nullptr;

		dropshipLoadout_DGreenList.push_back(globalHasRow
			? pGlobal->DropshipLoadout_DGreenList[i]
			: DropshipLoadoutHelpers::LoadDefaultDGreen(i));
	}

	if (pGlobal)
	{
		// Rows beyond the first 4 have no built-in default.
		for (size_t i = 4; i < pGlobal->DropshipLoadout_DGreenList.size(); ++i)
			dropshipLoadout_DGreenList.push_back(pGlobal->DropshipLoadout_DGreenList[i]);
	}

	// --- Sounds -------------------------------------------------------------
	buyClickSoundIdx = DropshipLoadoutHelpers::PickNullable(&pHouseTypeExt->DropshipLoadout_BuyClickSound,
		pGlobal ? &pGlobal->DropshipLoadout_BuyClickSound : nullptr, RulesClass::Instance->GenericClick);

	sellClickSoundIdx = DropshipLoadoutHelpers::PickNullable(&pHouseTypeExt->DropshipLoadout_SellClickSound,
		pGlobal ? &pGlobal->DropshipLoadout_SellClickSound : nullptr, RulesClass::Instance->SellSound);

	arrowsClickSoundIdx = DropshipLoadoutHelpers::PickNullable(&pHouseTypeExt->DropshipLoadout_ArrowsClickSound,
		pGlobal ? &pGlobal->DropshipLoadout_ArrowsClickSound : nullptr, RulesClass::Instance->GUITabSound);

	startingDragDropSoundIdx = DropshipLoadoutHelpers::PickNullable(&pHouseTypeExt->DropshipLoadout_StartingDragDropSound,
		pGlobal ? &pGlobal->DropshipLoadout_StartingDragDropSound : nullptr, -1);

	endingDragDropSoundIdx = DropshipLoadoutHelpers::PickNullable(&pHouseTypeExt->DropshipLoadout_EndingDragDropSound,
		pGlobal ? &pGlobal->DropshipLoadout_EndingDragDropSound : nullptr, -1);

	// --- Money --------------------------------------------------------------
	bool usesPlayerWallet = false;
	long configuredMoney = -1;

	if (pHouseTypeExt->DropshipLoadout_Money.isset())
		configuredMoney = static_cast<long>(pHouseTypeExt->DropshipLoadout_Money.Fetch());
	else if (pGlobal)
		configuredMoney = static_cast<long>(pGlobal->DropshipLoadout_Money);

	this->initialMoney = ResolveInitialMoney(configuredMoney, usesPlayerWallet);
	this->currentMoney = this->initialMoney;

	bool rememberPurchasedCargo = true;

	if (this->bRememberPurchasedCargo.isset())
		rememberPurchasedCargo = this->bRememberPurchasedCargo.Fetch();
	else if (pHouseTypeExt->DropshipLoadout_RememberPurchasedCargo.isset())
		rememberPurchasedCargo = pHouseTypeExt->DropshipLoadout_RememberPurchasedCargo.Fetch();
	else if (pGlobal)
		rememberPurchasedCargo = pGlobal->DropshipLoadout_RememberPurchasedCargo;

	// --- Initial units pool -------------------------------------------------
	if (!pHouseExt->DropshipLoadout_InitialUnitsSet)
	{
		if (!pHouseTypeExt->DropshipLoadout_InitialUnits.empty())
			pHouseExt->DropshipLoadout_InitialUnits = pHouseTypeExt->DropshipLoadout_InitialUnits;
		else if (pGlobal && !pGlobal->DropshipLoadout_InitialUnits.empty())
			pHouseExt->DropshipLoadout_InitialUnits = pGlobal->DropshipLoadout_InitialUnits;

		pHouseExt->DropshipLoadout_InitialUnitsSet = true;
	}

	std::vector<TechnoTypeClass*> initialUnitsRemaining;

	if (!this->bIgnoreFixedUnits)
	{
		for (size_t i = 0; i < pHouseExt->DropshipLoadout_InitialUnits.size() && i < static_cast<size_t>(nStartingDropships); ++i)
		{
			for (auto const pUnit : pHouseExt->DropshipLoadout_InitialUnits[i])
			{
				if (pUnit)
					initialUnitsRemaining.push_back(pUnit);
			}
		}
	}

	// --- Preload cargo ------------------------------------------------------
	const std::vector<std::vector<TechnoTypeClass*>>* pFixedUnitsSrc = nullptr;

	if (!this->bIgnoreFixedUnits)
	{
		if (!pHouseTypeExt->DropshipLoadout_FixedUnits.empty())
			pFixedUnitsSrc = &pHouseTypeExt->DropshipLoadout_FixedUnits;
		else if (pGlobal && !pGlobal->DropshipLoadout_FixedUnits.empty())
			pFixedUnitsSrc = &pGlobal->DropshipLoadout_FixedUnits;
	}

	bool canPreload = false;
	long totalPreloadedCost = 0;

	if (this->bPreloadCargo && !pHouseExt->DropshipLoadout_Cargo.empty())
	{
		for (size_t i = 0; i < pHouseExt->DropshipLoadout_Cargo.size() && i < static_cast<size_t>(nStartingDropships); ++i)
		{
			const std::vector<TechnoTypeClass*>* pFixedList = nullptr;

			if (pFixedUnitsSrc && i < pFixedUnitsSrc->size())
				pFixedList = &(*pFixedUnitsSrc)[i];

			totalPreloadedCost += AccountPreloadedUnits(pHouseExt->DropshipLoadout_Cargo[i], pFixedList, initialUnitsRemaining);
		}

		canPreload = usesPlayerWallet || rememberPurchasedCargo || currentMoney >= totalPreloadedCost;
	}

	if (canPreload)
	{
		if (!usesPlayerWallet && !rememberPurchasedCargo)
			currentMoney -= totalPreloadedCost;
	}
	else
	{
		this->bPreloadCargo = false;
	}

	// --- Available units ----------------------------------------------------
	std::vector<TechnoTypeClass*> allowableUnits;
	std::vector<int> allowableUnitMaximums;
	bool listFound = false;

	// Local because only LoadAssets picks a list index.
	auto const TryTakeList = [&](auto* pSource, int index) -> bool
		{
			if (!pSource)
				return false;

			if (index < 0 || index >= static_cast<int>(pSource->DropshipLoadout_AllowableUnitsLists.size()))
				return false;

			if (pSource->DropshipLoadout_AllowableUnitsLists[index].empty())
				return false;

			allowableUnits = pSource->DropshipLoadout_AllowableUnitsLists[index];

			// BUGFIX: the original indexed AllowableUnitMaximumsLists with the
			// same index without checking that vector's own size, so a mod that
			// defined more unit lists than maximum lists read out of bounds.
			if (index < static_cast<int>(pSource->DropshipLoadout_AllowableUnitMaximumsLists.size()))
				allowableUnitMaximums = pSource->DropshipLoadout_AllowableUnitMaximumsLists[index];
			else
				allowableUnitMaximums.clear();

			return true;
		};

	if (this->allowableUnitsIndex > 0)
	{
		listFound = TryTakeList(pHouseTypeExt, this->allowableUnitsIndex)
			|| TryTakeList(pGlobal, this->allowableUnitsIndex);
	}
	else
	{
		listFound = TryTakeList(pHouseTypeExt, 0) || TryTakeList(pGlobal, 0);

		if (!listFound && pHouseTypeExt && !pHouseTypeExt->DropshipLoadout_AllowableUnits.empty())
		{
			allowableUnits = pHouseTypeExt->DropshipLoadout_AllowableUnits;
			allowableUnitMaximums = pHouseTypeExt->DropshipLoadout_AllowableUnitMaximums;
			listFound = true;
		}
	}

	if (!listFound && this->allowableUnitsIndex != 0)
	{
		Debug::Log("[DropshipLoadout] Warning: Requested allowable units list index %d not found, falling back to default rules/all units.\n",
			this->allowableUnitsIndex);
	}

	if (!allowableUnits.empty())
	{
		BuildAvailableUnits(allowableUnits, std::move(allowableUnitMaximums));
	}
	else
	{
		// OPTIMIZE: this walks the entire TechnoTypeClass array every time the
		// screen opens and offers literally every infantry/vehicle in the mod,
		// including ones with no cameo and non-buildable dummies. A
		// `Prerequisite`/`AllowedToStartInMultiplayer` filter would be cheaper
		// and produce a saner default list.
		for (auto const pType : *TechnoTypeClass::Array)
		{
			if (!pType)
				continue;

			if (pType->WhatAmI() != AbstractType::InfantryType && pType->WhatAmI() != AbstractType::UnitType)
				continue;

			availableUnits.push_back(pType);
			availableUnitsMaximums.push_back(-1);
		}
	}

	// Initial units must always be buyable back after being removed.
	const std::vector<std::vector<TechnoTypeClass*>>* pInitialUnitsSrc = nullptr;

	if (!this->bIgnoreFixedUnits)
	{
		if (!pHouseTypeExt->DropshipLoadout_InitialUnits.empty())
			pInitialUnitsSrc = &pHouseTypeExt->DropshipLoadout_InitialUnits;
		else if (pGlobal && !pGlobal->DropshipLoadout_InitialUnits.empty())
			pInitialUnitsSrc = &pGlobal->DropshipLoadout_InitialUnits;
	}

	if (pInitialUnitsSrc)
	{
		for (size_t i = 0; i < pInitialUnitsSrc->size() && i < static_cast<size_t>(nStartingDropships); ++i)
		{
			for (auto const pUnit : (*pInitialUnitsSrc)[i])
				EnsureUnitAvailable(pUnit);
		}
	}
}

// ============================================================================
// CalculateLayout
//
// The original had two ~220-line branches that differed only in WHERE each
// value came from. Resolving the sources first collapses both into one body.
// ============================================================================

DropshipLoadoutClass::LayoutConfig DropshipLoadoutClass::ResolveLayoutConfig() const
{
	LayoutConfig cfg;
	auto const pGlobal = ScenarioExtData::Instance();

	if (pSWTypeExt)
	{
		cfg.UpArrow = DropshipLoadoutHelpers::PickNullable(&pSWTypeExt->DropshipLoadout_UpArrowLocation, Point2D::Empty);
		cfg.DownArrow = DropshipLoadoutHelpers::PickNullable(&pSWTypeExt->DropshipLoadout_DownArrowLocation, Point2D::Empty);
		cfg.Loadout = DropshipLoadoutHelpers::PickNullable(&pSWTypeExt->DropshipLoadout_LoadoutLocation, Point2D { 45, 2 });
		cfg.PilotLit = DropshipLoadoutHelpers::PickNullable(&pSWTypeExt->DropshipLoadout_PilotLitLocation, Point2D { 284, 151 });

		if (pSWTypeExt->DropshipLoadout_SidebarCameosCount.isset() && pSWTypeExt->DropshipLoadout_SidebarCameosCount.Fetch() > 0)
		{
			cfg.SidebarCameosCount = pSWTypeExt->DropshipLoadout_SidebarCameosCount.Fetch();
			cfg.pSidebarCameoLocations = &pSWTypeExt->DropshipLoadout_SidebarCameosLocations;
		}

		if (pSWTypeExt->DropshipLoadout_DGreenAnimationsCount.isset())
		{
			cfg.DGreenAnimationsCount = pSWTypeExt->DropshipLoadout_DGreenAnimationsCount.Fetch();
			cfg.pDGreenLocations = &pSWTypeExt->DropshipLoadout_DGreenLocations;
		}

		// A SW screen is always exactly one bay, so the flat location list is
		// wrapped into a single row.
		int const bayCameos = pSWTypeExt->DropshipLoadout_DropshipCameosCount.Get(0);

		if (bayCameos > 0)
		{
			auto& row = cfg.BayCameoLocations.emplace_back();
			auto const& src = pSWTypeExt->DropshipLoadout_DropshipCameosLocations;

			for (int j = 0; j < bayCameos; ++j)
				row.push_back(j < static_cast<int>(src.size()) ? src[j] : Point2D::Empty);
		}

		return cfg;
	}

	cfg.UpArrow = DropshipLoadoutHelpers::PickPoint(&pHouseTypeExt->DropshipLoadout_UpArrowLocation,
		pGlobal ? &pGlobal->DropshipLoadout_UpArrowLocation : nullptr, Point2D::Empty);

	cfg.DownArrow = DropshipLoadoutHelpers::PickPoint(&pHouseTypeExt->DropshipLoadout_DownArrowLocation,
		pGlobal ? &pGlobal->DropshipLoadout_DownArrowLocation : nullptr, Point2D::Empty);

	cfg.Loadout = DropshipLoadoutHelpers::PickPoint(&pHouseTypeExt->DropshipLoadout_LoadoutLocation,
		pGlobal ? &pGlobal->DropshipLoadout_LoadoutLocation : nullptr, Point2D { 45, 2 });

	cfg.PilotLit = DropshipLoadoutHelpers::PickPoint(&pHouseTypeExt->DropshipLoadout_PilotLitLocation,
		pGlobal ? &pGlobal->DropshipLoadout_PilotLitLocation : nullptr, Point2D { 284, 151 });

	// SUSPECT: the house-type member is DropshipLoadout_SidebarCameoLocations
	// (singular "Cameo") while the SW one is ...SidebarCameosLocations (plural).
	// Almost certainly a typo that shipped; renaming needs an INI-tag audit.
	if (pHouseTypeExt->DropshipLoadout_SidebarCameosCount.isset() && pHouseTypeExt->DropshipLoadout_SidebarCameosCount.Fetch() > 0)
	{
		cfg.SidebarCameosCount = pHouseTypeExt->DropshipLoadout_SidebarCameosCount.Fetch();
		cfg.pSidebarCameoLocations = &pHouseTypeExt->DropshipLoadout_SidebarCameoLocations;
	}
	else if (pGlobal && pGlobal->DropshipLoadout_SidebarCameosCount > 0)
	{
		cfg.SidebarCameosCount = pGlobal->DropshipLoadout_SidebarCameosCount;
		cfg.pSidebarCameoLocations = &pGlobal->DropshipLoadout_SidebarCameoLocations;
	}

	if (pHouseTypeExt->DropshipLoadout_DGreenAnimationsCount.isset())
	{
		cfg.DGreenAnimationsCount = pHouseTypeExt->DropshipLoadout_DGreenAnimationsCount.Fetch();
		cfg.pDGreenLocations = &pHouseTypeExt->DropshipLoadout_DGreenLocations;
	}
	else if (pGlobal && pGlobal->DropshipLoadout_DGreenAnimationsCount)
	{
		cfg.DGreenAnimationsCount = pGlobal->DropshipLoadout_DGreenAnimationsCount;
		cfg.pDGreenLocations = &pGlobal->DropshipLoadout_DGreenLocations;
	}

	// Per-carrier bay cameo rows.
	const std::vector<std::vector<Point2D>>* pRows = nullptr;
	int fallbackCount = 5;

	if (!pHouseTypeExt->DropshipLoadout_DropshipCameoLocations.empty())
	{
		pRows = &pHouseTypeExt->DropshipLoadout_DropshipCameoLocations;

		if (pHouseTypeExt->DropshipLoadout_DropshipCameosCount.Get(0) > 0)
			fallbackCount = pHouseTypeExt->DropshipLoadout_DropshipCameosCount.Get(0);
	}
	else if (pGlobal && !pGlobal->DropshipLoadout_DropshipCameoLocations.empty())
	{
		pRows = &pGlobal->DropshipLoadout_DropshipCameoLocations;

		if (pGlobal->DropshipLoadout_DropshipCameosCount > 0)
			fallbackCount = pGlobal->DropshipLoadout_DropshipCameosCount;
	}

	if (pRows)
	{
		for (int i = 0; i < nStartingDropships; ++i)
		{
			auto& row = cfg.BayCameoLocations.emplace_back();
			bool const hasRow = i < static_cast<int>(pRows->size());
			int const count = hasRow ? static_cast<int>((*pRows)[i].size()) : fallbackCount;

			// SUSPECT: when a mod configures fewer location rows than there are
			// carriers, every cameo of the missing carriers resolves to
			// Point2D::Empty and they all stack on top of each other at the
			// background origin. Silently unusable UI - worth a Debug::Log or a
			// clamp of nStartingDropships to pRows->size().
			for (int j = 0; j < count; ++j)
				row.push_back(hasRow && j < static_cast<int>((*pRows)[i].size()) ? (*pRows)[i][j] : Point2D::Empty);
		}
	}

	return cfg;
}

void DropshipLoadoutClass::BuildDefaultBayGrids(int backgroundX, int backgroundY, int cameoWidth, int cameoHeight)
{
	int const x = backgroundX + 55;

	// DIFF: the original spelled these out as three separate `if` blocks with
	// five emplace_back calls each (25 lines of copy-paste). Same coordinates.
	if (nStartingDropships == 1 || nStartingDropships == 2)
		DropshipLoadoutHelpers::AppendDefaultCameoGrid(dropshipBayCameLocations.emplace_back(), x, backgroundY + 69, cameoWidth, cameoHeight);

	if (nStartingDropships == 2)
		DropshipLoadoutHelpers::AppendDefaultCameoGrid(dropshipBayCameLocations.emplace_back(), x, backgroundY + 209, cameoWidth, cameoHeight);

	if (nStartingDropships == 3)
	{
		for (int i = 0; i < 3; ++i)
			DropshipLoadoutHelpers::AppendDefaultCameoGrid(dropshipBayCameLocations.emplace_back(), x, backgroundY + 39 + i * 120, cameoWidth, cameoHeight);
	}

	// Generic fill for 4+ bays (or any count the cases above did not cover).
	for (int i = static_cast<int>(dropshipBayCameLocations.size()); i < nStartingDropships; ++i)
		DropshipLoadoutHelpers::AppendDefaultCameoGrid(dropshipBayCameLocations.emplace_back(), x, backgroundY + 39 + i * 120, cameoWidth, cameoHeight);
}

void DropshipLoadoutClass::CalculateLayout(DSurface* pSurface)
{
	if (!pSurface)
		return;

	const int cameoWidth = 60;
	const int cameoHeight = 48;

	int const backgroundWidth = DropshipLoadoutHelpers::ImageWidth(dropshipLoadout_BackgroundPCX, dropshipLoadout_Background, 640);
	int const backgroundHeight = DropshipLoadoutHelpers::ImageHeight(dropshipLoadout_BackgroundPCX, dropshipLoadout_Background, 480);

	int const backgroundX = (pSurface->Get_Width() - backgroundWidth) / 2;
	int const backgroundY = (pSurface->Get_Height() - backgroundHeight) / 2;
	windowRectangle = { backgroundX, backgroundY, backgroundWidth, backgroundHeight };

	LayoutConfig const cfg = ResolveLayoutConfig();

	// --- Sidebar cameo column ----------------------------------------------
	nSidebarCameos = cfg.SidebarCameosCount > 0 ? cfg.SidebarCameosCount : 8;
	sidebarCameLocations.clear();

	if (cfg.SidebarCameosCount > 0 && cfg.pSidebarCameoLocations)
	{
		for (int i = 0; i < nSidebarCameos; ++i)
		{
			// BUGFIX: the original indexed the location array with i up to
			// SidebarCameosCount without ever checking the array's own size, so
			// `Count=12` with only 8 locations read past the end.
			Point2D const offset = i < static_cast<int>(cfg.pSidebarCameoLocations->size())
				? (*cfg.pSidebarCameoLocations)[i]
				: Point2D::Empty;

			sidebarCameLocations.emplace_back(backgroundX + offset.X, backgroundY + offset.Y, cameoWidth, cameoHeight);
		}
	}
	else
	{
		// Default: two columns, four rows.
		for (int i = 0; i < nSidebarCameos; ++i)
		{
			sidebarCameLocations.emplace_back(
				backgroundX + 493 + 68 * (i % 2),
				backgroundY + 25 + 50 * (i / 2),
				cameoWidth, cameoHeight);
		}
	}

	// --- Scroll arrows ------------------------------------------------------
	int centerOfCameoColumns = backgroundX + 500;
	int arrowsY = backgroundY + 400;

	if (sidebarCameLocations.size() >= 2)
	{
		int const firstRight = sidebarCameLocations[0].X + sidebarCameLocations[0].Width;
		centerOfCameoColumns = firstRight + (sidebarCameLocations[1].X - firstRight) / 2;
		arrowsY = sidebarCameLocations.back().Y + sidebarCameLocations.back().Height + 6;
	}

	int const upArrowWidth = DropshipLoadoutHelpers::ImageWidth(dropshipLoadout_UpArrowPCX, dropshipLoadout_UpArrow, 30);
	int const upArrowHeight = DropshipLoadoutHelpers::ImageHeight(dropshipLoadout_UpArrowPCX, dropshipLoadout_UpArrow, 30);

	upArrowX = cfg.UpArrow != Point2D::Empty ? (backgroundX + cfg.UpArrow.X) : (centerOfCameoColumns - upArrowWidth);
	upArrowY = cfg.UpArrow != Point2D::Empty ? (backgroundY + cfg.UpArrow.Y) : arrowsY;
	upArrowLocation = { upArrowX, upArrowY, upArrowWidth, upArrowHeight };

	int const downArrowWidth = DropshipLoadoutHelpers::ImageWidth(dropshipLoadout_DownArrowPCX, dropshipLoadout_DownArrow, 30);
	int const downArrowHeight = DropshipLoadoutHelpers::ImageHeight(dropshipLoadout_DownArrowPCX, dropshipLoadout_DownArrow, 30);

	downArrowX = cfg.DownArrow != Point2D::Empty ? (backgroundX + cfg.DownArrow.X) : centerOfCameoColumns;
	downArrowY = cfg.DownArrow != Point2D::Empty ? (backgroundY + cfg.DownArrow.Y) : arrowsY;
	downArrowLocation = { downArrowX, downArrowY, downArrowWidth, downArrowHeight };

	// --- DGreen row animation anchors ---------------------------------------
	dGreenLocation.clear();

	if (cfg.DGreenAnimationsCount > 0 && cfg.pDGreenLocations)
	{
		for (int i = 0; i < cfg.DGreenAnimationsCount; ++i)
		{
			// BUGFIX: same missing bounds check as the sidebar list above.
			Point2D const offset = i < static_cast<int>(cfg.pDGreenLocations->size())
				? (*cfg.pDGreenLocations)[i]
				: Point2D::Empty;

			dGreenLocation.push_back({ backgroundX + offset.X, backgroundY + offset.Y, 0, 0 });
		}
	}
	else
	{
		int dGreenY = 10;
		size_t const extraRows = !dropshipLoadout_DGreenListPCX.empty()
			? dropshipLoadout_DGreenListPCX.size()
			: dropshipLoadout_DGreenList.size();

		// Four default rows, then one extra anchor per configured row past 4.
		size_t const totalRows = std::max<size_t>(4, extraRows);

		for (size_t i = 0; i < totalRows; ++i)
		{
			dGreenLocation.push_back({ backgroundX + 371, backgroundY + dGreenY, 0, 0 });
			dGreenY += 50;
		}
	}

	// Size the anchors from whichever art source is actually in use.
	if (!dropshipLoadout_DGreenListPCX.empty())
	{
		for (size_t i = 0; i < dropshipLoadout_DGreenListPCX.size() && i < dGreenLocation.size(); ++i)
		{
			if (dropshipLoadout_DGreenListPCX[i].empty() || !dropshipLoadout_DGreenListPCX[i][0])
				continue;

			dGreenLocation[i].Width = dropshipLoadout_DGreenListPCX[i][0]->Width;
			dGreenLocation[i].Height = dropshipLoadout_DGreenListPCX[i][0]->Height;
		}
	}
	else
	{
		for (size_t i = 0; i < dropshipLoadout_DGreenList.size() && i < dGreenLocation.size(); ++i)
		{
			if (!dropshipLoadout_DGreenList[i])
				continue;

			dGreenLocation[i].Width = dropshipLoadout_DGreenList[i]->Width;
			dGreenLocation[i].Height = dropshipLoadout_DGreenList[i]->Height;
		}
	}

	// --- Loadout / PilotLit panels ------------------------------------------
	loadoutLocation = {
		backgroundX + cfg.Loadout.X,
		backgroundY + cfg.Loadout.Y,
		DropshipLoadoutHelpers::ImageWidth(dropshipLoadout_LoadoutPCX, dropshipLoadout_Loadout, 100),
		DropshipLoadoutHelpers::ImageHeight(dropshipLoadout_LoadoutPCX, dropshipLoadout_Loadout, 100)
	};

	pilotLitLocation = {
		backgroundX + cfg.PilotLit.X,
		backgroundY + cfg.PilotLit.Y,
		DropshipLoadoutHelpers::ImageWidth(dropshipLoadout_PilotLitPCX, dropshipLoadout_PilotLit, 100),
		DropshipLoadoutHelpers::ImageHeight(dropshipLoadout_PilotLitPCX, dropshipLoadout_PilotLit, 100)
	};

	// --- Bay cameo slots ----------------------------------------------------
	dropshipBayCameLocations.clear();

	if (!cfg.BayCameoLocations.empty())
	{
		for (auto const& row : cfg.BayCameoLocations)
		{
			auto& list = dropshipBayCameLocations.emplace_back();

			for (auto const& offset : row)
				list.emplace_back(backgroundX + offset.X, backgroundY + offset.Y, cameoWidth, cameoHeight);
		}
	}
	else if (pSWTypeExt)
	{
		DropshipLoadoutHelpers::AppendDefaultCameoGrid(dropshipBayCameLocations.emplace_back(), backgroundX + 55, backgroundY + 69, cameoWidth, cameoHeight);
	}
	else
	{
		BuildDefaultBayGrids(backgroundX, backgroundY, cameoWidth, cameoHeight);
	}

	// nDropshipBayCameos is the widest bay; button IDs are decoded against it.
	nDropshipBayCameos = 0;

	for (auto const& list : dropshipBayCameLocations)
		nDropshipBayCameos = std::max(nDropshipBayCameos, static_cast<int>(list.size()));

	nDropshipBayTotalSlots = nStartingDropships * nDropshipBayCameos;
}

// ============================================================================
// CreateControls
// ============================================================================

TechnoTypeClass* DropshipLoadoutClass::ResolveBaySlot(
	const BaySlotSource& source,
	int slotIdx,
	std::vector<TechnoTypeClass*>& fixedRemaining,
	std::vector<TechnoTypeClass*>& initialRemaining,
	bool& isFixedOut)
{
	isFixedOut = false;

	// Restoring a previously saved loadout.
	if (source.pSavedCargo)
	{
		if (slotIdx >= static_cast<int>(source.pSavedCargo->size()))
			return nullptr;

		auto const pUnit = (*source.pSavedCargo)[slotIdx];

		if (!pUnit)
			return nullptr;

		auto const itFixed = std::find(fixedRemaining.begin(), fixedRemaining.end(), pUnit);

		if (itFixed != fixedRemaining.end())
		{
			fixedRemaining.erase(itFixed);
			isFixedOut = true;
			return pUnit;
		}

		auto const itInitial = std::find(initialRemaining.begin(), initialRemaining.end(), pUnit);

		if (itInitial != initialRemaining.end())
			initialRemaining.erase(itInitial);

		++dropshipBayChosenUnitsCount[pUnit];
		return pUnit;
	}

	// Fresh screen: fixed units first, then the free initial units after them.
	int const nFixed = source.pFixedUnits ? static_cast<int>(source.pFixedUnits->size()) : 0;

	if (source.pFixedUnits && slotIdx < nFixed)
	{
		auto const pUnit = (*source.pFixedUnits)[slotIdx];
		isFixedOut = pUnit != nullptr;
		return pUnit;
	}

	int const initialIdx = slotIdx - nFixed;
	int const nInitial = source.pInitialUnits ? static_cast<int>(source.pInitialUnits->size()) : 0;

	if (source.pInitialUnits && initialIdx >= 0 && initialIdx < nInitial)
	{
		auto const pUnit = (*source.pInitialUnits)[initialIdx];

		if (pUnit)
			++dropshipBayChosenUnitsCount[pUnit];

		return pUnit;
	}

	return nullptr;
}

void DropshipLoadoutClass::CreateControls()
{
	static constexpr int cameoWidth = 60;
	static constexpr int cameoHeight = 48;

	static constexpr int btn_ScrollUp_ID = 100;
	static constexpr int btn_ScrollDown_ID = 101;
	static constexpr int btn_BasicDropshipCameo_ID = 200;
	static constexpr int btn_BasicSidebarCameo_ID = 300;

	buttonsList.clear();

	AddButton(btn_ScrollUp_ID, upArrowLocation);
	AddButton(btn_ScrollDown_ID, downArrowLocation);

	dropshipBayChosenUnitsLists.clear();
	dropshipBayFixedUnitsLists.clear();
	dropshipBayChosenUnitsCount.clear();

	auto const pHouseExt = HouseExtContainer::Instance.Find(HouseClass::CurrentPlayer);

	// --- Build one BaySlotSource per carrier --------------------------------
	// This is the only part that differed between the SW and country branches;
	// once the sources are normalised the fill loop below is shared.
	std::vector<BaySlotSource> sources;
	std::vector<TechnoTypeClass*> initialUnitsRemaining;

	if (pSWTypeExt)
	{
		if (!pHouseExt->DropshipLoadout_SWInitialUnitsSet)
		{
			pHouseExt->DropshipLoadout_SWInitialUnits = pSWTypeExt->DropshipLoadout_InitialUnits;
			pHouseExt->DropshipLoadout_SWInitialUnitsSet = true;
		}

		BaySlotSource source;

		if (!bIgnoreFixedUnits && !pSWTypeExt->DropshipLoadout_FixedUnits.empty())
			source.pFixedUnits = &pSWTypeExt->DropshipLoadout_FixedUnits;

		if (!bIgnoreFixedUnits && !pHouseExt->DropshipLoadout_SWInitialUnits.empty())
			source.pInitialUnits = &pHouseExt->DropshipLoadout_SWInitialUnits;

		if (bPreloadCargo && !pHouseExt->DropshipLoadout_SWCargo.empty())
			source.pSavedCargo = &pHouseExt->DropshipLoadout_SWCargo;

		if (source.pInitialUnits)
		{
			for (auto const pUnit : *source.pInitialUnits)
			{
				if (pUnit)
					initialUnitsRemaining.push_back(pUnit);
			}
		}

		sources.push_back(source);
	}
	else
	{
		const std::vector<std::vector<TechnoTypeClass*>>* pFixedUnitsSrc = nullptr;

		if (!bIgnoreFixedUnits)
		{
			auto const pGlobal = ScenarioExtData::Instance();

			if (!pHouseTypeExt->DropshipLoadout_FixedUnits.empty())
				pFixedUnitsSrc = &pHouseTypeExt->DropshipLoadout_FixedUnits;
			else if (pGlobal && !pGlobal->DropshipLoadout_FixedUnits.empty())
				pFixedUnitsSrc = &pGlobal->DropshipLoadout_FixedUnits;
		}

		if (!pHouseExt->DropshipLoadout_InitialUnitsSet && !bIgnoreFixedUnits)
		{
			auto const pGlobal = ScenarioExtData::Instance();

			if (!pHouseTypeExt->DropshipLoadout_InitialUnits.empty())
				pHouseExt->DropshipLoadout_InitialUnits = pHouseTypeExt->DropshipLoadout_InitialUnits;
			else if (pGlobal && !pGlobal->DropshipLoadout_InitialUnits.empty())
				pHouseExt->DropshipLoadout_InitialUnits = pGlobal->DropshipLoadout_InitialUnits;

			pHouseExt->DropshipLoadout_InitialUnitsSet = true;
		}

		const std::vector<std::vector<TechnoTypeClass*>>* pInitialUnitsSrc = nullptr;

		if (!bIgnoreFixedUnits && !pHouseExt->DropshipLoadout_InitialUnits.empty())
			pInitialUnitsSrc = &pHouseExt->DropshipLoadout_InitialUnits;

		if (pInitialUnitsSrc)
		{
			for (size_t i = 0; i < pInitialUnitsSrc->size() && i < static_cast<size_t>(nStartingDropships); ++i)
			{
				for (auto const pUnit : (*pInitialUnitsSrc)[i])
				{
					if (pUnit)
						initialUnitsRemaining.push_back(pUnit);
				}
			}
		}

		for (int i = 0; i < nStartingDropships; ++i)
		{
			BaySlotSource source;

			if (pFixedUnitsSrc && i < static_cast<int>(pFixedUnitsSrc->size()))
				source.pFixedUnits = &(*pFixedUnitsSrc)[i];

			if (pInitialUnitsSrc && i < static_cast<int>(pInitialUnitsSrc->size()))
				source.pInitialUnits = &(*pInitialUnitsSrc)[i];

			if (bPreloadCargo && i < static_cast<int>(pHouseExt->DropshipLoadout_Cargo.size()))
				source.pSavedCargo = &pHouseExt->DropshipLoadout_Cargo[i];

			sources.push_back(source);
		}
	}

	// --- Shared fill loop ---------------------------------------------------
	for (size_t i = 0; i < sources.size(); ++i)
	{
		auto& units = dropshipBayChosenUnitsLists.emplace_back();
		auto& fixed = dropshipBayFixedUnitsLists.emplace_back();

		if (i >= dropshipBayCameLocations.size())
			continue;

		std::vector<TechnoTypeClass*> fixedRemaining;

		if (sources[i].pFixedUnits)
		{
			for (auto const pUnit : *sources[i].pFixedUnits)
			{
				if (pUnit)
					fixedRemaining.push_back(pUnit);
			}
		}

		for (int j = 0; j < nDropshipBayCameos; ++j)
		{
			if (j >= static_cast<int>(dropshipBayCameLocations[i].size()))
				continue;

			// BUGFIX: the SW branch used a running counter for the button ID and
			// skipped incrementing it on `continue`, so a bay with a missing
			// location produced IDs that no longer matched the
			// (id - 200) / nDropshipBayCameos decode used in HandleInput() and
			// the drop handler - clicks landed on the wrong slot. Both branches
			// now use the positional formula.
			int const buttonID = btn_BasicDropshipCameo_ID + static_cast<int>(i) * nDropshipBayCameos + j;
			AddButton(buttonID, dropshipBayCameLocations[i][j]);

			bool isFixed = false;
			auto const pUnit = ResolveBaySlot(sources[i], j, fixedRemaining, initialUnitsRemaining, isFixed);

			units.push_back(pUnit);
			fixed.push_back(isFixed);
		}
	}

	// --- Sidebar cameo buttons ----------------------------------------------
	for (int i = 0; i < nSidebarCameos && i < static_cast<int>(sidebarCameLocations.size()); ++i)
		AddButton(btn_BasicSidebarCameo_ID + i, sidebarCameLocations[i]);
}

// ============================================================================
// Drag & drop state machine
//
// NOTE: this is a decomposition of Run(), not a de-duplication - it has a
// single call site. It was split out because the original Run() was ~330 lines
// with the drop resolution nested 9 levels deep, which made the repeated
// "occupy slot / refund / replace" blocks impossible to spot.
// ============================================================================

void DropshipLoadoutClass::UpdateDragState(int buttonID)
{
	if (!bDragPending && !bIsDragging)
		return;

	Point2D const mousePos = DropshipLoadoutHelpers::CursorPosition();

	// --- pending -> active transition ---------------------------------------
	if (bDragPending)
	{
		int const dist = std::abs(mousePos.X - dragStartMousePos.X) + std::abs(mousePos.Y - dragStartMousePos.Y);

		// SUSPECT: Manhattan distance, not Euclidean. Diagonal drags therefore
		// trigger after ~10px per axis while axis-aligned drags need 15px.
		// Harmless, but note it if the threshold ever gets tuned.
		if (dist >= 15)
		{
			bIsDragging = true;
			bDragPending = false;

			if (nSourceDropshipIdx != -1)
			{
				// Lift the unit out of its slot and refund it up front; the drop
				// handler re-charges or returns it.
				bDraggedIsFixed = dropshipBayFixedUnitsLists[nSourceDropshipIdx][nSourceSlotIdx];
				dropshipBayChosenUnitsLists[nSourceDropshipIdx][nSourceSlotIdx] = nullptr;
				dropshipBayFixedUnitsLists[nSourceDropshipIdx][nSourceSlotIdx] = false;

				if (!bDraggedIsFixed)
				{
					currentMoney += pDraggedUnitType->Cost;

					auto const it = dropshipBayChosenUnitsCount.find(pDraggedUnitType);

					if (it != dropshipBayChosenUnitsCount.end() && it->second > 0)
						--it->second;
				}
			}
			else
			{
				bDraggedIsFixed = false;
			}

			DropshipLoadoutHelpers::PlaySoundIfValid(startingDragDropSoundIdx);
			repaintAll = true;
		}
	}

	// Still held down - nothing to resolve yet.
	if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
		return;

	// --- quick click (released before crossing the drag threshold) -----------
	if (bDragPending)
	{
		bDragPending = false;

		if (nSourceDropshipIdx == -1)
			BuyIntoFirstFreeSlot(pDraggedUnitType);
		else
			lastSelected = pDraggedUnitType; // left-click never sells; that is right-click

		pDraggedUnitType = nullptr;
		repaintAll = true;
		return;
	}

	if (!bIsDragging)
		return;

	// --- drop resolution -----------------------------------------------------
	const int btn_BasicDropshipCameo_ID = 200;
	const int btn_BasicSidebarCameo_ID = 300;

	bool const droppedOnSlot = buttonID >= btn_BasicDropshipCameo_ID
		&& buttonID < (btn_BasicDropshipCameo_ID + nDropshipBayTotalSlots);

	bool const droppedOnSidebar = buttonID >= btn_BasicSidebarCameo_ID
		&& buttonID < (btn_BasicSidebarCameo_ID + nSidebarCameos);

	bool const droppedOnSidebarArea = nSourceDropshipIdx != -1
		&& DropshipLoadoutHelpers::RectContains(GetSidebarArea(), mousePos.X, mousePos.Y);

	// Put the dragged unit back where it came from, re-charging it.
	auto const ReturnToSource = [&](bool playSound = true)
		{
			if (nSourceDropshipIdx == -1)
				return;

			dropshipBayChosenUnitsLists[nSourceDropshipIdx][nSourceSlotIdx] = pDraggedUnitType;
			dropshipBayFixedUnitsLists[nSourceDropshipIdx][nSourceSlotIdx] = bDraggedIsFixed;

			if (!bDraggedIsFixed)
			{
				currentMoney -= pDraggedUnitType->Cost;
				++dropshipBayChosenUnitsCount[pDraggedUnitType];
			}

			if (playSound)
				VocClass::PlayGlobal(buyClickSoundIdx, Panning::Center, 1.0);
		};

	// Occupies a slot with the dragged unit. Appeared 5x inline in the original.
	auto const OccupySlot = [&](int carrier, int slot, bool isFixed, bool charge)
		{
			dropshipBayChosenUnitsLists[carrier][slot] = pDraggedUnitType;
			dropshipBayFixedUnitsLists[carrier][slot] = isFixed;

			if (charge)
			{
				currentMoney -= pDraggedUnitType->Cost;
				++dropshipBayChosenUnitsCount[pDraggedUnitType];
			}

			lastSelected = pDraggedUnitType;
			DropshipLoadoutHelpers::PlaySoundIfValid(endingDragDropSoundIdx);
		};

	// Refund the occupant and drop the dragged unit in its place. The original
	// had this block twice, character for character.
	auto const TryReplace = [&](int carrier, int slot, TechnoTypeClass* pTarget) -> bool
		{
			bool const limitOk = (pDraggedUnitType == pTarget)
				|| (GetInstanceCount(pDraggedUnitType) < GetMaxInstances(pDraggedUnitType));

			long const netCost = pDraggedUnitType->Cost - pTarget->Cost;

			if (!limitOk || netCost > currentMoney)
				return false;

			currentMoney += pTarget->Cost;

			auto const it = dropshipBayChosenUnitsCount.find(pTarget);

			if (it != dropshipBayChosenUnitsCount.end() && it->second > 0)
				--it->second;

			OccupySlot(carrier, slot, false, true);
			return true;
		};

	if (droppedOnSlot)
	{
		int const carrier = (buttonID - btn_BasicDropshipCameo_ID) / nDropshipBayCameos;
		int const slot = (buttonID - btn_BasicDropshipCameo_ID) - (carrier * nDropshipBayCameos);

		bool const validSlot = carrier < static_cast<int>(dropshipBayChosenUnitsLists.size())
			&& slot < static_cast<int>(dropshipBayChosenUnitsLists[carrier].size());

		if (!validSlot)
		{
			ReturnToSource();
		}
		else if (bDraggedIsFixed && carrier != nSourceDropshipIdx)
		{
			ReturnToSource(); // fixed units may not leave their own bay
		}
		else if (!CanCarrierHoldUnit(carrier, pDraggedUnitType))
		{
			ReturnToSource(); // too heavy for this carrier
		}
		else
		{
			auto const pTarget = dropshipBayChosenUnitsLists[carrier][slot];
			bool const bTargetIsFixed = dropshipBayFixedUnitsLists[carrier][slot];
			bool const underLimit = GetInstanceCount(pDraggedUnitType) < GetMaxInstances(pDraggedUnitType);

			if (!pTarget)
			{
				if (bDraggedIsFixed)
					OccupySlot(carrier, slot, true, false);
				else if (underLimit && pDraggedUnitType->Cost <= currentMoney)
					OccupySlot(carrier, slot, false, true);
				else
					ReturnToSource();
			}
			else if (nSourceDropshipIdx != -1)
			{
				// bay -> bay: swap
				if (pDraggedUnitType == pTarget)
				{
					ReturnToSource(false);
				}
				else if ((bDraggedIsFixed || bTargetIsFixed) && carrier != nSourceDropshipIdx)
				{
					ReturnToSource();
				}
				else if (!CanCarrierHoldUnit(nSourceDropshipIdx, pTarget))
				{
					ReturnToSource();
				}
				else
				{
					dropshipBayChosenUnitsLists[nSourceDropshipIdx][nSourceSlotIdx] = pTarget;
					dropshipBayFixedUnitsLists[nSourceDropshipIdx][nSourceSlotIdx] = bTargetIsFixed;
					OccupySlot(carrier, slot, bDraggedIsFixed, !bDraggedIsFixed);
				}
			}
			else if (bTargetIsFixed)
			{
				ReturnToSource(); // sidebar drags may not overwrite fixed units
			}
			else if (CarrierHasFreeSlot(carrier) && underLimit && pDraggedUnitType->Cost <= currentMoney)
			{
				// Insert at the drop position and let the rest shift down into
				// the first empty slot.
				//
				// DIFF: the original computed
				//     nullIdx = (nSourceDropshipIdx == carrier) ? nSourceSlotIdx : firstNull;
				// but this branch is only reachable when nSourceDropshipIdx == -1,
				// so the first arm was unreachable. Removed.
				auto& units = dropshipBayChosenUnitsLists[carrier];
				auto& fixed = dropshipBayFixedUnitsLists[carrier];

				auto const itNull = std::find(units.begin(), units.end(), nullptr);

				if (itNull != units.end())
				{
					auto const nullIdx = std::distance(units.begin(), itNull);

					units.erase(units.begin() + nullIdx);
					units.insert(units.begin() + slot, pDraggedUnitType);
					fixed.erase(fixed.begin() + nullIdx);
					fixed.insert(fixed.begin() + slot, false);
				}

				currentMoney -= pDraggedUnitType->Cost;
				++dropshipBayChosenUnitsCount[pDraggedUnitType];
				lastSelected = pDraggedUnitType;
				DropshipLoadoutHelpers::PlaySoundIfValid(endingDragDropSoundIdx);
			}
			else if (!TryReplace(carrier, slot, pTarget))
			{
				ReturnToSource();
			}
		}
	}
	else if ((droppedOnSidebar || droppedOnSidebarArea) && nSourceDropshipIdx != -1)
	{
		// SUSPECT: the original comment "Dropped on sidebar -> permanently
		// sold/removed" sat on the `bDraggedIsFixed` arm, which is the arm that
		// does the exact opposite (puts it back). Comment was misplaced; the
		// logic below is what actually shipped.
		if (bDraggedIsFixed)
			ReturnToSource();     // fixed units cannot be sold
		else
			VocClass::PlayGlobal(sellClickSoundIdx, Panning::Center, 1.0); // already refunded at drag start
	}
	else
	{
		ReturnToSource();
	}

	bIsDragging = false;
	pDraggedUnitType = nullptr;
	repaintAll = true;
}

// ============================================================================
// Run
// ============================================================================

void DropshipLoadoutClass::Run()
{
	DSurface* pSurface = DSurface::Hidden;

	if (!pSurface)
		return;

	pSurface->Fill(0);

	CalculateLayout(pSurface);
	CreateControls();

	auto const pGlobal = ScenarioExtData::Instance();

	int const voiceEva = DropshipLoadoutHelpers::PickNullable(&pHouseTypeExt->DropshipLoadout_StartEVA,
		pGlobal ? &pGlobal->DropshipLoadout_StartEVA : nullptr, -1);

	if (voiceEva >= 0)
		VoxClass::PlayIndex(voiceEva);

	int themeIdx = DropshipLoadoutHelpers::PickNullable(&pHouseTypeExt->DropshipLoadout_Theme,
		pGlobal ? &pGlobal->DropshipLoadout_Theme : nullptr, -1);

	if (themeIdx == -1)
		ThemeClass::Instance->Stop(true);
	else
		ThemeClass::Instance->Play(themeIdx);

	if (DisplayClass::Instance->CurrentSWTypeIndex != -1)
		DisplayClass::Instance->CurrentSWTypeIndex = -1;

	if (Unsorted::CurrentSWType() != -1)
		Unsorted::CurrentSWType = -1;

	if (auto const pMouse = WWMouseClass::Instance())
	{
		// SUSPECT: Hide immediately followed by Show, then RefCount is stomped
		// to 0 after CaptureMouse(). This looks like a workaround for an
		// unbalanced show/hide refcount somewhere else rather than something
		// this screen actually needs. Preserved verbatim.
		pMouse->HideCursor();
		pMouse->ShowCursor();
		pMouse->CaptureMouse();
		pMouse->RefCount = 0;
	}

	MouseClass::Instance->UpdateCursor(MouseCursorType::Default, false);

	if (commandManager)
		commandManager->TurnOn();

	loadoutTotalFrames = !dropshipLoadout_LoadoutPCX.empty()
		? static_cast<int>(dropshipLoadout_LoadoutPCX.size()) - 1
		: (dropshipLoadout_Loadout ? dropshipLoadout_Loadout->Frames : 0);

	pilotLitTotalFrames = !dropshipLoadout_PilotLitPCX.empty()
		? static_cast<int>(dropshipLoadout_PilotLitPCX.size()) - 1
		: (dropshipLoadout_PilotLit ? dropshipLoadout_PilotLit->Frames : 0);

	// SUSPECT: Random(0, 0) always returns 0, so the loadout animation has no
	// randomised start delay at all and animTimer_DelayedStartValue_Loadout is
	// effectively a constant zero. Either the range was meant to be non-zero
	// (compare the PilotLit 100..300 below) or the whole delay timer for the
	// loadout animation is dead weight and can be deleted.
	animTimer_DelayedStartValue_Loadout = ScenarioClass::Instance->Random(0, 0);
	animTimer_DelayedStartValue_PilotLit = ScenarioClass::Instance->Random(100, 300);

	animTimer_DelayedStartTimer_Loadout.Start(animTimer_DelayedStartValue_Loadout);
	animTimer_DelayedStartTimer_PilotLit.Start(animTimer_DelayedStartValue_PilotLit);
	animTimer_UpdateFrameTimer_Loadout.Start(loadoutFrameDelay);
	animTimer_UpdateFrameTimer_PilotLit.Start(pilotLitFrameDelay);

	if (sidebarRowAnimationIndex >= 0)
	{
		if (!dropshipLoadout_DGreenListPCX.empty())
		{
			if (sidebarRowAnimationIndex < static_cast<int>(dropshipLoadout_DGreenListPCX.size()))
				sidebarRowAnimationTotalFrames = static_cast<int>(dropshipLoadout_DGreenListPCX[sidebarRowAnimationIndex].size()) - 1;
		}
		else if (sidebarRowAnimationIndex < static_cast<int>(dropshipLoadout_DGreenList.size())
			&& dropshipLoadout_DGreenList[sidebarRowAnimationIndex])
		{
			sidebarRowAnimationTotalFrames = dropshipLoadout_DGreenList[sidebarRowAnimationIndex]->Frames;
		}
	}

	pressedSpaceKey = false;
	repaintAll = true;
	bDropshipLoadoutActive = true;
	pendingScrolls = 0;
	pHoveredUnitType = nullptr;
	hoveredDropshipIdx = -1;
	hoveredSlotIdx = -1;

	if (HWND const hGameWnd = Game::hWnd)
	{
		SetFocus(hGameWnd);
		SetActiveWindow(hGameWnd);
		SetForegroundWindow(hGameWnd);
	}

	// Drain stale keystrokes queued before the screen opened.
	// DIFF: the original used `while (PeekMessage(...)) { /* discard */ }` with
	// an empty body; an expression statement says the same thing without the
	// empty block.
	for (MSG flushMsg; PeekMessage(&flushMsg, nullptr, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE); )
		continue;

	// Edge detection needs the state as of screen entry, otherwise the key that
	// opened the screen immediately closes it again.
	auto const IsDown = [](int vk) { return (GetAsyncKeyState(vk) & 0x8000) != 0; };

	bool wasLButtonDown = IsDown(VK_LBUTTON);
	bool wasRButtonDown = IsDown(VK_RBUTTON);
	bool wasSpaceDown = IsDown(VK_SPACE);
	bool wasEscDown = IsDown(VK_ESCAPE);

	bool ignoreSpaceUntilReleased = wasSpaceDown;
	bool ignoreEscUntilReleased = wasEscDown;

	while (!pressedSpaceKey)
	{
		if (ignoreSpaceUntilReleased && !IsDown(VK_SPACE))
			ignoreSpaceUntilReleased = false;

		if (ignoreEscUntilReleased && !IsDown(VK_ESCAPE))
			ignoreEscUntilReleased = false;

		int command = 0;

		// Drain the key queue before Game::CallBack() gets a chance to eat it.
		for (MSG msg; PeekMessage(&msg, nullptr, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE); )
		{
			bool const isKeyDown = msg.message == WM_KEYDOWN || msg.message == WM_SYSKEYDOWN;
			bool handled = false;

			if (isKeyDown)
			{
				switch (msg.wParam)
				{
				case VK_SPACE:
					if (!ignoreSpaceUntilReleased)
						command = VK_SPACE;

					handled = true;
					break;

				case VK_ESCAPE:
					if (!ignoreEscUntilReleased)
						command = VK_ESCAPE;

					handled = true;
					break;

				case VK_UP:
					command = VK_UP;
					handled = true;
					break;

				case VK_DOWN:
					command = VK_DOWN;
					handled = true;
					break;

				default:
					break;
				}
			}

			if (!handled)
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		Game::CallBack();
		MouseClass::Instance->UpdateCursor(MouseCursorType::Default, false);

		if (command == 0 && commandManager)
			command = static_cast<int>(commandManager->Input());

		bool const isLButtonDown = IsDown(VK_LBUTTON);
		bool const isRButtonDown = IsDown(VK_RBUTTON);
		bool const isSpaceDown = IsDown(VK_SPACE);
		bool const isEscDown = IsDown(VK_ESCAPE);

		if (command == 0)
		{
			if (isLButtonDown && !wasLButtonDown)
				command = 1;
			else if (isRButtonDown && !wasRButtonDown)
				command = 2;
		}

		if (isSpaceDown && !wasSpaceDown && !ignoreSpaceUntilReleased)
			command = VK_SPACE;
		else if (isEscDown && !wasEscDown && !ignoreEscUntilReleased)
			command = VK_ESCAPE;

		wasLButtonDown = isLButtonDown;
		wasRButtonDown = isRButtonDown;
		wasSpaceDown = isSpaceDown;
		wasEscDown = isEscDown;

		int const buttonID = HitTestButton();

		UpdateDragState(buttonID);
		HandleInput(command, buttonID);
		UpdateAnimations();

		if (bIsDragging)
			repaintAll = true;

		if (repaintAll)
		{
			Render(pSurface);
			repaintAll = false;
		}

		MouseClass::Instance->UpdateCursor(MouseCursorType::Default, false);
		GScreenClass::Instance->DoBlit(true, pSurface, nullptr);

		// OPTIMIZE: Sleep(1) inside a spin loop gives an uncapped frame rate
		// bounded only by the scheduler quantum - this screen will happily peg
		// a core. A frame limiter tied to the game's own tick would be kinder,
		// and Render() is already gated behind repaintAll so the cost is mostly
		// DoBlit + Game::CallBack.
		Sleep(1);
	}

	bDropshipLoadoutActive = false;
	SaveCargo();
}

// ============================================================================
// HandleInput
// ============================================================================

void DropshipLoadoutClass::HandleInput(int command, int buttonID)
{
	const int btn_ScrollUp_ID = 100;
	const int btn_ScrollDown_ID = 101;
	const int btn_BasicDropshipCameo_ID = 200;
	const int btn_BasicSidebarCameo_ID = 300;

	// The drag state machine owns the mouse while a drag is live.
	if (bIsDragging || bDragPending)
		return;

	bool const pressedLeftClick = command == 1;
	bool const pressedRightClick = command == 2;

	bool const isAnySidebarCameo = buttonID >= btn_BasicSidebarCameo_ID
		&& buttonID < (btn_BasicSidebarCameo_ID + nSidebarCameos);

	bool const isAnyDropshipCameo = buttonID >= btn_BasicDropshipCameo_ID
		&& buttonID < (btn_BasicDropshipCameo_ID + nDropshipBayTotalSlots);

	int const sidebarIndex = isAnySidebarCameo
		? firstBrowsableCameo + (buttonID - btn_BasicSidebarCameo_ID)
		: -1;

	int const bayCarrier = isAnyDropshipCameo && nDropshipBayCameos > 0
		? (buttonID - btn_BasicDropshipCameo_ID) / nDropshipBayCameos
		: -1;

	int const baySlot = bayCarrier >= 0
		? (buttonID - btn_BasicDropshipCameo_ID) - (bayCarrier * nDropshipBayCameos)
		: -1;

	// Bounds-checked accessors; both are needed in several places below.
	auto const SidebarUnit = [&]() -> TechnoTypeClass*
		{
			return (sidebarIndex >= 0 && sidebarIndex < static_cast<int>(availableUnits.size()))
				? availableUnits[sidebarIndex]
				: nullptr;
		};

	auto const BayUnit = [&]() -> TechnoTypeClass*
		{
			if (bayCarrier < 0 || bayCarrier >= static_cast<int>(dropshipBayChosenUnitsLists.size()))
				return nullptr;

			if (baySlot < 0 || baySlot >= static_cast<int>(dropshipBayChosenUnitsLists[bayCarrier].size()))
				return nullptr;

			return dropshipBayChosenUnitsLists[bayCarrier][baySlot];
		};

	// --- Begin a drag --------------------------------------------------------
	if (pressedLeftClick)
	{
		if (auto const pSidebarType = SidebarUnit())
		{
			if (GetInstanceCount(pSidebarType) < GetMaxInstances(pSidebarType))
			{
				bDragPending = true;
				pDraggedUnitType = pSidebarType;
				nSourceDropshipIdx = -1;
				nSourceSlotIdx = -1;
				dragStartMousePos = DropshipLoadoutHelpers::CursorPosition();
				return;
			}
		}
		else if (auto const pBayType = BayUnit())
		{
			bDragPending = true;
			pDraggedUnitType = pBayType;
			nSourceDropshipIdx = bayCarrier;
			nSourceSlotIdx = baySlot;
			dragStartMousePos = DropshipLoadoutHelpers::CursorPosition();
			return;
		}
	}

	// --- Hover tracking ------------------------------------------------------
	TechnoTypeClass* const pPrevHovered = pHoveredUnitType;
	pHoveredUnitType = nullptr;
	hoveredDropshipIdx = -1;
	hoveredSlotIdx = -1;

	if (isAnySidebarCameo)
	{
		pHoveredUnitType = SidebarUnit();
	}
	else if (isAnyDropshipCameo && bayCarrier < static_cast<int>(dropshipBayChosenUnitsLists.size())
		&& baySlot < static_cast<int>(dropshipBayChosenUnitsLists[bayCarrier].size()))
	{
		pHoveredUnitType = dropshipBayChosenUnitsLists[bayCarrier][baySlot];
		hoveredDropshipIdx = bayCarrier;
		hoveredSlotIdx = baySlot;
	}

	if (pHoveredUnitType != pPrevHovered)
		repaintAll = true;

	// --- Scroll intent -------------------------------------------------------
	bool pressedUpArrow = command == VK_UP || (pressedLeftClick && buttonID == btn_ScrollUp_ID);
	bool pressedDownArrow = command == VK_DOWN || (pressedLeftClick && buttonID == btn_ScrollDown_ID);

	// NOTE: pendingScrolls is never written in this file - see the cross-cutting
	// note at the top. Wheel scrolling depends on an external hook.
	bool const isScrollFromWheel = pendingScrolls != 0;

	if (pendingScrolls < 0)
	{
		pressedUpArrow = true;
		++pendingScrolls;
	}
	else if (pendingScrolls > 0)
	{
		pressedDownArrow = true;
		--pendingScrolls;
	}

	bool const playScrollSound = !isScrollFromWheel;

	bool const isHoveringSidebarCameo = command == 0 && isAnySidebarCameo;
	bool const isHoveringBayCameo = command == 0 && isAnyDropshipCameo;
	bool const pressedSidebarBuy = pressedLeftClick && isAnySidebarCameo;
	bool const pressedSidebarSell = pressedRightClick && isAnySidebarCameo;
	bool const pressedBaySell = pressedRightClick && isAnyDropshipCameo;

	// DIFF: removed `mouseLocationInDropshipCameos` and the O(n*m) double loop
	// that computed it - the value was never read, and the indices it searched
	// for are already available directly as bayCarrier/baySlot.

	// DIFF: `freeDropshipSlots` is a member that nothing else in this file
	// reads. Kept updated in case an external hook consumes it; delete it and
	// this line together if not.
	freeDropshipSlots = false;

	for (int i = 0; i < static_cast<int>(dropshipBayChosenUnitsLists.size()) && !freeDropshipSlots; ++i)
		freeDropshipSlots = CarrierHasFreeSlot(i);

	// --- Actions -------------------------------------------------------------
	if (pressedUpArrow)
	{
		if (firstBrowsableCameo >= 2)
		{
			firstBrowsableCameo -= 2;
			repaintAll = true;

			if (playScrollSound)
				VocClass::PlayGlobal(arrowsClickSoundIdx, Panning::Center, 1.0);
		}
	}
	else if (pressedDownArrow)
	{
		if (availableUnits.size() > static_cast<size_t>(firstBrowsableCameo + nSidebarCameos))
		{
			firstBrowsableCameo += 2;
			repaintAll = true;

			if (playScrollSound)
				VocClass::PlayGlobal(arrowsClickSoundIdx, Panning::Center, 1.0);
		}
	}
	else if (pressedSidebarSell)
	{
		// Right-clicking a sidebar cameo refunds the LAST placed copy of it.
		if (auto const pType = SidebarUnit())
		{
			bool sold = false;

			for (int i = static_cast<int>(dropshipBayChosenUnitsLists.size()) - 1; i >= 0 && !sold; --i)
			{
				auto const& bay = dropshipBayChosenUnitsLists[i];

				for (int j = static_cast<int>(bay.size()) - 1; j >= 0 && !sold; --j)
				{
					if (bay[j] == pType && !dropshipBayFixedUnitsLists[i][j])
						sold = SellUnitAt(i, j);
				}
			}
		}
	}
	else if (pressedSidebarBuy)
	{
		auto const pType = SidebarUnit();

		// BuyIntoFirstFreeSlot performs the limit/affordability/capacity checks
		// that the original duplicated into `validSidebarCameoPurchase`.
		if (pType && BuyIntoFirstFreeSlot(pType))
		{
			if (sidebarRowAnimationIndex < 0)
			{
				sidebarRowAnimationIndex = (buttonID - btn_BasicSidebarCameo_ID) / 2;

				size_t const rowCount = !dropshipLoadout_DGreenListPCX.empty()
					? dropshipLoadout_DGreenListPCX.size()
					: dropshipLoadout_DGreenList.size();

				if (sidebarRowAnimationIndex < static_cast<int>(rowCount))
				{
					animTimer_UpdateFrameTimer_SidebarRowAnimation.Start(sidebarRowAnimationFrameDelay);

					if (!dropshipLoadout_DGreenListPCX.empty())
					{
						sidebarRowAnimationTotalFrames =
							static_cast<int>(dropshipLoadout_DGreenListPCX[sidebarRowAnimationIndex].size()) - 1;
					}
					else
					{
						auto const pShape = dropshipLoadout_DGreenList[sidebarRowAnimationIndex];
						sidebarRowAnimationTotalFrames = pShape ? pShape->Frames : 0;
					}
				}
				else
				{
					sidebarRowAnimationIndex = -1;
					sidebarRowAnimationTotalFrames = 0;
				}
			}
		}
	}
	else if (pressedBaySell)
	{
		SellUnitAt(bayCarrier, baySlot);
	}
	else if (isHoveringBayCameo || isHoveringSidebarCameo)
	{
		lastTimeWasOverCameos = true;
		repaintAll = true;
	}
	else if (lastTimeWasOverCameos)
	{
		lastTimeWasOverCameos = false;
		repaintAll = true;
	}

	if (command == VK_SPACE)
		pressedSpaceKey = true;

	if (command != VK_ESCAPE)
		return;

	// --- Escape: sell everything that is not fixed ---------------------------
	bool soldAny = false;
	lastSelected = nullptr;
	dropshipBayChosenUnitsCount.clear();

	for (size_t i = 0; i < dropshipBayChosenUnitsLists.size(); ++i)
	{
		auto& units = dropshipBayChosenUnitsLists[i];
		auto& fixed = dropshipBayFixedUnitsLists[i];
		size_t const slotCount = units.size();

		std::vector<TechnoTypeClass*> newUnits;
		std::vector<bool> newFixed;

		for (size_t j = 0; j < slotCount; ++j)
		{
			if (fixed[j])
			{
				newUnits.push_back(units[j]);
				newFixed.push_back(true);
			}
			else if (units[j])
			{
				soldAny = true;
			}
		}

		newUnits.resize(slotCount, nullptr);
		newFixed.resize(slotCount, false);

		units = std::move(newUnits);
		fixed = std::move(newFixed);
	}

	currentMoney = initialMoney;
	repaintAll = true;

	if (soldAny)
		VocClass::PlayGlobal(sellClickSoundIdx, Panning::Center, 1.0);
}

// ============================================================================
// UpdateAnimations
// ============================================================================

void DropshipLoadoutClass::UpdateAnimations()
{
	// The Loadout and PilotLit blocks were identical apart from the restart
	// delay range, so the shared shape is a lambda (single call site each, but
	// the pattern repeated inside this one function).
	auto const AdvanceLooping = [this](int& currentFrame, int totalFrames, auto& frameTimer, int frameDelay,
		auto& delayTimer, int& delayValue, int delayMin, int delayMax)
		{
			if (!delayTimer.Completed() || !frameTimer.Completed())
				return;

			if (currentFrame < totalFrames)
			{
				++currentFrame;
			}
			else
			{
				currentFrame = -1;
				delayValue = ScenarioClass::Instance->Random(delayMin, delayMax);
				delayTimer.Start(delayValue);
			}

			frameTimer.Start(frameDelay);
			repaintAll = true;
		};

	AdvanceLooping(currentLoadoutFrame, loadoutTotalFrames,
		animTimer_UpdateFrameTimer_Loadout, loadoutFrameDelay,
		animTimer_DelayedStartTimer_Loadout, animTimer_DelayedStartValue_Loadout, 0, 0);

	AdvanceLooping(currentPilotLitFrame, pilotLitTotalFrames,
		animTimer_UpdateFrameTimer_PilotLit, pilotLitFrameDelay,
		animTimer_DelayedStartTimer_PilotLit, animTimer_DelayedStartValue_PilotLit, 100, 300);

	// The sidebar row flash plays once and then disarms itself, so it does not
	// fit the looping shape above.
	if (sidebarRowAnimationIndex >= 0 && animTimer_UpdateFrameTimer_SidebarRowAnimation.Completed())
	{
		if (currentSidebarRowAnimationFrame < sidebarRowAnimationTotalFrames)
		{
			++currentSidebarRowAnimationFrame;
			animTimer_UpdateFrameTimer_SidebarRowAnimation.Start(sidebarRowAnimationFrameDelay);
		}
		else
		{
			currentSidebarRowAnimationFrame = -1;
			sidebarRowAnimationIndex = -1;
		}

		repaintAll = true;
	}

	// SUSPECT: animTimer_UpdateFrameTimer / animTimer_StartValue are restarted
	// here but never queried anywhere in this file. Either a consumer is
	// missing or this is a leftover from an earlier design - safe to delete
	// once you confirm no hook reads it.
	if (animTimer_UpdateFrameTimer.Completed())
		animTimer_UpdateFrameTimer.Start(animTimer_StartValue);
}

// ============================================================================
// Render
// ============================================================================

void DropshipLoadoutClass::Render(DSurface* pSurface)
{
	if (!pSurface)
		return;

	pSurface->Fill(0);

	GeneralUtils::DrawImage(
		pSurface,
		windowRectangle,
		dropshipLoadout_BackgroundPCX,
		dropshipLoadout_Background,
		dropshipLoadout_Palette
	);

	RectangleStruct const cursor = DropshipLoadoutHelpers::CursorRect();

	bool isHoveringSidebar = false;

	for (auto const& rect : sidebarCameLocations)
	{
		if (DropshipLoadoutHelpers::RectContains(rect, cursor.X, cursor.Y))
		{
			isHoveringSidebar = true;
			break;
		}
	}

	bool const isMouseOverSidebarArea = !sidebarCameLocations.empty()
		&& DropshipLoadoutHelpers::RectContains(GetSidebarArea(), cursor.X, cursor.Y);

	// --- Sidebar cameos ------------------------------------------------------
	for (int i = 0; i < nSidebarCameos && i < static_cast<int>(sidebarCameLocations.size()); ++i)
	{
		int const unitIndex = firstBrowsableCameo + i;

		if (unitIndex >= static_cast<int>(availableUnits.size()))
			continue;

		auto const pType = availableUnits[unitIndex];

		if (!pType)
			continue;

		bool const limitReached = GetInstanceCount(pType) >= GetMaxInstances(pType);
		bool const hasFreeSlot = HasCompatibleFreeSlot(pType);

		BlitterFlags const bf = (limitReached || !hasFreeSlot)
			? BlitterFlags(BlitterFlags::bf_400 | BlitterFlags::Darken)
			: BlitterFlags::None;

		bool const isHovering = !bIsDragging && !bDragPending && !pDraggedUnitType
			&& DropshipLoadoutHelpers::RectContains(sidebarCameLocations[i], cursor.X, cursor.Y);

		ColorStruct foreColor;
		bool showHighlight = false;

		if (isHovering)
		{
			showHighlight = true;

			bool const canBuyDirectly = !limitReached && pType->Cost <= currentMoney && hasFreeSlot;
			bool canReplaceAny = false;

			// OPTIMIZE: this scans every occupied slot for every hovered cameo,
			// every repaint. It only needs the CHEAPEST occupant in the bays -
			// caching that once per repaint turns O(slots) into O(1) here.
			if (!limitReached)
			{
				for (auto const& bay : dropshipBayChosenUnitsLists)
				{
					for (auto const pTarget : bay)
					{
						if (pTarget && pType != pTarget && (pType->Cost - pTarget->Cost) <= currentMoney)
						{
							canReplaceAny = true;
							break;
						}
					}

					if (canReplaceAny)
						break;
				}
			}

			if (canBuyDirectly)
				foreColor = ColorStruct { 0, 255, 0 };   // green  - affordable
			else if (canReplaceAny)
				foreColor = ColorStruct { 0, 0, 255 };   // blue   - only via replacement
			else
				foreColor = ColorStruct { 255, 0, 0 };   // red    - unavailable
		}
		else if (pType == lastSelected)
		{
			showHighlight = true;
			foreColor = ColorStruct { 255, 239, 99 };    // yellow - last selected
		}

		if (showHighlight)
			DropshipLoadoutHelpers::DrawSlotHighlight(pSurface, sidebarCameLocations[i], foreColor, 255);

		DropshipLoadoutHelpers::DrawCameo(pSurface, sidebarCameLocations[i], pType, bf);
	}

	// --- Scroll arrows -------------------------------------------------------
	GeneralUtils::DrawImage(pSurface, upArrowLocation, dropshipLoadout_UpArrowPCX,
		dropshipLoadout_UpArrow, dropshipLoadout_Palette, 0, -2);

	GeneralUtils::DrawImage(pSurface, downArrowLocation, dropshipLoadout_DownArrowPCX,
		dropshipLoadout_DownArrow, dropshipLoadout_Palette, 0, -2);

	// --- Bay slots -----------------------------------------------------------
	// The original nested this colour decision 7 levels deep inline. Same
	// rules, flattened; single call site so it stays a lambda.
	auto const ResolveBayHighlight = [&](int carrier, int slot, TechnoTypeClass* pType, ColorStruct& colorOut) -> bool
		{
			bool const isHovering = DropshipLoadoutHelpers::RectContains(dropshipBayCameLocations[carrier][slot], cursor.X, cursor.Y);
			bool const bTargetIsFixed = dropshipBayFixedUnitsLists[carrier][slot];

			if (isHovering && bIsDragging)
			{
				if (bDraggedIsFixed && carrier != nSourceDropshipIdx)
					return false; // a fixed unit may not leave its own bay

				if (!CanCarrierHoldUnit(carrier, pDraggedUnitType))
				{
					colorOut = ColorStruct { 170, 0, 255 }; // violet - too heavy
					return true;
				}

				if (!pType)
				{
					if (!bDraggedIsFixed && pDraggedUnitType->Cost > currentMoney)
						return false;

					colorOut = ColorStruct { 0, 0, 255 };   // blue - valid empty slot
					return true;
				}

				if (nSourceDropshipIdx != -1)
				{
					if (pDraggedUnitType == pType)
						return false;

					if ((bDraggedIsFixed || bTargetIsFixed) && carrier != nSourceDropshipIdx)
						return false;

					colorOut = CanCarrierHoldUnit(nSourceDropshipIdx, pType)
						? ColorStruct { 0, 0, 255 }         // blue   - swap
					: ColorStruct { 170, 0, 255 };      // violet - source cannot take it back

					return true;
				}

				if (bTargetIsFixed)
					return false;

				bool const canAffordShift = pDraggedUnitType->Cost <= currentMoney;
				bool const canAffordReplacement = (pDraggedUnitType->Cost - pType->Cost) <= currentMoney;

				if (CarrierHasFreeSlot(carrier) && canAffordShift)
				{
					colorOut = ColorStruct { 0, 0, 255 };   // blue - shift insert
					return true;
				}

				if (canAffordReplacement && pDraggedUnitType != pType)
				{
					colorOut = ColorStruct { 255, 0, 0 };   // red - overwrite
					return true;
				}

				return false;
			}

			if (isHovering)
			{
				if (!pType || bTargetIsFixed)
					return false;

				colorOut = ColorStruct { 255, 0, 0 };       // red - right-click sells
				return true;
			}

			if (bIsDragging || !isHoveringSidebar || !pHoveredUnitType)
				return false;

			// Not hovering this slot, but hovering a sidebar cameo: preview
			// where that unit could go.
			if (!CanCarrierHoldUnit(carrier, pHoveredUnitType))
			{
				colorOut = ColorStruct { 170, 0, 255 };     // violet - too heavy
				return true;
			}

			if (!pType)
				return false;

			bool const limitReached = GetInstanceCount(pHoveredUnitType) >= GetMaxInstances(pHoveredUnitType);
			bool const canBuyDirectly = !limitReached
				&& pHoveredUnitType->Cost <= currentMoney
				&& HasCompatibleFreeSlot(pHoveredUnitType);

			if (pHoveredUnitType == pType && !bTargetIsFixed)
			{
				if (!limitReached)
					return false;

				colorOut = ColorStruct { 255, 0, 0 };       // red - shows where the cap is spent
				return true;
			}

			// Replacement hints only matter when a plain purchase is impossible.
			if (canBuyDirectly || bTargetIsFixed || limitReached)
				return false;

			if ((pHoveredUnitType->Cost - pType->Cost) > currentMoney)
				return false;

			colorOut = ColorStruct { 0, 0, 255 };           // blue - replaceable
			return true;
		};

	for (int i = 0; i < static_cast<int>(dropshipBayCameLocations.size()); ++i)
	{
		if (i >= static_cast<int>(dropshipBayChosenUnitsLists.size()))
			continue;

		for (int j = 0; j < static_cast<int>(dropshipBayCameLocations[i].size()); ++j)
		{
			if (j >= static_cast<int>(dropshipBayChosenUnitsLists[i].size()))
				continue;

			auto const pType = dropshipBayChosenUnitsLists[i][j];

			ColorStruct foreColor;

			if (ResolveBayHighlight(i, j, pType, foreColor))
			{
				// Empty slots get a fainter wash so the cameos stay readable.
				DropshipLoadoutHelpers::DrawSlotHighlight(pSurface, dropshipBayCameLocations[i][j], foreColor, pType ? 255 : 76);
			}

			if (!pType)
				continue;

			BlitterFlags bf = dropshipBayFixedUnitsLists[i][j]
				? BlitterFlags(BlitterFlags::bf_400 | BlitterFlags::Darken)
				: BlitterFlags::None;

			DropshipLoadoutHelpers::DrawCameo(pSurface, dropshipBayCameLocations[i][j], pType, bf);
		}
	}

	// --- Decorative animations ----------------------------------------------
	auto const DrawAnimFrame = [&](const RectangleStruct& location, const std::vector<BSurface*>& pcxFrames,
		SHPStruct* pShape, int frame)
		{
			BSurface* pFramePCX = nullptr;

			if (frame >= 0 && frame < static_cast<int>(pcxFrames.size()))
				pFramePCX = pcxFrames[frame];

			GeneralUtils::DrawImage(pSurface, location, pFramePCX, pShape, dropshipLoadout_Palette, frame, -2);
		};

	if (currentLoadoutFrame >= 0)
		DrawAnimFrame(loadoutLocation, dropshipLoadout_LoadoutPCX, dropshipLoadout_Loadout, currentLoadoutFrame);

	if (currentPilotLitFrame >= 0)
		DrawAnimFrame(pilotLitLocation, dropshipLoadout_PilotLitPCX, dropshipLoadout_PilotLit, currentPilotLitFrame);

	if (sidebarRowAnimationIndex >= 0
		&& currentSidebarRowAnimationFrame >= 0
		&& sidebarRowAnimationIndex < static_cast<int>(dGreenLocation.size()))
	{
		static const std::vector<BSurface*> NoFrames;

		auto const& frames = sidebarRowAnimationIndex < static_cast<int>(dropshipLoadout_DGreenListPCX.size())
			? dropshipLoadout_DGreenListPCX[sidebarRowAnimationIndex]
			: NoFrames;

		SHPStruct* pShape = nullptr;

		if (sidebarRowAnimationIndex < static_cast<int>(dropshipLoadout_DGreenList.size()))
			pShape = dropshipLoadout_DGreenList[sidebarRowAnimationIndex];

		DrawAnimFrame(dGreenLocation[sidebarRowAnimationIndex], frames, pShape, currentSidebarRowAnimationFrame);
	}

	// --- Labels --------------------------------------------------------------
	// DIFF: was `wchar_t buffer[64]` reused for both labels.
	// BUGFIX: the second label did `swprintf_s(buffer, csfStartMission)` - a CSF
	// string passed as the FORMAT argument with no varargs. Any '%' a translator
	// types crashes or leaks stack. It is a plain string, so it is now printed
	// directly with no formatting pass.
	COLORREF foreColor = Drawing::RGB2DWORD(255, 239, 99);
	TextPrintType style = TextPrintType::FullShadow | TextPrintType::Point6Grad;

	const wchar_t* const csfCredits = GeneralUtils::LoadStringUnlessMissing("TXT_DROPSHIP_CREDITS", L"Credits: %d");
	std::wstring creditsText(128, L'\0');
	int const creditsLen = _snwprintf_s(creditsText.data(), creditsText.size(), _TRUNCATE, csfCredits, static_cast<int>(currentMoney));
	creditsText.resize(creditsLen > 0 ? static_cast<size_t>(creditsLen) : 0u);

	Point2D creditsLabel = { windowRectangle.Width - 140, windowRectangle.Height - 15 };
	pSurface->DSurfaceDrawText(creditsText.c_str(), &windowRectangle, &creditsLabel, foreColor, 0, style);

	const wchar_t* const csfStartMission = GeneralUtils::LoadStringUnlessMissing("TXT_DROPSHIP_START_MISSION", L"Press SPACE to continue");
	foreColor = Drawing::RGB2DWORD(255, 255, 255);
	style = TextPrintType::Center | TextPrintType::FullShadow | TextPrintType::Point6Grad;

	Point2D pressSpaceLabel = { (windowRectangle.Width - 175) / 2, windowRectangle.Height - 15 };
	pSurface->DSurfaceDrawText(csfStartMission, &windowRectangle, &pressSpaceLabel, foreColor, 0, style);

	// --- Dragged cameo -------------------------------------------------------
	if (bIsDragging && pDraggedUnitType)
	{
		const int cameoWidth = 60;
		const int cameoHeight = 48;

		Point2D const mousePos = DropshipLoadoutHelpers::CursorPosition();
		RectangleStruct const dragLoc = {
			mousePos.X - cameoWidth / 2,
			mousePos.Y - cameoHeight / 2,
			cameoWidth, cameoHeight
		};

		ColorStruct const dragBorderColor = (nSourceDropshipIdx != -1 && isMouseOverSidebarArea)
			? ColorStruct { 255, 0, 0 }   // red - dropping here sells it
		: ColorStruct { 0, 0, 255 };  // blue

		DropshipLoadoutHelpers::DrawSlotHighlight(pSurface, dragLoc, dragBorderColor, 255);
		DropshipLoadoutHelpers::DrawCameo(pSurface, dragLoc, pDraggedUnitType, BlitterFlags::bf_400 | BlitterFlags::Darken);
	}

	DrawTooltip(pSurface);
}

// ============================================================================
// DrawTooltip
// ============================================================================

void DropshipLoadoutClass::DrawTooltip(DSurface* pSurface)
{
	if (bIsDragging || !pHoveredUnitType || !pSurface)
		return;

	if (!BitFont::Instance() || !BitText::Instance())
		return;

	int const maxToolTipWidth = Phobos::UI::MaxToolTipWidth > 0 ? Phobos::UI::MaxToolTipWidth : 200;

	// The original measured four blocks and then drew them again with the same
	// coordinate arithmetic duplicated. Building the lines once removes the
	// possibility of the two passes disagreeing.
	struct Segment
	{
		std::wstring Text;
		COLORREF Color = 0;
		int Width = 0;
		int Height = 0;
	};

	struct Line
	{
		std::vector<Segment> Segments;
		int GapBefore = 0;
	};

	std::vector<Line> lines;

	auto const AddLine = [&](int gapBefore) -> Line&
		{
			auto& line = lines.emplace_back();
			line.GapBefore = gapBefore;
			return line;
		};

	auto const AddSegment = [&](Line& line, std::wstring text, COLORREF color)
		{
			auto& segment = line.Segments.emplace_back();
			segment.Text = std::move(text);
			segment.Color = color;
			BitFont::Instance->GetTextDimension(segment.Text.c_str(), &segment.Width, &segment.Height, maxToolTipWidth);
		};

	bool const isHoveredInDropship = hoveredDropshipIdx != -1;
	bool const isHoveredFixed = isHoveredInDropship
		&& hoveredDropshipIdx < static_cast<int>(dropshipBayFixedUnitsLists.size())
		&& hoveredSlotIdx < static_cast<int>(dropshipBayFixedUnitsLists[hoveredDropshipIdx].size())
		&& dropshipBayFixedUnitsLists[hoveredDropshipIdx][hoveredSlotIdx];

	// --- Name ---------------------------------------------------------------
	AddSegment(AddLine(0), pHoveredUnitType->UIName, Drawing::RGB2DWORD(255, 239, 99));

	// --- Availability (only when a cap is configured) -----------------------
	int const maxLimit = GetConfiguredMaximum(pHoveredUnitType);
	int const currentCount = GetInstanceCount(pHoveredUnitType);

	if (maxLimit > 0 && !isHoveredFixed)
	{
		int const remaining = maxLimit - currentCount;

		COLORREF availColor = Drawing::RGB2DWORD(255, 255, 255);

		if (remaining == 0)
			availColor = Drawing::RGB2DWORD(255, 0, 0);
		else if (remaining * 2 <= maxLimit)
			availColor = Drawing::RGB2DWORD(255, 255, 0);

		auto& line = AddLine(2);
		AddSegment(line, GeneralUtils::LoadStringUnlessMissing("TXT_DROPSHIP_AVAILABLE", L"Available: "), Drawing::RGB2DWORD(255, 255, 255));
		AddSegment(line, fmt::format(L"{}/{}", remaining, maxLimit), availColor);
	}

	// --- Cost ---------------------------------------------------------------
	if (!isHoveredFixed)
	{
		int const cost = pHoveredUnitType->GetActualCost(HouseClass::CurrentPlayer);

		COLORREF costColor = Drawing::RGB2DWORD(255, 255, 255);

		// SUSPECT: affordability colouring is suppressed for units already in a
		// bay. That is presumably deliberate (you are not buying it again), but
		// it means an unaffordable unit shows white in the bay and red in the
		// sidebar for the same hover session.
		if (!isHoveredInDropship)
		{
			if (currentMoney < cost)
				costColor = Drawing::RGB2DWORD(255, 0, 0);
			else if (currentMoney < cost * 2)
				costColor = Drawing::RGB2DWORD(255, 255, 0);
		}

		auto& line = AddLine(2);
		AddSegment(line, GeneralUtils::LoadStringUnlessMissing("TXT_DROPSHIP_COST", L"Cost: "), Drawing::RGB2DWORD(255, 255, 255));
		AddSegment(line, fmt::format(L"{}{}", Phobos::UI::CostLabel.c_str(), std::abs(cost)), costColor);
	}

	// --- Description --------------------------------------------------------
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pHoveredUnitType);

	if (Phobos::Config::ToolTipDescriptions && pTypeExt && !pTypeExt->UIDescription.Get().empty())
		AddSegment(AddLine(4), pTypeExt->UIDescription.Get().Text, Drawing::RGB2DWORD(200, 200, 200));

	// --- Box geometry --------------------------------------------------------
	const int boxPadding = 5;
	int textWidth = 0;
	int textHeight = 0;

	for (auto const& line : lines)
	{
		int lineWidth = 0;
		int lineHeight = 0;

		for (auto const& segment : line.Segments)
		{
			lineWidth += segment.Width;
			lineHeight = std::max(lineHeight, segment.Height);
		}

		textWidth = std::max(textWidth, lineWidth);
		textHeight += line.GapBefore + lineHeight;
	}

	int const boxWidth = textWidth + boxPadding * 2;
	int const boxHeight = textHeight + boxPadding * 2;

	Point2D const mousePos = DropshipLoadoutHelpers::CursorPosition();

	int const minX = windowRectangle.X;
	int const maxX = windowRectangle.X + windowRectangle.Width;
	int const minY = windowRectangle.Y;
	int const maxY = windowRectangle.Y + windowRectangle.Height;

	int boxX = mousePos.X + 15;
	int boxY = mousePos.Y + 15;

	if (boxX + boxWidth > maxX)
		boxX = mousePos.X - boxWidth - 5;

	if (boxY + boxHeight > maxY)
		boxY = maxY - boxHeight - 5;

	boxX = std::max(boxX, minX);
	boxY = std::max(boxY, minY);

	RectangleStruct boxRect = { boxX, boxY, boxWidth, boxHeight };

	ColorStruct bgColor(0, 0, 0);
	pSurface->Fill_Rect_Trans(&boxRect, &bgColor, 180);
	pSurface->Draw_Rect(boxRect, Drawing::RGB2DWORD(120, 120, 120));

	// --- Draw ----------------------------------------------------------------
	// SUSPECT: Bounds is restored by direct field assignment while it was set
	// through SetBounds(). If SetBounds() does anything besides store the rect
	// (clip recalculation, dirty flag) that side effect is never undone. Use a
	// second SetBounds(&oldBounds) if that turns out to be the case.
	LTRBStruct const oldBounds = BitFont::Instance->Bounds;
	WORD const oldColor = BitFont::Instance->Color;
	bool const oldField41 = BitFont::Instance->field_41;

	LTRBStruct ltrbBounds = { boxRect.X, boxRect.Y, boxRect.X + boxRect.Width, boxRect.Y + boxRect.Height };
	BitFont::Instance->field_41 = 1;
	BitFont::Instance->SetBounds(&ltrbBounds);

	int currentY = boxRect.Y + boxPadding;

	for (auto const& line : lines)
	{
		currentY += line.GapBefore;

		int currentX = boxRect.X + boxPadding;
		int lineHeight = 0;

		for (auto const& segment : line.Segments)
		{
			BitFont::Instance->Color = static_cast<WORD>(segment.Color);

			BitText::Instance->DrawText(
				BitFont::Instance,
				pSurface,
				segment.Text.c_str(),
				currentX,
				currentY,
				segment.Width,
				segment.Height,
				0, 0, 0
			);

			currentX += segment.Width;
			lineHeight = std::max(lineHeight, segment.Height);
		}

		currentY += lineHeight;
	}

	BitFont::Instance->Bounds = oldBounds;
	BitFont::Instance->Color = oldColor;
	BitFont::Instance->field_41 = oldField41;
}

// ============================================================================
// Carrier capacity
// ============================================================================

int DropshipLoadoutClass::GetCarrierSizeLimit(int carrierIdx)
{
	if (carrierIdx < 0 || carrierIdx >= nStartingDropships)
		return -1;

	if (pSWTypeExt)
	{
		if (pSWTypeExt->DropshipLoadout_SizeLimit.isset())
			return pSWTypeExt->DropshipLoadout_SizeLimit.Fetch();

		if (pSWTypeExt->DropshipLoadout_Carrier.isset())
			return static_cast<int>(pSWTypeExt->DropshipLoadout_Carrier.Fetch()->SizeLimit);

		if (auto const pHouseExt = HouseExtContainer::Instance.Find(HouseClass::CurrentPlayer))
		{
			if (pHouseExt->DropshipLoadout_SWCarrier)
				return static_cast<int>(pHouseExt->DropshipLoadout_SWCarrier->SizeLimit);
		}

		return -1;
	}

	auto const sizeLimits = GatherCarrierSizeLimits();

	int const configuredLimit = carrierIdx < static_cast<int>(sizeLimits.size())
		? sizeLimits[carrierIdx]
		: -1;

	// SUSPECT: 0 is a magic value meaning "inherit the carrier's own SizeLimit",
	// while -1 means "unlimited" and any positive number is a literal cap. Three
	// meanings on one int with no named constants - at minimum this wants
	// `constexpr int SizeLimit_InheritFromCarrier = 0;`.
	if (configuredLimit != 0)
		return configuredLimit;

	auto const carriers = GatherCarriers();

	if (carrierIdx < static_cast<int>(carriers.size()) && carriers[carrierIdx])
		return static_cast<int>(carriers[carrierIdx]->SizeLimit);

	return configuredLimit;
}

bool DropshipLoadoutClass::CanCarrierHoldUnit(int carrierIdx, TechnoTypeClass* pUnitType)
{
	if (!pUnitType)
		return true;

	int const limit = GetCarrierSizeLimit(carrierIdx);

	if (limit == -1)
		return true;

	return static_cast<int>(pUnitType->Size) <= limit;
}

// ============================================================================
// SaveCargo
// ============================================================================

std::vector<TechnoTypeClass*> DropshipLoadoutClass::RetainInitialUnitsPresentInCargo(
	const std::vector<TechnoTypeClass*>& initialUnits,
	std::vector<TechnoTypeClass*> cargoCopy)
{
	// Initial units the player threw away are consumed permanently; the ones
	// still loaded carry over to the next screen.
	std::vector<TechnoTypeClass*> retained;

	for (auto const pUnit : initialUnits)
	{
		if (!pUnit)
			continue;

		auto const it = std::find(cargoCopy.begin(), cargoCopy.end(), pUnit);

		if (it == cargoCopy.end())
			continue;

		retained.push_back(pUnit);
		cargoCopy.erase(it);
	}

	return retained;
}

void DropshipLoadoutClass::SettleMoney(bool addUnusedMoneyToPlayer, bool hasConfiguredMoney)
{
	auto const DeductSpent = [this]()
		{
			long const spent = HouseClass::CurrentPlayer->Available_Money() - currentMoney;
			HouseClass::CurrentPlayer->TransactMoney(-spent);
		};

	// startingMoney == -1 means "spend out of the player's own wallet".
	if (this->startingMoney == -1)
	{
		DeductSpent();
		return;
	}

	if (addUnusedMoneyToPlayer)
	{
		HouseClass::CurrentPlayer->TransactMoney(currentMoney);
		return;
	}

	// startingMoney == 0 with no configured budget also falls back to the wallet.
	if (this->startingMoney == 0 && !hasConfiguredMoney)
		DeductSpent();
}

void DropshipLoadoutClass::SaveCargo()
{
	if (!HouseClass::CurrentPlayer())
		return;

	auto const pHouseExt = HouseExtContainer::Instance.Find(HouseClass::CurrentPlayer);

	if (!pHouseExt)
		return;

	auto const pGlobal = ScenarioExtData::Instance();

	// Flattens one bay's slot list, dropping the empty slots.
	auto const CompactBay = [this](size_t carrierIdx)
		{
			std::vector<TechnoTypeClass*> units;

			if (carrierIdx < dropshipBayChosenUnitsLists.size())
			{
				for (auto const pTechno : dropshipBayChosenUnitsLists[carrierIdx])
				{
					if (pTechno)
						units.push_back(pTechno);
				}
			}

			return units;
		};

	if (pSWTypeExt)
	{
		auto unitsList = CompactBay(0);
		pHouseExt->DropshipLoadout_SWCargo = unitsList;

		TechnoTypeClass* pCarrier = nullptr;

		if (pSWTypeExt->DropshipLoadout_Carrier.isset())
		{
			pCarrier = pSWTypeExt->DropshipLoadout_Carrier.Fetch();
		}
		else
		{
			auto const carriers = GatherCarriers();

			if (!carriers.empty())
				pCarrier = carriers[0];
		}

		pHouseExt->DropshipLoadout_SWCarrier = pCarrier;

		pHouseExt->DropshipLoadout_SWInitialUnits =
			RetainInitialUnitsPresentInCargo(pHouseExt->DropshipLoadout_SWInitialUnits, std::move(unitsList));

		SettleMoney(this->bAddUnusedMoneyToPlayer.Get(false), pSWTypeExt->DropshipLoadout_Money.isset());
		return;
	}

	pHouseExt->DropshipLoadout_Cargo.clear();
	pHouseExt->DropshipLoadout_Carriers.clear();

	auto const carriers = GatherCarriers();
	int const nCarriers = static_cast<int>(carriers.size());

	for (int i = 0; i < nStartingDropships && i < nCarriers; ++i)
	{
		pHouseExt->DropshipLoadout_Carriers.push_back(carriers[i]);

		if (i >= static_cast<int>(dropshipBayChosenUnitsLists.size()))
			continue;

		// SUSPECT: the `continue` above skips the Cargo push_back but NOT the
		// Carriers push_back, so the two vectors can end up with different
		// lengths and index i no longer refers to the same bay in both. Only
		// reachable if CreateControls() built fewer bays than nStartingDropships,
		// but the desync would be silent.
		pHouseExt->DropshipLoadout_Cargo.push_back(CompactBay(static_cast<size_t>(i)));
	}

	for (size_t i = 0; i < pHouseExt->DropshipLoadout_InitialUnits.size() && i < static_cast<size_t>(nStartingDropships); ++i)
	{
		std::vector<TechnoTypeClass*> cargoCopy;

		if (i < pHouseExt->DropshipLoadout_Cargo.size())
			cargoCopy = pHouseExt->DropshipLoadout_Cargo[i];

		pHouseExt->DropshipLoadout_InitialUnits[i] =
			RetainInitialUnitsPresentInCargo(pHouseExt->DropshipLoadout_InitialUnits[i], std::move(cargoCopy));
	}

	bool addUnusedMoneyToPlayer = false;

	if (this->bAddUnusedMoneyToPlayer.isset())
		addUnusedMoneyToPlayer = this->bAddUnusedMoneyToPlayer.Fetch();
	else if (pHouseTypeExt->DropshipLoadout_AddUnusedMoneyToPlayer.isset())
		addUnusedMoneyToPlayer = pHouseTypeExt->DropshipLoadout_AddUnusedMoneyToPlayer.Fetch();
	else if (pGlobal)
		addUnusedMoneyToPlayer = pGlobal->DropshipLoadout_AddUnusedMoneyToPlayer;

	long configuredMoney = -1;

	if (pHouseTypeExt->DropshipLoadout_Money.isset())
		configuredMoney = static_cast<long>(pHouseTypeExt->DropshipLoadout_Money.Fetch());
	else if (pGlobal)
		configuredMoney = static_cast<long>(pGlobal->DropshipLoadout_Money);

	SettleMoney(addUnusedMoneyToPlayer, configuredMoney >= 0);
}

ASMJIT_PATCH(0x683D89, Dropship_Loadout_Remake, 0x6)
{
	enum { EndFunction = 0x683D9C };

	if (!HouseClass::CurrentPlayer() || !ScenarioClass::Instance())
		return EndFunction;

	auto const pHouseTypeExt = HouseTypeExtContainer::Instance.Find(HouseClass::CurrentPlayer->Type);

	if (!pHouseTypeExt)
		return EndFunction;

	int nStartingDropships = pHouseTypeExt->DropshipLoadout_StartingDropships.isset() ? pHouseTypeExt->DropshipLoadout_StartingDropships.Fetch() :
		ScenarioExtData::Instance()->DropshipLoadout_StartingDropships;

	if (nStartingDropships <= 0)
		return EndFunction;

	DropshipLoadoutClass loadout;

	if (loadout.Initialize())
		loadout.Run();

	return EndFunction;
}

void DropshipLoadoutClass::OpenInGameWindow(bool IgnoreFixedUnits, bool PreloadCargo, int AllowableUnitsIndex, int StartingMoney, Nullable<bool> AddUnusedMoneyToPlayer, Nullable<bool> RememberPurchasedCargo, SuperWeaponTypeClass* SWType)
{
	if (!ScenarioClass::Instance())
		return;

	ScenarioClass::Instance->PauseGame();

	const bool oldLocked = ScenarioClass::Instance->UserInputLocked;
	const bool oldPaused = ScenarioClass::Instance->IsGamePaused;

	ScenarioClass::Instance->UserInputLocked = false;
	ScenarioClass::Instance->IsGamePaused = false;

	DropshipLoadoutClass loadout;

	if (loadout.Initialize(IgnoreFixedUnits, PreloadCargo, AllowableUnitsIndex, StartingMoney, AddUnusedMoneyToPlayer, RememberPurchasedCargo, SWType))
		loadout.Run();

	ScenarioClass::Instance->IsGamePaused = oldPaused;
	ScenarioClass::Instance->UserInputLocked = oldLocked;

	ScenarioClass::Instance->ResumeGame();
}