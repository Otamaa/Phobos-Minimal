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

#include <HoverLocomotionClass.h>

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

DEFINE_DISABLE_HOOK(0x763226, WaveClass_DTOR_Ares)

struct WaveColorData
{
	Point3D Intent_Color;
	ColorStruct Color;
};

static constexpr WaveColorData DefaultLaser { { 0,0,0 } , { 64,0,96 } };
static constexpr WaveColorData DefaultSonic { { 0,256,256 } , {0,0,0 } };
static constexpr WaveColorData DefaultMag { { 128,0,1024 } , { 0,0,0 } };

namespace AresCreateWave
{
	WaveClass* Create(CoordStruct nFrom, CoordStruct nTo, TechnoClass* pOwner, WaveType nType, AbstractClass* pTarget,
		WeaponTypeClass* pWeapon)
	{
		if (auto const pWave = GameCreate<WaveClass>(nFrom, nTo, pOwner, nType, pTarget))
		{
			WaveExt::ExtMap.Find(pWave)->SetWeaponType(pWeapon, TechnoExt::ExtMap.Find(pOwner)->CurrentWeaponIdx);
			WaveExt::ExtMap.Find(pWave)->InitWeaponData();
			return pWave;
		}

		return nullptr;
	}

	void NOINLINE ToColorStruct(WORD const src, ColorStruct& out)
	{
		out.R = (BYTE)(src >> Drawing::RedShiftLeft) << Drawing::RedShiftRight;
		out.G = (BYTE)(src >> Drawing::GreenShiftLeft) << Drawing::GreenShiftRight;
		out.B = (BYTE)(src >> Drawing::BlueShiftLeft) << Drawing::BlueShiftRight;
	}

	WORD NOINLINE ToWord(ColorStruct& modified)
	{
		return (
		   (BYTE)(modified.R >> Drawing::RedShiftRight) << Drawing::RedShiftLeft |	   
		   (BYTE)(modified.B >> Drawing::BlueShiftRight) << Drawing::BlueShiftLeft |
		   (BYTE)(modified.G >> Drawing::GreenShiftRight) << Drawing::GreenShiftLeft
		   );
	}

	bool ModifyWaveColor(
	WORD const src, WORD& dest, int const intensity, WaveClass* const pWave, WaveColorData const* colorDatas)
	{
		if (!colorDatas->Color && !colorDatas->Intent_Color.IsValid())
			return false;

		ColorStruct modified;
		ToColorStruct(src, modified);

		// ugly hack to fix byte wraparound problems
		auto const upcolor = [=, &modified, &colorDatas]
		(int Point3D::* intentmember, BYTE ColorStruct::* member)
		{
			auto const component = std::clamp(modified.*member
				+ ((intensity * colorDatas->Intent_Color.*intentmember * modified.*member) >> 16)
				+ ((intensity * colorDatas->Color.*member) >> 8), 0, 255);

			modified.*member = static_cast<BYTE>(component);
		};

		upcolor(&Point3D::X, &ColorStruct::R);
		upcolor(&Point3D::Y, &ColorStruct::G);
		upcolor(&Point3D::Z, &ColorStruct::B);

		dest = ToWord(modified);

		return true;
	}

	WaveColorData GetWaveColor(WaveClass* pWave) {

		auto const pData = WaveExt::ExtMap.Find(pWave);

		if (pData->Weapon)
		{
			auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pData->Weapon);
			const auto pThis = pWeaponExt->OwnerObject();

			if (pWeaponExt->Wave_IsHouseColor && pWave->Owner && pWave->Owner->Owner)
			{				
				return
				{
					pWeaponExt->Wave_Intent.Get(DefaultLaser.Intent_Color) ,
					pWave->Owner->Owner->Color

				};
			}
			else
			{
				if (pThis->IsMagBeam)
				{
					return
					{
						pWeaponExt->Wave_Intent.Get(DefaultMag.Intent_Color) ,
						pWeaponExt->Wave_Color.Get(DefaultMag.Color)

					};
				}
				else if (pThis->IsSonic)
				{
					return
					{
						pWeaponExt->Wave_Intent.Get(DefaultSonic.Intent_Color) ,
						pWeaponExt->Wave_Color.Get(DefaultSonic.Color)

					};
				}
				else
				{
					return
					{
						pWeaponExt->Wave_Intent.Get(DefaultMag.Intent_Color) ,
						pWeaponExt->Wave_Color.Get(DefaultMag.Color)

					};
				}
			}
		}

		return { Point3D::Empty , ColorStruct::Empty };
	}

	ColorStruct GetWaveColor(WeaponTypeExt::ExtData* pData)
	{
		auto pThis = pData->OwnerObject();

		if (pThis->IsMagBeam)
		{
			return pData->Wave_Color.Get(WaveClass::DefaultWaveColorMagBeam);
		}
		else if (pThis->IsSonic)
		{
			return pData->Wave_Color.Get(WaveClass::DefaultWaveColorSonic);
		}
		else
		{
			return pData->Wave_Color.Get(WaveClass::DefaultWaveColor);
		}
	}

	bool ModifyWaveColor(
	WORD const src, WORD& dest, int const intensity, WaveClass* const pWave)
	{
		auto const pData = WaveExt::ExtMap.Find(pWave);

		if (!pData->Weapon)
			return false;

		auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pData->Weapon);

		auto const currentColor = (pWeaponExt->Wave_IsHouseColor && pWave->Owner && pWave->Owner->Owner)
			? pWave->Owner->Owner->Color
			: GetWaveColor(pWeaponExt);

		if (currentColor == ColorStruct::Empty)
		{
			return false;
		}

		ColorStruct modified;
		modified.R = static_cast<BYTE>(src >> Drawing::RedShiftLeft << Drawing::RedShiftRight);
		modified.G = static_cast<BYTE>(src >> Drawing::GreenShiftLeft << Drawing::GreenShiftRight);
		modified.B = static_cast<BYTE>(src >> Drawing::BlueShiftLeft << Drawing::BlueShiftRight);

		// ugly hack to fix byte wraparound problems
		auto const upcolor = [=, &modified](BYTE ColorStruct::* member)
		{
			auto const component = std::clamp(modified.*member
				+ (intensity * currentColor.*member) / 256, 0, 255);
			modified.*member = static_cast<BYTE>(component);
		};

		upcolor(&ColorStruct::R);
		upcolor(&ColorStruct::G);
		upcolor(&ColorStruct::B);

		dest = Drawing::Color16bit(modified);
		return true;
	}

	WaveColorData TempColor;
}

DEFINE_OVERRIDE_HOOK(0x6FF449, TechnoClass_Fire_SonicWave, 5)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(WeaponTypeClass* const, pSource, EBX);

	GET_BASE(AbstractClass*, pTarget, 0x8);

	REF_STACK(CoordStruct const, crdSrc, 0x44);
	REF_STACK(CoordStruct const, crdTgt, 0x88);

	pThis->Wave = AresCreateWave::Create(crdSrc, crdTgt, pThis, WaveType::Sonic, pTarget, pSource);
	return 0x6FF48A;
}

DEFINE_OVERRIDE_HOOK(0x6FF5F5, TechnoClass_Fire_OtherWaves, 6)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(WeaponTypeClass* const, pSource, EBX);
	GET(TechnoClass* const, pTarget, EDI);

	REF_STACK(CoordStruct const, crdSrc, 0x44);
	REF_STACK(CoordStruct const, crdTgt, 0x88);

	auto const pData = WeaponTypeExt::ExtMap.Find(pSource);

	if (!pData->IsWave() || pThis->Wave)
		return 0x6FF656;

	WaveType nType = WaveType::Sonic;
	if (pSource->IsMagBeam)
		nType = WaveType::Magnetron;
	else
		nType = pData->Wave_IsBigLaser
		? WaveType::BigLaser : WaveType::Laser;

	pThis->Wave = AresCreateWave::Create(crdSrc, crdTgt, pThis, nType, pTarget, pSource);
	return 0x6FF656;
}

DEFINE_OVERRIDE_HOOK(0x75FA29, WaveClass_Draw_Colors, 0x6)
{
	GET(WaveClass*, pThis, ESI);
	const auto nData = AresCreateWave::GetWaveColor(pThis);
	std::memcpy(&AresCreateWave::TempColor, &nData, sizeof(WaveColorData));
	return 0x0;
}

DEFINE_OVERRIDE_HOOK(0x760F50, WaveClass_Update, 0x6)
{
	GET(WaveClass*, pThis, ECX);

	auto pData = WaveExt::ExtMap.Find(pThis);
	const WeaponTypeClass* Weap = pData->Weapon;

	if (!Weap)
	{
		return 0;
	}

	int Intensity;

	if (Weap->AmbientDamage)
	{
		CoordStruct coords;
		for (auto const& pCell : pThis->Cells)
		{
			pThis->DamageArea(*pCell->Get3DCoords3(&coords));
		}
	}

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
			pThis->UnInit();
		}
		else
		{
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

	return (AresCreateWave::ModifyWaveColor(*dest, *dest, Wave->LaserIntensity, Wave, &AresCreateWave::TempColor))
		? 0x760CAFu
		: 0u
		;
}

DEFINE_OVERRIDE_HOOK(0x760DE2, WaveClass_Draw3, 0x9)
{
	GET(WaveClass*, Wave, EBX);
	GET(WORD*, dest, EDI);

	return (AresCreateWave::ModifyWaveColor(*dest, *dest, Wave->LaserIntensity, Wave, &AresCreateWave::TempColor))
		? 0x760ECBu
		: 0u
		;
}

DEFINE_OVERRIDE_HOOK(0x75EE57, WaveClass_Draw_Sonic, 0x7)
{
	GET_STACK(WaveClass*, Wave, 0x4);
	GET(WORD*, src, EDI);
	GET(DWORD, offset, ECX);

	return (AresCreateWave::ModifyWaveColor(src[offset], *src, R->ESI(), Wave, &AresCreateWave::TempColor))
		? 0x75EF1Cu
		: 0u
		;
}

DEFINE_OVERRIDE_HOOK(0x7601FB, WaveClass_Draw_Magnetron2, 0xB)
{
	GET_STACK(WaveClass*, Wave, 0x8);
	GET(WORD*, src, EBX);
	GET(DWORD, offset, ECX);

	return (AresCreateWave::ModifyWaveColor(src[offset], *src, R->EBP(), Wave, &AresCreateWave::TempColor))
		? 0x760285u
		: 0u
		;
}

DEFINE_OVERRIDE_HOOK(0x75F46E, WaveClass_DamageCell_Wall, 6)
{
	GET(WeaponTypeClass*, pWeapon, EBX);
	return pWeapon->Warhead->Wall ? 0x0 : 0x75F47C;
}

DEFINE_OVERRIDE_HOOK(0x762B62, WaveClass_Update_Beam, 0x6)
{
	GET(WaveClass*, Wave, ESI);

	TechnoClass* Firer = Wave->Owner;
	TechnoClass* Target = Wave->Target;

	const bool eligible = Target && Firer && Wave->WaveIntensity != 19 && Firer->Target == Target;

	if (!eligible)
	{
		return 0x762C40;
	}

	const auto pData = WaveExt::ExtMap.Find(Wave);

	if (Wave->Type == WaveType::Magnetron)
	{
		if (!Firer->IsCloseEnough(Target, pData->WeaponIdx))
		{
			return 0x762C40;
		}
	}
	else
	{
		auto nFirerCoord = Firer->GetCoords();
		auto nTargetCoord = Target->GetCoords();
		const auto nRange = (pData->Weapon ? pData->Weapon->Range : Firer->GetTechnoType()->GuardRange);
		if (nRange < (int)(nFirerCoord.DistanceFrom(nTargetCoord) / 1.414213562373095))
		{
			return 0x762C40;
		}
	}

	if (!Wave->bool_12C)
		return 0x762D57;

	const CoordStruct FLH = Firer->GetFLH(pData->WeaponIdx, CoordStruct::Empty);
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

DEFINE_OVERRIDE_SKIP_HOOK(0x760286, WaveClass_Draw_Magnetron3, 0x5, 7602D3)
DEFINE_OVERRIDE_HOOK(0x76110B, WaveClass_RecalculateAffectedCells_Clear, 0x5)
{
	GET(DynamicVectorClass<CellClass*>*, pVec, EBP);
	pVec->Count = 0; //clear count , dont destroy the vector
	return 0x761110;
}

DEFINE_HOOK(0x75F415, WaveClass_DamageCell_FixNoHouseOwner, 0x6)
{
	GET(TechnoClass*, pTechnoOwner, EAX);
	GET(CellClass*, pCell, EDI);
	GET(ObjectClass*, pVictim, ESI);
	GET_STACK(int, nDamage, STACK_OFFS(0x18, 0x4));
	GET_STACK(WarheadTypeClass*, pWarhead, STACK_OFFS(0x18, 0x8));

	if (const auto pUnit = specific_cast<UnitClass*>(pVictim))
		if (pUnit->DeathFrameCounter > 0)
			return 0x75F432;

	WarheadTypeExt::DetonateAt(pWarhead, pVictim, pCell->GetCoordsWithBridge(), pTechnoOwner, nDamage);
	return 0x75F432;
}