#include "TacticalButtonClass.h"
#include "SWSidebarClass.h"
#include <EventClass.h>
#include <CCToolTip.h>

#include <Ext/SWType/Body.h>
#include <Utilities/AresFunctions.h>

TacticalButtonClass::TacticalButtonClass(unsigned int id, int superIdx, int x, int y, int width, int height)
	: ControlClass(id, x, y, width, height, GadgetFlag((int)GadgetFlag::LeftPress | (int)GadgetFlag::LeftRelease), true)
	, SuperIndex(superIdx)
{
	this->Zap();
	GScreenClass::Instance->AddButton(this);
}

TacticalButtonClass::~TacticalButtonClass()
{
	if (this->ColumnIndex != -1)
	{
		auto& columns = SWSidebarClass::Instance.Columns;
		auto& buttons = columns[this->ColumnIndex]->Buttons;
		const auto it = std::find(buttons.begin(), buttons.end(), this);

		if (it != buttons.end())
			buttons.erase(it);
	}

	AnnounceInvalidPointer(SWSidebarClass::Instance.CurrentButton, this);
	GScreenClass::Instance->RemoveButton(this);
}

bool TacticalButtonClass::Draw(bool forced)
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

	if (pSuper->IsReady && !pCurrent->CanTransactMoney(pSWExt->Money_Amount) ||
		(pSWExt->SW_UseAITargeting && AresHelper::CanUseAres && !AresFunctions::IsTargetConstraintsEligible(AresFunctions::SWTypeExtMap_Find(pSuper->Type), HouseClass::CurrentPlayer, true)))
	{
		RectangleStruct darkenBounds { 0, 0, location.X + this->Width, location.Y + this->Height };
		pSurface->DrawSHP(FileSystem::SIDEBAR_PAL, FileSystem::DARKEN_SHP, 0, &location, &darkenBounds, BlitterFlags::bf_400 | BlitterFlags::Darken, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
	}

	if (this->IsHovering)
	{
		RectangleStruct cameoRect = { location.X, location.Y, this->Width, this->Height };
		const COLORREF tooltipColor = Drawing::RGB_To_Int(Drawing::TooltipColor());
		pSurface->DrawRect(&cameoRect, tooltipColor);
	}

	if (!pSuper->RechargeTimer.Completed())
	{
		Point2D loc = { location.X, location.Y };
		pSurface->DrawSHP(FileSystem::SIDEBAR_PAL, FileSystem::GCLOCK2_SHP, pSuper->AnimStage() + 1, &loc, &bounds, BlitterFlags::bf_400 | BlitterFlags::TransLucent50, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
	}

	const auto buffer = pSuper->NameReadiness();;

	if (buffer && *buffer)
	{
		Point2D textLoc = { location.X + this->Width / 2, location.Y };
		const COLORREF foreColor = Drawing::RGB_To_Int(Drawing::TooltipColor);
		TextPrintType printType = TextPrintType::FullShadow | TextPrintType::Point8 | TextPrintType::Background | TextPrintType::Center;

		pSurface->DrawTextA(buffer, &bounds, &textLoc, foreColor, 0, printType);
	}

	return true;
}

void TacticalButtonClass::OnMouseEnter()
{
	if (!SWSidebarClass::IsEnabled())
		return;

	this->IsHovering = true;
	SWSidebarClass::Instance.CurrentButton = this;
	SWSidebarClass::Instance.Columns[this->ColumnIndex]->OnMouseEnter();
	MouseClass::Instance->UpdateCursor(MouseCursorType::Default, false);
}

void TacticalButtonClass::OnMouseLeave()
{
	if (!SWSidebarClass::IsEnabled())
		return;

	this->IsHovering = false;
	this->IsPressed = false;
	SWSidebarClass::Instance.CurrentButton = nullptr;
	SWSidebarClass::Instance.Columns[this->ColumnIndex]->OnMouseLeave();
	MouseClass::Instance->UpdateCursor(MouseCursorType::Default, false);
	CCToolTip::Instance->MarkToRedraw(CCToolTip::Instance->CurrentToolTipData);
}

bool TacticalButtonClass::Action(GadgetFlag flags, DWORD* pKey, KeyModifier modifier)
{
	if ((int)flags & (int)GadgetFlag::LeftPress)
		this->IsPressed = true;

	if (((int)flags & (int)GadgetFlag::LeftRelease) && this->IsPressed)
	{
		this->IsPressed = false;
		this->LaunchSuper();
	}

	return this->ControlClass::Action(flags, pKey, KeyModifier::None);
}

void TacticalButtonClass::SetColumn(int column)
{
	this->ColumnIndex = column;
}

bool TacticalButtonClass::LaunchSuper() const
{
	const auto pCurrent = HouseClass::CurrentPlayer();
	const auto pSuper = pCurrent->Supers[this->SuperIndex];
	const auto pSWExt = SWTypeExt::ExtMap.Find(pSuper->Type);
	VocClass::PlayGlobal(RulesClass::Instance->GUIBuildSound, 0x2000, 1.0);
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
