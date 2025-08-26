#include "DistributionMode.h"

#include <Ext/Techno/Body.h>
#include <Utilities/Helpers.Alex.h>
#include <Helpers/Macro.h>

#include <HouseClass.h>

bool DistributionModeHoldDownCommandClass::Enabled = false;
bool DistributionModeHoldDownCommandClass::OnMessageShowed = false;
bool DistributionModeHoldDownCommandClass::OffMessageShowed = false;
int DistributionModeHoldDownCommandClass::ShowTime = 0;

const char* SwitchNoMoveCommandClass::GetName() const
{
	return "Switch No Move Command";
}

const wchar_t* SwitchNoMoveCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_SWITCH_NOMOVE", L"Switch no-move");
}

const wchar_t* SwitchNoMoveCommandClass::GetUICategory() const
{
	return CATEGORY_CONTROL;
}

const wchar_t* SwitchNoMoveCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_SWITCH_NOMOVE_DESC", L"Make unit does not move around the target when receiving no-move command");
}

void SwitchNoMoveCommandClass::Execute(WWKey eInput) const
{
	Phobos::Config::ApplyNoMoveCommand = !Phobos::Config::ApplyNoMoveCommand;
}

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

void __fastcall DistributionModeHoldDownCommandClass::ClickedWaypoint(ObjectClass* pSelect, int idxPath, signed char idxWP)
{
	pSelect->AssignPlanningPath(idxPath, idxWP);

	if (const auto pFoot = abstract_cast<FootClass*, true>(pSelect))
		pFoot->unknown_bool_430 = false;
}

void __fastcall DistributionModeHoldDownCommandClass::ClickedTargetAction(ObjectClass* pSelect, Action action, ObjectClass* pTarget)
{
	pSelect->ObjectClickedAction(action, pTarget, false);
	Unsorted::MoveFeedback = false;
}

void __fastcall DistributionModeHoldDownCommandClass::ClickedCellAction(ObjectClass* pSelect, Action action, CellStruct* pCell, CellStruct* pSecondCell)
{
	pSelect->CellClickedAction(action, pCell, pSecondCell, false);
	Unsorted::MoveFeedback = false;
}

void __fastcall DistributionModeHoldDownCommandClass::AreaGuardAction(TechnoClass* pTechno)
{
	pTechno->ClickedMission(Mission::Area_Guard, reinterpret_cast<ObjectClass*>(pTechno->GetCellAgain()), nullptr, nullptr);
	Unsorted::MoveFeedback = false;
}

DEFINE_HOOK(0x4AE7B3, DisplayClass_ActiveClickWith_Iterate, 0x0)
{
	enum { SkipGameCode = 0x4AE99B };

	const int count = ObjectClass::CurrentObjects.Count;

	if (count > 0)
	{
		{
			GET_STACK(int, idxPath, STACK_OFFSET(0x18, -0x8));
			GET_STACK(unsigned char, idxWP, STACK_OFFSET(0x18, -0xC));

			for (const auto& pSelect : ObjectClass::CurrentObjects)
			{
				DistributionModeHoldDownCommandClass::ClickedWaypoint(pSelect, idxPath, idxWP);
			}
		}

		GET_STACK(ObjectClass* const, pTarget, STACK_OFFSET(0x18, 0x4));
		GET_STACK(Action const, action, STACK_OFFSET(0x18, 0xC));

		if (pTarget)
		{
			const int spreadMode = Phobos::Config::DistributionSpreadMode;
			const int filterMode = Phobos::Config::DistributionFilterMode;
			const bool noMove = !Phobos::Config::ApplyNoMoveCommand;
			const auto pTechno = abstract_cast<TechnoClass*, true>(pTarget);

			// Distribution mode main
			if (DistributionModeHoldDownCommandClass::Enabled
				&& spreadMode
				&& count > 1
				&& action != Action::NoMove
				&& !PlanningNodeClass::PlanningModeActive
				&& pTechno
				&& !pTechno->IsInAir()
				&& (HouseClass::CurrentPlayer->IsAlliedWith(pTechno->Owner)
					? Phobos::Config::AllowDistributionCommand_AffectsAllies
					: Phobos::Config::AllowDistributionCommand_AffectsEnemies))
			{
				VocClass::PlayGlobal(RulesExt::Global()->AddDistributionModeCommandSound, 0x2000, 1.0);
				const bool targetIsNeutral = pTechno->Owner->IsNeutral();
				const auto pType = pTechno->GetTechnoType();
				const int range = (2 << spreadMode);
				const auto center = pTechno->GetCoords();
				const auto pItems = Helpers::Alex::getCellSpreadItems(center, range);

				std::vector<std::pair<TechnoClass*, int>> record;
				const size_t maxSize = pItems.size();
				record.reserve(maxSize);

				int current = 1;

				for (const auto& pItem : pItems)
				{
					if (pItem->IsDisguisedAs(HouseClass::CurrentPlayer))
						continue;

					if (pItem->CloakState == CloakState::Cloaked && !pItem->GetCell()->Sensors_InclHouse(HouseClass::CurrentPlayer->ArrayIndex))
						continue;

					auto coords = pItem->GetCoords();

					if (!MapClass::Instance.IsWithinUsableArea(coords))
						continue;

					coords.Z = MapClass::Instance.GetCellFloorHeight(coords);

					if (MapClass::Instance.GetCellAt(coords)->ContainsBridge())
						coords.Z += CellClass::BridgeHeight;

					if (!MapClass::Instance.IsLocationShrouded(coords))
						record.emplace_back(pItem, 0);
				}

				const size_t recordSize = record.size();
				std::sort(&record[0], &record[recordSize],[&center](const auto& pairA, const auto& pairB)
					{
						const auto coordsA = pairA.first->GetCoords();
						const double distanceA = Point2D{coordsA.X, coordsA.Y}.DistanceFromSquared(Point2D{center.X, center.Y});

						const auto coordsB = pairB.first->GetCoords();
						const double distanceB = Point2D{coordsB.X, coordsB.Y}.DistanceFromSquared(Point2D{center.X, center.Y});

						return distanceA < distanceB;
					});

				for (const auto& pSelect : ObjectClass::CurrentObjects)
				{
					size_t canTargetIndex = maxSize;
					size_t newTargetIndex = maxSize;

					for (size_t i = 0; i < recordSize; ++i)
					{
						const auto& [pItem, num] = record[i];

						if (pSelect->MouseOverObject(pItem) != action)
							continue;

						if (!targetIsNeutral && pItem->Owner->IsNeutral())
							continue;

						if (filterMode)
						{
							const auto pItemType = pItem->GetTechnoType();

							if (!pItemType)
								continue;

							if (TechnoTypeExt::ExtMap.Find(pType)->FakeOf != pItemType
								&& TechnoTypeExt::ExtMap.Find(pItemType)->FakeOf != pType)
							{
								if (filterMode == 1)
								{
									if (pItemType->Armor != pType->Armor)
										continue;
								}
								else if (filterMode == 2)
								{
									if (pItem->WhatAmI() != pTechno->WhatAmI())
										continue;
								}
								else // filterMode == 3
								{
									if (TechnoTypeExt::GetSelectionGroupID(pItemType) != TechnoTypeExt::GetSelectionGroupID(pType))
										continue;
								}
							}
						}

						canTargetIndex = i;

						if (num < current)
						{
							newTargetIndex = i;
							break;
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

						DistributionModeHoldDownCommandClass::ClickedTargetAction(pSelect, action, pNewTarget);

						++recordCount;
						continue;
					}

					const auto currentAction = pSelect->MouseOverObject(pTechno);

					if (noMove && currentAction == Action::NoMove && (pSelect->AbstractFlags & AbstractFlags::Techno) != AbstractFlags::None)
						DistributionModeHoldDownCommandClass::AreaGuardAction(static_cast<TechnoClass*>(pSelect));
					else
						DistributionModeHoldDownCommandClass::ClickedTargetAction(pSelect, currentAction, pTechno);
				}
			}
			else
			{
				for (const auto& pSelect : ObjectClass::CurrentObjects)
				{
					const auto currentAction = pSelect->MouseOverObject(pTarget);

					if (noMove && action != Action::NoMove && currentAction == Action::NoMove && (pSelect->AbstractFlags & AbstractFlags::Techno) != AbstractFlags::None)
						DistributionModeHoldDownCommandClass::AreaGuardAction(static_cast<TechnoClass*>(pSelect));
					else
						DistributionModeHoldDownCommandClass::ClickedTargetAction(pSelect, currentAction, pTarget);
				}
			}
		}
		else
		{
			LEA_STACK(CellStruct* const, pCell, STACK_OFFSET(0x18, 0x8));

			auto invalidCell = CellStruct { -1, -1 };
			auto pSecondCell = action == Action::Move || action == Action::PatrolWaypoint || action == Action::NoMove ? pCell : &invalidCell;

			for (const auto& pSelect : ObjectClass::CurrentObjects)
			{
				const auto currentAction = pSelect->MouseOverCell(pCell, false, false);

				DistributionModeHoldDownCommandClass::ClickedCellAction(pSelect, currentAction, pCell, pSecondCell);
			}
		}
	}

	Unsorted::MoveFeedback = true;

	return SkipGameCode;
}

DEFINE_HOOK(0x6DBE74, TacticalClass_DrawAllRadialIndicators_DrawDistributionRange, 0x7)
{
	if (!DistributionModeHoldDownCommandClass::Enabled && SystemTimer::GetTime() - DistributionModeHoldDownCommandClass::ShowTime > 30)
		return 0;

	const auto spreadMode = Phobos::Config::DistributionSpreadMode;
	const auto filterMode = Phobos::Config::DistributionFilterMode;

	if (spreadMode || filterMode)
	{
		const auto pCell = MapClass::Instance.GetCellAt(DisplayClass::Instance.CurrentFoundation_CenterCell);
		const auto color = (filterMode > 1)
			? ((filterMode == 3) ? ColorStruct { 255, 0, 0 } : ColorStruct { 200, 200, 0 })
			: ((filterMode == 1) ? ColorStruct { 0, 100, 255 } : ColorStruct { 0, 255, 50 });
		Game::DrawRadialIndicator(false, true, pCell->GetCoords(), color, static_cast<float>(spreadMode ? (2 << spreadMode) : 0.5), false, true);
	}

	return 0;
}
