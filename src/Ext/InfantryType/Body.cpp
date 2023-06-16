#include "Body.h"

#include <Ext/Infantry/Body.h>

void InfantryTypeExt::ExtData::Initialize()
{
	const auto pID = this->Get()->ID;
	this->Is_Deso = IS_SAME_STR_(pID, GameStrings::DESO());
	this->Is_Cow = IS_SAME_STR_(pID, GameStrings::COW());
}

void InfantryTypeExt::ExtData::LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr)
{
	const char* pID = this->Get()->ID;

	INI_EX exINI(pINI);

	if (!pINI->GetSection(pID))
		return;

	this->Is_Deso.Read(exINI, pID,  "IsDesolator");
	this->Is_Cow.Read(exINI, pID, "IsCow");
	this->C4Delay.Read(exINI, pID, "C4Delay");
	this->C4ROF.Read(exINI, pID, "C4ROF");
	this->C4Damage.Read(exINI, pID, "C4Damage");
	this->C4Warhead.Read(exINI, pID, "C4Warhead");

	this->HideWhenDeployAnimPresent.Read(exINI, pID, "Deploy.HideWhenDeployAnimPresent");
	this->DeathBodies_UseDieSequenceAsIndex.Read(exINI, pID, "DeathBodies.UseDieSequenceAsIndex");

	auto const nPriData = this->Get()->GetWeapon(0);
	auto const nPriEliteData = this->Get()->GetEliteWeapon(0);
	auto const nSecData = this->Get()->GetWeapon(1);
	auto const nSecEliteData = this->Get()->GetEliteWeapon(1);

	Valueable<WeaponTypeClass*> pWeaponReader { nullptr };
	pWeaponReader.Read(exINI, pID, "Primary.CrawlWeapon", true);
	this->CrawlingWeaponDatas[0].WeaponType = pWeaponReader.Get();
	this->CrawlingWeaponDatas[0].BarrelLength = nPriData->BarrelLength;
	this->CrawlingWeaponDatas[0].BarrelThickness = nPriData->BarrelThickness;
	this->CrawlingWeaponDatas[0].TurretLocked = nPriData->TurretLocked;

	pWeaponReader.Read(exINI, pID, "Primary.EliteCrawlWeapon", true);
	this->CrawlingWeaponDatas[1].WeaponType = pWeaponReader.Get();
	this->CrawlingWeaponDatas[1].BarrelLength = nPriEliteData->BarrelLength;
	this->CrawlingWeaponDatas[1].BarrelThickness = nPriEliteData->BarrelThickness;
	this->CrawlingWeaponDatas[1].TurretLocked = nPriEliteData->TurretLocked;

	pWeaponReader.Read(exINI, pID, "Secondary.CrawlWeapon", true);
	this->CrawlingWeaponDatas[2].WeaponType = pWeaponReader.Get();
	this->CrawlingWeaponDatas[2].BarrelLength = nSecData->BarrelLength;
	this->CrawlingWeaponDatas[2].BarrelThickness = nSecData->BarrelThickness;
	this->CrawlingWeaponDatas[2].TurretLocked = nSecData->TurretLocked;

	pWeaponReader.Read(exINI, pID, "Secondary.EliteCrawlWeapon", true);
	this->CrawlingWeaponDatas[3].WeaponType = pWeaponReader.Get();
	this->CrawlingWeaponDatas[3].BarrelLength = nSecEliteData->BarrelLength;
	this->CrawlingWeaponDatas[3].BarrelThickness = nSecEliteData->BarrelThickness;
	this->CrawlingWeaponDatas[3].TurretLocked = nSecEliteData->TurretLocked;
}

// =============================
// load / save

template <typename T>
void InfantryTypeExt::ExtData::Serialize(T& Stm)
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
		;
}

// =============================
// container
InfantryTypeExt::ExtContainer InfantryTypeExt::ExtMap;
InfantryTypeExt::ExtContainer::ExtContainer() : Container("InfantryTypeClass") { }
InfantryTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x523970, InfantryTypeClass_CTOR, 0x5)
{
	GET(InfantryTypeClass*, pItem, ESI);
	if(auto pExt = InfantryTypeExt::ExtMap.Allocate(pItem))
		pExt->Type = TechnoTypeExt::ExtMap.Find(pItem);
	return 0;
}

DEFINE_HOOK(0x5239D0, InfantryTypeClass_DTOR, 0x5)
{
	GET(InfantryTypeClass* const, pItem, ESI);
	InfantryTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x524960, InfantryTypeClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x524B60, InfantryTypeClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(InfantryTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);
	InfantryTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x524B53, InfantryTypeClass_Load_Suffix, 0x5)
{
	InfantryTypeExt::ExtMap.LoadStatic();
	return 0;
}

//DEFINE_HOOK(0x524B57, InfantryTypeClass_Load_Suffix, 0x7)
//{
//	InfantryTypeExt::ExtMap.LoadStatic();
//	return 0;
//}

//DEFINE_HOOK_AGAIN(0x524C59, InfantryTypeClass_Save_Suffix, 0x5)

// Before :  0x524C50 , 0x5 
// After : 0x524C52 , 0x7 
DEFINE_HOOK(0x524C52, InfantryTypeClass_Save_Suffix, 0x7)
{
	InfantryTypeExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(0x52474E , InfantryTypeClass_LoadFromINI , 0x5)
DEFINE_HOOK(0x52473F, InfantryTypeClass_LoadFromINI, 0x5)
{
	GET(InfantryTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0xD0);
	InfantryTypeExt::ExtMap.LoadFromINI(pItem, pINI , R->Origin() == 0x52474E);
	return 0;
}