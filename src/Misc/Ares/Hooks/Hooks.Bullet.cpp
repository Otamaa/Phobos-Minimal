#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <Ext/WarheadType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/BuildingType/Body.h>

DEFINE_DISABLE_HOOK(0x46A5B2, BulletClass_Shrapnel_WeaponType1_ares)
DEFINE_DISABLE_HOOK(0x46AA27, BulletClass_Shrapnel_WeaponType2_ares)

DEFINE_DISABLE_HOOK(0x4664ba, BulletClass_CTOR_ares)
DEFINE_DISABLE_HOOK(0x4665e9, BulletClass_DTOR_ares)
DEFINE_DISABLE_HOOK(0x46ae70, BulletClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x46af97, BulletClass_Load_Suffix_ares)
DEFINE_DISABLE_HOOK(0x46af9e, BulletClass_Load_Suffix_ares)
DEFINE_DISABLE_HOOK(0x46afb0, BulletClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x46afc4, BulletClass_Save_Suffix_ares)

DEFINE_OVERRIDE_HOOK(0x5f4fe7, ObjectClass_Put, 8)
{
	GET(ObjectClass*, pThis, ESI);
	GET(ObjectTypeClass*, pType, EBX);

	if(auto pBullet = specific_cast<BulletClass*>(pThis)) {
		BulletExt::ExtMap.Find(pBullet)->CreateAttachedSystem();
	}

	return pType ?  0x5F4FEF : 0x5F5210;
}

DEFINE_OVERRIDE_HOOK(0x469467, BulletClass_DetonateAt_CanTemporalTarget, 0x5)
{
	GET(TechnoClass* const, Target, ECX);

	switch (Target->InWhichLayer())
	{
	case Layer::Ground:
	case Layer::Air:
	case Layer::Top:
		return 0x469475;
	}

	return 0x469AA4;
}

// #1708: this mofo was raising an event without checking whether
// there is a valid tag. this is the only faulty call of this kind.
DEFINE_OVERRIDE_HOOK(0x4692A2, BulletClass_DetonateAt_RaiseAttackedByHouse, 0x6)
{
	GET(ObjectClass* const, pVictim, EDI);
	return pVictim->AttachedTag ? 0 : 0x4692BD;
}

// Overpowerer no longer just infantry
DEFINE_OVERRIDE_HOOK(0x4693B0, BulletClass_DetonateAt_Overpower, 0x6)
{
	GET(TechnoClass* const, pT, ECX);
	switch (pT->WhatAmI())
	{
	case InfantryClass::AbsID:
	case UnitClass::AbsID:
	case BuildingClass::AbsID:
		return 0x4693BC;
	default:
		return 0x469AA4;
	}
}

DEFINE_OVERRIDE_HOOK(0x4664FB, BulletClass_Initialize_Ranged, 0x6)
{
	GET(BulletClass*, pThis, ECX);
	// conservative approach for legacy-initialized bullets
	pThis->Range = std::numeric_limits<int>::max();
	return 0;
}

class AresTrajectoryHelper
{
private:
	static bool IsCliffHit(
		CellClass const* pSource, CellClass const* pBefore,
		CellClass const* pAfter);

	static bool IsWallHit(
		CellClass const* pSource, CellClass const* pCheck,
		CellClass const* pTarget, HouseClass const* pOwner);

	static bool IsBuildingHit(
		AbstractClass const* pSource, AbstractClass const* pTarget,
		CoordStruct const& crdCur, HouseClass const* pOwner);

	static Vector2D<int> AbsoluteDifference(const CoordStruct& coords);

	static Vector2D<int> AbsoluteDifference(const CellStruct& cell);

public:
	// gets whether collision checks are needed
	static bool SubjectToAnything(
		BulletTypeClass const* pType, BulletTypeExt::ExtData const* pTypeExt)
	{
		return pType->SubjectToCliffs
			|| pType->SubjectToWalls
			|| pTypeExt->SubjectToSolid;
	}

	// gets the obstacle when moving from pCellBullet to crdCur
	static CellClass* GetObstacle(
		CellClass const* pCellSource, CellClass const* pCellTarget,
		AbstractClass const* pSource, AbstractClass const* pTarget,
		CellClass const* pCellBullet, CoordStruct const& crdCur,
		BulletTypeClass const* pType,
		BulletTypeExt::ExtData const* pTypeExt, HouseClass const* pOwner);

	// gets the first obstacle when moving from crdSrc to crdTarget
	static CellClass* FindFirstObstacle(
		CoordStruct const& crdSrc, CoordStruct const& crdTarget,
		AbstractClass const* pSource, AbstractClass const* pTarget,
		BulletTypeClass const* pType,
		BulletTypeExt::ExtData const* pTypeExt, HouseClass const* pOwner);

	// gets the first obstacle from crdSrc to crdTarget a weapon cannot destroy
	static CellClass* FindFirstImpenetrableObstacle(
		CoordStruct const& crdSrc, CoordStruct const& crdTarget,
		AbstractClass const* pSource, AbstractClass const* pTarget,
		WeaponTypeClass const* pWeapon, HouseClass const* pOwner);
};

bool AresTrajectoryHelper::IsCliffHit(
	CellClass const* const pSource, CellClass const* const pBefore,
	CellClass const* const pAfter)
{
	auto const levelAfter = pAfter->GetLevel();
	return levelAfter - pBefore->GetLevel() >= CellClass::BridgeLevels
		&& levelAfter - pSource->GetLevel() > 0;
}

bool AresTrajectoryHelper::IsWallHit(
	CellClass const* const pSource, CellClass const* const pCheck,
	CellClass const* const pTarget, HouseClass const* const pOwner)
{
	if (pCheck != pTarget && pCheck->OverlayTypeIndex != -1)
	{
		if (OverlayTypeClass::Array->Items[pCheck->OverlayTypeIndex]->Wall)
		{
			if (pSource->Level <= pTarget->Level)
			{
				auto const& index = pCheck->WallOwnerIndex;
				return !RulesClass::Instance->AlliedWallTransparency
					|| !HouseClass::Array->Items[index]->IsAlliedWith_(pOwner);
			}
		}
	}

	return false;
}

bool AresTrajectoryHelper::IsBuildingHit(
	AbstractClass const* const pSource, AbstractClass const* const pTarget,
	CoordStruct const& crdCur, HouseClass const* const pOwner)
{
	auto const pCellBullet = MapClass::Instance->GetCellAt(crdCur);

	if (auto const pBld = pCellBullet->GetBuilding())
	{
		// source building and target buildings are always traversable.
		// this should fix the issue of aircraft being unable to hit buildings
		if (pBld == pSource || pBld == pTarget)
		{
			return false;
		}

		// does the building let allies through?
		auto const isTransparent = RulesExt::Global()->AlliedSolidTransparency
			&& pBld->Owner->IsAlliedWith_(pOwner);

		if (isTransparent)
		{
			return false;
		}

		auto const pBldType = pBld->Type;

		// open gates are not hit: let it pass through
		if (pBldType->Gate && pBld->IsTraversable())
		{
			return false;
		}

		auto const pBldTypeExt = BuildingTypeExt::ExtMap.Find(pBldType);
		if (int solidHeight = pBldTypeExt->Solid_Height)
		{
			if (solidHeight < 0)
			{
				solidHeight = pBldType->Height;
			}

			auto const floor = MapClass::Instance->GetCellFloorHeight(crdCur);
			return crdCur.Z <= floor + solidHeight * 256;
		}
	}

	return false;
}

Vector2D<int> AresTrajectoryHelper::AbsoluteDifference(const CoordStruct& coords)
{
	return{ std::abs(coords.X), std::abs(coords.Y) };
}

Vector2D<int> AresTrajectoryHelper::AbsoluteDifference(const CellStruct& cell)
{
	return{ std::abs(cell.X), std::abs(cell.Y) };
}

CellClass* AresTrajectoryHelper::GetObstacle(
	CellClass const* const pCellSource, CellClass const* const pCellTarget,
	AbstractClass const* const pSource, AbstractClass const* const pTarget,
	CellClass const* const pCellBullet, CoordStruct const& crdCur,
	BulletTypeClass const* const pType, BulletTypeExt::ExtData const* pTypeExt,
	HouseClass const* const pOwner)
{
	auto const cellCur = CellClass::Coord2Cell(crdCur);
	auto const pCellCur = MapClass::Instance->GetCellAt(cellCur);

	auto const IsCliffHit = [=]()
	{
		return pType->SubjectToCliffs
			&& AresTrajectoryHelper::IsCliffHit(pCellSource, pCellBullet, pCellCur);
	};

	auto const IsWallHit = [=]()
	{
		return pType->SubjectToWalls
			&& AresTrajectoryHelper::IsWallHit(pCellSource, pCellCur, pCellTarget, pOwner);
	};

	auto const IsBuildingHit = [=]()
	{
		return pTypeExt->SubjectToSolid
			&& AresTrajectoryHelper::IsBuildingHit(pSource, pTarget, crdCur, pOwner);
	};

	auto const isHit = IsCliffHit() || IsWallHit() || IsBuildingHit();

	return isHit ? pCellCur : nullptr;
}

CellClass* AresTrajectoryHelper::FindFirstObstacle(
	CoordStruct const& crdSrc, CoordStruct const& crdTarget,
	AbstractClass const* const pSource, AbstractClass const* const pTarget,
	BulletTypeClass const* const pType,
	BulletTypeExt::ExtData const* const pTypeExt,
	HouseClass const* const pOwner)
{
	if (AresTrajectoryHelper::SubjectToAnything(pType, pTypeExt))
	{
		auto const cellTarget = CellClass::Coord2Cell(crdTarget);
		auto const pCellTarget = MapClass::Instance->GetCellAt(cellTarget);

		auto const cellSrc = CellClass::Coord2Cell(crdSrc);
		auto const pCellSrc = MapClass::Instance->GetCellAt(cellSrc);

		auto const delta = AbsoluteDifference(cellSrc - cellTarget);
		auto const maxDelta = static_cast<size_t>(MaxImpl(delta.X, delta.Y));

		auto const step = !maxDelta ? CoordStruct::Empty
			: (crdTarget - crdSrc) * (1.0 / maxDelta);

		CoordStruct crdCur = crdSrc;
		auto pCellCur = pCellSrc;
		for (size_t i = 0; i < maxDelta; ++i)
		{
			if (auto const pCell = GetObstacle(pCellSrc, pCellTarget, pSource,
				pTarget, pCellCur, crdCur, pType, pTypeExt, pOwner))
			{
				return pCell;
			}

			pCellCur = MapClass::Instance->GetCellAt(crdCur);
			crdCur += step;
		}
	}

	return nullptr;
}

CellClass* AresTrajectoryHelper::FindFirstImpenetrableObstacle(
	CoordStruct const& crdSrc, CoordStruct const& crdTarget,
	AbstractClass const* const pSource, AbstractClass const* const pTarget,
	WeaponTypeClass const* const pWeapon, HouseClass const* const pOwner)
{
	auto const pProjectile = pWeapon->Projectile;
	auto const pProjectileExt = BulletTypeExt::ExtMap.Find(pProjectile);

	if (auto const pCell = FindFirstObstacle(
		crdSrc, crdTarget, pSource, pTarget, pProjectile, pProjectileExt,
		pOwner))
	{
		if (pCell->ConnectsToOverlay(-1, -1))
		{
			if (pWeapon->Warhead->Wall)
			{
				return nullptr;
			}
		}
		else if (auto const pBld = pCell->GetBuilding())
		{
			// only willingfully fire through enemy buildings
			if (!pOwner->IsAlliedWith_(pBld))
			{
				auto const pBldTypeExt = BuildingTypeExt::ExtMap.Find(pBld->Type);

				// penetrable if warhead level is at least equal to building level
				if (pProjectileExt->Solid_Level >= pBldTypeExt->Solid_Level)
				{
					return nullptr;
				}
			}
		}

		return pCell;
	}

	return nullptr;
}

DEFINE_OVERRIDE_HOOK(0x468BE2, BulletClass_ShouldDetonate_Obstacle, 6)
{
	GET(BulletClass* const, pThis, ESI);
	GET(CoordStruct* const, pOutCoords, EDI);

	auto const pTypeExt = BulletTypeExt::ExtMap.Find(pThis->Type);

	if (AresTrajectoryHelper::SubjectToAnything(pThis->Type, pTypeExt))
	{
		auto const Map = MapClass::Instance();
		auto const pCellSource = Map->GetCellAt(pThis->SourceCoords);
		auto const pCellTarget = Map->GetCellAt(pThis->TargetCoords);
		auto const pCellLast = Map->GetCellAt(pThis->LastMapCoords);

		auto const pOwner = pThis->Owner ? pThis->Owner->Owner : nullptr;

		if (AresTrajectoryHelper::GetObstacle(
			pCellSource, pCellTarget, pThis->Owner, pThis->Target, pCellLast,
			*pOutCoords, pThis->Type, pTypeExt, pOwner))
		{
			return 0x468C76;
		}
	}

	return 0x468C86;
}

DEFINE_OVERRIDE_HOOK_AGAIN(0x6F7631, TechnoClass_IsCloseEnoughToTarget_Obstacle, 6)
DEFINE_OVERRIDE_HOOK(0x6F7511, TechnoClass_IsCloseEnoughToTarget_Obstacle, 6)
{
	GET_BASE(WeaponTypeClass* const, pWeapon, 0x10);
	GET_BASE(AbstractClass* const, pTarget, 0xC);
	GET(CoordStruct const* const, pSource, ESI);
	REF_STACK(CoordStruct const, dest, STACK_OFFS(0x3C, 0x1C));

	auto const pThis = (R->Origin() == 0x6F7631)
		? R->EDI<TechnoClass*>()
		: R->EAX<TechnoClass*>();

	R->EAX(AresTrajectoryHelper::FindFirstImpenetrableObstacle(
		*pSource, dest, pThis, pTarget, pWeapon, pThis->Owner));

	return 0x6F7647;
}

DEFINE_OVERRIDE_HOOK(0x4CC360, TrajectoryHelper_GetObstacle, 5)
{
	GET(CellClass* const, pCellSource, ECX);
	GET(CellClass* const, pCellTarget, EDX);
	GET_STACK(CellClass* const, pCellBullet, 0x4);
	REF_STACK(CoordStruct const, crdCur, 0x8);
	GET_STACK(BulletTypeClass* const, pType, 0x14);
	GET_STACK(HouseClass const* const, pOwner, 0x18);

	const auto pTypeExt = BulletTypeExt::ExtMap.Find(pType);

	const auto ret = AresTrajectoryHelper::GetObstacle(
		pCellSource, pCellTarget, nullptr, nullptr, pCellBullet, crdCur, pType,
		pTypeExt, pOwner);

	R->EAX(ret);
	return 0x4CC671;
}

DEFINE_OVERRIDE_HOOK(0x4CC100, TrajectoryHelper_FindFirstObstacle, 7)
{
	GET(CoordStruct const* const, pSource, ECX);
	GET(CoordStruct const* const, pTarget, EDX);
	GET_STACK(BulletTypeClass* const, pType, 0x4);
	GET_STACK(HouseClass* const, pOwner, 0x8);

	const auto pTypeExt = BulletTypeExt::ExtMap.Find(pType);

	const auto ret = AresTrajectoryHelper::FindFirstObstacle(
		*pSource, *pTarget, nullptr, nullptr, pType, pTypeExt, pOwner);

	R->EAX(ret);
	return 0x4CC30B;
}

DEFINE_OVERRIDE_HOOK(0x4CC310, TrajectoryHelper_FindFirstImpenetrableObstacle, 5)
{
	GET(CoordStruct const* const, pSource, ECX);
	GET(CoordStruct const* const, pTarget, EDX);
	GET_STACK(WeaponTypeClass const* const, pWeapon, 0x4);
	GET_STACK(HouseClass* const, pOwner, 0x8);

	const auto ret = AresTrajectoryHelper::FindFirstImpenetrableObstacle(
		*pSource, *pTarget, nullptr, nullptr, pWeapon, pOwner);

	R->EAX(ret);
	return 0x4CC357;
}

DEFINE_HOOK(0x46837F, BulletClass_DrawSHP_SetAnimPalette, 6)
{
	GET(BulletTypeClass* const, pType, EAX);

	if (!pType)
		return 0x0;

	const auto pTypeExt = BulletTypeExt::ExtMap.Find(pType);

	if (const auto pConvert = pTypeExt->GetBulletConvert()) {
		R->EBX(pConvert);
		return 0x4683D7;
	}

	return 0x0;
}

#include <Ext/Techno/Body.h>

DEFINE_OVERRIDE_HOOK(0x469C4E, BulletClass_DetonateAt_DamageAnimSelected, 5)
{
	enum { Continue = 0x469D06, NukeWarheadExtras = 0x469CAF };

	GET(BulletClass* const, pThis, ESI);
	GET(AnimTypeClass* const, AnimType, EBX);
	LEA_STACK(CoordStruct*, XYZ, 0x64);

	const auto pWarheadExt = WarheadTypeExt::ExtMap.Find(pThis->WH);
	bool createdAnim = false;

	int creationInterval = pWarheadExt->Splashed ?
			pWarheadExt->SplashList_CreationInterval : pWarheadExt->AnimList_CreationInterval;

	int* remainingInterval = &pWarheadExt->RemainingAnimCreationInterval;
	if (creationInterval > 0 && pThis->Owner)
			remainingInterval = &TechnoExt::ExtMap.Find(pThis->Owner)->WHAnimRemainingCreationInterval;

	if (creationInterval < 1 || *remainingInterval <= 0)
	{
		HouseClass* pInvoker = nullptr ;
		HouseClass* pVictim = nullptr;

		if (const TechnoClass* Target = generic_cast<TechnoClass*>(pThis->Target)) {
			pVictim = Target->Owner;
		}

		if (const auto pTech = pThis->Owner) {
			pInvoker = pThis->Owner->GetOwningHouse();

		} else {
			if (auto const pBulletExt = BulletExt::ExtMap.Find(pThis))
				pInvoker = pBulletExt->Owner;
		}

		auto types = make_iterator_single(AnimType);

		if (pWarheadExt->SplashList_CreateAll && pWarheadExt->Splashed)
			types = pWarheadExt->SplashList.GetElements(RulesClass::Instance->SplashList);
		else if (pWarheadExt->AnimList_CreateAll && !pWarheadExt->Splashed)
			types = pWarheadExt->OwnerObject()->AnimList;

			for (auto pType : types)
			{
				if (!pType)
					continue;

					if (auto const pAnim = GameCreate<AnimClass>(pType, XYZ, 0, 1, (AnimFlag)0x2600, -15, false))
					{
						createdAnim = true;

						if (const auto pTech = pThis->Owner) {
							if (auto const pAnimExt = AnimExt::ExtMap.Find(pAnim))
								pAnimExt->Invoker = pTech;
						}

						if (pAnim->Type->MakeInfantry > -1)
						{
							AnimExt::SetAnimOwnerHouseKind(pAnim, pInvoker, pVictim);
						}
						else
						{
							AnimExt::SetAnimOwnerHouseKind(pAnim, pInvoker, pVictim, pInvoker);
						}
					}
			}
	}else
	{
		(*remainingInterval)--;
	}

	return (createdAnim && pWarheadExt->IsNukeWarhead.Get()) ? NukeWarheadExtras : Continue;
}

DEFINE_OVERRIDE_HOOK(0x46670F, BulletClass_Update_PreImpactAnim, 6)
{
	GET(BulletClass*, pThis, EBP);

	const auto pWarheadTypeExt = WarheadTypeExt::ExtMap.Find(pThis->WH);

	if (!pThis->NextAnim)
		return 0x46671D;

	if (pWarheadTypeExt->PreImpact_Moves.Get())
	{
		auto coords = pThis->NextAnim->GetCoords();
		pThis->Location = coords;
		pThis->Target = MapClass::Instance->TryGetCellAt(coords);
	}

	return 0x467FEE;
}

DEFINE_OVERRIDE_HOOK(0x46867F, BulletClass_SetMovement_Parachute, 5)
{
	GET(CoordStruct*, XYZ, EAX);
	GET(BulletClass*, Bullet, ECX);
	//	GET_BASE(VelocityClass *, Trajectory, 0xC);

	R->EBX<BulletClass*>(Bullet);

	const auto pBulletData = BulletTypeExt::ExtMap.Find(Bullet->Type);

	bool result = false;
	if (pBulletData->Parachuted)
	{
		result = Bullet->SpawnParachuted(*XYZ);
		Bullet->IsABomb = true;
	}
	else
	{
		result = Bullet->Unlimbo(*XYZ, DirType::North);
	}

	R->EAX(result);
	return 0x468689;
}

DEFINE_OVERRIDE_HOOK(0x468EB9, BulletClass_Fire_SplitsA, 6)
{
	//GET(BulletClass*, pThis, ESI);
	GET(BulletTypeClass* const, pType, EAX);
	return !BulletTypeExt::ExtMap.Find(pType)->HasSplitBehavior()
		? 0x468EC7u : 0x468FF4u;
}

DEFINE_OVERRIDE_HOOK(0x468FFA, BulletClass_Fire_SplitsB, 6)
{
	GET(BulletTypeClass* const, pType, EAX);
	return BulletTypeExt::ExtMap.Find(pType)->HasSplitBehavior()
		? 0x46909Au : 0x469008u;
}

DEFINE_OVERRIDE_HOOK(0x469EBA, BulletClass_DetonateAt_Splits, 6)
{
	GET(BulletClass*, pThis, ESI);
	BulletExt::ApplyAirburst(pThis);
	return 0x46A290;
}

DEFINE_OVERRIDE_HOOK(0x468000, BulletClass_GetAnimFrame, 6)
{
	GET(BulletClass*, pThis, ECX);

	int frame = 0;
	if (pThis->Type->AnimLow || pThis->Type->AnimHigh)
	{
		frame = pThis->AnimFrame;
	}
	else if (pThis->Type->Rotates())
	{
		DirStruct dir(-pThis->Velocity.Y, pThis->Velocity.X);
		const auto ReverseFacing32 = *reinterpret_cast<int(*)[8]>(0x7F4890);
		const auto facing = ReverseFacing32[(short)dir.GetValue(5)];
		const int length = BulletTypeExt::ExtMap.Find(pThis->Type)->AnimLength.Get();

		if (length > 1)
		{
			frame = facing * length + ((Unsorted::CurrentFrame / pThis->Type->AnimRate) % length);
		}
		else
		{
			frame = facing;
		}
	}

	R->EAX(frame);
	return 0x468088;
}