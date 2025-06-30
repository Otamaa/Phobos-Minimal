#include <Ext/WeaponType/Body.h>
#include <EBolt.h>

#include <Helpers/Macro.h>

#include <ParticleSystemClass.h>

#include <Ext/Techno/Body.h>
#include <Ext/Ebolt/Body.h>

#include <Utilities/Macro.h>

class NOVTABLE EBoltFake final : public EBolt
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

	R->EAX(EboltExtData::_CreateOneOf(pWeapon, pThis));
	R->ESI(0);

	return 0x6FD480;
}

ASMJIT_PATCH(0x4C285D, EBolt_DrawAll_BurstIndex, 0x5)
{
	enum { SkipGameCode = 0x4C2882 };

	GET(TechnoClass*, pTechno, ECX);
	GET_STACK(EBolt*, pThis, STACK_OFFSET(0x34, -0x24));

	int burstIndex = pTechno->CurrentBurstIndex;
	pTechno->CurrentBurstIndex = EboltExtData::Container[pThis].BurstIndex;
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

namespace EboltTemp {
	EboltExtData* Data;
}

ASMJIT_PATCH(0x4C1F33, EBolt_Draw_Colors, 7)
{
	GET(EBolt*, pThis, ECX);
	//GET_BASE(int, nColorIdx, 0x20);
	EboltTemp::Data = EboltExtData::Container.tryfind(pThis);
	return 0x4C1F66;
}

ASMJIT_PATCH(0x4C2951, EBolt_DTOR, 0x5)
{
	GET(EBolt* const, pBolt, ECX);

	EboltExtData::Container.erase(pBolt);

	return 0;
}

ASMJIT_PATCH(0x4C24E4, Ebolt_DrawFist_Disable, 0x8)
{
	//GET_STACK(EBolt* const, pBolt, 0x40);

	if (EboltTemp::Data && EboltTemp::Data->Disable[0]) {
		return 0x4C2515;
	}

	return 0;
}

ASMJIT_PATCH(0x4C20BC, EBolt_DrawArcs, 0x5)
{
	enum { DoLoop = 0x4C20C7, Break = 0x4C2400 };

	//GET_STACK(EBolt*, pBolt, 0x40);
	GET_STACK(int, plotIndex, STACK_OFFSET(0x408, -0x3E0))

	if(EboltTemp::Data){
		return plotIndex < EboltTemp::Data->Arcs
		? DoLoop : Break;
	}

	return 0x0;
}

ASMJIT_PATCH(0x4C25FD, Ebolt_DrawSecond_Disable, 0xA)
{
	if (EboltTemp::Data && EboltTemp::Data->Disable[1]) {
		return 0x4C262A;
	}

	return 0;
}

ASMJIT_PATCH(0x4C26EE, Ebolt_DrawThird_Disable, 0x6)
{
	if (EboltTemp::Data && EboltTemp::Data->Disable[2]) {
		return 0x4C2710;
	}

	return  0;
}

ASMJIT_PATCH(0x4C24BE, EBolt_Draw_Color1, 5)
{
	R->EAX(EboltTemp::Data->Color[0]);
	return 0x4C24E4;
}

ASMJIT_PATCH(0x4C25CB, EBolt_Draw_Color2, 5)
{
	R->Stack<int>(0x18, EboltTemp::Data->Color[1]);
	return 0x4C25FD;
}

ASMJIT_PATCH(0x4C26CF, EBolt_Draw_Color3, 5)
{
	R->EAX(EboltTemp::Data->Color[2]);
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

	if (auto pData = EboltExtData::Container.tryfind(pThis)) {
		if (!pData->ParticleSysEnabled) {
			return DWORD(_EBolt_Fire_Particles_RET);
		}

		pParticleSys = pData->pSys;
	}

	if (pParticleSys)
		GameCreate<ParticleSystemClass>(pParticleSys, pThis->Point2, nullptr, pThis->Owner, CoordStruct::Empty, pThis->Owner ? pThis->Owner->GetOwningHouse() : nullptr);

	return DWORD(_EBolt_Fire_Particles_RET);
}