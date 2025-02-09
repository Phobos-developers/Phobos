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

#include <AircraftTrackerClass.h>
#include <BuildingClass.h>
#include <BuildingTypeClass.h>
#include <CellSpread.h>
#include <Unsorted.h>
#include <Helpers/Iterators.h>
#include <Helpers/Enumerators.h>

#include <set>
#include <functional>
#include <algorithm>
#include <iterator>
#include <vector>

namespace Helpers
{

	namespace Alex
	{
		//! Represents a set of unique items.
		template<typename T>
		using DistinctCollector = std::set<T, decltype([](T a, T b)
			{
				if constexpr (std::is_pointer_v<std::remove_cv_t<T>>)
					return *a < *b;
				else
					return a < b;
			}) > ;

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
		inline int getCappedDuration(int CurrentValue, int Duration, int Cap)
		{
			// Usually, the new duration is just added.
			int ProposedDuration = CurrentValue + Duration;

			if (Duration > 0)
			{
				// Positive damage.
				if (Cap < 0)
				{
					// Do not stack. Use the maximum value.
					return std::max(Duration, CurrentValue);
				}
				else if (Cap > 0)
				{
					// Cap the duration.
					int cappedValue = std::min(ProposedDuration, Cap);
					return std::max(CurrentValue, cappedValue);
				}
				else
				{
					// There is no cap. Allow the duration to stack up.
					return ProposedDuration;
				}
			}
			else
			{
				// Negative damage.
				return (Cap < 0 ? ProposedDuration : std::min(ProposedDuration, Cap));
			}
		}

		//! Gets a list of all units in range of a cell spread weapon.
		/*!
			CellSpread is handled as described in
			http://modenc.renegadeprojects.com/CellSpread.

			\param coords The location the projectile detonated.
			\param spread The range to find items in.
			\param includeInAir Include items that are currently InAir.

			\author AlexB
			\date 2010-06-28

			\modifications by Starkku
			\date 2024-05-20
			\modifications by Trsdy
			\date 2024-12-05
		*/
		inline DistinctCollector<TechnoClass*> getCellSpreadItems(
			CoordStruct const& coords, double const spread,
			bool const includeInAir = false)
		{
			// set of possibly affected objects. every object can be here only once.
			DistinctCollector<TechnoClass*> set;
			double const spreadMult = spread * Unsorted::LeptonsPerCell;

			// the quick way. only look at stuff residing on the very cells we are affecting.
			auto const cellCoords = MapClass::Instance->GetCellAt(coords)->MapCoords;
			auto const range = static_cast<size_t>(spread + 0.99);
			for (CellSpreadEnumerator it(range); it; ++it)
			{
				auto const pCell = MapClass::Instance->TryGetCellAt(*it + cellCoords);
				if (!pCell)continue;
				bool isCenter = pCell->MapCoords == cellCoords;
				for (NextObject obj(pCell->GetContent()); obj; ++obj)
				{
					if (auto const pTechno = abstract_cast<TechnoClass*>(*obj))
					{
						// Starkku: Buildings need their distance from the origin coords checked at cell level.
						if (pTechno->WhatAmI() == AbstractType::Building)
						{
							if (static_cast<BuildingClass*>(pTechno)->Type->InvisibleInGame)
								continue;
							auto const cellCenterCoords = pCell->GetCenterCoords();
							double dist = cellCenterCoords.DistanceFrom(coords);

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

			// flying objects are not included normally
			// Starkku: Reimplemented using AircraftTrackerClass.
			if (includeInAir)
			{
				auto const airTracker = &AircraftTrackerClass::Instance;
				airTracker->FillCurrentVector(MapClass::Instance->GetCellAt(coords), Game::F2I(spread));

				for (auto pTechno = airTracker->Get(); pTechno; pTechno = airTracker->Get())
				{
					if (pTechno->IsAlive && pTechno->IsOnMap && pTechno->Health > 0)
					{
						auto dist = pTechno->Location.DistanceFrom(coords);
						// reduce the distance for flying aircraft
						if (pTechno->WhatAmI() == AbstractType::Aircraft)
							dist *= 0.5;
						if (dist <= spreadMult)
							set.insert(pTechno);
					}
				}
			}

			return set;
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
		inline bool for_each_in_rect(
			CellStruct const center, float widthOrRange, int height, Func&& action)
		{
			if (height > 0)
			{
				auto const width = static_cast<int>(widthOrRange);

				if (width > 0)
				{
					// the coords mark the center of the area
					auto topleft = center;
					topleft.X -= static_cast<short>(width / 2);
					topleft.Y -= static_cast<short>(height / 2);

					auto const rect = LTRBStruct {
						topleft.X, topleft.Y, topleft.X + width, topleft.Y + height };

					CellRectIterator<T>{}(rect, std::forward<Func>(action));
					return true;
				}
			}

			return false;
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
		inline bool for_each_in_rect_or_range(CellStruct center, float widthOrRange, int height, Func&& action)
		{
			if (for_each_in_rect<T>(center, widthOrRange, height, action))
			{
				return true;
			}

			if (height <= 0 && widthOrRange >= 0.0)
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
		inline bool for_each_in_rect_or_spread(CellStruct center, float widthOrRange, int height, Func&& action)
		{
			if (for_each_in_rect<T>(center, widthOrRange, height, action))
			{
				return true;
			}

			if (height <= 0)
			{
				auto const spread = static_cast<size_t>(
					Math::max(static_cast<int>(widthOrRange), 0));

				if (spread > 0)
				{
					CellSpreadIterator<T>{}(center, spread, std::forward<Func>(action));
					return true;
				}
			}

			return false;
		}

		template <typename Value, typename Option>
		constexpr bool is_any_of(Value&& value, Option&& option)
		{
			return value == option;
		}

		template <typename Value, typename Option, typename... Options>
		constexpr bool is_any_of(Value&& value, Option&& first_option, Options&&... other_options)
		{
			return value == first_option || is_any_of(std::forward<Value>(value), std::forward<Options>(other_options)...);
		}

		inline void remove_non_paradroppables(std::vector<TechnoTypeClass*>& types, const char* section, const char* key)
		{
			// remove all types that aren't either infantry or unit types
			types.erase(std::remove_if(types.begin(), types.end(), [section, key](TechnoTypeClass* pItem) -> bool
				{
					if (!is_any_of(pItem->WhatAmI(), AbstractType::InfantryType, AbstractType::UnitType))
					{
						Debug::INIParseFailed(section, key, pItem->ID, "Only InfantryTypes and UnitTypes are supported.");
						return true;
					}
					return false;
				}), types.end());
		}

		//! Invokes an action for every element that suffices a predicate.
		/*!
			Complexity: linear. distance(first, last) invocations of pred,
			up to distance(first, last) invocations of func.

			\param first Input iterator to be beginning.
			\param last Input iterator to the last.
			\param pred The predicate to decide whether to call func with an element.
			\param func The action to invoke for each element that suffices pred.

			\author AlexB
			\date 2014-08-27
		*/
		template <typename InIt, typename Pred, typename Fn>
		inline void for_each_if(InIt first, InIt last, Pred pred, Fn func)
		{
			first = std::find_if(first, last, pred);

			while (first != last)
			{
				func(*first++);

				first = std::find_if(first, last, pred);
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
		inline void for_each_if_n(InIt first, InIt last, size_t count, Pred pred, Fn func)
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

		//! Stable (partial) selection sort using a predicate or std::less
		/*!
			For the overloads not taking middle, assume middle equals last. For
			the overloads not taking pred, assume pred equals std::less<>.

			After the function returns, std::is_sorted(first, middle, pred)
			evaluates to true, and [middle, last) contains all the elements
			that don't compare less than the element at middle according to
			pred, in unspecified order.

			Complexity: quadratic. Less than distance(first, last) *
			distance(first, middle) invocations of pred, up to
			max(distance(first, middle) - 1, 0) swaps.

			\param first Forward iterator to be beginning.
			\param middle Forward iterator to be end of the sorted range.
			\param last Forward iterator to the end.
			\param pred The predicate to compare two elements.

			\author AlexB
			\date 2015-08-11
		*/
		template <typename FwdIt>
		inline void selectionsort(FwdIt first, FwdIt last)
		{
			// this is a special case of a full partial sort
			selectionsort(first, last, last);
		}

		template <typename FwdIt, typename Pred>
		inline void selectionsort(FwdIt first, FwdIt last, Pred pred)
		{
			// this is a special case of a full partial sort
			selectionsort(first, last, last, pred);
		}

		template <typename FwdIt>
		inline void selectionsort(FwdIt first, FwdIt middle, FwdIt last)
		{
			selectionsort(first, middle, last, std::less<>());
		}

		template <typename FwdIt, typename Pred>
		inline void selectionsort(FwdIt first, FwdIt middle, FwdIt last, Pred pred)
		{
			while (first != middle)
			{
				auto const it = std::min_element(first, last, pred);
				std::iter_swap(first, it);
				++first;
			}
		}
	};
};
