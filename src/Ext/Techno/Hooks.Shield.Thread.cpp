#include "Body.h"
#include <Ext/TechnoType/Body.h>

// ASMJIT_PATCH(0x708BA3, TechnoClass_ThreatPosed_Shield, 0x6)
// {
// 	GET(TechnoClass*, pThis, ESI);
// 	R->EAX(TechnoExtData::GetThreadPosed(pThis));
// 	return 0x708BA9;
// }

// ASMJIT_PATCH(0x708B9A, TechnoClass_ThreadPosed_BunkerLinked_Shield, 0x6)
// {
// 	GET(TechnoClass*, pThis, ECX);
// 	R->EAX(TechnoExtData::GetThreadPosed(pThis));
// 	return 0x708BA0;
// }

// ASMJIT_PATCH(0x4F6B97, HouseClass_Apparent_Category_Power_Shield, 0x6)
// {
// 	GET(TechnoClass*, pThis, ESI);
// 	R->ECX(TechnoExtData::GetThreadPosed(pThis));
// 	return 0x4F6B9D;
// }

// ASMJIT_PATCH(0x4F6B27, HouseClass_Category_Power_Shield, 0x6)
// {
// 	GET(TechnoClass*, pThis, ESI);
// 	R->ECX(TechnoExtData::GetThreadPosed(pThis));
// 	return 0x4F6B2D;
// }
