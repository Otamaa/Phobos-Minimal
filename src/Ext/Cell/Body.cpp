#include "Body.h"

#include <TiberiumClass.h>

#include <Utilities/Macro.h>

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

#include <OverlayClass.h>
#include <Ext/TerrainType/Body.h>
#include <Ext/Terrain/Body.h>

static int __fastcall GetTiberiumType(int Overlay) {

    if ( Overlay == -1) {
        return -1;
    }

	auto pOverlay = OverlayTypeClass::Array->Items[Overlay];
    if (!pOverlay->Tiberium || TiberiumClass::Array->Count <= 0) {
        return -1;
    }


	for(auto pTib : *TiberiumClass::Array) {
		const auto v5 = pTib->Image->ArrayIndex;
		if(Overlay >= v5 && Overlay < (v5 + pTib->NumImages)) {
			return pTib->ArrayIndex;
		}

		int NumImages = pTib->NumImages;
        if (Overlay >= NumImages + v5 && Overlay < v5 + NumImages + pTib->SlopeFrames ) {
             return pTib->ArrayIndex;
         }
	}

	//Debug::LogInfo("Overlay [%s - %s] not really tiberium[%d]", pOverlay->ID , pOverlay->Name , 0);
	return 0;
}

// DEFINE_JUMP(CALL,0x47C206, MiscTools::to_DWORD(&GetTiberiumType));
// DEFINE_JUMP(CALL,0x47C966, MiscTools::to_DWORD(&GetTiberiumType));
// DEFINE_JUMP(CALL,0x47D820, MiscTools::to_DWORD(&GetTiberiumType));
// DEFINE_JUMP(CALL,0x47F863, MiscTools::to_DWORD(&GetTiberiumType));
// DEFINE_JUMP(CALL,0x47FBCE, MiscTools::to_DWORD(&GetTiberiumType));
// DEFINE_JUMP(CALL,0x480B6E, MiscTools::to_DWORD(&GetTiberiumType));
// DEFINE_JUMP(CALL,0x4818F3, MiscTools::to_DWORD(&GetTiberiumType));
// DEFINE_JUMP(CALL,0x4819AE, MiscTools::to_DWORD(&GetTiberiumType));
// DEFINE_JUMP(CALL,0x483639, MiscTools::to_DWORD(&GetTiberiumType));
// DEFINE_JUMP(CALL,0x4836A6, MiscTools::to_DWORD(&GetTiberiumType));
// DEFINE_JUMP(CALL,0x483725, MiscTools::to_DWORD(&GetTiberiumType));
// DEFINE_JUMP(CALL,0x48376B, MiscTools::to_DWORD(&GetTiberiumType));
// DEFINE_JUMP(CALL,0x48379E, MiscTools::to_DWORD(&GetTiberiumType));
// DEFINE_JUMP(CALL,0x4837EE, MiscTools::to_DWORD(&GetTiberiumType));
// DEFINE_JUMP(CALL,0x484FF3, MiscTools::to_DWORD(&GetTiberiumType));
// DEFINE_JUMP(CALL,0x485013, MiscTools::to_DWORD(&GetTiberiumType));
// DEFINE_JUMP(CALL,0x485026, MiscTools::to_DWORD(&GetTiberiumType));
// DEFINE_JUMP(CALL,0x487393, MiscTools::to_DWORD(&GetTiberiumType));
// DEFINE_JUMP(CALL,0x4873E9, MiscTools::to_DWORD(&GetTiberiumType));
// DEFINE_JUMP(CALL,0x5FDA77, MiscTools::to_DWORD(&GetTiberiumType));
// DEFINE_JUMP(CALL,0x74A3A2, MiscTools::to_DWORD(&GetTiberiumType));
// DEFINE_JUMP(CALL,0x74A602, MiscTools::to_DWORD(&GetTiberiumType));
DEFINE_JUMP(LJMP,0x5FDD20, MiscTools::to_DWORD(&GetTiberiumType));

int FakeCellClass::_GetTiberiumType()
{
    if (this->OverlayTypeIndex == -1) {
        return -1;
    }

	auto pOverlay = OverlayTypeClass::Array->Items[this->OverlayTypeIndex];
    if (!pOverlay->Tiberium || TiberiumClass::Array->Count <= 0) {
        return -1;
    }


	for(auto pTib : *TiberiumClass::Array) {
		const auto v5 = pTib->Image->ArrayIndex;
		if(this->OverlayTypeIndex >= v5 && this->OverlayTypeIndex < (v5 + pTib->NumImages)) {
			return pTib->ArrayIndex;
		}

		int NumImages = pTib->NumImages;
        if (this->OverlayTypeIndex >= NumImages + v5 && this->OverlayTypeIndex < v5 + NumImages + pTib->SlopeFrames ) {
             return pTib->ArrayIndex;
         }
	}

	//Debug::LogInfo("Overlay [%s - %s] not really tiberium[%d]", pOverlay->ID , pOverlay->Name , 0);
	return 0;
}

DEFINE_JUMP(CALL,0x424D9C, MiscTools::to_DWORD(&FakeCellClass::_GetTiberiumType));
DEFINE_JUMP(CALL,0x452FEC, MiscTools::to_DWORD(&FakeCellClass::_GetTiberiumType));
DEFINE_JUMP(CALL,0x522EFD, MiscTools::to_DWORD(&FakeCellClass::_GetTiberiumType));
DEFINE_JUMP(CALL,0x5A2D29, MiscTools::to_DWORD(&FakeCellClass::_GetTiberiumType));
DEFINE_JUMP(CALL,0x5FD1BB, MiscTools::to_DWORD(&FakeCellClass::_GetTiberiumType));
DEFINE_JUMP(CALL,0x72291E, MiscTools::to_DWORD(&FakeCellClass::_GetTiberiumType));
DEFINE_JUMP(CALL,0x723009, MiscTools::to_DWORD(&FakeCellClass::_GetTiberiumType));
DEFINE_JUMP(CALL,0x72340E, MiscTools::to_DWORD(&FakeCellClass::_GetTiberiumType));
DEFINE_JUMP(CALL,0x73D541, MiscTools::to_DWORD(&FakeCellClass::_GetTiberiumType));

bool FakeCellClass::_SpreadTiberium(bool force)
{
	int tib_ = -1;
	if (!force && !ScenarioClass::Instance->SpecialFlags.StructEd.TiberiumSpreads)
		return false;

	tib_ = GetTiberiumType(this->OverlayTypeIndex);

	if (!force)
	{

		if (tib_ == -1
			  || this->OverlayData <= tib_ / 2
			  || this->SlopeIndex
			  || TiberiumClass::Array->Items[tib_]->SpreadPercentage < 0.00001
			  || this->FirstObject)
		{
			return false;
		}

	}
	else
	{
		if (tib_ == -1)
		{
			tib_ = 0;
		}
	}

	auto pTib = TiberiumClass::Array->Items[tib_];
	auto facing = (BYTE)ScenarioClass::Instance->Random.RandomRangedSpecific(FacingType::Min, FacingType::Max);
	int index = 0;
	CellClass* newcell = nullptr;

	while (true)
	{
		const auto v9 = ((BYTE)index + facing) & 7;

		if (v9 < 8)
		{
			newcell = MapClass::Instance->GetCellAt(CellSpread::AdjacentCell[v9] + this->MapCoords);
		}
		else
		{
			newcell = this;
		}
		if (newcell && newcell->CanTiberiumGerminate(pTib))
		{
			break;
		}

		if (++index >= 8)
		{
			return false;
		}
	}

	return newcell->IncreaseTiberium(tib_, 3);
}

bool FakeCellClass::_SpreadTiberium_2(TerrainClass* pTerrain, bool force)
{
	if (!pTerrain)
		Debug::FatalErrorAndExit(__FUNCTION__" Need `TerrainClass` !");

	auto pTerrainTypeExt = TerrainTypeExtContainer::Instance.Find(pTerrain->Type);

	size_t tib_ = (size_t)pTerrainTypeExt->SpawnsTiberium_Type;

	if (tib_ >= (size_t)TiberiumClass::Array->Count)
		tib_ = GetTiberiumType(this->OverlayTypeIndex);

	if (!force)
	{
		if (!ScenarioClass::Instance->SpecialFlags.StructEd.TiberiumSpreads)
		{
			return false;
		}

		if (tib_ >= (size_t)TiberiumClass::Array->Count || (TiberiumClass::Array->Items[tib_]->SlopeFrames <= 0 && this->SlopeIndex))
			return false;

		if (TiberiumClass::Array->Items[tib_]->SpreadPercentage < 0.00001
			|| this->FirstObject)
		{
			return false;
		}

	}
	else
	{
		if (tib_ >= (size_t)TiberiumClass::Array->Count)
		{
			tib_ = 0;
		}
	}

	auto pTib = TiberiumClass::Array->Items[tib_];
	auto pTerrainExt = TerrainExtContainer::Instance.Find(pTerrain);
	size_t size = pTerrainExt->Adjencentcells.size();
	const int rand = ScenarioClass::Instance->Random.RandomFromMax(size - 1);
	const int growth = pTerrainTypeExt->GetTiberiumGrowthStage();

	for (int i = 0; i < (int)size; i++)
	{
		CellClass* tgtCell = MapClass::Instance->GetCellAt(this->MapCoords + pTerrainExt->Adjencentcells[(i + rand) % size]);

		if (tgtCell->CanTiberiumGerminate(pTib))
		{
			return tgtCell->IncreaseTiberium(tib_, growth);
		}
	}

	return false;
}

void FakeCellClass::_Invalidate(AbstractClass* ptr, bool removed)
{
	auto pExt = this->_GetExtData();
	
	if (removed)
	{
		if (ptr == static_cast<void*>(this->AltObject)) {
			Debug::LogInfo("Cell {} - at ( {} . {} ) with Invalud Alt Obj {}", (void*)this, this->MapCoords.X , this->MapCoords.Y , (void*)this->AltObject);
		}

		if (ptr == static_cast<void*>(this->FirstObject)) {
			Debug::LogInfo("Cell {} - at ( {} . {} ) with Invalud Obj {}", (void*)this, this->MapCoords.X, this->MapCoords.Y, (void*)this->FirstObject);
		}

		if (ptr == static_cast<void*>(pExt->IncomingUnit)) {
			Debug::LogInfo("Cell {} - at ( {} . {} ) with Invalud IncomingObj {}", (void*)this, this->MapCoords.X, this->MapCoords.Y, (void*)this->FirstObject);

		}

		if (ptr == static_cast<void*>(pExt->IncomingUnitAlt)) {
			Debug::LogInfo("Cell {} - at ( {} . {} ) with Invalud IncomingAltObj {}", (void*)this, this->MapCoords.X, this->MapCoords.Y, (void*)this->FirstObject);
		}
	}
}

// ============================ =
// load / save
template <typename T>
void CellExtData::Serialize(T& Stm) {

	Stm
		.Process(this->Initialized)
		.Process(this->NewPowerups)
		.Process(this->IncomingUnit)
		.Process(this->IncomingUnitAlt)
		;
}

// =============================
// container
CellExtContainer CellExtContainer::Instance;

// =============================
// container hooks

DEFINE_HOOK(0x47BDA1, CellClass_CTOR, 0x5)
{
	GET(CellClass*, pItem, ESI);
	CellExtContainer::Instance.Allocate(pItem);
	return 0;
}

DEFINE_HOOK(0x47BB60, CellClass_DTOR, 0x6)
{
	GET(CellClass*, pItem, ECX);
	CellExtContainer::Instance.Remove(pItem);
	return 0;
}


HRESULT __stdcall FakeCellClass::_Load(IStream* pStm)
{
	CellExtContainer::Instance.PrepareStream(this, pStm);
	HRESULT res = this->CellClass::Load(pStm);

	if (SUCCEEDED(res))
		CellExtContainer::Instance.LoadStatic();

	return res;
}

HRESULT __stdcall FakeCellClass::_Save(IStream* pStm, bool clearDirty)
{
	CellExtContainer::Instance.PrepareStream(this, pStm);
	HRESULT res = this->CellClass::Save(pStm, clearDirty);

	if (SUCCEEDED(res))
		CellExtContainer::Instance.SaveStatic();

	return res;
}

DEFINE_JUMP(VTABLE, 0x7E4F00, MiscTools::to_DWORD(&FakeCellClass::_Load))
DEFINE_JUMP(VTABLE, 0x7E4F04, MiscTools::to_DWORD(&FakeCellClass::_Save))

DEFINE_JUMP(VTABLE, 0x7E4F14, MiscTools::to_DWORD(&FakeCellClass::_Invalidate));
