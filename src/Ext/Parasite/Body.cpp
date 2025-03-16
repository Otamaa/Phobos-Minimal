#include "Body.h"

#include <Ext/Techno/Body.h>

void TechnoExtData::DrawParasitedPips(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds)
{
#ifdef PARASITE_PIPS
	{
		//bool IsHost = false;
		//bool IsSelected = false;					//Red         //Green           //White
		//ColorScheme Color = IsSelected ? (IsHost ? Drawings::ColorRed : Drawings::ColorGreen) : Drawings::ColorWhite;
		int xOffset = 0;
		int yOffset = 0;

		nBracket = TechnoExtData::GetDisguiseType, (pThis).first->PixelSelectionBracketDelta;

		switch ((((DWORD*)pThis)[0]))
		{
		case UnitClass::vtable:
		case AircraftClass::vtable:
		{
			const auto& offset = RulesExtData::Instance()->Pips_SelfHeal_Units_Offset.Get();
			xOffset = offset.X;
			yOffset = offset.Y + nBracket;
		}
		break;
		case InfantryClass::vtable:
		{
			const auto& offset = RulesExtData::Instance()->Pips_SelfHeal_Infantry_Offset.Get();
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
//template <typename T>
//void ParasiteExt::ExtData::Serialize(T& Stm) {
//	//Debug::LogInfo("Processing Element From ParasiteExt ! ");
//	Stm
//		.Process(this->Initialized)
//		.Process(this->LastVictimLocation)
//		;
//}

// =============================
// container
//ParasiteExt::ExtContainer ParasiteExt::ExtMap;

// =============================
// container hooks

//ASMJIT_PATCH_AGAIN(0x62924C , ParasiteClass_CTOR,0x5 )
//ASMJIT_PATCH(0x62932E, ParasiteClass_CTOR, 0x6)
//{
//	GET(ParasiteClass*, pItem, ESI);
//	ParasiteExt::ExtMap.Allocate(pItem);
//	return 0;
//}
//
//ASMJIT_PATCH_AGAIN(0x62AFFE , ParasiteClass_DTOR, 0x6)
//ASMJIT_PATCH(0x62946E, ParasiteClass_DTOR, 0x6)
//{
//	GET(ParasiteClass*, pItem, ESI);
//	ParasiteExt::ExtMap.Remove(pItem);
//	return 0;
//}
//
//ASMJIT_PATCH_AGAIN(0x6296B0, ParasiteClass_SaveLoad_Prefix, 0x8)
//ASMJIT_PATCH(0x6295B0, ParasiteClass_SaveLoad_Prefix, 0x5)
//{
//
//	GET_STACK(ParasiteClass*, pItem, 0x4);
//	GET_STACK(IStream*, pStm, 0x8);
//
//	ParasiteExt::ExtMap.PrepareStream(pItem, pStm);
//	return 0;
//}
//
//ASMJIT_PATCH(0x62969D, ParasiteClass_Load_Suffix, 0x5)
//{
//	ParasiteExt::ExtMap.LoadStatic();
//	return 0;
//}
//
//ASMJIT_PATCH(0x6296BC, ParasiteClass_Save_Suffix, 0x8)
//{
//	GET(ParasiteClass*, pThis, ECX);
//	GET(IStream*, pStream, EAX);
//	GET(BOOL, bClearDirty, EAX);
//
//	auto const nRes = AbstractClass::_Save(pThis, pStream, bClearDirty);
//
//	if (SUCCEEDED(nRes))
//		ParasiteExt::ExtMap.SaveStatic();
//
//	R->EAX(nRes);
//	return 0x6296C4;
//}
//detach