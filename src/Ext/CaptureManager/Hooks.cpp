
#include "Body.h"
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Utilities/Macro.h>

DEFINE_HOOK(0x4721E6, CaptureManagerClass_DrawLinkToVictim, 0xC)
{
	GET(CaptureManagerClass*, pThis, EDI);
	GET(TechnoClass*, pVictim, ECX);
	GET_STACK(int, nNodeCount, STACK_OFFS(0x30, 0x1C));

	const auto pAttacker = pThis->Owner;
	const auto pAttackerType = pAttacker->GetTechnoType();

	if (CaptureExt::AllowDrawLink(pAttackerType))
	{
		auto nVictimCoord = pVictim->Location;
		nVictimCoord.Z += pAttackerType->LeptonMindControlOffset;
		const auto nFLH = pAttacker->GetFLH(-1 - nNodeCount % 5, CoordStruct::Empty);
		CaptureExt::DrawLinkTo(nFLH, nVictimCoord, pAttacker->Owner->Color);
	}

	R->EBP(nNodeCount);
	return 0x472287;
}

DEFINE_JUMP(CALL,0x6FA730,GET_OFFSET(CaptureExt::Overload_AI));

DEFINE_HOOK(0x471D40, CaptureManagerClass_CaptureUnit, 0x7)
{
	GET(CaptureManagerClass*, pThis, ECX);
	GET_STACK(TechnoClass*, pTechno, 0x4);

	R->AL(CaptureExt::CaptureUnit(pThis, pTechno));

	return 0x471D5A;
}

DEFINE_HOOK(0x471FF0, CaptureManagerClass_FreeUnit, 0x8)
{
	GET(CaptureManagerClass*, pThis, ECX);
	GET_STACK(TechnoClass*, pTechno, 0x4);

	R->AL(CaptureExt::FreeUnit(pThis, pTechno));

	return 0x472006;
}

DEFINE_HOOK(0x6FCB34, TechnoClass_CanFire_CanCapture, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(TechnoClass*, pTarget, EBP);

	R->AL(CaptureExt::CanCapture(pThis->CaptureManager, pTarget));

	return 0x6FCB40;
}