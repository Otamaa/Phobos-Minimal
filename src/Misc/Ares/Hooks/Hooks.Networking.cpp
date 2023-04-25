#include <NetworkEvents.h>

#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

// was desynct all time 
// not sure WTF is happening
namespace Fix
{
	static constexpr reference<uint8_t, 0x8208ECu,36u> const EventLengthArr {};

	unsigned int EventLength_Hares(int nInput)
	{
		if (nInput == 0x80)
			return 10;
		if (nInput == 0x81 || nInput == 0x82)
			return 5;

		return NetworkEvent::EventLength[nInput];
	}

	uint8_t NOINLINE EventLength(uint8_t nInput)
	{
		if (nInput <= 0x2Eu)
			return Fix::EventLengthArr[nInput];
		if (nInput == 0x60u)
			return 10u;
		if (nInput == 0x61u)
			return 5u;

		return 0;
	}
};

DEFINE_OVERRIDE_HOOK(0x64C314, sub_64BDD0_PayloadSize2, 0x8)
{
	GET(uint8_t, nSize, ESI);

	const auto nFix = Fix::EventLength(nSize);

	R->ECX(nFix);
	R->EBP(nFix + (nSize == 4u));

	return 0x64C321;
}

DEFINE_OVERRIDE_HOOK(0x64BE83, sub_64BDD0_PayloadSize1, 0x8)
{
	GET(uint8_t, nSize, EDI);

	const auto nFix = Fix::EventLength(nSize);

	R->ECX(nFix);
	R->EBP(nFix);
	R->Stack(0x20, nFix);

	return nSize == 4 ? 0x64BF1A : 0x64BE97;
}

DEFINE_OVERRIDE_HOOK(0x64B704, sub_64B660_PayloadSize, 0x8)
{
	GET(uint8_t, nSize, EDI);

	const auto nFix = Fix::EventLength(nSize);

	R->EDX(nFix);
	R->EBP(nFix);

	return (nSize == 0x1Fu) ? 0x64B710 : 0x64B71D;
}