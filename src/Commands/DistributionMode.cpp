#include "DistributionMode.h"

#include <Ext/TechnoType/Body.h>
#include <Utilities/Helpers.Alex.h>
#include <Helpers/Macro.h>

#include <HouseClass.h>

bool DistributionModeHoldDownCommandClass::Enabled = false;
bool DistributionModeHoldDownCommandClass::OnMessageShowed = false;
bool DistributionModeHoldDownCommandClass::OffMessageShowed = false;
int DistributionModeHoldDownCommandClass::ShowTime = 0;

const char* DistributionModeSpreadCommandClass::GetName() const
{
	return "Distribution Mode Spread";
}

const wchar_t* DistributionModeSpreadCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DISTR_SPREAD", L"Distribution spread");
}

const wchar_t* DistributionModeSpreadCommandClass::GetUICategory() const
{
	return CATEGORY_CONTROL;
}

const wchar_t* DistributionModeSpreadCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DISTR_SPREAD_DESC", L"Automatically and averagely select similar targets around the original target. This is for changing the search range");
}

void DistributionModeSpreadCommandClass::Execute(WWKey eInput) const
{
	Phobos::Config::DistributionSpreadMode = ((Phobos::Config::DistributionSpreadMode + 1) & 3);
	DistributionModeHoldDownCommandClass::ShowTime = SystemTimer::GetTime();
}

const char* DistributionModeFilterCommandClass::GetName() const
{
	return "Distribution Mode Filter";
}

const wchar_t* DistributionModeFilterCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DISTR_FILTER", L"Distribution filter");
}

const wchar_t* DistributionModeFilterCommandClass::GetUICategory() const
{
	return CATEGORY_CONTROL;
}

const wchar_t* DistributionModeFilterCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DISTR_FILTER_DESC", L"Automatically and averagely select similar targets around the original target. This is for changing the filter criteria");
}

void DistributionModeFilterCommandClass::Execute(WWKey eInput) const
{
	Phobos::Config::DistributionFilterMode = ((Phobos::Config::DistributionFilterMode + 1) & 3);
	DistributionModeHoldDownCommandClass::ShowTime = SystemTimer::GetTime();
}

const char* DistributionModeHoldDownCommandClass::GetName() const
{
	return "Distribution Mode Hold Down";
}

const wchar_t* DistributionModeHoldDownCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DISTR_HOLDDOWN", L"Distribution hold down");
}

const wchar_t* DistributionModeHoldDownCommandClass::GetUICategory() const
{
	return CATEGORY_CONTROL;
}

const wchar_t* DistributionModeHoldDownCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DISTR_HOLDDOWN_DESC", L"Automatically and averagely select similar targets around the original target. This is for holding down to toggle on/off");
}

bool DistributionModeHoldDownCommandClass::ExtraTriggerCondition(WWKey eInput) const
{
	return true;
}

void DistributionModeHoldDownCommandClass::Execute(WWKey eInput) const
{
	if (eInput & WWKey::Release)
	{
		DistributionModeHoldDownCommandClass::Enabled = false;

		if (HouseClass::IsCurrentPlayerObserver() || SessionClass::Instance.MultiplayerObserver)
			return;

		VocClass::PlayGlobal(RulesExt::Global()->EndDistributionModeSound, 0x2000, 1.0);

		if (!DistributionModeHoldDownCommandClass::OffMessageShowed)
		{
			DistributionModeHoldDownCommandClass::OffMessageShowed = true;
			MessageListClass::Instance.PrintMessage(GeneralUtils::LoadStringUnlessMissing("MSG:DistributionModeOff", L"Distribution mode unabled."), RulesClass::Instance->MessageDelay, HouseClass::CurrentPlayer->ColorSchemeIndex, true);
		}
	}
	else if (!HouseClass::IsCurrentPlayerObserver() && !SessionClass::Instance.MultiplayerObserver)
	{
		DistributionModeHoldDownCommandClass::Enabled = true;
		VocClass::PlayGlobal(RulesExt::Global()->StartDistributionModeSound, 0x2000, 1.0);

		if (!DistributionModeHoldDownCommandClass::OnMessageShowed)
		{
			DistributionModeHoldDownCommandClass::OnMessageShowed = true;
			MessageListClass::Instance.PrintMessage(GeneralUtils::LoadStringUnlessMissing("MSG:DistributionModeOn", L"Distribution mode enabled."), RulesClass::Instance->MessageDelay, HouseClass::CurrentPlayer->ColorSchemeIndex, true);
		}
	}
}

void DistributionModeHoldDownCommandClass::DistributionSpreadModeExpand()
{
	Phobos::Config::DistributionSpreadMode = std::min(3, Phobos::Config::DistributionSpreadMode + 1);
}

void DistributionModeHoldDownCommandClass::DistributionSpreadModeReduce()
{
	Phobos::Config::DistributionSpreadMode = std::max(0, Phobos::Config::DistributionSpreadMode - 1);
}

DEFINE_HOOK(0x4AE818, DisplayClass_sub_4AE750_AutoDistribution, 0xA)
{
	enum { SkipGameCode = 0x4AE85C };

	GET(ObjectClass* const, pTarget, EBP);
	GET_STACK(const Action, mouseAction, STACK_OFFSET(0x20, 0xC));

	const auto count = ObjectClass::CurrentObjects.Count;

	if (count > 0)
	{
		const auto mode1 = Phobos::Config::DistributionSpreadMode;
		const auto mode2 = Phobos::Config::DistributionFilterMode;

		// Distribution mode main
		if (DistributionModeHoldDownCommandClass::Enabled && mode1 && count > 1 && mouseAction != Action::NoMove && !PlanningNodeClass::PlanningModeActive
			&& (pTarget->AbstractFlags & AbstractFlags::Techno) != AbstractFlags::None && !pTarget->IsInAir())
		{
			VocClass::PlayGlobal(RulesExt::Global()->AddDistributionModeCommandSound, 0x2000, 1.0);
			const auto pSpecial = HouseClass::FindSpecial();
			const auto pCivilian = HouseClass::FindCivilianSide();
			const auto pNeutral = HouseClass::FindNeutral();

			const auto pTargetHouse = static_cast<TechnoClass*>(pTarget)->Owner;
			const bool targetIsNeutral = pTargetHouse == pSpecial || pTargetHouse == pCivilian || pTargetHouse == pNeutral;

			const auto range = (2 << mode1);
			const auto center = pTarget->GetCoords();
			const auto pItems = Helpers::Alex::getCellSpreadItems(center, range);

			std::vector<std::pair<TechnoClass*, int>> record;
			const auto maxSize = pItems.size();
			record.reserve(maxSize);

			int current = 1;

			for (const auto& pItem : pItems)
			{
				if (!pItem->IsDisguisedAs(HouseClass::CurrentPlayer)
					&& (pItem->CloakState != CloakState::Cloaked || pItem->GetCell()->Sensors_InclHouse(HouseClass::CurrentPlayer->ArrayIndex)))
				{
					auto coords = pItem->GetCoords();

					if (!MapClass::Instance.IsWithinUsableArea(coords))
						continue;

					coords.Z = MapClass::Instance.GetCellFloorHeight(coords);

					if (MapClass::Instance.GetCellAt(coords)->ContainsBridge())
						coords.Z += CellClass::BridgeHeight;

					if (!MapClass::Instance.IsLocationShrouded(coords))
						 record.emplace_back(pItem, 0);
				}
			}

			const auto recordSize = record.size();
			std::sort(&record[0], &record[recordSize],[&center](const auto& pairA, const auto& pairB)
			{
				const auto coordsA = pairA.first->GetCoords();
				const auto distanceA = Point2D{coordsA.X, coordsA.Y}.DistanceFromSquared(Point2D{center.X, center.Y});
				const auto coordsB = pairB.first->GetCoords();
				const auto distanceB = Point2D{coordsB.X, coordsB.Y}.DistanceFromSquared(Point2D{center.X, center.Y});
				return distanceA < distanceB;
			});

			for (const auto& pSelect : ObjectClass::CurrentObjects)
			{
				size_t canTargetIndex = maxSize;
				size_t newTargetIndex = maxSize;

				for (size_t i = 0; i < recordSize; ++i)
				{
       				const auto& [pItem, num] = record[i];

					if (pSelect->MouseOverObject(pItem) == mouseAction
						&& (targetIsNeutral || (pItem->Owner != pSpecial && pItem->Owner != pCivilian && pItem->Owner != pNeutral))
						&& (mode2 < 2 || (pItem->WhatAmI() == pTarget->WhatAmI()
							&& (mode2 < 3 || TechnoTypeExt::GetSelectionGroupID(pItem->GetTechnoType()) == TechnoTypeExt::GetSelectionGroupID(pTarget->GetTechnoType())))))
					{
						canTargetIndex = i;

						if (num < current)
						{
							newTargetIndex = i;
							break;
						}
					}
				}

				if (newTargetIndex == maxSize && canTargetIndex != maxSize)
				{
					++current;
					newTargetIndex = canTargetIndex;
				}

				if (newTargetIndex != maxSize)
				{
					auto& [pNewTarget, recordCount] = record[newTargetIndex];

					++recordCount;
					pSelect->ObjectClickedAction(mouseAction, pNewTarget, false);
				}
				else
				{
					const auto currentAction = pSelect->MouseOverObject(pTarget);

					if (mode2 && currentAction == Action::NoMove && (pSelect->AbstractFlags & AbstractFlags::Techno) != AbstractFlags::None)
						static_cast<TechnoClass*>(pSelect)->ClickedMission(Mission::Area_Guard, reinterpret_cast<ObjectClass*>(pSelect->GetCellAgain()), nullptr, nullptr);
					else
						pSelect->ObjectClickedAction(currentAction, pTarget, false);
				}

				Unsorted::MoveFeedback = false;
			}
		}
		else // Vanilla
		{
			for (const auto& pSelect : ObjectClass::CurrentObjects)
			{
				const auto currentAction = pSelect->MouseOverObject(pTarget);

				if (mode2 && mouseAction != Action::NoMove && currentAction == Action::NoMove && (pSelect->AbstractFlags & AbstractFlags::Techno) != AbstractFlags::None)
					static_cast<TechnoClass*>(pSelect)->ClickedMission(Mission::Area_Guard, reinterpret_cast<ObjectClass*>(pSelect->GetCellAgain()), nullptr, nullptr);
				else
					pSelect->ObjectClickedAction(currentAction, pTarget, false);

				Unsorted::MoveFeedback = false;
			}
		}
	}

	return SkipGameCode;
}

DEFINE_HOOK(0x6DBE74, TacticalClass_DrawAllRadialIndicators_DrawDistributionRange, 0x7)
{
	if (!DistributionModeHoldDownCommandClass::Enabled && SystemTimer::GetTime() - DistributionModeHoldDownCommandClass::ShowTime > 30)
		return 0;

	const auto mode1 = Phobos::Config::DistributionSpreadMode;
	const auto mode2 = Phobos::Config::DistributionFilterMode;

	if (mode1 || mode2)
	{
		const auto pCell = MapClass::Instance.GetCellAt(DisplayClass::Instance.CurrentFoundation_CenterCell);
		const auto color = ((mode2 > 1)
			? ((mode2 == 3) ? ColorStruct { 255, 0, 0 } : ColorStruct { 200, 200, 0 })
			: (mode2 == 1) ? ColorStruct { 0, 100, 255 } : ColorStruct { 0, 255, 50 });
		Game::DrawRadialIndicator(false, true, pCell->GetCoords(), color, static_cast<float>(mode1 ? (2 << mode1) : 0.5), false, true);
	}

	return 0;
}
