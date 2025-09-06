#include "Body.h"

#include <Ext/Cell/Body.h>

#include <InfantryClass.h>

bool TiberiumExtData::LoadFromINI(CCINIClass* pINI, bool parseFailAddr)
{
	if (parseFailAddr)
		return false;

	auto pThis = this->This();
	const char* pSection = pThis->ID;

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
	Nullable<int> Variety { };

	Variety.Read(exINI, pSection, "Variety");
	int MaxCount = !slopes ? 12 : 20;

	if(Variety.isset()) {
		MaxCount = MaxImpl(MaxCount, Variety.Get());
	}

	if(!this->LinkedOverlayType->empty()) {

		OverlayTypeClass* first = nullptr;

		for (int i = 0; i < MaxCount; ++i) {
			const std::string Find = (this->LinkedOverlayType.Get() + fmt::format("{:02}", i + 1));
			OverlayTypeClass* pOverlay = OverlayTypeClass::Find(Find.c_str());

			if (!pOverlay)
				Debug::FatalErrorAndExit("CannotFind %s OverlayType for Tiberium[%s]", Find.c_str(), pSection);

			if(!pOverlay->Tiberium)
				Debug::FatalErrorAndExit("OverlayType[%s] for Tiberium[%s] is not Tiberium", Find.c_str(), pSection);

			if (i == 0) {
				first = pOverlay;

				auto iter = TiberiumExtContainer::LinkedType.get_key_iterator(pOverlay);

				if (iter != TiberiumExtContainer::LinkedType.end()) {
					if (iter->second != this->This())
						Debug::FatalErrorAndExit("OverlayType[%s] already assigned to [%s] Tiberium! ", pOverlay->ID, iter->second->ID);
				} else {
					TiberiumExtContainer::LinkedType.emplace_unchecked(pOverlay, this->This());
				}
			}
			else if (first && pOverlay->ArrayIndex != (first->ArrayIndex + i)) {
				Debug::FatalErrorAndExit("OverlayType index of [%s - %d] is invalid compared to the first[%s - %d] (+ %d) ", Find.c_str(), pOverlay->ArrayIndex, i ,first->ID, first->ArrayIndex);
			}

			//if (Phobos::Otamaa::IsAdmin)
			//	Debug::LogInfo("Reading[%s] With CurOverlay[%s] ", pSection, Find.c_str());
		}

		detail::read<int>(pThis->NumFrames, exINI, pSection, "NumFrames");
		pThis->SlopeFrames = !slopes ? 0 : 8;
		pThis->NumImages = MaxCount;
	}

	return true;
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

static reference<RectangleStruct, 0x87F8D4> const MapSize {};

COMPILETIMEEVAL int GetMapSizeTotals()
{
	return (2 * MapSize->Width * (MapSize->Height + 4));
}

COMPILETIMEEVAL int CellStruct_totibarray_42B1C0(CellStruct* pCell)
{
	return (((pCell->X - pCell->Y + MapSize->Width - 1) >> 1) + MapSize->Width * (pCell->X - MapSize->Width + pCell->Y - 1));
}

COMPILETIMEEVAL void Heapify(TPriorityQueueClass<MapSurfaceData>* pHeap, int index)
{
	int left = 2 * index;
	int right = 2 * index + 1;
	if (2 * index > pHeap->Count || pHeap->Nodes[index]->Score <= (double)pHeap->Nodes[2 * index]->Score)
	{
		left = index;
	}
	if (right <= pHeap->Count && pHeap->Nodes[left]->Score > (double)pHeap->Nodes[right]->Score)
	{
		left = 2 * index + 1;
	}
	while (left != index)
	{
		auto left_of_left = 2 * left + 1;
		auto entry = pHeap->Nodes[index];
		pHeap->Nodes[index] = pHeap->Nodes[left];
		index = left;
		pHeap->Nodes[left] = entry;
		if (2 * left <= pHeap->Count && pHeap->Nodes[left]->Score > (double)pHeap->Nodes[2 * left]->Score)
		{
			left *= 2;
		}
		if (left_of_left <= pHeap->Count && pHeap->Nodes[left]->Score > (double)pHeap->Nodes[left_of_left]->Score)
		{
			left = left_of_left;
		}
	}
}

#pragma region Spread

void FakeTiberiumClass::__RecalcSpreadData()
{
	this->Spread = 0;
	this->SpreadLogic.Heap->Clear();

	const int MapTotal = GetMapSizeTotals();
	for (int i = MapTotal - 1; i >= 0; this->SpreadLogic.States[i + 1] = 0)
	{
		--i;
	}

	MapClass::Instance->CellIteratorReset();
	for (auto j = MapClass::Instance->CellIteratorNext(); j; j = MapClass::Instance->CellIteratorNext())
	{
		if (j->GetContainedTiberiumIndex() == this->ArrayIndex && j->CanTiberiumSpread())
		{
			this->SpreadLogic.Datas[this->Spread].MapCoord = j->MapCoords;
			this->SpreadLogic.Datas[this->Spread].Score = 0.0;
			this->SpreadLogic.Heap->WWPush(&this->SpreadLogic.Datas[this->Spread]);
			++this->Spread;
			this->SpreadLogic.States[CellStruct_totibarray_42B1C0(&j->MapCoords)] = true;
		}
	}
}

void FakeTiberiumClass::__QueueSpreadAt(CellStruct* pCell)
{
	int tib_arr = CellStruct_totibarray_42B1C0(pCell);
	auto pCellClass = MapClass::Instance->GetCellAt(pCell);

	if (pCellClass->CanTiberiumSpread() && !this->SpreadLogic.States[tib_arr])
	{
		if (this->Spread >= GetMapSizeTotals() - 20)
			__RecalcSpreadData();

		this->SpreadLogic.Datas[this->Spread].MapCoord = *pCell;
		this->SpreadLogic.Datas[this->Spread].Score = (float)(Unsorted::CurrentFrame + (int)abs(abs(ScenarioClass::Instance->Random.Random() % 50)));
		this->SpreadLogic.Heap->WWPush(&this->SpreadLogic.Datas[this->Spread]);
		++this->Spread;
		this->SpreadLogic.States[tib_arr] = true;
	}
}

void FakeTiberiumClass::__Spread()
{
	auto spreadHeaps = this->SpreadLogic.Heap;

	if (spreadHeaps && spreadHeaps->Count && this->SpreadPercentage > 0.00001)
	{
		int get_percent = int(spreadHeaps->Count * this->SpreadPercentage);

		if (get_percent < 5)
			get_percent = 5;
		else if (get_percent > 25)
			get_percent = 25;

		if (spreadHeaps->Count > GetMapSizeTotals() - 20)
			this->__RecalcSpreadData();

		MapSurfaceData* pSurface = nullptr;
		if (auto poped = spreadHeaps->Top())
		{
			pSurface = poped;
			Heapify(spreadHeaps, 1);
		}

		int size_after = abs(ScenarioClass::Instance->Random.Random()) % get_percent + 1;

		int increment = 0;
		if (size_after > 0)
		{
			while (true)
			{
				while (true)
				{
					if (!pSurface)
						return;

					int getminateIdx = 0;
					auto pNewCell = MapClass::Instance->GetCellAt(pSurface->MapCoord);
					for (int c = 0; c < 8; ++c)
					{
						auto pAdjencent = pNewCell->GetAdjacentCell((FacingType)c);
						if (pAdjencent->CanTiberiumGerminate(nullptr))
							++getminateIdx;
					}

					if (getminateIdx)
					{
						((FakeCellClass*)(pNewCell))->_SpreadTiberium(false);
						++increment;
						if (getminateIdx > 1)
						{
							this->SpreadLogic.Datas[this->Spread].MapCoord = pNewCell->MapCoords;
							this->SpreadLogic.Datas[this->Spread].Score = 0.0f;
							this->SpreadLogic.Heap->WWPush(&this->SpreadLogic.Datas[this->Spread]);
							++this->Spread;
							this->SpreadLogic.States[CellStruct_totibarray_42B1C0(&pNewCell->MapCoords)] = true;
						}
					}
					else
					{
						this->SpreadLogic.States[CellStruct_totibarray_42B1C0(&pNewCell->MapCoords)] = false;
					}

					if (++increment >= size_after)
						return;

					if (this->SpreadLogic.Heap->Count)
						break;
				}

				Heapify(this->SpreadLogic.Heap, 1);
			}
		}
	}
}

#pragma endregion

#pragma region Growth

void FakeTiberiumClass::__RecalcGrowthData()
{
	this->Growth = 0;
	this->GrowthLogic.Heap->Clear();
	for (int i = GetMapSizeTotals() - 1; i >= 0; this->GrowthLogic.States[i + 1] = 0)
	{
		--i;
	}

	MapClass::Instance->CellIteratorReset();
	for (auto j = MapClass::Instance->CellIteratorNext(); j; j = MapClass::Instance->CellIteratorNext())
	{
		if (j->GetContainedTiberiumIndex() == this->ArrayIndex && j->CanTiberiumGrowth())
		{
			//auto p_Position = &j->MapCoords;
			//auto v16 = p_Position;
			this->GrowthLogic.Datas[this->Growth].MapCoord = j->MapCoords;
			this->GrowthLogic.Datas[this->Growth].Score = 0.0;
			this->GrowthLogic.Heap->WWPush(&this->GrowthLogic.Datas[this->Growth]);
			++this->Growth;
			this->GrowthLogic.States[CellStruct_totibarray_42B1C0(&j->MapCoords)] = true;
		}
	}
}

void FakeTiberiumClass::__QueueGrowthAt(CellStruct* pCell)
{
	int tib_arr = CellStruct_totibarray_42B1C0(pCell);
	auto pCellClass = MapClass::Instance->GetCellAt(pCell);

	if (pCellClass->OverlayData < 11u)
	{
		if (this->Growth >= GetMapSizeTotals() - 10)
			__RecalcGrowthData();

		this->GrowthLogic.Datas[this->Growth].MapCoord = *pCell;
		this->GrowthLogic.Datas[this->Growth].Score = (float)(Unsorted::CurrentFrame + (int)abs(abs(ScenarioClass::Instance->Random.Random() % 50)));
		this->GrowthLogic.Heap->WWPush(&this->GrowthLogic.Datas[this->Growth]);
		++this->Growth;
		this->GrowthLogic.States[tib_arr] = true;
	}
}

void FakeTiberiumClass::__Growth()
{
	auto growthHeaps = this->GrowthLogic.Heap;

	if (growthHeaps && growthHeaps->Count && this->GrowthPercentage > 0.00001)
	{
		int get_percent = int(growthHeaps->Count * this->GrowthPercentage);

		if (get_percent < 5)
			get_percent = 5;
		else if (get_percent > 50)
			get_percent = 50;

		int copy_heapSize = growthHeaps->Count;
		int size_after = abs(ScenarioClass::Instance->Random.Random()) % get_percent + 1;

		if (copy_heapSize > GetMapSizeTotals() - 2 * size_after)
		{
			this->__RecalcGrowthData();
		}

		MapSurfaceData* pSurface = nullptr;
		if (auto poped = growthHeaps->Top())
		{
			pSurface = poped;
			Heapify(growthHeaps, 1);
		}

		int increment = 0;
		if (size_after > 0)
		{

			while (true)
			{
				while (true)
				{
					if (!pSurface)
						return;

					auto pNewCell = (FakeCellClass*)MapClass::Instance->GetCellAt(pSurface->MapCoord);

					if (pNewCell->_GetTiberiumType() == this->ArrayIndex)
					{
						pNewCell->GrowTiberium();
						if (pNewCell->OverlayData >= 11u)
						{
							this->GrowthLogic.States[CellStruct_totibarray_42B1C0(&pNewCell->MapCoords)] = false;
						}
						else
						{
							this->GrowthLogic.Datas[this->Growth].MapCoord = pNewCell->MapCoords;
							this->GrowthLogic.Datas[this->Growth].Score = (float)(int)(Unsorted::CurrentFrame() + abs(ScenarioClass::Instance->Random.Random() % 50));
							this->GrowthLogic.Heap->WWPush(&this->GrowthLogic.Datas[this->Growth]);
							++this->Growth;
							__QueueSpreadAt(&pNewCell->MapCoords);
						}
					}
					if (++increment >= size_after)
						return;

					if (this->GrowthLogic.Heap->Count)
						break;
				}

				Heapify(growthHeaps, 1);
			}
		}
	}
}

#pragma endregion

// =============================
// container
template <typename T>
void TiberiumExtData::Serialize(T& Stm)
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
PhobosMap<OverlayTypeClass*, TiberiumClass*> TiberiumExtContainer::LinkedType;
std::vector<TiberiumExtData*> Container<TiberiumExtData>::Array;

bool TiberiumExtContainer::LoadGlobals(PhobosStreamReader& Stm)
{
	Clear();

	size_t Count = 0;
	if (!Stm.Load(Count))
		return false;

	Array.reserve(Count);

	for (size_t i = 0; i < Count; ++i)
	{

		void* oldPtr = nullptr;

		if (!Stm.Load(oldPtr))
			return false;

		auto newPtr = new TiberiumExtData(nullptr, noinit_t());
		PHOBOS_SWIZZLE_REGISTER_POINTER((long)oldPtr, newPtr, "TiberiumExtData")
		ExtensionSwizzleManager::RegisterExtensionPointer(oldPtr, newPtr);
		newPtr->LoadFromStream(Stm);
		Array.push_back(newPtr);
	}

	return Stm
		.Process(LinkedType)
		.Success();
}

bool TiberiumExtContainer::SaveGlobals(PhobosStreamWriter& Stm)
{
	Stm.Save(Array.size());

	for (auto& item : Array)
	{
		// write old pointer and name, then delegate
		Stm.Save(item);
		item->SaveToStream(Stm);
	}

	return Stm
		.Process(LinkedType)
		.Success();
}

void TiberiumExtContainer::Clear()
{
	Array.clear();
	LinkedType.clear();
}

// =============================
// container hooks

// was 7217CC
ASMJIT_PATCH(0x721876, TiberiumClass_CTOR, 0x5)
{
	GET(TiberiumClass*, pItem, ESI);
	TiberiumExtContainer::Instance.Allocate(pItem);
	return 0;
}

ASMJIT_PATCH(0x721888, TiberiumClass_DTOR, 0x6)
{
	GET(TiberiumClass*, pItem, ECX);
	TiberiumExtContainer::Instance.Remove(pItem);
	return 0;
}

ASMJIT_PATCH(0x721C7B, TiberiumClass_LoadFromINI, 0xA)
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
}ASMJIT_PATCH_AGAIN(0x721CDC, TiberiumClass_LoadFromINI, 0xA)
ASMJIT_PATCH_AGAIN(0x721CE9, TiberiumClass_LoadFromINI, 0xA)

HRESULT __stdcall FakeTiberiumClass::_Load(IStream* pStm)
{
	auto hr = this->TiberiumClass::Load(pStm);

	if (SUCCEEDED(hr))
	{
		hr = TiberiumExtContainer::Instance.ReadDataFromTheByteStream(this,
			TiberiumExtContainer::Instance.AllocateNoInit(this), pStm);
	}

	return hr;
}

HRESULT __stdcall FakeTiberiumClass::_Save(IStream* pStm, BOOL clearDirty)
{
	auto hr = this->TiberiumClass::Save(pStm, clearDirty);

	if (SUCCEEDED(hr))
	{
		hr = TiberiumExtContainer::Instance.WriteDataToTheByteStream(this, pStm);
	}

	return hr;
}

//DEFINE_FUNCTION_JUMP(VTABLE, 0x7F573C, FakeTiberiumClass::_Load)
//DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5740, FakeTiberiumClass::_Save)