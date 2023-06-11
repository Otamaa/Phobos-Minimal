#include "TechnoTypeConvertData.h"

#include <Misc/AresData.h>
#include <Utilities/Debug.h>

void TechnoTypeConvertData::ApplyConvert(const std::vector<TechnoTypeConvertData>& nPairs , HouseClass * pHouse, TechnoClass * pTarget)
{
	if (nPairs.empty())
		return;

	const auto pCurType = pTarget->GetTechnoType();

	for (auto const& [pFrom, pTo, nAffect] : nPairs)
	{
		if (!pFrom || !pTo || pFrom == pTo)
			continue;

		if (!EnumFunctions::CanTargetHouse(nAffect, pHouse, pTarget->Owner))
			continue;

		if (pFrom == pCurType)
		{
			if (!AresData::ConvertTypeTo(pTarget, pTo))
				Debug::Log("WarheadTypeExt::ExtData::ApplyUpgrade Failed to ConvertType ! \n");
		}
	}
}