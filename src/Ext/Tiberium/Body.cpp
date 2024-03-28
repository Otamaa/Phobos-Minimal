#include "Body.h"

void TiberiumExtData::LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr)
{
	auto pThis = this->AttachedToObject;
	const char* pSection = pThis->ID;

	if (parseFailAddr)
		return;

	INI_EX exINI(pINI);

	this->Palette.Read(exINI, pSection, "CustomPalette");
	this->OreTwinkle.Read(exINI, pSection,"OreTwinkle");
	this->OreTwinkleChance.Read(exINI, pSection, "OreTwinkleChance");
	this->Ore_TintLevel.Read(exINI, pSection, "OreTintLevel");

	this->MinimapColor.Read(exINI, pSection, "MinimapColor");
	this->EnableLighningFix.Read(exINI, pSection, "EnableLighningFix");

	//INI_EX exArtINI(CCINIClass::INI_Art());
	//auto const pArtSection = pThis->Image->ImageFile;
	//if (!exArtINI.GetINI()->GetSection(pArtSection))
	//	return;

	this->UseNormalLight.Read(exINI, pSection, "UseNormalLight");
	this->EnablePixelFXAnim.Read(exINI, pSection, "EnablePixelFX");


	this->Damage.Read(exINI, pSection, "Damage");
	this->Warhead.Read(exINI, pSection, "Warhead");

	this->Heal_Step.Read(exINI, pSection, "Heal.Step");
	this->Heal_IStep.Read(exINI, pSection, "Heal.IStep");
	this->Heal_UStep.Read(exINI, pSection, "Heal.UStep");
	this->Heal_Delay.Read(exINI, pSection, "Heal.Delay");

	this->ExplosionWarhead.Read(exINI, pSection, "ExplosionWarhead");
	this->ExplosionDamage.Read(exINI, pSection, "ExplosionDamage");

	this->DebrisChance.Read(exINI, pSection, "Debris.Chance");

	//TIB3_21
	this->LinkedOverlayType.Read(exINI, pSection, "LinkedOverlayType");

	if(this->LinkedOverlayType.size()){
		detail::read<int>(pThis->NumFrames, exINI, pSection, "NumFrames");
		detail::read<int>(pThis->field_EC, exINI, pSection, "field_EC");
		pThis->NumImages = this->LinkedOverlayType.size();
	}

	int Image = -1;
	detail::read<int>(Image, exINI, pSection, GameStrings::Image());
	this->PipIndex.Read(exINI, pSection, "PipIndex");
	switch (Image)
	{
	case -1:
		if (this->PipIndex == -1)
			this->PipIndex = 2;
		break;
	case 2:
		this->PipIndex = 5;
		break;
	case 3:
		this->PipIndex = 2;
		break;
	case 4:
		this->PipIndex = 2;
		break;
	default:
		this->PipIndex = 2;
		break;
	}
}

int TiberiumExtData::GetHealStep(TechnoClass* pTechno) const
{
	const auto pType = pTechno->GetTechnoType();

	switch (pType->WhatAmI())
	{
	case InfantryTypeClass::AbsID:
		return this->Heal_IStep.Get(pType->GetRepairStep());
	case UnitTypeClass::AbsID:
		return this->Heal_UStep.Get(pType->GetRepairStep());
	default:
		return this->Heal_Step.Get(pType->GetRepairStep());
	}
}

// =============================
// container
template <typename T>
void TiberiumExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)
		.Process(this->Palette)
		.Process(this->OreTwinkle)
		.Process(this->OreTwinkleChance)
		.Process(this->Ore_TintLevel)
		.Process(this->MinimapColor)
		.Process(this->EnableLighningFix)
		.Process(this->UseNormalLight)
		//.Process(this->Replaced_EC)
		.Process(this->Damage)
		.Process(this->Warhead)
		.Process(this->Heal_Step)
		.Process(this->Heal_IStep)
		.Process(this->Heal_UStep)
		.Process(this->Heal_Delay)
		.Process(this->ExplosionWarhead)
		.Process(this->ExplosionDamage)
		.Process(this->DebrisChance)
		.Process(this->LinkedOverlayType)
		.Process(this->PipIndex)
	;
}

TiberiumExtContainer TiberiumExtContainer::Instance;

// =============================
// container hooks

// was 7217CC
DEFINE_HOOK(0x721876, TiberiumClass_CTOR, 0x5)
{
	GET(TiberiumClass*, pItem, ESI);
	TiberiumExtContainer::Instance.Allocate(pItem);
	return 0;
}

DEFINE_HOOK(0x721888, TiberiumClass_DTOR, 0x6)
{
	GET(TiberiumClass*, pItem, ECX);
	TiberiumExtContainer::Instance.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x7220D0, TiberiumClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x721E80, TiberiumClass_SaveLoad_Prefix, 0x7)
{
	GET_STACK(TiberiumClass*, pThis, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	TiberiumExtContainer::Instance.PrepareStream(pThis, pStm);

	return 0;
}

DEFINE_HOOK(0x72208C, TiberiumClass_Load_Suffix, 0x7)
{
	TiberiumExtContainer::Instance.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x72212C, TiberiumClass_Save_Suffix, 0x5)
{
	TiberiumExtContainer::Instance.SaveStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(0x721CDC, TiberiumClass_LoadFromINI, 0xA)
DEFINE_HOOK_AGAIN(0x721CE9, TiberiumClass_LoadFromINI, 0xA)
DEFINE_HOOK(0x721C7B, TiberiumClass_LoadFromINI, 0xA)
{
	GET(TiberiumClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, STACK_OFFS(0xC4, -0x4));

	TiberiumExtContainer::Instance.LoadFromINI(pItem, pINI , R->Origin() == 0x721CE9);

	if (R->Origin() == 0x721CDC && !TiberiumExtContainer::Instance.Find(pItem)->LinkedOverlayType.empty()) {
		if (auto pLinked = TiberiumExtContainer::Instance.Find(pItem)->LinkedOverlayType[0]) {
			pItem->Image = pLinked; //this is suppose to be linked list
			// so this will decide !
		}
	}
	return 0;
}