#include "TacticalButtonClass.h"
#include <EventClass.h>
#include <WWMouseClass.h>
#include <BitFont.h>
#include <CCToolTip.h>

#include <Misc/PhobosToolTip.h>
#include <Ext/Side/Body.h>
#include <Ext/Surface/Body.h>
#include <Ext/House/Body.h>
#include <Utilities/AresFunctions.h>

std::vector<TacticalButtonClass*> TacticalButtonClass::Buttons {};
bool TacticalButtonClass::Initialized { false };
TacticalButtonClass* TacticalButtonClass::CurrentButton { nullptr };

TacticalButtonClass::TacticalButtonClass(unsigned int id, int superIdx, int x, int y, int width, int height)
	: ToggleClass(id, x, y, width, height)
	, SuperIndex(superIdx)
{
	TacticalButtonClass::Buttons.emplace_back(this);

	this->Zap();
	GScreenClass::Instance->AddButton(this);
}

TacticalButtonClass::~TacticalButtonClass()
{
	auto& buttons = TacticalButtonClass::Buttons;
	const auto it = std::find(buttons.begin(), buttons.end(), this);

	if (it != buttons.end())
		buttons.erase(it);

	if (TacticalButtonClass::CurrentButton == this)
		TacticalButtonClass::CurrentButton = nullptr;

	GScreenClass::Instance->RemoveButton(this);
}

bool TacticalButtonClass::Draw(bool forced)
{
	/*if (!this->ControlClass::Draw(forced))
		return false;*/

	auto pSurface = DSurface::Composite();
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

	if (pSuper->IsCharged && !pCurrent->CanTransactMoney(pSWExt->Money_Amount) || (pSWExt->SW_UseAITargeting && !AresFunctions::IsTargetConstraintsEligible(AresFunctions::SWTypeExtMap_Find(pSuper->Type), HouseClass::CurrentPlayer, true)))
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
		pSurface->DrawSHP(FileSystem::SIDEBAR_PAL, FileSystem::GCLOCK2_SHP, pSuper->GetCameoChargeState() + 1, &loc, &bounds, BlitterFlags::bf_400 | BlitterFlags::TransLucent50, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
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
	this->IsHovering = true;
	TacticalButtonClass::CurrentButton = this;
}

void TacticalButtonClass::OnMouseLeave()
{
	this->IsHovering = false;
	this->IsPressed = false;
	TacticalButtonClass::CurrentButton = nullptr;
	CCToolTip::Instance->MarkToRedraw(CCToolTip::Instance->CurrentToolTipData);
}

bool TacticalButtonClass::Action(GadgetFlag flags, DWORD* pKey, KeyModifier modifier)
{
	if (!this->ControlClass::Action(flags, pKey, modifier))
		return false;

	if ((int)flags & (int)GadgetFlag::LeftPress)
		this->IsPressed = true;

	if ((int)flags & (int)GadgetFlag::LeftRelease && this->IsPressed)
	{
		this->IsPressed = false;
		return this->LaunchSuper(this->SuperIndex);
	}

	return true;
}

bool TacticalButtonClass::LaunchSuper(int superIdx)
{
	const auto pCurrent = HouseClass::CurrentPlayer();
	const auto pSuper = pCurrent->Supers[superIdx];
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
	else if (!pSWExt->SW_UseAITargeting || AresFunctions::IsTargetConstraintsEligible(AresFunctions::SWTypeExtMap_Find(pSuper->Type), HouseClass::CurrentPlayer, true))
	{
		if (!manual && !unstopable)
		{
			const auto swIndex = pSuper->Type->ArrayIndex;

			if (pSuper->Type->Action == Action::None || pSWExt->SW_UseAITargeting)
			{
				EventClass Event = EventClass(pCurrent->ArrayIndex, EventType::SPECIAL_PLACE, swIndex, CellStruct::Empty);
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

bool TacticalButtonClass::AddButton(int superIdx)
{
	TacticalButtonClass::Initialized = true;

	if (!Phobos::UI::ExclusiveSuperWeaponSidebar || Unsorted::ArmageddonMode)
		return false;

	auto& buttons = TacticalButtonClass::Buttons;

	if (std::any_of(buttons.begin(), buttons.end(), [superIdx](TacticalButtonClass* const button) { return button->SuperIndex == superIdx; }))
		return false;

	DLLCreate<TacticalButtonClass>(superIdx + 2200, superIdx, 0, 0, 60, 48);
	SortButtons();
	return true;
}

bool TacticalButtonClass::RemoveButton(int superIdx)
{
	auto& buttons = TacticalButtonClass::Buttons;

	if (buttons.empty())
		return false;

	const auto it = std::find_if(buttons.begin(), buttons.end(), [superIdx](TacticalButtonClass* const button) { return button->SuperIndex == superIdx; });

	if (it == buttons.end())
		return false;

	DLLDelete(*it);
	SortButtons();
	return true;
}

void TacticalButtonClass::ClearButtons()
{
	TacticalButtonClass::CurrentButton = nullptr;
	auto& buttons = TacticalButtonClass::Buttons;

	if (buttons.empty())
		return;

	for (const auto button : buttons)
		DLLDelete(button);

	buttons.clear();
}

void TacticalButtonClass::SortButtons()
{
	auto& buttons = TacticalButtonClass::Buttons;

	if (buttons.empty())
		return;

	std::stable_sort(buttons.begin(), buttons.end(), [](TacticalButtonClass* const a, TacticalButtonClass* const b)
		{
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

DEFINE_HOOK(0x692419, DisplayClass_ProcessClickCoords_TacticalButton, 0x7)
{
	return TacticalButtonClass::CurrentButton ? 0x6925FC : 0;
}

DEFINE_HOOK(0x4AE51E, DisplayClass_GetToolTip_TacticalButton, 0x6)
{
	enum { ApplyToolTip = 0x4AE69D };

	const auto button = TacticalButtonClass::CurrentButton;

	if (!button)
		return 0;

	PhobosToolTip::Instance.IsCameo = true;
	const auto pSuper = HouseClass::CurrentPlayer->Supers[button->SuperIndex];
	PhobosToolTip::Instance.HelpText(pSuper->Type);
	R->EAX(PhobosToolTip::Instance.GetBuffer());
	return ApplyToolTip;
}

DEFINE_HOOK(0x72426F, ToolTipManager_ProcessMessage_TacticalButton, 0x5)
{
	if (TacticalButtonClass::CurrentButton)
		R->EDX(0);

	return 0;
}

DEFINE_HOOK(0x72428C, ToolTipManager_ProcessMessage_TacticalButton2, 0x5)
{
	return TacticalButtonClass::CurrentButton ? 0x724297 : 0;
}

DEFINE_HOOK(0x724B2E, ToolTipManager_SetX_TacticalButtons, 0x6)
{
	if (const auto button = TacticalButtonClass::CurrentButton)
	{
		R->EDX(button->X + button->Width);
		R->EAX(button->Y + 27);
	}

	return 0;
}

DEFINE_HOOK(0x6CB7BA, SuperClass_Lose_UpdateTacticalButton, 0x6)
{
	GET(SuperClass*, pSuper, ECX);

	if (pSuper->Owner == HouseClass::CurrentPlayer)
		TacticalButtonClass::RemoveButton(pSuper->Type->ArrayIndex);

	return 0;
}

DEFINE_HOOK(0x6A6300, SidebarClass_AddCameo_SuperWeapon_TacticalButton, 0x6)
{
	enum { SkipGameCode = 0x6A6606 };

	if (!Phobos::UI::ExclusiveSuperWeaponSidebar)
		return 0;

	GET_STACK(AbstractType, whatAmI, 0x4);
	GET_STACK(int, index, 0x8);

	switch (whatAmI)
	{
	case AbstractType::Super:
	case AbstractType::SuperWeaponType:
	case AbstractType::Special:
		if (const auto pSWType = SuperWeaponTypeClass::Array->GetItemOrDefault(index))
		{
			const auto pSWExt = SWTypeExt::ExtMap.Find(pSWType);

			if (pSWExt->AllowInExclusiveSidebar && pSWExt->SW_ShowCameo.Get(!pSWExt->SW_AutoFire))
			{
				TacticalButtonClass::AddButton(index);
				R->AL(false);
				return SkipGameCode;
			}
		}
		break;

	default:
		break;
	}

	return 0;
}

DEFINE_HOOK(0x6A5030, SidebarClass_Init_Clear_InitializedTacticalButton, 0x6)
{
	TacticalButtonClass::Initialized = false;
	TacticalButtonClass::ClearButtons();
	return 0;
}

DEFINE_HOOK(0x55B6B3, LogicClass_AI_InitializedTacticalButton, 0x5)
{
	if (TacticalButtonClass::Initialized)
		return 0;

	TacticalButtonClass::Initialized = true;
	const auto pCurrent = HouseClass::CurrentPlayer();

	if (!pCurrent || pCurrent->Defeated)
		return 0;

	for (const auto pSuper : pCurrent->Supers)
	{
		if (!pSuper->Granted)
			continue;

		const auto pSWExt = SWTypeExt::ExtMap.Find(pSuper->Type);

		if (pSWExt->AllowInExclusiveSidebar && pSWExt->SW_ShowCameo.Get(!pSWExt->SW_AutoFire))
			TacticalButtonClass::AddButton(pSuper->Type->ArrayIndex);
	}

	return 0;
}
