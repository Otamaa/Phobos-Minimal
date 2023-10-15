/*
char buffer[0x30];
auto pINI = CCINIClass::INI_Rules();
INI_EX iniEX(pINI);

InfantryTypeClass::Array->for_each([&](InfantryTypeClass* pInfType) {
	WarheadTypeClass::Array->for_each([&](WarheadTypeClass* pWarhead) {
		if (auto const pExt = WarheadTypeExtContainer::Instance.TryFind(pWarhead))
		{
			Nullable<AnimTypeClass*> nBuffer {};
			IMPL_SNPRNINTF(buffer, sizeof(buffer), "%s.InfDeathAnim", pInfType->ID);
			nBuffer.Read(iniEX, pWarhead->ID, buffer);

			if (!nBuffer.isset() || !nBuffer.Get())
				return;

			//Debug::Log("Found specific InfDeathAnim for [WH : %s Inf : %s Anim %s]\n", pWarhead->ID, pInfType->ID, nBuffer->ID);
			pExt->InfDeathAnims[pInfType->ArrayIndex] = nBuffer;
		}
	});
});*/

//for (auto pType : *TechnoTypeClass::Array)
//{
//	if (auto pTypeExt = TechnoTypeExtContainer::Instance.TryFind(pType)){
//
//		char Temp_[0x80];
//		pTypeExt->SpecificExpFactor.clear();
//		for (int i = 0;; ++i)
//		{
//			Nullable<TechnoTypeClass*> Temp {};
//			IMPL_SNPRNINTF(Temp_, sizeof(Temp_), "SpecificExperience%d.Type", i);
//				Temp.Read(iniEX, pType->ID, Temp_);
//
//			if (!Temp)
//				break;
//
//			IMPL_SNPRNINTF(Temp_, sizeof(Temp_), "SpecificExperience%d.Factor", i);
//			pTypeExt->SpecificExpFactor[Temp.Get()].Read(iniEX, pType->ID, Temp_);
//		}
//	}
//}