DEFINE_HOOK(0x6F8721, TechnoClass_EvalObject_VHPScan, 0x7)
{
	GET(TechnoClass* const, pThis, EDI);
	GET(ObjectClass* const, pTarget, ESI);
	GET(int*, pRiskValue, EBP);

	const auto pExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
	const auto pTechnoTarget = generic_cast<TechnoClass* const>(pTarget);

	int nValue = pExt->VHPscan_Value;
	if (nValue <= 0)
		nValue = 2;

	switch (NewVHPScan(pExt->AttachedToObject->VHPScan))
	{
	case NewVHPScan::Normal:
	{
		if (pTarget->EstimatedHealth <= 0)
		{
			*pRiskValue /= nValue;
			break;
		}

		if (pTarget->EstimatedHealth > (pTarget->GetType()->Strength / 2))
			break;

		*pRiskValue *= nValue;
	}
	break;
	case NewVHPScan::Threat:
	{
		*pRiskValue *= (*pRiskValue / nValue);
		break;
	}
	break;
	case NewVHPScan::Health:
	{
		int nRes = *pRiskValue;
		if (pTarget->EstimatedHealth > pTarget->GetType()->Strength / 2)
			nRes = nValue * nRes;
		else
			nRes = nRes / nValue;

		*pRiskValue = nRes;
	}
	break;
	case NewVHPScan::Damage:
	{
		if (!pTechnoTarget)
		{
			*pRiskValue = 0;
			break;
		}

		*pRiskValue = pTechnoTarget->CombatDamage(-1) / nValue * (*pRiskValue);
	}
	break;
	case NewVHPScan::Value:
	{
		if (!pTechnoTarget)
		{
			*pRiskValue = 0;
			break;
		}

		const int nSelectedWeapon = pTechnoTarget->SelectWeapon(pThis);
		const auto nFireError = pTechnoTarget->GetFireError(pThis, nSelectedWeapon, 0);
		if (nFireError == FireError::NONE ||
			nFireError == FireError::FACING ||
			nFireError == FireError::REARM ||
			nFireError == FireError::ROTATING
			)
		{
			*pRiskValue *= nValue;
			break;
		}

		*pRiskValue /= nValue;
	}
	break;
	case NewVHPScan::Non_Infantry:
	{
		if (!pTechnoTarget || pTechnoTarget->WhatAmI() == InfantryClass::AbsID)
		{
			*pRiskValue = 0;
		}
	}
	break;
	default:
		break;
	}

	return 0x6F875F;
}