#include "SWButtonClass.h"
#include "SWSidebarClass.h"
#include <EventClass.h>
#include <CCToolTip.h>
#include <CommandClass.h>
#include <GameOptionsClass.h>
#include <UI.h>

#include <Ext/SWType/Body.h>
#include <Utilities/AresFunctions.h>
#include <Ext/Side/Body.h>

SWButtonClass::SWButtonClass(unsigned int id, int superIdx, int x, int y, int width, int height)
	: ControlClass(id, x, y, width, height, GadgetFlag::LeftPress, true)
	, SuperIndex(superIdx)
{
	if (const auto backColumn = SWSidebarClass::Instance.Columns.back())
		backColumn->Buttons.emplace_back(this);
}

bool SWButtonClass::Draw(bool forced)
{
	if (!forced)
		return false;

	const auto pSurface = DSurface::Composite();
	auto bounds = pSurface->GetRect();
	Point2D location = { this->X, this->Y };
	RectangleStruct destRect = { location.X, location.Y, this->Width, this->Height };

	const auto pCurrent = HouseClass::CurrentPlayer();
	const auto pSuper = pCurrent->Supers[this->SuperIndex];
	const auto pSWExt = SWTypeExt::ExtMap.Find(pSuper->Type);

	// support for pcx cameos
	if (const auto pPCXCameo = pSWExt->SidebarPCX.GetSurface())
	{
		PCX::Instance->BlitToSurface(&destRect, pSurface, pPCXCameo);
	}
	else if (const auto pCameo = pSuper->Type->SidebarImage) // old shp cameos, fixed palette
	{
		const auto pCameoRef = pCameo->AsReference();
		char pFilename[0x20];
		strcpy_s(pFilename, RulesExt::Global()->MissingCameo.data());
		_strlwr_s(pFilename);

		if (!_stricmp(pCameoRef->Filename, GameStrings::XXICON_SHP) && strstr(pFilename, ".pcx"))
		{
			PCX::Instance->LoadFile(pFilename);

			if (const auto CameoPCX = PCX::Instance->GetSurface(pFilename))
				PCX::Instance->BlitToSurface(&destRect, pSurface, CameoPCX);
		}
		else
		{
			const auto pConvert = pSWExt->SidebarPal.Convert ? pSWExt->SidebarPal.GetConvert() : FileSystem::CAMEO_PAL;
			pSurface->DrawSHP(pConvert, pCameo, 0, &location, &bounds, BlitterFlags::bf_400, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
		}
	}

	if (this->IsHovering)
	{
		RectangleStruct cameoRect = { location.X, location.Y, this->Width, this->Height };
		const COLORREF tooltipColor = Drawing::RGB_To_Int(Drawing::TooltipColor());
		pSurface->DrawRect(&cameoRect, tooltipColor);
	}

	if (pSuper->IsReady && !pCurrent->CanTransactMoney(pSWExt->Money_Amount) ||
		(pSWExt->SW_UseAITargeting && AresHelper::CanUseAres && !AresFunctions::IsTargetConstraintsEligible(AresFunctions::SWTypeExtMap_Find(pSuper->Type), HouseClass::CurrentPlayer, true)))
	{
		RectangleStruct darkenBounds { 0, 0, location.X + this->Width, location.Y + this->Height };
		pSurface->DrawSHP(FileSystem::SIDEBAR_PAL, FileSystem::DARKEN_SHP, 0, &location, &darkenBounds, BlitterFlags::bf_400 | BlitterFlags::Darken, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
	}

	const bool ready = !pSuper->IsSuspended && (pSuper->Type->UseChargeDrain ? pSuper->ChargeDrainState == ChargeDrainState::Ready : pSuper->IsReady);
	bool drawReadiness = true;

	if (ready && this->ColumnIndex == 0)
	{
		auto& buttons = SWSidebarClass::Instance.Columns[this->ColumnIndex]->Buttons;
		const int buttonId = std::distance(buttons.begin(), std::find(buttons.begin(), buttons.end(), this));

		if (buttonId < 10)
		{
			unsigned short hotkey = 0;
			for (int i = 0; i < CommandClass::Hotkeys->IndexCount; i++)
			{
				if (CommandClass::Hotkeys->IndexTable[i].Data == SWSidebarClass::Commands[buttonId])
					hotkey = CommandClass::Hotkeys->IndexTable[i].ID;
			}

			Point2D textLoc = { location.X + this->Width / 2, location.Y };
			const COLORREF foreColor = Drawing::RGB_To_Int(Drawing::TooltipColor);
			constexpr TextPrintType printType = TextPrintType::FullShadow | TextPrintType::Point8 | TextPrintType::Background | TextPrintType::Center;

			wchar_t buffer[64];
			UI::GetKeyboardKeyString(hotkey, buffer);

			if (std::wcslen(buffer))
			{
				pSurface->DrawTextA(buffer, &bounds, &textLoc, foreColor, 0, printType);
				drawReadiness = false;
			}
		}
	}

	if (drawReadiness)
	{
		if (const auto buffer = pSuper->NameReadiness())
		{
			Point2D textLoc = { location.X + this->Width / 2, location.Y };
			const COLORREF foreColor = Drawing::RGB_To_Int(Drawing::TooltipColor);
			constexpr TextPrintType printType = TextPrintType::FullShadow | TextPrintType::Point8 | TextPrintType::Background | TextPrintType::Center;

			pSurface->DrawTextA(buffer, &bounds, &textLoc, foreColor, 0, printType);
		}
	}

	if (pSuper->ShouldDrawProgress())
	{
		Point2D loc = { location.X, location.Y };
		pSurface->DrawSHP(FileSystem::SIDEBAR_PAL, FileSystem::GCLOCK2_SHP, pSuper->AnimStage() + 1, &loc, &bounds, BlitterFlags::bf_400 | BlitterFlags::TransLucent50, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
	}

	return true;
}

void SWButtonClass::OnMouseEnter()
{
	if (!SWSidebarClass::IsEnabled())
		return;

	this->IsHovering = true;
	SWSidebarClass::Instance.CurrentButton = this;
	SWSidebarClass::Instance.Columns[this->ColumnIndex]->OnMouseEnter();
	MouseClass::Instance->UpdateCursor(MouseCursorType::Default, false);
}

void SWButtonClass::OnMouseLeave()
{
	if (!SWSidebarClass::IsEnabled())
		return;

	this->IsHovering = false;
	SWSidebarClass::Instance.CurrentButton = nullptr;
	SWSidebarClass::Instance.Columns[this->ColumnIndex]->OnMouseLeave();
	MouseClass::Instance->UpdateCursor(MouseCursorType::Default, false);
	CCToolTip::Instance->MarkToRedraw(CCToolTip::Instance->CurrentToolTipData);
}

bool SWButtonClass::Action(GadgetFlag flags, DWORD* pKey, KeyModifier modifier)
{
	if ((int)flags & (int)GadgetFlag::LeftPress)
	{
		VocClass::PlayGlobal(RulesClass::Instance->GUIBuildSound, 0x2000, 1.0);
		this->LaunchSuper();
	}

	return this->ControlClass::Action(flags, pKey, KeyModifier::None);
}

void SWButtonClass::SetColumn(int column)
{
	this->ColumnIndex = column;
}

bool SWButtonClass::LaunchSuper() const
{
	const auto pCurrent = HouseClass::CurrentPlayer();
	const auto pSuper = pCurrent->Supers[this->SuperIndex];
	const auto pSWExt = SWTypeExt::ExtMap.Find(pSuper->Type);
	const bool manual = !pSWExt->SW_ManualFire && pSWExt->SW_AutoFire;
	const bool unstopable = pSuper->Type->UseChargeDrain && pSuper->ChargeDrainState == ChargeDrainState::Draining && pSWExt->SW_Unstoppable;

	if (!pSuper->CanFire() && !manual)
	{
		VoxClass::PlayIndex(pSWExt->EVA_Impatient);
		return false;
	}

	if (!pCurrent->CanTransactMoney(pSWExt->Money_Amount))
	{
		VoxClass::PlayIndex(pSWExt->EVA_InsufficientFunds);
		pSWExt->PrintMessage(pSWExt->Message_InsufficientFunds, pCurrent);
	}
	else if (!pSWExt->SW_UseAITargeting || (AresHelper::CanUseAres && AresFunctions::IsTargetConstraintsEligible(AresFunctions::SWTypeExtMap_Find(pSuper->Type), HouseClass::CurrentPlayer, true)))
	{
		if (!manual && !unstopable)
		{
			const auto swIndex = pSuper->Type->ArrayIndex;

			if (pSuper->Type->Action == Action::None || pSWExt->SW_UseAITargeting)
			{
				EventClass Event = EventClass(pCurrent->ArrayIndex, EventType::SpecialPlace, swIndex, CellStruct::Empty);
				EventClass::AddEvent(Event);
			}
			else
			{
				DisplayClass::Instance->CurrentBuilding = nullptr;
				DisplayClass::Instance->CurrentBuildingType = nullptr;
				DisplayClass::Instance->unknown_11AC = static_cast<DWORD>(-1);
				DisplayClass::Instance->SetActiveFoundation(nullptr);
				MapClass::Instance->SetRepairMode(0);
				DisplayClass::Instance->SetSellMode(0);
				DisplayClass::Instance->PowerToggleMode = false;
				DisplayClass::Instance->PlanningMode = false;
				DisplayClass::Instance->PlaceBeaconMode = false;
				DisplayClass::Instance->CurrentSWTypeIndex = swIndex;
				MapClass::Instance->UnselectAll();
				VoxClass::PlayIndex(pSWExt->EVA_SelectTarget);
			}

			return true;
		}
	}
	else
	{
		pSWExt->PrintMessage(pSWExt->Message_CannotFire, pCurrent);
	}

	return false;
}

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
	const auto pShape = pSideExt->ExclusiveSWSidebar_ToggleShape.Get();

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
