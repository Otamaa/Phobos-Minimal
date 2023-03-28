
#include "Body.h"
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Utilities/Macro.h>
#include <SlaveManagerClass.h>

#include <Commands/ShowTeamLeader.h>

DEFINE_HOOK(0x6D47A6, TacticalClass_Render_Techno, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	if (auto const pOwner = pThis->SlaveOwner) {

		if (!pOwner->IsSelected || pThis->InLimbo)
			return 0x0;

		Drawing::DrawLinesTo(pOwner->GetRenderCoords(), pThis->Location, pOwner->Owner->Color);
	}

	if (auto const pOwner = pThis->SpawnOwner) {

		if (!pOwner->IsSelected || pThis->InLimbo)
			return 0x0;

		Drawing::DrawLinesTo(pOwner->GetRenderCoords(), pThis->Location, pOwner->Owner->Color);
	}

	if(ShowTeamLeaderCommandClass::IsActivated()) {
		if (auto const pFoot = generic_cast<FootClass*>(pThis)) {
			if (auto pTeam = pFoot->Team) {
				if (auto const pTeamLeader = pTeam->FetchLeader()) {

					if(pTeamLeader != pFoot)
						Drawing::DrawLinesTo(pTeamLeader->GetRenderCoords(), pThis->Location, pTeamLeader->Owner->Color);
				}
			}
		}
	}

	return 0x0;
}


DEFINE_HOOK(0x6F5190, TechnoClass_DrawIt_Add, 0x6)
{
	GET(TechnoClass*, pThis, ECX);
	GET_STACK(Point2D*, pLocation, 0x4);
	GET_STACK(RectangleStruct*, pBound, 0x8);

	auto DrawTheStuff = [&pLocation, &pThis, &pBound](const wchar_t* pFormat) {
		auto nPoint = *pLocation;
		//DrawingPart
		RectangleStruct nTextDimension;
		Drawing::GetTextDimensions(&nTextDimension, pFormat, nPoint, TextPrintType::Center | TextPrintType::FullShadow | TextPrintType::Efnt, 4, 2);
		auto nIntersect = Drawing::Intersect(nTextDimension, *pBound);
		auto nColorInt = pThis->Owner->Color.ToInit();//0x63DAD0

		DSurface::Temp->Fill_Rect(nIntersect, (COLORREF)0);
		DSurface::Temp->Draw_Rect(nIntersect, (COLORREF)nColorInt);
		Point2D nRet;
		Simple_Text_Print_Wide(&nRet, pFormat, DSurface::Temp.get(), pBound, &nPoint, (COLORREF)nColorInt, (COLORREF)0, TextPrintType::Center | TextPrintType::FullShadow | TextPrintType::Efnt, true);
	};

	if (ShowTeamLeaderCommandClass::IsActivated()) {
		if (auto const pFoot = generic_cast<FootClass*>(pThis)) {
			if (auto pTeam = pFoot->Team) {
				if (auto const pTeamLeader = pTeam->FetchLeader()) {
					if (pTeamLeader == pThis) {
						DrawTheStuff(L"Team Leader");
					}
				}
			}
		}
	}

	return 0x0;
}

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
		const auto nFLH = pAttacker->GetFLH(-1 - nNodeCount % 5, CoordStruct::Empty);
		Drawing::DrawLinesTo(nFLH, nVictimCoord, pAttacker->Owner->Color);
	}

	R->EBP(nNodeCount);
	return 0x472287;
}

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

DEFINE_HOOK(0x4DBF23, FootClass_ChangeOwner_IAmNowHuman, 0x6)
{
	GET(FootClass* const, pThis, ESI);
	// This is not limited to mind control, could possibly affect many map triggers
	// This is still not even correct, but let's see how far this can help us

	pThis->ShouldScanForTarget = false;
	pThis->ShouldEnterAbsorber = false;
	pThis->ShouldEnterOccupiable = false;
	pThis->ShouldGarrisonStructure = false;
	pThis->CurrentTargets.Clear();

	if (pThis->HasAnyLink() || pThis->GetTechnoType()->ResourceGatherer) // Don't want miners to stop
		return 0;

	switch (pThis->GetCurrentMission())
	{
	case Mission::Harvest:
	case Mission::Sleep:
	case Mission::Harmless:
	case Mission::Repair:
		return 0;
	}

	pThis->Override_Mission(Mission::Guard, nullptr, nullptr); // I don't even know what this is, just clear the target and destination for me
	pThis->ShouldLoseTargetNow = TRUE;
	pThis->QueueMission(pThis->GetTechnoType()->DefaultToGuardArea ? Mission::Area_Guard : Mission::Guard, true);

	return 0;
}