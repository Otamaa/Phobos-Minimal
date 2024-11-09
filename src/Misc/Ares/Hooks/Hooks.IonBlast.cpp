#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/Infantry/Body.h>

#include <IonBlastClass.h>

DEFINE_HOOK(0x53CB91, IonBlastClass_DTOR, 6)
{
	GET(IonBlastClass*, IB, ECX);
	WarheadTypeExtData::IonBlastExt.erase(IB);
	return 0;
}

DEFINE_HOOK(0x53CC0D, IonBlastClass_Update_DTOR, 5)
{
	GET(IonBlastClass*, IB, EBX);
	WarheadTypeExtData::IonBlastExt.erase(IB);
	return 0;
}

DEFINE_HOOK(0x53CBF5, IonBlastClass_Update_Duration, 5)
{
	GET(IonBlastClass*, IB, EBX);

	int Ripple_Radius = 79;
	if (auto pData = WarheadTypeExtData::IonBlastExt.get_or_default(IB)) {
		Ripple_Radius = MinImpl(Ripple_Radius, pData->Ripple_Radius + 1);
	}

	return IB->Lifetime < Ripple_Radius ?
		0x53CC3A : 0x53CBFA;
}

DEFINE_HOOK(0x53CC63, IonBlastClass_Update_Beam, 6)
{
	GET(IonBlastClass*, IB, EBX);

	if (const auto pWHExt = WarheadTypeExtData::IonBlastExt.get_or_default(IB))
	{
		auto nLoc = IB->Location;
		const auto pCell = MapClass::Instance->GetCellAt(nLoc);

		AnimTypeClass* pBlast = nullptr;
		if (pCell->LandType == LandType::Water)
			pBlast = RulesClass::Instance->SplashList[RulesClass::Instance->SplashList.Count - 1];
		else
			pBlast = pWHExt->Ion_Blast.Get(RulesClass::Instance->IonBlast);

		auto animCoord = nLoc;
		animCoord.Z += 5;

		if (pBlast) {
			GameCreate<AnimClass>(pBlast, animCoord);
		}

		if (auto pBeam = pWHExt->Ion_Beam.Get(RulesClass::Instance->IonBeam)) {
			GameCreate<AnimClass>(pBeam, animCoord);
		}

		if (auto pIonWH = pWHExt->Ion_WH.Get(RulesClass::Instance->IonCannonWarhead))
		{
			int nDamage = pWHExt->Ion_Damage.Get(RulesClass::Instance->IonCannonDamage);

			if (pCell->ContainsBridge())
			{
				auto BridgeCoord = nLoc;
				BridgeCoord.Z += Unsorted::BridgeHeight;

				MapClass::DamageArea(BridgeCoord, nDamage, nullptr, pIonWH, pIonWH->Tiberium, nullptr);
			}

			MapClass::DamageArea(nLoc, nDamage, nullptr, pIonWH, pIonWH->Tiberium, nullptr);
			MapClass::FlashbangWarheadAt(nDamage, pIonWH, nLoc);
		}

		return pWHExt->Ion_Rocking ? 0x53CE0A : 0x53D302;
	}

	return 0;
}