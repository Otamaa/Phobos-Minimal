#include <Phobos.h>
#include <Utilities/Macro.h>

// this douchebag blows your base up when it thinks you're cheating
// this already handled below , the validation will aways return true , this prevent  this to even happen
//DEFINE_SKIP_HOOK(0x55CFDF, CopyProtection_DontBlowMeUp,0x7, 55D059);
//DEFINE_PATCH(0x55CFD8 , 0x90, 0x90, 0x90, 0x90, 0x90);

//void NAKED CopyProtection_DontBlowMeUp_Ret_()
//{
//	PUSH_REG(ebx);
//	PUSH_REG(esi);
//	JMP(0x55D059);
//}
//DEFINE_HOOK(0x55CFD8, CopyProtection_DontBlowMeUp, 0x5) {
//	return (DWORD)CopyProtection_DontBlowMeUp_Ret_;
//}

// Allows run game without the launcher
DEFINE_PATCH(0x49F5C0,    // CopyProtect_IsLauncherRunning
	0xB0, 0x01,           // mov    al, 1
	0xC3);                // retn

DEFINE_PATCH(0x49F620,    // CopyProtect_NotifyLauncher
	0xB0, 0x01,           // mov    al, 1
	0xC3);                // retn

DEFINE_PATCH(0x49F7A0,    // CopyProtect_Validate
	0xB0, 0x01,           // mov    al, 1
	0xC3);                // retn

DEFINE_STRONG_HOOK(0x47AE36, _YR_CDFileClass_SetFileName, 8)
{
	GET(void*, CDControl, EAX);

	if (!CDControl || Phobos::Otamaa::NoCD)
	{
		return 0x47AEF0;
	}
	return 0x47AE3E;
}

DEFINE_STRONG_HOOK(0x47B026, _YR_FileFindOpen, 8)
{
	GET(void*, CDControl, EBX);

	if (!CDControl || Phobos::Otamaa::NoCD)
	{
		return 0x47B0AE;
	}
	return 0x47B02E;
}

DEFINE_STRONG_HOOK(0x4A80D0, CD_AlwaysFindYR, 6)
{
	if (Phobos::Otamaa::NoCD)
	{
		R->EAX(2);
		return 0x4A8265;
	}
	return 0;
}

DEFINE_STRONG_HOOK(0x4790E0, CD_AlwaysAvailable, 7)
{
	if (Phobos::Otamaa::NoCD)
	{
		R->AL(1);
		return 0x479109;
	}
	return 0;
}

DEFINE_STRONG_HOOK(0x479110, CD_NeverAsk, 5)
{
	if (Phobos::Otamaa::NoCD)
	{
		R->AL(1);
		return 0x4791EA;
	}
	return 0;
}