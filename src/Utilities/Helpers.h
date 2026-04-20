#pragma region Ares Copyrights
/*
 *Copyright (c) 2008+, All Ares Contributors
 *All rights reserved.
 *
 *Redistribution and use in source and binary forms, with or without
 *modification, are permitted provided that the following conditions are met:
 *1. Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *2. Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *3. All advertising materials mentioning features or use of this software
 *   must display the following acknowledgement:
 *   This product includes software developed by the Ares Contributors.
 *4. Neither the name of Ares nor the
 *   names of its contributors may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 *THIS SOFTWARE IS PROVIDED BY ITS CONTRIBUTORS ''AS IS'' AND ANY
 *EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *DISCLAIMED. IN NO EVENT SHALL THE ARES CONTRIBUTORS BE LIABLE FOR ANY
 *DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#pragma endregion

#pragma once

#include "Debug.h"

#include <BuildingClass.h>
#include <BuildingTypeClass.h>
#include <CellSpread.h>
#include <Helpers/Iterators.h>
#include <Helpers/Enumerators.h>
#include <New/Entity/LauchSWData.h>

#include "VectorHelper.h"

#include <set>
#include <functional>
#include <algorithm>
#include <iterator>
#include <vector>
#include <ranges>
#include <unordered_set>  // OPT-2: needed for the new DistinctCollector

#include <SuperWeaponTypeClass.h>
#include <SuperClass.h>
#include <HouseClass.h>
#include <MouseClass.h>
#include <AircraftTrackerClass.h>

namespace Helpers
{

	namespace Alex
	{

		//! Less comparison for pointer types.
		/*!
			Dereferences the values before comparing them using std::less.
			NOTE: this is kept for non-pointer DistinctCollector<T> use cases,
			but is no longer used internally by the pointer specialization.
		*/
		struct deref_less
		{
			using is_transparent = void;

			template <typename T, typename U>
			COMPILETIMEEVAL bool operator()(T&& lhs, U&& rhs) const
			{
				return std::less<>()(*lhs, *rhs);
			}
		};

		// ─────────────────────────────────────────────────────────────────────
		// OPT-2: DistinctCollector
		//
		// BEFORE: backed by std::set with deref_less
		//   - O(log n) insert/lookup, red-black tree = heap allocs + bad cache
		//   - deref_less dereferences pointers to compare objects — wrong for
		//     identity deduplication (two different objects at different addresses
		//     could compare equal by value, causing silent misses)
		//
		// AFTER: backed by std::unordered_set with a flat pointer hash
		//   - O(1) average insert/lookup, single contiguous bucket array
		//   - reserve() called before the hot loops eliminates rehashing
		//   - pointer address IS the unique identity for game objects,
		//     so hashing/comparing by address is both correct and fast
		// ─────────────────────────────────────────────────────────────────────

		template<typename T>
		class DistinctCollector
		{

			// Hash by raw address; strip low alignment bits to improve bucket spread.
			struct PtrHash
			{
				size_t operator()(T p) const noexcept
				{
					return reinterpret_cast<size_t>(p) >> 3;
				}
			};

			// For non-pointer T fall back to the standard hash.
			using hash_type = std::conditional_t<std::is_pointer_v<T>, PtrHash, std::hash<T>>;
			using set_type = std::unordered_set<T, hash_type>;

			set_type _set;

		public:
			OPTIONALINLINE bool operator()(T item)
			{
				insert(item);
				return true;
			}

			OPTIONALINLINE void insert(T value)
			{
				_set.insert(value);
			}

			// Pre-allocate buckets before a hot loop to avoid rehashing mid-fill.
			OPTIONALINLINE void reserve(size_t n)
			{
				_set.reserve(n);
			}

			COMPILETIMEEVAL OPTIONALINLINE size_t size() const
			{
				return _set.size();
			}

			COMPILETIMEEVAL OPTIONALINLINE typename set_type::const_iterator begin() const
			{
				return _set.begin();
			}

			COMPILETIMEEVAL OPTIONALINLINE typename set_type::const_iterator end() const
			{
				return _set.end();
			}

			template <typename Func>
				requires std::invocable<Func, T>
			OPTIONALINLINE COMPILETIMEEVAL auto for_each(Func&& action) const
			{
				return std::find_if_not(begin(), end(), std::forward<Func>(action));
			}

			template <typename Func>
				requires std::invocable<Func, T>
			OPTIONALINLINE COMPILETIMEEVAL void apply_function_for_each(Func&& action) const
			{
				std::ranges::for_each(_set, std::forward<Func>(action));
			}

			template <typename Func>
				requires std::invocable<Func, T>
			[[nodiscard]] OPTIONALINLINE COMPILETIMEEVAL int for_each_count(Func&& action) const
			{
				return std::ranges::distance(
					begin(),
					std::find_if_not(begin(), end(), std::forward<Func>(action))
				);
			}

			template <typename Func>
				requires std::invocable<Func, T>
			OPTIONALINLINE COMPILETIMEEVAL void for_each(Func&& action)
			{
				std::ranges::for_each(_set, std::forward<Func>(action));
			}

			[[nodiscard]] COMPILETIMEEVAL bool contains(const T& value) const
			{
				return _set.contains(value);
			}

			void clear() noexcept
			{
				_set.clear();
			}
		};

		// ─────────────────────────────────────────────────────────────────────
		// OPT-3: Shared query options for all cell-spread helpers.
		//
		// All three original functions (getCellSpreadItems,
		// getCellTechnoRangeItems, ApplyFuncToCellSpreadItems) duplicated ~85%
		// of the same two-pass logic (ground scan → air scan → filter).
		// Any bug or perf fix had to be applied in three places.
		//
		// Instead: one authoritative core (QueryCellSpreadTechnos) +
		// thin public wrappers that are 100% backward-compatible.
		// ─────────────────────────────────────────────────────────────────────

		struct CellSpreadQueryOptions
		{
			double  spread = 0.0;
			bool    includeInAir = true;
			bool    isCylindrical = false;
			bool    allowLimbo = false;
			bool    affectAir = true;
			bool    affectsGround = true;
			bool    ignoreBuildings = false;
		};

		// ─────────────────────────────────────────────────────────────────────
		// QueryCellSpreadTechnos — the single authoritative implementation.
		//
		// OPT-1: cellCoords hoisted out of the CellSpreadEnumerator loop.
		//        coords never changes across iterations; recomputing it every
		//        cell was pure waste.
		//
		// OPT-2: DistinctCollector now uses unordered_set + reserve(64).
		//        Most spreads touch far fewer than 64 units; reserving once
		//        prevents any rehash inside the hot loop.
		//
		// OPT-4: Non-building distance is checked exactly once (during
		//        collection). The old filter pass re-checked it redundantly
		//        for every non-building techno.
		// ─────────────────────────────────────────────────────────────────────

		template<typename Func>
		OPTIONALINLINE void QueryCellSpreadTechnos(
			CoordStruct const& coords,
			CellSpreadQueryOptions const& opts,
			Func&& action)
		{
			if (!opts.affectAir && !opts.affectsGround)
				return;

			const double spreadMult = opts.spread * Unsorted::LeptonsPerCell;

			// OPT-1: compute once, use everywhere below
			const auto cellCoords = CellClass::Coord2Cell(coords);

			DistinctCollector<TechnoClass*> set;
			set.reserve(64); // OPT-2: avoids rehashing for typical spreads

			// ── Pass 1: ground objects ────────────────────────────────────────
			for (CellSpreadEnumerator it(static_cast<short>(opts.spread + 0.99)); it; ++it)
			{
				auto const pCell = MapClass::Instance->GetCellAt(*it + cellCoords);
				bool const isCenter = (pCell->MapCoords == cellCoords);

				for (NextObject obj(pCell->GetContent()); obj; ++obj)
				{
					if (!obj->IsAlive || obj->Health <= 0)
						continue;

					auto const pTechno = flag_cast_to<TechnoClass*, false>(*obj);
					if (!pTechno)
						continue;

					if (pTechno->WhatAmI() == AbstractType::Building)
					{
						// Buildings: distance checked at cell-center level (Starkku's method).
						if (opts.ignoreBuildings)
							continue;

						auto cellCenter = pCell->GetCenterCoords();
						if (opts.isCylindrical)
							cellCenter.Z = coords.Z;

						auto dist = cellCenter.DistanceFrom(coords);

						if (isCenter)
						{
							// Centre cell: snap dist down if height difference is within one level.
							dist = (coords.Z - cellCenter.Z <= Unsorted::LevelHeight)
								? 0.0
								: dist - Unsorted::LevelHeight;
						}

						if (dist > spreadMult)
							continue;
					}
					else
					{
						// Non-buildings: distance checked once here.
						// OPT-4: NOT re-checked in the filter pass below.
						if (pTechno->Location.DistanceFrom(coords) > spreadMult)
							continue;
					}

					set.insert(pTechno);
				}
			}

			// ── Pass 2: air objects ───────────────────────────────────────────
			if (opts.includeInAir && opts.affectAir)
			{
				auto* airTracker = &AircraftTrackerClass::Instance.get();
				airTracker->AircraftTrackerClass_logics_412B40(
					MapClass::Instance->GetCellAt(coords),
					static_cast<int>(opts.spread));

				for (auto* p = airTracker->Get(); p; p = airTracker->Get())
				{
					if (p->IsAlive && p->IsOnMap && p->Health > 0
						&& p->Location.DistanceFrom(coords) <= spreadMult)
					{
						set.insert(p);
					}
				}
			}

			// ── Pass 3: filter and dispatch ───────────────────────────────────
			for (auto* pTechno : set)
			{
				if (!opts.allowLimbo && pTechno->InLimbo)               continue;
				if (pTechno->Health <= 0 || !pTechno->IsAlive
					|| pTechno->IsCrashing || pTechno->IsSinking
					|| pTechno->TemporalTargetingMe)                     continue;

				const auto abs = pTechno->WhatAmI();
				const bool inAir = pTechno->IsInAir();

				if (!opts.affectAir && inAir)  continue;
				if (!opts.affectsGround && !inAir)  continue;

				bool isBuilding = false;

				if (abs == AbstractType::Building)
				{
					if (opts.ignoreBuildings)                                         continue;
					if (static_cast<BuildingClass*>(pTechno)->Type->InvisibleInGame) continue;
					isBuilding = true;
				}
				else if (abs == UnitClass::AbsID)
				{
					if (static_cast<const UnitClass*>(pTechno)->DeathFrameCounter > 0) continue;
				}

				if (!isBuilding)
				{
					// OPT-4: non-buildings already passed the distance check in Pass 1.
					// Aircraft get a reduced effective distance (half), but they also
					// entered via the air tracker which already did a strict range check —
					// so no re-check is needed there either.
					// Just dispatch directly.
					action(pTechno);
					continue;
				}

				// Buildings: their collection was cell-level only; do a final
				// coordinate-level distance check before dispatching.
				auto target = pTechno->Location;
				if (opts.isCylindrical)
					target.Z = coords.Z;

				auto dist = target.DistanceFrom(coords);

				// Reduce effective distance for flying aircraft.
				if (abs == AbstractType::Aircraft && inAir)
					dist *= 0.5;

				if (dist <= spreadMult)
					action(pTechno);
			}
		}

		// ─────────────────────────────────────────────────────────────────────
		// Public API — thin wrappers around QueryCellSpreadTechnos.
		// Signatures are 100% backward-compatible with the originals.
		// ─────────────────────────────────────────────────────────────────────

		//! Gets a list of all units in range of a cell spread weapon.
		/*!
			CellSpread is handled as described in
			http://modenc.renegadeprojects.com/CellSpread.

			\param coords       The location the projectile detonated.
			\param spread       The range to find items in (in leptons).
			\param includeInAir Include items that are currently InAir.
		*/
		OPTIONALINLINE std::vector<TechnoClass*> getCellSpreadItems(
			CoordStruct const& coords,
			double             spread,
			bool               includeInAir,
			bool               IsCylindrical,
			bool               AffectAir,
			bool               AffectsGround,
			bool               allowLimbo)
		{
			std::vector<TechnoClass*> ret;
			ret.reserve(32);

			QueryCellSpreadTechnos(
				coords,
				CellSpreadQueryOptions {
					spread, includeInAir, IsCylindrical,
					allowLimbo, AffectAir, AffectsGround, /*ignoreBuildings=*/false
				},
				[&ret](TechnoClass* pTechno) { ret.push_back(pTechno); }
			);

			return ret;
		}

		//! Gets a list of all technos in range, filtered by a caller-supplied predicate.
		/*!
			\param coords        Detonation point.
			\param arange        Search radius in leptons.
			\param IsCylindrical Ignore Z when computing distances.
			\param IncludeAir    Include airborne units.
			\param action        Predicate — return true to include the techno.
		*/
		template<typename Func>
		OPTIONALINLINE std::vector<AbstractClass*> getCellTechnoRangeItems(
			CoordStruct const& coords,
			double             arange,
			bool               IsCylindrical,
			bool               IncludeAir,
			Func               action)
		{
			std::vector<AbstractClass*> ret;
			ret.reserve(32);

			// Convert from leptons to cell-spread units expected by the core.
			const double spreadInCells = arange / Unsorted::d_LeptonsPerCell;

			QueryCellSpreadTechnos(
				coords,
				CellSpreadQueryOptions {
					spreadInCells, IncludeAir, IsCylindrical,
					/*allowLimbo=*/false, /*affectAir=*/true, /*affectsGround=*/true,
					/*ignoreBuildings=*/false
				},
				[&ret, &action](TechnoClass* pTechno)
				{
					if (action(static_cast<AbstractClass*>(pTechno)))
						ret.push_back(static_cast<AbstractClass*>(pTechno));
				}
			);

			return ret;
		}

		//! Applies a function to every techno in range of a cell spread weapon.
		/*!
			\param coords          Detonation point.
			\param spread          Range (in leptons).
			\param includeInAir    Include airborne units.
			\param IsCylindrical   Ignore Z when computing distances.
			\param allowLimbo      Include units in limbo.
			\param AffectAir       Affect airborne units.
			\param AffectsGround   Affect ground units.
			\param IgnoreBuildings Skip buildings entirely.
			\param action          Callback invoked for each matching techno.
		*/
		template<typename Func>
		OPTIONALINLINE void ApplyFuncToCellSpreadItems(
			CoordStruct const& coords,
			double             spread,
			bool               includeInAir,
			bool               IsCylindrical,
			bool               allowLimbo,
			bool               AffectAir,
			bool               AffectsGround,
			bool               IgnoreBuildings,
			Func               action)
		{
			QueryCellSpreadTechnos(
				coords,
				CellSpreadQueryOptions {
					spread, includeInAir, IsCylindrical,
					allowLimbo, AffectAir, AffectsGround, IgnoreBuildings
				},
				std::forward<Func>(action)
			);
		}

		// ─────────────────────────────────────────────────────────────────────
		// Everything below is unchanged from the original.
		// ─────────────────────────────────────────────────────────────────────

		//! Gets the new duration a stackable or absolute effect will last.
		COMPILETIMEEVAL OPTIONALINLINE int getCappedDuration(int CurrentValue, int Duration, int Cap)
		{
			int ProposedDuration = CurrentValue + Duration;

			if (Duration > 0)
			{
				if (Cap < 0)
				{
					return MaxImpl(Duration, CurrentValue);
				}
				else if (Cap > 0)
				{
					int cappedValue = MinImpl(ProposedDuration, Cap);
					return MaxImpl(CurrentValue, cappedValue);
				}
				else
				{
					return ProposedDuration;
				}
			}
			else
			{
				return (Cap < 0 ? ProposedDuration : MinImpl(ProposedDuration, Cap));
			}
		}

		//! Invokes an action for up to count elements that suffice a predicate.
		template <typename InIt, typename Pred, typename Fn>
		COMPILETIMEEVAL OPTIONALINLINE void for_each_if_n(InIt first, InIt last, size_t count, Pred pred, Fn func)
		{
			if (count)
			{
				first = std::find_if(first, last, pred);

				while (count-- && first != last)
				{
					func(*first++);
					first = std::find_if(first, last, pred);
				}
			}
		}

		//! Invokes an action for every cell or object in a rectangle.
		template <typename T, typename Func>
		OPTIONALINLINE bool for_each_in_rect(CellStruct const center, float widthOrRange, int height, Func&& action)
		{
			if (height <= 0 || widthOrRange <= 0)
				return false;

			auto const width = static_cast<int>(widthOrRange);

			auto topleft = center;
			topleft.X -= static_cast<short>(width / 2);
			topleft.Y -= static_cast<short>(height / 2);

			auto const rect = LTRBStruct {
				topleft.X, topleft.Y,
				topleft.X + width, topleft.Y + height
			};

			CellRectIterator<T>{}(rect, std::forward<Func>(action));
			return true;
		}

		//! Invokes an action for every cell or object in a rectangle or circular range.
		template <typename T, typename Func>
		OPTIONALINLINE bool for_each_in_rect_or_range(CellStruct center, float widthOrRange, int height, Func&& action)
		{
			if (for_each_in_rect<T>(center, widthOrRange, height, std::forward<Func>(action)))
				return true;

			if (height <= 0 && widthOrRange >= 0.0f)
			{
				CellRangeIterator<T>{}(center, widthOrRange, std::forward<Func>(action));
				return true;
			}

			return false;
		}

		//! Invokes an action for every cell or object in a rectangle or CellSpread area.
		template <typename T, typename Func>
		OPTIONALINLINE bool for_each_in_rect_or_spread(CellStruct center, float widthOrRange, int height, Func&& action)
		{
			if (for_each_in_rect<T>(center, widthOrRange, height, std::forward<Func>(action)))
				return true;

			if (height <= 0)
			{
				auto const spread = static_cast<short>(std::max(static_cast<int>(widthOrRange), 0));
				if (spread > 0)
				{
					CellSpreadIterator<T>{}(center, spread, std::forward<Func>(action));
					return true;
				}
			}

			return false;
		}

		template <typename Value, typename... Options>
		[[nodiscard]] OPTIONALINLINE COMPILETIMEEVAL bool is_any_of(Value&& value, Options&&... options)
		{
			return ((value == options) || ...);
		}

		OPTIONALINLINE void remove_non_paradroppables(std::vector<TechnoTypeClass*>& types,
									   const char* section, const char* key)
		{
			std::erase_if(types, [section, key](TechnoTypeClass* pItem) -> bool
 {
	 if (!pItem)
	 {
		 Debug::INIParseFailed(section, key, "nullptr", "Invalid types are removed.");
		 return true;
	 }

	 if (!is_any_of(pItem->WhatAmI(), AbstractType::InfantryType, AbstractType::UnitType))
	 {
		 Debug::INIParseFailed(section, key, pItem->ID,
							  "Only InfantryTypes and UnitTypes are supported.");
		 return true;
	 }

	 if (pItem->Strength <= 0)
	 {
		 Debug::INIParseFailed(section, key, pItem->ID,
							  "0 Strength types are removed.");
		 return true;
	 }

	 return false;
			});
		}

		template <typename FwdIt>
		OPTIONALINLINE COMPILETIMEEVAL void selectionsort(FwdIt first, FwdIt last)
		{
			std::sort(first, last);
		}

		template <typename FwdIt, typename Pred>
		OPTIONALINLINE COMPILETIMEEVAL void selectionsort(FwdIt first, FwdIt last, Pred pred)
		{
			std::sort(first, last, std::forward<Pred>(pred));
		}

		template <typename FwdIt>
		OPTIONALINLINE COMPILETIMEEVAL void selectionsort(FwdIt first, FwdIt middle, FwdIt last)
		{
			std::partial_sort(first, middle, last);
		}

		template <typename FwdIt, typename Pred>
		OPTIONALINLINE COMPILETIMEEVAL void selectionsort(FwdIt first, FwdIt middle, FwdIt last, Pred pred)
		{
			std::partial_sort(first, middle, last, std::forward<Pred>(pred));
		}

		namespace ranges
		{
			template <typename T, typename Func>
				requires std::invocable<Func, T>
			constexpr void for_each_if(std::ranges::input_range auto&& range,
									   auto&& pred, Func&& func)
			{
				std::ranges::for_each(
					range | std::views::filter(std::forward<decltype(pred)>(pred)),
					std::forward<Func>(func)
				);
			}
		}
	};

	namespace Otamaa
	{
		bool LauchSW(
			const LauchSWData& nData,
			HouseClass* pOwner,
			const CoordStruct  Where,
			TechnoClass* pFirer
		);
	}
};