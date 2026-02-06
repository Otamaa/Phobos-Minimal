#include "Body.h"

#include <Utilities/GeneralUtils.h>
#include <Utilities/Helpers.h>
#include <Utilities/Macro.h>

#include <DiscreteDistributionClass.h>

#include <Phobos.SaveGame.h>

void HouseTypeExtData::Initialize()
{
	const char* pID = This()->ID;

	COMPILETIMEEVAL static char const* const countries[] = {
		"Americans",
		"Alliance",
		"French",
		"Germans",
		"British",
		"Africans",
		"Arabs",
		"Confederation",
		"Russians",
		"YuriCountry"
	};

	const auto it = std::ranges::find_if(countries,
		[=](const char* pCountry) { return IS_SAME_STR_(pID, pCountry); });

	const size_t index = it != std::ranges::end(countries) ? std::distance(std::ranges::begin(countries), it) : -1;
	this->TauntFileName->resize(19);

	switch ((Countries)index)
	{
	case Countries::Americans: // USA
		std::memcpy(this->TauntFileName->data() , "taunts\\tauam~~.wav" , 19u);
		this->LoadScreenPalette = "mplsu.pal";
		this->LoadScreenBackground = "ls%sustates.shp";

		this->LoadScreenName = "Name:Americans";
		this->LoadScreenSpecialName = "Name:Para";
		this->LoadScreenBrief = "LoadBrief:USA";
		this->StatusText = "STT:PlayerSideAmerica";

		this->FlagFile.Insert("usai.pcx");
		this->ObserverBackgroundSHP = FileSystem::LoadSHPFile("obsalli.shp");
		this->ObserverFlagSHP = FileSystem::LoadSHPFile("usai.shp");
		break;
	case Countries::Alliance: //Korea
		std::memcpy(this->TauntFileName->data(), "taunts\\tauko~~.wav", 19u);
		this->LoadScreenPalette = "mplsk.pal";
		this->LoadScreenBackground = "ls%skorea.shp";

		this->LoadScreenName = "Name:Alliance";
		this->LoadScreenSpecialName = "Name:BEAGLE";
		this->LoadScreenBrief = "LoadBrief:Korea";
		this->StatusText = "STT:PlayerSideKorea";

		this->FlagFile.Insert("japi.pcx");
		this->ObserverBackgroundSHP = FileSystem::LoadSHPFile("obsalli.shp");
		this->ObserverFlagSHP = FileSystem::LoadSHPFile("japi.shp");
		break;
	case Countries::French: //France
		std::memcpy(this->TauntFileName->data(), "taunts\\taufr~~.wav", 19u);
		this->LoadScreenPalette = "mplsf.pal";
		this->LoadScreenBackground = "ls%sfrance.shp";

		this->LoadScreenName = "Name:French";
		this->LoadScreenSpecialName = "Name:GTGCAN";
		this->LoadScreenBrief = "LoadBrief:French";
		this->StatusText = "STT:PlayerSideFrance";

		this->FlagFile.Insert("frai.pcx");
		this->ObserverBackgroundSHP = FileSystem::LoadSHPFile("obsalli.shp");
		this->ObserverFlagSHP = FileSystem::LoadSHPFile("frai.shp");
		break;
	case Countries::Germans: //Germany
		std::memcpy(this->TauntFileName->data(), "taunts\\tauge~~.wav", 19u);
		this->LoadScreenPalette = "mplsg.pal";
		this->LoadScreenBackground = "ls%sgermany.shp";

		this->LoadScreenName = "Name:Germans";
		this->LoadScreenSpecialName = "Name:TNKD";
		this->LoadScreenBrief = "LoadBrief:Germans";
		this->StatusText = "STT:PlayerSideGermany";

		this->FlagFile.Insert("geri.pcx");
		this->ObserverBackgroundSHP = FileSystem::LoadSHPFile("obsalli.shp");
		this->ObserverFlagSHP = FileSystem::LoadSHPFile("geri.shp");
		break;
	case Countries::British: //United Kingdom
		std::memcpy(this->TauntFileName->data(), "taunts\\taubr~~.wav", 19u);
		this->LoadScreenPalette = "mplsuk.pal";
		this->LoadScreenBackground = "ls%sukingdom.shp";

		this->LoadScreenName = "Name:British";
		this->LoadScreenSpecialName = "Name:SNIPE";
		this->LoadScreenBrief = "LoadBrief:British";
		this->StatusText = "STT:PlayerSideBritain";

		this->FlagFile.Insert("gbri.pcx");
		this->ObserverBackgroundSHP = FileSystem::LoadSHPFile("obsalli.shp");
		this->ObserverFlagSHP = FileSystem::LoadSHPFile("gbri.shp");
		break;
	case Countries::Africans: //Libya
		std::memcpy(this->TauntFileName->data(), "taunts\\tauli~~.wav", 19u);
		this->LoadScreenPalette = "mplsl.pal";
		this->LoadScreenBackground = "ls%slibya.shp";

		this->LoadScreenName = "Name:Africans";
		this->LoadScreenSpecialName = "Name:DTRUCK";
		this->LoadScreenBrief = "LoadBrief:Lybia";
		this->StatusText = "STT:PlayerSideLibya";

		this->FlagFile.Insert("djbi.pcx");
		this->ObserverBackgroundSHP = FileSystem::LoadSHPFile("obssovi.shp");
		this->ObserverFlagSHP = FileSystem::LoadSHPFile("djbi.shp");
		break;
	case Countries::Arabs: //Iraq
		std::memcpy(this->TauntFileName->data(), "taunts\\tauir~~.wav", 19u);
		this->LoadScreenPalette = "mplsi.pal";
		this->LoadScreenBackground = "ls%siraq.shp";

		this->LoadScreenName = "Name:Arabs";
		this->LoadScreenSpecialName = "Name:DESO";
		this->LoadScreenBrief = "LoadBrief:Iraq";
		this->StatusText = "STT:PlayerSideIraq";

		this->FlagFile.Insert("arbi.pcx");
		this->ObserverBackgroundSHP = FileSystem::LoadSHPFile("obssovi.shp");
		this->ObserverFlagSHP = FileSystem::LoadSHPFile("arbi.shp");
		break;
	case Countries::Confederation: //Cuba
		std::memcpy(this->TauntFileName->data(), "taunts\\taucu~~.wav", 19u);
		this->LoadScreenPalette = "mplsc.pal";
		this->LoadScreenBackground = "ls%scuba.shp";

		this->LoadScreenName = "Name:Confederation";
		this->LoadScreenSpecialName = "Name:TERROR";
		this->LoadScreenBrief = "LoadBrief:Cuba";
		this->StatusText = "STT:PlayerSideCuba";

		this->FlagFile.Insert("lati.pcx");
		this->ObserverBackgroundSHP = FileSystem::LoadSHPFile("obssovi.shp");
		this->ObserverFlagSHP = FileSystem::LoadSHPFile("lati.shp");
		break;
	case Countries::Russians: //Russia
		std::memcpy(this->TauntFileName->data(), "taunts\\tauru~~.wav", 19u);
		this->LoadScreenPalette = "mplsr.pal";
		this->LoadScreenBackground = "ls%srussia.shp";

		this->LoadScreenName = "Name:Russians";
		this->LoadScreenSpecialName = "Name:TTNK";
		this->LoadScreenBrief = "LoadBrief:Russia";
		this->StatusText = "STT:PlayerSideRussia";

		this->FlagFile.Insert("rusi.pcx");
		this->ObserverBackgroundSHP = FileSystem::LoadSHPFile("obssovi.shp");
		this->ObserverFlagSHP = FileSystem::LoadSHPFile("rusi.shp");
		break;
	case Countries::YuriCountry: //Yuri
		std::memcpy(this->TauntFileName->data(), "taunts\\tauyu~~.wav", 19u);
		this->LoadScreenPalette = "mpyls.pal";
		this->LoadScreenBackground = "ls%syuri.shp";

		this->LoadScreenName = "Name:YuriCountry";
		this->LoadScreenSpecialName = "Name:YURI";
		this->LoadScreenBrief = "LoadBrief:YuriCountry";
		this->StatusText = "STT:PlayerSideYuriCountry";

		this->FlagFile.Insert("yrii.pcx");
		this->ObserverBackgroundSHP = FileSystem::LoadSHPFile("obsyuri.shp");
		this->ObserverFlagSHP = FileSystem::LoadSHPFile("yrii.shp");
		this->ObserverFlagYuriPAL = true;
		break;
	default: //Unknown
		std::memcpy(this->TauntFileName->data(), "taunts\\tauam~~.wav",19u);
		this->LoadScreenPalette = "mplsobs.pal";
		this->LoadScreenBackground = "ls%sobs.shp";

		this->LoadScreenName = "GUI:Unknown";
		this->LoadScreenSpecialName = "GUI:Unknown";
		this->LoadScreenBrief = "GUI:Unknown";
		this->StatusText = "GUI:Unknown";

		this->FlagFile.Insert("rani.pcx");
		break;
	}

	switch (This()->SideIndex)
	{
	case 0:
		this->LoadTextColor = ColorScheme::FindIndex("AlliedLoad");
		break;
	case 1:
		this->LoadTextColor = ColorScheme::FindIndex("SovietLoad");
		break;
	case 2:
		this->LoadTextColor = ColorScheme::FindIndex("YuriLoad");
		if (this->LoadTextColor == -1)
		{
			// there is no YuriLoad in the original game. fall
			// back to a decent value.
			this->LoadTextColor = ColorScheme::FindIndex("Purple");
		}
		break;
	}
}

void HouseTypeExtData::InheritSettings(HouseTypeClass* pThis)
{
	if (auto ParentCountry = pThis->FindParentCountry()) {
		if (const auto ParentData = HouseTypeExtContainer::Instance.Find(ParentCountry)) {
			this->SurvivorDivisor = ParentData->SurvivorDivisor;
			this->Crew = ParentData->Crew;
			this->Engineer = ParentData->Engineer;
			this->Technician = ParentData->Technician;
			this->ParaDropPlane = ParentData->ParaDropPlane;
			this->SpyPlane = ParentData->SpyPlane;
			this->HunterSeeker = ParentData->HunterSeeker;
			this->ParaDropTypes = ParentData->ParaDropTypes;
			this->ParaDropNum = ParentData->ParaDropNum;
			this->GivesBounty = ParentData->GivesBounty;
			this->CanBeDriven = ParentData->CanBeDriven;
			this->ParachuteAnim = ParentData->ParachuteAnim;
			this->StartInMultiplayer_WithConst = ParentData->StartInMultiplayer_WithConst;
			this->Powerplants = ParentData->Powerplants;
			this->VeteranBuildings = ParentData->VeteranBuildings;
			this->TauntFileName = ParentData->TauntFileName;
			this->TauntFile = ParentData->TauntFile;
			this->Degrades = ParentData->Degrades;
			this->StartInMultiplayer_Types = ParentData->StartInMultiplayer_Types;

			this->LoadScreenBackground = ParentData->LoadScreenBackground;
			this->LoadScreenPalette = ParentData->LoadScreenPalette;

			this->LoadTextColor = ParentData->LoadTextColor;

			this->LoadScreenName = ParentData->LoadScreenName;
			this->LoadScreenSpecialName = ParentData->LoadScreenSpecialName;
			this->LoadScreenBrief = ParentData->LoadScreenBrief;

			this->StatusText = ParentData->StatusText;

			this->RandomSelectionWeight = ParentData->RandomSelectionWeight;

			this->FlagFile = ParentData->FlagFile;
			this->ObserverBackground = ParentData->ObserverBackground;
			this->ObserverBackgroundSHP = ParentData->ObserverBackgroundSHP;
			this->ObserverFlag = ParentData->ObserverFlag;
			this->ObserverFlagSHP = ParentData->ObserverFlagSHP;
			this->ObserverFlagYuriPAL = ParentData->ObserverFlagYuriPAL;
		}
	}

	this->SettingsInherited = true;
}

bool HouseTypeExtData::LoadFromINI(CCINIClass* pINI, bool parseFailAddr)
{
	auto pThis = This();
	const char* pSection = pThis->ID;

	if (!this->SettingsInherited
		&& *pThis->ParentCountry
		&& !IS_SAME_STR_(pThis->ParentCountry, pThis->ID))
	{
		this->InheritSettings(pThis);
	}

	if (parseFailAddr)
		return false;

	INI_EX exINI(pINI);

	this->SurvivorDivisor.Read(exINI, pSection, "SurvivorDivisor");
	this->Crew.Read(exINI, pSection, "Crew", true);
	this->Engineer.Read(exINI, pSection, "Engineer", true);
	this->Technician.Read(exINI, pSection, "Technician", true);
	this->ParaDropPlane.Read(exINI, pSection, "ParaDrop.Aircraft" , true);
	this->HunterSeeker.Read(exINI, pSection, "HunterSeeker", true);
	this->SpyPlane.Read(exINI, pSection, "SpyPlane.Aircraft", true);
	this->ParaDropTypes.Read(exINI, pSection, "ParaDrop.Types", true);

	this->StartInMultiplayer_Types.Read(exINI, pSection, "StartInMultiplayer.Types", true);

	this->ParaDropNum.Read(exINI, pSection, "ParaDrop.Num");
	this->GivesBounty.Read(exINI, pSection, "GivesBounty");
	this->CanBeDriven.Read(exINI, pSection, "CanBeDriven");

	// Disabled atm
	this->NewTeamsSelector_MergeUnclassifiedCategoryWith.Read(exINI, pSection, "NewTeamsSelector.MergeUnclassifiedCategoryWith");
	this->NewTeamsSelector_UnclassifiedCategoryPercentage.Read(exINI, pSection, "NewTeamsSelector.UnclassifiedCategoryPercentage");
	this->NewTeamsSelector_GroundCategoryPercentage.Read(exINI, pSection, "NewTeamsSelector.GroundCategoryPercentage");
	this->NewTeamsSelector_AirCategoryPercentage.Read(exINI, pSection, "NewTeamsSelector.AirCategoryPercentage");
	this->NewTeamsSelector_NavalCategoryPercentage.Read(exINI, pSection, "NewTeamsSelector.NavalCategoryPercentage");
	//

	this->ParachuteAnim.Read(exINI, pSection, "Parachute.Anim" , true);
	this->StartInMultiplayer_WithConst.Read(exINI, pSection, "StartInMultiplayer.WithConst");
	this->Powerplants.Read(exINI, pSection, "AI.PowerPlants", true);
	this->VeteranBuildings.Read(exINI, pSection, "VeteranBuildings", true);

	this->Degrades.Read(exINI, pSection, "Degrades");
	this->Disguise.Read(exINI, pSection, "DefaultDisguise", true);

	this->LoadTextColor.Read(exINI, pSection, "LoadScreenText.Color");

	this->BattlePoints.Read(exINI, pSection, "BattlePoints");
	this->BattlePoints_CanUseStandardPoints.Read(exINI, pSection, "BattlePoints.CanUseStandardPoints");

	return true;
}

void HouseTypeExtData::LoadFromRulesFile(CCINIClass* pINI) {
	auto pThis = This();
	const char* pSection = pThis->ID;

	INI_EX exINI(pINI);

	auto ReadShpOrPcxImage = [pINI, pSection](const char* key, PhobosPCXFile& Pcx, SHPStruct** ppShp)
	{
			// read the key and convert it to lower case
		if (pINI->ReadString(pSection, key, Pcx.GetFilename(), Phobos::readBuffer))
		{
			// parse the value
			if (GameStrings::IsNone(Phobos::readBuffer))
			{
					// explicitly set to no image
				if (ppShp)
				{
					*ppShp = nullptr;
				}
				Pcx.Erase();
			}
			else if (!ppShp || strstr(Phobos::readBuffer, ".pcx"))
			{
					// clear shp and load pcx
				if (ppShp)
				{
					*ppShp = nullptr;
				}
				Pcx.Insert(Phobos::readBuffer);
				if (!Pcx.Exists())
				{
					// log error and clear invalid name
					Debug::INIParseFailed(pSection, key, Phobos::readBuffer);
					Pcx.Erase();
				}
			}
			else if (ppShp)
			{
				// allowed to load as shp
				*ppShp = FileSystem::LoadSHPFile(Phobos::readBuffer);
				if (!*ppShp)
				{
					// log error and clear invalid name
					Debug::INIParseFailed(pSection, key, Phobos::readBuffer);
					Pcx.Erase();
				}
			}
			else
			{
				// disallowed file type
				Debug::INIParseFailed(pSection, key, Phobos::readBuffer, "File type not allowed.");
			}
		}
	};

	ReadShpOrPcxImage("File.Flag", this->FlagFile, nullptr);
	ReadShpOrPcxImage("File.ObserverFlag", this->ObserverFlag, &this->ObserverFlagSHP);
	ReadShpOrPcxImage("File.ObserverBackground", this->ObserverBackground, &this->ObserverBackgroundSHP);

	this->ObserverFlagYuriPAL.Read(exINI, pSection, "File.ObserverFlagAltPalette");

	this->TauntFileName.Read(exINI, pSection, "File.Taunt");
	COMPILETIMEEVAL char digits[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };

	if(!this->TauntFileName->empty()) {

		const auto nPos = this->TauntFileName->find("~~");
		this->TauntFile.clear();

		if (nPos != std::string::npos) {
			this->TauntFile.resize((sizeof(digits) - 1), this->TauntFileName.Get());

			for (size_t i = 0; i < this->TauntFile.size(); ++i) {
				this->TauntFile[i][nPos] = digits[0];
				this->TauntFile[i][nPos + 1] = digits[i + 1];

				//if (Phobos::Otamaa::IsAdmin)
				//	Debug::LogInfo("Reading taunt File[%d] for[%s] = %s", i, pSection, this->TauntFile[i].c_str());
			}
		}
	}

	this->LoadScreenBackground.Read(pINI, pSection, "File.LoadScreen");
	this->LoadScreenPalette.Read(pINI, pSection, "File.LoadScreenPAL");

	this->LoadScreenName.Read(exINI, pSection, "LoadScreenText.Name");
	this->LoadScreenSpecialName.Read(exINI, pSection, "LoadScreenText.SpecialName");
	this->LoadScreenBrief.Read(exINI, pSection, "LoadScreenText.Brief");
	this->StatusText.Read(exINI, pSection, "MenuText.Status");
}

Iterator<BuildingTypeClass*> HouseTypeExtData::GetPowerplants() const
{
	if (!this->Powerplants.empty()) {
		return this->Powerplants;
	}

	return this->GetDefaultPowerplants();
}

Iterator<BuildingTypeClass*> HouseTypeExtData::GetDefaultPowerplants() const
{
	BuildingTypeClass** ppPower = nullptr;
	switch (This()->SideIndex)
	{
	case 0:
		ppPower = &RulesClass::Instance->GDIPowerPlant;
		break;
	case 1:
		ppPower = &RulesClass::Instance->NodRegularPower;
		break;
	case 2:
		ppPower = &RulesClass::Instance->ThirdPowerPlant;
		break;
	}

	size_t count = (ppPower && *ppPower) ? 1u : 0u;
	return { ppPower, count };
}

int HouseTypeExtData::PickRandomCountry()
{
	DiscreteDistribution<int> items {};

	for (int i = 0; i < HouseTypeClass::Array->Count; i++) {
		HouseTypeClass* pCountry = HouseTypeClass::Array->Items[i];
		if (pCountry->Multiplay) {
				items.add(i,
				HouseTypeExtContainer::Instance.Find(pCountry)->RandomSelectionWeight);
		}
	}

	int ret = 0;
	if (!items.select(ScenarioClass::Instance->Random, &ret)) {
		Debug::FatalErrorAndExit("No countries eligible for random selection!");
	}

	return ret;
}

template <typename T>
void  HouseTypeExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->SettingsInherited)

		.Process(this->SurvivorDivisor)
		.Process(this->Crew)
		.Process(this->Engineer)
		.Process(this->Technician)
		.Process(this->ParaDropPlane)
		.Process(this->SpyPlane)
		.Process(this->HunterSeeker)
		.Process(this->ParaDropTypes)
		.Process(this->ParaDropNum)
		.Process(this->GivesBounty)
		.Process(this->CanBeDriven)
		.Process(this->NewTeamsSelector_MergeUnclassifiedCategoryWith)
		.Process(this->NewTeamsSelector_UnclassifiedCategoryPercentage)
		.Process(this->NewTeamsSelector_GroundCategoryPercentage)
		.Process(this->NewTeamsSelector_AirCategoryPercentage)
		.Process(this->NewTeamsSelector_NavalCategoryPercentage)
		.Process(this->ParachuteAnim)
		.Process(this->StartInMultiplayer_WithConst)
		.Process(this->Powerplants)
		.Process(this->VeteranBuildings)
		.Process(this->TauntFile)
		.Process(this->TauntFileName)
		.Process(this->Degrades)
		.Process(this->Disguise)
		.Process(this->StartInMultiplayer_Types)
		.Process(this->LoadScreenBackground)
		.Process(this->LoadScreenPalette)
		.Process(this->LoadTextColor)
		.Process(this->RandomSelectionWeight)

		.Process(this->LoadScreenName)
		.Process(this->LoadScreenSpecialName)
		.Process(this->LoadScreenBrief)

		.Process(this->StatusText)

		.Process(this->FlagFile)
		.Process(this->ObserverBackground)
		.Process(this->ObserverBackgroundSHP)
		.Process(this->ObserverFlag)
		.Process(this->ObserverFlagSHP)
		.Process(this->ObserverFlagYuriPAL)

		.Process(this->BattlePoints)
		.Process(this->BattlePoints_CanUseStandardPoints)
		;
}

// =============================
// container

HouseTypeExtContainer HouseTypeExtContainer::Instance;

bool HouseTypeExtContainer::LoadAll(const json& root)
{
	this->Clear();

	if (root.contains(HouseTypeExtContainer::ClassName))
	{
		auto& container = root[HouseTypeExtContainer::ClassName];

		for (auto& entry : container[HouseTypeExtData::ClassName])
		{
			uint32_t oldPtr = 0;
			if (!ExtensionSaveJson::ReadHex(entry, "OldPtr", oldPtr))
				return false;

			size_t dataSize = entry["datasize"].get<size_t>();
			std::string encoded = entry["data"].get<std::string>();
			auto buffer = this->AllocateNoInit();

			PhobosByteStream loader(dataSize);
			loader.data = std::move(Base64Handler::decodeBase64(encoded, dataSize));
			PhobosStreamReader reader(loader);

			PHOBOS_SWIZZLE_REGISTER_POINTER(oldPtr, buffer, HouseTypeExtData::ClassName);

			buffer->LoadFromStream(reader);

			if (!reader.ExpectEndOfBlock())
				return false;
		}

		return true;
	}

	return false;

}

bool HouseTypeExtContainer::SaveAll(json& root)
{
	auto& first_layer = root[HouseTypeExtContainer::ClassName];

	json _extRoot = json::array();
	for (auto& _extData : HouseTypeExtContainer::Array)
	{
		PhobosByteStream saver(sizeof(*_extData));
		PhobosStreamWriter writer(saver);

		_extData->SaveToStream(writer);

		json entry;
		ExtensionSaveJson::WriteHex(entry, "OldPtr", (uint32_t)_extData);
		entry["datasize"] = saver.data.size();
		entry["data"] = Base64Handler::encodeBase64(saver.data);
		_extRoot.push_back(std::move(entry));
	}

	first_layer[HouseTypeExtData::ClassName] = std::move(_extRoot);

	return true;
}

void HouseTypeExtContainer::LoadFromINI(HouseTypeClass* key, CCINIClass* pINI, bool parseFailAddr)
{
	if (auto ptr = this->Find(key)) {
		if (!pINI) {
			return;
		}

		switch (ptr->GetInitState())
		{

		case InitState::Blank:
		{
			ptr->SetInitState(InitState::Inited);

			//Load from rules INI File
			if (pINI == CCINIClass::INI_Rules)
			{
				ptr->LoadFromRulesFile(pINI);
			}

			ptr->SetInitState(InitState::Ruled);
		}
		break;
		case InitState::Ruled:
		case InitState::Constanted:
		{
			//load anywhere other than rules
			ptr->LoadFromINI(pINI, parseFailAddr);
			//this function can be called again multiple time but without need to re-init the data
			ptr->SetInitState(InitState::Ruled);
		}
		break;
		{
		default:
			break;
		}
		}
	}

}

void HouseTypeExtContainer::WriteToINI(HouseTypeClass* key, CCINIClass* pINI)
{

	if (auto ptr = this->TryFind(key)) {
		if (!pINI) {
			return;
		}

		ptr->WriteToINI(pINI);
	}
}

// =============================
// container hooks

ASMJIT_PATCH(0x511643, HouseTypeClass_CTOR, 0x5)
{
	GET(HouseTypeClass*, pItem, EAX);

	HouseTypeExtContainer::Instance.Allocate(pItem);

	return 0;
}ASMJIT_PATCH_AGAIN(0x511635, HouseTypeClass_CTOR, 0x5)

ASMJIT_PATCH(0x5127CF, HouseTypeClass_DTOR, 0x6)
{
	GET(HouseTypeClass*, pItem, ESI);

	HouseTypeExtContainer::Instance.Remove(pItem);
	return 0;
}

bool FakeHouseTypeClass::_ReadFromINI(CCINIClass* pINI)
{
	HouseTypeExtContainer::Instance.LoadFromINI(this, pINI, false);
	bool status = this->HouseTypeClass::LoadFromINI(pINI);
	HouseTypeExtContainer::Instance.LoadFromINI(this, pINI, !status);
	return status;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7EABBC, FakeHouseTypeClass::_ReadFromINI)
