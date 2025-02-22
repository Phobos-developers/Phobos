#include "ToggleSWButtonClass.h"
#include "SWSidebarClass.h"
#include <CCToolTip.h>
#include <GameOptionsClass.h>
#include <ScenarioClass.h>

#include <Ext/Side/Body.h>

ToggleSWButtonClass::ToggleSWButtonClass(unsigned int id, int x, int y, int width, int height)
	: ControlClass(id, x, y, width, height, (GadgetFlag::LeftPress | GadgetFlag::LeftRelease), false)
{
	SWSidebarClass::Instance.ToggleButton = this;
}

bool ToggleSWButtonClass::Draw(bool forced)
{
	auto& columns = SWSidebarClass::Instance.Columns;

	if (columns.empty())
		return false;

	const auto pSideExt = SideExt::ExtMap.Find(SideClass::Array->Items[ScenarioClass::Instance->PlayerSideIndex]);
	const auto pTogglePCX = SWSidebarClass::IsEnabled() ? pSideExt->SuperWeaponSidebar_OnPCX.GetSurface() : pSideExt->SuperWeaponSidebar_OffPCX.GetSurface();

	if (!pTogglePCX)
		return false;

	RectangleStruct destRect { this->X, this->Y, this->Width, this->Height };
	PCX::Instance->BlitToSurface(&destRect, DSurface::Composite, pTogglePCX);

	if (this->IsHovering)
	{
		const COLORREF tooltipColor = Drawing::RGB_To_Int(Drawing::TooltipColor());
		DSurface::Composite->DrawRect(&destRect, tooltipColor);
	}

	return true;
}

void ToggleSWButtonClass::OnMouseEnter()
{
	auto& columns = SWSidebarClass::Instance.Columns;

	if (columns.empty())
		return;

	this->IsHovering = true;
	MouseClass::Instance->UpdateCursor(MouseCursorType::Default, false);
}

void ToggleSWButtonClass::OnMouseLeave()
{
	this->IsHovering = false;
	this->IsPressed = false;
	MouseClass::Instance->UpdateCursor(MouseCursorType::Default, false);
}

bool ToggleSWButtonClass::Action(GadgetFlag flags, DWORD* pKey, KeyModifier modifier)
{
	auto& columns = SWSidebarClass::Instance.Columns;

	if (columns.empty())
		return false;

	const auto pSideExt = SideExt::ExtMap.Find(SideClass::Array->Items[ScenarioClass::Instance->PlayerSideIndex]);

	if (SWSidebarClass::IsEnabled() ? !pSideExt->SuperWeaponSidebar_OnPCX.GetSurface() : !pSideExt->SuperWeaponSidebar_OffPCX.GetSurface())
		return false;

	if (flags & GadgetFlag::LeftPress)
		this->IsPressed = true;

	if ((flags & GadgetFlag::LeftRelease) && this->IsPressed)
	{
		this->IsPressed = false;
		VocClass::PlayGlobal(RulesClass::Instance->GUIMainButtonSound, 0x2000, 1.0);
		ToggleSWButtonClass::SwitchSidebar();
	}

	// this->ControlClass::Action(flags, pKey, KeyModifier::None);
	reinterpret_cast<bool(__thiscall*)(ControlClass*, GadgetFlag, DWORD*, KeyModifier)>(0x48E5A0)(this, flags, pKey, KeyModifier::None);
	return true;
}

void ToggleSWButtonClass::UpdatePosition()
{
	Point2D position = Point2D::Empty;
	auto& columns = SWSidebarClass::Instance.Columns;

	if (!columns.empty())
	{
		const auto backColumn = columns.back();
		position.X = SWSidebarClass::Instance.IsEnabled() ? backColumn->X + backColumn->Width : 0;
		position.Y = backColumn->Y + (backColumn->Height - this->Height) / 2;
	}
	else
	{
		position.X = 0;
		position.Y = (GameOptionsClass::Instance->ScreenHeight - this->Height) / 2;
	}

	this->SetPosition(position.X, position.Y);
}

bool ToggleSWButtonClass::SwitchSidebar()
{
	SidebarExt::Global()->SWSidebar_Enable = !SidebarExt::Global()->SWSidebar_Enable;

	if (const auto toggleButton = SWSidebarClass::Instance.ToggleButton)
		toggleButton->UpdatePosition();

	return SWSidebarClass::Instance.IsEnabled();
}
