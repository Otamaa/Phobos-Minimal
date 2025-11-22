
#pragma region House32LimitFix
//HelperedVector<HouseTypeClass*> Allies;
//HelperedVector<HouseTypeClass*> StartingAllies;
//HelperedVector<HouseTypeClass*> RadarVisibleTo;
//HelperedVector<HouseTypeClass*> TechnoClass_DisplayProductionTo;
// 0x5788 Allies
// 0x1605C StartingAllies
// 0x54E4 RadarVisibleTo
// 0x20D TechnoClass_DisplayProductionTo

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
		  || !HouseClass::IsControlledByHuman(v4)
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
