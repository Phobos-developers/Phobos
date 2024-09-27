#include "SWSidebarClass.h"
#include <Misc/PhobosToolTip.h>
#include <Ext/House/Body.h>

SWSidebarClass SWSidebarClass::Instance;

bool SWSidebarClass::AddColumn()
{
	auto& columns = this->Columns;

	if (static_cast<int>(columns.size()) >= Phobos::UI::ExclusiveSWSidebar_MaxColumn)
		return false;

	return DLLCreate<SWColumnClass>(2200 + SuperWeaponTypeClass::Array->Count + static_cast<int>(columns.size()), 0, 0, 60 + Phobos::UI::ExclusiveSWSidebar_Interval, 48);
}

bool SWSidebarClass::RemoveColumn()
{
	auto& columns = this->Columns;

	if (const auto backColumn = columns.back())
	{
		DLLDelete(backColumn);
		return true;
	}

	return false;
}

void SWSidebarClass::ClearColumns()
{
	auto& columns = this->Columns;

	if (columns.empty())
		return;

	for (const auto button : columns)
		DLLDelete(button);

	columns.clear();
}

bool SWSidebarClass::AddButton(int superIdx)
{
	this->Initialized = true;
	auto& columns = this->Columns;

	if (columns.empty() && !this->AddColumn())
		return false;

	if (std::any_of(columns.begin(), columns.end(), [superIdx](SWColumnClass* column) { return std::any_of(column->Buttons.begin(), column->Buttons.end(), [superIdx](TacticalButtonClass* button) { return button->SuperIndex == superIdx; }); }))
		return true;

	return columns.back()->AddButton(superIdx);
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
		return;

	std::vector<TacticalButtonClass*> vec_Buttons;
	vec_Buttons.reserve(this->GetMaximumButtonCount());

	for (const auto column : columns)
	{
		for (const auto button : column->Buttons)
			vec_Buttons.emplace_back(button);

		column->ClearButtons(false);
	}

	const unsigned int ownerBits = 1u << HouseClass::CurrentPlayer->Type->ArrayIndex;

	std::stable_sort(vec_Buttons.begin(), vec_Buttons.end(), [ownerBits](TacticalButtonClass* const a, TacticalButtonClass* const b)
		{
			const auto pLeftExt = SWTypeExt::ExtMap.Find(SuperWeaponTypeClass::Array->Items[a->SuperIndex]);
			const auto pRightExt = SWTypeExt::ExtMap.Find(SuperWeaponTypeClass::Array->Items[b->SuperIndex]);

			if ((pLeftExt->ExclusiveSidebar_PriorityHouses & ownerBits) && !(pRightExt->ExclusiveSidebar_PriorityHouses & ownerBits))
				return true;

			if (!(pLeftExt->ExclusiveSidebar_PriorityHouses & ownerBits) && (pRightExt->ExclusiveSidebar_PriorityHouses & ownerBits))
				return true;

			return BuildType::SortsBefore(AbstractType::Special, a->SuperIndex, AbstractType::Special, b->SuperIndex);
		 });

	const int buttonCount = static_cast<int>(vec_Buttons.size());
	const int cameoWidth = 60, cameoHeight = 48;
	const int maximum = Phobos::UI::ExclusiveSWSidebar_Max;
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
			location = { location.X + cameoWidth + Phobos::UI::ExclusiveSWSidebar_Interval, location_Y };
		}
		else
		{
			location.Y += cameoHeight;
		}
	}

	for (const auto column : columns)
		column->SetHeight(column->Buttons.size() * 48);
}

int SWSidebarClass::GetMaximumButtonCount()
{
	const int firstColumn = Phobos::UI::ExclusiveSWSidebar_Max;
	const int columns = std::min(firstColumn, Phobos::UI::ExclusiveSWSidebar_MaxColumn);
	return (firstColumn + (firstColumn - (columns - 1))) * columns / 2;
}

bool SWSidebarClass::IsEnabled()
{
	return SidebarExt::Global()->SWSidebar_Enable;
}

// Hooks

DEFINE_HOOK(0x692419, DisplayClass_ProcessClickCoords_SWSidebar, 0x7)
{
	enum { Nothing = 0x6925FC };

	return SWSidebarClass::IsEnabled() && SWSidebarClass::Instance.CurrentColumn ? Nothing : 0;
}

// I cannot add it into YRppp :(
// It always failed, help me
static void __fastcall HouseClass_UpdateSuperWeaponsUnavailable(HouseClass* pHouse)
{
	JMP_STD(0x50B1D0);
}

DEFINE_HOOK(0x4F92FB, HouseClass_UpdateTechTree_SWSidebar, 0x7)
{
	enum { SkipGameCode = 0x4F9302 };

	GET(HouseClass*, pHouse, ESI);

	HouseClass_UpdateSuperWeaponsUnavailable(pHouse);

	if (pHouse->IsCurrentPlayer())
	{
		for (const auto column : SWSidebarClass::Instance.Columns)
		{
			for (const auto button : column->Buttons)
			{
				if (HouseClass::CurrentPlayer->Supers[button->SuperIndex]->IsPresent)
					continue;

				column->RemoveButton(button->SuperIndex);
			}
		}
	}

	return SkipGameCode;
}

DEFINE_HOOK(0x6A6300, SidebarClass_AddCameo_SuperWeapon_SWSidebar, 0x6)
{
	enum { SkipGameCode = 0x6A6606 };

	GET_STACK(AbstractType, whatAmI, 0x4);
	GET_STACK(int, index, 0x8);

	switch (whatAmI)
	{
	case AbstractType::Super:
	case AbstractType::SuperWeaponType:
	case AbstractType::Special:
		if (SWSidebarClass::Instance.AddButton(index))
		{
			R->AL(false);
			return SkipGameCode;
		}
		break;

	default:
		break;
	}

	return 0;
}

DEFINE_HOOK(0x6A5030, SidebarClass_Init_Clear_InitializedSWSidebar, 0x6)
{
	SWSidebarClass::Instance.Initialized = false;
	SWSidebarClass::Instance.ClearColumns();
	return 0;
}

DEFINE_HOOK(0x55B6B3, LogicClass_AI_InitializedSWSidebar, 0x5)
{
	if (SWSidebarClass::Instance.Initialized)
		return 0;

	SWSidebarClass::Instance.Initialized = true;
	const auto pCurrent = HouseClass::CurrentPlayer();

	if (!pCurrent || pCurrent->Defeated)
		return 0;

	for (const auto pSuper : pCurrent->Supers)
	{
		if (!pSuper->IsPresent)
			continue;

		SWSidebarClass::Instance.AddButton(pSuper->Type->ArrayIndex);
	}

	return 0;
}
