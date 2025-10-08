#include "Body.h"

#include <Ext/Anim/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>

#include <Locomotor/Cast.h>

#include <Utilities/Macro.h>

#include <TacticalClass.h>

#define GET_LOCO(reg_Loco) \
	GET(ILocomotion*, Loco, reg_Loco); \
	TeleportLocomotionClass* pLocomotor = static_cast<TeleportLocomotionClass*>(Loco); \
	TechnoClass* pOwner =  pLocomotor->LinkedTo ? pLocomotor->LinkedTo : pLocomotor->Owner; \
	TechnoTypeClass* pType = pOwner->GetTechnoType(); \
	TechnoTypeExtData *pExt = TechnoTypeExtContainer::Instance.Find(pType);

ASMJIT_PATCH(0x7197DF, TeleportLocomotionClass_Process_ChronospherePreDelay, 0x6)
{
	//GET(TeleportLocomotionClass*, pThis, ESI);
	GET(FootClass*, pLinked, ECX);

	auto pTypeExtData = TechnoTypeExtContainer::Instance.Find(pLinked->GetTechnoType());
	R->ECX(pTypeExtData->ChronoSpherePreDelay.Get(RulesExtData::Instance()->ChronoSpherePreDelay));
	return 0x7197E4;
}

ASMJIT_PATCH(0x719BD9, TeleportLocomotionClass_Process_ChronosphereDelay2, 0x6)
{
	GET(TeleportLocomotionClass*, pThis, ESI);

	auto const pExt = TechnoExtContainer::Instance.Find(pThis->Owner);

	if (!pExt->IsBeingChronoSphered)
		return 0;

	auto pTypeExtData = TechnoTypeExtContainer::Instance.Find(pExt->Type);
	int delay = pTypeExtData->ChronoSphereDelay.Get(RulesExtData::Instance()->ChronoSphereDelay);

	if (delay > 0)
	{
		pThis->Owner->WarpingOut = true;
		pExt->HasRemainingWarpInDelay = true;
		pExt->LastWarpInDelay = std::max(delay, pExt->LastWarpInDelay);
	}
	else
	{
		pExt->IsBeingChronoSphered = false;
	}

	return 0;
}

ASMJIT_PATCH(0x7193F6, TeleportLocomotionClass_ILocomotion_Process_WarpoutAnim, 0x6)
{
	GET_LOCO(ESI);

	TechnoExtData::PlayAnim(pExt->WarpOut.GetOrDefault(pOwner , RulesClass::Instance->WarpOut), pOwner);

	if (const auto pWeapon = pExt->WarpOutWeapon.Get(pOwner))
		WeaponTypeExtData::DetonateAt1(pWeapon, pOwner, pOwner , true , nullptr);

	const int distance = (int)Math::sqrt(pOwner->Location.DistanceFromSquared(pLocomotor->LastCoords));
	TechnoExtContainer::Instance.Find(pOwner)->LastWarpDistance = distance;

	if (auto pImage = pType->AlphaImage) {
		auto xy = TacticalClass::Instance->CoordsToClient(pOwner->Location);
		RectangleStruct Dirty = { xy.X - (pImage->Width / 2) , xy.Y - (pImage->Height / 2),
		  pImage->Width, pImage->Height };
		TacticalClass::Instance->RegisterDirtyArea(Dirty, true);
	}

	int duree = pExt->ChronoMinimumDelay.GetOrDefault(pOwner, RulesClass::Instance->ChronoMinimumDelay);
	const auto factor = pExt->ChronoRangeMinimum.GetOrDefault(pOwner, RulesClass::Instance->ChronoRangeMinimum);

	if (distance >= factor
		&& pExt->ChronoTrigger.GetOrDefault(pOwner, RulesClass::Instance->ChronoTrigger))
	{
		const auto f_factor = pExt->ChronoDistanceFactor.GetOrDefault(pOwner, RulesClass::Instance->ChronoDistanceFactor);
		duree = MaxImpl(distance / MaxImpl(f_factor, 1), duree);

	}

	pLocomotor->Timer.Start(duree);
	pOwner->WarpingOut = true;

	if (auto pUnit = cast_to<UnitClass*, false>(pOwner)) {
		if (pUnit->Type->Harvester || pUnit->Type->Weeder) {
			pLocomotor->Timer.Start(0);
			pUnit->WarpingOut = false;
		}
	}

	auto const pLinkedExt = TechnoExtContainer::Instance.Find(pOwner);
	pLinkedExt->LastWarpInDelay = std::max(pLocomotor->Timer.GetTimeLeft(), pLinkedExt->LastWarpInDelay);
	return 0x7195BC;
}

ASMJIT_PATCH(0x719742, TeleportLocomotionClass_ILocomotion_Process_WarpInAnim, 0x6)
{
	GET_LOCO(ESI);

	//WarpIn is unused , maybe a type on WW side
	TechnoExtData::PlayAnim(pExt->WarpIn.GetOrDefault(pOwner ,RulesClass::Instance->WarpOut), pOwner);

	const auto pTechnoExt = TechnoExtContainer::Instance.Find(pOwner);

	const auto Rank = pOwner->CurrentRanking;
	const auto pWarpInWeapon = pExt->WarpInWeapon.GetFromSpecificRank(Rank);

	const auto pWeapon = pTechnoExt->LastWarpDistance < pExt->ChronoRangeMinimum.GetOrDefault(pOwner ,RulesClass::Instance->ChronoRangeMinimum)
		? pExt->WarpInMinRangeWeapon.GetFromSpecificRank(Rank)->Get(pWarpInWeapon) : pWarpInWeapon;

	if (pWeapon) {
		const int damage = pExt->WarpInWeapon_UseDistanceAsDamage.Get(pOwner) ?
			(pTechnoExt->LastWarpDistance / Unsorted::LeptonsPerCell) : pWeapon->Damage;

		WeaponTypeExtData::DetonateAt2(pWeapon, pOwner, pOwner, damage, true, nullptr);
	}

	return 0x719796;
}

ASMJIT_PATCH(0x719827, TeleportLocomotionClass_ILocomotion_Process_WarpAway, 0x6)
{
	GET_LOCO(ESI);

	TechnoExtData::PlayAnim(pExt->WarpAway.GetOrDefault(pOwner , RulesClass::Instance->WarpOut), pOwner);
	return 0x719878;
}

ASMJIT_PATCH(0x71997B, TeleportLocomotionClass_ILocomotion_Process_ChronoDelay, 0x6)
{
	GET_LOCO(ESI);
	GET(RulesClass*, pRules, EAX);

	R->ECX(pExt->ChronoDelay.GetOrDefault(pOwner, pRules->ChronoDelay));

	return 0x719981;
}

//FORCEDINLINE std::pair<Matrix3D, Matrix3D> SimplifiedTiltingConsideration(float arf, float ars, TechnoTypeClass* linkedType)
//{
//	double scalex = linkedType->VoxelScaleX;
//	double scaley = linkedType->VoxelScaleY;
//
//	Matrix3D pre = Matrix3D::GetIdentity();
//	pre.TranslateZ(float(Math::abs(Math::sin(ars)) * scalex + Math::abs(Math::sin(arf)) * scaley));
//
//	Matrix3D post = Matrix3D::GetIdentity();
//	post.TranslateX(float(Math::signum(arf) * (scaley * (1 - Math::cos(arf)))));
//	post.TranslateY(float(Math::signum(-ars) * (scalex * (1 - Math::cos(ars)))));
//	post.RotateX(ars);
//	post.RotateY(arf);
//
//	return { pre,post };
//}

#ifndef FUCKTHESE

// Author : chaserli
Matrix3D* __stdcall LocomotionClass_Draw_Matrix(ILocomotion* pThis, Matrix3D* ret, VoxelIndexKey* pIndex)
{
	auto loco = static_cast<LocomotionClass*>(pThis);
	auto slope_idx = MapClass::Instance->GetCellAt(loco->Owner->Location)->SlopeIndex;

	if (pIndex && pIndex->Is_Valid_Key())
		*(int*)(pIndex) = slope_idx + (*(int*)(pIndex) << 6);

	Matrix3D _DrawMtx {};
	loco->LocomotionClass::Draw_Matrix(&_DrawMtx,pIndex);
	*ret = Game::VoxelRampMatrix[slope_idx] * _DrawMtx;

	float arf = loco->Owner->AngleRotatedForwards;
	float ars = loco->Owner->AngleRotatedSideways;

	if (Math::abs(ars) >= 0.005 || Math::abs(arf) >= 0.005)
	{
		//just forget about ramp here, math too complicated, not considered for other locos either
		if (pIndex)
			*(int*)pIndex = -1;

		double scalex = loco->Owner->GetTechnoType()->VoxelScaleX;
		double scaley = loco->Owner->GetTechnoType()->VoxelScaleY;

		Matrix3D pre = Matrix3D::GetIdentity();
		pre.TranslateZ(float(Math::abs(Math::sin(ars)) * scalex + Math::abs(Math::sin(arf)) * scaley));
		ret->TranslateX(float(Math::signum(arf) * (scaley * (1 - Math::cos(arf)))));
		ret->TranslateY(float(Math::signum(-ars) * (scalex * (1 - Math::cos(ars)))));
		ret->RotateX(ars);
		ret->RotateY(arf);

		*ret = pre * *ret;
		return ret;
	}

	return ret;
}

//DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5028, 0x5142A0);//TeleportLocomotionClass_Shadow_Matrix : just use hover's to save my ass

DEFINE_FUNCTION_JUMP(VTABLE , 0x7F5024, LocomotionClass_Draw_Matrix)

ASMJIT_PATCH(0x729B5D, TunnelLocomotionClass_DrawMatrix_Tilt, 0x8)
{
	GET(ILocomotion*, iloco, ESI);
	GET_BASE(VoxelIndexKey*, pIndex, 0x10);
	GET_BASE(Matrix3D*, ret, 0xC);
	R->EAX(LocomotionClass_Draw_Matrix(iloco, ret, pIndex));
	return 0x729C09;
}
//DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5A4C, 0x5142A0);//TunnelLocomotionClass_Shadow_Matrix : just use hover's to save my ass
#endif
#undef GET_LOCO