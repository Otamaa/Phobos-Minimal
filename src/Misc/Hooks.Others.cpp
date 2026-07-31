
#include <AbstractClass.h>
#include <RadarEventClass.h>
#include <InfantryClass.h>
#include <UnitClass.h>
#include <AircraftClass.h>
#include <BuildingClass.h>
#include <TeamClass.h>
#include <SlaveManagerClass.h>
#include <SpawnManagerClass.h>
#include <AirstrikeClass.h>

#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/Terrain/Body.h>
#include <Ext/InfantryType/Body.h>
#include <Ext/TeamType/Body.h>
#include <Ext/HouseType/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/Super/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Tiberium/Body.h>
#include <Ext/ParticleType/Body.h>
#include <Ext/Particle/Body.h>
#include <Ext/House/Body.h>

#include <Utilities/Helpers.h>

#include <Locomotor/HoverLocomotionClass.h>

#include <New/Type/ArmorTypeClass.h>

#include <Misc/PhobosGlobal.h>
#include <Misc/DamageArea.h>

#include <WWKeyboardClass.h>
#include <MPGameModeClass.h>
#include <LoadOptionsClass.h>
#include <VersionHelpers.h>
#include <FPSCounter.h>
#include <EventClass.h>
#include <dxcore.h>
#include <TerrainTypeClass.h>
#include <Notifications.h>

#include <strsafe.h>

#include "AresChecksummer.h"

#include <Misc/Spawner/Main.h>

//MapClass CTOR
DEFINE_JUMP(LJMP, 0x565215, 0x56522D); // i assume this one for fixing bug that crate are gone when loading the game

//ASMJIT_PATCH(0x6E2290, ActionClass_PlayAnimAt, 0x6)
//{
//	GET(TActionClass*, pThis, ECX);
//	GET_STACK(HouseClass*, pOwner, 0x4);
//	//GET_STACK(TechnoClass*, pInvoker, 0x8);
//	//GET_STACK(TriggerClass*, pTrigger, 0xC);
//	//GET_STACK(CellStruct*, pCell, 0x10);
//
//	auto nCell = ScenarioClass::Instance->GetWaypointCoords(pThis->Waypoint);
//	auto nCoord = CellClass::Cell2Coord(nCell);
//	nCoord.Z = MapClass::Instance->GetZPos(&nCoord);
//	auto pCellTarget = MapClass::Instance->GetCellAt(nCell);
//	nCoord = pCellTarget->GetCoordsWithBridge();
//
//	if (AnimTypeClass* AnimType = AnimTypeClass::Array->GetItemOrDefault(pThis->Value))
//	{
//		if (AnimClass* pAnim = GameCreate<AnimClass>(AnimType, nCoord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false))
//		{
//			pAnim->IsPlaying = true;
//			AnimExtData::SetAnimOwnerHouseKind(pAnim, pOwner, nullptr, false);
//		}
//	}
//
//	R->EAX(1);
//	return 0x6E2387;
//}

//Skirmish_DialogFunc_MultiEngineer
DEFINE_JUMP(LJMP, 0x6AD0ED, 0x6AD16C); //Allow solo skirmish

ASMJIT_PATCH(0x437CCC, BSurface_DrawSHPFrame1_Buffer, 0x8)
{
	REF_STACK(RectangleStruct const, bounds, STACK_OFFS(0x7C, 0x10));
	//0x89C568
	REF_STACK(unsigned char const*, pBuffer, STACK_OFFS(0x7C, 0x6C));

	auto const width = static_cast<size_t>(std::clamp<int>(
		static_cast<int>(bounds.Width), 0, std::numeric_limits<int>::max()));

	// buffer overrun is now not as forgiving as it was before
	auto& Buffer = PhobosGlobal::Instance()->ShpCompression1Buffer;

	if (Buffer.size() < width)
	{
		Buffer.insert(Buffer.end(), width - Buffer.size(), 0u);
	}

	pBuffer = Buffer.data();

	return 0x437CD4;
}

ASMJIT_PATCH(0x7387D1, UnitClass_Destroyed_Shake, 0x6)
{
	GET(UnitClass* const, pUnit, ESI); //forEXT

	if (!pUnit || !pUnit->Type || !RulesClass::Instance->ShakeScreen || Phobos::Config::HideShakeEffects)
		return 0x738801;

	if (!pUnit->Type->Strength)
		return 0x738801;

	if (!TechnoTypeExtContainer::Instance.Find(pUnit->Type)->DontShake.Get())
		TechnoExtData::ShakeScreen(pUnit, pUnit->Type->Strength, RulesClass::Instance->ShakeScreen);

	return 0x738801;
}

// replaced entire function. error was using delete[] instead of delete.
// it potentially crashed when any of the files were present in the
// game directory.
ASMJIT_PATCH(0x5F77F0, ObjectTypeClass_UnloadPipsSHP, 0x5)
{
	for (int i = 0; i < (int)TechnoTypeClass::ShapesIsAllocated.size(); ++i)
	{
		if (TechnoTypeClass::ShapesIsAllocated[i] && FileSystem::ShapesAllocated[i])
		{
			GameDelete<true, false>(std::exchange(FileSystem::ShapesAllocated[i], nullptr));
			TechnoTypeClass::ShapesIsAllocated[i] = false;
		}
	}

	return 0x5F78FB;
}

// naive way to fix negative indexes to be generated. proper way would be to replace
// the entire function, and the function consuming the indexes. it is not yet known
// whether the out of bounds read causes desync errors. this function appears to
// have been inlined prominently in 585F40
ASMJIT_PATCH(0x56BC54, ThreatPosedEstimates_GetIndex, 0x5)
{
	GET(const CellStruct*, pCell, ECX);

	int index = -1;
	if (pCell->X >= 0 && pCell->Y >= 0 && pCell->X < 512 && pCell->Y < 512)
	{
		index = pCell->X / 4 + 130 * pCell->Y / 4 + 131;
	}

	R->EAX(index);
	return 0x56BC7D;
}

//CellClass_Load
DEFINE_JUMP(LJMP, 0x483BF1, 0x483BFE);// #895374: skip the code that removes the crates (size 7)

ASMJIT_PATCH(0x699C1C, Game_ParsePKTs_ClearFile, 0x7)
{
	LEA_STACK(CCINIClass*, pINI, 0x24);
	pINI->Clear(nullptr, nullptr);
	return 0;
}

// Guard command failure
ASMJIT_PATCH(0x730DB0, GuardCommandClass_Execute, 0xA)
{
	GET(TechnoClass*, T, ESI);
	return (T->Owner != HouseClass::CurrentPlayer() || !T->IsControllable())
		? 0x730E62
		: 0x730DBE
		;
}

#ifndef _old
ASMJIT_PATCH(0x551A30, LayerClass_YSortReorder, 0x5)
{
	GET(LayerClass*, pThis, ECX);

	auto const step = pThis->Count / 15;
	auto const slice = Unsorted::CurrentFrame() % 15;

	auto const begin = pThis->Items + step * slice;
	auto const end = (slice >= 14)
		? pThis->Items + pThis->Count
		: begin + step + step / 4;

	std::sort(begin, end, [](ObjectClass* const pA, ObjectClass* const pB) {
		return pA->GetYSort() < pB->GetYSort();
	});

	return 0x551A84;
}
#else

FORCEDINLINE void fast_ysort(ObjectClass** begin, ObjectClass** end)
{
	size_t n = (size_t)(end - begin);
	if (n <= 1) return;

	// tiny-range optimization: insertion sort for very small arrays
	if (n <= 32)
	{
		for (size_t i = 1; i < n; ++i)
		{
			ObjectClass* key = begin[i];
			int kval = key->GetYSort();
			size_t j = i;
			while (j > 0 && begin[j - 1]->GetYSort() > kval)
			{
				begin[j] = begin[j - 1];
				--j;
			}
			begin[j] = key;
		}
		return;
	}

	// Build key-pointer pairs to avoid virtual calls in comparisons
	// Also compute min/max while we are at it.
	std::vector<int> keys;
	keys.reserve(n);
	int minv = INT32_MAX, maxv = INT32_MIN;
	for (size_t i = 0; i < n; ++i)
	{
		int k = begin[i]->GetYSort();
		keys.push_back(k);
		if (k < minv) minv = k;
		if (k > maxv) maxv = k;
	}

	// If keys fit in a reasonably small range, do counting sort (stable).
	// Tunable: max_range_allowed controls memory usage and speed tradeoff.
	const int64_t max_range_allowed = 1 << 16; // 65536
	int64_t range = (int64_t)maxv - (int64_t)minv + 1;
	if (range > 0 && range <= max_range_allowed)
	{
		// counting sort
		std::vector<uint32_t> counts((size_t)range);
		for (size_t i = 0; i < n; ++i) counts[(size_t)(keys[i] - minv)]++;

		// prefix sums
		uint32_t sum = 0;
		for (size_t i = 0; i < counts.size(); ++i)
		{
			uint32_t c = counts[i];
			counts[i] = sum;
			sum += c;
		}

		// output buffer (stable)
		std::vector<ObjectClass*> out(n);
		for (size_t i = 0; i < n; ++i)
		{
			auto idx = (size_t)(keys[i] - minv);
			out[counts[idx]++] = begin[i];
		}

		// copy back
		std::memcpy(begin, out.data(), n * sizeof(ObjectClass*));
		return;
	}

	// Fallback: sort by (key, pointer) pair to avoid repeated GetYSort() calls
	// (This is typically faster than calling the virtual in comparator repeatedly)
	struct KP { int key; ObjectClass* ptr; };
	std::vector<KP> arr;
	arr.reserve(n);
	for (size_t i = 0; i < n; ++i) arr.push_back({ keys[i], begin[i] });

	std::sort(arr.begin(), arr.end(), [](const KP& a, const KP& b)
 {
	 return a.key < b.key; // stable-ness not strictly required but ok
	});

	// copy back
	for (size_t i = 0; i < n; ++i) begin[i] = arr[i].ptr;
}

// --- Fixed buffer std::sort wrapper ----------------------------------------
// Use preallocated index buffer to avoid allocations during sort when building comparators


struct YSortComparator
{
	inline bool operator()(ObjectClass* a, ObjectClass* b) const
	{
		return a->GetYSort() < b->GetYSort();
	}
};

// EXPECTED_MAX_ITEMS should be set to worst-case number of objects to sort.
// If you're certain the mod won't exceed a given number, set accordingly to save memory.
#define EXPECTED_MAX_ITEMS 50000

// --- Thread-local buffers & vectors ---------------------------------------
// Pre-reserved std::vector to avoid allocations during std::sort fallback.
thread_local ObjectClass* g_tmpRadixA[EXPECTED_MAX_ITEMS];
thread_local ObjectClass* g_tmpRadixB[EXPECTED_MAX_ITEMS];
thread_local uint32_t     g_keyBuffer[EXPECTED_MAX_ITEMS];
thread_local int          g_count[256];

template<typename GetKey>
static void RadixSort32(ObjectClass** items, int n, GetKey getKey)
{
	if (n <= 1) return;

	// Preload all keys – important for performance
	for (int i = 0; i < n; ++i)
		g_keyBuffer[i] = getKey(items[i]);

	ObjectClass** src = items;
	ObjectClass** dst = g_tmpRadixA;

	for (int pass = 0; pass < 4; ++pass)
	{
		int shift = pass * 8;

		// zero counts
		for (int i = 0; i < 256; ++i)
			g_count[i] = 0;

		// count
		for (int i = 0; i < n; ++i)
			g_count[(g_keyBuffer[i] >> shift) & 0xFF]++;

		// prefix sum
		int sum = 0;
		for (int i = 0; i < 256; ++i)
		{
			int c = g_count[i];
			g_count[i] = sum;
			sum += c;
		}

		// scatter
		for (int i = 0; i < n; ++i)
		{
			uint32_t key = g_keyBuffer[i];
			int bucket = (key >> shift) & 0xFF;
			dst[g_count[bucket]++] = src[i];
		}

		// swap buffers for next pass
		std::swap(src, dst);
	}

	// If final output is not items[], copy back
	if (src != items)
		std::memcpy(items, src, n * sizeof(ObjectClass*));
}

class FakeLayerClass : public LayerClass
{
public:

	void __short()
	{
		const int nCount = this->Count;

		// Early exit for trivial cases
		if (nCount <= 1) return;

		constexpr int NUM_SLICES = 15;
		const int currentFrame = Unsorted::CurrentFrame % NUM_SLICES;
		const int chunkSize = nCount / NUM_SLICES;

		// Calculate slice boundaries
		const int startIndex = chunkSize * currentFrame;
		ObjectClass** begin = &this->Items[startIndex];
		ObjectClass** end;

		if (currentFrame >= NUM_SLICES - 1)
		{
			end = &this->Items[nCount];
		}
		else
		{
			const int sliceSize = chunkSize + (chunkSize >> 2);  // chunk + chunk/4
			end = begin + sliceSize;
			// Clamp to valid range
			if (end > &this->Items[nCount])
			{
				end = &this->Items[nCount];
			}
		}

		const size_t rangeSize = end - begin;

		// Skip if nothing to sort
		if (rangeSize <= 1) return;

		// Cache structure for sort keys
		struct SortKey
		{
			ObjectClass* obj;
			int y;
		};

		// Use static thread_local for better performance in multithreaded scenarios
		thread_local static std::vector<SortKey> cache;
		cache.clear();
		cache.reserve(rangeSize);

		// Build cache with prefetching
		for (auto it = begin; it != end; ++it)
		{
			// Prefetch next iteration's data
			if (it + 1 < end)
			{
				_mm_prefetch((const char*)(it + 1), _MM_HINT_T0);
				// Also prefetch the object we'll be calling GetYSort on
				if (*(it + 1))
				{
					_mm_prefetch((const char*)(*(it + 1)), _MM_HINT_T0);
				}
			}

			// Null check (in case of invalid pointers)
			if (*it)
			{
				cache.push_back({ *it, (*it)->GetYSort() });
			}
		}

		// Sort by cached Y values
		std::sort(cache.begin(), cache.end(),
			[](const SortKey& a, const SortKey& b)
 {
	 return a.y < b.y;
			}
		);

		// Write back sorted pointers with prefetching
		auto outIt = begin;
		for (size_t i = 0; i < cache.size(); ++i)
		{
			// Prefetch next write location
			if (i + 1 < cache.size() && outIt + 1 < end)
			{
				_mm_prefetch((const char*)(outIt + 1), _MM_HINT_T0);
			}
			*outIt++ = cache[i].obj;
		}
	}

	bool __sortedadd(ObjectClass* object)
	{
		// Grow if needed
		if (this->Count >= this->Capacity)
		{
			if (!this->IsAllocated && this->Capacity != 0)
			{
				return false;
			}

			if (this->CapacityIncrement <= 0)
			{
				return false;
			}

			if (!this->set_capacity(this->Capacity + this->CapacityIncrement, nullptr))
			{
				return false;
			}
		}

		// Binary search for insertion index using GameLess
		int lo = 0;
		int hi = this->Count; // insertion position in [0..ActiveCount]


		// If comparator is broken (always false), fall back to linear find to preserve original
		bool anyLess = false;
		for (int i = 0; i < this->Count; ++i)
		{
			if (YSortComparator()(this->Items[i], object)) { anyLess = true; break; }
		}


		int index = this->Count; // default append
		if (!anyLess)
		{
			// comparator never returns true for existing < new; behaviour in original code
			// would have broken out at index=0 (because test was true), but you observed "always false".
			// To maintain compatibility, we'll fall back to original linear scan.
			for (index = 0; index < this->Count; ++index)
			{
				if (YSortComparator()(this->Items[index], object)) break;
			}
		}
		else
		{
			// binary search: find first position where not (existing < object)
			while (lo < hi)
			{
				int mid = (lo + hi) >> 1;
				if (YSortComparator()(this->Items[mid], object))
				{
					lo = mid + 1;
				}
				else
				{
					hi = mid;
				}
			}
			index = lo;
		}


		// shift elements up by one => Vector_Item[index+1] becomes new value
		int ac = this->Count;
		// memmove is safe for overlapping ranges
		if (index < ac)
		{
			std::memmove(&this->Items[index + 1], &this->Items[index], (ac - index) * sizeof(ObjectClass*));
		}

		this->Items[index] = object;
		++this->Count;
		return true;
	}

	bool __submit(ObjectClass* object, bool sort)
	{
		if (!object)
			Debug::FatalErrorAndExit("Trying To submit nullptr object to layer !\n");

		if (sort)
		{
			return this->__sortedadd(object);
		}

		return this->push_back(object);
	}
};

//DEFINE_FUNCTION_JUMP(VTABLE, 0x7E607C, FakeLayerClass::__submit);
//DEFINE_FUNCTION_JUMP(LJMP, 0x551A90, FakeLayerClass::__sortedadd);
//DEFINE_FUNCTION_JUMP(LJMP, 0x5519B0, FakeLayerClass::__submit);
//DEFINE_FUNCTION_JUMP(CALL, 0x55BABB, FakeLayerClass::__submit);
//DEFINE_FUNCTION_JUMP(CALL, 0x4A9759, FakeLayerClass::__submit);
DEFINE_FUNCTION_JUMP(CALL, 0x55DBC8, FakeLayerClass::__short);
DEFINE_FUNCTION_JUMP(LJMP, 0x551A30, FakeLayerClass::__short);

//ASMJIT_PATCH(0x551A30, LayerClass_YSortReorder, 0x5)
//{
//    GET(LayerClass*, pThis, ECX);
//
//    auto const nCount = pThis->Count;
//	if (nCount <= 1) return 0;
//		return 0x551A84;
//    auto nBegin = &pThis->Items[nCount / 15 * (Unsorted::CurrentFrame % 15)];
//    auto nEnd = (Unsorted::CurrentFrame % 15 >= 14)
//        ? (&pThis->Items[nCount])
//        : (&nBegin[nCount / 15 + nCount / 15 / 4]);
//
//	fast_ysort((ObjectClass**)nBegin, (ObjectClass**)nEnd);
//	// std::sort(nBegin, nEnd, [](const ObjectClass* A, const ObjectClass* B) {
//	// 	return A->GetYSort() < B->GetYSort();
//	// });
//    return 0x551A84;
//}
#endif

//speeds up preview drawing by insane amounts
ASMJIT_PATCH(0x5FED00, OverlayTypeClass_GetRadarColor, 0x6)
{
	GET(OverlayTypeClass*, ovType, ECX);
	GET_STACK(ColorStruct*, color, 0x04);
	*color = ovType->RadarColor;
	R->EAX<ColorStruct*>(color);
	return 0x5FEDDA;
}

//Handle_Static_Messages_LoopingMovie
DEFINE_JUMP(LJMP, 0x615BD3, 0x615BE0);
//sub_789960_RemoveWOLResolutionCheck
DEFINE_JUMP(LJMP, 0x78997B, 0x789A58);
//DSurface_CTOR_SkipVRAM
DEFINE_JUMP(LJMP, 0x4BA61B, 0x4BA623);

ASMJIT_PATCH(0x545904, IsometricTileTypeClass_CreateFromINIList_MediansFix, 0x7)
{
	if (R->EAX() == -1)
	{
		// all theaters except snow have this set, so I'll assume that this was tripped by snow.
		// don't like it? put the damned tag in the INI.
		R->EAX(71);
	}
	return 0;
}

// skip the entire method, we handle it ourselves
// PsyDom_Update
DEFINE_JUMP(LJMP, 0x53AF40, 0x53B060);

ASMJIT_PATCH(0x65EA43, SendReinforcement_Opentopped, 0x6)
{
	GET(AircraftClass*, pPlane, ESI);
	GET(FootClass*, pPassenger, EDI);

	if (pPlane->Type->OpenTopped)
		pPlane->EnteredOpenTopped(pPassenger);

	pPassenger->Transporter = pPlane;

	return 0x0;
}

// PrismSupportModifier repair
//ASMJIT_PATCH(0x671152, RulesClass_Addition_General_PrismSupportModifier, 0x6)
//{
//	GET(RulesClass*, pThis, ESI);
//	REF_STACK(double, param, 0x0);
//	param = pThis->PrismSupportModifier / 100.0;
//	return 0x67115B;
//}

ASMJIT_PATCH(0x6B72F9, SpawnManagerClass_Update_Buildings, 0x5)
{
	GET(SpawnManagerClass*, pThis, ESI);
	GET(SpawnNode*, nNode, EAX);

	auto const pOwner = pThis->Owner;
	return (nNode->Status != SpawnNodeStatus::TakeOff
		|| !pOwner
		|| pOwner->WhatAmI() == BuildingClass::AbsID)
		? 0x6B735C : 0x6B72FE;
}

ASMJIT_PATCH(0x725A1F, AnnounceInvalidPointer_SkipBehind, 0x5)
{
	GET(AnimClass*, pAnim, ESI);
	return pAnim->Type == RulesClass::Instance->Behind ?
		0x725C08 : 0x0;
}

//sub_731D90_FakeOf
ASMJIT_PATCH(0x731E08, Select_By_Units_Text_FakeOf, 0x6)
{
	int nCost = 0;

	for (const auto pObj : ObjectClass::CurrentObjects())
	{
		if (const auto pTechno = flag_cast_to<TechnoClass*>(pObj))
		{
			const auto pTypeExt = GET_TECHNOTYPEEXT(pTechno);

			TechnoTypeClass* pType = pTypeExt->This();
			if (pTypeExt->Fake_Of)
				pType = pTypeExt->Fake_Of;

			nCost += pType->GetActualCost(pTechno->Owner);
		}
	}

	R->EBX(nCost);
	return 0x731E4D;
}

ASMJIT_PATCH(0x6DA665, sub_6DA5C0_GroupAs, 0xA)
{
	GET(ObjectClass*, pThis, ESI);
	R->EAX(TechnoTypeExtData::GetSelectionGroupID(pThis->GetType()));
	return R->Origin() + 13;
}

ASMJIT_PATCH(0x7BB445, XSurface_20, 0x6)
{
	return R->EAX<void*>() ? 0x0 : 0x7BB90C;
}

ASMJIT_PATCH(0x716D98, TechnoTypeClass_Load_Palette, 0x5)
{
	GET(TechnoTypeClass*, pThis, EDI);

	pThis->Palette = nullptr;
	return pThis->PaletteFile[0] == 0 ? 0x716DAA : 0x716D9D;
}

#include <Ext/Cell/Body.h>

// this was only a leftover stub from TS. reimplemented
// using the same mechanism.
void __fastcall FakeCellClass::_ChainReaction(CellStruct* cell)
{
	const auto pCell = (FakeCellClass*)MapClass::Instance->GetCellAt(cell);
	TiberiumClass* pTib = TiberiumClass::Array->get_or_default(pCell->_GetTiberiumType());

	if (!pTib)
		return;

	OverlayTypeClass* pOverlay = OverlayTypeClass::Array->get_or_default(pCell->OverlayTypeIndex);

	if (!pOverlay || !pOverlay->ChainReaction || pCell->OverlayData <= 1u)
		return;

	if (ScenarioClass::Instance->Random.RandomFromMax(99) <
		(FakeRulesClass::Instance()->ChainReact_Multiplier * pCell->OverlayData))
	{
		const bool wasFullGrown = (pCell->OverlayData >= 11);

		unsigned char delta = pCell->OverlayData / 2;
		int damage = pTib->Power * delta;

		// remove some of the tiberium
		pCell->OverlayData -= delta;
		pCell->MarkForRedraw();

		// get the warhead
		auto pExt = TiberiumExtContainer::Instance.Find(pTib);
		CoordStruct crd = pCell->GetCoords();

		if (auto pWarhead = pExt->GetExplosionWarhead())
		{
			// create an explosion
			if (auto pType = MapClass::SelectDamageAnimation(4 * damage, pWarhead, pCell->LandType, crd))
			{
				GameCreate<AnimClass>(pType, crd, 0, 1, 0x600, 0);
			}

			// damage the area, without affecting tiberium
			DamageArea::Apply(&crd, damage, nullptr, pWarhead, false, nullptr);
		}

		// spawn some animation on the neighbour cells
		if (auto pType = AnimTypeClass::Find(GameStrings::Anim_INVISO()))
		{
			for (int i = 0; i < 8; ++i)
			{
				auto pNeighbour = (FakeCellClass*)pCell->GetNeighbourCell((FacingType)i);

				if (pNeighbour->_GetTiberiumType() != -1 && pNeighbour->OverlayData > 2u)
				{
					if (ScenarioClass::Instance->Random.RandomFromMax(99) < FakeRulesClass::Instance()->ChainReact_SpreadChance)
					{
						int delay = ScenarioClass::Instance->Random.RandomRanged(FakeRulesClass::Instance()->ChainReact_MinDelay, FakeRulesClass::Instance()->ChainReact_MaxDelay);
						crd = pNeighbour->GetCoords();

						GameCreate<AnimClass>(pType, crd, delay, 1, 0x600, 0);
					}
				}
			}
		}

		if (wasFullGrown)
		{
			pTib->RegisterForGrowth(cell);
		}
	}

	return;
}

//DEFINE_SKIP_HOOK(0x715857, TechnoTypeClass_LoadFromINI_LimitPalettes, 5, 715876)
DEFINE_JUMP(LJMP, 0x715857, 0x715876);

//WinMain_LogGameClasses
DEFINE_JUMP(LJMP, 0x6BB9DD, 0x6BBE2B);

//TechnoClass_DealParticleDamage_DontDestroyCliff
DEFINE_JUMP(LJMP, 0x70CAD8, 0x70CB30);

// bugfix #187: Westwood idiocy
// Game_BulkDataInit_MultipleDataInitFix1
DEFINE_JUMP(LJMP, 0x531726, 0x53173A);

// bugfix #187: Westwood idiocy
//Game_BulkDataInit_MultipleDataInitFix2
DEFINE_JUMP(LJMP, 0x53173F, 0x531749);

//this hook taking a lot of time , i guess because of UnitTypeClass::InitOneTimeData thing
// ASMJIT_PATCH(0x531726, Game_BulkDataInit_MultipleDataInitFix, 5)
// {
// 	BuildingTypeClass::InitOneTimeData();
// 	UnitTypeClass::InitOneTimeData();
// 	return 0x531749;
// }

DEFINE_PATCH(0x535DB9, 0x01);

//ASMJIT_PATCH(0x535DB6, SetStructureTabCommandClass_Execute_Power, 6)
//{
//	GET(BuildingClass*, pBuild, EAX);
//	R->EAX(pBuild->FindFactory(false, true));
//	return 0x535DC2;
//}

DEFINE_PATCH(0x535E79, 0x01);

//ASMJIT_PATCH(0x535E76, SetDefenseTabCommandClass_Execute_Power, 6)
//{
//	GET(BuildingClass*, pBuild, EAX);
//	R->EAX(pBuild->FindFactory(false, true));
//	return 0x535E82;
//}

ASMJIT_PATCH(0x4B93BD, ScenarioClass_GenerateDropshipLoadout_FreeAnims, 7)
{
	GET_STACK(SHPStruct*, pBackground, 0xAC);

	if (pBackground)
	{
		GameDelete<true, false>(std::exchange(pBackground, nullptr));
	}

	LEA_STACK(SHPStruct**, pSwipeAnims, 0x290);

	for (auto i = 0; i < 4; ++i)
	{
		if (auto pAnim = pSwipeAnims[i])
		{
			GameDelete<true, false>(std::exchange(pAnim, nullptr));
		}
	}

	return 0x4B9445;
}

// fix for ultra-fast processors overrunning the performance evaluator function
ASMJIT_PATCH(0x5CB0B1, Game_QueryPerformance, 5)
{
	if (!R->EAX())
	{
		R->EAX(1);
	}

	return 0;
}

ASMJIT_PATCH(0x657D3D, MapClass_MinimapChanged_Lock, 6)
{
	RadarClass::RadarEvenSurface->Lock();
	RadarClass::RadarEvenSurface_B->Lock();
	return 0;
}ASMJIT_PATCH_AGAIN(0x657CF2, MapClass_MinimapChanged_Lock, 6)


ASMJIT_PATCH(0x657D8A, MapClass_MinimapChanged_Unlock, 7)
{
	RadarClass::RadarEvenSurface->Unlock();
	RadarClass::RadarEvenSurface_B->Unlock();
	return 0;
}ASMJIT_PATCH_AGAIN(0x657D35, MapClass_MinimapChanged_Unlock, 7)

ASMJIT_PATCH(0x65731F, RadarClass_UpdateMinimap_Lock, 6)
{
	GET(RadarClass*, pRadar, ESI);
	pRadar->unknown_121C->Lock();
	pRadar->unknown_1220->Lock();
	return 0;
}

ASMJIT_PATCH(0x65757C, RadarClass_UpdateMinimap_Unlock, 8)
{
	GET(RadarClass*, pRadar, ESI);
	pRadar->unknown_1220->Unlock();
	pRadar->unknown_121C->Unlock();

	return R->EAX() ? 0x657584 : 0x6576A5;
}

ASMJIT_PATCH(0x4B769B, ScenarioClass_GenerateDropshipLoadout, 5)
{
	WWKeyboardClass::Instance->Clear();
	WWMouseClass::Instance->ShowCursor();
	return 0x4B76A0;
}

#include <Ext/Scenario/Body.h>

// issue #279: per unit AirstrikeAttackVoice and AirstrikeAbortSound
ASMJIT_PATCH(0x41D940, AirstrikeClass_Fire_AirstrikeAttackVoice, 5)
{
	GET(AirstrikeClass*, pAirstrike, EDI);
	GET(TechnoClass*, pTarget, ESI);

	// get default from rules
	int index = RulesClass::Instance->AirstrikeAttackVoice;

	// get from aircraft
	const auto pAircraftExt = GET_TECHNOTYPEEXT(pAirstrike->FirstObject);
	if (pAircraftExt->VoiceAirstrikeAttack.isset())
		index = pAircraftExt->VoiceAirstrikeAttack.Fetch();

	// get from designator
	if (const auto pOwner = pAirstrike->Owner)
	{
		auto pOwnerExt = GET_TECHNOTYPEEXT(pOwner);

		if (pOwnerExt->VoiceAirstrikeAttack.isset())
			index = pOwnerExt->VoiceAirstrikeAttack.Fetch();
	}

	VocClass::SafeImmedietelyPlayAt(index, &pAirstrike->FirstObject->Location, nullptr);
	pAirstrike->Target = pTarget;

	if (pTarget)
	{
		const auto pTargetExt = TechnoExtContainer::Instance.Find(pTarget);
		pTargetExt->AirstrikeTargetingMe = pAirstrike;
		pTarget->StartAirstrikeTimer(100000);

		if (auto pBld = cast_to<BuildingClass*, false>(pTarget))
		{
			pBld->IsAirstrikeTargetingMe = true;
			pBld->Mark(MarkType::Redraw);
		}
	}

	//return 0x41D970;
	return 0x41DA0B;
}

ASMJIT_PATCH(0x41D5AE, AirstrikeClass_PointerGotInvalid_AirstrikeAbortSound, 9)
{
	GET(AirstrikeClass*, pAirstrike, ESI);

	// get default from rules
	int index = RulesClass::Instance->AirstrikeAbortSound;

	// get from aircraft
	const auto pAircraftExt = GET_TECHNOTYPEEXT(pAirstrike->FirstObject);
	if (pAircraftExt->VoiceAirstrikeAbort.isset())
		index = pAircraftExt->VoiceAirstrikeAbort.Fetch();

	// get from designator
	if (const auto pOwner = pAirstrike->Owner)
	{
		auto pOwnerExt = GET_TECHNOTYPEEXT(pOwner);
		if (pOwnerExt->VoiceAirstrikeAbort.isset())
			index = pOwnerExt->VoiceAirstrikeAbort.Fetch();
	}

	VocClass::SafeImmedietelyPlayAt(index, &pAirstrike->FirstObject->Location, nullptr);
	return 0x41D5E0;
}

ASMJIT_PATCH(0x6BED08, Game_Terminate_Mouse, 7)
{
	GameDelete<true, false>(R->ECX<SHPStruct*>());
	return 0x6BED34;
}

ASMJIT_PATCH(0x621B80, DSurface_FillRecWithColor, 5)
{
	GET(RectangleStruct*, rect, ECX);
	GET(Surface*, surface, EDX);

	int surfaceWidth = surface->Get_Width();
	int surfaceHeight = surface->Get_Height();

	//make sure the rectangle to fill is within the surface's boundaries, this should do the trick
	rect->X = (rect->X >= 0) ? rect->X : 0;
	rect->Y = (rect->Y >= 0) ? rect->Y : 0;
	rect->Width = (rect->X + rect->Width <= surfaceWidth) ? rect->Width : surfaceWidth - rect->X;
	rect->Height = (rect->Y + rect->Height <= surfaceHeight) ? rect->Height : surfaceHeight - rect->Y;

	if (rect->Width == 0 || rect->Height == 0)
		return 0x621D26;
	else
		return 0;
}

ASMJIT_PATCH(0x4ABFBE, DisplayClass_LeftMouseButtonUp_ExecPowerToggle, 7)
{
	GET(TechnoClass*, Target, ESI);
	return (Target && Target->Owner->IsControlledByHuman() && Target->WhatAmI() == AbstractType::Building)
		? 0x4ABFCE
		: 0x4AC294
		;
}

//InitGame_Delay
DEFINE_JUMP(LJMP, 0x52CA37, 0x52CA65)

#include <CD.h>

static COMPILETIMEEVAL reference<MixFileClass*, 0x884E04> const Language {};
static COMPILETIMEEVAL reference<MixFileClass*, 0x884E00> const LangMD {};
static COMPILETIMEEVAL constant_ptr<const char, 0x840D5C> const LANGMD_MIX {};
static COMPILETIMEEVAL constant_ptr<const char, 0x840D4C> const LANGUAGE_MIX {};

static COMPILETIMEEVAL reference<MixFileClass*, 0x884DF8> const RA2MD {};
static COMPILETIMEEVAL reference<MixFileClass*, 0x884DFC> const RA2 {};
static COMPILETIMEEVAL constant_ptr<const char, 0x82667C> const RA2MD_MIX {};
static COMPILETIMEEVAL constant_ptr<const char, 0x826674> const RA2_MIX {};

static COMPILETIMEEVAL reference<MixFileClass*, 0x884E48> const CACHEMD {};
static COMPILETIMEEVAL reference<MixFileClass*, 0x884E4C> const CACHE {};
static COMPILETIMEEVAL constant_ptr<const char, 0x82665C> const CACHEMD_MIX {};
static COMPILETIMEEVAL constant_ptr<const char, 0x826650> const CACHE_MIX {};

static COMPILETIMEEVAL reference<MixFileClass*, 0x884E50> const LOCALMD {};
static COMPILETIMEEVAL reference<MixFileClass*, 0x884E54> const LOCAL {};
static COMPILETIMEEVAL constant_ptr<const char, 0x826644> const LOCALMD_MIX {};
static COMPILETIMEEVAL constant_ptr<const char, 0x826638> const LOCAL_MIX {};

static COMPILETIMEEVAL reference<MixFileClass*, 0x884E38> const CONQMD {};
static COMPILETIMEEVAL constant_ptr<const char, 0x826838> const CONQMD_MIX {};

bool NOINLINE  __fastcall MixFilesBoostrap()
{
	int disk = CD::Disk();
	CD::SetReqCD(-2);

	auto pKey = MixFileClass::Key();
	auto AllocateMix = [pKey](const char* mixName, MixFileClass*& mix) {
		mix = GameCreate<MixFileClass>(mixName, pKey);
		Debug::LogInfo("Loading {}", mixName);
		return mix;
	};

	if (SpawnerMain::Configs::Enabled) {
		for (auto& preloadMix : SpawnerMain::GetGameConfigs()->PreloadMixes) {
			SpawnerMain::LoadedMixFiles.push_back(GameCreate<MixFileClass>(preloadMix.c_str(), pKey));
			Debug::LogInfo("Loading Preloaded Mix Name : {} ", preloadMix.c_str());
		}
	}

	for (int i = 99; i >= 0; --i) {
		char buffer[256];
		_snprintf(buffer, sizeof(buffer) - 1, GameStrings::EXPANDMD02d(), i);
		RawFileClass _raw(buffer);
		if (_raw.IsAvaible()) {
			auto pFileName = _raw.FileName();

			Debug::LogInfo("Loading [{} - {}]", buffer, pFileName);
			auto mix = GameCreate<MixFileClass>(buffer, pKey);

			if (!mix->IsValid()) {
				Debug::LogInfo("Failed Loading [{} - {}]", buffer, pFileName);
				continue;
			}

			MixFileClass::Array->push_back(mix);
		}
	}

	if (!AllocateMix(RA2MD_MIX(), RA2MD()))
		return false;

	if (!AllocateMix(RA2_MIX(), RA2()))
		return false;

	if (!AllocateMix(CACHEMD_MIX(), CACHEMD()))
		return false;

	if (!AllocateMix(CACHE_MIX(), CACHE()))
		return false;

	if (!AllocateMix(LOCALMD_MIX(), LOCALMD()))
		return false;

	if (!AllocateMix(LOCAL_MIX(), LOCAL()))
		return false;

	if (SpawnerMain::Configs::Enabled) {
		for (auto& postloadMix : SpawnerMain::GetGameConfigs()->PostloadMixes) {
			SpawnerMain::LoadedMixFiles.push_back(GameCreate<MixFileClass>(postloadMix.c_str(), pKey));
			Debug::LogInfo("Loading Postload Mix Name : {} ", postloadMix.c_str());
		}
	}

	CD::SetReqCD(disk);
	return true;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x5301A0, MixFilesBoostrap);

#include <Misc/CSF.h>

void __cdecl Prog_End() {
	JMP_STD(0x6BE1C0);
}

int __cdecl atexitCall(void (__cdecl *a1)()) {
	JMP_STD(0x7C978A);
}

void __fastcall Start_Mouse_Thread(){
	JMP_FAST(0x7B84F0);
}

int SystemMessageDialog(HWND hwnd, const WCHAR* message, LPCWSTR caption, UINT uType){
	const std::string msg = PhobosCRT::WideStringToString(message);
	const std::string cap = PhobosCRT::WideStringToString(caption);
	return MessageBoxA(hwnd, msg.c_str(), cap.c_str(), uType);
}

#include <shlwapi.h>  // DLLVERSIONINFO, DLLGETVERSIONPROC

int Get_DLL_Version(LPCSTR lpLibFileName){
	int result = 0;

	for (auto& dlls : Patch::ModuleDatas){
		if (IS_SAME_STR_(dlls.ModuleName.c_str(), lpLibFileName)){
			if (auto dllver = reinterpret_cast<DLLGETVERSIONPROC>(GetProcAddress(dlls.Handle, "DllGetVersion"))) {
				DLLVERSIONINFO info {};
				info.cbSize = sizeof(DLLVERSIONINFO);

				if (dllver(&info) >= 0)
					result = LOWORD(info.dwMinorVersion) | (LOWORD(info.dwMajorVersion) << 16);
			}

			break;
		}
	}

	return result;
}

#include <Phobos.Lua.h>

static COMPILETIMEEVAL constant_ptr<const char, 0x840D40> const ra2md_str {};
//WinMain_
DEFINE_JUMP(LJMP, 0x6BD7D5, 0x6BD83C);

ASMJIT_PATCH(0x6BD7C5, WinMain_Expand_MIX_Reorg, 5)
{

	SpawnerMain::GameConfigs::Init();

	auto pKey = MixFileClass::Key();

	PhobosGlobal::Instance()->aresMIX.reset(GameCreate<MixFileClass>("ares.mix", pKey));
	if (SpawnerMain::Configs::Enabled) {
		SpawnerMain::LoadedMixFiles.push_back(GameCreate<MixFileClass>("cncnet.mix", pKey));
	}

	MixFilesBoostrap();
	
	int disk = CD::Disk();

	LangMD = GameCreate<MixFileClass>(LANGMD_MIX(), pKey);
	Language = GameCreate<MixFileClass>(LANGUAGE_MIX(), pKey);

	CD::SetReqCD(disk);

	//atexitCall(Prog_End);
	// do not all this inside dll thread
	//Start_Mouse_Thread();

	if (!CSFLoader::PhobosInit(ra2md_str())) {
		const std::string _msg = fmt::format(
			"Unable to initialize '{0}', please reinstall {1}.\n"
			"Keine Initialisierung von '{0}' möglich. Bitte installieren Sie {1} erneut.\n"
			"Initialisation de '{0}' impossible. Veuillez réinstaller {1}.",
			ra2md_str(), LuaData::MainWindowStr);

		MessageBoxA(NULL, _msg.c_str(), LuaData::MainWindowStr.c_str(), 0x10u);
		return 0x6BD86F;
	}

	fmt::basic_memory_buffer<char, 60> buffer {};

	CSFLoader::LoadAdditionalCSF("ares.csf", true);

	buffer.clear();
	std::string res = "us";
	if (const auto* language = StringTable::GetLanguage(StringTable::Language()))
		res = language->Letter;

	fmt::format_to(std::back_inserter(buffer), "ares_{}.csf", res);
	buffer.push_back('\0');
	CSFLoader::LoadAdditionalCSF(buffer.data());
	buffer.clear();

	for (int idx = 0; idx < 100; ++idx) {
		fmt::format_to(std::back_inserter(buffer), fmt::runtime(LuaData::AdditionalStringTableFmt) , idx);
		buffer.push_back('\0');
		CSFLoader::LoadAdditionalCSF(buffer.data());
		buffer.clear();
	}

	// skip error "А mouse is required for playing Yurts Revenge" - remove the GetSystemMetrics check
	//DEFINE_JUMP(LJMP, 0x6BD8A4, 0x6BD8C2); // WinMain
	//if (!GetSystemMetrics(SM_MOUSEPRESENT) || !GetSystemMetrics(SM_CMOUSEBUTTONS)) {
	//	auto v208 = CSFLoader::FetchStringManager("TXT_SHORT_TITLE", nullptr, nullptr, -1);
	//	auto v202 = CSFLoader::FetchStringManager("TXT_MOUSE_REQUIRED", nullptr, nullptr, -1);
	//	SystemMessageDialog(0, v202, v208, 0x10u);
	//	Phobos::ExeTerminate();
	//	CRT::exit_returnsomething(1, 0, 0);
	//	Debug::DetachLogger();
	//}

	const auto comctlVer = Get_DLL_Version("comctl32.dll");
	if (comctlVer < 0x40046) {
		wchar_t a2[512];
		auto v185 = CSFLoader::FetchStringManager("TXT_DLL_INVALID", nullptr, nullptr, -1);
		_swprintf(a2, v185, "comctl32.dll", 4, 70, "comctl32.dll");
		auto v186 = CSFLoader::FetchStringManager("TXT_SHORT_TITLE", nullptr, nullptr, -1);
		SystemMessageDialog(0, a2, v186, 0x10u);
		Phobos::ExeTerminate();
		CRT::exit_returnsomething(1, 0, 0);
		Debug::DetachLogger();
	} else {
		Debug::LogInfo("comctl dll version {} detected.", (unsigned)comctlVer);
	}

	return 0x6BD934;
}

DEFINE_JUMP(LJMP, 0x52BB64, 0x52BB95) //Expand_MIX_Deorg

static COMPILETIMEEVAL reference<MixFileClass*, 0x884E58> const NTRLMD {};
static COMPILETIMEEVAL constant_ptr<const char, 0x827DA0> const NTRLMD_MIX {};

static COMPILETIMEEVAL reference<MixFileClass*, 0x884E5C> const NEUTRAL {};
static COMPILETIMEEVAL constant_ptr<const char, 0x827D80> const NEUTRAL_MIX {};

void NOINLINE __fastcall Release_Neutral() {

	if (NEUTRAL())
	{
		Debug::LogInfo("Releasing {} ", NEUTRAL_MIX());
		GameDelete<true, false>(NEUTRAL());
		NEUTRAL = nullptr;
	}

	if (NTRLMD())
	{
		Debug::LogInfo("Releasing {} ", NTRLMD_MIX());
		GameDelete<true, false>(NTRLMD());
		NTRLMD = nullptr;
	}
}

bool NOINLINE __fastcall Prep_Neutral()
{
	Release_Neutral();

	if (!Phobos::Otamaa::ExeTerminated)
	{
		Debug::LogInfo("Loading {} ", NTRLMD_MIX());
		NTRLMD = GameCreate<MixFileClass>(NTRLMD_MIX(), MixFileClass::Key());

		if (!NTRLMD())
			return false;

		Debug::LogInfo("Loading {} ", NEUTRAL_MIX());
		NEUTRAL = GameCreate<MixFileClass>(NEUTRAL_MIX(), MixFileClass::Key());

		if (!NEUTRAL())
			return false;
	}

	return true;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x534E50, Prep_Neutral);
DEFINE_FUNCTION_JUMP(CALL, 0x72AB0A, Prep_Neutral);
DEFINE_FUNCTION_JUMP(CALL, 0x72DDBD, Prep_Neutral);
DEFINE_FUNCTION_JUMP(CALL, 0x72E071, Prep_Neutral);
DEFINE_FUNCTION_JUMP(CALL, 0x72E462, Prep_Neutral);

DEFINE_FUNCTION_JUMP(LJMP, 0x534DE0, Release_Neutral);
DEFINE_FUNCTION_JUMP(CALL, 0x72ACFA, Release_Neutral);
DEFINE_FUNCTION_JUMP(CALL, 0x72DFA0, Release_Neutral);
DEFINE_FUNCTION_JUMP(CALL, 0x72E060, Release_Neutral);

#include <ThemeClass.h>

static COMPILETIMEEVAL reference<MixFileClass*, 0x884E18> const GENERMD {};
static COMPILETIMEEVAL constant_ptr<const char, 0x826820> const GENERMD_MIX {};

static COMPILETIMEEVAL reference<MixFileClass*, 0x884E14> const GENERIC {};
static COMPILETIMEEVAL constant_ptr<const char, 0x826814> const GENERIC_MIX {};

static COMPILETIMEEVAL reference<MixFileClass*, 0x884E24> const ISOGEN {};
static COMPILETIMEEVAL constant_ptr<const char, 0x8267F8> const ISOGEN_MIX {};

static COMPILETIMEEVAL reference<MixFileClass*, 0x884E28> const ISOGENMD {};
static COMPILETIMEEVAL constant_ptr<const char, 0x826804> const ISOGENMD_MIX {};

static COMPILETIMEEVAL reference<MixFileClass*, 0x884E3C> const CONQUER {};
static COMPILETIMEEVAL constant_ptr<const char, 0x8267EC> const CONQUER_MIX {};

static COMPILETIMEEVAL reference<MixFileClass*, 0x884E40> const CAMEOMD {};
static COMPILETIMEEVAL constant_ptr<const char, 0x8267D0> const CAMEOMD_MIX {};

static COMPILETIMEEVAL reference<MixFileClass*, 0x884E44> const CAMEO {};
static COMPILETIMEEVAL constant_ptr<const char, 0x8267B4> const CAMEO_MIX {};

static COMPILETIMEEVAL constant_ptr<const char, 0x826790> const MAPS__MIX {};
static COMPILETIMEEVAL reference<MixFileClass*, 0x884E60> const MAPS {};

static COMPILETIMEEVAL constant_ptr<const char, 0x82679C> const MAPSMD__MIX {};
static COMPILETIMEEVAL reference<MixFileClass*, 0x884E64> const MAPSMD {};

static COMPILETIMEEVAL constant_ptr<const char, 0x81C2EC> const MAPSMD___MIX {};
static COMPILETIMEEVAL constant_ptr<const char, 0x81C2C4> const MAPS___MIX {};

static COMPILETIMEEVAL reference<MixFileClass*, 0x884DD8> const MULTIMD {};
static COMPILETIMEEVAL constant_ptr<const char, 0x826780> const MULTIMD_MIX {};

static COMPILETIMEEVAL constant_ptr<const char, 0x81C24C> const THEMEMD_MIX {};
static COMPILETIMEEVAL constant_ptr<const char, 0x81C220> const THEME_MIX {};
static COMPILETIMEEVAL reference<MixFileClass*, 0x87E738> const THEME {};

// ---------------------------------------------------------------------------
// Init_Secondary_Mixfiles
// ---------------------------------------------------------------------------
bool __fastcall Init_Secondary_Mixfiles()
{
	auto pKey = MixFileClass::Key();
	Debug::LogInfo("");

	auto LoadMixWildcard = [pKey](const char* pattern, MixFileClass*& primary, DynamicVectorClass<MixFileClass*>& vec)
		{
			std::string v73 = pattern;

			if (!Game::File_Finder_Start(v73.c_str()))
				return false;

			Debug::LogInfo("Loading {}", v73.c_str());

			primary = GameCreate<MixFileClass>(v73.c_str(), pKey);

			while (Game::File_Finder_Next_Name(v73.c_str()))
			{
				Debug::LogInfo("Loading {}", v73.c_str());
				vec.emplace_back(GameCreate<MixFileClass>(v73.c_str(), pKey));
			}

			Game::File_Finder_End();
			return true;
		};

	auto CheckAndAllocateMix = [pKey](const char* mixName, MixFileClass*& mix) {
		CCFileClass tmp(mixName);

		if (!tmp.IsAvaible(0))
			return false;

		if (mix) {
			GameDelete<true, false>(mix);
			mix = nullptr;
		}

		{
			mix = GameCreate<MixFileClass>(mixName, pKey);
			const auto pFileName = tmp.FileName();

			Debug::LogInfo("Loading [{} - {}]", mixName , pFileName);
			
			if (!mix->IsValid()) {
				Debug::LogInfo("Failed Loading [{} - {}]", mixName , pFileName);
				return false;
			}
		}

		return true;
	};
	
	// ------------------------------------------------------------------
	// CONQMD.MIX — required; bail if missing
	// ------------------------------------------------------------------
	if (!CheckAndAllocateMix(CONQMD_MIX(), CONQMD()))
		return false;

	// ------------------------------------------------------------------
	// GENERMD.MIX / GENERIC.MIX / ISOGENMD.MIX / ISOGEN.MIX
	// ------------------------------------------------------------------
	CheckAndAllocateMix(GENERMD_MIX(), GENERMD());
	CheckAndAllocateMix(GENERIC_MIX(), GENERIC());
	CheckAndAllocateMix(ISOGENMD_MIX(), ISOGENMD());
	CheckAndAllocateMix(ISOGEN_MIX(), ISOGEN());

	// ------------------------------------------------------------------
	// CONQUER.MIX — required; bail if missing
	// ------------------------------------------------------------------
	if (!CheckAndAllocateMix(CONQUER_MIX(), CONQUER()))
		return false;

	// ------------------------------------------------------------------
	// CAMEOMD.MIX — required; bail if missing
	// ------------------------------------------------------------------
	if (!CheckAndAllocateMix(CAMEOMD_MIX(), CAMEOMD()))
		return false;

	// ------------------------------------------------------------------
	// CAMEO.MIX — required; bail if missing
	// ------------------------------------------------------------------
	if (!CheckAndAllocateMix(CAMEO_MIX(), CAMEO()))
		return false;

	// ------------------------------------------------------------------
	// MAPS*.MIX / MAPSMD*.MIX
	// ------------------------------------------------------------------
	const int cdIndex = Game::Get_Volume_Index(60) + 1;

	if (CD::IsLocal()) {
		if (!LoadMixWildcard(MAPSMD__MIX(), MAPSMD(), MixFileClass::Maps()))
			LoadMixWildcard(MAPS__MIX(), MAPS(), MixFileClass::Maps());
	} else {
		char v73[260];
		sprintf_s(v73, MAPSMD___MIX(), cdIndex);
		CheckAndAllocateMix(v73, MAPSMD());
		sprintf_s(v73, MAPS___MIX(), cdIndex);
		CheckAndAllocateMix(v73, MAPS());
	}

	if (!MAPSMD())
		return false;

	// ------------------------------------------------------------------
	// MULTIMD.MIX — required; bail if missing
	// ------------------------------------------------------------------
	if(!CheckAndAllocateMix(MULTIMD_MIX, MULTIMD()))
		return false;

	// ------------------------------------------------------------------
	// THEMEMD.MIX / THEME.MIX (optional — missing ThemeMix is non-fatal)
	// ------------------------------------------------------------------
	if (!CheckAndAllocateMix(THEMEMD_MIX, THEME()) && !CheckAndAllocateMix(THEME_MIX, THEME())) {
		Debug::Log("Successfully finding Theme mix file\n");
		ThemeClass::ScoresPresent = 1;
		ThemeClass::Instance->Scan();
	}

	// ------------------------------------------------------------------
	// MOVMD*.MIX / MOVIES*.MIX
	// ------------------------------------------------------------------
	bool MoviesFound = false;
	bool IsLocal = false;
	char v73[260];

	{

		static COMPILETIMEEVAL reference<MixFileClass*, 0x884E2C> const MoviesMix {};
		static COMPILETIMEEVAL constant_ptr<const char, 0x826738> const MOVIES01_MIX {};
		static COMPILETIMEEVAL constant_ptr<const char, 0x826714> const MIXFILES_MOVIES01_MIX {};
		static COMPILETIMEEVAL constant_ptr<const char, 0x8266F0> const MOVMD01_MIX {};
		static COMPILETIMEEVAL constant_ptr<const char, 0x8266D8> const MIXFILES_MOVMD01_MIX {};
		static COMPILETIMEEVAL constant_ptr<const char, 0x8266B8> const MOVMD03_MIX {};
		static COMPILETIMEEVAL constant_ptr<const char, 0x8266A0> const MIXFILES_MOVMD03_MIX {};
		static COMPILETIMEEVAL constant_ptr<const char, 0x8266FC> const MIXFILES_MOVIES__MIX {};
		static COMPILETIMEEVAL constant_ptr<const char, 0x8266C4> const MIXFILES_MOVMD__MIX {};
		static COMPILETIMEEVAL constant_ptr<const char, 0x82672C> const MOVIES__MIX {};
		static COMPILETIMEEVAL constant_ptr<const char, 0x81C200> const MOVIES02d_MIX {};
		static COMPILETIMEEVAL constant_ptr<const char, 0x81C210> const MOVMD02d_MIX {};
		static COMPILETIMEEVAL constant_ptr<const char, 0x826748> const MOVMD__MIX {};

		if (CD::IsLocal()) {

			IsLocal = true;
			// Probe for the best-matching movie wildcard pattern.
			// Original checks several sentinel filenames in priority order to
			// pick a search pattern; last match wins.
			const struct { const char* probe; const char* pattern; } movieProbes[] =
			{
				{ MOVIES01_MIX(),           MOVIES__MIX() },
				{ MIXFILES_MOVIES01_MIX(), MIXFILES_MOVIES__MIX() },
				{ MOVMD01_MIX(),             MOVMD__MIX() },
				{ MIXFILES_MOVMD01_MIX(),  MIXFILES_MOVMD__MIX() },
				{ MOVMD03_MIX(),             MOVMD__MIX() },
				{ MIXFILES_MOVMD03_MIX(),  MIXFILES_MOVMD__MIX() },
			};

			strcpy_s(v73, MOVMD__MIX()); // default pattern

			for (const auto& p : movieProbes) {
				RawFileClass probe(p.probe);
				if (probe.IsAvaible(0))
					strcpy_s(v73, p.pattern);
			}

			if (!LoadMixWildcard(v73, MoviesMix, MixFileClass::Movies())) {
				MixFileClass::Movies->emplace_back(GameCreate<MixFileClass>(v73, pKey));
			}

			MoviesFound =  MoviesMix() != nullptr;
		}else{

			// CD/disc movie files — numbered.
			sprintf_s(v73, MOVMD02d_MIX(), cdIndex);
			if(!CheckAndAllocateMix(v73, MoviesMix())) {
				// MOVMD not present — try MOVIES##.MIX instead.
				sprintf_s(v73, MOVIES02d_MIX(), cdIndex);
				CheckAndAllocateMix(v73, MoviesMix());
			}

			Debug::LogInfo("Loading {}", v73);
			MoviesFound = MoviesMix() != nullptr;
		}
	}

	if(!MoviesFound)
		Debug::LogInfo("FailedToLoad Movies local {}" , IsLocal);

	return MoviesFound;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x530460, Init_Secondary_Mixfiles);

ASMJIT_PATCH(0x52C58F, InitGame_SecondaryMixInit, 0x5)
{
	Debug::LogInfo("Init Secondary Mixfiles.....");
	const bool result = Init_Secondary_Mixfiles();
	Debug::LogInfo(" ...{} !!!", !result ? "FAILED" : "OK");
	
	if (!SpawnerMain::Configs::Enabled) {
		if (!Phobos::Otamaa::NoLogo)
			Game::PlayMovie("EA_WWLOGO", -1, true, true, true, false);

		Game::PlayLoadingScreen();
	}
	
	return 0x52C5F8;
}

ASMJIT_PATCH(0x6BE9BD, Game_ProgramEnd_ClearResource, 6)
{
	PhobosGlobal::Instance()->aresMIX.reset(nullptr);
	if (SpawnerMain::Configs::Enabled)
	{
		for (auto& Spawner_Mix : SpawnerMain::LoadedMixFiles)
			GameDelete<true>(std::exchange(Spawner_Mix, nullptr));
	}
	return 0;
}

//we dont use this anymore lmao
//ASMJIT_PATCH(0x531413, Load_Game_LoadScreen_Add, 5)
//{
//	int topActive = 500;
//	DSurface::Hidden->DrawText_Old(L"Ares is active.", 10, topActive, COLOR_GREEN);
//	DSurface::Hidden->DrawText_Old(L"Ares is © The Ares Contributors 2007 - 2021.", 10, 520, COLOR_GREEN);
//	DSurface::Hidden->DrawText_Old(L"Ares version: 3.0p1 Backport", 10, 540, COLOR_RED | COLOR_GREEN);
//	return 0;
//}

//
typedef BOOL(__stdcall* FP_MoveWindow)(HWND hWnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint);
static COMPILETIMEEVAL referencefunc<FP_MoveWindow, 0x7E1398> const Game_MoveWindow {};

ASMJIT_PATCH(0x532017, DlgProc_MainMenu_Version, 5)
{
	GET(HWND, hWnd, ESI);

	// account for longer version numbers
	const int MinimumWidth = 168;

	RECT Rect;
	if (Imports::GetWindowRect.invoke()(hWnd, &Rect))
	{
		int width = Rect.right - Rect.left;

		if (width < MinimumWidth)
		{
			// extend to the left by the difference
			Rect.left -= (MinimumWidth - width);

			// if moved out of screen, move right by this amount
			if (Rect.left < 0)
			{
				Rect.right += -Rect.left;
				Rect.left = 0;
			}

			Game_MoveWindow.invoke()(hWnd, Rect.left, Rect.top, Rect.right - Rect.left, Rect.bottom - Rect.top, FALSE);
		}
	}

	return 0;
}

ASMJIT_PATCH(0x5FACDF, Options_LoadFromINI, 5)
{
	Phobos::Config::Read_RA2MD();
	Phobos::Config::Read_UIMD();
	return 0x0;
}

//#include <Ext/Convert/Body.h>
//
//_GET_FUNCTION_ADDRESS(ConvertClassExt::AllocBlitters, GetConvertClassExtAllocBlittersAddress)
//_GET_FUNCTION_ADDRESS(ConvertClassExt::DeallocBlitters, GetConvertClassExtDeallocBlittersAddress)
//

ASMJIT_PATCH(0x7C89D4, DDRAW_Create, 6)
{
	R->Stack<DWORD>(0x4, Phobos::Config::GFX_DX_Force);
	return 0;
}


ASMJIT_PATCH(0x55E477, Game_ComScenarioDialog_ChatBox, 0x5)
{
	if (FakeRulesClass::Instance()->AllowChatBoxInSinglePlayer)
		return 0x55E48D;

	return 0;
}

ASMJIT_PATCH(0x55E62F, Game_ComScenarioDialog_ChatBox2, 0x6)
{
	if (FakeRulesClass::Instance()->AllowChatBoxInSinglePlayer)
		return 0x55E637;

	return 0;
}

ASMJIT_PATCH(0x55E693, Game_ComScenarioDialog_ChatBox3, 0x6)
{
	if (FakeRulesClass::Instance()->AllowChatBoxInSinglePlayer)
		return 0x55E69B;

	return 0;
}

ASMJIT_PATCH(0x55E746, Game_ComScenarioDialog_ChatBox4, 0x5)
{
	if (FakeRulesClass::Instance()->AllowChatBoxInSinglePlayer)
		return 0x55E77B;

	return 0;
}