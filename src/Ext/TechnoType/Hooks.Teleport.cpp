#include "Body.h"

#include <Ext/Anim/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>

#include <Locomotor/Cast.h>

#define GET_LOCO(reg_Loco) \
	GET(ILocomotion*, Loco, reg_Loco); \
	TeleportLocomotionClass* pLocomotor = static_cast<TeleportLocomotionClass*>(Loco); \
	TechnoClass* pOwner =  pLocomotor->LinkedTo ? pLocomotor->LinkedTo : pLocomotor->Owner; \
	TechnoTypeClass* pType = pOwner->GetTechnoType(); \
	TechnoTypeExtData *pExt = TechnoTypeExtContainer::Instance.Find(pType);

DEFINE_HOOK(0x7193F6, TeleportLocomotionClass_ILocomotion_Process_WarpoutAnim, 0x6)
{
	GET_LOCO(ESI);

	TechnoExtData::PlayAnim(pExt->WarpOut.GetOrDefault(pOwner , RulesClass::Instance->WarpOut), pOwner);

	if (const auto pWeapon = pExt->WarpOutWeapon.Get(pOwner))
		WeaponTypeExtData::DetonateAt(pWeapon, pOwner, pOwner , true , nullptr);

	return 0x719447;
}

DEFINE_HOOK(0x719742, TeleportLocomotionClass_ILocomotion_Process_WarpInAnim, 0x6)
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

		WeaponTypeExtData::DetonateAt(pWeapon, pOwner, pOwner, damage, true, nullptr);
	}

	return 0x719796;
}

DEFINE_HOOK(0x719827, TeleportLocomotionClass_ILocomotion_Process_WarpAway, 0x6)
{
	GET_LOCO(ESI);

	TechnoExtData::PlayAnim(pExt->WarpAway.GetOrDefault(pOwner , RulesClass::Instance->WarpOut), pOwner);
	return 0x719878;
}

DEFINE_HOOK(0x7194D0, TeleportLocomotionClass_ILocomotion_Process_ChronoTrigger, 0x6)
{
	GET_LOCO(ESI);
	GET(RulesClass*, pRules, EBX);
	GET(int, val, EDX);
	enum { SetTimer = 0x7194E9, CheckTheTimer = 0x7194FD };

	if (pExt->ChronoTrigger.GetOrDefault(pOwner, pRules->ChronoTrigger)) {

		R->ECX(Unsorted::CurrentFrame());

		const auto nDecided = pExt->ChronoDistanceFactor.GetOrDefault(pOwner, pRules->ChronoDistanceFactor);
		// fix factor 0 crash by force it to 1 (Vanilla bug)
		R->EAX(val / MaxImpl(nDecided, 1));
		return SetTimer;
	}

	return CheckTheTimer;
}

DEFINE_HOOK(0x719519, TeleportLocomotionClass_ILocomotion_Process_ChronoMinimumDelay, 0x6)
{
	GET_LOCO(ESI);
	GET(RulesClass*, pRules, EBX);

	R->EBX(pExt->ChronoMinimumDelay.GetOrDefault(pOwner, pRules->ChronoMinimumDelay));

	return 0x71951F;
}

DEFINE_HOOK(0x719555, TeleportLocomotionClass_ILocomotion_Process_ChronoRangeMinimum, 0x6)
{
	enum { SetTimer = 0x719568, SetWarpingOut = 0x719576 };

	GET_LOCO(ESI);
	GET(RulesClass*, pRules, ECX);
	GET(int, comparator, EDX);

	TechnoExtContainer::Instance.Find(pOwner)->LastWarpDistance = comparator;
	const auto factor = pExt->ChronoRangeMinimum.GetOrDefault(pOwner, pRules->ChronoRangeMinimum);

	if(comparator < factor) {
		R->EAX(Unsorted::CurrentFrame());
		R->ECX(pExt->ChronoMinimumDelay.GetOrDefault(pOwner, pRules->ChronoMinimumDelay));
		return SetTimer;
	}

	return SetWarpingOut;
}

DEFINE_HOOK(0x71997B, TeleportLocomotionClass_ILocomotion_Process_ChronoDelay, 0x6)
{
	GET_LOCO(ESI);
	GET(RulesClass*, pRules, EAX);

	R->ECX(pExt->ChronoDelay.GetOrDefault(pOwner, pRules->ChronoDelay));

	return 0x719981;
}

//FORCEINLINE std::pair<Matrix3D, Matrix3D> SimplifiedTiltingConsideration(float arf, float ars, TechnoTypeClass* linkedType)
//{
//	double scalex = linkedType->VoxelScaleX;
//	double scaley = linkedType->VoxelScaleY;
//
//	Matrix3D pre = Matrix3D::GetIdentity();
//	pre.TranslateZ(float(std::abs(Math::sin(ars)) * scalex + std::abs(Math::sin(arf)) * scaley));
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

struct LocomotionClass_Clone : public LocomotionClass {
	static Matrix3D* __stdcall __Draw_Matrix_(LocomotionClass* pLoco , Matrix3D* ret, VoxelIndexKey* pIndex) {
		JMP_STD(0x55A730);
	}
};
static_assert(sizeof(LocomotionClass_Clone) == sizeof(LocomotionClass), "Invalid size!");

// Author : chaserli
Matrix3D* __stdcall LocomotionClass_Draw_Matrix(ILocomotion* pThis, Matrix3D* ret, VoxelIndexKey* pIndex)
{
	auto loco = static_cast<LocomotionClass*>(pThis);
	auto slope_idx = MapClass::Instance->GetCellAt(loco->LinkedTo->Location)->SlopeIndex;

	//if (!pIndex) {
	//	return LocomotionClass_Clone::__Draw_Matrix_(loco , ret, pIndex);
	//}

	if (pIndex && pIndex->Is_Valid_Key())
		*(int*)(pIndex) = slope_idx + (*(int*)(pIndex) << 6);

	if (slope_idx && pIndex && pIndex->Is_Valid_Key()){
		loco->LocomotionClass::Draw_Matrix(ret,pIndex);
		*ret = Game::VoxelRampMatrix[slope_idx] * (*ret);
	} else {
		loco->LocomotionClass::Draw_Matrix(ret,pIndex);
	}

	float arf = loco->LinkedTo->AngleRotatedForwards;
	float ars = loco->LinkedTo->AngleRotatedSideways;

	if (std::abs(ars) >= 0.005 || std::abs(arf) >= 0.005)
	{
		//just forget about ramp here, math too complicated, not considered for other locos either
		if (pIndex)
			*(int*)pIndex = -1;

		double scalex = loco->LinkedTo->GetTechnoType()->VoxelScaleX;
		double scaley = loco->LinkedTo->GetTechnoType()->VoxelScaleY;

		Matrix3D pre = Matrix3D::GetIdentity();
		pre.TranslateZ(float(std::abs(Math::sin(ars)) * scalex + std::abs(Math::sin(arf)) * scaley));
		ret->TranslateX(float(Math::signum(arf) * (scaley * (1 - Math::cos(arf)))));
		ret->TranslateY(float(Math::signum(-ars) * (scalex * (1 - Math::cos(ars)))));
		ret->RotateX(ars);
		ret->RotateY(arf);

		*ret = pre * *ret;
		return ret;
	}

	return ret;
}
DEFINE_JUMP(VTABLE, 0x7F5024, GET_OFFSET(LocomotionClass_Draw_Matrix));
DEFINE_JUMP(VTABLE, 0x7F5028, 0x5142A0);//TeleportLocomotionClass_Shadow_Matrix : just use hover's to save my ass

DEFINE_HOOK(0x729B5D, TunnelLocomotionClass_DrawMatrix_Tilt, 0x8)
{
	GET(ILocomotion*, iloco, ESI);
	GET_BASE(VoxelIndexKey*, pIndex, 0x10);
	GET_BASE(Matrix3D*, ret, 0xC);
	R->EAX(LocomotionClass_Draw_Matrix(iloco, ret, pIndex));
	return 0x729C09;
}
DEFINE_JUMP(VTABLE, 0x7F5A4C, 0x5142A0);//TunnelLocomotionClass_Shadow_Matrix : just use hover's to save my ass
#endif
#undef GET_LOCO