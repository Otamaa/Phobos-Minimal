#include <Phobos.h>

bool NoCD = true;

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