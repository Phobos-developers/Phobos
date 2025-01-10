#include "SWColumnClass.h"
#include "SWSidebarClass.h"

#include <Ext/SWType/Body.h>
#include <Ext/Side/Body.h>

SWColumnClass::SWColumnClass(unsigned int id, int x, int y, int width, int height)
	: ControlClass(id, x, y, width, height, static_cast<GadgetFlag>(0), true)
{
	auto& columns = SWSidebarClass::Instance.Columns;
	columns.emplace_back(this);

	this->MaxButtons = Phobos::UI::SuperWeaponSidebar_Max - (static_cast<int>(columns.size()) - 1);
}

bool SWColumnClass::Draw(bool forced)
{
	if (!SWSidebarClass::IsEnabled())
		return false;

	const auto pSideExt = SideExt::ExtMap.Find(SideClass::Array->Items[ScenarioClass::Instance->PlayerSideIndex]);
	const int cameoWidth = 60, cameoHeight = 48;
	const int cameoBackgroundWidth = Phobos::UI::SuperWeaponSidebar_Interval + cameoWidth;

	if (const auto pCenterPCX = pSideExt->SuperWeaponSidebar_CenterPCX.GetSurface())
	{
		const int cameoHarfInterval = (Phobos::UI::SuperWeaponSidebar_CameoHeight - cameoHeight) / 2;

		for (const auto button : this->Buttons)
		{
			RectangleStruct drawRect { this->X, button->Y - cameoHarfInterval, cameoBackgroundWidth, Phobos::UI::SuperWeaponSidebar_CameoHeight };
			PCX::Instance->BlitToSurface(&drawRect, DSurface::Composite, pCenterPCX);
		}
	}

	if (const auto pTopPCX = pSideExt->SuperWeaponSidebar_TopPCX.GetSurface())
	{
		const int height = pTopPCX->GetHeight();
		RectangleStruct drawRect { this->X, this->Y, cameoBackgroundWidth, height };
		PCX::Instance->BlitToSurface(&drawRect, DSurface::Composite, pTopPCX);
	}

	if (const auto pBottomPCX = pSideExt->SuperWeaponSidebar_BottomPCX.GetSurface())
	{
		const int height = pBottomPCX->GetHeight();
		RectangleStruct drawRect { this->X, this->Y + this->Height - height, cameoBackgroundWidth, height };
		PCX::Instance->BlitToSurface(&drawRect, DSurface::Composite, pBottomPCX);
	}

	for (const auto button : this->Buttons)
		button->Draw(true);

	return true;
}

void SWColumnClass::OnMouseEnter()
{
	if (!SWSidebarClass::IsEnabled())
		return;

	SWSidebarClass::Instance.CurrentColumn = this;
	MouseClass::Instance->UpdateCursor(MouseCursorType::Default, false);
}

void SWColumnClass::OnMouseLeave()
{
	SWSidebarClass::Instance.CurrentColumn = nullptr;
	MouseClass::Instance->UpdateCursor(MouseCursorType::Default, false);
}

bool SWColumnClass::Clicked(DWORD* pKey, GadgetFlag flags, int x, int y, KeyModifier modifier)
{
	for (const auto button : this->Buttons)
	{
		if (button->Clicked(pKey, flags, x, y, modifier))
			return true;
	}

	return false;
}

bool SWColumnClass::AddButton(int superIdx)
{
	if (const auto pSWType = SuperWeaponTypeClass::Array->GetItemOrDefault(superIdx))
	{
		const auto pSWExt = SWTypeExt::ExtMap.Find(pSWType);

		if (!pSWExt->SW_ShowCameo)
			return true;

		if (!Phobos::UI::SuperWeaponSidebar)
			return false;

		if (!pSWExt->SuperWeaponSidebar_Allow)
			return false;

		const unsigned int ownerBits = 1u << HouseClass::CurrentPlayer->Type->ArrayIndex;

		if ((pSWExt->SuperWeaponSidebar_RequiredHouses & ownerBits) == 0)
			return false;
	}
	else
	{
		return true;
	}

	auto& buttons = this->Buttons;

	if (static_cast<int>(buttons.size()) >= this->MaxButtons && !SWSidebarClass::Instance.AddColumn())
		return false;

	const int cameoWidth = 60, cameoHeight = 48;
	const auto button = DLLCreate<SWButtonClass>(SWButtonClass::StartID + superIdx, superIdx, 0, 0, cameoWidth, cameoHeight);

	if (!button)
		return false;

	button->Zap();
	GScreenClass::Instance->AddButton(button);
	SWSidebarClass::Instance.SortButtons();

	if (const auto toggleButton = SWSidebarClass::Instance.ToggleButton)
		toggleButton->UpdatePosition();

	return true;
}

bool SWColumnClass::RemoveButton(int superIdx)
{
	auto& buttons = this->Buttons;

	const auto it = std::find_if(buttons.begin(), buttons.end(), [superIdx](SWButtonClass* const button) { return button->SuperIndex == superIdx; });

	if (it == buttons.end())
		return false;

	AnnounceInvalidPointer(SWSidebarClass::Instance.CurrentButton, *it);
	GScreenClass::Instance->RemoveButton(*it);

	buttons.erase(it);
	return true;
}

void SWColumnClass::ClearButtons(bool remove)
{
	auto& buttons = this->Buttons;

	if (remove)
	{
		for (const auto button : buttons)
			GScreenClass::Instance->RemoveButton(button);
	}

	buttons.clear();
}

void SWColumnClass::SetHeight(int height)
{
	const auto pSideExt = SideExt::ExtMap.Find(SideClass::Array->Items[ScenarioClass::Instance->PlayerSideIndex]);

	this->Height = height;

	if (const auto pTopPCX = pSideExt->SuperWeaponSidebar_TopPCX.GetSurface())
		this->Height += pTopPCX->GetHeight();

	if (const auto pBottomPCX = pSideExt->SuperWeaponSidebar_BottomPCX.GetSurface())
		this->Height += pBottomPCX->GetHeight();
}
