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

// Prism need these , replacing it will lead on crash
//Ares 3.0 optimization : using global var
Point3D BoltColors {};

DEFINE_HOOK(4C1F33, EBolt_Draw_Colors, 7)
{
	GET(EBolt*, pThis, ECX);
	GET_BASE(int, nColorIdx, 0x20);

	auto const pDefaultPal = FileSystem::PALETTE_PAL();
	auto const nColorBuffer = pDefaultPal->BufferMid;

	if (pDefaultPal->BytesPerPixel == 1)
	{
		BoltColors.X = BoltColors.Y = nColorBuffer[nColorIdx];
		BoltColors.Z = nColorBuffer[WeaponTypeExt::EBoltColorIndx];
	}
	else
	{
		BoltColors.X = BoltColors.Y = nColorBuffer[2 * nColorIdx];
		BoltColors.Z = nColorBuffer[2 * WeaponTypeExt::EBoltColorIndx];
	}

	if (auto pData = WeaponTypeExt::BoltExt.get_or_default(pThis))
	{
		auto& clr1 = pData->Bolt_Color1;
		if (clr1.isset()) { BoltColors.X = Drawing::ColorStructToWord(clr1); }

		auto& clr2 = pData->Bolt_Color2;
		if (clr2.isset()) { BoltColors.Y = Drawing::ColorStructToWord(clr2); }

		auto& clr3 = pData->Bolt_Color3;
		if (clr3.isset()) { BoltColors.Z = Drawing::ColorStructToWord(clr3); }
	}

	return 0x4C1F66;
}

DEFINE_HOOK(6FD469, TechnoClass_FireEBolt, 9)
{
	//GET(TechnoClass*, pThis, EDI);
	GET_STACK(WeaponTypeClass*, pWeapon, STACK_OFFS(0x30, -0x8));

	R->EAX(WeaponTypeExt::CreateBolt(pWeapon));
	R->ESI(0);

	return 0x6FD480;
}

DEFINE_HOOK(4C2951, EBolt_DTOR, 5)
{
	GET(EBolt*, Bolt, ECX);
	WeaponTypeExt::BoltExt.erase(Bolt);
	return 0;
}

DEFINE_HOOK(4C24BE, EBolt_Draw_Color1, 5)
{
	R->EAX(BoltColors.X);
	return 0x4C24E4;
}

DEFINE_HOOK(4C25CB, EBolt_Draw_Color2, 5)
{
	R->Stack<int>(0x18, BoltColors.Y);
	return 0x4C25FD;
}

DEFINE_HOOK(4C26CF, EBolt_Draw_Color3, 5)
{
	R->EAX(BoltColors.Z);
	return 0x4C26EE;
}

//DEFINE_HOOK(4C2AFF, EBolt_Fire_Particles, 5)
//{
//	GET(EBolt*, pThis, ESI);
//
//	auto pParticleSys = RulesGlobal->DefaultSparkSystem;
//
//	if (auto pData = WeaponTypeExt::BoltExt.get_or_default(pThis))
//	{
//		if (!pData->Bolt_ParticleSys_Enabled.Get())
//		{	
//			// because ESI got pop-ed early , and this weirdass assembly code 
//			// this nessesary
//			R->EAX(0);
//			return 0x4C2B0C;
//		}
//
//		if (pData->Bolt_ParticleSys.isset())
//			pParticleSys = pData->Bolt_ParticleSys.Get();
//	}
//
//	if (pParticleSys)
//		GameCreate<ParticleSystemClass>(pParticleSys, pThis->Point2, nullptr, pThis->Owner, CoordStruct::Empty, pThis->Owner ? pThis->Owner->GetOwningHouse() : nullptr);
//
//	// because ESI got pop-ed early , and this weirdass assembly code 
//	// this nessesary
//	R->EAX(0);
//	return 0x4C2B0C;
//}