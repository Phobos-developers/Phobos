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

	const auto pSurface = DSurface::Composite();
	auto bounds = pSurface->GetRect();

	const auto pSideExt = SideExt::ExtMap.Find(SideClass::Array->Items[ScenarioClass::Instance->PlayerSideIndex]);
	const auto centerPCX = pSideExt->SuperWeaponSidebar_CenterPCX.GetSurface();

	if (centerPCX)
	{
		RectangleStruct backRect = { this->X, 0, centerPCX->GetWidth(), centerPCX->GetHeight() };
		backRect.Width = centerPCX->GetWidth();
		backRect.Height = centerPCX->GetHeight();

		for (const auto button : this->Buttons)
		{
			backRect.Y = button->Y;
			PCX::Instance->BlitToSurface(&backRect, pSurface, centerPCX);
		}
	}

	if (const auto topPCX = pSideExt->SuperWeaponSidebar_TopPCX.GetSurface())
	{
		RectangleStruct backRect = { this->X, this->Y, topPCX->GetWidth(), topPCX->GetHeight() };;
		backRect.Y -= backRect.Height;
		PCX::Instance->BlitToSurface(&backRect, pSurface, topPCX);
	}

	if (const auto bottomPCX = pSideExt->SuperWeaponSidebar_BottomPCX.GetSurface())
	{
		RectangleStruct backRect = { this->X, this->Y, bottomPCX->GetWidth(), bottomPCX->GetHeight() };;
		backRect.Y += this->Height;
		PCX::Instance->BlitToSurface(&backRect, pSurface, bottomPCX);
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
}

void SWColumnClass::OnMouseLeave()
{
	SWSidebarClass::Instance.CurrentColumn = nullptr;
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

	const auto button = DLLCreate<SWButtonClass>(SWButtonClass::StartID + superIdx, superIdx, 0, 0, 60, 48);

	if (!button)
		return false;

	button->Zap();
	GScreenClass::Instance->AddButton(button);
	SWSidebarClass::Instance.SortButtons();
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

	DLLDelete(*it);
	buttons.erase(it);
	SWSidebarClass::Instance.SortButtons();
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
	this->Height = height;
}
