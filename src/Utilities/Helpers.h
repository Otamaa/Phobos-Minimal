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

#include <SuperWeaponTypeClass.h>
#include <SuperClass.h>
#include <HouseClass.h>
#include <MouseClass.h>
#include <AircraftTrackerClass.h>

namespace Helpers {

	namespace Alex {

		//! Less comparison for pointer types.
		/*!
			Dereferences the values before comparing them using std::less.

			This compares the actual objects pointed to instead of their
			arbitrary pointer values.
		*/
		struct deref_less {
			using is_transparent = void;

			template <typename T, typename U>
			COMPILETIMEEVAL bool operator()(T&& lhs, U&& rhs) const {
				return std::less<>()(*lhs, *rhs);
			}
		};

		//! Represents a set of unique items.
		/*!
			Items can be added using the insert method. Even though an item
			can be added multiple times, it is only contained once in the set.

			Use either the for_each method to call a method using each item as
			a parameter, or iterate the set through the begin and end methods.
		*/
		template<typename T>
		class DistinctCollector {
			using less_type = std::conditional_t<std::is_pointer<T>::value, deref_less, std::less<>>;
			using set_type = std::set<T, less_type>;
			set_type _set;

		public:
			OPTIONALINLINE bool operator() (T item) {
				insert(item);
				return true;
			}

			OPTIONALINLINE void insert(T value) {
				_set.insert(value);
			}

			COMPILETIMEEVAL OPTIONALINLINE size_t size() const {
				return _set.size();
			}

			COMPILETIMEEVAL OPTIONALINLINE typename set_type::const_iterator begin() const {
				return _set.begin();
			}

			COMPILETIMEEVAL OPTIONALINLINE typename set_type::const_iterator end() const {
				return _set.end();
			}

			// C++20: Use concepts for better error messages
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

			// C++20: Add contains() for convenience
			[[nodiscard]] COMPILETIMEEVAL bool contains(const T& value) const
			{
				return _set.contains(value);
			}

			void clear() noexcept
			{
				_set.clear();
			}
		};

		//! Gets a list of all units in range of a cell spread weapon.
		/*!
			CellSpread is handled as described in
			http://modenc.renegadeprojects.com/CellSpread.

			\param coords The location the projectile detonated.
			\param spread(in leptons) The range to find items in.
			\param includeInAir Include items that are currently InAir.

			\author AlexB
			\date 2010-06-28
		*/
		OPTIONALINLINE std::vector<TechnoClass*> getCellSpreadItems(
		CoordStruct const& coords, double const spread,
			bool includeInAir, bool IsCylindrical, bool AffectAir, bool AffectsGround, bool allowLimbo)
		{
			if (AffectAir || AffectsGround)
			{
				// set of possibly affected objects. every object can be here only once.
				DistinctCollector<TechnoClass*> set;
				double const spreadMult = spread * Unsorted::LeptonsPerCell;

				// the quick way. only look at stuff residing on the very cells we are affecting.
				//auto const cellCoords = MapClass::Instance->GetCellAt(coords)->MapCoords;
				for (CellSpreadEnumerator it(static_cast<short>(spread + 0.99)); it; ++it)
				{
					auto cellCoords = CellClass::Coord2Cell(coords);
					auto const pCell = MapClass::Instance->GetCellAt(*it + cellCoords);
					bool isCenter = pCell->MapCoords == cellCoords;
					for (NextObject obj(pCell->GetContent()); obj; ++obj)
					{
						if (!obj->IsAlive || obj->Health <= 0)
							continue;

						if (auto const pTechno = flag_cast_to<TechnoClass*, false>(*obj))
						{

							// Starkku: Buildings need their distance from the origin coords checked at cell level.
							if (pTechno->WhatAmI() == AbstractType::Building)
							{
								auto cellCenterCoords = pCell->GetCenterCoords();
								// Ignore Z coordinate if detonation is cylindrical.
								if (IsCylindrical)
									cellCenterCoords.Z = coords.Z;

								auto dist = cellCenterCoords.DistanceFrom(coords);

								// If this is the center cell, there's some different behaviour.
								if (isCenter)
								{
									if (coords.Z - cellCenterCoords.Z <= Unsorted::LevelHeight)
										dist = 0;
									else
										dist -= Unsorted::LevelHeight;
								}

								if (dist > spreadMult)
									continue;
							}
							else if (pTechno->Location.DistanceFrom(coords) > spreadMult)
							{
								continue;
							}

							set.insert(pTechno);
						}
					}
				}

				// flying objects are not included normally, use AircraftTrackerClass to find the affected ones.
				if (includeInAir && AffectAir)
				{
					auto const airTracker = &AircraftTrackerClass::Instance.get();
					airTracker->AircraftTrackerClass_logics_412B40(MapClass::Instance->GetCellAt(coords), int(spread));

					for (auto pTechno = airTracker->Get(); pTechno; pTechno = airTracker->Get())
					{
						if (pTechno->IsAlive && pTechno->IsOnMap && pTechno->Health > 0)
						{
							if (pTechno->Location.DistanceFrom(coords) <= spreadMult)
							{
								set.insert(pTechno);
							}
						}
					}
				}

				// look closer. the final selection. put all affected items in a vector.
				std::vector<TechnoClass*> ret;
				ret.reserve(set.size());

				for (auto const& pTechno : set) {

					if (!allowLimbo && pTechno->InLimbo)
						continue;

					//basic checking
					if (pTechno->Health <= 0
						|| !pTechno->IsAlive
						|| pTechno->IsCrashing
						|| pTechno->IsSinking
						|| pTechno->TemporalTargetingMe)
						continue;

					auto const abs = pTechno->WhatAmI();
					bool inAir = pTechno->IsInAir();

					if (!AffectAir && inAir || !AffectsGround && !inAir)
						continue;

					bool isBuilding = false;

					// ignore buildings that are not visible, like ambient light posts
					if (abs == AbstractType::Building)
					{
						auto const pBuilding = static_cast<BuildingClass*>(pTechno);
						if (pBuilding->Type->InvisibleInGame) { continue; }

						isBuilding = true;
					}
					else
					{
						if (abs == UnitClass::AbsID)
						{
							if (static_cast<const UnitClass*>(pTechno)->DeathFrameCounter > 0)
							{
								continue;
							}
						}
					}

					// get distance from impact site
					auto target = pTechno->Location;
					// Ignore Z coordinate if detonation is cylindrical.
					if (IsCylindrical)
						target.Z = coords.Z;

					auto dist = target.DistanceFrom(coords);

					// reduce the distance for flying aircraft
					if (abs == AbstractType::Aircraft && pTechno->IsInAir())
					{
						dist *= 0.5;
					}

					// this is good
					// Starkku: Building distance is checked prior on cell level, skip here.
					if (isBuilding || dist <= spreadMult)
					{
						ret.push_back(pTechno);
					}
				}

				return ret;
			}

			return {};
		}

		template<typename Func>
		OPTIONALINLINE std::vector<AbstractClass*> getCellTechnoRangeItems(CoordStruct const& coords, double arange, bool IsCylindrical, bool IncludeAir, Func action)
		{
			// set of possibly affected objects. every object can be here only once.
			DistinctCollector<AbstractClass*> set;
			double const spread = arange / Unsorted::d_LeptonsPerCell;

			// the quick way. only look at stuff residing on the very cells we are affecting.

			for (CellSpreadEnumerator it(static_cast<short>(spread + 0.99)); it; ++it)
			{
				auto cellCoords = CellClass::Coord2Cell(coords);
				auto const pCell = MapClass::Instance->GetCellAt(*it + cellCoords);
				bool isCenter = pCell->MapCoords == cellCoords;
				for (NextObject obj(pCell->GetContent()); obj; ++obj)
				{
					if (!obj->IsAlive || obj->Health <= 0)
						continue;

					if (auto const pTechno = flag_cast_to<TechnoClass*, false>(*obj))
					{

						// Starkku: Buildings need their distance from the origin coords checked at cell level.
						if (pTechno->WhatAmI() == AbstractType::Building)
						{
							auto cellCenterCoords = pCell->GetCenterCoords();
							// Ignore Z coordinate if detonation is cylindrical.
							if (IsCylindrical)
								cellCenterCoords.Z = coords.Z;

							auto dist = cellCenterCoords.DistanceFrom(coords);
							// If this is the center cell, there's some different behaviour.
							if (isCenter)
							{
								if (coords.Z - cellCenterCoords.Z <= Unsorted::LevelHeight)
									dist = 0;
								else
									dist -= Unsorted::LevelHeight;
							}

							if (dist > arange)
								continue;
						}
						else if (pTechno->Location.DistanceFrom(coords) > arange)
						{
							continue;
						}

						set.insert(pTechno);
					}
				}
			}

			// flying objects are not included normally, use AircraftTrackerClass to find the affected ones.
			if (IncludeAir) {
				auto const airTracker = &AircraftTrackerClass::Instance.get();
				airTracker->AircraftTrackerClass_logics_412B40(MapClass::Instance->GetCellAt(coords), int(arange));

				for (auto pTechno = airTracker->Get(); pTechno; pTechno = airTracker->Get()) {
					if (pTechno->IsAlive && pTechno->IsOnMap && pTechno->Health > 0) {
						if (pTechno->Location.DistanceFrom(coords) <= arange)
						{
							set.insert(pTechno);
						}
					}
				}
			}

			// look closer. the final selection. put all affected items in a vector.
			std::vector<AbstractClass*> ret;
			ret.reserve(set.size());

			for (auto const& pItems : set) {

				if (pItems->WhatAmI() != AbstractType::Building) {
					auto target = pItems->GetCoords();
					// Ignore Z coordinate if detonation is cylindrical.
					if (IsCylindrical)
						target.Z = coords.Z;

					if (target.DistanceFrom(coords) > arange) {
						continue;
					}
				}

				if (action(pItems)) {
					ret.push_back(pItems);
				}
			}

			return ret;
		}

		template<typename Func>
		OPTIONALINLINE void ApplyFuncToCellSpreadItems(
			CoordStruct const& coords, double const spread, bool includeInAir, bool IsCylindrical, bool allowLimbo, bool AffectAir, bool AffectsGround , bool IgnoreBuildings, Func action)
		{
			if (AffectAir || AffectsGround)
			{
				// set of possibly affected objects. every object can be here only once.
				DistinctCollector<TechnoClass*> set;
				double const spreadMult = spread * Unsorted::LeptonsPerCell;

				// the quick way. only look at stuff residing on the very cells we are affecting.
				//auto const cellCoords = MapClass::Instance->GetCellAt(coords)->MapCoords;
				for (CellSpreadEnumerator it(static_cast<short>(spread + 0.99)); it; ++it)
				{
					auto cellCoords = CellClass::Coord2Cell(coords);
					auto const pCell = MapClass::Instance->GetCellAt(*it + cellCoords);
					bool isCenter = pCell->MapCoords == cellCoords;
					for (NextObject obj(pCell->GetContent()); obj; ++obj)
					{
						if (!obj->IsAlive || obj->Health <= 0)
							continue;

						if (auto const pTechno = flag_cast_to<TechnoClass*, false>(*obj))
						{

							// Starkku: Buildings need their distance from the origin coords checked at cell level.
							if (pTechno->WhatAmI() == AbstractType::Building && !IgnoreBuildings)
							{
								auto cellCenterCoords = pCell->GetCenterCoords();
								// Ignore Z coordinate if detonation is cylindrical.
								if (IsCylindrical)
									cellCenterCoords.Z = coords.Z;

								auto dist = cellCenterCoords.DistanceFrom(coords);

								// If this is the center cell, there's some different behaviour.
								if (isCenter)
								{
									if (coords.Z - cellCenterCoords.Z <= Unsorted::LevelHeight)
										dist = 0;
									else
										dist -= Unsorted::LevelHeight;
								}

								if (dist > spreadMult)
									continue;
							}
							else if (pTechno->Location.DistanceFrom(coords) > spreadMult)
							{
								continue;
							}

							set.insert(pTechno);
						}
					}
				}

				// flying objects are not included normally, use AircraftTrackerClass to find the affected ones.
				if (includeInAir && AffectAir)
				{
					auto const airTracker = &AircraftTrackerClass::Instance.get();
					airTracker->AircraftTrackerClass_logics_412B40(MapClass::Instance->GetCellAt(coords), int(spread));

					for (auto pTechno = airTracker->Get(); pTechno; pTechno = airTracker->Get())
					{
						if (pTechno->IsAlive && pTechno->IsOnMap && pTechno->Health > 0)
						{
							if (pTechno->Location.DistanceFrom(coords) <= spreadMult)
							{
								set.insert(pTechno);
							}
						}
					}
				}

				for (auto const& pTechno : set) {

					if (!allowLimbo && pTechno->InLimbo)
						continue;

					//basic checking
					if (pTechno->Health <= 0
						|| !pTechno->IsAlive
						|| pTechno->IsCrashing
						|| pTechno->IsSinking
						|| pTechno->TemporalTargetingMe)
						continue;

					auto const abs = pTechno->WhatAmI();
					bool inAir = pTechno->IsInAir();

					if (!AffectAir && inAir || !AffectsGround && !inAir)
						continue;

					bool isBuilding = false;

					// ignore buildings that are not visible, like ambient light posts
					if (abs == AbstractType::Building) {
						if (IgnoreBuildings)
							continue;

						auto const pBuilding = static_cast<BuildingClass*>(pTechno);
						if (pBuilding->Type->InvisibleInGame) { continue; }

						isBuilding = true;
					} else {
						if (abs == UnitClass::AbsID) {
							if (static_cast<const UnitClass*>(pTechno)->DeathFrameCounter > 0) {
								continue;
							}
						}
					}

					// get distance from impact site
					auto target = pTechno->Location;
					// Ignore Z coordinate if detonation is cylindrical.
					if (IsCylindrical)
						target.Z = coords.Z;

					auto dist = target.DistanceFrom(coords);

					// reduce the distance for flying aircraft
					if (abs == AbstractType::Aircraft && pTechno->IsInAir())
					{
						dist *= 0.5;
					}

					// this is good
					// Starkku: Building distance is checked prior on cell level, skip here.
					if (isBuilding || dist <= spreadMult) {
						action(pTechno);
					}
				}
			}
		}

		//! Gets the new duration a stackable or absolute effect will last.
		/*!
			The new frames count is calculated the following way:

			If Duration is positive it will inflict damage. If Cap is larger than zero,
			the maximum amount of frames will be defined by Cap. If the current value
			already is larger than that, in will not be reduced. If Cap is zero, then
			the duration can add up infinitely. If Cap is less than zero, duration will
			be set to Duration, if the current value is not higher already.

			If Duration is negative, the effect will be reduced. A negative Cap
			reduces the current value by Duration. A positive or zero Cap will do the
			same, but additionally shorten it to Cap if the result would be higher than
			that. Thus, a Cap of zero removes the current effect altogether.

			\param CurrentValue The Technos current remaining time.
			\param Duration The duration the effect uses.
			\param Cap The maximum Duration this effect can cause.

			\returns The new effect frames count.

			\author AlexB
			\date 2010-04-27
		*/
		COMPILETIMEEVAL OPTIONALINLINE int getCappedDuration(int CurrentValue, int Duration, int Cap) {
			// Usually, the new duration is just added.
			int ProposedDuration = CurrentValue + Duration;

			if (Duration > 0) {
				// Positive damage.
				if (Cap < 0) {
					// Do not stack. Use the maximum value.
					return MaxImpl(Duration, CurrentValue);
				}
				else if (Cap > 0) {
					// Cap the duration.
					int cappedValue = MinImpl(ProposedDuration, Cap);
					return MaxImpl(CurrentValue, cappedValue);
				}
				else {
					// There is no cap. Allow the duration to stack up.
					return ProposedDuration;
				}
			}
			else {
				// Negative damage.
				return (Cap < 0 ? ProposedDuration : MinImpl(ProposedDuration, Cap));
			}
		}

		//! Invokes an action for up to count elements that suffice a predicate.
		/*!
			Complexity: linear. Up to distance(first, last) invocations of pred,
			up to min(distance(first, last), count) invocations of func.

			\param first Input iterator to be beginning.
			\param last Input iterator to the last.
			\param count The maximum number of elements to call func with.
			\param pred The predicate to decide whether to call func with an element.
			\param func The action to invoke for up to count elements that suffice pred.

			\author AlexB
			\date 2014-08-27
		*/
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

		//! Invokes an action for every cell or every object contained on the cells.
		/*!
			action is invoked only once per cell. action can be invoked multiple times
			on other objects.

			\param center The center cell of the area.
			\param widthOrRange The width of the rectangle.
			\param height The height of the rectangle.
			\param action The action to invoke for each object.

			\returns Returns true if widthOrRange and height describe a valid rectangle,
					 false otherwise.

			\author AlexB
		*/
		template <typename T, typename Func>
		OPTIONALINLINE bool for_each_in_rect(CellStruct const center, float widthOrRange, int height, Func&& action)
		{
			if (height <= 0 || widthOrRange <= 0)
			{
				return false;
			}

			auto const width = static_cast<int>(widthOrRange);

			// the coords mark the center of the area
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

		//! Invokes an action for every cell or every object contained on the cells.
		/*!
			action is invoked only once per cell. action can be invoked multiple times
			on other objects.

			\param center The center cell of the area.
			\param widthOrRange The width of the rectangle, or the radius, if height <= 0.
			\param height The height of the rectangle. Use 0 to create a circular area.
			\param action The action to invoke for each object.

			\returns Returns true if widthOrRange and height describe a valid rectangle
					 or circle, false otherwise.

			\author AlexB
		*/
		template <typename T, typename Func>
		OPTIONALINLINE bool for_each_in_rect_or_range(CellStruct center, float widthOrRange, int height, Func&& action)
		{
			if (for_each_in_rect<T>(center, widthOrRange, height, std::forward<Func>(action)))
			{
				return true;
			}

			if (height <= 0 && widthOrRange >= 0.0f)
			{
				CellRangeIterator<T>{}(center, widthOrRange, std::forward<Func>(action));
				return true;
			}

			return false;
		}

		//! Invokes an action for every cell or every object contained on the cells.
		/*!
			action is invoked only once per cell. action can be invoked multiple times
			on other objects.

			\param center The center cell of the area.
			\param widthOrRange The width of the rectangle, or the spread, if height <= 0.
			\param height The height of the rectangle. Use 0 to create a CellSpread area.
			\param action The action to invoke for each object.

			\returns Returns true if widthOrRange and height describe a valid rectangle
					 or CellSpread range, false otherwise.

			\author AlexB
		*/
		template <typename T, typename Func>
		OPTIONALINLINE bool for_each_in_rect_or_spread(CellStruct center, float widthOrRange, int height, Func&& action)
		{
			if (for_each_in_rect<T>(center, widthOrRange, height, std::forward<Func>(action)))
			{
				return true;
			}

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
		[[nodiscard]] OPTIONALINLINE COMPILETIMEEVAL bool is_any_of(Value&& value, Options&&... options) {
			return ((value == options) || ...);
		}

		OPTIONALINLINE void remove_non_paradroppables(std::vector<TechnoTypeClass*>& types,
									   const char* section, const char* key) {
			// Use std::erase_if (C++20) if available, otherwise use erase-remove
			std::erase_if(types, [section, key](TechnoTypeClass* pItem) -> bool {
				if (!pItem) {
					Debug::INIParseFailed(section, key, "nullptr", "Invalid types are removed.");
					return true;
				}

				if (!is_any_of(pItem->WhatAmI(), AbstractType::InfantryType, AbstractType::UnitType)) {
					Debug::INIParseFailed(section, key, pItem->ID, 
										 "Only InfantryTypes and UnitTypes are supported.");
					return true;
				}

				if (pItem->Strength <= 0) {
					Debug::INIParseFailed(section, key, pItem->ID, 
										 "0 Strength types are removed.");
					return true;
				}

				return false;
			});
		}

		// IMPROVED: Use std::partial_sort instead of custom selection sort
		template <typename FwdIt>
		OPTIONALINLINE COMPILETIMEEVAL void selectionsort(FwdIt first, FwdIt last) {
			// Use standard library's more efficient implementation
			std::sort(first, last);
		}

		template <typename FwdIt, typename Pred>
		OPTIONALINLINE COMPILETIMEEVAL void selectionsort(FwdIt first, FwdIt last, Pred pred) {
			std::sort(first, last, std::forward<Pred>(pred));
		}

		template <typename FwdIt>
		OPTIONALINLINE COMPILETIMEEVAL void selectionsort(FwdIt first, FwdIt middle, FwdIt last) {
			// Use std::partial_sort - much more efficient than selection sort
			std::partial_sort(first, middle, last);
		}

		template <typename FwdIt, typename Pred>
		OPTIONALINLINE COMPILETIMEEVAL void selectionsort(FwdIt first, FwdIt middle, FwdIt last, Pred pred) {
			std::partial_sort(first, middle, last, std::forward<Pred>(pred));
		}

		namespace ranges {
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
		const CoordStruct Where,
		TechnoClass* pFirer
		);
	}
};
