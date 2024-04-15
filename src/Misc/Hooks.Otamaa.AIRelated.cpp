#include "Hooks.Otamaa.h"

#include <Ext/BuildingType/Body.h>
#include <Ext/Building/Body.h>

#include <FootClass.h>
#include <UnitClass.h>
#include <TeamClass.h>
#include <HouseClass.h>

DEFINE_HOOK(0x6EBB86, TeamClass_MoveToFocus_IsInStray, 0x6)
{
	GET(FootClass*, pFoot, ESI);
	GET(TeamClass*, pThis, EBP);

	if (pFoot->GetHeight() > 0 && pFoot->WhatAmI() == UnitClass::AbsID && pThis->Target)
	{
		auto nCoord = pFoot->GetCoords();
		auto nCoord_target = pThis->Target->GetCoords();
		R->EAX((int)nCoord_target.DistanceFrom(nCoord));
	}
	else
		R->EAX(pFoot->DistanceFrom(pThis->SpawnCell));

	return 0x6EBB91;
}

DEFINE_HOOK(0x6EBE69, TeamClass_MoveToFocus_SetDestination, 0xA)
{
	GET(FootClass*, pFoot, ESI);

	auto const pType = pFoot->GetTechnoType();

	return (pType->BalloonHover
		|| (pType->WhatAmI() == UnitTypeClass::AbsID
			&& static_cast<UnitTypeClass*>(pType)->JumpJet
			&& static_cast<UnitTypeClass*>(pType)->IsSimpleDeployer)) ?
		0x6EBE9C : 0x6EBE82;
}

DEFINE_HOOK(0x6EBEDB, TeamClass_MoveToFocus_BalloonHover, 0xA)
{
	GET(FootClass*, pFoot, ESI);

	auto const pType = pFoot->GetTechnoType();

	return (pType->BalloonHover
		|| (pType->WhatAmI() == UnitTypeClass::AbsID
			&& static_cast<UnitTypeClass*>(pType)->JumpJet
			&& static_cast<UnitTypeClass*>(pType)->IsSimpleDeployer)) ?

		0x6EBEEF : 0x6EBEFF;
}

//?TODO : replace these with patches
#pragma region HouseCheckReplace

//DEFINE_PATCH_TYPED(BYTE, 0x4FAD6B
//	, 0x50 // push eax
//	, 0x8B, 0xCF // mov ecx edi
//	, 0xE8, 0xDD , 0xEC , 0xFF , 0xFF // 4FAD6E , call HouseClass::IsAlliedWith
//	, 0x83, 0xF8 , 0x01
//	, 0x75, 0x64 // if ally , 4FAD7E -> 0x4FADD9
//	, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90
//	, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90
//	, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90
//	, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90
//);

DEFINE_HOOK(0x4FAD64, HouseClass_SpecialWeapon_Update, 0x7)
{
	GET(HouseClass*, pThis, EDI);
	GET(BuildingClass*, pThat, ESI);

	return pThis->IsAlliedWith(pThat->GetOwningHouse()) ? 0x4FADD9 : 0x4FAD9E;
}

//DEFINE_PATCH_TYPED(BYTE, 0x50A24D
//	, 0x50 // push eax
//	, 0x8B, 0xCF // mov ecx edi //
//	, 0xE8, 0xFB , 0x07 , 0x10 , 0x00 // 50A250 , call HouseClass::IsAlliedWith (0x4F9A50)
//	, 0x83, 0xF8 , 0x00
//	, 0x75, 0x64 // if not ally , -> 0x50A292 , change 0x64
//	  skip untils 50A278 , check the NOP amount
//	  skip untils 50A278 , check the NOP amount
//	, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90
//	, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90
//	, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90
//	, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90
//);

DEFINE_HOOK(0x50A23A, HouseClass_Target_Dominator, 0x6)
{
	GET(HouseClass*, pThis, EDI);
	GET(TechnoClass*, pThat, ESI);

	return pThis->IsAlliedWith(pThat->Owner) ? 0x50A292 : 0x50A278;
}

DEFINE_HOOK(0x50A04B, HouseClass_Target_GenericMutator, 0x7)
{
	GET(HouseClass*, pThis, EBX);
	GET(TechnoClass*, pThat, ESI);

	return pThis->IsAlliedWith(pThat->Owner) ? 0x50A096 : 0x50A087;
}

DEFINE_HOOK(0x5094F9, HouseClass_AdjustThreats, 0x6)
{
	return R->EBX<HouseClass*>()->IsAlliedWith(R->ESI<HouseClass*>()) ? 0x5095B6 : 0x509532;
}

DEFINE_HOOK(0x4F9432, HouseClass_Attacked, 0x6)
{
	return R->EDI<HouseClass*>()->IsAlliedWith(R->EAX<HouseClass*>()) ? 0x4F9474 : 0x4F9478;
}

DEFINE_HOOK(0x4FBD1C, HouseClass_DoesEnemyBuildingExist, 0x6)
{
	return R->ESI<HouseClass*>()->IsAlliedWith(R->EAX<HouseClass*>()) ? 0x4FBD57 : 0x4FBD47;
}

DEFINE_HOOK(0x5003BA, HouseClass_FindJuicyTarget, 0x6)
{
	return R->EDI<HouseClass*>()->IsAlliedWith(R->EAX<HouseClass*>()) ? 0x5003F7 : 0x5004B1;
}

DEFINE_HOOK(0x5047F5, HouseClass_UpdateAngetNodes, 0x6)
{
	return R->EAX<HouseClass*>()->IsAlliedWith(R->EDX<HouseClass*>()) ? 0x504826 : 0x504820;
}

DEFINE_HOOK(0x4F9A90, HouseClass_IsAlly_ObjectClass, 0x7)
{
	GET_STACK(TechnoClass*, pTarget, 0x4);
	GET(HouseClass*, pThis, ECX);

	bool result = false;
	if (pTarget)
	{
		result = pThis->IsAlliedWith(pTarget->GetOwningHouse());
	}

	R->AL(result);
	return 0x4F9ADE;
}

DEFINE_HOOK(0x4F9A50, HouseClass_IsAlly_HouseClass, 0x6)
{
	GET_STACK(HouseClass*, pTarget, 0x4);
	GET(HouseClass*, pThis, ECX);

	bool result = false;
	if (pTarget)
	{
		result = pThis->IsAlliedWith(pTarget->ArrayIndex);
	}

	R->AL(result);
	return 0x4F9A8C;
}

DEFINE_HOOK(0x4F9B0A, HouseClass_IsAlly_AbstractClass, 0x6)
{
	GET(HouseClass*, pThis, ESI);
	GET(HouseClass*, pThat, EAX);

	R->AL(pThis->IsAlliedWith(pThat));
	return 0x4F9B43;
}

DEFINE_HOOK(0x501548, HouseClass_IsAllowedToAlly_1, 0x6)
{
	return R->ESI<HouseClass*>()->IsAlliedWith(R->EDI<HouseClass*>()) ? 0x501575 : 0x50157C;
}

DEFINE_HOOK(0x5015F2, HouseClass_IsAllowedToAlly_2, 0x6)
{
	return 0x501628 - (int)R->ESI<HouseClass*>()->IsAlliedWith(R->EAX<HouseClass*>());
}

DEFINE_HOOK(0x4F9D01, HouseClass_MakeAlly_3, 0x6)
{
	return R->ESI<HouseClass*>()->IsAlliedWith(R->EAX<HouseClass*>()) ? 0x4F9D34 : 0x4F9D40;
}

DEFINE_HOOK(0x4F9E10, HouseClass_MakeAlly_4, 0x8)
{
	return R->EBP<HouseClass*>()->IsAlliedWith(R->ESI<HouseClass*>()) ? 0x4F9E49 : 0x4F9EC9;
}

DEFINE_HOOK(0x4F9E56, HouseClass_MakeAlly_5, 0x9)
{
	GET(HouseClass* , pHouse , EBP);
	GET(HouseClass* , pHouse_2 , ESI);

	if (!pHouse_2->IsAlliedWith(HouseClass::CurrentPlayer()))
		return 0x4F9EBD;

	return pHouse->IsAlliedWith(HouseClass::CurrentPlayer()) ? 0x4F9EB1 : 0x4F9EBD;
}

#ifdef _TODO
// 4F9A10 -> this->Allies
//main part
int __cdecl HouseClass_IsAlly_HouseIndex(REGISTERS* a1)
{
	unsigned int v2; // edi
	int v3; // eax

	if (!ExtendedHouse)
		return 0;
	v2 = *(a1->_ESP.data + 4);
	if (v2 == *(a1->_ECX.data + 48))
	{
		LOBYTE(a1->_EAX.data) = 1;
		return 0x4F9A1D;
	}
	else
	{
		if (v2 == -1)
		{
			LOBYTE(a1->_EAX.data) = 0;
		}
		else
		{
			v3 = ExtMap_Find(&HouseExtContainer, a1->_ECX.data);
			LOBYTE(a1->_EAX.data) = sub_100072A0((v3 + 0x78), v2);
		}
		return 0x4F9A1D;
	}
}

int __cdecl HouseClass_MakeAlly_1(int a1)
{
	int v2; // ecx
	int v3; // eax
	unsigned int v4; // esi
	_DWORD* v5; // edx
	int v6; // ecx

	if (!ExtendedHouse)
		return 0;
	v2 = ExtMap_Find(&HouseExtContainer, *(a1 + 12)) + 120;
	v3 = *(a1 + 16);
	if (*(v3 + 48) >= 0x100u)
		sub_10007220();
	v4 = *(v3 + 48) & 0x3F;
	v5 = (v2 + 8 * (*(v3 + 48) >> 6));
	v6 = 0;
	if (v4 >= 0x20)
		v6 = 1 << v4;
	*v5 |= v6 ^ (1 << v4);
	v5[1] |= v6;
	return 0x4F9BAF;
}

int __cdecl HouseClass_MakeAlly_2(int a1)
{
	int v2; // ecx
	int v3; // eax
	unsigned int v4; // esi
	_DWORD* v5; // edx
	int v6; // ecx

	if (!ExtendedHouse)
		return 0;
	v2 = ExtMap_Find(&HouseExtContainer, *(a1 + 12)) + 152;
	v3 = *(a1 + 16);
	if (*(v3 + 48) >= 0x100u)
		sub_10007220();
	v4 = *(v3 + 48) & 0x3F;
	v5 = (v2 + 8 * (*(v3 + 48) >> 6));
	v6 = 0;
	if (v4 >= 0x20)
		v6 = 1 << v4;
	*v5 |= v6 ^ (1 << v4);
	v5[1] |= v6;
	return 0x4F9C1F;
}

int __cdecl HouseClass_MakeEnemy(int a1)
{
	DWORD* v1; // esi
	int result; // eax
	DWORD* v3; // edi
	int v4; // eax
	unsigned int v5; // ebx
	_DWORD* v6; // edx
	int v7; // ecx
	unsigned int v8; // ebx
	_DWORD* v9; // edx
	int v10; // ecx
	int v11; // eax
	unsigned int v12; // ebx
	_DWORD* v13; // edx
	int v14; // ecx

	v1 = *(a1 + 8);
	if (ExtendedHouse)
	{
		v3 = *(a1 + 12);
		if (HouseClass::Is_Ally_WithHouse(v3, v1))
		{
			v4 = ExtMap_Find(&HouseExtContainer, v3);
			if (v1[12] >= 0x100)
				sub_10007220();
			v5 = v1[12] & 0x3F;
			v6 = (v4 + 120 + 8 * (v1[12] >> 6));
			v7 = 0;
			if (v5 >= 0x20)
				v7 = 1 << v5;
			*v6 &= ~(v7 ^ (1 << v5));
			v6[1] &= ~v7;
			if (MEMORY[0xA8E7AC])
			{
				if (v1[12] >= 0x100)
					sub_10007220();
				v8 = v1[12] & 0x3F;
				v9 = (v4 + 152 + 8 * (v1[12] >> 6));
				v10 = 0;
				if (v8 >= 0x20)
					v10 = 1 << v8;
				*v9 &= ~(v10 ^ (1 << v8));
				v9[1] &= ~v10;
			}
			HouseClass::Adjust_Threats(v3);
			if (HouseClass::Is_Ally_WithHouse(v1, v3))
			{
				*(a1 + 24) = 1;
				v11 = ExtMap_Find(&HouseExtContainer, v1);
				if (v3[12] >= 0x100)
					sub_10007220();
				v12 = v3[12] & 0x3F;
				v13 = (v11 + 120 + 8 * (v3[12] >> 6));
				v14 = 0;
				if (v12 >= 0x20)
					v14 = 1 << v12;
				*v13 &= ~(v14 ^ (1 << v12));
				v13[1] &= ~v14;
				if (MEMORY[0xA8E7AC])
					sub_10007230((v11 + 152), v3[12], 0);
				HouseClass::Adjust_Threats(v1);
				HouseClass::Update_Anger_Nodes(v1, 1, v1);
			}
			return 0x4FA0E4;
		}
		else
		{
			return 0x4FA1DA;
		}
	}
	else
	{
		result = 0x4FA1DA;
		if (v1)
			return 0x4F9FEF;
	}
	return result;
}

int __cdecl HouseClass_PlayerDefeat_ChangeOwner(REGISTERS* a1)
{
	unsigned int data; // ebx
	DWORD** v2; // edi
	DWORD** v3; // ebp
	DWORD v4; // esi
	_DWORD* v5; // ebp
	int v7; // ecx
	int v8; // eax
	REGISTERS* v9; // [esp+14h] [ebp+4h]

	data = a1->_ESI.data;
	if (!*(RulesExt::Global + 0x12C))
		return 0;
	v2 = MEMORY[0xA8022C];
	v3 = &MEMORY[0xA8022C][MEMORY[0xA80238]];
	v9 = v3;
	if (MEMORY[0xA8022C] == v3)
		return 0;
	while (1)
	{
		v4 = *v2;
		if (*v2 == data
		  || *(v4 + 501)
		  || v4 == MEMORY[0xAC1198]
		  || !strcmpi((*(v4 + 52) + 36), "Observer")
		  || !HouseClass::Is_Player_Control(v4)
		  || !HouseClass::Is_Ally_WithHouse(data, v4))
		{
			goto LABEL_11;
		}
		v5 = ExtMap_Find(&HouseExtContainer, data);
		if (v5)
			break;
		v3 = v9;
	LABEL_11:
		if (++v2 == v3)
			return 0;
	}
	v7 = *(data + 0x24);
	*(data + 502) = 0;
	v8 = (*(v7 + 0x18))(data + 0x24);
	if (v8 <= 0)
		HouseClass::Spend_Money(v4, -v8);
	else
		HouseClass::Refund_Money(v4, v8);
	ResetToNeutral(v5, v4);
	return 0x4F87FF;
}

DWORD __thiscall ResetToNeutral(struct_this_1* this, int a2)
{
	int v2; // esi
	int dword4; // edi
	DWORD** v4; // ebp
	DWORD result; // eax
	DWORD i; // ecx

	v2 = MEMORY[0xA8EC7C];
	dword4 = this->dword4;
	v4 = &MEMORY[0xA8EC7C][MEMORY[0xA8EC88]];
	if (MEMORY[0xA8EC7C] != v4)
	{
		do
		{
			if (*(*v2 + 0x21C) == dword4)
				(*(**v2 + 0x3D4))(*v2, a2, 0);
			v2 += 4;
		}
		while (v2 != v4);
	}
	MapClass::Reset_CellIterator(0x87F7E8u);
	result = MapClass::Get_Cell_From_Iterator(0x87F7E8u);
	for (i = result; result; i = result)
	{
		if (*(i + 0x50) == *(dword4 + 0x30))
			*(i + 0x50) = *(a2 + 48);
		result = MapClass::Get_Cell_From_Iterator(0x87F7E8u);
	}
	return result;
}

int __cdecl HouseClass_ReadINI_MakeAllies(REGISTERS* a1)
{
	unsigned int v2; // esi
	unsigned int v3; // eax
	int v4; // ecx

	if (!ExtendedHouse)
		return 0;
	v2 = *(a1->_EAX.data + 48);
	if (v2 >= 0x100)
		sub_10007220();
	v3 = *(a1->_EAX.data + 48) & 0x3F;
	v4 = 0;
	if (v3 >= 0x20)
		v4 = 1 << v3;
	if (__PAIR64__(
		*(*(a1->_ESP.data + 16) + 8 * (v2 >> 6) + 4) & v4,
		*(*(a1->_ESP.data + 16) + 8 * (v2 >> 6)) & (v4 ^ (1 << v3))))
	{
		return 0x5010DD;
	}
	else
	{
		return 0x5010E7;
	}
}
#endif

#pragma endregion

DEFINE_HOOK(0x6EA192, TeamClass_Regroup_LimboDelivered, 0x6)
{
	enum { advance = 0x6EA38C, ret = 0x0 };
	GET(BuildingClass*, pBuilding, ESI);
	return BuildingExtContainer::Instance.Find(pBuilding)->LimboID != -1 ?
		advance : ret;
}

DEFINE_HOOK(0x6EEC6D, TeamClass_FindTargetBuilding_LimboDelivered, 0x6)
{
	enum { advance = 0x6EEE45, ret = 0x0 };
	GET(BuildingClass*, pBuilding, ESI);
	return BuildingExtContainer::Instance.Find(pBuilding)->LimboID != -1 ?
		advance : ret;
}

DEFINE_HOOK(0x6EE8D9, TeamClass_Scout_LimboDelivered, 0x9)
{
	enum { advance = 0x6EE928, ret = 0x0 };
	GET(BuildingClass**, pBuilding, ESI);
	return BuildingExtContainer::Instance.Find(*pBuilding)->LimboID != -1 ?
		advance : ret;
}

DEFINE_HOOK(0x6EEEF2, TeamClass_6EEEA0_LimboDelivered, 0xA)
{
	enum { advance = 0x6EF0D7, ret = 0x0 };
	GET(BuildingClass*, pBuilding, ESI);
	return BuildingExtContainer::Instance.Find(pBuilding)->LimboID != -1 ?
		advance : ret;
}

DEFINE_HOOK(0x6F7D90, TechnoClass_Threat_Forbidden, 0x6)
{
	GET(ObjectClass*, pTarget, ESI);

	if (pTarget->InLimbo || !pTarget->IsAlive)
		return 0x6F894F;

	if (const auto pTechno = generic_cast<TechnoClass*>(pTarget))
	{

		if (pTechno->IsCrashing || pTechno->IsSinking)
			return 0x6F894F;

		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pTechno->GetTechnoType());

		if (pTypeExt->IsDummy)
			return 0x6F894F;

		switch (pTechno->WhatAmI())
		{
		case AbstractType::Building:
		{
			const auto pBld = (BuildingClass*)pTarget;

			if (BuildingExtContainer::Instance.Find(pBld)->LimboID != -1)
				return 0x6F894F;

			break;
		}
		case AbstractType::Unit:
		{

			const auto pUnit = (UnitClass*)pTarget;

			if (pUnit->DeathFrameCounter > 0)
				return 0x6F894F;

			break;
		}
		default:
			break;
		}
	}

	return 0x6F7D9E;
}
