#include "Body.h"

CellExt::ExtContainer CellExt::ExtMap;

TiberiumClass* CellExt::GetTiberium(CellClass* pCell)
{
	if (pCell->OverlayTypeIndex != -1)
		if (const auto pTiberium = TiberiumClass::Array->GetItemOrDefault(TiberiumClass::FindIndex(pCell->OverlayTypeIndex)))
			return pTiberium;

	return nullptr;
}

int CellExt::GetOverlayIndex(CellClass* pCell, TiberiumClass* pTiberium)
{
	if (pTiberium) {
		return (pCell->SlopeIndex > 0) ?
		(pCell->SlopeIndex + pTiberium->Image->ArrayIndex + pTiberium->NumImages - 1) : (pTiberium->Image->ArrayIndex + pCell->MapCoords.X * pCell->MapCoords.Y % pTiberium->NumImages)
		;
	}

	return 0;
}

int CellExt::GetOverlayIndex(CellClass* pCell)
{
	if (pCell->OverlayTypeIndex != -1) {
		if (const auto pTiberium = TiberiumClass::Array->GetItemOrDefault(TiberiumClass::FindIndex(pCell->OverlayTypeIndex))) {
			return (pCell->SlopeIndex > 0) ?
			(pCell->SlopeIndex + pTiberium->Image->ArrayIndex + pTiberium->NumImages - 1) : (pTiberium->Image->ArrayIndex + pCell->MapCoords.X * pCell->MapCoords.Y % pTiberium->NumImages);
		}
	}

	return 0 ;
}

// ============================ =
// load / save
template <typename T>
void CellExt::ExtData::Serialize(T& Stm) {

	Stm
		//.Process(AttachedTerrain)
		;
}

void CellExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	TExtension<CellClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void CellExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	TExtension<CellClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool CellExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool CellExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

CellExt::ExtContainer::ExtContainer() : TExtensionContainer("CellClass") { };
CellExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks
/* loading this from save causing performance issues for some reason :s*/

//DEFINE_HOOK(0x47BDA1, CellClass_CTOR, 0x5)
//{
//	GET(CellClass*, pItem, ESI);
////#ifdef ENABLE_NEWHOOKS
//	CellExt::ExtMap.JustAllocate(pItem, pItem, "Trying To Allocate from nullptr !");
////#else
////	CellExt::ExtMap.FindOrAllocate(pItem);
////#endif
//	return 0;
//}
//
//DEFINE_HOOK(0x47BB60, CellClass_DTOR, 0x6)
//{
//	GET(CellClass*, pItem, ECX);
//	CellExt::ExtMap.Remove(pItem);
//	return 0;
//}
//
//DEFINE_HOOK_AGAIN(0x483C10, CellClass_SaveLoad_Prefix, 0x5)
//DEFINE_HOOK(0x4839F0, CellClass_SaveLoad_Prefix, 0x7)
//{
//	GET_STACK(CellClass*, pItem, 0x4);
//	GET_STACK(IStream*, pStm, 0x8);
//	CellExt::ExtMap.PrepareStream(pItem, pStm);
//	return 0;
//}
//
//DEFINE_HOOK(0x483C00, CellClass_Load_Suffix, 0x5)
//{
//	CellExt::ExtMap.LoadStatic();
//	return 0;
//}
//
//DEFINE_HOOK(0x483C79, CellClass_Save_Suffix, 0x6)
//{
//	CellExt::ExtMap.SaveStatic();
//	return 0;
//}