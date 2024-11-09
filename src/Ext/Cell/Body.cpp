#include "Body.h"

#include <TiberiumClass.h>

TiberiumClass* CellExtData::GetTiberium(CellClass* pCell)
{
	if (pCell->OverlayTypeIndex != -1)
		if (const auto pTiberium = TiberiumClass::Find(pCell->OverlayTypeIndex))
			return pTiberium;

	return nullptr;
}

int CellExtData::GetOverlayIndex(CellClass* pCell, TiberiumClass* pTiberium)
{
	if (pTiberium) {
		return (pCell->SlopeIndex > 0) ?
		(pCell->SlopeIndex + pTiberium->Image->ArrayIndex + pTiberium->NumImages - 1) : (pTiberium->Image->ArrayIndex + pCell->MapCoords.X * pCell->MapCoords.Y % pTiberium->NumImages)
		;
	}

	return 0;
}

int CellExtData::GetOverlayIndex(CellClass* pCell)
{
	if (pCell->OverlayTypeIndex != -1) {
		if (const auto pTiberium = TiberiumClass::Find(pCell->OverlayTypeIndex)) {
			return (pCell->SlopeIndex > 0) ?
			(pCell->SlopeIndex + pTiberium->Image->ArrayIndex + pTiberium->NumImages - 1) : (pTiberium->Image->ArrayIndex + pCell->MapCoords.X * pCell->MapCoords.Y % pTiberium->NumImages);
		}
	}

	return 0 ;
}

// ============================ =
// load / save
template <typename T>
void CellExtData::Serialize(T& Stm) {

	Stm
		.Process(this->Initialized)
		.Process(this->NewPowerups)
		;
}

// =============================
// container
CellExtContainer CellExtContainer::Instance;

// =============================
// container hooks

//DEFINE_HOOK(0x47BDA1, CellClass_CTOR, 0x5)
//{
//	GET(CellClass*, pItem, ESI);
//	CellExtContainer::Instance.Allocate(pItem);
//	return 0;
//}
//
//DEFINE_HOOK(0x47BB60, CellClass_DTOR, 0x6)
//{
//	GET(CellClass*, pItem, ECX);
//	CellExtContainer::Instance.Remove(pItem);
//	return 0;
//}
//
//DEFINE_HOOK_AGAIN(0x483C10, CellClass_SaveLoad_Prefix, 0x5)
//DEFINE_HOOK(0x4839F0, CellClass_SaveLoad_Prefix, 0x7)
//{
//	GET_STACK(CellClass*, pItem, 0x4);
//	GET_STACK(IStream*, pStm, 0x8);
//	CellExtContainer::Instance.PrepareStream(pItem, pStm);
//	return 0;
//}
//
//DEFINE_HOOK(0x483C00, CellClass_Load_Suffix, 0x5)
//{
//	CellExtContainer::Instance.LoadStatic();
//	return 0;
//}
//
//DEFINE_HOOK(0x483C79, CellClass_Save_Suffix, 0x6)
//{
//	CellExtContainer::Instance.SaveStatic();
//	return 0;
//}