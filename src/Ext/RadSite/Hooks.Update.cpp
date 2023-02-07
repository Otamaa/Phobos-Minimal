#include "Body.h"

DEFINE_HOOK(0x65B8C8, RadSiteClass_AI_Damaging, 0x5)
{
	enum { Expired = 0x65B8CF , Return = 0x65B8DC };
	GET(RadSiteClass*, pThis, ESI);




	return pThis->RadTimeLeft <= 0 ?
		Expired : Return;
}