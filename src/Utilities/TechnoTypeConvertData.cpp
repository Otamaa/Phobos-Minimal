#include "TechnoTypeConvertData.h"

#include <Misc/Ares/Hooks/Header.h>
#include <Utilities/Debug.h>

void TechnoTypeConvertData::ApplyConvert(const std::vector<TechnoTypeConvertData>& nPairs , HouseClass * pHouse, TechnoClass * pTarget, AnimTypeClass* SucceededAnim)
{
	if (nPairs.empty())
		return;

	const auto pCurType = pTarget->GetTechnoType();

	for (auto const& [pFrom, pTo, nAffect] : nPairs)
	{
		if (!pTo)
			continue;

		if (pHouse && !EnumFunctions::CanTargetHouse(nAffect, pHouse, pTarget->Owner))
			continue;

		if (!pFrom.empty())
		{
			for (auto* pFrm : pFrom) {

				if (pFrm != pCurType)
					continue;

				const auto bConvertStatus = TechnoExt_ExtData::ConvertToType(pTarget, pTo);

				if (!bConvertStatus)
					Debug::Log("Failed to ConvertType From[%x] To [%s]!\n", pFrm->ID, pTo->ID);
				else
				{
					if (SucceededAnim)
					{
						auto pAnim = GameCreate<AnimClass>(SucceededAnim, pTarget->Location);
						pAnim->SetOwnerObject(pTarget);
					}
				}
			}
		} else {
			const auto bConvertStatus = TechnoExt_ExtData::ConvertToType(pTarget, pTo);

			if (!bConvertStatus)
				Debug::Log("Failed to ConvertType From[%x] To [%s]!\n", pCurType->ID, pTo->ID);
			else
			{
				if (SucceededAnim)
				{
					auto pAnim = GameCreate<AnimClass>(SucceededAnim, pTarget->Location);
					pAnim->SetOwnerObject(pTarget);
				}
			}
		}
	}
}

void TechnoTypeConvertData::Parse(bool useDevelopversion, std::vector<TechnoTypeConvertData>& list, INI_EX& exINI, const char* pSection, const char* pKey)
{
	if (!useDevelopversion)
	{
		if (exINI.ReadString(pSection, pKey))
		{
			list.clear();
			char* context = nullptr;
			for (auto cur = strtok_s(exINI.value(), Phobos::readDelims, &context);
				cur;
				cur = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				std::string copy = cur;
				std::erase(copy, ' ');

				const auto nDelim = copy.find(":");
				if (nDelim == std::string::npos)
					continue;

				auto nFirst = copy.substr(0, nDelim);
				//second countais b:c
				auto nSecondPair = copy.substr(nDelim + 1);
				const auto nDelim2 = nSecondPair.find(":");
				auto value = list.emplace_back();

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
						else if (!GameStrings::IsBlank(pCur))
							Debug::INIParseFailed(pSection, pKey, pCur, nullptr);
					}

					Parser<TechnoTypeClass*>::Parse(nSecondPair_1.c_str(), &value.To);
					detail::getresult<AffectedHouse>(value.Eligible, nSecondPair_2, pSection, pKey, false);

					//Debug::Log("parsing[%s]%s with 3 values [%s - %s - %s]\n", pSection , pKey , nFirst.c_str() , nSecondPair_1.c_str() , nSecondPair_2.c_str());
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
						else if (!GameStrings::IsBlank(pCur))
							Debug::INIParseFailed(pSection, pKey, pCur, nullptr);
					}

					Parser<TechnoTypeClass*>::Parse(nSecondPair.c_str(), &value.To);
				}
			}
		}
	}
	{
		for (size_t i = 0; ; ++i)
		{
			std::string base_("Convert");
			base_ += std::to_string(i);
			ValueableVector<TechnoTypeClass*> convertFrom;
			Nullable<TechnoTypeClass*> convertTo;
			Nullable<AffectedHouse> convertAffectedHouses;
			convertFrom.Read(exINI, pSection, (base_+ ".From").c_str());
			convertTo.Read(exINI, pSection, (base_ + ".To").c_str());
			convertAffectedHouses.Read(exINI, pSection, (base_ + ".AffectedHouses").c_str());

			if (!convertTo.isset())
				break;

			if (!convertAffectedHouses.isset())
				convertAffectedHouses = AffectedHouse::All;

			list.emplace_back(convertFrom, convertTo, convertAffectedHouses);
		}
		ValueableVector<TechnoTypeClass*> convertFrom;
		Nullable<TechnoTypeClass*> convertTo;
		Nullable<AffectedHouse> convertAffectedHouses;
		convertFrom.Read(exINI, pSection, "Convert.From");
		convertTo.Read(exINI, pSection, "Convert.To");
		convertAffectedHouses.Read(exINI, pSection, "Convert.AffectedHouses");
		if (convertTo.isset())
		{
			if (!convertAffectedHouses.isset())
				convertAffectedHouses = AffectedHouse::All;

			if (list.size())
				list[0] = { convertFrom, convertTo, convertAffectedHouses };
			else
				list.emplace_back(convertFrom, convertTo, convertAffectedHouses);
		}
	}
}
