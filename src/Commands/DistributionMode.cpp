#include "DistributionMode.h"

#include <Ext/TechnoType/Body.h>
#include <Utilities/Helpers.Alex.h>
#include <Helpers/Macro.h>

#include <HouseClass.h>
#include <WWMouseClass.h>

int DistributionMode1CommandClass::Mode = 0;
int DistributionMode2CommandClass::Mode = 0;

void __fastcall PrintDistributionModeMessage()
{
	wchar_t text[0x40];
	swprintf_s(text, L"Distribution Mode < Spread: r=%2d , Filter: %s >",
		(DistributionMode1CommandClass::Mode ? (2 << DistributionMode1CommandClass::Mode) : 0),
		((DistributionMode2CommandClass::Mode > 1) ? ((DistributionMode2CommandClass::Mode == 3) ? L"Name" : L"Type") : (DistributionMode2CommandClass::Mode == 1) ? L"Auto" : L"None")
	);
	MessageListClass::Instance->PrintMessage(text, 200, 5, true);
}

const char* DistributionMode1CommandClass::GetName() const
{
	return "Distribution Mode Spread";
}

const wchar_t* DistributionMode1CommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DISTR_SPREAD", L"Distribution spread");
}

const wchar_t* DistributionMode1CommandClass::GetUICategory() const
{
	return CATEGORY_CONTROL;
}

const wchar_t* DistributionMode1CommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DISTR_SPREAD_DESC", L"Automatically and averagely select similar targets around the original target. This is for changing the search range");
}

void DistributionMode1CommandClass::Execute(WWKey eInput) const
{
	DistributionMode1CommandClass::Mode = ((DistributionMode1CommandClass::Mode + 1) & 3);
	PrintDistributionModeMessage();
}

const char* DistributionMode2CommandClass::GetName() const
{
	return "Distribution Mode Filter";
}

const wchar_t* DistributionMode2CommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DISTR_FILTER", L"Distribution filter");
}

const wchar_t* DistributionMode2CommandClass::GetUICategory() const
{
	return CATEGORY_CONTROL;
}

const wchar_t* DistributionMode2CommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DISTR_FILTER_DESC", L"Automatically and averagely select similar targets around the original target. This is for changing the filter criteria");
}

void DistributionMode2CommandClass::Execute(WWKey eInput) const
{
	DistributionMode2CommandClass::Mode = ((DistributionMode2CommandClass::Mode + 1) & 3);
	PrintDistributionModeMessage();
}

DEFINE_HOOK(0x4AE818, DisplayClass_sub_4AE750_AutoDistribution, 0xA)
{
	enum { SkipGameCode = 0x4AE85C };

	GET(ObjectClass* const, pTarget, EBP);
	GET_STACK(const Action, mouseAction, STACK_OFFSET(0x20, 0xC));

	const auto count = ObjectClass::CurrentObjects->Count;

	if (count > 0)
	{
		const auto mode1 = DistributionMode1CommandClass::Mode;
		const auto mode2 = DistributionMode2CommandClass::Mode;

		// Distribution mode main
		if (mode1 && count > 1 && mouseAction != Action::NoMove && !PlanningNodeClass::PlanningModeActive && (pTarget->AbstractFlags & AbstractFlags::Techno) != AbstractFlags::None && !pTarget->IsInAir())
		{
			const auto pSpecial = HouseClass::FindSpecial();
			const auto pCivilian = HouseClass::FindCivilianSide();
			const auto pNeutral = HouseClass::FindNeutral();

			const auto pTargetHouse = static_cast<TechnoClass*>(pTarget)->Owner;
			const bool targetIsNeutral = pTargetHouse == pSpecial || pTargetHouse == pCivilian || pTargetHouse == pNeutral;

			const auto range = (2 << mode1);
			const auto pItems = Helpers::Alex::getCellSpreadItems(pTarget->Location, range);
			std::map<TechnoClass*, int> record;
			int current = 1;

			for (const auto& pItem : pItems)
				record[pItem] = 0;

			for (const auto& pSelect : ObjectClass::CurrentObjects())
			{
				TechnoClass* pCanTarget = nullptr;
				TechnoClass* pNewTarget = nullptr;

				for (const auto& [pItem, num] : record)
				{
					if (pSelect->MouseOverObject(pItem) == mouseAction && (targetIsNeutral || (pItem->Owner != pSpecial && pItem->Owner != pCivilian && pItem->Owner != pNeutral))
						&& (mode2 < 2 || (pItem->WhatAmI() == pTarget->WhatAmI()
						&& (mode2 < 3 || TechnoTypeExt::GetSelectionGroupID(pItem->GetTechnoType()) == TechnoTypeExt::GetSelectionGroupID(pTarget->GetTechnoType())))))
					{
						pCanTarget = pItem;

						if (num < current)
						{
							pNewTarget = pCanTarget;
							break;
						}
					}
				}

				if (!pNewTarget)
				{
					if (pCanTarget)
					{
						++current;
						pNewTarget = pCanTarget;
					}
				}

				if (pNewTarget)
				{
					if (record.contains(pNewTarget))
						++record[pNewTarget];

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
			for (const auto& pSelect : ObjectClass::CurrentObjects())
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
/*
DEFINE_HOOK(0x4F4596, GScreenClass_DrawOnTop_DrawDistributionModeShape, 0x7)
{
	const auto mode1 = DistributionMode1CommandClass::Mode;
	const auto mode2 = DistributionMode2CommandClass::Mode;

	if (mode1 || mode2)
	{
		auto position = WWMouseClass::Instance->XY1;
		RectangleStruct rect = DSurface::Composite->GetRect();
		rect.Height -= 32;

		if (position.X < rect.Width && position.Y < rect.Height)
		{
			position.Y += 20;
			const auto mode2Text = ((mode2 > 1) ? ((mode2 == 3) ? L"Name" : L"Type") : (mode2 == 1) ? L"Auto" : L"None");
			wchar_t text[0x20];
			swprintf_s(text, L"%s:%2d", mode2Text, (mode1 ? (2 << mode1) : 0));
			RectangleStruct dim = Drawing::GetTextDimensions(text, Point2D::Empty, 0);
			dim.Width += 4;
			const int dX = rect.Width - dim.Width;
			const int dY = rect.Height - dim.Height;

			if (position.X >= dX)
				position.X = dX;

			if (position.Y >= dY)
				position.Y = dY;

			dim.X = position.X;
			dim.Y = position.Y;
			ColorStruct color { 0, 0, 0 };
			DSurface::Composite->FillRectTrans(&dim, &color, 40);
			position.X += 2;
			DSurface::Composite->DrawTextA(text, &rect, &position, COLOR_WHITE, COLOR_BLACK, TextPrintType::LightShadow);
		}
	}

	return 0;
}
*/
