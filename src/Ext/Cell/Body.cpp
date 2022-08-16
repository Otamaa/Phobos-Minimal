#include "Body.h"

CellExt::ExtContainer CellExt::ExtMap;

int CellExt::GetOverlayIndex(CellClass* pCell)
{
	if (pCell->OverlayTypeIndex != -1)
	{
		const auto nTibIndex = TiberiumClass::FindIndex(pCell->OverlayTypeIndex);

		if (nTibIndex != -1)
		{
			if (const auto pTiberium = TiberiumClass::Array->GetItem(nTibIndex))
			{
				if (pCell->SlopeIndex > 0)
					return pCell->SlopeIndex + pTiberium->Image->ArrayIndex + pTiberium->NumImages - 1;

				return pTiberium->Image->ArrayIndex + pCell->MapCoords.X * pCell->MapCoords.Y % pTiberium->NumImages;
			}
		}
	}

	return 0;
}

// ============================ =
// load / save
template <typename T>
void CellExt::ExtData::Serialize(T& Stm) {

	Stm
		;
}

void CellExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<CellClass>::Serialize(Stm);
	this->Serialize(Stm);
}

void CellExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<CellClass>::Serialize(Stm);
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

CellExt::ExtContainer::ExtContainer() : Container("CellClass") { };
CellExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks
/* loading this from save causing performance issues for some reason :s*/

#ifdef ENABLE_NEWHOOKS
DEFINE_HOOK(0x47BDA3, CellClass_CTOR, 0x5)
{
	GET(CellClass*, pItem, EAX);
#ifdef ENABLE_NEWHOOKS
	CellExt::ExtMap.JustAllocate(pItem, pItem, "Trying To Allocate from nullptr !");
#else
	CellExt::ExtMap.FindOrAllocate(pItem);
#endif
	return 0;
}

DEFINE_HOOK(0x47BB60, CellClass_DTOR, 0x6)
{
	GET(CellClass*, pItem, ECX);
	CellExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x483C10, CellClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x4839F0, CellClass_SaveLoad_Prefix, 0x7)
{
	GET_STACK(CellClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);
	CellExt::ExtMap.PrepareStream(pItem, pStm);
	return 0;
}

DEFINE_HOOK(0x483C00, CellClass_Load_Suffix, 0x5)
{
	CellExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x483C79, CellClass_Save_Suffix, 0x6)
{
	CellExt::ExtMap.SaveStatic();
	return 0;
}
#endif