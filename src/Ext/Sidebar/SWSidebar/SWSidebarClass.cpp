#include "SWSidebarClass.h"
#include <Misc/PhobosToolTip.h>
#include <Ext/House/Body.h>

SWSidebarClass SWSidebarClass::Instance;

bool SWSidebarClass::AddButton(int superIdx)
{
	SWSidebarClass::Instance.Initialized = true;

	if (const auto pSWType = SuperWeaponTypeClass::Array->GetItemOrDefault(superIdx))
	{
		const auto pSWExt = SWTypeExt::ExtMap.Find(pSWType);

		if (!pSWExt->SW_ShowCameo)
			return true;

		if (!Phobos::UI::ExclusiveSuperWeaponSidebar || Unsorted::ArmageddonMode)
			return false;

		if (!pSWExt->ExclusiveSidebar_Allow)
			return false;

		const unsigned int ownerBits = 1u << HouseClass::CurrentPlayer->Type->ArrayIndex;

		if ((pSWExt->ExclusiveSidebar_RequiredHouses & ownerBits) == 0)
			return false;
	}
	else
	{
		return true;
	}

	auto& buttons = SWSidebarClass::Instance.Buttons;

	if (std::any_of(buttons.begin(), buttons.end(), [superIdx](TacticalButtonClass* const button) { return button->SuperIndex == superIdx; }))
		return true;

	DLLCreate<TacticalButtonClass>(superIdx + 2200, superIdx, 0, 0, 60, 48);
	SortButtons();
	return true;
}

bool SWSidebarClass::RemoveButton(int superIdx)
{
	auto& buttons = SWSidebarClass::Instance.Buttons;

	const auto it = std::find_if(buttons.begin(), buttons.end(), [superIdx](TacticalButtonClass* const button) { return button->SuperIndex == superIdx; });

	if (it == buttons.end())
		return false;

	DLLDelete(*it);
	SortButtons();
	return true;
}

void SWSidebarClass::ClearButtons()
{
	SWSidebarClass::Instance.CurrentButton = nullptr;
	auto& buttons = SWSidebarClass::Instance.Buttons;

	if (buttons.empty())
		return;

	for (const auto button : buttons)
		DLLDelete(button);

	buttons.clear();
}

void SWSidebarClass::SortButtons()
{
	auto& buttons = SWSidebarClass::Instance.Buttons;

	if (buttons.empty())
		return;

	const unsigned int ownerBits = 1u << HouseClass::CurrentPlayer->Type->ArrayIndex;

	std::stable_sort(buttons.begin(), buttons.end(), [ownerBits](TacticalButtonClass* const a, TacticalButtonClass* const b)
		{
			const auto pLeftExt = SWTypeExt::ExtMap.Find(SuperWeaponTypeClass::Array->Items[a->SuperIndex]);
			const auto pRightExt = SWTypeExt::ExtMap.Find(SuperWeaponTypeClass::Array->Items[b->SuperIndex]);

			if ((pLeftExt->ExclusiveSidebar_PriorityHouses & ownerBits) && !(pRightExt->ExclusiveSidebar_PriorityHouses & ownerBits))
				return false;

			if (!(pLeftExt->ExclusiveSidebar_PriorityHouses & ownerBits) && (pRightExt->ExclusiveSidebar_PriorityHouses & ownerBits))
				return true;

			return BuildType::SortsBefore(AbstractType::Special, a->SuperIndex, AbstractType::Special, b->SuperIndex);
		 });

	const int buttonCount = static_cast<int>(buttons.size());
	const int cameoWidth = 60, cameoHeight = 48;
	const int maximum = Phobos::UI::ExclusiveSuperWeaponSidebar_Max;
	Point2D location = { 0, (DSurface::ViewBounds().Height - std::min(buttonCount, maximum) * cameoHeight) / 2 };
	int location_Y = location.Y;
	int row = 0, line = 0;

	for (int idx = 0; idx < buttonCount && maximum - line > 0; idx++)
	{
		const auto button = buttons[idx];
		button->SetPosition(location.X, location.Y);
		row++;

		if (row >= maximum - line)
		{
			row = 0;
			line++;
			location_Y += cameoHeight / 2;
			location = { location.X + cameoWidth, location_Y };
		}
		else
		{
			location.Y += cameoHeight;
		}
	}
}

// Hooks

DEFINE_HOOK(0x692419, DisplayClass_ProcessClickCoords_TacticalButton, 0x7)
{
	return SWSidebarClass::Instance.CurrentButton ? 0x6925FC : 0;
}

// I cannot add it into YRppp :(
// It always failed, help me
static void __fastcall HouseClass_UpdateSuperWeaponsUnavailable(HouseClass* pHouse)
{
	JMP_STD(0x50B1D0);
}

DEFINE_HOOK(0x4F92FB, HouseClass_UpdateTechTree_UpdateSupers, 0x7)
{
	enum { SkipGameCode = 0x4F9302 };

	GET(HouseClass*, pHouse, ESI);

	HouseClass_UpdateSuperWeaponsUnavailable(pHouse);

	if (pHouse->IsCurrentPlayer())
	{
		for (const auto button : SWSidebarClass::Instance.Buttons)
		{
			if (!HouseClass::CurrentPlayer->Supers[button->SuperIndex]->IsPresent)
				SWSidebarClass::Instance.RemoveButton(button->SuperIndex);
		}
	}

	return SkipGameCode;
}

DEFINE_HOOK(0x6A6300, SidebarClass_AddCameo_SuperWeapon_TacticalButton, 0x6)
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

DEFINE_HOOK(0x6A5030, SidebarClass_Init_Clear_InitializedTacticalButton, 0x6)
{
	SWSidebarClass::Instance.Initialized = false;
	SWSidebarClass::Instance.ClearButtons();
	return 0;
}

DEFINE_HOOK(0x55B6B3, LogicClass_AI_InitializedTacticalButton, 0x5)
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
