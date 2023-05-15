 #include "Body.h"

#include <Ext/House/Body.h>

DEFINE_HOOK(0x4401BB, Factory_AI_PickWithFreeDocks, 0x6) //was C
{
	GET(BuildingClass*, pBuilding, ESI);

	if (Phobos::Config::AllowParallelAIQueues)
		return 0;

	if (!pBuilding)
		return 0;

	const HouseClass* pOwner = pBuilding->Owner;

	if (!pOwner)
		return 0;

	if (pOwner->IsCurrentPlayer() || pOwner->IsNeutral())
		return 0;

	if (pBuilding->Type->Factory == AbstractType::AircraftType)
	{
		if (pBuilding->Factory
			&& !BuildingExt::HasFreeDocks(pBuilding))
		{
			BuildingExt::UpdatePrimaryFactoryAI(pBuilding);
		}
	}

	return 0;
}

DEFINE_HOOK(0x443CCA, BuildingClass_KickOutUnit_AircraftType_Phobos, 0xA)
{
	GET(HouseClass*, pHouse, EDX);

	if (Phobos::Config::AllowParallelAIQueues || Phobos::Config::ForbidParallelAIQueues_Aircraft)
		return 0;

	HouseExt::ExtMap.Find(pHouse)->Factory_AircraftType = nullptr;
	return 0;
}

DEFINE_HOOK(0x44531F, BuildingClass_KickOutUnit_BuildingType_Phobos, 0xA)
{
	GET(HouseClass*, pHouse, EAX);

	if (Phobos::Config::AllowParallelAIQueues || Phobos::Config::ForbidParallelAIQueues_Building)
		return 0;

	HouseExt::ExtMap.Find(pHouse)->Factory_BuildingType = nullptr;
	return 0;
}

DEFINE_HOOK(0x444131, BuildingClass_KickOutUnit_InfantryType_Phobos, 0x6)
{
	GET(HouseClass*, pHouse, EAX);

	if (Phobos::Config::AllowParallelAIQueues || Phobos::Config::ForbidParallelAIQueues_Infantry)
		return 0;

	HouseExt::ExtMap.Find(pHouse)->Factory_InfantryType = nullptr;
	return 0;
}

DEFINE_HOOK(0x444119, BuildingClass_KickOutUnit_UnitType_Phobos, 0x6)
{
	GET(UnitClass*, pUnit, EDI);
	GET(BuildingClass*, pFactory, ESI);

	if (!Phobos::Config::AllowParallelAIQueues)
		return 0;

	if(HouseExt::ExtData* pData = HouseExt::ExtMap.Find(pFactory->Owner)){

		if (!pUnit->Type->Naval) {
			if (Phobos::Config::ForbidParallelAIQueues_Vehicle)
				pData->Factory_VehicleType = nullptr;
		} else {
			if (Phobos::Config::ForbidParallelAIQueues_Navy)
				pData->Factory_NavyType = nullptr; 
		}
	}

	return 0;
}

DEFINE_HOOK(0x4CA07A, FactoryClass_AbandonProduction_Phobos, 0x8)
{
	GET(FactoryClass*, pFactory, ESI);

	if (!Phobos::Config::AllowParallelAIQueues || !pFactory->Owner)
		return 0;

	if(HouseClass* pOwner = pFactory->Owner) {
	HouseExt::ExtData* pData = HouseExt::ExtMap.Find(pOwner);
	TechnoClass* pTechno = pFactory->Object;

	switch ((((DWORD*)pTechno)[0]))
	{
	case BuildingClass::vtable:
		if (Phobos::Config::ForbidParallelAIQueues_Building)
			pData->Factory_BuildingType = nullptr;
		break;
	case UnitClass::vtable:
		if (!pTechno->GetTechnoType()->Naval)
		{
			if (Phobos::Config::ForbidParallelAIQueues_Vehicle)
				pData->Factory_VehicleType = nullptr;
		}
		else
		{
			if (Phobos::Config::ForbidParallelAIQueues_Navy)
				pData->Factory_NavyType = nullptr;
		}
		break;
	case InfantryClass::vtable:
		if (Phobos::Config::ForbidParallelAIQueues_Infantry)
			pData->Factory_InfantryType = nullptr;
		break;
	case AircraftClass::vtable:
		if (Phobos::Config::ForbidParallelAIQueues_Aircraft)
			pData->Factory_AircraftType = nullptr;
		break;
	}
	}

	return 0;
}

DEFINE_HOOK(0x4502F4, BuildingClass_Update_Factory_Phobos, 0x6)
{
	enum { Skip = 0x4503CA };

	GET(BuildingClass*, pBuilding, ESI);
	HouseClass* pOwner = pBuilding->Owner;

	if (pOwner && pOwner->Production && Phobos::Config::AllowParallelAIQueues)
	{
		HouseExt::ExtData* pData = HouseExt::ExtMap.Find(pOwner);
		BuildingClass** currFactory = nullptr;

		switch (pBuilding->Type->Factory)
		{
		case AbstractType::BuildingType:
			currFactory = &pData->Factory_BuildingType;
			break;
		case AbstractType::UnitType:
			if (!pBuilding->Type->Naval)
				currFactory = &pData->Factory_VehicleType;
			else
				currFactory = &pData->Factory_NavyType;
			break;
		case AbstractType::InfantryType:
			currFactory = &pData->Factory_InfantryType;
			break;
		case AbstractType::AircraftType:
			currFactory = &pData->Factory_AircraftType;
			break;
		default:
			break;
		}

		if (!currFactory)
		{
			*currFactory = pBuilding;
			return 0;
		}else if (!*currFactory)
		{
			*currFactory = pBuilding;
			return 0;
		}
		else if (*currFactory != pBuilding)
		{
			switch (pBuilding->Type->Factory)
			{
			case AbstractType::BuildingType:
			{
				if(Phobos::Config::ForbidParallelAIQueues_Building)
					return Skip;
			}
			break;
			case AbstractType::UnitType:
			{
				if(Phobos::Config::ForbidParallelAIQueues_Vehicle && !pBuilding->Type->Naval)
					return Skip;
				else if (Phobos::Config::ForbidParallelAIQueues_Navy && pBuilding->Type->Naval)
					return Skip;
			}
			break;
			case AbstractType::InfantryType:
			{
				if (Phobos::Config::ForbidParallelAIQueues_Infantry)
					return Skip;
			}
			break;
			case AbstractType::AircraftType:
			{
				if (Phobos::Config::ForbidParallelAIQueues_Aircraft)
					return Skip;
			}
			break;
			default:
				break;
			}
		}
	}

	return 0;
}