//#include "Body.h"
//
//#define GET_TIMETO_BUILD(ret)\
//GET(TechnoClass*, pTech, ECX);\
//GET(int, nTime, EDX);\
//if (pTech) {\
//	nTime = static_cast<int>(BuildingTypeExt::GetExternalFactorySpeedBonus(pTech) * pTech->TimeToBuild()); }\
//R->EDX(nTime);\
//return ret;
//
//DEFINE_HOOK(0x4CA702, HouseClass_RecalcAllFactory_ExternalBonus, 0x9) //4
//{
//	GET_TIMETO_BUILD(0x4CA70D);
//}
//
//DEFINE_HOOK(0x4C9FB5, FactoryClass_TimeToBuild_ExternalBonus, 0x9) //4
//{
//	GET_TIMETO_BUILD(0x4C9FC0);
//}
//
//DEFINE_HOOK(0x4C9EEB, FactoryClass_Start_ExternalBonus, 0x9) //4
//{
//	GET_TIMETO_BUILD(0x4C9EF6);
//}
//#undef GET_TIMETO_BUILD