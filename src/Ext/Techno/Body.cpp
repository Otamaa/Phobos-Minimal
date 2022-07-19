#include "Body.h"

#include <BuildingClass.h>
#include <HouseClass.h>
#include <BulletClass.h>
#include <BulletTypeClass.h>
#include <ScenarioClass.h>
#include <SpawnManagerClass.h>
#include <SlaveManagerClass.h>
#include <InfantryClass.h>
#include <Unsorted.h>

#include <BitFont.h>

#include <New/Entity/FlyingStrings.h>

#include <Ext/Anim/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/House/Body.h>

#include <JumpjetLocomotionClass.h>

#include <Utilities/EnumFunctions.h>
#include <Utilities/Cast.h>

#include <Phobos_ECS.h>

#ifdef COMPILE_PORTED_DP_FEATURES
#include <Misc/DynamicPatcher/Trails/TrailsManager.h>
#endif

#include <memory>

template<> const DWORD Extension<TechnoClass>::Canary = 0x55555555;

TechnoExt::ExtContainer TechnoExt::ExtMap;

TechnoExt::ExtData* TechnoExt::GetExtData(TechnoExt::base_type* pThis)
{
	return  ExtMap.Find(pThis);
}

int DrawHealthBar_Pip(TechnoClass* pThis, bool isBuilding, const Point3D& Pip)
{
	const auto strength = pThis->GetTechnoType()->Strength;

	if (pThis->Health > RulesClass::Instance->ConditionYellow * strength && Pip.X != -1)
		return Pip.X;
	else if (pThis->Health > RulesClass::Instance->ConditionRed * strength && (Pip.Y != -1 || Pip.X != -1))
		return Pip.Y == -1 ? Pip.X : Pip.Y;
	else if (Pip.Z != -1 || Pip.X != -1)
		return Pip.Z == -1 ? Pip.X : Pip.Z;

	return isBuilding ? 5 : 16;
}

int DrawHealthBar_PipAmount(TechnoClass* pThis, int iLength)
{
	return pThis->Health > 0
		? Math::clamp(static_cast<int>(round(pThis->GetHealthPercentage() * iLength)), 0, iLength)
		: 0;
}

void DrawGroupID_Building(TechnoClass* pThis, Point2D* pLocation, const Point2D& GroupID_Offs)
{
	auto const pHouse = pThis->GetOwningHouse() ? pThis->GetOwningHouse(): HouseExt::FindCivilianSide();

	CoordStruct vCoords = { 0, 0, 0 };
	pThis->GetTechnoType()->Dimension2(&vCoords);

	CoordStruct vCoords2 = { -vCoords.X / 2, vCoords.Y / 2,vCoords.Z };

	Point2D vPos = { 0, 0 };
	TacticalClass::Instance->CoordsToScreen(&vPos, &vCoords2);

	Point2D vLoc = *pLocation;
	vLoc.X += vPos.X;
	vLoc.Y += vPos.Y;

	Point2D vOffset = GroupID_Offs;

	vLoc.X += vOffset.X;
	vLoc.Y += vOffset.Y;

	if (pThis->Group >= 0)
	{
		const COLORREF GroupIDColor = Drawing::RGB2DWORD(pHouse->Color);

		RectangleStruct rect
		{
			vLoc.X - 7,
			vLoc.Y + 26,
			12,13
		};

		DSurface::Temp->Fill_Rect(rect, COLOR_BLACK);
		DSurface::Temp->Draw_Rect(rect, GroupIDColor);

		const int groupid = (pThis->Group == 9) ? 0 : (pThis->Group + 1);

		wchar_t GroupID[0x20];
		swprintf_s(GroupID, L"%d", groupid);

		Point2D vGroupPos
		{
			vLoc.X - 4,
			vLoc.Y + 25
		};

		rect = DSurface::Temp->Get_Rect_WithoutBottomBar();
		Point2D nTemp { };
		Fancy_Text_Print_Wide(&nTemp, GroupID, DSurface::Temp(), &rect, &vGroupPos, GroupIDColor, 0, TextPrintType::NoShadow);
	}
}

void DrawGroupID_Other(TechnoClass* pThis,Point2D* pLocation, const Point2D& GroupID_Offs)
{
	auto const pHouse = pThis->GetOwningHouse() ? pThis->GetOwningHouse() : HouseExt::FindCivilianSide();

	Point2D vLoc = *pLocation;
	Point2D vOffset = GroupID_Offs;

	vLoc.X += vOffset.X;
	vLoc.Y += vOffset.Y;

	if (pThis->Group >= 0)
	{
		if (pThis->WhatAmI() == AbstractType::Infantry)
		{
			vLoc.X -= 20;
			vLoc.Y -= 25;
		}
		else
		{
			vLoc.X -= 30;
			vLoc.Y -= 23;
		}

		const COLORREF GroupIDColor = Drawing::RGB2DWORD(pHouse->Color);

		RectangleStruct rect
		{
			vLoc.X,
			vLoc.Y,
			12,13
		};

		DSurface::Temp->Fill_Rect(rect, COLOR_BLACK);
		DSurface::Temp->Draw_Rect(rect, GroupIDColor);

		const int groupid = (pThis->Group == 9) ? 0 : (pThis->Group + 1);

		wchar_t GroupID[0x20];
		swprintf_s(GroupID, L"%d", groupid);

		rect = DSurface::Temp->Get_Rect_WithoutBottomBar();

		Point2D vGroupPos
		{
			vLoc.X + 2,
			vLoc.Y - 1
		};

		Point2D nTemp { };
		Fancy_Text_Print_Wide(&nTemp, GroupID, DSurface::Temp(), &rect, &vGroupPos, GroupIDColor, 0, TextPrintType::NoShadow);

	}
}

void DrawHealthBar_Building(TechnoClass* pThis, int iLength, Point2D* pLocation, RectangleStruct* pBound , ConvertClass* PipsPAL, SHPStruct* PipsSHP , const Point3D& pip)
{
	if (PipsSHP == nullptr) return;
	if (PipsPAL == nullptr) return;

	CoordStruct vCoords = { 0, 0, 0 };
	pThis->GetTechnoType()->Dimension2(&vCoords);
	Point2D vPos2 = { 0, 0 };
	CoordStruct vCoords2 = { -vCoords.X / 2, vCoords.Y / 2,vCoords.Z };
	TacticalClass::Instance->CoordsToScreen(&vPos2, &vCoords2);

	Point2D vLoc = *pLocation;
	vLoc.Y += 1;

	Point2D vPos = { 0, 0 };


	const int iTotal = DrawHealthBar_PipAmount(pThis, iLength);
	int frame = DrawHealthBar_Pip(pThis, true , pip);

	if (iTotal > 0)
	{
		int frameIdx, deltaX, deltaY;
		for (frameIdx = iTotal, deltaX = 0, deltaY = 0;
			frameIdx;
			frameIdx--, deltaX += 4, deltaY -= 2)
		{
			vPos.X = vPos2.X + vLoc.X + 4 * iLength + 3 - deltaX;
			vPos.Y = vPos2.Y + vLoc.Y - 2 * iLength + 4 - deltaY;

			DSurface::Composite->DrawSHP(PipsPAL, PipsSHP,
				frame, &vPos, pBound, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
		}
	}

	if (iTotal < iLength)
	{
		int frameIdx, deltaX, deltaY;
		for (frameIdx = iLength - iTotal, deltaX = 4 * iTotal, deltaY = -2 * iTotal;
			frameIdx;
			frameIdx--, deltaX += 4, deltaY -= 2)
		{
			vPos.X = vPos2.X + vLoc.X + 4 * iLength + 3 - deltaX;
			vPos.Y = vPos2.Y + vLoc.Y - 2 * iLength + 4 - deltaY;

			DSurface::Composite->DrawSHP(PipsPAL, PipsSHP,
				0, &vPos, pBound, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
		}
	}

}

void DrawHealthBar_Other(TechnoClass* pThis, int iLength, Point2D* pLocation, RectangleStruct* pBound, ConvertClass* PipsPAL, SHPStruct* PipSHP, SHPStruct* PipBrdSHP, ConvertClass* PipBrdPAL, const Point3D& pip,const Point2D& DrawOffs , int nXOffset)
{
	if (PipSHP == nullptr) return;
	if (PipsPAL == nullptr) return;
	if (PipBrdSHP == nullptr) return;
	if (PipBrdPAL == nullptr) return;

	Point2D vPos = { 0,0 };
	Point2D vLoc = *pLocation;

	int frame, XOffset, YOffset;
	YOffset = pThis->GetTechnoType()->PixelSelectionBracketDelta;
	vLoc.Y -= 5;

	vLoc.X += nXOffset;

	if (iLength == 8)
	{
		vPos.X = vLoc.X + 11;
		vPos.Y = vLoc.Y - 20 + YOffset;
		frame = 1;
		XOffset = -5;
		YOffset -= 19;
	}
	else
	{
		vPos.X = vLoc.X + 1;
		vPos.Y = vLoc.Y - 21 + YOffset;
		frame = 0;
		XOffset = -15;
		YOffset -= 20;
	}

	if (pThis->IsSelected) {
		DSurface::Temp->DrawSHP(PipBrdPAL, PipBrdSHP,frame, &vPos, pBound, BlitterFlags(0xE00), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}

	const int iTotal = DrawHealthBar_PipAmount(pThis, iLength);

	frame = DrawHealthBar_Pip(pThis, false , pip);

	Point2D DrawOffset = DrawOffs ? DrawOffs : Point2D {2,0};

	for (int i = 0; i < iTotal; ++i)
	{
		vPos.X = vLoc.X + XOffset + DrawOffset.X * i;
		vPos.Y = vLoc.Y + YOffset + DrawOffset.Y * i;

		DSurface::Temp->DrawSHP(PipsPAL, PipSHP,
			frame, &vPos, pBound, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}
}

void TechnoExt::SyncIronCurtainStatus(TechnoClass* pFrom, TechnoClass* pTo)
{
	if (pFrom->IsIronCurtained() && !pFrom->ForceShielded) {
		const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pFrom->GetTechnoType());
		if (pTypeExt->IronCurtain_SyncDeploysInto.Get(RulesExt::Global()->IronCurtain_SyncDeploysInto))
		{
			pTo->IronCurtain(pFrom->IronCurtainTimer.GetTimeLeft(), pFrom->Owner, false);
			pTo->IronTintStage = pFrom->IronTintStage;
		}
	}
}

void TechnoExt::ExtData::InitializeConstants()
{
	LaserTrails.reserve(2);
#ifdef COMPILE_PORTED_DP_FEATURES
	Trails.reserve(2);
#endif

}

void TechnoExt::PlayAnim(AnimTypeClass* const pAnim, TechnoClass* pInvoker)
{
	if (pAnim && pInvoker) {
		if (auto pCreated = GameCreate<AnimClass>(pAnim, pInvoker->Location)) {
			AnimExt::SetAnimOwnerHouseKind(pCreated, pInvoker->GetOwningHouse(), nullptr, pInvoker, false);
		}
	}
}

double TechnoExt::GetDamageMult(TechnoClass* pSouce, bool ForceDisable)
{
	if (!pSouce || ForceDisable || !pSouce->GetTechnoType())
		return 1.0;

	bool firepower = false;
	auto pTechnoType = pSouce->GetTechnoType();

	if (pSouce->Veterancy.IsElite())
	{
		firepower = pTechnoType->VeteranAbilities.FIREPOWER || pTechnoType->EliteAbilities.FIREPOWER;
	}
	else if (pSouce->Veterancy.IsVeteran())
	{
		firepower = pTechnoType->VeteranAbilities.FIREPOWER;
	}

	return (!firepower ? 1.0 : RulesGlobal->VeteranCombat) * pSouce->FirepowerMultiplier * ((!pSouce->Owner || !pSouce->Owner->Type) ? 1.0 : pSouce->Owner->Type->FirepowerMult);
}

std::pair<bool, CoordStruct> TechnoExt::GetBurstFLH(TechnoClass* pThis, int weaponIndex)
{
	auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	bool FLHFound = false;
	CoordStruct FLH = CoordStruct::Empty;

	if (!pThis || weaponIndex < 0 || !pExt)
		goto Return;

	const auto pInf = abstract_cast<InfantryClass*>(pThis);
	auto& pickedFLHs = pExt->WeaponBurstFLHs;

	if (pThis->Veterancy.IsElite())
	{
		if (pInf && pInf->IsDeployed() && !pExt->EliteDeployedWeaponBurstFLHs.empty())
			pickedFLHs = pExt->EliteDeployedWeaponBurstFLHs;
		else if (pInf && pInf->Crawling && !pExt->EliteCrouchedWeaponBurstFLHs.empty())
			pickedFLHs = pExt->EliteCrouchedWeaponBurstFLHs;
		else
			pickedFLHs = pExt->EliteWeaponBurstFLHs;
	}
	else
	{
		if (pInf && pInf->IsDeployed() && !pExt->DeployedWeaponBurstFLHs.empty())
			pickedFLHs = pExt->DeployedWeaponBurstFLHs;
		else if (pInf && pInf->Crawling && !pExt->CrouchedWeaponBurstFLHs.empty())
			pickedFLHs = pExt->CrouchedWeaponBurstFLHs;
	}

	if (!pickedFLHs.empty())
	{
		if (pickedFLHs[weaponIndex].Count > pThis->CurrentBurstIndex)
		{
			FLHFound = true;
			FLH = pickedFLHs[weaponIndex][pThis->CurrentBurstIndex];
		}
	}

	Return:
	return { FLHFound , FLH };
}

std::pair<bool, CoordStruct> TechnoExt::GetInfantryFLH(InfantryClass* pThis, int weaponIndex)
{
	bool FLHFound = false;
	CoordStruct FLH = CoordStruct::Empty;

	if (!pThis || weaponIndex < 0)
		goto Return;

	if (auto pTechnoType = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
	{
		Nullable<CoordStruct> pickedFLH;

		if (pThis->IsDeployed())
		{
			if (weaponIndex == 0)
				pickedFLH = pThis->Veterancy.IsElite() ? pTechnoType->E_DeployedPrimaryFireFLH : pTechnoType->DeployedPrimaryFireFLH;
			else if (weaponIndex == 1)
				pickedFLH = pThis->Veterancy.IsElite() ? pTechnoType->E_DeployedSecondaryFireFLH : pTechnoType->DeployedSecondaryFireFLH;
		}
		else
		{
			if (pThis->Crawling)
			{
				if (weaponIndex == 0) {
					pickedFLH = pThis->Veterancy.IsElite() ?
						pTechnoType->E_PronePrimaryFireFLH
#ifdef COMPILE_PORTED_DP_FEATURES
						.isset() ?
						pTechnoType->E_PronePrimaryFireFLH:
						pTechnoType->Elite_PrimaryCrawlFLH
#endif
						: pTechnoType->PronePrimaryFireFLH
#ifdef COMPILE_PORTED_DP_FEATURES
						.isset() ?
						pTechnoType->PronePrimaryFireFLH:
						pTechnoType->PrimaryCrawlFLH
#endif
						;
				}

				else if (weaponIndex == 1) {
					pickedFLH = pThis->Veterancy.IsElite() ?

					pTechnoType->E_ProneSecondaryFireFLH
#ifdef COMPILE_PORTED_DP_FEATURES
					.isset() ?
					pTechnoType->E_ProneSecondaryFireFLH : pTechnoType->E_ProneSecondaryFireFLH
#endif
					: pTechnoType->ProneSecondaryFireFLH
#ifdef COMPILE_PORTED_DP_FEATURES
					.isset() ? pTechnoType->ProneSecondaryFireFLH
					:pTechnoType->SecondaryCrawlFLH
#endif
					;
				}
			}
		}

		if (pickedFLH.isset() && pickedFLH.Get())
		{
			FLH = pickedFLH.Get();
			FLHFound = true;
		}
	}

	Return:
	return { FLHFound , FLH };
}

void TechnoExt::DrawSelectBrd(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt, int iLength, Point2D* pLocation, RectangleStruct* pBound, bool isInfantry , bool sIsDisguised)
{
	if (!pTypeExt->UseCustomSelectBrd.Get(RulesExt::Global()->UseSelectBrd.Get(Phobos::Config::EnableSelectBrd)))
		return;

	SHPStruct* GlbSelectBrdSHP = nullptr;

	if (isInfantry)
		GlbSelectBrdSHP = RulesExt::Global()->SHP_SelectBrdSHP_INF.Get();
	else
		GlbSelectBrdSHP = RulesExt::Global()->SHP_SelectBrdSHP_UNIT.Get();

	SHPStruct* SelectBrdSHP = pTypeExt->SHP_SelectBrdSHP.Get(GlbSelectBrdSHP);

	if (!SelectBrdSHP)
		return;

	ConvertClass* GlbSelectBrdPAL = nullptr;

	if (isInfantry)
		GlbSelectBrdPAL = RulesExt::Global()->SHP_SelectBrdPAL_INF.GetConvert();
	else
		GlbSelectBrdPAL = RulesExt::Global()->SHP_SelectBrdPAL_UNIT.GetConvert();

	ConvertClass* SelectBrdPAL = pTypeExt->SHP_SelectBrdPAL.GetOrDefaultConvert(GlbSelectBrdPAL);

	if (!SelectBrdPAL)
		return;

	Point2D vPos = { 0, 0 };
	Point2D vLoc = *pLocation;
	Point2D vOfs = { 0, 0 };
	int frame, XOffset, YOffset;

	Point3D glbSelectbrdFrame = isInfantry ?
		RulesExt::Global()->SelectBrd_Frame_Infantry.Get() :
		RulesExt::Global()->SelectBrd_Frame_Unit.Get();

	Point3D selectbrdFrame = pTypeExt->SelectBrd_Frame.Get(glbSelectbrdFrame);

	auto const nFlag = BlitterFlags::Centered | BlitterFlags::Nonzero | BlitterFlags::MultiPass | EnumFunctions::GetTranslucentLevel(pTypeExt->SelectBrd_TranslucentLevel.Get(RulesExt::Global()->SelectBrd_DefaultTranslucentLevel.Get()));
	auto const canSee = sIsDisguised && pThis->DisguisedAsHouse? pThis->DisguisedAsHouse->IsAlliedWith(HouseClass::Player) :
		pThis->Owner->IsAlliedWith(HouseClass::Player)
		|| HouseClass::IsPlayerObserver()
		|| pTypeExt->SelectBrd_ShowEnemy.Get(RulesExt::Global()->SelectBrd_DefaultShowEnemy.Get());

	vOfs = pTypeExt->SelectBrd_DrawOffset.Get(isInfantry ?
		RulesExt::Global()->SelectBrd_DrawOffset_Infantry.Get() : RulesExt::Global()->SelectBrd_DrawOffset_Unit.Get());

	XOffset = vOfs.X;
	YOffset = pTypeExt->OwnerObject()->PixelSelectionBracketDelta + vOfs.Y;
	vLoc.Y -= 5;

	if (iLength == 8)
	{
		vPos.X = vLoc.X + 1 + XOffset;
		vPos.Y = vLoc.Y + 6 + YOffset;
	}
	else
	{
		vPos.X = vLoc.X + 2 + XOffset;
		vPos.Y = vLoc.Y + 6 + YOffset;
	}

	if (pThis->IsSelected && canSee)
	{
		if (pThis->IsGreenHP())
			frame = selectbrdFrame.X;
		else if (pThis->IsYellowHP())
			frame = selectbrdFrame.Y;
		else
			frame = selectbrdFrame.Z;

		DSurface::Temp->DrawSHP(SelectBrdPAL, SelectBrdSHP,
			frame, &vPos, pBound, nFlag, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}
}

// Based on Ares source.
void TechnoExt::DrawInsignia(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds)
{
	Point2D offset = *pLocation;

	SHPStruct* pShapeFile = FileSystem::PIPS_SHP;
	int defaultFrameIndex = -1;

	auto pTechnoType = pThis->GetTechnoType();
	auto pOwner = pThis->Owner;

	if (pThis->IsDisguised() && !pThis->IsClearlyVisibleTo(HouseClass::Player))
	{
		//check the proper type before casting , duh
		if (auto const pType = type_cast<TechnoTypeClass*>(pThis->Disguise))
		{
			pTechnoType = pType;
			pOwner = pThis->DisguisedAsHouse;
		}
		else if (!pOwner->IsAlliedWith(HouseClass::Player) && !HouseClass::IsPlayerObserver())
		{
			return;
		}
	}

	TechnoTypeExt::ExtData* pExt = TechnoTypeExt::ExtMap.Find(pTechnoType);

	bool isVisibleToPlayer = (pOwner && pOwner->IsAlliedWith(HouseClass::Player))
		|| HouseClass::IsPlayerObserver()
		|| pExt->Insignia_ShowEnemy.Get(RulesExt::Global()->EnemyInsignia);

	if (!isVisibleToPlayer)
		return;

	bool isCustomInsignia = false;

	if (SHPStruct* pCustomShapeFile = pExt->Insignia.Get(pThis))
	{
		pShapeFile = pCustomShapeFile;
		defaultFrameIndex = 0;
		isCustomInsignia = true;
	}

	VeterancyStruct* pVeterancy = &pThis->Veterancy;
	auto& insigniaFrames = pExt->InsigniaFrames.Get();
	int insigniaFrame = insigniaFrames.X;

	if (pVeterancy->IsVeteran())
	{
		defaultFrameIndex = !isCustomInsignia ? 14 : defaultFrameIndex;
		insigniaFrame = insigniaFrames.Y;
	}
	else if (pVeterancy->IsElite())
	{
		defaultFrameIndex = !isCustomInsignia ? 15 : defaultFrameIndex;
		insigniaFrame = insigniaFrames.Z;
	}

	int frameIndex = pExt->InsigniaFrame.Get(pThis);
	frameIndex = frameIndex == -1 ? insigniaFrame : frameIndex;

	if (frameIndex == -1)
		frameIndex = defaultFrameIndex;

	if (frameIndex != -1 && pShapeFile)
	{
		offset.X += 5;
		offset.Y += pThis->WhatAmI() != AbstractType::Infantry ? 4:2;

		DSurface::Temp->DrawSHP(
			FileSystem::PALETTE_PAL, pShapeFile, frameIndex, &offset, pBounds, BlitterFlags(0xE00), 0, -2, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}

	return;
}

bool TechnoExt::CheckIfCanFireAt(TechnoClass* pThis, AbstractClass* pTarget)
{
	const int wpnIdx = pThis->SelectWeapon(pTarget);
	const FireError fErr = pThis->GetFireError(pTarget, wpnIdx, true);
	if (fErr != FireError::ILLEGAL
		&& fErr != FireError::CANT
		&& fErr != FireError::MOVING
		&& fErr != FireError::RANGE)

	{
		return pThis->IsCloseEnough(pTarget, wpnIdx);
	}
	else
		return false;
}

void TechnoExt::ForceJumpjetTurnToTarget(TechnoClass* pThis)
{
	const auto pType = pThis->GetTechnoType();
	if (pType->Locomotor == LocomotionClass::CLSIDs::Jumpjet && pThis->IsInAir()
		&& !pType->TurretSpins)
	{
		const auto pFoot = abstract_cast<UnitClass*>(pThis);
		const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

		if (pTypeExt && pTypeExt->JumpjetTurnToTarget.Get(RulesExt::Global()->JumpjetTurnToTarget)
			&& pFoot && pFoot->GetCurrentSpeed() == 0)
		{
			if (const auto pTarget = pThis->Target)
			{
				const auto pLoco = static_cast<JumpjetLocomotionClass*>(pFoot->Locomotor.get());
				if (pLoco && !pLoco->Facing.in_motion() && TechnoExt::CheckIfCanFireAt(pThis, pTarget))
				{
					const CoordStruct source = pThis->Location;
					const CoordStruct target = pTarget->GetCoords();
					const DirStruct tgtDir = DirStruct(Math::arctanfoo(source.Y - target.Y, target.X - source.X));

					if (pThis->GetRealFacing().current().value32() != tgtDir.value32())
						pLoco->Facing.turn(tgtDir);
				}
			}
		}
	}
}

void TechnoExt::DisplayDamageNumberString(TechnoClass* pThis, int damage, bool isShieldDamage)
{
	if (!pThis || damage == 0)
		return;

	const auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (!pExt)
		return;

	auto color = isShieldDamage ? damage > 0 ? ColorStruct { 0, 160, 255 } : ColorStruct { 0, 255, 230 } :
		damage > 0 ? ColorStruct { 255, 0, 0 } : ColorStruct { 0, 255, 0 };

	wchar_t damageStr[0x20];
	swprintf_s(damageStr, L"%d", damage);
	auto coords = CoordStruct::Empty;
	coords = *pThis->GetCenterCoord(&coords);

	int maxOffset = 30;
	int width = 0, height = 0;
	BitFont::Instance->GetTextDimension(damageStr, &width, &height, 120);

	if (pExt->DamageNumberOffset >= maxOffset || pExt->DamageNumberOffset.empty())
		pExt->DamageNumberOffset = -maxOffset;

	if (auto pBuilding = specific_cast<BuildingClass*>(pThis))
		coords.Z += 104 * pBuilding->Type->Height;
	else
		coords.Z += 256;

	if (auto const pCell = MapClass::Instance->TryGetCellAt(coords))
	{
		if (!pCell->IsFogged() && !pCell->IsShrouded())
		{
			if (pThis->VisualCharacter(0, HouseClass::Player()) != VisualType::Hidden)
			{
				FlyingStrings::Add(damageStr, coords, color, Point2D { pExt->DamageNumberOffset - (width / 2), 0 });
			}
		}
	}

	pExt->DamageNumberOffset = pExt->DamageNumberOffset + width;
}

int TechnoExt::GetSizeLeft(FootClass* const pFoot)
{
	return pFoot->GetTechnoType()->Passengers - pFoot->Passengers.GetTotalSize();
}

void TechnoExt::Stop(TechnoClass* pThis, Mission const& eMission)
{
	pThis->ForceMission(eMission);
	pThis->CurrentTargets.Clear();
	pThis->SetFocus(nullptr);
	pThis->Stun();
}

bool TechnoExt::IsOnLimbo(TechnoClass* pThis, bool bIgnore) {
	return !bIgnore && pThis->InLimbo;
}

bool TechnoExt::IsDeactivated(TechnoClass* pThis, bool bIgnore) {
	return !bIgnore && pThis->Deactivated;
}

bool TechnoExt::IsUnderEMP(TechnoClass* pThis, bool bIgnore) {
	return !bIgnore && pThis->IsUnderEMP();
}

bool TechnoExt::IsActive(TechnoClass* pThis, bool bCheckEMP, bool bCheckDeactivated, bool bIgnoreLimbo, bool bIgnoreIsOnMap , bool bIgnoreAbsorb )
{
	if (!TechnoExt::IsAlive(pThis , bIgnoreLimbo, bIgnoreIsOnMap, bIgnoreAbsorb))
		return false;

	if (pThis->BeingWarpedOut || IsUnderEMP(pThis, !bCheckEMP) || IsDeactivated(pThis, !bCheckDeactivated))
		return false;

	return true ;
}

bool TechnoExt::IsAlive(TechnoClass* pThis, bool bIgnoreLimbo, bool bIgnoreIsOnMap, bool bIgnoreAbsorb)
{
	if (!pThis)
		return false;

	if ((IsOnLimbo(pThis,!bIgnoreLimbo)) || (pThis->Absorbed && !bIgnoreAbsorb) || (!pThis->IsOnMap && !bIgnoreIsOnMap))
		return false;

	if (pThis->IsCrashing || pThis->IsSinking)
		return false;

	if (pThis->WhatAmI() == AbstractType::Unit)
		return (static_cast<UnitClass*>(pThis)->DeathFrameCounter > 0) ? false : true;

	return pThis->IsAlive && pThis->Health > 0;
}

void TechnoExt::ObjectKilledBy(TechnoClass* pVictim, TechnoClass* pKiller)
{
	if (auto pVictimTechno = static_cast<TechnoClass*>(pVictim))
	{
		auto pVictimTechnoData = TechnoExt::ExtMap.Find(pVictim);

		if (pVictimTechnoData && pKiller)
		{
			TechnoClass* pObjectKiller = nullptr;

			if ((pKiller->GetTechnoType()->Spawned || pKiller->GetTechnoType()->MissileSpawn) && pKiller->SpawnOwner)
				pObjectKiller = pKiller->SpawnOwner;
			else
				pObjectKiller = pKiller;

			if (pObjectKiller && pObjectKiller->BelongsToATeam())
			{
				if (auto pKillerTechnoData = TechnoExt::ExtMap.Find(pObjectKiller))
				{
					auto const pFootKiller = abstract_cast<FootClass*>(pObjectKiller);
					auto const pFocus = abstract_cast<TechnoClass*>(pFootKiller->Team->Focus);
					/*
					Debug::Log("DEBUG: pObjectKiller -> [%s] [%s] registered a kill of the type [%s]\n",
						pFootKiller->Team->Type->ID, pObjectKiller->get_ID(), pVictim->get_ID());
					*/
					pKillerTechnoData->LastKillWasTeamTarget = false;
					if (pFocus == pVictim)
						pKillerTechnoData->LastKillWasTeamTarget = true;
				}
			}
		}
	}
}

void TechnoExt::ApplyMindControlRangeLimit(TechnoClass* pThis)
{
	if (const auto pCapturer = pThis->MindControlledBy)
	{
		const auto pCapturerExt = TechnoTypeExt::ExtMap.Find(pCapturer->GetTechnoType());
		if (pCapturerExt && pCapturerExt->MindControlRangeLimit.Get() > 0 &&
			pThis->DistanceFrom(pCapturer) > pCapturerExt->MindControlRangeLimit.Get())
		{
			pCapturer->CaptureManager->FreeUnit(pThis);
		}
	}
}

void TechnoExt::ApplyInterceptor(TechnoClass* pThis, TechnoTypeExt::ExtData* pExt)
{
	if (!TechnoExt::IsActive(pThis, true, true,true,true))
		return;

	if (pExt->Interceptor.Get() && !pThis->Target &&
		!(pThis->WhatAmI() == AbstractType::Aircraft && pThis->GetHeight() <= 0))
	{
		if (auto const pTransport = pThis->Transporter) {
			if (pTransport->WhatAmI() == AbstractType::Aircraft)
				if (!pTransport->IsInAir())
					return;
		}

		auto const iter = std::find_if(BulletClass::Array->begin(), BulletClass::Array->end(), [=](BulletClass* const pBullet)
		{

		if (const auto pBulletTypeData = BulletTypeExt::ExtMap.Find(pBullet->Type))
		{
			if (!pBulletTypeData->Interceptable || pBullet->Health <= 0 || pBullet->InLimbo)
				return false;

			const auto& guardRange = pExt->Interceptor_GuardRange.Get(pThis);
			const auto& minguardRange = pExt->Interceptor_MinimumGuardRange.Get(pThis);
			const auto  distance = pBullet->Location.DistanceFrom(pThis->Location);

			if (distance > guardRange || distance < minguardRange)
			{
				if (static_cast<int>(pBulletTypeData->Armor) >= 0)
				{
					int weaponIndex = pThis->SelectWeapon(pBullet);
					auto pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType;
					double versus = GeneralUtils::GetWarheadVersusArmor(pWeapon->Warhead, pBulletTypeData->Armor);

					if (!((fabs(versus) >= 0.001)))
						return false;
				}

				// dont target same bulet multiple time , ugh ,..
				if (const auto pBulletExt = BulletExt::ExtMap.Find(pBullet))
				{
					const auto bulletOwner = pBullet->Owner ? pBullet->Owner->GetOwningHouse() : pBulletExt->Owner;

					if (!EnumFunctions::CanTargetHouse(pExt->Interceptor_CanTargetHouses.Get(), pThis->Owner, bulletOwner))
						return false;

					if (pBulletExt->InterceptedStatus == InterceptedStatus::Targeted)
						return false;
				}

				return true;
			}
		}

		return false;
		});

		if (iter != BulletClass::Array->end()) {
			pThis->SetTarget((*iter));
		}
	}
}

void TechnoExt::ApplySpawn_LimitRange(TechnoClass* pThis, TechnoTypeExt::ExtData* pExt)
{
	if (!TechnoExt::IsActive(pThis, true, true,true))
		return;

	if (pExt->Spawn_LimitedRange)
	{
		if (auto const pManager = pThis->SpawnManager)
		{
			int weaponRange = 0;
			int weaponRangeExtra = pExt->Spawn_LimitedExtraRange * 256;

			auto setWeaponRange = [&weaponRange](WeaponTypeClass* pWeaponType)
			{
				if (pWeaponType && pWeaponType->Spawner && pWeaponType->Range > weaponRange)
					weaponRange = pWeaponType->Range;
			};

			setWeaponRange(pExt->OwnerObject()->Weapon[0].WeaponType);
			setWeaponRange(pExt->OwnerObject()->Weapon[1].WeaponType);
			setWeaponRange(pExt->OwnerObject()->EliteWeapon[0].WeaponType);
			setWeaponRange(pExt->OwnerObject()->EliteWeapon[1].WeaponType);

			weaponRange += weaponRangeExtra;

			if (pManager->Target && (pThis->DistanceFrom(pManager->Target) > weaponRange))
				pManager->ResetTarget();
		}
	}
}

bool TechnoExt::IsHarvesting(TechnoClass* pThis)
{
	if (!TechnoExt::IsActive(pThis,true,true))
		return false;

	const auto slave = pThis->SlaveManager;
	if (slave && slave->State != SlaveManagerStatus::Ready)
		return true;

	if (pThis->WhatAmI() == AbstractType::Building && pThis->IsPowerOnline())
		return true;

	const auto mission = pThis->GetCurrentMission();
	if ((mission == Mission::Harvest || mission == Mission::Unload || mission == Mission::Enter)
		&& TechnoExt::HasAvailableDock(pThis))
	{
		return true;
	}

	return false;
}

bool TechnoExt::HasAvailableDock(TechnoClass* pThis)
{
	for (auto const& pBld : pThis->GetTechnoType()->Dock) {
		if (pThis->Owner->CountOwnedAndPresent(pBld) > 0)
			return true;
	}

	return false;
}

void TechnoExt::InitializeLaserTrail(TechnoClass* pThis, bool bIsconverted)
{
	const auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (!pExt || pThis->WhatAmI() == AbstractType::Building || pThis->GetTechnoType()->Invisible)
		return;

	if (bIsconverted)
		pExt->LaserTrails.clear();

	if (const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
	{
		auto const pOwner = pThis->GetOwningHouse() ? pThis->GetOwningHouse() : HouseExt::FindCivilianSide();

		if (pExt->LaserTrails.empty())
		{
			for (auto const& entry : pTypeExt->LaserTrailData)
			{
				if (auto const pLaserType = LaserTrailTypeClass::Array[entry.idxType].get())
				{
					pExt->LaserTrails.push_back((std::make_unique<LaserTrailClass>(
						pLaserType, pOwner->LaserColor, entry.FLH, entry.IsOnTurret)));
				}
			}
		}
	}
}

void TechnoExt::FireWeaponAtSelf(TechnoClass* pThis, WeaponTypeClass* pWeaponType)
{
	WeaponTypeExt::DetonateAt(pWeaponType, pThis, pThis);
}

Matrix3D TechnoExt::GetMatrix(FootClass* pThis)
{
	// Step 1: get body transform matrix
	if (pThis && pThis->Locomotor)
		return pThis->Locomotor->Draw_Matrix(nullptr);

	 // no locomotor means no rotation or transform of any kind (f.ex. buildings) - Kerbiter
	const Matrix3D Mtx { };
	Mtx.MakeIdentity();
	return 	Mtx;
}

// reversed from 6F3D60
CoordStruct TechnoExt::GetFLHAbsoluteCoords(TechnoClass* pThis, const CoordStruct& pCoord, bool isOnTurret, const CoordStruct& Overrider)
{
	auto const pType = pThis->GetTechnoType();
	Matrix3D mtx = TechnoExt::GetMatrix(abstract_cast<FootClass*>(pThis));

	// Steps 2-3: turret offset and rotation
	if (isOnTurret && pThis->HasTurret())
	{
		TechnoTypeExt::ApplyTurretOffset(pType, &mtx);

		double turretRad = (pThis->TurretFacing().value32() - 8) * -(Math::Pi / 16);
		double bodyRad = (pThis->PrimaryFacing.current().value32() - 8) * -(Math::Pi / 16);
		float angle = static_cast<float>(turretRad - bodyRad);

		mtx.RotateZ(angle);
	}

	// Step 4: apply FLH offset
	mtx.Translate(static_cast<float>(pCoord.X), static_cast<float>(pCoord.Y), static_cast<float>(pCoord.Z));

	Vector3D<float> result = Matrix3D::MatrixMultiply(mtx, Vector3D<float>::Empty);

	// Resulting coords are mirrored along X axis, so we mirror it back
	result.Y *= -1;

	// Step 5: apply as an offset to global object coords
	CoordStruct location = Overrider ? Overrider : pThis->GetCoords();
	location += { static_cast<int>(result.X), static_cast<int>(result.Y), static_cast<int>(result.Z) };

	return location;
}

int TechnoExt::GetEatPassangersTotalTime(TechnoExt::ExtData const* pExt, TechnoTypeExt::ExtData const* pData, FootClass const* pPassenger)
{

	if (pData->PassengerDeletion_UseCostAsRate.Get())
	{
		// Use passenger cost as countdown.
		auto timerLength = static_cast<int>(pPassenger->GetTechnoType()->Cost * pData->PassengerDeletion_CostMultiplier);

		if (pData->PassengerDeletion_Rate.Get() > 0)
			timerLength = std::min(timerLength, pData->PassengerDeletion_Rate.Get());

		return timerLength;
	}
	else if (pData->PassengerDeletion_Rate.Get() > 0)
	{
		// Use explicit rate optionally multiplied by unit size as countdown.
		auto timerLength = pData->PassengerDeletion_Rate.Get();
		if (pData->PassengerDeletion_Rate_SizeMultiply.Get() && pPassenger->GetTechnoType()->Size > 1.0)
			timerLength *= static_cast<int>(pPassenger->GetTechnoType()->Size + 0.5);

		return timerLength;
	}

	return 0;
}

void TechnoExt::EatPassengers(TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	auto const pThis = pExt->OwnerObject();

	if (!TechnoExt::IsActive(pThis, false, false))
		return;

	if ((pTypeExt->PassengerDeletion_Rate > 0 || pTypeExt->PassengerDeletion_UseCostAsRate))
	{
		if (pThis->Passengers.NumPassengers > 0)
		{
			FootClass* pPassenger = pThis->Passengers.GetFirstPassenger();

			if (pExt->PassengerDeletionCountDown < 0)
			{
				// Setting & start countdown. Bigger units needs more time
				int passengerSize = TechnoExt::GetEatPassangersTotalTime(pExt, pTypeExt, pPassenger);
				pExt->PassengerDeletionCountDown = passengerSize;
				pExt->PassengerDeletionTimer.Start(passengerSize);
			}
			else
			{
				if (pExt->PassengerDeletionTimer.Completed())
				{
					ObjectClass* pLastPassenger = nullptr;

					// Passengers are designed as a FIFO queue but being implemented as a list
					while (pPassenger->NextObject)
					{
						pLastPassenger = pPassenger;
						pPassenger = abstract_cast<FootClass*>(pPassenger->NextObject);
					}

					if (pLastPassenger)
						pLastPassenger->NextObject = nullptr;
					else
						pThis->Passengers.FirstPassenger = nullptr;

					--pThis->Passengers.NumPassengers;

					if (pPassenger) {

						pPassenger->LiberateMember();

						if(pTypeExt->PassengerDeletion_ReportSound.isset() && pTypeExt->PassengerDeletion_ReportSound != -1)
							VocClass::PlayAt(pTypeExt->PassengerDeletion_ReportSound, pThis->GetCoords(), nullptr);

						if (const auto pAnimType = pTypeExt->PassengerDeletion_Anim.Get(nullptr)) {
							if (auto const pAnim = GameCreate<AnimClass>(pAnimType, pThis->Location)) {
								pAnim->SetOwnerObject(pThis);
								AnimExt::SetAnimOwnerHouseKind(pAnim, pThis->GetOwningHouse(), pPassenger->GetOwningHouse(), pThis, false);
							}
						}

						if (auto const pPassengerType = pPassenger->GetTechnoType())
						{
							// Check if there is money refund
							if (pTypeExt->PassengerDeletion_Soylent)
							{
								int nMoneyToGive = static_cast<int>(pPassengerType->GetRefund(pPassenger->Owner, true) *
									pTypeExt->PassengerDeletion_SoylentMultiplier.Get());

								// Is allowed the refund of friendly units?
								if (!pTypeExt->PassengerDeletion_SoylentFriendlies.Get() &&
									pPassenger->GetOwningHouse() &&
									pPassenger->GetOwningHouse()->IsAlliedWith(pThis))
									nMoneyToGive = 0;

								if (nMoneyToGive != 0 && pThis->GetOwningHouse() && pThis->GetOwningHouse()->CanTransactMoney(nMoneyToGive))
								{
									pThis->Owner->TransactMoney(nMoneyToGive);

									if (pTypeExt->PassengerDeletion_DisplaySoylent)
									{
										FlyingStrings::AddMoneyString(true, nMoneyToGive, pThis,
											pTypeExt->PassengerDeletion_DisplaySoylentToHouses.Get(), pThis->GetCoords(), pTypeExt->PassengerDeletion_DisplaySoylentOffset.Get());
									}
								}
							}
						}

						// Handle gunner change.
						if (pThis->GetTechnoType()->Gunner) {
							if (auto const pFoot = abstract_cast<FootClass*>(pThis)) {
								pFoot->RemoveGunner(pPassenger);

								if (pThis->Passengers.NumPassengers > 0)
									pFoot->ReceiveGunner(pThis->Passengers.FirstPassenger);
							}
						}

						if (pThis->GetTechnoType()->OpenTopped) {
							pThis->ExitedOpenTopped(pPassenger);
						}

						if (auto pPassangerOwner = pPassenger->GetOwningHouse()) {
							if (auto pBld = specific_cast<BuildingClass*>(pThis)){
								if (pBld->Absorber()){
									if (pBld->Type->ExtraPowerBonus > 0) {
										pBld->Owner->RecheckPower = true;
									}
								}
							}

							if (!pPassangerOwner->IsNeutral() && !pThis->GetTechnoType()->Insignificant){
								pPassangerOwner->RegisterLoss(pPassenger, false);
								pPassangerOwner->RemoveTracking(pPassenger);
								pPassangerOwner->RecheckTechTree = true;
							}
						}

						pPassenger->UnInit();
					}

					pExt->PassengerDeletionTimer.Stop();
					pExt->PassengerDeletionCountDown = -1;
				}
			}
		}
		else
		{
			pExt->PassengerDeletionTimer.Stop();
			pExt->PassengerDeletionCountDown = -1;
		}
	}
}

bool TechnoExt::CanFireNoAmmoWeapon(TechnoClass* pThis, int weaponIndex)
{
	if (pThis->GetTechnoType()->Ammo > 0)
	{
		if (const auto pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
		{
			if (pThis->Ammo <= pExt->NoAmmoAmount && (pExt->NoAmmoWeapon = weaponIndex || pExt->NoAmmoWeapon == -1))
				return true;
		}
	}

	return false;
}

void TechnoExt::HandleRemove(TechnoClass* pThis)
{
	pThis->KillCargo(pThis);

	if (const auto pBuilding = specific_cast<BuildingClass*>(pThis)) {
		pBuilding->KillOccupants(nullptr);
	}

	if (const auto pOwner = pThis->GetOwningHouse()) {
		if (!pOwner->IsNeutral() && !pThis->GetTechnoType()->Insignificant) {
			pOwner->RegisterLoss(pThis, false);
			pOwner->RemoveTracking(pThis);
			pOwner->RecheckTechTree = true;
		}
	}

	if (auto const pFoot = generic_cast<FootClass*>(pThis))
		pFoot->LiberateMember();

	pThis->RemoveFromTargetingAndTeam();

	for (auto const pBullet : *BulletClass::Array)
		if (pBullet && pBullet->Target == pThis)
			pBullet->LoseTarget();

}

void TechnoExt::KillSelf(TechnoClass* pThis, bool isPeaceful)
{
	if (isPeaceful)
	{
		// this shit is not really good idea to pull of
		// some stuffs doesnt really handled properly , wtf
		if(!pThis->InLimbo)
			pThis->Limbo();

		TechnoExt::HandleRemove(pThis);
	}
	else
	{
		pThis->ReceiveDamage(&pThis->Health, 0, RulesClass::Instance()->C4Warhead, nullptr, false, false, pThis->Owner);
	}
}

void TechnoExt::KillSelf(TechnoClass* pThis, const KillMethod& deathOption , bool RegisterKill)
{
	KillMethod nOpt = deathOption;
	if (deathOption == KillMethod::Random) {
		nOpt = static_cast<KillMethod>(ScenarioClass::Instance->Random.RandomRanged((int)KillMethod::Explode, (int)KillMethod::Sell));
	}

	switch (nOpt)
	{
	case KillMethod::Explode:
	{
		Kill:
		pThis->ReceiveDamage(&pThis->Health, 0, RulesClass::Instance()->C4Warhead, nullptr, true, false, pThis->Owner);
	}break;
	case KillMethod::Vanish:
	{
		// this shit is not really good idea to pull off
		// some stuffs doesnt really handled properly , wtf
		if (!pThis->InLimbo)
			pThis->Limbo();

		if(RegisterKill)
			pThis->RegisterKill(pThis->Owner);

		TechnoExt::HandleRemove(pThis);

	}break;
	case KillMethod::Sell:
	{
		if (const auto pBld = specific_cast<BuildingClass*>(pThis)) {
			if (pBld->Type->LoadBuildup()) {
				pBld->Sell(true);
				return;
			}

		} else if (const auto pFoot = generic_cast<FootClass*>(pThis)) {
			if(pFoot->WhatAmI() != AbstractType::Infantry) {
				if (auto const pCell = pFoot->GetCell()) {
					if (auto const pBuilding = pCell->GetBuilding()) {
						if (pBuilding->Type->UnitRepair) {
							pFoot->Sell(true);
							return;
						}
					}
				}
			}
		}

		Debug::Log("Techno [%s] can't be sold, killing it instead\n", pThis->get_ID());
		goto Kill;

	}break;
	}
}

// Feature: Kill Object Automatically
void TechnoExt::CheckDeathConditions(TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	const auto pThis = pExt->OwnerObject();

	if (!TechnoExt::IsActive(pThis, false, false))
		return;

	auto pTypeThis = pTypeExt->OwnerObject();
	const bool peacefulDeath = pTypeExt->Death_Peaceful.Get();
	const auto nKillMethod = peacefulDeath ? KillMethod::Vanish : pTypeExt->Death_Method.Get();

	// Death if no ammo
	if (pTypeExt->Death_NoAmmo)
	{
		if (pTypeThis->Ammo > 0 && pThis->Ammo <= 0)
		{
			TechnoExt::KillSelf(pThis, nKillMethod);
			return;
		}
	}

	// Death if countdown ends
	if (pTypeExt->Death_Countdown > 0)
	{
		if (pExt->Death_Countdown >= 0)
		{
			if (pExt->Death_Countdown > 0)
			{
				pExt->Death_Countdown--; // Update countdown
			}
			else
			{
				// Countdown ended. Kill the unit
				pExt->Death_Countdown = -1;
				TechnoExt::KillSelf(pThis, nKillMethod);
				return;
			}
		}
		else
		{
			pExt->Death_Countdown = pTypeExt->Death_Countdown; // Start countdown
		}
	}

}

void TechnoExt::ApplyGainedSelfHeal(TechnoClass* pThis)
{
	const int healthDeficit = pThis->GetTechnoType()->Strength - pThis->Health;

	if (pThis->Health && healthDeficit > 0)
	{
		if (auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
		{
			const bool isBuilding = pThis->WhatAmI() == AbstractType::Building;
			const bool isOrganic = pThis->WhatAmI() == AbstractType::Infantry || pThis->WhatAmI() == AbstractType::Unit && pThis->GetTechnoType()->Organic;
			const auto defaultSelfHealType = isBuilding ? SelfHealGainType::None : isOrganic ? SelfHealGainType::Infantry : SelfHealGainType::Units;
			const auto selfHealType = pExt->SelfHealGainType.Get(defaultSelfHealType);
			bool applyHeal = false;
			int amount = 0;

			switch (selfHealType)
			{
			case SelfHealGainType::Infantry:
			{
				const int count = RulesExt::Global()->InfantryGainSelfHealCap.isset() ?
					std::clamp(RulesExt::Global()->InfantryGainSelfHealCap.Get(), 1, pThis->Owner->InfantrySelfHeal) :
					pThis->Owner->InfantrySelfHeal;

				amount = RulesClass::Instance->SelfHealInfantryAmount * count;

				if (!(Unsorted::CurrentFrame % RulesClass::Instance->SelfHealInfantryFrames) && amount)
					applyHeal = true;
			}
			break;
			case SelfHealGainType::Units:
			{
				const int count = RulesExt::Global()->UnitsGainSelfHealCap.isset() ?
					std::clamp(RulesExt::Global()->UnitsGainSelfHealCap.Get(),1, pThis->Owner->UnitsSelfHeal):
					pThis->Owner->UnitsSelfHeal;

				amount = RulesClass::Instance->SelfHealUnitAmount * count;

				if (!(Unsorted::CurrentFrame % RulesClass::Instance->SelfHealUnitFrames) && amount)
					applyHeal = true;
			}
			break;
			default:
				return;
			}

			if (applyHeal && amount)
			{
				if (amount >= healthDeficit)
					amount = healthDeficit;

				const bool wasDamaged = pThis->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow;

				pThis->Health += amount;

				if (wasDamaged && (pThis->GetHealthPercentage() > RulesClass::Instance->ConditionYellow
					|| pThis->GetHeight() < -10))
				{
					if (auto const pBuilding = abstract_cast<BuildingClass*>(pThis))
					{
						pBuilding->UpdatePlacement(PlacementType::Redraw);
						pBuilding->ToggleDamagedAnims(false);
					}

					if (pThis->WhatAmI() == AbstractType::Unit || pThis->WhatAmI() == AbstractType::Building) {
						if (const auto dmgParticle = pThis->DamageParticleSystem)
							dmgParticle->UnInit();
					}
				}
			}
		}
	}

	return;
}

void TechnoExt::DrawSelfHealPips(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds)
{
	bool drawPip = false;
	bool isInfantryHeal = false;
	int selfHealFrames = 0;

	if (auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
	{
		if (pExt->SelfHealGainType.isset() && pExt->SelfHealGainType.Get() == SelfHealGainType::None)
			return;

		const bool hasInfantrySelfHeal = pExt->SelfHealGainType.isset() && pExt->SelfHealGainType.Get() == SelfHealGainType::Infantry;
		const bool hasUnitSelfHeal = pExt->SelfHealGainType.isset() && pExt->SelfHealGainType.Get() == SelfHealGainType::Units;
		const bool isOrganic = pThis->WhatAmI() == AbstractType::Infantry || (pThis->GetTechnoType()->Organic && (pThis->WhatAmI() == AbstractType::Unit));
		//bool isAircraft = pThis->WhatAmI() == AbstractType::Aircraft || (pThis->GetTechnoType()->ConsideredAircraft && pThis->WhatAmI() == AbstractType::Unit);

		if (pThis->Owner->InfantrySelfHeal > 0 && (hasInfantrySelfHeal || isOrganic))
		{
			drawPip = true;
			selfHealFrames = RulesClass::Instance->SelfHealInfantryFrames;
			isInfantryHeal = true;
		}
		else if (pThis->Owner->UnitsSelfHeal > 0
			&& (hasUnitSelfHeal || pThis->WhatAmI() == AbstractType::Unit)
			)
		{
			drawPip = true;
			selfHealFrames = RulesClass::Instance->SelfHealUnitFrames;
		}
	}

	if (drawPip)
	{
		Point2D pipFrames { 0,0 };
		bool isSelfHealFrame = false;
		int xOffset = 0;
		int yOffset = 0;

		if (Unsorted::CurrentFrame % selfHealFrames <= 5
			&& pThis->Health < pThis->GetTechnoType()->Strength) {
			isSelfHealFrame = true;
		}

		int nBracket = pThis->GetTechnoType()->PixelSelectionBracketDelta;
		if (auto pFoot = generic_cast<FootClass*>(pThis->Disguise))
			if(pThis->IsDisguised() && !pThis->IsClearlyVisibleTo(HouseClass::Player))
				nBracket = pFoot->GetTechnoType()->PixelSelectionBracketDelta;

		switch (pThis->WhatAmI())
		{
		case AbstractType::Unit:
		case AbstractType::Aircraft:
		{
			const auto& offset = RulesExt::Global()->Pips_SelfHeal_Units_Offset.Get();
			pipFrames = RulesExt::Global()->Pips_SelfHeal_Units.Get();
			xOffset = offset.X;
			yOffset = offset.Y + nBracket;
		}
		break;
		case AbstractType::Infantry:
		{
			const auto& offset = RulesExt::Global()->Pips_SelfHeal_Infantry_Offset.Get();
			pipFrames = RulesExt::Global()->Pips_SelfHeal_Infantry.Get();
			xOffset = offset.X;
			yOffset = offset.Y + nBracket;
		}
		break;
		case AbstractType::Building:
		{
			const auto pType = static_cast<BuildingTypeClass*>(pThis->GetTechnoType());
			int fHeight = pType->GetFoundationHeight(false);
			int yAdjust = -Unsorted::CellHeightInPixels / 2;

			const auto& offset = RulesExt::Global()->Pips_SelfHeal_Buildings_Offset.Get();
			pipFrames = RulesExt::Global()->Pips_SelfHeal_Buildings.Get();
			xOffset = offset.X + Unsorted::CellWidthInPixels / 2 * fHeight;
			yOffset = offset.Y + yAdjust * fHeight + pType->Height * yAdjust;
		}
		break;
		}

		int pipFrame = isInfantryHeal ? pipFrames.X : pipFrames.Y;

		Point2D position { pLocation->X + xOffset, pLocation->Y + yOffset };

		auto flags = BlitterFlags::bf_400 | BlitterFlags::Centered;

		if (isSelfHealFrame)
			flags = flags | BlitterFlags::Darken;

		DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS_SHP,
			pipFrame, &position, pBounds, flags, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}
}

void TechnoExt::UpdateSharedAmmo(TechnoClass* pThis)
{
	if (!pThis)
		return;

	if (const auto pType = pThis->GetTechnoType())
	{
		if (pType->OpenTopped && pThis->Passengers.NumPassengers > 0)
		{
			if (const auto pExt = TechnoTypeExt::ExtMap.Find(pType))
			{
				if (pExt->Ammo_Shared && pType->Ammo > 0)
				{
					auto passenger = pThis->Passengers.FirstPassenger;

					do
					{
						TechnoTypeClass*  passengerType = passenger->GetTechnoType();
						auto pPassengerExt = TechnoTypeExt::ExtMap.Find(passengerType);

						if (pPassengerExt && pPassengerExt->Ammo_Shared.Get())
						{
							if (pExt->Ammo_Shared_Group.Get() < 0 || pExt->Ammo_Shared_Group.Get() == pPassengerExt->Ammo_Shared_Group.Get())
							{
								if (pThis->Ammo > 0 && (passenger->Ammo < passengerType->Ammo))
								{
									pThis->Ammo--;
									passenger->Ammo++;
								}
							}
						}

						passenger = static_cast<FootClass*>(passenger->NextObject);
					}
					while (passenger);
				}
			}
		}
	}
}

double TechnoExt::GetCurrentSpeedMultiplier(FootClass* pThis)
{
	double houseMultiplier = 1.0;

	switch (pThis->WhatAmI())
	{
	case AbstractType::Aircraft:
		houseMultiplier = pThis->Owner->Type->SpeedAircraftMult;
		break;
	case AbstractType::Infantry:
		houseMultiplier = pThis->Owner->Type->SpeedInfantryMult;
		break;
	case AbstractType::Unit:
		houseMultiplier = pThis->Owner->Type->SpeedUnitsMult;
		break;
	}

	return pThis->SpeedMultiplier * houseMultiplier *
		(pThis->HasAbility(AbilityType::Faster) ? RulesClass::Instance->VeteranSpeed : 1.0);
}

void TechnoExt::UpdateMindControlAnim(TechnoExt::ExtData* pExt)
{
	if (const auto pThis  = pExt->OwnerObject())
	{
		if (pThis->IsMindControlled())
		{
			if (pThis->MindControlRingAnim && !pExt->MindControlRingAnimType)
			{
				pExt->MindControlRingAnimType = pThis->MindControlRingAnim->Type;
			}
			else if (!pThis->MindControlRingAnim && pExt->MindControlRingAnimType &&
				pThis->CloakState == CloakState::Uncloaked && !pThis->InLimbo && pThis->IsAlive)
			{
				auto coords = CoordStruct::Empty;
				coords = *pThis->GetCoords(&coords);
				int offset = 0;

				if (const auto pBuilding = specific_cast<BuildingClass*>(pThis))
					offset = Unsorted::LevelHeight * pBuilding->Type->Height;
				else
					offset = pThis->GetTechnoType()->MindControlRingOffset;

				coords.Z += offset;

				if (auto anim = GameCreate<AnimClass>(pExt->MindControlRingAnimType, coords, 0, 1))
				{
					pThis->MindControlRingAnim = anim;
					pThis->MindControlRingAnim->SetOwnerObject(pThis);

					if (pThis->WhatAmI() == AbstractType::Building)
						pThis->MindControlRingAnim->ZAdjust = -1024;
				}
			}
		}
		else if (pExt->MindControlRingAnimType)
		{
			pExt->MindControlRingAnimType = nullptr;
		}
	}
}

void TechnoExt::RunFireSelf(TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	const auto pThis = pExt->OwnerObject();
	if (pThis->IsRedHP() && !pTypeExt->FireSelf_Weapon_RedHeath.empty() && !pTypeExt->FireSelf_ROF_RedHeath.empty())
	{
		pExt->FireSelf_Weapon = pTypeExt->FireSelf_Weapon_RedHeath;
		pExt->FireSelf_ROF = pTypeExt->FireSelf_ROF_RedHeath;
	}
	else if (pThis->IsYellowHP() && !pTypeExt->FireSelf_Weapon_YellowHeath.empty() && !pTypeExt->FireSelf_ROF_YellowHeath.empty())
	{
		pExt->FireSelf_Weapon = pTypeExt->FireSelf_Weapon_YellowHeath;
		pExt->FireSelf_ROF = pTypeExt->FireSelf_ROF_YellowHeath;
	}
	else if (pThis->IsGreenHP() && !pTypeExt->FireSelf_Weapon_GreenHeath.empty() && !pTypeExt->FireSelf_ROF_GreenHeath.empty())
	{
		pExt->FireSelf_Weapon = pTypeExt->FireSelf_Weapon_GreenHeath;
		pExt->FireSelf_ROF = pTypeExt->FireSelf_ROF_GreenHeath;
	}
	else
	{
		pExt->FireSelf_Weapon = pTypeExt->FireSelf_Weapon;
		pExt->FireSelf_ROF = pTypeExt->FireSelf_ROF;
	}

	if (pExt->FireSelf_Weapon.empty()) return;
	if (pExt->FireSelf_Count.size() < pExt->FireSelf_Weapon.size())
	{
		int p = int(pExt->FireSelf_Count.size());
		while (pExt->FireSelf_Count.size() < pExt->FireSelf_Weapon.size())
		{
			int ROF = 10;
			if (p >= (int)pExt->FireSelf_ROF.size()) ROF = pExt->FireSelf_Weapon[p]->ROF;
			else ROF = pExt->FireSelf_ROF[p];
			pExt->FireSelf_Count.emplace_back(ROF);
		}
	}
	for (int i = 0; i < (int)pExt->FireSelf_Count.size(); i++)
	{
		pExt->FireSelf_Count[i]--;
		if (pExt->FireSelf_Count[i] > 0) continue;
		else
		{
			int ROF = 10;
			if (i >= (int)pExt->FireSelf_ROF.size()) ROF = pExt->FireSelf_Weapon[i]->ROF;
			else ROF = pExt->FireSelf_ROF[i];
			pExt->FireSelf_Count[i] = ROF;
			WeaponTypeExt::DetonateAt(pExt->FireSelf_Weapon[i], pThis, pThis);
		}
	}
}

// =============================
// load / save

template <typename T>
void TechnoExt::ExtData::Serialize(T& Stm)
{
	Debug::Log("Processing Element From TechnoExt ! \n");

	Stm
		//.Process(this->GenericFuctions)
		.Process(this->ID)
		.Process(this->Shield)
		//.Process(this->BExt)
		.Process(this->LaserTrails)
		.Process(this->ReceiveDamage)
		.Process(this->PassengerDeletionTimer)
		.Process(this->PassengerDeletionCountDown)
		.Process(this->CurrentShieldType)
		.Process(this->LastWarpDistance)
		.Process(this->Death_Countdown)
		.Process(this->MindControlRingAnimType)
		.Process(this->DamageNumberOffset)
		.Process(this->CurrentLaserWeaponIndex)
		.Process(this->OriginalPassengerOwner)
		.Process(this->IsDriverKilled)
		.Process(this->GattlingDmageDelay)
		.Process(this->GattlingDmageSound)
		.Process(this->AircraftOpentoppedInitEd)
		.Process(this->FireSelf_Count)
		.Process(this->FireSelf_Weapon)
		.Process(this->FireSelf_ROF)
#ifdef COMPILE_PORTED_DP_FEATURES
		.Process(this->aircraftPutOffsetFlag)
		.Process(this->aircraftPutOffset)
		.Process(this->VirtualUnit)
		.Process(this->IsMissileHoming)
		.Process(this->SkipVoice)
		.Process(this->HomingTargetLocation)
		.Process(this->ExtraWeaponTimers)
		.Process(this->Trails)
		.Process(this->MyGiftBox)
		.Process(this->PaintBallState)
#ifdef ENABLE_HOMING_MISSILE
		.Process(this->MissileTargetTracker)
#endif
#endif;
		;
	//should put this inside techo ext , ffs
#ifdef COMPILE_PORTED_DP_FEATURES
	this->MyWeaponManager.Serialize(Stm);
	this->MyDriveData.Serialize(Stm);
	this->MyDiveData.Serialize(Stm);
	this->MyJJData.Serialize(Stm);
	this->MySpawnSuport.Serialize(Stm);
	this->MyFighterData.Serialize(Stm);

#endif;
}

void TechnoExt::ExtData::InvalidatePointer(void* ptr, bool bRemoved)
{
	if (auto pShield = this->GetShield())
		pShield->InvalidatePointer(ptr);

#ifdef COMPILE_PORTED_DP_FEATURES
	MyWeaponManager.InvalidatePointer(ptr, bRemoved);
#endif
	AnnounceInvalidPointer(OriginalPassengerOwner, ptr);

#ifdef ENABLE_HOMING_MISSILE
	if (MissileTargetTracker)
		MissileTargetTracker->InvalidatePointer(ptr, bRemoved);
#endif
}

void TechnoExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<TechnoClass>::Serialize(Stm);
	this->Serialize(Stm);
}

void TechnoExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<TechnoClass>::Serialize(Stm);
	this->Serialize(Stm);
}

TechnoExt::ExtContainer::ExtContainer() : Container("TechnoClass") { }
TechnoExt::ExtContainer::~ExtContainer() = default;

bool TechnoExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool TechnoExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container hooks

DEFINE_HOOK(0x6F3260, TechnoClass_CTOR, 0x5)
{
	GET(TechnoClass*, pItem, ESI);
	TechnoExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(0x6F4500, TechnoClass_DTOR, 0x5)
{
	GET(TechnoClass*, pItem, ECX);
	TechnoExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x70C250, TechnoClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x70BF50, TechnoClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(TechnoClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);
	TechnoExt::ExtMap.PrepareStream(pItem, pStm);
	return 0;
}

DEFINE_HOOK(0x70C249, TechnoClass_Load_Suffix, 0x5)
{
	TechnoExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x70C264, TechnoClass_Save_Suffix, 0x5)
{
	TechnoExt::ExtMap.SaveStatic();
	return 0;
}

/*
DEFINE_HOOK(0x70783B, TechnoClass_Detach, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(void*, target, EBP);
	GET_STACK(bool, all, STACK_OFFS(0xC, -0x8));

	if (const auto pExt = TechnoExt::ExtMap.Find(pThis))
		pExt->InvalidatePointer(target, all);

	return pThis->BeingManipulatedBy == target ? 0x707843 : 0x707849;
}*/