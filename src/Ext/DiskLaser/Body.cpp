#include "Body.h"

#include <Ext/Techno/Body.h>

// =============================
// load / save

//template <typename T>
//void DiskLaserExt::ExtData::Serialize(T& Stm)
//{
//	Stm
//		.Process(this->Initialized)
//		;
//}

// =============================
// container

//DiskLaserExt::ExtContainer DiskLaserExt::ExtMap;

// =============================
// container hooks

//ASMJIT_PATCH(0x4A7A6A, DiskLaserClass_CTOR, 0x6)
//{
//	GET(DiskLaserClass*, pItem, ESI);
//#
//	DiskLaserExt::ExtMap.Allocate(pItem);
//
//	return 0;
//}
//
//ASMJIT_PATCH_AGAIN(0x4A7B00 , DiskLaserClass_SDDTOR, 0x8)
//ASMJIT_PATCH(0x4A7C90, DiskLaserClass_SDDTOR, 0x8)
//{
//	GET(DiskLaserClass *, pItem, ECX);
//	DiskLaserExt::ExtMap.Remove(pItem);
//	return 0;
//}
//
//ASMJIT_PATCH_AGAIN(0x4A7B90, DiskLaserClass_SaveLoad_Prefix, 0x5)
//ASMJIT_PATCH(0x4A7C10, DiskLaserClass_SaveLoad_Prefix, 0x8)
//{
//	GET_STACK(DiskLaserClass*, pItem, 0x4);
//	GET_STACK(IStream*, pStm, 0x8);
//	DiskLaserExt::ExtMap.PrepareStream(pItem, pStm);
//	return 0;
//}
//
//ASMJIT_PATCH(0x4A7BEE, DiskLaserClass_Load_Suffix, 0x9)
//{
//	GET(DiskLaserClass*, pThis, ESI);
//	SwizzleManagerClass::Instance->Swizzle((void**)&pThis->Weapon);
//	DiskLaserExt::ExtMap.LoadStatic();
//	return 0x438BBB;
//}
//
//ASMJIT_PATCH(0x4A7C1C, DiskLaserClass_Save_Suffix, 0x8)
//{
//	GET(ParasiteClass*, pThis, ECX);
//	GET(IStream*, pStream, EAX);
//	GET(BOOL, bClearDirty, EAX);
//
//	const auto nRes = AbstractClass::_Save(pThis, pStream, bClearDirty);
//
//	if(SUCCEEDED(nRes))
//		DiskLaserExt::ExtMap.SaveStatic();
//
//	R->EAX(nRes);
//	return 0x4A7C24;
//}

//detach

//ASMJIT_PATCH(0x4A7755, DiskLaserClass_Update_ChargedUpSound, 0x6) //B
//{
//	GET(DiskLaserClass* const, pThis, ESI);
//
//	if (pThis && pThis->Owner)
//	{
//		R->ECX(TechnoTypeExtContainer::Instance.Find(pThis->Owner->GetTechnoType())->DiskLaserChargeUp.Get(RulesClass::Instance->DiskLaserChargeUp));
//		return 0x4A7760;
//	}
//
//	return 0x0;
//}

//// Angles = [ Pi/180*int((i*360/16+270)%360) for i in range(0,16)]
//static COMPILETIMEEVAL double CosLUT[DiskLaserClass::DrawCoords.c_size()]
//{
//	0, 0.37460659341591196, 0.7071067811865474, 0.9205048534524403,
//	1, 0.9271838545667874, 0.7071067811865476, 0.3907311284892737,
//	0, -0.37460659341591207, -0.7071067811865475, -0.9205048534524404,
//	-1, -0.9271838545667874, -0.7071067811865477, -0.3907311284892738
//};
//
//static COMPILETIMEEVAL double SinLUT[DiskLaserClass::DrawCoords.c_size()]
//{
//	-1, -0.9271838545667874, -0.7071067811865477, -0.3907311284892739,
//	0, 0.374606593415912, 0.7071067811865476, 0.9205048534524404,
//	1, 0.9271838545667874, 0.7071067811865476, 0.39073112848927377,
//	0, -0.374606593415912, -0.7071067811865475, -0.9205048534524403
//};

//ASMJIT_PATCH(0x4A757B, DiskLaserClass_AI_Circle, 0x6)
//{
//	GET(FakeWeaponTypeClass*, pWeapon, EDX);
//
//	if (WeaponTypeExtData::nOldCircumference != pWeapon->_GetExtData()->DiskLaser_Circumference)
//	{
//
//		const int new_Circumference = pWeapon->_GetExtData()->DiskLaser_Circumference;
//		WeaponTypeExtData::nOldCircumference = new_Circumference;
//
//		for (size_t i = 0u; i < DiskLaserClass::DrawCoords.c_size(); i++)
//		{
//			DiskLaserClass::DrawCoords[i].X = (int)(new_Circumference * WeaponTypeExtData::cosLUT[i]);
//			DiskLaserClass::DrawCoords[i].Y = (int)(new_Circumference * WeaponTypeExtData::sinLUT[i]);
//		}
//	}
//
//	return 0;
//}

//ASMJIT_PATCH(0x4A76ED, DiskLaserClass_Update_Anim, 7)
//{
//	GET(DiskLaserClass* const, pThis, ESI);
//	REF_STACK(CoordStruct, coords, STACK_OFFS(0x54, 0x1C));
//
//	auto const pWarhead = pThis->Weapon->Warhead;
//
//	if (RulesExtData::Instance()->DiskLaserAnimEnabled)
//	{
//		auto const pType = MapClass::SelectDamageAnimation(
//			pThis->Damage, pWarhead, LandType::Clear, coords);
//
//		if (pType)
//		{
//			AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pType, coords),
//				pThis->Owner ? pThis->Owner->Owner : nullptr,
//				pThis->Target ? pThis->Target->Owner : nullptr,
//				pThis->Owner, false, false
//			);
//		}
//	}
//
//	MapClass::FlashbangWarheadAt(pThis->Damage, pWarhead, coords);
//
//	return 0;
//}

#include <TechnoClass.h>
#include <BuildingClass.h>
#include <BuildingTypeClass.h>
#include <ColorStruct.h>

#include <Ext/Anim/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/TechnoType/Body.h>

#include <Misc/DamageArea.h>

static Point2D DiscLaserCoords[16] = {
	{    0, -240 },  // 0  - Top
	{   89, -222 },  // 1
	{  169, -169 },  // 2
	{  220,  -93 },  // 3
	{  240,    0 },  // 4  - Right
	{  222,   89 },  // 5
	{  169,  169 },  // 6
	{   93,  220 },  // 7
	{    0,  240 },  // 8  - Bottom
	{  -89,  222 },  // 9
	{ -169,  169 },  // 10
	{ -220,   93 },  // 11
	{ -240,    0 },  // 12 - Left
	{ -222,  -89 },  // 13
	{ -137, -196 },  // 14
	{  -54, -233 },  // 15
};

void FakeDiskLaserClass::__AI()
{ 
	int const state = this->DrawRateCounter;

	// State < 0: Destroy
	if (state < 0) {
		this->DrawRateCounter = -1;
		AbstractClass::Array2->erase(this);
		return;
	}

	// State > 0: Countdown
	if (state > 0) {
		this->DrawRateCounter = state - 1;
		return;
	}

	// State == 0: Main firing logic
	auto pFirer = this->Owner;
	auto pTarget = this->Target;
	auto pWeapon = this->Weapon;

	auto pTypeExt = GET_TECHNOTYPEEXT(pFirer);

	// Get firer center coordinates
	CoordStruct firerCoords = pFirer->GetCoords();

	// If firer is in air, use target's Z
	if (pFirer->IsInAir()) {
		firerCoords.Z = pTarget->GetCoords().Z;
	}

	// Calculate distance to target
	CoordStruct const targetCoords = pTarget->GetCoords();
	CoordStruct const delta {
		firerCoords.X - targetCoords.X,
		firerCoords.Y - targetCoords.Y,
		firerCoords.Z - targetCoords.Z
	};

	int range = static_cast<int>(delta.Length());

	// Adjust range for buildings
	if (auto pBuilding = cast_to<BuildingClass*, false>(pTarget)) {
		int const height = pBuilding->Type->GetFoundationHeight(false);
		int const width = pBuilding->Type->GetFoundationWidth();
		range -= (height + width) * 64;

		if (range < 0)
			range = 0;
	}

	// Out of range - destroy
	if (range > pWeapon->Range) {
		this->DrawRateCounter = -1;
		AbstractClass::Array2->erase(this);
		return;
	}

	// Firer crashing - destroy
	if (pFirer->IsCrashing) {
		this->DrawRateCounter = -1;
		AbstractClass::Array2->erase(this);
		return;
	}

	// Get weapon FLH position
	CoordStruct flhCoords;
	pFirer->GetFLH(&flhCoords, 0, CoordStruct::Empty);
	int const z = flhCoords.Z;

	// Calculate rotating indices
	int const offset38 = this->DrawCounter;
	int const offset34 = this->Facing;

	int const idx0 = (offset34 + offset38) % std::size(DiscLaserCoords);      // Current position
	int const idx1 = (offset34 - offset38 + 16) % std::size(DiscLaserCoords); // Opposite position
	int const idx2 = (offset34 + offset38 + 1) % std::size(DiscLaserCoords);  // Next position
	int const idx3 = (offset34 - offset38 + 15) % std::size(DiscLaserCoords); // Opposite next

	// ============================================================
	// HOOK: 0x4A757B - DiskLaserClass_AI_Circle
	// Update circle coords if weapon has custom circumference
	// ============================================================
	{
		auto const pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);
		int const newCircumference = pWeaponExt->DiskLaser_Circumference;

		if (WeaponTypeExtData::nOldCircumference != newCircumference) {
			WeaponTypeExtData::nOldCircumference = newCircumference;

			for (size_t i = 0u; i < std::size(DiscLaserCoords); ++i) {
				DiscLaserCoords[i].X = static_cast<int>(newCircumference * WeaponTypeExtData::cosLUT[i]);
				DiscLaserCoords[i].Y = static_cast<int>(newCircumference * WeaponTypeExtData::sinLUT[i]);
			}
		}
	}
	// ============================================================

	// Determine colors
	ColorStruct innerColor;
	ColorStruct outerColor;
	ColorStruct* pOuterSpread = &pWeapon->LaserOuterSpread;

	if (pWeapon->IsHouseColor) {
		innerColor = pFirer->Owner->LaserColor;
		outerColor.R = pFirer->Owner->LaserColor.R >> 1;
		outerColor.G = pFirer->Owner->LaserColor.G >> 1;
		outerColor.B = pFirer->Owner->LaserColor.B >> 1;
	} else {
		innerColor = pWeapon->LaserInnerColor;
		outerColor = pWeapon->LaserOuterColor;
	}

	// Final convergence frame?
	bool const isFinalFrame = (idx0 == idx1) && (offset38 != 0);

	if (isFinalFrame) {
		// === FINAL FRAME: Fire at target ===

		CoordStruct const laserStart {
			flhCoords.X + DiscLaserCoords[idx0].X,
			flhCoords.Y + DiscLaserCoords[idx0].Y,
			z
		};

		// Get target coords
		CoordStruct laserEnd;
		if (auto pTargetObject = flag_cast_to<ObjectClass*, false>(pTarget)) {
			laserEnd = pTargetObject->GetTargetCoords();
		} else {
			laserEnd = pTarget->GetCenterCoords();
		}

		// Create final laser beam
		GameCreate<LaserDrawClass>(
			laserStart,
			laserEnd,
			innerColor,
			outerColor,
			*pOuterSpread,
			pWeapon->LaserDuration
		);

		if(!pTypeExt->DiskLaserDetonate){
			// Deal damage at target
			DamageArea::Apply(
				&laserEnd,
				this->Damage,
				pFirer,
				pWeapon->Warhead,
				true,
				nullptr
			);
		} else {
			WeaponTypeExtData::DetonateAt2(this->Weapon, pTarget, pFirer, this->Damage, true, pFirer->Owner);
		}

		// ============================================================
		// HOOK: 0x4A76ED - DiskLaserClass_Update_Anim
		// Create damage animation and flashbang effect
		// ============================================================
		{
			auto const pWarhead = pWeapon->Warhead;

			if (RulesExtData::Instance()->DiskLaserAnimEnabled) {
				auto const pAnimType = MapClass::SelectDamageAnimation(
					this->Damage,
					pWarhead,
					LandType::Clear,
					laserEnd
				);

				if (pAnimType) {
					AnimExtData::SetAnimOwnerHouseKind(
						GameCreate<AnimClass>(pAnimType, laserEnd),
						pFirer ? pFirer->Owner : nullptr,
						pTarget ? pTarget->GetOwningHouse() : nullptr,
						pFirer,
						false,
						false
					);
				}
			}

			MapClass::FlashbangWarheadAt(this->Damage, pWarhead, laserEnd);
		}
		// ============================================================

		// Play weapon sound
		auto const& sound = pWeapon->Report;
		if (sound.Count > 0) {
			VocClass::SafeImmedietelyPlayAt(sound[pFirer->weapon_sound_randomnumber_3C8 % sound.Count], &laserStart, nullptr);
		}

		this->DrawRateCounter = -1;
	} else {
		// === CHARGING PHASE: Draw rotating laser pairs ===

		int const duration = 8 - offset38;

		// First pair: idx0 -> idx2
		CoordStruct start1 {
			flhCoords.X + DiscLaserCoords[idx0].X,
			flhCoords.Y + DiscLaserCoords[idx0].Y,
			z
		};

		CoordStruct end1 {
			flhCoords.X + DiscLaserCoords[idx2].X,
			flhCoords.Y + DiscLaserCoords[idx2].Y,
			z
		};

		// ============================================================
		// HOOK: 0x4A7755 - DiskLaserClass_Update_ChargedUpSound
		// Play charge-up sound on first frame (with TechnoType extension)
		// ============================================================
		if (offset38 == 0) {
			VocClass::SafeImmedietelyPlayAt(pTypeExt->DiskLaserChargeUp.Get(RulesClass::Instance->DiskLaserChargeUp), &start1, nullptr);
		}
		// ============================================================

		// First laser
		GameCreate<LaserDrawClass>(
			start1,
			end1,
			innerColor,
			outerColor,
			*pOuterSpread,
			duration
		);

		// Second pair: idx1 -> idx3
		CoordStruct const start2 {
			flhCoords.X + DiscLaserCoords[idx1].X,
			flhCoords.Y + DiscLaserCoords[idx1].Y,
			z
		};

		CoordStruct const end2 {
			flhCoords.X + DiscLaserCoords[idx3].X,
			flhCoords.Y + DiscLaserCoords[idx3].Y,
			z
		};

		// Second laser
		GameCreate<LaserDrawClass>(
			start2,
			end2,
			innerColor,
			outerColor,
			*pOuterSpread,
			duration
		);

		// Advance animation state
		this->DrawRateCounter = 1;
		this->DrawCounter = offset38 + 1;
	}
}

void FakeDiskLaserClass::__Fire(TechnoClass* pFirer, AbstractClass* pTarget, WeaponTypeClass* pWeapon, int damageMultiplier)
{
	// Validate all required parameters
	if (!pFirer) {
		this->DrawRateCounter = -1;
		return;
	}

	if (!pTarget) {
		this->DrawRateCounter = -1;
		return;
	}

	if (!pWeapon) {
		this->DrawRateCounter = -1;
		return;
	}

	// Initialize disk laser state
	this->Owner = pFirer;
	this->Damage = damageMultiplier;
	this->Target = pTarget;
	this->Weapon = pWeapon;

	// Get center coordinates of target and firer
	CoordStruct const targetCoords = pTarget->GetCoords();
	CoordStruct const firerCoords = pFirer->GetCoords();

	// Calculate angle from firer to target
	// Atan2(firerY - targetY, targetX - firerX)
	double const angle = Math::atan2(
		static_cast<double>(firerCoords.Y - targetCoords.Y),
		static_cast<double>(targetCoords.X - firerCoords.X)
	);

	// Convert angle to binary angle format
	// Subtract 90 degrees and convert using BINARY_ANGLE_MAGIC
	double const adjustedAngle = angle - Math::DEG90_AS_RAD;
	int const binaryAngle = static_cast<int>(adjustedAngle * Math::BINARY_ANGLE_MAGIC);

	// Calculate starting offset for laser animation
	// Extract lower 16 bits, then calculate index
	unsigned short const angleWord = static_cast<unsigned short>(binaryAngle);
	const int offset = (((((angleWord >> 11) + 1) >> 1) & 0xF) + 8);

	// Use signed mod 16 for final value
	this->Facing = offset % std::size(DiscLaserCoords);
	this->DrawRateCounter = 0;
	this->DrawCounter = 0;

	// Add to the array for AI processing
	AbstractClass::Array2->push_back(this);
}

DEFINE_FUNCTION_JUMP(LJMP, 0x4A71A0, FakeDiskLaserClass::__Fire)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E6014, FakeDiskLaserClass::__AI)