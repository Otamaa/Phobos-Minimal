#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>

DEFINE_OVERRIDE_HOOK(0x6FD64A, TechnoClass_FireRadBeam1, 6)
{
	BYTE idxWeapon = *reinterpret_cast<BYTE*>(R->Stack32(0x18) + 0xC); // hack! 0x18 fetches the caller's EBP, which gives us access to its locals, including idxWeapon
	GET_STACK(TechnoClass*, Techno, 0x14);

	// get the default color
	GET(TechnoClass*, SourceUnit, EDI);
	GET(RadBeam*, Rad, ESI);

	if (SourceUnit && SourceUnit->Owner)
	{
		Rad->Color = SourceUnit->Owner->Color;
	}
	else
	{
		Rad->Color = RulesClass::Instance->RadColor;
	}

	TechnoExtContainer::Instance.Find(Techno)->idxSlot_Beam = idxWeapon;

	R->Stack<int>(0x0, idxWeapon);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x6FD79C, TechnoClass_FireRadBeam2, 6)
{
	GET(RadBeam*, Rad, ESI);
	GET_STACK(WeaponTypeClass*, pSource, 0xC);

	WeaponTypeExtData* pData = WeaponTypeExtContainer::Instance.Find(pSource);

	if (!pData->Beam_IsHouseColor) {
		Rad->Color = pData->GetBeamColor();
	}

	Rad->Period = pData->Beam_Duration;
	Rad->Amplitude = pData->Beam_Amplitude;
	return 0x6FD7A8;
}
