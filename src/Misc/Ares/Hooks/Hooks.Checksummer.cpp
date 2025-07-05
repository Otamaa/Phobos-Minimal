#include <Phobos.h>
#include "AresChecksummer.h"

#include <Helpers/Macro.h>

ASMJIT_PATCH(0x4A1C10, Checksummer_Add_BYTE, 5)
{
	GET(CRCEngine*, pThis, ECX);
	REF_STACK(BYTE, value, STACK_OFFS(0x0, -0x4));
	R->EAX(pThis->operator()(value));
	return 0x4A1C8E;
}

ASMJIT_PATCH(0x4A1CA0, Checksummer_Add_bool, 5)
{
	GET(CRCEngine*, pThis, ECX);
	REF_STACK(const bool, value, STACK_OFFS(0x0, -0x4));
	R->EAX(pThis->operator()(value));
	return 0x4A1D23;
}

ASMJIT_PATCH(0x4A1D30, Checksummer_Add_WORD, 5)
{
	GET(CRCEngine*, pThis, ECX);
	REF_STACK(const WORD, value, STACK_OFFS(0x0, -0x4));
	R->EAX(pThis->operator()(value));
	return 0x4A1D46;
}

ASMJIT_PATCH(0x4A1D50, Checksummer_Add_DWORD, 8)
{
	GET(CRCEngine*, pThis, ECX);
	REF_STACK(const DWORD, value, STACK_OFFS(0x0, -0x4));
	R->EAX(pThis->operator()(value));
	return 0x4A1D64;
}

ASMJIT_PATCH(0x4A1D70, Checksummer_Add_float, 8)
{
	GET(CRCEngine*, pThis, ECX);
	REF_STACK(const float, value, STACK_OFFS(0x0, -0x4));
	R->EAX(pThis->operator()(value));
	return 0x4A1D84;
}

ASMJIT_PATCH(0x4A1D90, Checksummer_Add_double, 8)
{
	GET(CRCEngine*, pThis, ECX);
	REF_STACK(const double, value, STACK_OFFS(0x0, -0x4));
	R->EAX(pThis->operator()(value));
	return 0x4A1DAC;
}

ASMJIT_PATCH(0x4A1DE0, Checksummer_Add_Buffer, 6)
{
	GET(CRCEngine*, pThis, ECX);
	GET_STACK(const void*, data, STACK_OFFS(0x0, -0x4));
	GET_STACK(size_t, length, STACK_OFFS(0x0, -0x8));
	R->EAX(pThis->operator()(data ,length));
	return 0x4A1FA6;
}