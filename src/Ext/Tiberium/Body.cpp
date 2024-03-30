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

	this->LinkedOverlayType.Read(exINI, pSection, "OverlayType.Initial");

	int Image = -1;
	detail::read<int>(Image, exINI, pSection, GameStrings::Image());
	this->PipIndex.Read(exINI, pSection, "PipIndex");

	bool slopes = false;

	switch (Image)
	{
	case -1:
		if (this->PipIndex == -1)
			this->PipIndex = 2;
		break;
	case 2:
		if (this->LinkedOverlayType->empty()) {
			this->LinkedOverlayType = "GEM";
		}
		this->PipIndex = 5;
		break;
	case 3:
		if (this->LinkedOverlayType->empty()) {
			this->LinkedOverlayType = "TIB2_";
		}
		slopes = true;
		this->PipIndex = 2;
		break;
	case 4:
		if (this->LinkedOverlayType->empty()) {
			this->LinkedOverlayType = "TIB3_";
		}
		slopes = true;
		this->PipIndex = 2;
		break;
	default:
		if (this->LinkedOverlayType->empty()) {
			this->LinkedOverlayType = "TIB";
		}
		slopes = true;
		this->PipIndex = 2;
		break;
	}

	detail::read<bool>(slopes, exINI, pSection, "UseSlopes");

	if(!this->LinkedOverlayType->empty()) {

		const int MaxCount = !slopes ? 12 : 20;
		OverlayTypeClass* first = nullptr;

		for (int i = 0; i < MaxCount; ++i) {
			const std::string Find = (this->LinkedOverlayType.Get() + std::format("{:02}", i + 1));
			OverlayTypeClass* pOverlay = OverlayTypeClass::Find(Find.c_str());

			if (!pOverlay)
				Debug::FatalErrorAndExit("CannotFind %s OverlayType for Tiberium[%s]\n", Find.c_str(), pSection);

			if(!pOverlay->Tiberium)
				Debug::FatalErrorAndExit("OverlayType[%s] for Tiberium[%s] is not Tiberium\n", Find.c_str(), pSection);

			if (i == 0) {
				first = pOverlay;
			}
			else if (first && pOverlay->ArrayIndex != (first->ArrayIndex + i)) {
				Debug::FatalErrorAndExit("OverlayType index of [%s - %d] is invalid compared to the first[%s - %d] (+ %d) \n", Find.c_str(), pOverlay->ArrayIndex, i ,first->ID, first->ArrayIndex);
			}

			if (Phobos::Otamaa::IsAdmin)
				Debug::Log("Reading[%s] With CurOverlay[%s] \n", pSection, Find.c_str());
		}

		detail::read<int>(pThis->NumFrames, exINI, pSection, "NumFrames");
		pThis->SlopeFrames = !slopes ? 0 : 8;
		pThis->NumImages = MaxCount;
	}
}

int TiberiumExtData::GetHealStep(TechnoClass* pTechno) const
{
	const auto pType = pTechno->GetTechnoType();
	const Nullable<int>* look = &this->Heal_Step;

	switch (pTechno->WhatAmI())
	{
	case InfantryClass::AbsID:
		look = &this->Heal_IStep;
		break;
	case UnitClass::AbsID:
		look = &this->Heal_UStep;
		break;
	default:
		break;
	}

	return look->Get(pType->GetRepairStep());
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

	if (R->Origin() == 0x721CDC && !TiberiumExtContainer::Instance.Find(pItem)->LinkedOverlayType->empty()) {
		if (auto pLinked = OverlayTypeClass::Find((TiberiumExtContainer::Instance.Find(pItem)->LinkedOverlayType.Get() + "01").c_str())) {
			pItem->Image = pLinked;
		}
	}
	return 0;
}