#include <Ext/WeaponType/Body.h>
#include <EBolt.h>

#include <Helpers/Macro.h>

#include <ParticleSystemClass.h>

DEFINE_HOOK(0x6FD469, TechnoClass_FireEBolt, 9)
{
	//GET(TechnoClass*, pThis, EDI);
	GET_STACK(WeaponTypeClass*, pWeapon, STACK_OFFS(0x30, -0x8));

	R->EAX(WeaponTypeExtData::CreateBolt(pWeapon));
	R->ESI(0);

	return 0x6FD480;
}

DEFINE_HOOK(0x6FD5FC, TechnoClass_CreateEbolt_UnnessesaryData, 0xA)
{
	GET(UnitClass*, pThis, ESI);
	GET(int, nWeaponIdx, EBX);
	GET(EBolt*, pBolt, EDI);

	pThis->ElectricBolt = pBolt;
	pBolt->Owner = pThis;
	pBolt->WeaponSlot = nWeaponIdx;

	return 0x6FD60B;
}

namespace BoltTemp
{
	const WeaponTypeExtData* pType = nullptr;
}

inline unsigned inline_02(ConvertClass* pConvert , int idx)
{
	switch (pConvert->BytesPerPixel)
	{
	default:
	case ConvertClass::BytesPerPixel::One:
		return reinterpret_cast<uint8_t*>(pConvert->BufferMid)[idx];
	case ConvertClass::BytesPerPixel::Two:
		return reinterpret_cast<uint16_t*>(pConvert->BufferMid)[idx];
	};
}

int boltColor1;
int boltColor2;
int boltColor3;

DEFINE_HOOK(0x4C1F33, EBolt_Draw_Colors, 7)
{
	GET(EBolt*, pThis, ECX);
	GET_BASE(int, nColorIdx, 0x20);

	auto& data1 = boltColor1;
	auto& data2 = boltColor2;
	auto& data3 = boltColor3;
	const auto& nMap = WeaponTypeExtData::boltWeaponTypeExt;

	const auto nFirst = FileSystem::PALETTE_PAL()->inline_02(nColorIdx);
	const auto nSec = FileSystem::PALETTE_PAL()->inline_02(15);
	data1 = data2 = nFirst;
	data3 = nSec;

	auto pWeaponExt = nMap.get_or_default(pThis);

	if (pWeaponExt)
	{
		BoltTemp::pType = pWeaponExt;

		const auto& clr1 = pWeaponExt->Bolt_Color1;
		if (clr1.isset()) { data1 = Drawing::ColorStructToWord(clr1.Get()); }

		const auto& clr2 = pWeaponExt->Bolt_Color2;
		if (clr2.isset()) { data2 = Drawing::ColorStructToWord(clr2.Get()); }

		const auto& clr3 = pWeaponExt->Bolt_Color3;
		if (clr3.isset()) { data3 = Drawing::ColorStructToWord(clr3.Get()); }
	}

	return 0x4C1F66;
}

DEFINE_HOOK(0x4C2C10, EboltClass_Cleanup, 0x5) {
	GET(EBolt* const, pBolt, ECX);
	Debug::Log("CleaningUp Ebot[%x]\n", pBolt);
	return 0x0;
}

DEFINE_HOOK(0x4C2951, EBolt_DTOR, 0x5)
{
	GET(EBolt* const, pBolt, ECX);

	WeaponTypeExtData::boltWeaponTypeExt.erase(pBolt);

	return 0;
}

DEFINE_HOOK(0x4C24E4, Ebolt_DrawFist_Disable, 0x8)
{
	//GET_STACK(EBolt* const, pBolt, 0x40);

	if (BoltTemp::pType && BoltTemp::pType->Bolt_Disable1){
		return 0x4C2515;
	}

	return 0;
}

DEFINE_HOOK(0x4C20BC, EBolt_DrawArcs, 0x5)
{
	enum { DoLoop = 0x4C20C7, Break = 0x4C2400 };

	//GET_STACK(EBolt*, pBolt, 0x40);
	GET_STACK(int, plotIndex, STACK_OFFSET(0x408, -0x3E0))

	if(BoltTemp::pType){
		return plotIndex < BoltTemp::pType->Bolt_Arcs
		? DoLoop : Break;
	}

	return 0x0;
}

DEFINE_HOOK(0x4C25FD, Ebolt_DrawSecond_Disable, 0xA)
{
	if (BoltTemp::pType && BoltTemp::pType->Bolt_Disable2) {
		return 0x4C262A;
	}

	return 0;
}

DEFINE_HOOK(0x4C26EE, Ebolt_DrawThird_Disable, 0x6)
{
	if (BoltTemp::pType && BoltTemp::pType->Bolt_Disable3) {
		return 0x4C2710;
	}

	return  0;
}

DEFINE_HOOK(0x4C24BE, EBolt_Draw_Color1, 5)
{
	R->EAX(boltColor1);
	return 0x4C24E4;
}

DEFINE_HOOK(0x4C25CB, EBolt_Draw_Color2, 5)
{
	R->Stack<int>(0x18, boltColor2);
	return 0x4C25FD;
}

DEFINE_HOOK(0x4C26CF, EBolt_Draw_Color3, 5)
{
	R->EAX(boltColor3);
	return 0x4C26EE;
}

void NAKED _EBolt_Fire_Particles_RET()
{
	POP_REG(esi); // We need to handle origin two push here...
	JMP(0x4C2B35);
}

DEFINE_HOOK(0x4C2AFF, EBolt_Fire_Particles, 5)
{
	GET(EBolt*, pThis, ESI);

	auto pParticleSys = RulesClass::Instance->DefaultSparkSystem;

	if (auto pData = WeaponTypeExtData::boltWeaponTypeExt.get_or_default(pThis))
	{
		if (!pData->Bolt_ParticleSys_Enabled)
		{
			return DWORD(_EBolt_Fire_Particles_RET);
		}

		if (pData->Bolt_ParticleSys.isset())
			pParticleSys = pData->Bolt_ParticleSys.Get();
	}

	if (pParticleSys)
		GameCreate<ParticleSystemClass>(pParticleSys, pThis->Point2, nullptr, pThis->Owner, CoordStruct::Empty, pThis->Owner ? pThis->Owner->GetOwningHouse() : nullptr);

	return DWORD(_EBolt_Fire_Particles_RET);
}