#include <Ext/Wave/Body.h>

#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>

#include <New/Type/ArmorTypeClass.h>

#include <Notifications.h>
#include <algorithm>

DEFINE_DISABLE_HOOK(0x763226, WaveClass_DTOR_ares)

namespace AresCreateWave
{
	WaveColorData TempColor;
}

DEFINE_OVERRIDE_HOOK(0x6FF449, TechnoClass_Fire_SonicWave, 5)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(WeaponTypeClass* const, pSource, EBX);

	GET_BASE(AbstractClass*, pTarget, 0x8);

	REF_STACK(CoordStruct const, crdSrc, 0x44);
	REF_STACK(CoordStruct const, crdTgt, 0x88);

	pThis->Wave = WaveExtData::Create(crdSrc, crdTgt, pThis, WaveType::Sonic, pTarget, pSource);
	return 0x6FF48A;
}

DEFINE_OVERRIDE_HOOK(0x6FF5F5, TechnoClass_Fire_OtherWaves, 6)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(WeaponTypeClass* const, pSource, EBX);
	GET(TechnoClass* const, pTarget, EDI);

	REF_STACK(CoordStruct const, crdSrc, 0x44);
	REF_STACK(CoordStruct const, crdTgt, 0x88);

	auto const pData = WeaponTypeExtContainer::Instance.Find(pSource);

	if (!pData->IsWave() || pThis->Wave)
		return 0x6FF656;

	WaveType nType = WaveType::Sonic;
	if (pSource->IsMagBeam)
		nType = WaveType::Magnetron;
	else
		nType = pData->Wave_IsBigLaser
		? WaveType::BigLaser : WaveType::Laser;

	pThis->Wave = WaveExtData::Create(crdSrc, crdTgt, pThis, nType, pTarget, pSource);
	return 0x6FF656;
}

DEFINE_OVERRIDE_HOOK(0x75FA29, WaveClass_Draw_Colors, 0x6)
{
	GET(WaveClass*, pThis, ESI);
	AresCreateWave::TempColor = WaveExtData::GetWaveColor(pThis);
	return 0x0;
}

DEFINE_OVERRIDE_HOOK(0x760F50, WaveClass_Update, 0x6)
{
	GET(WaveClass*, pThis, ECX);

	const auto pData = WaveExtContainer::Instance.Find(pThis);

	if (pData->Weapon && pData->Weapon->AmbientDamage) {
		CoordStruct coords;
		for (auto const& pCell : pThis->Cells) {
			pCell->Get3DCoords3(&coords);
			pThis->DamageArea(coords);
		}
	}

	int Intensity;

	switch (pThis->Type)
	{
	case WaveType::Magnetron:
	case WaveType::Sonic:
	{
		pThis->WaveAI();
		Intensity = pThis->WaveIntensity;
		--Intensity;
		pThis->WaveIntensity = Intensity;
		if (Intensity < 0)
		{
			//GameDelete<true,false>(pThis);
			pThis->UnInit();
		} else {
			pThis->ObjectClass::Update();
		}
	}
	break;
	case WaveType::BigLaser:
	case WaveType::Laser:
	{
		Intensity = pThis->LaserIntensity;
		Intensity -= 6;
		pThis->LaserIntensity = Intensity;
		if (Intensity < 32)
		{
			//GameDelete<true,false>(pThis);
			pThis->UnInit();
		}

		break;
	}
	}

	return 0x76101A;
}

DEFINE_OVERRIDE_HOOK(0x760BC2, WaveClass_Draw2, 0x9)
{
	GET(WaveClass*, Wave, EBX);
	GET(WORD*, dest, EBP);

	return (WaveExtData::ModifyWaveColor(*dest, *dest, Wave->LaserIntensity, Wave, &AresCreateWave::TempColor))
		? 0x760CAFu
		: 0u
		;
}

DEFINE_OVERRIDE_HOOK(0x760DE2, WaveClass_Draw3, 0x9)
{
	GET(WaveClass*, Wave, EBX);
	GET(WORD*, dest, EDI);

	return (WaveExtData::ModifyWaveColor(*dest, *dest, Wave->LaserIntensity, Wave, &AresCreateWave::TempColor))
		? 0x760ECBu
		: 0u
		;
}

DEFINE_OVERRIDE_HOOK(0x75EE57, WaveClass_Draw_Sonic, 0x7)
{
	GET_STACK(WaveClass*, Wave, 0x4);
	GET(WORD*, src, EDI);
	GET(DWORD, offset, ECX);

	return (WaveExtData::ModifyWaveColor(src[offset], *src, R->ESI(), Wave, &AresCreateWave::TempColor))
		? 0x75EF1Cu
		: 0u
		;
}

DEFINE_OVERRIDE_HOOK(0x7601FB, WaveClass_Draw_Magnetron2, 0xB)
{
	GET_STACK(WaveClass*, Wave, 0x8);
	GET(WORD*, src, EBX);
	GET(DWORD, offset, ECX);

	return (WaveExtData::ModifyWaveColor(src[offset], *src, R->EBP(), Wave, &AresCreateWave::TempColor))
		? 0x760285u
		: 0u
		;
}

DEFINE_OVERRIDE_HOOK(0x75F46E, WaveClass_DamageCell_Wall, 6)
{
	GET(WeaponTypeClass*, pWeapon, EBX);
	return pWeapon->Warhead->Wall ? 0x0 : 0x75F47C;
}

DEFINE_OVERRIDE_HOOK(0x75F38F, WaveClass_DamageCell_SelectWeapon, 0x6)
{
	GET(WaveClass*, pWave, EBP);
	R->EDI(R->EAX());
	R->EBX(WaveExtContainer::Instance.Find(pWave)->Weapon);
	return 0x75F39D;
}

 /*
 *	YES , this fuckery is removing WaveClass::WaveAI function call for later , replace it with boolean ,
	so it can be done after all data set is completed !
 */
DEFINE_HOOK(0x75EBC5, WaveClass_CTOR_AllowWaveUpdate, 0x7)
{
	GET(WaveClass*, Wave, ESI);
	WaveExtContainer::Instance.Find(Wave)->CanDoUpdate = true;
	return 0x75EBCC;
}

/*
	FUCKING WW DOING THESE INSIDE THE CONSTRUCTOR ,..
	that mean some Ext variable not yer executed , and when i try to use the data it wont work
	need to move those to separate function after data set done ,..
*/
DEFINE_OVERRIDE_HOOK(0x762B62, WaveClass_WaveAI , 0x6)
{
	GET(WaveClass*, Wave, ESI);

	TechnoClass* Firer = Wave->Owner;
	TechnoClass* Target = Wave->Target;

	const bool eligible = Target && Firer && Wave->WaveIntensity != 19 && Firer->Target == Target;

	if (!eligible) {
		return 0x762C40;
	}

	const auto pData = WaveExtContainer::Instance.Find(Wave);

	if (Wave->Type == WaveType::Magnetron)
	{
		if (pData->WeaponIdx != -1)
		{
			if (!Firer->IsCloseEnough(Target, pData->WeaponIdx))
			{
				return 0x762C40;
			}
		}
		else
		{
			auto nFirerCoord = pData->SourceCoord;
			auto nTargetCoord = Target->GetCoords();
			const auto nRange = (pData->Weapon ? pData->Weapon->Range : Firer->GetTechnoType()->GuardRange);
			if (nRange < (int)(nFirerCoord.DistanceFrom(nTargetCoord) / 1.414213562373095))
			{
				return 0x762C40;
			}
		}

	}
	else
	{
		auto nFirerCoord = pData->WeaponIdx != -1 ? Firer->GetCoords() : pData->SourceCoord;
		auto nTargetCoord = Target->GetCoords();
		const auto nRange = (pData->Weapon ? pData->Weapon->Range : Firer->GetTechnoType()->GuardRange);
		if (nRange < (int)(nFirerCoord.DistanceFrom(nTargetCoord) / 1.414213562373095))
		{
			return 0x762C40;
		}
	}

	if (!Wave->bool_12C)
		return 0x762D57;

	CoordStruct FLH;
	if(pData->WeaponIdx != -1)
		Firer->GetFLH(&FLH , pData->WeaponIdx, CoordStruct::Empty);
	else
		 FLH = pData->SourceCoord;

	const CoordStruct xyzTgt = Target->GetCenterCoords(); // not GetCoords() !

	if (Wave->Type == WaveType::Magnetron)
	{
		pData->ReverseAgainstTarget
			? Wave->DrawMag(xyzTgt, FLH)
			: Wave->DrawMag(FLH, xyzTgt);
	}
	else
	{
		pData->ReverseAgainstTarget
			? Wave->DrawNonMag(xyzTgt, FLH)
			: Wave->DrawNonMag(FLH, xyzTgt);
	}

	return 0x762D57;
}

DEFINE_OVERRIDE_HOOK(0x75EE2E, WaveClass_Draw_Green, 0x8)
{
	GET(int, Q, EDX);
	if (Q > 0x15F8F)
	{
		Q = 0x15F8F;
	}
	R->EDX(Q);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x7601C7, WaveClass_Draw_Magnetron, 0x8)
{
	GET(int, Q, EDX);
	if (Q > 0x15F8F)
	{
		Q = 0x15F8F;
	}
	R->EDX(Q);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x7609E3, WaveClass_Draw_NodLaser_Details, 0x5)
{
	R->EAX(2);
	return 0x7609E8;
}

DEFINE_DISABLE_HOOK(0x760286, WaveClass_Draw_Magnetron3_ares)//, 0x5, 7602D3)
DEFINE_JUMP(LJMP, 0x760286, 0x7602D3);

DEFINE_OVERRIDE_HOOK(0x76110B, WaveClass_RecalculateAffectedCells_Clear, 0x5)
{
	GET(DynamicVectorClass<CellClass*>*, pVec, EBP);
	pVec->Reset();
	return 0x761110;
}

DEFINE_HOOK(0x75F415, WaveClass_DamageCell_FixNoHouseOwner, 0x6)
{
	GET(WaveClass*, pThis, EBP);
	GET(CellClass*, pCell, EDI);
	GET(ObjectClass*, pVictim, ESI);
	GET_STACK(int, nDamage, STACK_OFFS(0x18, 0x4));
	GET_STACK(WarheadTypeClass*, pWarhead, STACK_OFFS(0x18, 0x8));

	if (const auto pTechnoVictim = generic_cast<TechnoClass*>(pVictim)){
		if (pTechnoVictim->IsSinking || pTechnoVictim->IsCrashing)
			return 0x75F432;

		if (const auto pUnit = specific_cast<UnitClass*>(pVictim)) {
			if (pUnit->DeathFrameCounter > 0)
				return 0x75F432;
		}
	}

	//pVictim->ReceiveDamage(&nDamage, 0, pWarhead, pTechnoOwner, false, false, pTechnoOwner->Owner);
	WarheadTypeExtData::DetonateAt(pWarhead, pVictim, pCell->GetCoordsWithBridge(), pThis->Owner, nDamage);
	return 0x75F432;
}