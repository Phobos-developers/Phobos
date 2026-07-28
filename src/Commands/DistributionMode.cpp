#include "DistributionMode.h"

#include "AdvancedCommandBarButtons.h"
#include <Ext/Techno/Body.h>
#include <Utilities/Helpers.Alex.h>
#include <Helpers/Macro.h>

#include <HouseClass.h>
#include <TacticalClass.h>
#include <ShapeButtonClass.h>

bool DistributionModeHoldDownCommandClass::Enabled = false;
bool DistributionModeHoldDownCommandClass::OnMessageShowed = false;
bool DistributionModeHoldDownCommandClass::OffMessageShowed = false;
int DistributionModeHoldDownCommandClass::ShowTime = 0;

bool DistributionModeHoldDownCommandClass::IsDragDistributing = false;
CoordStruct DistributionModeHoldDownCommandClass::DragStartCenter = {};
DistributionTargetInfo DistributionModeHoldDownCommandClass::DragInfo = {};

namespace
{
	constexpr unsigned int DistributionSpreadRangeMax = 5120;     // 20 cells (x LeptonsPerCell)
	constexpr unsigned int DistributionSpreadScrollStepMin = 16;
	constexpr unsigned int DistributionSpreadHotkeySteps[] = { 0, 1024, 2048, 4096 };
	constexpr size_t DistributionSpreadHotkeyStepsCount = std::size(DistributionSpreadHotkeySteps);
}

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
	return GeneralUtils::LoadStringUnlessMissing("TXT_DISTR_SPREAD", L"Change distribution range");
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
	auto& range = Phobos::Config::DistributionSpreadRange;

	// Cycle to the next hotkey step strictly greater than the current range.
	for (size_t i = 0; i < DistributionSpreadHotkeyStepsCount; ++i)
	{
		if (DistributionSpreadHotkeySteps[i] > range)
		{
			range = DistributionSpreadHotkeySteps[i];
			DistributionModeHoldDownCommandClass::ShowTime = SystemTimer::GetTime();
			return;
		}
	}

	// Wrap around to the first step (0 = no distribution).
	range = DistributionSpreadHotkeySteps[0];
	DistributionModeHoldDownCommandClass::ShowTime = SystemTimer::GetTime();
}

const char* DistributionModeFilterCommandClass::GetName() const
{
	return "Distribution Mode Filter";
}

const wchar_t* DistributionModeFilterCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DISTR_FILTER", L"Change distribution filter");
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

bool DistributionModeHoldDownCommandClass::PreventCombinationOverride(WWKey eInput) const
{
	return (eInput & WWKey::Shift | WWKey::Ctrl | WWKey::Alt) != WWKey(0);
}

bool DistributionModeHoldDownCommandClass::ExtraTriggerCondition(WWKey eInput) const
{
	return true;
}

void DistributionModeHoldDownCommandClass::Execute(WWKey eInput) const
{
	if (eInput & WWKey::Release)
		DistributionModeHoldDownCommandClass::DistributionModeOff();
	else
		DistributionModeHoldDownCommandClass::DistributionModeOn();
}

void DistributionModeHoldDownCommandClass::DistributionModeOn()
{
	if (DistributionModeHoldDownCommandClass::Enabled)
		return;

	if (SessionClass::Instance.MultiplayerObserver)
		return;

	DistributionModeHoldDownCommandClass::Enabled = true;

	if (const auto pButton = AdvancedCommandBarButton::GetShapeButton("DistributionMode"))
	{
		if (!pButton->IsOn)
			pButton->TurnOn();
	}

	VocClass::PlayGlobal(RulesExt::Global()->StartDistributionModeSound, 0x2000, 1.0);

	if (!DistributionModeHoldDownCommandClass::OnMessageShowed)
	{
		DistributionModeHoldDownCommandClass::OnMessageShowed = true;
		MessageListClass::Instance.PrintMessage(GeneralUtils::LoadStringUnlessMissing("MSG:DistributionModeOn", L"Distribution mode enabled."), RulesClass::Instance->MessageDelay, HouseClass::CurrentPlayer->ColorSchemeIndex, true);
	}
}

void DistributionModeHoldDownCommandClass::DistributionModeOff()
{
	if (!DistributionModeHoldDownCommandClass::Enabled)
		return;

	DistributionModeHoldDownCommandClass::Enabled = false;

	if (const auto pButton = AdvancedCommandBarButton::GetShapeButton("DistributionMode"))
	{
		if (pButton->IsOn)
			pButton->TurnOff();
	}

	if (SessionClass::Instance.MultiplayerObserver)
		return;

	VocClass::PlayGlobal(RulesExt::Global()->EndDistributionModeSound, 0x2000, 1.0);

	if (!DistributionModeHoldDownCommandClass::OffMessageShowed)
	{
		DistributionModeHoldDownCommandClass::OffMessageShowed = true;
		MessageListClass::Instance.PrintMessage(GeneralUtils::LoadStringUnlessMissing("MSG:DistributionModeOff", L"Distribution mode disabled."), RulesClass::Instance->MessageDelay, HouseClass::CurrentPlayer->ColorSchemeIndex, true);
	}
}

void DistributionModeHoldDownCommandClass::DistributionSpreadModeExpand()
{
	auto& range = Phobos::Config::DistributionSpreadRange;
	const auto step = std::max(DistributionSpreadScrollStepMin, Phobos::Config::DistributionSpreadScrollStep);
	range = (range >= DistributionSpreadRangeMax - step) ? DistributionSpreadRangeMax : range + step;
	DistributionModeHoldDownCommandClass::ShowTime = SystemTimer::GetTime();
}

void DistributionModeHoldDownCommandClass::DistributionSpreadModeReduce()
{
	auto& range = Phobos::Config::DistributionSpreadRange;
	const auto step = std::max(DistributionSpreadScrollStepMin, Phobos::Config::DistributionSpreadScrollStep);
	range = (range <= step) ? 0 : range - step;
	DistributionModeHoldDownCommandClass::ShowTime = SystemTimer::GetTime();
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

void DistributionModeHoldDownCommandClass::ProcessWaypointCommand(int idxPath, unsigned char idxWP)
{
	for (const auto& pSelect : ObjectClass::CurrentObjects)
		ClickedWaypoint(pSelect, idxPath, idxWP);
}

bool grinderCheck(Action action, TechnoClass* pTechno)
{
		if (!pTechno)
			return true;

		// Yeah, they use Action::Repair for grinder.
		// Sounds like some black humor.
		if (action != Action::Repair)
			return true;

		auto pItemType = pTechno->GetTechnoType();
		
		if (auto pBuildingItemType = abstract_cast<BuildingTypeClass*>(pItemType))
		{
			if (pBuildingItemType->Grinding)
				return false;
		}
		return true;
}

bool DistributionModeHoldDownCommandClass::IsDistributionModeEligible(unsigned int range, int count, Action action, TechnoClass* pTechno)
{
	return Enabled
		&& range > 0
		&& count > 1
		&& action != Action::NoMove
		&& !PlanningNodeClass::PlanningModeActive
		&& pTechno
		&& !pTechno->IsInAir()
		&& grinderCheck(action, pTechno)
		&& (pTechno->Owner->IsNeutral() ? Phobos::Config::AllowDistributionCommand_AffectsNeutral :
			(HouseClass::CurrentPlayer->IsAlliedWith(pTechno->Owner)
				? (Phobos::Config::AllowDistributionCommand_AffectsAllies && action != Action::Attack)
				: Phobos::Config::AllowDistributionCommand_AffectsEnemies));
}

std::vector<std::pair<TechnoClass*, int>> DistributionModeHoldDownCommandClass::CollectAndSortTargets(CoordStruct center, double range)
{
	const auto pItems = Helpers::Alex::getCellSpreadItems(center, range);

	std::vector<std::pair<TechnoClass*, int>> record;
	record.reserve(pItems.size());

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

	std::sort(record.begin(), record.end(), [&center](const auto& pairA, const auto& pairB)
		{
			const auto coordsA = pairA.first->GetCoords();
			const double distanceA = Point2D{coordsA.X, coordsA.Y}.DistanceFromSquared(Point2D{center.X, center.Y});

			const auto coordsB = pairB.first->GetCoords();
			const double distanceB = Point2D{coordsB.X, coordsB.Y}.DistanceFromSquared(Point2D{center.X, center.Y});

			return distanceA < distanceB;
		});

	return record;
}

DistributionTargetInfo DistributionModeHoldDownCommandClass::CollectTargetInfo(TechnoClass* pTechno, Action action)
{
	DistributionTargetInfo info;
	info.Center = pTechno->GetCoords();
	info.TargetIsNeutral = pTechno->Owner->IsNeutral();
	info.pType = pTechno->GetTechnoType();
	info.WhatAmI = pTechno->WhatAmI();
	info.Action = action;
	return info;
}

void DistributionModeHoldDownCommandClass::ProcessDistributionMode(const DistributionTargetInfo& info, ObjectClass* pTarget, int filterMode, bool noMove)
{
	VocClass::PlayGlobal(RulesExt::Global()->AddDistributionModeCommandSound, 0x2000, 1.0);
	const auto spreadRange = Phobos::Config::DistributionSpreadRange;
	auto record = CollectAndSortTargets(info.Center, (double)spreadRange / Unsorted::LeptonsPerCell);

	const size_t recordSize = record.size();
	const size_t maxSize = recordSize;
	int current = 1;

	for (const auto& pSelect : ObjectClass::CurrentObjects)
	{
		size_t canTargetIndex = maxSize;
		size_t newTargetIndex = maxSize;

		for (size_t i = 0; i < recordSize; ++i)
		{
			const auto& [pItem, num] = record[i];

			if (pSelect->MouseOverObject(pItem) != info.Action)
				continue;

			if (!info.TargetIsNeutral && pItem->Owner->IsNeutral())
				continue;

			if (!grinderCheck(info.Action, pItem))
				continue;

			if (filterMode)
			{
				const auto pItemType = pItem->GetTechnoType();

				if (!pItemType)
					continue;

				if (TechnoTypeExt::ExtMap.Find(info.pType)->FakeOf != pItemType
					&& TechnoTypeExt::ExtMap.Find(pItemType)->FakeOf != info.pType)
				{
					if (filterMode == 1)
					{
						if (pItemType->Armor != info.pType->Armor)
							continue;
					}
					else if (filterMode == 2)
					{
						if (pItem->WhatAmI() != info.WhatAmI)
							continue;
					}
					else // filterMode == 3
					{
						if (TechnoTypeExt::GetSelectionGroupID(pItemType) != TechnoTypeExt::GetSelectionGroupID(info.pType))
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

			ClickedTargetAction(pSelect, info.Action, pNewTarget);

			++recordCount;
			continue;
		}

		const auto currentAction = pTarget ? pSelect->MouseOverObject(pTarget) : Action::NoMove;

		if (noMove && currentAction == Action::NoMove && (pSelect->AbstractFlags & AbstractFlags::Techno) != AbstractFlags::None)
			AreaGuardAction(static_cast<TechnoClass*>(pSelect));
		else if (pTarget)
			ClickedTargetAction(pSelect, currentAction, pTarget);
	}
}

void DistributionModeHoldDownCommandClass::ProcessNormalTargetClick(ObjectClass* pTarget, Action action, bool noMove)
{
	for (const auto& pSelect : ObjectClass::CurrentObjects)
	{
		const auto currentAction = pSelect->MouseOverObject(pTarget);

		if (noMove && action != Action::NoMove && currentAction == Action::NoMove && (pSelect->AbstractFlags & AbstractFlags::Techno) != AbstractFlags::None)
			AreaGuardAction(static_cast<TechnoClass*>(pSelect));
		else
			ClickedTargetAction(pSelect, currentAction, pTarget);
	}
}

void DistributionModeHoldDownCommandClass::ProcessCellClick(CellStruct* pCell, Action action)
{
	auto invalidCell = CellStruct { -1, -1 };
	auto pSecondCell = action == Action::Move || action == Action::PatrolWaypoint || action == Action::NoMove ? pCell : &invalidCell;

	for (const auto& pSelect : ObjectClass::CurrentObjects)
	{
		const auto currentAction = pSelect->MouseOverCell(pCell, false, false);

		ClickedCellAction(pSelect, currentAction, pCell, pSecondCell);
	}
}

DEFINE_HOOK(0x4AE7B3, DisplayClass_ActiveClickWith_Iterate, 0x0)
{
	enum { SkipGameCode = 0x4AE99B };

	const int count = ObjectClass::CurrentObjects.Count;

	if (!Phobos::Config::AllowDistributionCommand_UseClick)
		return 0;

	if (count > 0)
	{
		GET_STACK(int, idxPath, STACK_OFFSET(0x18, -0x8));
		GET_STACK(unsigned char, idxWP, STACK_OFFSET(0x18, -0xC));
		DistributionModeHoldDownCommandClass::ProcessWaypointCommand(idxPath, idxWP);

		GET_STACK(ObjectClass* const, pTarget, STACK_OFFSET(0x18, 0x4));
		GET_STACK(Action const, action, STACK_OFFSET(0x18, 0xC));

		if (pTarget)
		{
			const bool noMove = !Phobos::Config::ApplyNoMoveCommand;
			const auto pTechno = abstract_cast<TechnoClass*, true>(pTarget);

			if (DistributionModeHoldDownCommandClass::IsDistributionModeEligible(Phobos::Config::DistributionSpreadRange, count, action, pTechno))
			{
				auto info = DistributionModeHoldDownCommandClass::CollectTargetInfo(pTechno, action);
				DistributionModeHoldDownCommandClass::ProcessDistributionMode(info, pTarget, Phobos::Config::DistributionFilterMode, noMove);
			}
			else
				DistributionModeHoldDownCommandClass::ProcessNormalTargetClick(pTarget, action, noMove);
		}
		else // Vanilla
		{
			LEA_STACK(CellStruct* const, pCell, STACK_OFFSET(0x18, 0x8));
			DistributionModeHoldDownCommandClass::ProcessCellClick(pCell, action);
		}
	}

	Unsorted::MoveFeedback = true;

	return SkipGameCode;
}

DEFINE_HOOK(0x6DBE74, TacticalClass_DrawAllRadialIndicators_DrawDistributionRange, 0x7)
{
	if (!DistributionModeHoldDownCommandClass::IsDragDistributing
		&& (!Phobos::Config::AllowDistributionCommand_UseClick
			|| (!DistributionModeHoldDownCommandClass::Enabled && SystemTimer::GetTime() - DistributionModeHoldDownCommandClass::ShowTime > 30)))
	{
		return 0;
	}

	const double spreadRange = Phobos::Config::DistributionSpreadRange * Unsorted::CellWidthInPixels / Unsorted::LeptonsPerCell / Math::Sqrt2;
	const int filterMode = Phobos::Config::DistributionFilterMode;

	if (spreadRange || filterMode)
	{
		const auto center = DistributionModeHoldDownCommandClass::IsDragDistributing
			? DistributionModeHoldDownCommandClass::DragStartCenter
			: MapClass::Instance.GetCellAt(DisplayClass::Instance.CurrentFoundation_CenterCell)->GetCoords();
		const auto color = (filterMode > 1)
			? ((filterMode == 3) ? ColorStruct { 255, 0, 0 } : ColorStruct { 200, 200, 0 })
			: ((filterMode == 1) ? ColorStruct { 0, 100, 255 } : ColorStruct { 0, 255, 50 });
		Game::DrawRadialIndicator(false, true, center, color, (float)spreadRange, true, true);
	}

	return 0;
}

const char* DistributionModeHoldDownButtonClass::GetName() const
{
	return "DistributionMode";
}

const char* DistributionModeHoldDownButtonClass::GetTipName() const
{
	return "Tip:DistributionMode";
}

bool DistributionModeHoldDownButtonClass::CanHoldDown() const
{
	return true;
}

void DistributionModeHoldDownButtonClass::Execute(bool isOn) const
{
	if (isOn)
		DistributionModeHoldDownCommandClass::DistributionModeOn();
	else
		DistributionModeHoldDownCommandClass::DistributionModeOff();
}

DEFINE_HOOK(0x4AC4B9, DisplayClass_LeftPressAndDragging_DistributionDragStart, 0x6)
{
	enum { SkipGameCode = 0x4AC4DF };

	if (!DistributionModeHoldDownCommandClass::Enabled || !Phobos::Config::AllowDistributionCommand_SpreadModeDrag)
		return 0;

	const int count = ObjectClass::CurrentObjects.Count;

	if (count <= 1 || Phobos::Config::DistributionSpreadRange == 0)
		return 0;

	Point2D screenPos = DisplayClass::Instance.LeftDownPosition;
	CellStruct cell;
	CoordStruct coords;
	ObjectClass* pTarget = nullptr;
	BYTE a5 = 0, a6 = 0;

	if (!DisplayClass::Instance.ProcessClickCoords(&screenPos, &cell, &coords, &pTarget, &a5, &a6))
		return 0;

	const auto pTechno = abstract_cast<TechnoClass*>(pTarget);

	if (!pTechno)
		return 0;

	const auto action = DisplayClass::Instance.DecideAction(cell, pTarget, 0);

	if (!DistributionModeHoldDownCommandClass::IsDistributionModeEligible(
		Phobos::Config::DistributionSpreadRange, count, action, pTechno))
		return 0;

	DistributionModeHoldDownCommandClass::DragInfo =
		DistributionModeHoldDownCommandClass::CollectTargetInfo(pTechno, action);
	DistributionModeHoldDownCommandClass::DragStartCenter = coords;
	DistributionModeHoldDownCommandClass::IsDragDistributing = true;

	return SkipGameCode;
}

DEFINE_HOOK(0x4AC411, DisplayClass_LeftPressAndDragging_DistributionDragUpdate, 0x6)
{
	enum { SkipGameCode = 0x4AC42E };

	if (!DistributionModeHoldDownCommandClass::IsDragDistributing)
		return 0;

	GET_STACK(Point2D, currentScreen, STACK_OFFSET(0x18, -0x10));

	const auto currentWorld = TacticalClass::Instance->ClientToCoords(currentScreen);
	const auto diff = Point2D{
		currentWorld.X - DistributionModeHoldDownCommandClass::DragStartCenter.X,
		currentWorld.Y - DistributionModeHoldDownCommandClass::DragStartCenter.Y
	};
	const auto distance = static_cast<unsigned int>(diff.DistanceFrom({}));

	Phobos::Config::DistributionSpreadRange = std::min(distance, DistributionSpreadRangeMax);
	DistributionModeHoldDownCommandClass::ShowTime = SystemTimer::GetTime();

	return SkipGameCode;
}

DEFINE_HOOK(0x4ABCA7, DisplayClass_LeftPressAndDragging_DistributionDragEnd, 0x6)
{
	enum { SkipGameCode = 0x4ABD07 };

	if (!DistributionModeHoldDownCommandClass::IsDragDistributing)
		return 0;

	const bool noMove = !Phobos::Config::ApplyNoMoveCommand;
	DistributionModeHoldDownCommandClass::ProcessDistributionMode(
		DistributionModeHoldDownCommandClass::DragInfo, nullptr,
		Phobos::Config::DistributionFilterMode, noMove);

	DisplayClass::Instance.LeftPressAndDraggingRectangle = false;
	DistributionModeHoldDownCommandClass::IsDragDistributing = false;
	TacticalClass::StartDrawActionLineTimer();

	return SkipGameCode;
}

/*
TODO
- Target highlight within the range
*/
