#include <Phobos.h>

bool NoCD = true;

// this douchebag blows your base up when it thinks you're cheating
DEFINE_OVERRIDE_SKIP_HOOK(0x55CFDF, CopyProtection_DontBlowMeUp, 0, 55D059);
//DEFINE_JUMP(LJMP, 0x55CFDF, 0x55D059);

DEFINE_OVERRIDE_HOOK(0x49F5C0, CopyProtection_IsLauncherRunning, 0x8)
{
	R->AL(1);
	return 0x49F61A;
}

DEFINE_OVERRIDE_HOOK(0x49F620, CopyProtection_NotifyLauncher, 0x5)
{
	R->AL(1);
	return 0x49F733;
}

DEFINE_OVERRIDE_HOOK(0x49F7A0, CopyProtection_CheckProtectedData, 0x8)
{
	R->AL(1);
	return 0x49F8A7;
}

DEFINE_OVERRIDE_HOOK(0x47AE36, _YR_CDFileClass_SetFileName, 8)
{
	GET(void*, CDControl, EAX);

	if (!CDControl || NoCD)
	{
		return 0x47AEF0;
	}
	return 0x47AE3E;
}

DEFINE_OVERRIDE_HOOK(0x47B026, _YR_FileFindOpen, 8)
{
	GET(void*, CDControl, EBX);

	if (!CDControl || NoCD)
	{
		return 0x47B0AE;
	}
	return 0x47B02E;
}

DEFINE_OVERRIDE_HOOK(0x4A80D0, CD_AlwaysFindYR, 6)
{
	if (NoCD)
	{
		R->EAX(2);
		return 0x4A8265;
	}
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x4790E0, CD_AlwaysAvailable, 7)
{
	if (NoCD)
	{
		R->AL(1);
		return 0x479109;
	}
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x479110, CD_NeverAsk, 5)
{
	if (NoCD)
	{
		R->AL(1);
		return 0x4791EA;
	}
	return 0;
}