
#include "Body.h"
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Utilities/Macro.h>
#include <SlaveManagerClass.h>

DEFINE_HOOK(0x4721E6, CaptureManagerClass_DrawLinkToVictim, 0x6) //C
{
	GET(CaptureManagerClass*, pThis, EDI);
	GET(TechnoClass*, pVictim, ECX);
	GET_STACK(int, nNodeCount, STACK_OFFS(0x30, 0x1C));

	const auto pAttacker = pThis->Owner;
	const auto pAttackerType = pAttacker->GetTechnoType();
	if (CaptureExt::AllowDrawLink(pAttackerType)) {
		auto nVictimCoord = pVictim->Location;
		nVictimCoord.Z += pAttackerType->LeptonMindControlOffset;
		CoordStruct nFLH ;
		pAttacker->GetFLH(&nFLH ,-1 - nNodeCount % 5, CoordStruct::Empty);
		Drawing::DrawLinesTo(nFLH, nVictimCoord, pAttacker->Owner->Color);
	}

	R->EBP(nNodeCount);
	return 0x472287;
}

DEFINE_HOOK(0x471D40, CaptureManagerClass_CaptureUnit_ReplaceVanillaFunc, 0x7)
{
	GET(CaptureManagerClass*, pThis, ECX);
	GET_STACK(AbstractClass*, pTechno, 0x4);

	R->AL(CaptureExt::CaptureUnit(pThis, generic_cast<TechnoClass*>(pTechno)));

	return 0x471D5A;
}

DEFINE_HOOK(0x471FF0, CaptureManagerClass_FreeUnit, 0x8)
{
	GET(CaptureManagerClass*, pThis, ECX);
	GET_STACK(TechnoClass*, pTechno, 0x4);

	R->AL(CaptureExt::FreeUnit(pThis, pTechno));

	return 0x472006;
}

// DEFINE_HOOK(0x6FCB34, TechnoClass_CanFire_CanCapture, 0x6)
// {
// 	GET(TechnoClass*, pThis, ESI);
// 	GET(TechnoClass*, pTarget, EBP);
//
// 	return pThis->CaptureManager->CanCapture(pTarget) ?
// 	 0x6FCB53  : 0x6FCB44 ;
// }