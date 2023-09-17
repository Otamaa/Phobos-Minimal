// list of hooks that not yet backported with some explanation with

// sub_48D1E0_PlayTaunt , idk what the f is happening there atm
// hWnd_UpdatePlayerColors , dont really care
// hWnd_SetPlayerColor , dont really care
// hWnd_PopulateWithCountryNames , dont really care
// hWnd_PopulateWithColors , dont really care
// hWnd_IsAvailableColor , dont really care
// hWnd_GetSlotColorIndex , dont really care

// VoxClass_SetEVAIndex , later maybe
// VoxClass_PlayEVASideSpecific , later maybe
// VoxClass_LoadFromINI , later maybe
// VoxClass_GetFilename
// VoxClass_DeleteAll
// VoxClass_CreateFromINIList
// VocClassData_AddSample

//TODO :
// better port these
// DEFINE_HOOK(6FB757, TechnoClass_UpdateCloak, 8)

/* TODO : Addition Weapon shenanegans , need to port whole TechnoClass::Update

*/

//TacticalClass_Draw_TimerVisibility  , the default value sometime initialized at NewSuper init function , and not readed from ini
	// if i want to replace this , i need to figure out the current NewSuper index doing
	// it is mostlikely change since there is new super were added !

// DEFINE_HOOK(6D4E79, TacticalClass_DrawOverlay_GraphicalText, 6) , hmm not now

//DEFINE_HOOK(0x4CAD00, FastMath_Cos_Replace, 0xA)
//{
//	GET_STACK(double, val, 0x4);
//	const auto nResult = Math::cos(val);
//	__asm { fld nResult };
//	return 0x4CAD48;
//}
//
//DEFINE_HOOK(0x4CB1A0 , FastMath_Cos_float_Replace , 0xA)
//{
//	GET_STACK(float, val, 0x4);
//	const auto nResult = Math::cos(val);
//	__asm { fld nResult };
//	return 0x4CB1F1;
//}
//
//DEFINE_HOOK(0x4CACB0, FastMath_Sin_Replace, 0xA)
//{
//	GET_STACK(double, val, 0x4);
//	const auto nResult = Math::sin(val);
//	__asm { fld nResult };
//	return 0x4CACF1;
//}
//
//DEFINE_HOOK(0x4CB150, FastMath_Sin_float_Replace, 0xA)
//{
//	GET_STACK(float, val, 0x4);
//	const auto nResult = Math::sin(val);
//	__asm { fld nResult };
//	return 0x4CB19A;
//}
//
//DEFINE_HOOK(0x4CAD50, FastMath_Tan_Replace, 0xA)
//{
//	GET_STACK(double, val, 0x4);
//	const auto nResult = Math::tan(val);
//	__asm { fld nResult };
//	return 0x4CAD7B;
//}
//
//DEFINE_HOOK(0x4CB320, FastMath_Tan_float_Replace, 0xA)
//{
//	GET_STACK(float, val, 0x4);
//	const auto nResult = Math::tan(val);
//	__asm { fld nResult };
//	return 0x4CB350;
//}
//
//DEFINE_HOOK(0x4CADE0, FastMath_ATan_Replace, 0x8)
//{
//	GET_STACK(double, val, 0x4);
//	const auto nResult = Math::atan(val);
//	__asm { fld nResult };
//	return 0x4CAE21;
//}
//
//DEFINE_HOOK(0x4CB480, FastMath_ATan_float_Replace, 0xA)
//{
//	GET_STACK(float, val, 0x4);
//	const auto nResult = Math::atan(val);
//	__asm { fld nResult };
//	return 0x4CB4BD;
//}
//
//DEFINE_HOOK(0x4CAE30, FastMath_ATan2_Replace, 0x5)
//{
//	GET_STACK(double, val, 0x4);
//	GET_STACK(double, val2, 0xC);
//	const auto nResult = Math::atan2(val , val2);
//	__asm { fld nResult };
//	return 0x4CAEE1;
//}
//
//DEFINE_HOOK(0x4CB3D0, FastMath_ATan2_float_Replace, 0xA)
//{
//	GET_STACK(float, val, 0x4);
//	GET_STACK(float, val2, 0xC);
//	const auto nResult = Math::atan2(val , val2);
//	__asm { fld nResult };
//	return 0x4CB472;
//}
//
//DEFINE_HOOK(0x4CAC40, FastMath_sqrt_Replace, 0xA)
//{
//	GET_STACK(double, val, 0x4);
//	const auto nResult = Math::sqrt(val);
//	__asm { fld nResult };
//	return 0x4CACAD;
//}
//
//DEFINE_HOOK(0x4CB060, FastMath_sqrt_float_Replace, 0xA)
//{
//	GET_STACK(float, val, 0x4);
//	const auto nResult = Math::sqrt(val);
//	__asm { fld nResult };
//	return 0x4CB0D5;
//}

//DEFINE_HOOK(0x71F1A2, TEventClass_HasOccured_DestroyedAll, 6)
//{
//	enum{ retfalse = 0x71F163 , rettrue = 0x71F1B1 };
//	GET(HouseClass*, pHouse, ESI);
//
//	if (pHouse->ActiveInfantryTypes.Total <= 0)
//	{
//		auto nPos = &pHouse->Buildings.Items[0];
//		const auto nEnd = &pHouse->Buildings.Items[pHouse->Buildings.Count];
//		for (auto& pBld : pHouse->Buildings)
//		{
//
//		}
//		if (nPos == nEnd)
//			return 0x71F1B1;
//
//		while (!(*nPos)->Type->CanBeOccupied || (*nPos)->Occupants.Count <= 0)
//		{
//			if (nPos++ == nEnd)
//				return 0x71F1B1;
//		}
//	}
//
//	return retfalse;
//}

//#include <EventClass.h>

//hmm dunno ,
//void Test(REGISTERS* R)
//{
//	GET(EventClass*, pEvent, EAX);
//
//	auto& Out = EventClass::OutList();
//	if (Out.Count < 128)
//	{
//		std::memcpy(&Out.List[Out.Tail], pEvent, sizeof(Out.List[Out.Tail])));
//		Out.Timings[Out.Tail] = Imports::TimeGetTime.get();
//	}
//

//DEFINE_HOOK(0x474E40, INIClass_GetMovementZone, 7)
//{
//	GET(INIClass*, pINI, ECX);
//	GET_STACK(const char*, Section, 0x4);
//	GET_STACK(const char*, Key, 0x8);
//	LEA_STACK(const char*, Default, 0xC);
//
//	if (pINI->ReadString(Section, Key, Default, Phobos::readBuffer)) {
//		if (!isdigit(Phobos::readBuffer[0])) {
//			for (size_t i = 0; i < TechnoTypeClass::MovementZonesToString.c_size(); ++i) {
//				if (!CRT::strcmpi(TechnoTypeClass::MovementZonesToString[i], Phobos::readBuffer)) {
//					R->EAX(i);
//					return 0x474E96;
//				}
//			}
//
//		} else {
//
//			const auto nValue = std::atoi(Phobos::readBuffer);
//			if ((size_t)nValue < TechnoTypeClass::MovementZonesToString.c_size()) {
//				R->EAX(nValue);
//				return 0x474E96;
//			}
//		}
//
//		Debug::INIParseFailed(Section, Key, Phobos::readBuffer, "Expect valid MovementZones");
//	}
//
//	R->EAX(0);
//	return 0x474E96;
//}