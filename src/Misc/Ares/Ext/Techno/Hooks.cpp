#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

DEFINE_OVERRIDE_HOOK(0x416C4D, AircraftClass_Carryall_Unload_DestroyCargo, 0x5)
{
	GET(AircraftClass*, pCarryall, EDI);
	GET(UnitClass*, pCargo, ESI);

	int Damage = pCargo->Health;
	pCargo->ReceiveDamage(&Damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, true, nullptr);

	Damage = pCarryall->Health;
	pCarryall->ReceiveDamage(&Damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, true, nullptr);

	return 0x416C53;
}
