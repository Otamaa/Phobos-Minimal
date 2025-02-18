#include "TechnoTypeConvertData.h"

#include <Misc/Ares/Hooks/Header.h>
#include <Utilities/Debug.h>
#include <AnimClass.h>

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
					Debug::LogInfo("Failed to ConvertType From[{}] To [{}]!", pFrm->ID, pTo->ID);
				else
				{
					if (SucceededAnim)
					{
						auto pDrawer = pTarget;
						if (pTarget->InLimbo && pTarget->Transporter)
							pDrawer = pTarget->Transporter;
						else if(pTarget->InLimbo && !pTarget->Transporter)
							pDrawer = nullptr;

						if(pDrawer){
							auto pAnim = GameCreate<AnimClass>(SucceededAnim, pDrawer->Location);
							pAnim->SetOwnerObject(pDrawer);
						}
					}
				}
			}
		} else {
			const auto bConvertStatus = TechnoExt_ExtData::ConvertToType(pTarget, pTo);

			if (!bConvertStatus)
				Debug::LogInfo("Failed to ConvertType From[{}] To [{}]!", pCurType->ID, pTo->ID);
			else
			{
				if (SucceededAnim)
				{
					auto pDrawer = pTarget;
					if (pTarget->InLimbo && pTarget->Transporter)
						pDrawer = pTarget->Transporter;
					else if (pTarget->InLimbo && !pTarget->Transporter)
						pDrawer = nullptr;

					if (pDrawer)
					{
						auto pAnim = GameCreate<AnimClass>(SucceededAnim, pDrawer->Location);
						pAnim->SetOwnerObject(pDrawer);
					}
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
				auto list_value = &list.emplace_back();
				char* contexthere = nullptr;

				if (nDelim2 != std::string::npos)
				{
					auto nSecondPair_1 = nSecondPair.substr(0, nDelim2);
					auto nSecondPair_2 = nSecondPair.substr(nDelim2 + 1);

					list_value->From.clear();

					for (auto pCur = strtok_s(nFirst.data(), Phobos::readDelims, &contexthere);
							pCur;
							pCur = strtok_s(nullptr, Phobos::readDelims, &contexthere)) {
						TechnoTypeClass* buffer = nullptr;

						if (Parser<TechnoTypeClass*>::Parse(pCur, &buffer) || GameStrings::IsBlank(pCur))
							list_value->From.push_back(buffer);
						else
							Debug::INIParseFailed(pSection, pKey, pCur, nullptr);
					}

					Parser<TechnoTypeClass*>::Parse(nSecondPair_1.c_str(), &list_value->To);
					detail::getresult<AffectedHouse>(list_value->Eligible, nSecondPair_2, pSection, pKey, false);

					//Debug::LogInfo("parsing[%s]%s with 3 values [%s - %s - %s]", pSection , pKey , nFirst.c_str() , nSecondPair_1.c_str() , nSecondPair_2.c_str());
				}
				else
				{
					list_value->From.clear();

					for (auto pCur = strtok_s(nFirst.data(), Phobos::readDelims, &contexthere);
							pCur;
							pCur = strtok_s(nullptr, Phobos::readDelims, &contexthere))
					{
						TechnoTypeClass* buffer = nullptr;

						if (Parser<TechnoTypeClass*>::Parse(pCur, &buffer) || GameStrings::IsBlank(pCur))
							list_value->From.push_back(buffer);
						else
							Debug::INIParseFailed(pSection, pKey, pCur, nullptr);
					}

					Parser<TechnoTypeClass*>::Parse(nSecondPair.c_str(), &list_value->To);
				}
			}
		}
	}
	else
	{
		list.clear();

		for (size_t i = 0; ; ++i)
		{
			std::string base_("Convert");
			base_ += std::to_string(i);
			ValueableVector<TechnoTypeClass*> convertFrom {};
			Nullable<TechnoTypeClass*> convertTo {};
			Nullable<AffectedHouse> convertAffectedHouses {};
			convertTo.Read(exINI, pSection, (base_ + ".To").c_str());

			if (!convertTo.isset() || !convertTo.Get())
				break;

			convertFrom.Read(exINI, pSection, (base_ + ".From").c_str());
			convertAffectedHouses.Read(exINI, pSection, (base_ + ".AffectedHouses").c_str());

			auto& back = list.emplace_back();

			back.From = convertFrom;
			back.To = convertTo;

			back.Eligible = convertAffectedHouses.Get(AffectedHouse::All);
		}

		ValueableVector<TechnoTypeClass*> convertFrom {};
		Nullable<TechnoTypeClass*> convertTo {};
		Nullable<AffectedHouse> convertAffectedHouses {};

		convertFrom.Read(exINI, pSection, "Convert.From");
		convertTo.Read(exINI, pSection, "Convert.To");
		convertAffectedHouses.Read(exINI, pSection, "Convert.AffectedHouses");

		if (convertTo.isset() && convertTo.Get())
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