#ifdef ENABLE_NEWCHECK

DEFINE_HOOK(0x4A23A8, CreditClass_Graphic_Logic_ReplaceCheck, 0x8)
{
	auto const pHouse = HouseClass::CurrentPlayer();
	return pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer") ?
		0x4A23B0 : 0x4A24F4;
}

DEFINE_HOOK(0x4A2614, CreditClass_Graphic_AI_ReplaceCheck, 0x8)
{
	auto const pHouse = HouseClass::CurrentPlayer();
	R->EAX(pHouse);
	return pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer") ?
		0x4A261D : 0x4A267D;
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

DEFINE_HOOK(0x501548, HouseClass_IsAllowedToAlly, 0x6)
{
	return R->ESI<HouseClass*>()->IsAlliedWith(R->EDI<HouseClass*>()) ? 0x501575 : 0x50157C;
}

DEFINE_HOOK(0x5015F2, HouseClass_IsAllowedToAlly_2, 0x6)
{
	return R->ESI<HouseClass*>()->IsAlliedWith(R->EAX<HouseClass*>()) ? 0x501627 : 0x501628;

}


DEFINE_HOOK(0x4FC4DF, HouseClass_MPlayer_Defeated, 0x6)
{
	GET(HouseClass*, pThis, EDX);
	GET(HouseClass*, pThat, EAX);

	return (!pThis->IsAlliedWith(pThat)
	  || !pThat->IsAlliedWith(pThis)) ? 0x4FC57C : 0x4FC52D;
}

DEFINE_HOOK(0x4F9CFA, HouseClass_MakeAlly_3, 0x7)
{
	GET(HouseClass*, pThis, ESI);
	GET(TechnoClass*, pThat, EAX);

	return pThis->IsAlliedWith(pThat->GetOwningHouse()) ? 0x4F9D34 : 0x4F9D40;
}

DEFINE_HOOK(0x4F9E10, HouseClass_MakeAlly_4, 0x8)
{
	GET(HouseClass*, pThis, ESI);
	GET(HouseClass*, pThat, EBP);

	return (!pThis || !pThis->IsAlliedWith(pThat))
		? 0x4F9EC9 : 0x4F9E49;
}

DEFINE_HOOK(0x4F9E5A, HouseClass_MakeAlly_5, 0x5)
{
	GET(HouseClass*, pThis, ESI);
	GET(HouseClass*, pThat, EBP);
	return (!pThis->IsAlliedWith(HouseClass::CurrentPlayer()) || !pThat->IsAlliedWith(HouseClass::CurrentPlayer())) ? 0x4F9EBD : 0x4F9EB1;
}

DEFINE_HOOK(0x4FAD64, HouseClass_SpecialWeapon_Update, 0x7)
{
	GET(HouseClass*, pThis, EDI);
	GET(BuildingClass*, pThat, ESI);

	return pThis->IsAlliedWith(pThat->GetOwningHouse()) ? 0x4FADD9 : 0x4FAD9E;
}

DEFINE_HOOK(0x50A23A, HouseClass_Target_Dominator, 0x6)
{
	GET(HouseClass*, pThis, EDI);
	GET(TechnoClass*, pThat, ESI);

	return pThis->IsAlliedWith(pThat->GetOwningHouse()) ? 0x50A292 : 0x50A278;
}

DEFINE_HOOK(0x50A04B, HouseClass_Target_GenericMutator, 0x7)
{
	GET(HouseClass*, pThis, EBX);
	GET(TechnoClass*, pThat, ESI);

	return pThis->IsAlliedWith(pThat->GetOwningHouse()) ? 0x50A096 : 0x50A087;
}

DEFINE_HOOK(0x5047F5, HouseClass_UpdateAngetNodes, 0x6)
{
	GET(HouseClass*, pThis, EAX);
	GET(HouseClass*, pThat, EDX);

	return pThis->IsAlliedWith(pThat) ? 0x504826 : 0x504820;
}

DEFINE_HOOK(0x5C98E5, MultiplayerScore_5C98A0, 0x6)
{
	GET(HouseClass*, pHouse, EDI);
	return (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer")) ? 0x5C9A7E : 0x5C98F1;
}

DEFINE_HOOK(0x6C6F83, SendStatisticsPacket, 0x6)
{
	auto const pHouse = HouseClass::CurrentPlayer();
	return (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer"))
		? 0x6C6F8B : 0x6C6F9D;
}

DEFINE_HOOK(0x6C7402, SendStatisticsPacket2, 0x8)
{
	GET(HouseClass*, pHouse, EAX);
	GET_STACK(int, nPlayerCount, 0x2C);

	if (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer"))
		return 0x6C7414;

	R->EBX(nPlayerCount);
	return 0x6C740A;
}

DEFINE_HOOK(0x6A55B7, SidebarClass_InitIO, 0x6)
{
	GET(HouseClass*, pHouse, EAX);
	return (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer")) ? 0x6A55CF : 0x6A55BF;
}

DEFINE_HOOK(0x6A5694, SidebarClass_InitIO2, 0x6)
{
	GET(HouseClass*, pHouse, ESI);
	return (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer")) ? 0x6A569C : 0x6A56AD;
}

DEFINE_HOOK(0x6A57EE, SidebarClass_InitIO3, 0x6)
{
	GET(HouseClass*, pHouse, EAX);
	return (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer")) ? 0x6A580E : 0x6A57F6;
}

DEFINE_HOOK(0x6A6AA6, SidebarClass_Scroll, 0x6)
{
	auto const pHouse = HouseClass::CurrentPlayer();
	R->EDX(pHouse);
	return (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer")) ? 0x6A6AB0 : 0x6A6AC6;
}

DEFINE_HOOK(0x6A7BA2, SidebarClass_Update, 0x5)
{
	GET(HouseClass*, pHouse, EBX);
	R->Stack(0x14, R->EDX());
	return  (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer")) ? 0x6A7BAF : 0x6A7BB7;
}

DEFINE_HOOK(0x6A7BE7, SidebarClass_Update_2, 0x6)
{
	GET(HouseClass*, pHouse, EBX);

	if (pHouse != HouseClass::Observer() && _strcmpi(pHouse->get_ID(), "Observer"))
		return 0x6A7C07;

	R->EAX(R->EDX());
	return 0x6A7BED;
}

DEFINE_HOOK(0x6A7CD9, SidebarClass_Update_3, 0x6)
{
	GET(HouseClass*, pHouse, EAX);
	return (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer")) ? 0x6A7CE3 : 0x6A7CE8;
}

DEFINE_HOOK(0x6A6B75, SidebarClass_handlestrips0, 0x6)
{
	R->Stack(0x10, R->EDX());
	auto const pHouse = HouseClass::CurrentPlayer();
	return (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer")) ? 0x6A6B7D : 0x6A6B85;
}

DEFINE_HOOK(0x6A6BCC, SidebarClass_handlestrips0_2, 0x6)
{
	GET(HouseClass*, pHouse, EBX);
	R->EAX(R->EDX());
	return (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer")) ? 0x6A6BD2 : 0x6A6BEC;
}

DEFINE_HOOK(0x6A6615, SidebarClass_togglestuff, 0x6)
{
	GET(HouseClass*, pHouse, EAX);
	return (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer")) ? 0x6A66EA : 0x6A6623;
}

DEFINE_HOOK(0x6A88D2, StripClass_6A8860, 0x6)
{
	auto const pHouse = HouseClass::CurrentPlayer();
	if (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer"))
		R->ESI(pHouse);

	return 0;
}

DEFINE_HOOK(0x8A898E, StripClass_6A8920, 0x6)
{
	GET(HouseClass*, pHouse, ESI);
	return (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer")) ? 0x6A8998 : 0x6A89B2;
}

DEFINE_HOOK(0x6A8A41, StripClass_6A89E0, 0x6)
{
	GET(HouseClass*, pHouse, EBX);
	R->ECX(R->EDX());
	return (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer")) ? 0x6A8A47 : 0x6A8A4C;;
}

DEFINE_HOOK(0x6A8AA8, StripClass_6A89E0_2, 0x6)
{
	GET(HouseClass*, pHouse, EBX);
	R->EAX(R->EDX());
	return (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer")) ? 0x6A8AAE : 0x6A8AD2;
}

DEFINE_HOOK(0x6A95BC, StripClass_DrawIt, 0x5)
{
	GET(StripClass*, pThis, ESI);

	auto const pHouse = HouseClass::CurrentPlayer();
	if (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer"))
		R->EAX(pThis->CameoCount);

	return 0x6A95C1;
}

DEFINE_HOOK(0x6AA04F, StripClass_DrawIt_2, 0x8)
{
	GET(HouseClass*, pHouse, EBX);
	return (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer")) ? 0x6AA057 : 0x6AA59B;;
}

DEFINE_HOOK(0x6A964E, StripClass_DrawIt_3, 0x6)
{
	auto const pHouse = HouseClass::CurrentPlayer();
	return (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer")) ? 0x6AA05B : 0x6A9654;
}

DEFINE_HOOK(0x6A8BB4, StripClass_Update, 0x5)
{
	GET(HouseClass*, pHouse, EBP);

	R->ESI(2 * R->EAX());

	return (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer")) ? 0x6A8BB9 : 0x6A8BCB;
}

DEFINE_HOOK(0x6A9038, StripClass_Update_2, 0x6)
{
	auto const pHouse = HouseClass::CurrentPlayer();
	return (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer")) ? 0x6A904B : 0x6A9258;

}

DEFINE_HOOK(0x6A9142, StripClass_Update_3, 0x6)
{
	GET(HouseClass*, pHouse, ESI);
	return (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer")) ? 0x6A914A : 0x6A915B;
}

DEFINE_HOOK(0x6A91EE, StripClass_Update_4, 0x5)
{
	GET(HouseClass*, pHouse, ESI);
	return (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer")) ? 0x6A91F7 : 0x6A9208;
}

#endif
