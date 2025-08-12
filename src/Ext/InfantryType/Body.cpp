#include "Body.h"

#include <Ext/Infantry/Body.h>

void Phobos_DoControls::ReadSequence(DoInfoStruct* pDoInfo, InfantryTypeClass* pInf, CCINIClass* pINI)
{
	/*
	INI_EX IniEX(pINI);

	char section[0x100];
	if (pINI->GetString(pInf->ImageFile, "Sequence", section) > 0) {
		for (int i = 0; i < DoControls::MaxCount; ++i) {
			char sequenceData[0x100];
			if (pINI->GetString(section, DoControls::DoType_toStr[i], sequenceData) > 0) {
				auto& data = pDoInfo[i];
				std::string basename = DoControls::DoType_toStr[i];

				char bufferFacing[4];
				if(sscanf(sequenceData, "%d,%d,%d,%s",
					&data.StartFrame,
					&data.CountFrames,
					&data.FacingMultiplier,
					bufferFacing
				) > 3){
					for (size_t i = 0; i < EnumFunctions::FacingType_to_strings.size(); ++i)
					{
						if (IS_SAME_STR_(EnumFunctions::FacingType_to_strings[i], bufferFacing))
						{
							data.Facing = DoTypeFacing(i);
						}
					}
				}

				char bufferSounds[0x100];
				if (pINI->GetString(section, (basename + "Sounds").c_str(), bufferSounds) > 0)
				{
					auto v7 = strtok(bufferSounds, " ,\t");
					while (v7)
					{
						auto v8 = atoi(v7);
						auto v9 = strtok(0, " ,\t");
						if (!v9)
						{
							break;
						}

						data.SoundCount = v8;

						auto v10 = VocClass::FindIndexById(v9);
						v7 = strtok(0, " ,\t");
						if (v10 != -1)
						{
							for (auto at = data.SoundData;
								at != std::end(data.SoundData);
								++at)
							{
								at->Index = v10;
							}
						}
					}
				}
			}
		}
	}*/
}

void InfantryTypeExtData::Initialize()
{
	const auto pID = this->AttachedToObject->ID;
	this->Is_Deso = IS_SAME_STR_(pID, GameStrings::DESO());
	this->Is_Cow = IS_SAME_STR_(pID, GameStrings::COW());
}

void InfantryTypeExtData::LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr)
{
	const char* pID = this->AttachedToObject->ID;

	INI_EX exINI(pINI);
	INI_EX iniEX_art(CCINIClass::INI_Art());
	const auto pSection_art = this->AttachedToObject->ImageFile;

	this->Is_Deso.Read(exINI, pID,  "IsDesolator");
	this->Is_Cow.Read(exINI, pID, "IsCow");
	this->C4Delay.Read(exINI, pID, "C4Delay");
	this->C4ROF.Read(exINI, pID, "C4ROF");
	this->C4Damage.Read(exINI, pID, "C4Damage");
	this->C4Warhead.Read(exINI, pID, "C4Warhead");

	this->HideWhenDeployAnimPresent.Read(exINI, pID, "Deploy.HideWhenDeployAnimPresent");
	this->DeathBodies_UseDieSequenceAsIndex.Read(exINI, pID, "DeathBodies.UseDieSequenceAsIndex");
	this->VoiceGarrison.Read(exINI, pID, "VoiceGarrison");

	this->OnlyUseLandSequences.Read(iniEX_art, pSection_art, "OnlyUseLandSequences");

	this->WhenInfiltrate_Warhead.Read(exINI, pID, "WhenInfiltrate.Warhead.%s");
	this->WhenInfiltrate_Weapon.Read(exINI, pID, "WhenInfiltrate.Weapon.%s");
	this->WhenInfiltrate_Damage.Read(exINI, pID, "WhenInfiltrate.Damage.%s");
	this->WhenInfiltrate_Warhead_Full.Read(exINI, pID, "WhenInfiltrate.Warhead.Full");

	this->AllSequnceEqualRates.Read(exINI, pID, "AllSequnceEqualRates");
	this->AllowReceiveSpeedBoost.Read(exINI, pID, "AllowReceiveSpeedBoost");
	this->ProneSpeed.Read(exINI, pID, "ProneSpeed");

	this->InfantryAutoDeploy.Read(exINI, pID, "InfantryAutoDeploy");

	// TODO , this stupid parsing thing
	//auto const nPriData = this->Get()->GetWeapon(0);
	//auto const nPriEliteData = this->Get()->GetEliteWeapon(0);
	//auto const nSecData = this->Get()->GetWeapon(1);
	//auto const nSecEliteData = this->Get()->GetEliteWeapon(1);

	//((Valueable<WeaponTypeClass*>)this->CrawlingWeaponDatas[0].WeaponType)
	//	.Read(exINI, pID, "Primary.CrawlWeapon", true);

	//Nullable<CoordStruct> bufFLH_N {};
	//bufFLH_N.Read(iniEX_art, pSection_art, "Primary.CrawlWeaponFLH");
	//this->CrawlingWeaponDatas[0].FLH = bufFLH_N.Get(nPriData->FLH);

	//Nullable<int> bufBrlLngth_N {};
	//bufBrlLngth_N.Read(iniEX_art, pSection_art, "Primary.CrawlWeaponBarrelLength");
	//this->CrawlingWeaponDatas[0].BarrelLength = bufBrlLngth_N.Get(nPriData->BarrelLength);

	//Nullable<int> bufBrlthic_N {};
	//bufBrlthic_N.Read(iniEX_art, pSection_art, "Primary.CrawlWeaponBarrelThickness");
	//this->CrawlingWeaponDatas[0].BarrelThickness = bufBrlthic_N.Get(nPriData->BarrelThickness);

	//Nullable<bool> bufturrlck_N {};
	//bufturrlck_N.Read(iniEX_art, pSection_art, "Primary.CrawlWeaponTurretLocked");
	//this->CrawlingWeaponDatas[0].TurretLocked = bufturrlck_N.Get(nPriData->TurretLocked);

	////==================================================================================================//
	//Nullable<WeaponTypeClass*> buffWeapon_N {};
	//buffWeapon_N.Read(exINI, pID, "Primary.EliteCrawlWeapon", true);
	//this->CrawlingWeaponDatas[1].WeaponType = buffWeapon_N.Get(this->CrawlingWeaponDatas[0].WeaponType);

	//pWeaponReader.Read(exINI, pID, "Primary.EliteCrawlWeapon", true);
	//temp = { pWeaponReader , nPriEliteData->FLH , nPriEliteData->BarrelLength, nPriEliteData->BarrelThickness,  nPriEliteData->TurretLocked };
	//std::memcpy(this->CrawlingWeaponDatas + 1, &temp, sizeof(WeaponStruct));

	//pWeaponReader.Read(exINI, pID, "Secondary.CrawlWeapon", true);
	//temp = { pWeaponReader , nSecData->FLH , nSecData->BarrelLength, nSecData->BarrelThickness,  nSecData->TurretLocked };
	//std::memcpy(this->CrawlingWeaponDatas  + 2, &temp, sizeof(WeaponStruct));


	//pWeaponReader.Read(exINI, pID, "Secondary.EliteCrawlWeapon", true);
	//temp = { pWeaponReader , nSecEliteData->FLH , nSecEliteData->BarrelLength, nSecEliteData->BarrelThickness,  nSecEliteData->TurretLocked };
	//std::memcpy(this->CrawlingWeaponDatas + 3, &temp, sizeof(WeaponStruct));

	//Phobos_DoControls::ReadSequence(this->Sequences, this->Get(), iniEX_art.GetINI());
}

// =============================
// load / save

template <typename T>
void InfantryTypeExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)
		.Process(this->Type)
		.Process(this->Is_Deso)
		.Process(this->Is_Cow)
		.Process(this->C4Delay)
		.Process(this->C4ROF)
		.Process(this->C4Damage)
		.Process(this->C4Warhead)
		.Process(this->HideWhenDeployAnimPresent)
		.Process(this->DeathBodies_UseDieSequenceAsIndex)
		.Process(this->CrawlingWeaponDatas)
		.Process(this->VoiceGarrison)
		.Process(this->OnlyUseLandSequences)
		.Process(this->SquenceRates)

		.Process(this->WhenInfiltrate_Warhead)
		.Process(this->WhenInfiltrate_Weapon)
		.Process(this->WhenInfiltrate_Damage)
		.Process(this->WhenInfiltrate_Warhead_Full)

		.Process(this->AllSequnceEqualRates)
		.Process(this->AllowReceiveSpeedBoost)
		.Process(this->ProneSpeed)
		.Process(this->InfantryAutoDeploy)
		;
}

// =============================
// container
InfantryTypeExtContainer InfantryTypeExtContainer::Instance;

// =============================
// container hooks


//merged with the new sequence hooks
//ASMJIT_PATCH(0x523876, InfantryTypeClass_CTOR, 0x6)
//{
//	GET(InfantryTypeClass*, pItem, ESI);
//	if(auto pExt = InfantryTypeExtContainer::Instance.Allocate(pItem))
//		pExt->Type = TechnoTypeExtContainer::Instance.Find(pItem);
//	return 0;
//}

ASMJIT_PATCH(0x5239D0, InfantryTypeClass_DTOR, 0x5)
{
	GET(InfantryTypeClass* const, pItem, ESI);
	InfantryTypeExtContainer::Instance.Remove(pItem);
	return 0;
}

ASMJIT_PATCH(0x524B60, InfantryTypeClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(InfantryTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);
	InfantryTypeExtContainer::Instance.PrepareStream(pItem, pStm);

	return 0;
}ASMJIT_PATCH_AGAIN(0x524960, InfantryTypeClass_SaveLoad_Prefix, 0x8)

#include <Misc/ImageSwapModules.h>

ASMJIT_PATCH(0x524B53, InfantryTypeClass_Load_Suffix, 0x5)
{
	if (Phobos::Config::ArtImageSwap) {
		GET(BYTE*, poisonedVal, EDI);
		poisonedVal -= 0xE20;
		TechnoImageReplacer::Replace(reinterpret_cast<InfantryTypeClass*>(poisonedVal));
	}

	InfantryTypeExtContainer::Instance.LoadStatic();
	return 0;
}

ASMJIT_PATCH(0x524C52, InfantryTypeClass_Save_Suffix, 0x7)
{
	InfantryTypeExtContainer::Instance.SaveStatic();
	return 0;
}

ASMJIT_PATCH(0x52473F, InfantryTypeClass_LoadFromINI, 0x5)
{
	GET(InfantryTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0xD0);
	InfantryTypeExtContainer::Instance.LoadFromINI(pItem, pINI , R->Origin() == 0x52474E);
	return 0;
}ASMJIT_PATCH_AGAIN(0x52474E, InfantryTypeClass_LoadFromINI, 0x5)