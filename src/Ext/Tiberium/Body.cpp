#include "Body.h"

TiberiumExt::ExtContainer TiberiumExt::ExtMap;

// void TiberiumExt::ExtData::InvalidatePointer(void *ptr, bool bRemoved) {}

void TiberiumExt::ExtData::LoadFromINIFile(CCINIClass* pINI)
{
	auto pThis = this->Get();
	const char* pSection = pThis->ID;

	if (!pINI->GetSection(pSection))
		return;

	INI_EX exINI(pINI);

	this->Palette.Read(pINI, pSection, "CustomPalette");
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
}

// =============================
// container
template <typename T>
void TiberiumExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Palette)
		.Process(this->OreTwinkle)
		.Process(this->OreTwinkleChance)
		.Process(this->Ore_TintLevel)
		.Process(this->MinimapColor)
		.Process(this->EnableLighningFix)
		.Process(this->UseNormalLight)
		//.Process(this->Replaced_EC)
	;
}

void TiberiumExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<TiberiumClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void TiberiumExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<TiberiumClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool TiberiumExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool TiberiumExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

TiberiumExt::ExtContainer::ExtContainer() : Container("TiberiumClass") {}
TiberiumExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x7217CC, TiberiumClass_CTOR, 0x5)
{
	GET(TiberiumClass*, pItem, ESI);
	TiberiumExt::ExtMap.Allocate(pItem);
	return 0;
}

DEFINE_HOOK(0x721888, TiberiumClass_DTOR, 0x6)
{
	GET(TiberiumClass*, pItem, ECX);
	TiberiumExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x7220D0, TiberiumClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x721E80, TiberiumClass_SaveLoad_Prefix, 0x7)
{
	GET_STACK(TiberiumClass*, pThis, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	TiberiumExt::ExtMap.PrepareStream(pThis, pStm);

	return 0;
}

DEFINE_HOOK(0x72208C, TiberiumClass_Load_Suffix, 0x7)
{
	TiberiumExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x72212C, TiberiumClass_Save_Suffix, 0x5)
{
	TiberiumExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(0x721CDC, TiberiumClass_LoadFromINI, 0xA)
DEFINE_HOOK_AGAIN(0x721CE9, TiberiumClass_LoadFromINI, 0xA)
DEFINE_HOOK(0x721C7B, TiberiumClass_LoadFromINI, 0xA)
{
	GET(TiberiumClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, STACK_OFFS(0xC4, -0x4));

	TiberiumExt::ExtMap.LoadFromINI(pItem, pINI);

	return 0;
}

// Replace EC check here
//DEFINE_HOOK(0x5FDD85, OverlayClass_Get_Tiberium_ReplaceEC, 0x6)
//{
//	GET(TiberiumClass*, pThis, EAX);
//	R->EBP(TiberiumExt::ExtMap.Find(pThis)->Replaced_EC);
//	return 0x5FDD8B;
//}
//
//DEFINE_HOOK(0x721CC6, TiberiumClass_ReadINI_ReplaceEC, 0xA)
//{
//	GET(TiberiumClass*, pItem, ESI);
//	TiberiumExt::ExtMap.Find(pItem)->Replaced_EC = 8;
//	return 0x721CD0;
//}