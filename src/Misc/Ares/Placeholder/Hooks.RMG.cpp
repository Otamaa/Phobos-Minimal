class RMG
{
public:
	static bool UrbanAreas;
	static bool UrbanAreasRead;
	static int UrbanStructuresReadSoFar;
	static VectorNames UrbanStructures;
	static VectorNames UrbanVehicles;
	static VectorNames UrbanInfantry;
};

bool RMG::UrbanAreas = 0;
bool RMG::UrbanAreasRead = 0;

int RMG::UrbanStructuresReadSoFar;
VectorNames RMG::UrbanStructures;
VectorNames RMG::UrbanVehicles;
VectorNames RMG::UrbanInfantry;

DEFINE_OVERRIDE_HOOK(0x5A6998, MapSeedClass_Generate_PlaceUrbanFoots, 5)
{
	int Length = RMG::UrbanInfantry.Count() + RMG::UrbanVehicles.Count();
	if (Length == 0)
	{
		return 0x5A6B96; // no possible items - nothing to do
	}

	auto Index = Random2Class::NonCriticalRandomNumber()(0, Length - 1);

	GET(HouseClass*, Owner, EBP);
	ObjectClass* Item = nullptr;
	if (Index < RMG::UrbanInfantry.Count())
	{
		if (auto const IType = InfantryTypeClass::Find(RMG::UrbanInfantry[Index]))
		{
			Item = IType->CreateObject(Owner);
		}
		else
		{
			Debug::Log("Unknown InfantryType %s in RMG config!\n", RMG::UrbanInfantry[Index]);
		}
	}
	else
	{
		Index -= RMG::UrbanInfantry.Count();
		if (auto const UType = UnitTypeClass::Find(RMG::UrbanVehicles[Index]))
		{
			Item = UType->CreateObject(Owner);
		}
		else
		{
			Debug::Log("Unknown VehicleType %s in RMG config!\n", RMG::UrbanVehicles[Index]);
		}
	}
	R->ESI<ObjectClass*>(Item);

	return 0x5A6A31;
}

DEFINE_OVERRIDE_HOOK(0x5982D5, MapSeedClass_LoadFromINI, 6)
{
	if (!RMG::UrbanAreasRead)
	{
		GET(CCINIClass*, pINI, EDI);
		RMG::UrbanAreas = pINI->ReadBool("General", "GenerateUrbanAreas", RMG::UrbanAreas);

		//I can should this be theater-related in the future... ~pd

		pINI->ReadString("Urban", "Structures",
			"CABUNK01,CABUNK02,CAARMY01,CAARMY02,CAARMY03,CAARMY04,CACHIG03,CANEWY01,CANEWY14,CANWY09,CANWY26,CANWY25,CATEXS07",
			Ares::readBuffer);
		RMG::UrbanStructures.Tokenize(Ares::readBuffer);

		pINI->ReadString("Urban", "Infantry",
			"CIV1,CIV2,CIV3,CIVA,CIVB,CIVC",
			Ares::readBuffer);
		RMG::UrbanInfantry.Tokenize(Ares::readBuffer);

		pINI->ReadString("Urban", "Vehicles",
			"TRUCKA,TRUCKB,COP,EUROC,SUVW,SUVB,FTRK,AMBU"
			, Ares::readBuffer);
		RMG::UrbanVehicles.Tokenize(Ares::readBuffer);

		RMG::UrbanAreasRead = 1;
	}
	return 0;
}

// #882 select from all available options and randomize urban areas
DEFINE_OVERRIDE_HOOK(0x596786, MapSeedClass_DialogFunc_SurpriseMe, 9)
{
	GET(HWND, hDlg, EBP);
	Random2Class* pRand = &Random2Class::NonCriticalRandomNumber();
	MapSeedClass* pMapSeed = &MapSeedClass::Global();

	// selects map terrain type from all the items in the combobox
	if (HWND hDlgItem = hDlgItem = GetDlgItem(hDlg, 0x405))
	{
		int count = SendMessageA(hDlgItem, CB_GETCOUNT, 0, 0);
		int index = pRand->RandomRanged(0, count - 1);
		int itemdata = SendMessageA(hDlgItem, CB_GETITEMDATA, index, 0);
		pMapSeed->MapType = itemdata;
	}

	// selects theater / climate from all the items in the combobox
	if (HWND hDlgItem = hDlgItem = GetDlgItem(hDlg, 0x407))
	{
		int count = SendMessageA(hDlgItem, CB_GETCOUNT, 0, 0);
		int index = pRand->RandomRanged(0, count - 1);
		int itemdata = SendMessageA(hDlgItem, CB_GETITEMDATA, index, 0);
		pMapSeed->Theater = itemdata;
	}

	// randomize creation of urban areas
	if (HWND hDlgItem = hDlgItem = GetDlgItem(hDlg, ARES_CHK_RMG_URBAN_AREAS))
	{
		int enabled = pRand->RandomRanged(1, 100);
		RMG::UrbanAreas = (enabled > 50);
	}

	// recreate random value for "map time of the day"
	R->EAX(pRand->RandomRanged(0, 3));
	return 0x5967C1;
}

DEFINE_OVERRIDE_HOOK(0x5970EA, RMG_EnableDesert, 9)
{
	GET(HWND, hWnd, EDI);

	//List the item
	LRESULT result =
		SendMessageA(
		hWnd,
		WW_CB_ADDITEM,		//CUSTOM BY WESTWOOD
		0,
		reinterpret_cast<LPARAM>(StringTable::LoadString("Name:Desert"))); // oh pd

	//Set the item data
	SendMessageA(
		hWnd,
		CB_SETITEMDATA,
		result,
		3);			//Make it actually be desert

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x596FFE, RMG_EnableArchipelago, 0)
{
	R->EBP(0);						//start at index 0 instead of 1
	R->EBX(0x82B034);				//set the list offset to "TXT_MAP_ARCHIPELAGO"
	return 0x597008;
}

DEFINE_OVERRIDE_HOOK(0x596C81, MapSeedClass_DialogFunc_GetData, 5)
{
	GET(HWND, hDlg, EBP);
	if (HWND hDlgItem = GetDlgItem(hDlg, ARES_CHK_RMG_URBAN_AREAS))
	{
		RMG::UrbanAreas = (1 == SendMessageA(hDlgItem, BM_GETCHECK, 0, 0));
	}
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x5971EA, MapSeedClass_DialogFunc_SetData, 5)
{
	GET(HWND, hDlg, EBX);
	if (HWND hDlgItem = GetDlgItem(hDlg, ARES_CHK_RMG_URBAN_AREAS))
	{
		SendMessageA(hDlgItem, BM_SETCHECK, (RMG::UrbanAreas ? 1 : 0), 0);
	}
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x5A65CA, MapSeedClass_Generate_PlaceUrbanStructures_Start, 5)
{
	RMG::UrbanStructuresReadSoFar = 0;
	if (!RMG::UrbanStructures.Count())
	{
		return 0x5A68F1; // no structures - nothing to do
	}
	R->ESI(RMG::UrbanStructures.ToString());
	return 0x5A65D5;
}

DEFINE_OVERRIDE_HOOK(0x5A6619, MapSeedClass_Generate_PlaceUrbanStructures_Loop, 6)
{
	++RMG::UrbanStructuresReadSoFar;
	return (RMG::UrbanStructures.Count() > RMG::UrbanStructuresReadSoFar)
		? 0x5A65D1
		: 0x5A6621
		;
}

DEFINE_OVERRIDE_HOOK(0x5A66B0, MapSeedClass_Generate_PlaceUrbanStructures_SanityCheck, 5)
{
	GET(int, Index, EAX);
	return (Index > -1)
		? 0
		: 0x5A68D8
		;
}