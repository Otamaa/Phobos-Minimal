#include <Ext/WeaponType/Body.h>
#include <EBolt.h>

#include <Helpers/Macro.h>

#include <ParticleSystemClass.h>

#include <Ext/Techno/Body.h>
#include <Utilities/Macro.h>

class EBoltFake final : public EBolt
{
public:
	void _SetOwner(TechnoClass* pTechno, int weaponIndex);
	void _RemoveFromOwner();
};

void EBoltFake::_SetOwner(TechnoClass* pTechno, int weaponIndex)
{
	if (pTechno && pTechno->IsAlive)
	{
		auto const pWeapon = pTechno->GetWeapon(weaponIndex)->WeaponType;
		auto const pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

		if (!pWeaponExt->Bolt_FollowFLH.Get(pTechno->WhatAmI() == AbstractType::Unit))
			return;

		this->Owner = pTechno;
		this->WeaponSlot = weaponIndex;

		auto const pExt = TechnoExtContainer::Instance.Find(pTechno);
		pExt->ElectricBolts.emplace_back(this);
	}
}

void EBoltFake::_RemoveFromOwner()
{
	TechnoExtContainer::Instance.Find(this->Owner)->ElectricBolts.remove(this);
	this->Owner = nullptr;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x4C2BD0, EBoltFake::_SetOwner); // Failsafe in case called in another module

ASMJIT_PATCH(0x6FD5D6, TechnoClass_InitEBolt, 0x6)
{
	enum { SkipGameCode = 0x6FD60B };

	GET(TechnoClass*, pThis, ESI);
	GET(EBoltFake*, pBolt, EAX);
	GET(int, weaponIndex, EBX);

	if (pBolt)
		pBolt->_SetOwner(pThis, weaponIndex);

	return SkipGameCode;
}

ASMJIT_PATCH(0x6FD469, TechnoClass_FireEBolt, 9)
{
	GET(TechnoClass*, pThis, EDI);
	GET_STACK(WeaponTypeClass*, pWeapon, STACK_OFFS(0x30, -0x8));

	R->EAX(WeaponTypeExtData::CreateBolt(pWeapon, pThis));
	R->ESI(0);

	return 0x6FD480;
}

ASMJIT_PATCH(0x4C285D, EBolt_DrawAll_BurstIndex, 0x5)
{
	enum { SkipGameCode = 0x4C2882 };

	GET(TechnoClass*, pTechno, ECX);
	GET_STACK(EBolt*, pThis, STACK_OFFSET(0x34, -0x24));

	int burstIndex = pTechno->CurrentBurstIndex;
	pTechno->CurrentBurstIndex = WeaponTypeExtData::boltWeaponTypeExt[pThis].BurstIndex;
	CoordStruct fireCoords {};
	pTechno->GetFLH(&fireCoords, pThis->WeaponSlot, CoordStruct::Empty);
	pTechno->CurrentBurstIndex = burstIndex;
	R->EAX(&fireCoords);

	return SkipGameCode;
}

ASMJIT_PATCH(0x4C299F, EBolt_DrawAll_EndOfLife, 0x6)
{
	enum { SkipGameCode = 0x4C29B9 };

	GET(EBoltFake*, pThis, EAX);

	if (pThis->Owner)
		pThis->_RemoveFromOwner();

	return SkipGameCode;
}

ASMJIT_PATCH(0x4C2A02, EBolt_DestroyVector, 0x6)
{
	enum { SkipGameCode = 0x4C2A08 };

	GET(EBoltFake*, pThis, EAX);
	pThis->_RemoveFromOwner();
	return SkipGameCode;
}

namespace BoltTemp
{
	const WeaponTypeExtData* pType = nullptr;
}

OPTIONALINLINE unsigned inline_02(ConvertClass* pConvert , int idx)
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

ASMJIT_PATCH(0x4C1F33, EBolt_Draw_Colors, 7)
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

	if (auto pWeaponExt = nMap.get_or_default(pThis).Weapon)
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

ASMJIT_PATCH(0x4C2951, EBolt_DTOR, 0x5)
{
	GET(EBolt* const, pBolt, ECX);

	WeaponTypeExtData::boltWeaponTypeExt.erase(pBolt);

	return 0;
}

ASMJIT_PATCH(0x4C24E4, Ebolt_DrawFist_Disable, 0x8)
{
	//GET_STACK(EBolt* const, pBolt, 0x40);

	if (BoltTemp::pType && BoltTemp::pType->Bolt_Disable1){
		return 0x4C2515;
	}

	return 0;
}

ASMJIT_PATCH(0x4C20BC, EBolt_DrawArcs, 0x5)
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

ASMJIT_PATCH(0x4C25FD, Ebolt_DrawSecond_Disable, 0xA)
{
	if (BoltTemp::pType && BoltTemp::pType->Bolt_Disable2) {
		return 0x4C262A;
	}

	return 0;
}

ASMJIT_PATCH(0x4C26EE, Ebolt_DrawThird_Disable, 0x6)
{
	if (BoltTemp::pType && BoltTemp::pType->Bolt_Disable3) {
		return 0x4C2710;
	}

	return  0;
}

ASMJIT_PATCH(0x4C24BE, EBolt_Draw_Color1, 5)
{
	R->EAX(boltColor1);
	return 0x4C24E4;
}

ASMJIT_PATCH(0x4C25CB, EBolt_Draw_Color2, 5)
{
	R->Stack<int>(0x18, boltColor2);
	return 0x4C25FD;
}

ASMJIT_PATCH(0x4C26CF, EBolt_Draw_Color3, 5)
{
	R->EAX(boltColor3);
	return 0x4C26EE;
}

void NAKED _EBolt_Fire_Particles_RET()
{
	POP_REG(esi); // We need to handle origin two push here...
	JMP(0x4C2B35);
}

ASMJIT_PATCH(0x4C2AFF, EBolt_Fire_Particles, 5)
{
	GET(EBolt*, pThis, ESI);

	auto pParticleSys = RulesClass::Instance->DefaultSparkSystem;

	if (auto pData = WeaponTypeExtData::boltWeaponTypeExt.get_or_default(pThis).Weapon)
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