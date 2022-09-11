#include "Body.h"

#include <Ext/Techno/Body.h>

ParasiteExt::ExtContainer ParasiteExt::ExtMap;

void TechnoExt::DrawParasitedPips(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds)
{
#ifdef PARASITE_PIPS
	{
		//bool IsHost = false;
		//bool IsSelected = false;					//Red         //Green           //White
		//ColorScheme Color = IsSelected ? (IsHost ? {255, 0, 0} : {0, 255, 0}) : {255,255,255};
		int xOffset = 0;
		int yOffset = 0;

		int nBracket = pThis->GetTechnoType()->PixelSelectionBracketDelta;
		if (auto pFoot = generic_cast<FootClass*>(pThis->Disguise))
			if (pThis->IsDisguised() && !pThis->IsClearlyVisibleTo(HouseClass::Player))
				nBracket = pFoot->GetTechnoType()->PixelSelectionBracketDelta;

		switch (pThis->WhatAmI())
		{
		case AbstractType::Unit:
		case AbstractType::Aircraft:
		{
			const auto& offset = RulesExt::Global()->Pips_SelfHeal_Units_Offset.Get();
			xOffset = offset.X;
			yOffset = offset.Y + nBracket;
		}
		break;
		case AbstractType::Infantry:
		{
			const auto& offset = RulesExt::Global()->Pips_SelfHeal_Infantry_Offset.Get();
			xOffset = offset.X;
			yOffset = offset.Y + nBracket;
		}
		break;
		}

		int pipFrame = 4;

		Point2D position { pLocation->X + xOffset, pLocation->Y + yOffset };

		auto flags = BlitterFlags::bf_400 | BlitterFlags::Centered;

		DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS_SHP,
			pipFrame, &position, pBounds, flags, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}
#endif
}

// =============================
// load / save
template <typename T>
void ParasiteExt::ExtData::Serialize(T& Stm) {
	Debug::Log("Processing Element From ParasiteExt ! \n");
}

void ParasiteExt::ExtData::LoadFromStream(PhobosStreamReader& Stm) {
	Extension<ParasiteClass>::Serialize(Stm);
	this->Serialize(Stm);
}

void ParasiteExt::ExtData::SaveToStream(PhobosStreamWriter& Stm) {
	Extension<ParasiteClass>::Serialize(Stm);
	this->Serialize(Stm);
}

bool ParasiteExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool ParasiteExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

ParasiteExt::ExtContainer::ExtContainer() : Container("ParasiteClass") { };
ParasiteExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks
#ifdef ENABLE_NEWHOOKS
DEFINE_HOOK(0x62937F, ParasiteClass_CTOR, 0x6)
{
	//Debug::Log("%s Executed ! \n", __FUNCTION__);
	GET(ParasiteClass*, pItem, ESI);
#ifdef ENABLE_NEWHOOKS
	ParasiteExt::ExtMap.JustAllocate(pItem, pItem, "Trying To Allocate from nullptr !");
#else
	ParasiteExt::ExtMap.FindOrAllocate(pItem);
#endif
	return 0;
}

DEFINE_HOOK(0x6294B7, ParasiteClass_DTOR, 0x7)
{
	//Debug::Log("%s Executed ! \n", __FUNCTION__);
	GET(ParasiteClass*, pItem, ESI);
	ParasiteExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x6296B0, ParasiteClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x6295B0, ParasiteClass_SaveLoad_Prefix, 0x5)
{
	//Debug::Log("%s Executed ! \n", __FUNCTION__);
	GET_STACK(ParasiteClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	ParasiteExt::ExtMap.PrepareStream(pItem, pStm);
	return 0;
}

DEFINE_HOOK_AGAIN(0x6296A7, ParasiteClass_Load_Suffix, 0x5)
DEFINE_HOOK(0x6296A0, ParasiteClass_Load_Suffix, 0x5)
{
	//Debug::Log("%s Executed ! \n", __FUNCTION__);
	ParasiteExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x6296C4, ParasiteClass_Save_Suffix, 0x5)
{
	//Debug::Log("%s Executed ! \n", __FUNCTION__);
	ParasiteExt::ExtMap.SaveStatic();
	return 0;
}
#endif