
#ifdef CHECK_PTR_VALID
DEFINE_STRONG_HOOK_AGAIN(0x4F9A10, HouseClass_IsAlliedWith, 0x6)
DEFINE_STRONG_HOOK_AGAIN(0x4F9A50, HouseClass_IsAlliedWith, 0x6)
DEFINE_STRONG_HOOK_AGAIN(0x4F9AF0, HouseClass_IsAlliedWith, 0x7)
DEFINE_STRONG_HOOK(0x4F9A90, HouseClass_IsAlliedWith, 0x7)
{
	GET(HouseClass*, pThis, ECX);
	GET_STACK(DWORD, called, 0x0);

	if (!pThis || VTable::Get(pThis) != HouseClass::vtable)
	{
		Debug::FatalError("HouseClass - IsAlliedWith[%x] , Called from[%x] with `nullptr` pointer !", R->Origin(), called);
	}

	return 0;
}
#endif

#ifdef CheckForMapSaveCrash
DEFINE_HOOK(0x50126A, HouseClass_WritetoIni0, 0x6)
{
	GET(HouseClass*, pThis, EBX);

	if (IS_SAME_STR_("USSR", pThis->Type->ID))
		Debug::LogInfo("Writing to ini TechLevel for[%s]", pThis->Type->ID);

	return 0x0;
}

DEFINE_HOOK(0x501284, HouseClass_WritetoIni2, 0x6)
{
	GET(HouseClass*, pThis, EBX);

	if (IS_SAME_STR_("USSR", pThis->Type->ID))
		Debug::LogInfo("Writing to ini InitialCredit for[%s]", pThis->Type->ID);

	return 0x0;
}

DEFINE_HOOK(0x5012AF, HouseClass_WritetoIni3, 0x6)
{
	GET(HouseClass*, pThis, EBX);

	if (IS_SAME_STR_("USSR", pThis->Type->ID))
		Debug::LogInfo("Writing to ini0 Control_IQ for[%s]", pThis->Type->ID);

	return 0x0;
}

DEFINE_HOOK(0x5012C9, HouseClass_WritetoIni4, 0x6)
{
	GET(HouseClass*, pThis, EBX);

	if (IS_SAME_STR_("USSR", pThis->Type->ID))
		Debug::LogInfo("Writing to ini0 Control_Edge for[%s]", pThis->Type->ID);

	return 0x0;
}

DEFINE_HOOK(0x5012E1, HouseClass_WritetoIni5, 0x6)
{
	GET(HouseClass*, pThis, EBX);

	if (IS_SAME_STR_("USSR", pThis->Type->ID))
		Debug::LogInfo("Writing to ini0 PlayerControl for[%s]", pThis->Type->ID);

	return 0x0;
}

DEFINE_HOOK(0x5012F9, HouseClass_WritetoIni6, 0x6)
{
	GET(HouseClass*, pThis, EBX);

	if (IS_SAME_STR_("USSR", pThis->Type->ID))
		Debug::LogInfo("Writing to ini0 Color for[%s]", pThis->Type->ID);

	return 0x0;
}

DEFINE_HOOK(0x50134C, HouseClass_WritetoIni7, 0x5)
{
	GET(HouseClass*, pThis, EBX);

	if (IS_SAME_STR_("USSR", pThis->Type->ID))
		Debug::LogInfo("Writing to ini0 Allies for[%s]", pThis->Type->ID);

	return 0x0;
}

DEFINE_HOOK(0x50136E, HouseClass_WritetoIni8, 0x5)
{
	GET(HouseClass*, pThis, EBX);

	if (IS_SAME_STR_("USSR", pThis->Type->ID))
		Debug::LogInfo("Writing to ini0 UINAME for[%s]", pThis->Type->ID);

	return 0x0;
}

DEFINE_HOOK(0x501380, HouseClass_WritetoIni9, 0xA)
{
	GET(HouseClass*, pThis, EBX);

	if (IS_SAME_STR_("USSR", pThis->Type->ID))
		Debug::LogInfo("Writing to ini0 Base for[%s]", pThis->Type->ID);

	return 0x0;
}

DEFINE_HOOK(0x42ED72, BaseClass_WriteToINI1, 0x7)
{
	GET(BaseClass*, pThis, ESI);
	HouseClass* ptr = reinterpret_cast<HouseClass*>((DWORD)pThis - offsetof(HouseClass, Base));

	if (IS_SAME_STR_("USSR", ptr->Type->ID))
		Debug::LogInfo("Writing to ini0 Base PercentBuilt for[%s]", ptr->Type->ID);

	return 0x0;
}

DEFINE_HOOK(0x42ED8C, BaseClass_WriteToINI2, 0x5)
{
	GET(BaseClass*, pThis, ESI);
	HouseClass* ptr = reinterpret_cast<HouseClass*>((DWORD)pThis - offsetof(HouseClass, Base));

	if (IS_SAME_STR_("USSR", ptr->Type->ID))
		Debug::LogInfo("Writing to ini0 Base NodeCount(%d) for[%s]", pThis->BaseNodes.Count, ptr->Type->ID);

	return 0x0;
}
#endif