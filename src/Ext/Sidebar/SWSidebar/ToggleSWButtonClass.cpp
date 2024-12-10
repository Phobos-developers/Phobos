#include "ToggleSWButtonClass.h"
#include "SWSidebarClass.h"
#include <CCToolTip.h>
#include <GameOptionsClass.h>
#include <ScenarioClass.h>

#include <Ext/Side/Body.h>

ToggleSWButtonClass::ToggleSWButtonClass(unsigned int id, int x, int y, int width, int height)
	: ControlClass(id, x, y, width, height, static_cast<GadgetFlag>((int)GadgetFlag::LeftPress | (int)GadgetFlag::LeftRelease), true)
{
	SWSidebarClass::Instance.ToggleButton = this;
}

bool ToggleSWButtonClass::Draw(bool forced)
{
	auto& columns = SWSidebarClass::Instance.Columns;

	if (columns.empty())
		return false;

	const auto pSideExt = SideExt::ExtMap.Find(SideClass::Array->Items[ScenarioClass::Instance->PlayerSideIndex]);
	const auto pShape = pSideExt->SuperWeaponSidebar_ToggleShape.Get();

	if (!pShape)
		return false;

	const auto pConvert = FileSystem::SIDEBAR_PAL();
	const auto pSurface = DSurface::Composite();
	Point2D position = { this->X, this->Y };
	RectangleStruct destRect = { position.X, position.Y, this->Width, this->Height };
	pSurface->DrawSHP(pConvert, pShape, SWSidebarClass::IsEnabled(), &position, &destRect, BlitterFlags::bf_400, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);

	if (this->IsHovering)
	{
		const COLORREF tooltipColor = Drawing::RGB_To_Int(Drawing::TooltipColor());
		pSurface->DrawRect(&destRect, tooltipColor);
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
	auto& columns = SWSidebarClass::Instance.Columns;

	if (columns.empty())
		return;

	this->IsHovering = false;
	this->IsPressed = false;
	MouseClass::Instance->UpdateCursor(MouseCursorType::Default, false);
	CCToolTip::Instance->MarkToRedraw(CCToolTip::Instance->CurrentToolTipData);
}

bool ToggleSWButtonClass::Action(GadgetFlag flags, DWORD* pKey, KeyModifier modifier)
{
	auto& columns = SWSidebarClass::Instance.Columns;

	if (columns.empty())
		return false;

	if ((int)flags & (int)GadgetFlag::LeftPress)
		this->IsPressed = true;

	if (((int)flags & (int)GadgetFlag::LeftRelease) && this->IsPressed)
	{
		this->IsPressed = false;
		VocClass::PlayGlobal(RulesClass::Instance->GUIBuildSound, 0x2000, 1.0);
		ToggleSWButtonClass::SwitchSidebar();
	}

	return this->ControlClass::Action(flags, pKey, KeyModifier::None);
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
