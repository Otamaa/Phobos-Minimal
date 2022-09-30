#include "Body.h"

#include <WeaponTypeClass.h>

/*
DEFINE_HOOK(0x762BA7, WaveClass_Update_CheckInRange, 0x5)
{
	GET(WaveClass* const, pThis, ESI);
	
	bool EligibleDistance = pThis->Owner->DistanceFrom(pThis->Target) <= WP_Distance;
	if (!EligibleDistance)
		pThis->field_12C = static_cast<std::make_unsigned<char>::type>(256);
	
	return 0x762C4E;
}*/

/**
 *  Patch in the new Generate_Tables.
 *
 *  @author: CCHyper
 */
DECLARE_PATCH(_WaveClass_Setup_Wave_Size)
{
	GET_REGISTER_STATIC(WaveClass *, this_ptr, edi);

	do_vectors(this_ptr);

	JMP(0x00672414);
}

/**
 *  These patches allow us to store the firing weapon type pointer
 *  to allow us to fetch custom overrides. Nasty hack...
 *
 *  @author: CCHyper
 */
DECLARE_PATCH(_TechnoClass_Laser_Zap_Store_WeaponType_Ptr)
{
	GET_REGISTER_STATIC(TechnoClass *, this_ptr, edi);
	GET_REGISTER_STATIC(WeaponTypeClass *, weapontype, esi);

	Wave_TempWeaponTypePtr = weapontype;

	/**
	 *  Stolen bytes/code.
	 */
	_asm { mov dl, [esi + 0x0E3] } // WeaponTypeClass.IsBigLaser

	JMP_REG(ecx, 0x006301DE);
}

DECLARE_PATCH(_TechnoClass_Fire_At_Store_WeaponType_Ptr)
{
	GET_REGISTER_STATIC(TechnoClass *, this_ptr, esi);
	GET_REGISTER_STATIC(WeaponTypeClass *, weapontype, ebx);

	Wave_TempWeaponTypePtr = weapontype;

	/**
	 *  Stolen bytes/code.
	 */
	_asm { mov ecx, [ebp + 8] }
	_asm { mov eax, [esi] }

	JMP_REG(edx, 0x006311B5);
}
