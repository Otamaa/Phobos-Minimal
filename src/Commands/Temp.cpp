for (auto pObj : ObjectClass::CurrentObjects)
{
	auto pFoot = abstract_cast<FootClass*>(pObj);
	if (pFoot && ... && pFoot->Owner->IsControlledByCurrentPlayer())
	{
		if (pFoot->IsMindControlled())
		{
			pFoot->EnterGrinder();
		}
		else if (auto pUnit = abstract_cast<UnitClass*>(pFoot))
		{
			if (pUnit->Type->Bunkerable)
				pFoot->EnterTankBunker();
		}
		else if (auto pInf = abstract_cast<InfantryClass*>(pFoot))
		{
			if (pInf->Type->Occupier)
				pFoot->GarrisonStructure();
		}
	}
}