#include "SWColumnClass.h"
#include "SWSidebarClass.h"

#include <Ext/SWType/Body.h>
#include <Ext/Side/Body.h>

SWColumnClass::SWColumnClass(int maxButtons, int x, int y, int width, int height)
	: GadgetClass(x, y, width, height, static_cast<GadgetFlag>(0), false)
	, MaxButtons(maxButtons)
{
	SWSidebarClass::Instance.Columns.emplace_back(this);
	this->Disabled = !SWSidebarClass::IsEnabled();
}

SWColumnClass::~SWColumnClass()
{
	// The vanilla game did not consider adding/deleting buttons midway through the game,
	// so this behavior needs to be made known to the global variable
	if (this == Make_Global<GadgetClass*>(0x8B3E94))
		this->OnMouseLeave();
}

bool SWColumnClass::Draw(bool forced)
{
	if (!SWSidebarClass::IsEnabled())
		return false;

	const auto pSideExt = SideExt::ExtMap.Find(SideClass::Array.Items[ScenarioClass::Instance->PlayerSideIndex]);
	const int cameoWidth = 60, cameoHeight = 48;
	const int cameoBackgroundWidth = Phobos::UI::SuperWeaponSidebar_Interval + cameoWidth;
	const int coordX = this->X;

	if (const auto pCenterPCX = pSideExt->SuperWeaponSidebar_CenterPCX.GetSurface())
	{
		const int cameoHarfInterval = (Phobos::UI::SuperWeaponSidebar_CameoHeight - cameoHeight) / 2;

		for (const auto button : this->Buttons)
		{
			RectangleStruct drawRect { coordX, button->Y - cameoHarfInterval, cameoBackgroundWidth, Phobos::UI::SuperWeaponSidebar_CameoHeight };
			PCX::Instance.BlitToSurface(&drawRect, DSurface::Composite, pCenterPCX);
		}
	}

	if (const auto pTopPCX = pSideExt->SuperWeaponSidebar_TopPCX.GetSurface())
	{
		const int height = pTopPCX->GetHeight();
		RectangleStruct drawRect { coordX, this->Y, cameoBackgroundWidth, height };
		PCX::Instance.BlitToSurface(&drawRect, DSurface::Composite, pTopPCX);
	}

	if (const auto pBottomPCX = pSideExt->SuperWeaponSidebar_BottomPCX.GetSurface())
	{
		const int height = pBottomPCX->GetHeight();
		RectangleStruct drawRect { coordX, this->Y + this->Height - height, cameoBackgroundWidth, height };
		PCX::Instance.BlitToSurface(&drawRect, DSurface::Composite, pBottomPCX);
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
	MouseClass::Instance.UpdateCursor(MouseCursorType::Default, false);
}

void SWColumnClass::OnMouseLeave()
{
	SWSidebarClass::Instance.CurrentColumn = nullptr;
	MouseClass::Instance.UpdateCursor(MouseCursorType::Default, false);
}

bool SWColumnClass::Clicked(DWORD* pKey, GadgetFlag flags, int x, int y, KeyModifier modifier)
{
	return false;
}

bool SWColumnClass::AddButton(int superIdx)
{
	auto& buttons = this->Buttons;
	const int buttonCount = static_cast<int>(buttons.size());
	auto& sidebar = SWSidebarClass::Instance;

	if (buttonCount >= this->MaxButtons && !SWSidebarClass::Instance.AddColumn())
	{
		const unsigned int ownerBits = 1u << HouseClass::CurrentPlayer->Type->ArrayIndex;

		auto Compare = [ownerBits](const int left, const int right)
		{
			const auto pExtA = SWTypeExt::ExtMap.TryFind(SuperWeaponTypeClass::Array.GetItemOrDefault(left));
			const auto pExtB = SWTypeExt::ExtMap.TryFind(SuperWeaponTypeClass::Array.GetItemOrDefault(right));

			if (pExtB && (pExtB->SuperWeaponSidebar_PriorityHouses & ownerBits) && (!pExtA || !(pExtA->SuperWeaponSidebar_PriorityHouses & ownerBits)))
				return false;

			if ((!pExtB || !(pExtB->SuperWeaponSidebar_PriorityHouses & ownerBits)) && pExtA && (pExtA->SuperWeaponSidebar_PriorityHouses & ownerBits))
				return true;

			return BuildType::SortsBefore(AbstractType::Special, left, AbstractType::Special, right);
		};

		const int backIdx = buttons.back()->SuperIndex;

		if (!Compare(superIdx, backIdx))
			return false;

		this->RemoveButton(backIdx);
		sidebar.DisableEntry = true;
		SidebarClass::Instance.AddCameo(AbstractType::Special, backIdx);
		SidebarClass::Instance.RepaintSidebar(SidebarClass::GetObjectTabIdx(AbstractType::Super, backIdx, false));
		sidebar.DisableEntry = false;
	}

	const int cameoWidth = 60, cameoHeight = 48;
	const auto button = GameCreate<SWButtonClass>(superIdx, 0, 0, cameoWidth, cameoHeight);

	if (!button)
		return false;

	button->Zap();
	GScreenClass::Instance.AddButton(button);
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

	auto& indices = ScenarioExt::Global()->SWSidebar_Indices;
	const auto it_Idx = std::find(indices.cbegin(), indices.cend(), superIdx);

	if (it_Idx != indices.cend())
		indices.erase(it_Idx);

	const auto pButton = *it;
	GScreenClass::Instance.RemoveButton(pButton);
	GameDelete(pButton);
	buttons.erase(it);
	return true;
}

void SWColumnClass::ClearButtons(bool remove)
{
	auto& buttons = this->Buttons;

	if (remove)
	{
		for (const auto button : buttons)
		{
			GScreenClass::Instance.RemoveButton(button);
			GameDelete(button);
		}
	}

	buttons.clear();
}

void SWColumnClass::SetHeight(int height)
{
	const auto pSideExt = SideExt::ExtMap.Find(SideClass::Array.Items[ScenarioClass::Instance->PlayerSideIndex]);

	this->Height = height;

	if (const auto pTopPCX = pSideExt->SuperWeaponSidebar_TopPCX.GetSurface())
		this->Height += pTopPCX->GetHeight();

	if (const auto pBottomPCX = pSideExt->SuperWeaponSidebar_BottomPCX.GetSurface())
		this->Height += pBottomPCX->GetHeight();
}
