#include "SWSidebarClass.h"
#include <CommandClass.h>
#include <Ext/House/Body.h>
#include <Ext/Side/Body.h>
#include <sstream>

SWSidebarClass SWSidebarClass::Instance;
CommandClass* SWSidebarClass::Commands[10];


// =============================
// functions

bool SWSidebarClass::AddColumn()
{
	auto& columns = this->Columns;

	if (static_cast<int>(columns.size()) >= Phobos::UI::SuperWeaponSidebar_MaxColumns)
		return false;

	const auto column = DLLCreate<SWColumnClass>(SWButtonClass::StartID + SuperWeaponTypeClass::Array->Count + 1 + static_cast<int>(columns.size()), 0, 0, 60 + Phobos::UI::SuperWeaponSidebar_Interval, 48);

	if (!column)
		return false;

	column->Zap();
	GScreenClass::Instance->AddButton(column);
	return true;
}

bool SWSidebarClass::RemoveColumn()
{
	auto& columns = this->Columns;

	if (columns.empty())
		return false;

	if (const auto backColumn = columns.back())
	{
		AnnounceInvalidPointer(SWSidebarClass::Instance.CurrentColumn, backColumn);
		GScreenClass::Instance->RemoveButton(backColumn);

		DLLDelete(backColumn);
		columns.erase(columns.end() - 1);
		return true;
	}

	return false;
}

void SWSidebarClass::InitClear()
{
	this->CurrentColumn = nullptr;
	this->CurrentButton = nullptr;

	if (const auto toggleButton = this->ToggleButton)
	{
		this->ToggleButton = nullptr;
		GScreenClass::Instance->RemoveButton(toggleButton);
		DLLDelete(toggleButton);
	}

	auto& columns = this->Columns;

	for (const auto column : columns)
	{
		column->ClearButtons();
		GScreenClass::Instance->RemoveButton(column);
		DLLDelete(column);
	}

	columns.clear();
}

bool SWSidebarClass::AddButton(int superIdx)
{
	auto& columns = this->Columns;

	if (columns.empty() && !this->AddColumn())
		return false;

	if (std::any_of(columns.begin(), columns.end(), [superIdx](SWColumnClass* column) { return std::any_of(column->Buttons.begin(), column->Buttons.end(), [superIdx](SWButtonClass* button) { return button->SuperIndex == superIdx; }); }))
		return true;

	const bool success = columns.back()->AddButton(superIdx);

	if (success)
		SidebarExt::Global()->SWSidebar_Indices.AddUnique(superIdx);

	return success;
}

void SWSidebarClass::SortButtons()
{
	auto& columns = this->Columns;

	if (columns.empty())
		return;

	columns.erase(
		std::remove_if(columns.begin(), columns.end(),
			[](SWColumnClass* const column)
			{ return column->Buttons.empty(); }
		),
		columns.end()
	);

	if (columns.empty())
	{
		if (const auto toggleButton = this->ToggleButton)
			toggleButton->UpdatePosition();

		return;
	}

	std::vector<SWButtonClass*> vec_Buttons;
	vec_Buttons.reserve(this->GetMaximumButtonCount());

	for (const auto column : columns)
	{
		for (const auto button : column->Buttons)
			vec_Buttons.emplace_back(button);

		column->ClearButtons(false);
	}

	std::stable_sort(vec_Buttons.begin(), vec_Buttons.end(), [](SWButtonClass* const a, SWButtonClass* const b)
		{
			return BuildType::SortsBefore(AbstractType::Special, a->SuperIndex, AbstractType::Special, b->SuperIndex);
		});

	const int buttonCount = static_cast<int>(vec_Buttons.size());
	const int cameoWidth = 60, cameoHeight = 48;
	const int maximum = Phobos::UI::SuperWeaponSidebar_Max;
	Point2D location = { 0, (DSurface::ViewBounds().Height - std::min(buttonCount, maximum) * cameoHeight) / 2 };
	int location_Y = location.Y;
	int rowIdx = 0, columnIdx = 0;

	for (const auto button : vec_Buttons)
	{
		const auto column = columns[columnIdx];

		if (rowIdx == 0)
			column->SetPosition(location.X, location.Y);

		column->Buttons.emplace_back(button);
		button->SetColumn(columnIdx);
		button->SetPosition(location.X, location.Y);
		rowIdx++;

		if (rowIdx >= maximum - columnIdx)
		{
			rowIdx = 0;
			columnIdx++;
			location_Y += cameoHeight / 2;
			location = { location.X + cameoWidth + Phobos::UI::SuperWeaponSidebar_Interval, location_Y };
		}
		else
		{
			location.Y += cameoHeight;
		}
	}

	for (const auto column : columns)
		column->SetHeight(column->Buttons.size() * 48);

	if (const auto toggleButton = this->ToggleButton)
		toggleButton->UpdatePosition();
}

int SWSidebarClass::GetMaximumButtonCount()
{
	const int firstColumn = Phobos::UI::SuperWeaponSidebar_Max;
	const int columns = std::min(firstColumn, Phobos::UI::SuperWeaponSidebar_MaxColumns);
	return (firstColumn + (firstColumn - (columns - 1))) * columns / 2;
}

bool SWSidebarClass::IsEnabled()
{
	return SidebarExt::Global()->SWSidebar_Enable;
}

// Hooks

DEFINE_HOOK(0x692419, DisplayClass_ProcessClickCoords_SWSidebar, 0x7)
{
	enum { DoNothing = 0x6925FC };

	if (SWSidebarClass::IsEnabled() && SWSidebarClass::Instance.CurrentColumn)
		return DoNothing;

	const auto toggleButton = SWSidebarClass::Instance.ToggleButton;

	return toggleButton && toggleButton->IsHovering ? DoNothing : 0;
}

DEFINE_HOOK(0x4F92FB, HouseClass_UpdateTechTree_SWSidebar, 0x7)
{
	enum { SkipGameCode = 0x4F9302 };

	GET(HouseClass*, pHouse, ESI);

	pHouse->AISupers();

	if (pHouse->IsCurrentPlayer())
	{
		for (const auto column : SWSidebarClass::Instance.Columns)
		{
			for (const auto button : column->Buttons)
			{
				if (HouseClass::CurrentPlayer->Supers[button->SuperIndex]->IsPresent)
					continue;

				if (column->RemoveButton(button->SuperIndex))
					SidebarExt::Global()->SWSidebar_Indices.Remove(button->SuperIndex);
			}
		}
	}

	return SkipGameCode;
}

DEFINE_HOOK(0x6A6316, SidebarClass_AddCameo_SuperWeapon_SWSidebar, 0x6)
{
	enum { ReturnFalse = 0x6A65FF };

	GET_STACK(AbstractType, whatAmI, STACK_OFFSET(0x14, 0x4));
	GET_STACK(int, index, STACK_OFFSET(0x14, 0x8));

	switch (whatAmI)
	{
	case AbstractType::Super:
	case AbstractType::SuperWeaponType:
	case AbstractType::Special:
		if (SWSidebarClass::Instance.AddButton(index))
			return ReturnFalse;

		break;

	default:
		break;
	}

	return 0;
}

DEFINE_HOOK(0x6A5082, SidebarClass_InitClear_InitializeSWSidebar, 0x5)
{
	SWSidebarClass::Instance.InitClear();
	return 0;
}

DEFINE_HOOK(0x6A5839, SidebarClass_InitIO_InitializeSWSidebar, 0x5)
{
	if (!Phobos::UI::SuperWeaponSidebar || Unsorted::ArmageddonMode)
		return 0;

	if (const auto pSideExt = SideExt::ExtMap.Find(SideClass::Array->Items[ScenarioClass::Instance->PlayerSideIndex]))
	{
		if (const auto toggleShape = pSideExt->SuperWeaponSidebar_ToggleShape.Get())
		{
			if (const auto toggleButton = DLLCreate<ToggleSWButtonClass>(SWButtonClass::StartID + SuperWeaponTypeClass::Array->Count, 0, 0, toggleShape->Width, toggleShape->Height))
			{
				toggleButton->Zap();
				GScreenClass::Instance->AddButton(toggleButton);
				SWSidebarClass::Instance.ToggleButton = toggleButton;
				toggleButton->UpdatePosition();
			}
		}
	}

	for (const auto superIdx : SidebarExt::Global()->SWSidebar_Indices)
		SWSidebarClass::Instance.AddButton(superIdx);

	return 0;
}
