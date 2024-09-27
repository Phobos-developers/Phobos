#include "SWColumnClass.h"
#include "SWSidebarClass.h"

#include <Ext/SWType/Body.h>
#include <Ext/Side/Body.h>

SWColumnClass::SWColumnClass(unsigned int id, int x, int y, int width, int height)
	: ControlClass(id, x, y, width, height, static_cast<GadgetFlag>(0), true)
{
	auto& columns = SWSidebarClass::Instance.Columns;
	columns.emplace_back(this);

	this->MaxButtons = Phobos::UI::ExclusiveSWSidebar_Max - (static_cast<int>(columns.size()) - 1);

	this->Zap();
	GScreenClass::Instance->AddButton(this);
}

SWColumnClass::~SWColumnClass()
{
	auto& columns = SWSidebarClass::Instance.Columns;
	const auto it = std::find(columns.begin(), columns.end(), this);

	if (it != columns.end())
		columns.erase(it);

	AnnounceInvalidPointer(SWSidebarClass::Instance.CurrentColumn, this);
	GScreenClass::Instance->RemoveButton(this);
}

bool SWColumnClass::Draw(bool forced)
{
	if (!SWSidebarClass::IsEnabled())
		return false;

	const auto pSurface = DSurface::Composite();
	auto bounds = pSurface->GetRect();

	const auto pSideExt = SideExt::ExtMap.Find(SideClass::Array->Items[ScenarioClass::Instance->PlayerSideIndex]);
	const auto centerPCX = pSideExt->ExclusiveSWSidebar_CenterPCX.GetSurface();

	if (centerPCX)
	{
		RectangleStruct backRect = { this->X, this->Y, centerPCX->GetWidth(), centerPCX->GetHeight() };
		backRect.Width = centerPCX->GetWidth();
		backRect.Height = centerPCX->GetHeight();
		PCX::Instance->BlitToSurface(&backRect, pSurface, centerPCX);
	}

	if (const auto topPCX = pSideExt->ExclusiveSWSidebar_TopPCX.GetSurface())
	{
		RectangleStruct backRect = { this->X, this->Y, topPCX->GetWidth(), topPCX->GetHeight() };;
		backRect.Y -= backRect.Height;
		PCX::Instance->BlitToSurface(&backRect, pSurface, topPCX);
	}

	if (const auto bottomPCX = pSideExt->ExclusiveSWSidebar_BottomPCX.GetSurface())
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

		if (!Phobos::UI::ExclusiveSWSidebar || Unsorted::ArmageddonMode)
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

	auto& buttons = this->Buttons;

	if (static_cast<int>(buttons.size()) >= this->MaxButtons && !SWSidebarClass::Instance.AddColumn())
		return false;

	const auto button = DLLCreate<TacticalButtonClass>(superIdx + 2200, superIdx, 0, 0, 60, 48);

	if (!button)
		return false;

	buttons.emplace_back(button);
	SWSidebarClass::Instance.SortButtons();
	return true;
}

bool SWColumnClass::RemoveButton(int superIdx)
{
	auto& buttons = this->Buttons;

	const auto it = std::find_if(buttons.begin(), buttons.end(), [superIdx](TacticalButtonClass* const button) { return button->SuperIndex == superIdx; });

	if (it == buttons.end())
		return false;

	DLLDelete(*it);
	SWSidebarClass::Instance.SortButtons();
	return true;
}

void SWColumnClass::ClearButtons(bool remove)
{
	auto& buttons = this->Buttons;

	if (buttons.empty())
		return;

	if (remove)
	{
		for (const auto button : buttons)
			DLLDelete(button);
	}

	buttons.clear();
}

void SWColumnClass::SetHeight(int height)
{
	this->Height = height;
}
