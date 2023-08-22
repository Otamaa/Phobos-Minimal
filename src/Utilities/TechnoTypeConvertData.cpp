#include "TechnoTypeConvertData.h"

#include <Misc/AresData.h>
#include <Utilities/Debug.h>

void TechnoTypeConvertData::ApplyConvert(const std::vector<TechnoTypeConvertData>& nPairs , HouseClass * pHouse, TechnoClass * pTarget, AnimTypeClass* SucceededAnim)
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
			const auto bConvertStatus = AresData::ConvertTypeTo(pTarget, pTo);

			if (!bConvertStatus)
				Debug::Log("Failed to ConvertType From[%x] To [%s]!\n" , pFrom->ID, pTo->ID);
			else
			{
				if (SucceededAnim)
					if (auto pAnim = GameCreate<AnimClass>(SucceededAnim, pTarget->Location))
						pAnim->SetOwnerObject(pTarget);
			}
		}
	}
}