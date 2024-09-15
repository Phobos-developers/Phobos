#include <GameOptionsClass.h>
#include <EventClass.h>
#include <SuperClass.h>
#include <MessageListClass.h>
#include <Ext/Side/Body.h>
#include <Ext/House/Body.h>
#include <Ext/SWType/Body.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/AresHelper.h>
#include <TacticalClass.h>
#include <WWMouseClass.h>

#include "PhobosToolTip.h"
#include "TacticalButtons.h"

TacticalButtonClass TacticalButtonClass::Instance;

// Functions

// Private functions
int TacticalButtonClass::CheckMouseOverButtons(const Point2D* pMousePosition)
{
	if (pMousePosition->X < 65 && pMousePosition->X >= 5) // Button index 1-9 : Super weapons buttons
	{
		const int currentCounts = HouseExt::ExtMap.Find(HouseClass::CurrentPlayer)->SuperWeaponButtonCount;
		const int height = DSurface::Composite->GetHeight();
		int checkHight = (height - 32 - 48 * currentCounts - 2 * (currentCounts - 1)) / 2;

		for (int i = 0; i < currentCounts; ++i)
		{
			if (pMousePosition->Y < checkHight)
				break;

			checkHight += 48;

			if (pMousePosition->Y < checkHight)
				return i + 1;

			checkHight += 2;
		}
	}

	// TODO New buttons (Start from index = 10)

	if (CheckMouseOverBackground(pMousePosition))
		return 0; // Button index 0 : Background

	return -1;
}

bool TacticalButtonClass::CheckMouseOverBackground(const Point2D* pMousePosition)
{
	if (RulesExt::Global()->SWSidebarBackground && pMousePosition->X < 80 && pMousePosition->X >= 0)
	{
		const int currentCounts = HouseExt::ExtMap.Find(HouseClass::CurrentPlayer)->SuperWeaponButtonCount;
		const int height = DSurface::Composite->GetHeight();
		const int checkHight = (height - 32 - 48 * currentCounts - 2 * (currentCounts - 1)) / 2 - 21;

		if (pMousePosition->Y >= checkHight && pMousePosition->Y < (checkHight + currentCounts * 50 + 40))
			return true;
	}

	// TODO New button backgrounds

	return false;
}

// Inline functions
inline bool TacticalButtonClass::MouseOverButtons()
{
	return this->ButtonIndex > 0;
}

inline bool TacticalButtonClass::MouseOverTactical()
{
	return this->ButtonIndex < 0;
}

// Cite functions
int TacticalButtonClass::GetButtonIndex()
{
	return this->ButtonIndex;
}

// General functions
void TacticalButtonClass::SetMouseButtonIndex(const Point2D* pMousePosition)
{
	if (this->LastPosition == *pMousePosition)
		return;

	this->LastPosition = *pMousePosition;
	this->ButtonIndex = CheckMouseOverButtons(pMousePosition);

	// SW ToolTip
	if (this->ButtonIndex > 0)
	{
		HouseClass* const pHouse = HouseClass::CurrentPlayer;
		const int index = HouseExt::ExtMap.Find(pHouse)->SuperWeaponButtonData[this->ButtonIndex - 1];
		SuperClass* const pSuper = pHouse->Supers.Items[index];

		if (pSuper != this->pRecordSuper)
		{
			PhobosToolTip::Instance.HelpText(pSuper);
			this->pRecordSuper = pSuper;
		}
	}
	else if (this->pRecordSuper)
	{
		this->pRecordSuper = nullptr;
	}
}

void TacticalButtonClass::PressDesignatedButton(int triggerIndex)
{
	const int buttonIndex = this->ButtonIndex;

	if (!buttonIndex) // In buttons background
		return;

	if (buttonIndex <= 9) // Button index 1-9 : Super weapons buttons
	{
		if (!triggerIndex)
			TriggerButtonForSW(buttonIndex);
		else if (triggerIndex == 2)
			DisplayClass::Instance->CurrentSWTypeIndex = -1;
	}
/*	else if (?) // TODO New buttons (Start from index = 10)
	{
		;
	}*/
}

// SW buttons functions
void TacticalButtonClass::DrawButtonForSW()
{
	HouseClass* const pHouse = HouseClass::CurrentPlayer;
	HouseExt::ExtData* const pHouseExt = HouseExt::ExtMap.Find(pHouse);
	SideExt::ExtData* const pSideExt = SideExt::ExtMap.Find(SideClass::Array->GetItem(pHouse->SideIndex));
	const int currentCounts = pHouseExt->SuperWeaponButtonCount;
	const bool drawSWSidebarBackground = RulesExt::Global()->SWSidebarBackground;

	if (!currentCounts)
		return;

	auto& data = pHouseExt->SuperWeaponButtonData;
	const int height = DSurface::Composite->GetHeight();
	const int color = Drawing::RGB_To_Int(Drawing::TooltipColor);

	Point2D position { 5, (height - 32 - 48 * currentCounts - 2 * (currentCounts - 1)) / 2 };
	RectangleStruct rect { 0, 0, 65, position.Y + 48 };
	int recordHeight = -1;

	// Draw top background (80 * 20)
	Point2D backPosition { 0, position.Y - 21 };

	if (drawSWSidebarBackground)
	{
		if (BSurface* const CameoPCX = pSideExt->SWSidebarBackground_TopPCX.GetSurface())
		{
			RectangleStruct drawRect { backPosition.X, backPosition.Y, 60, 48 };
			PCX::Instance->BlitToSurface(&drawRect, DSurface::Composite, CameoPCX);
		}
		else
		{
			RectangleStruct backRect { 0, backPosition.Y, 80, 20};
			DSurface::Composite->FillRect(&backRect, COLOR_BLACK);
		}
	}

	// Draw each buttons
	for (int i = 0; i < currentCounts; position.Y += 50, rect.Height += 50) // Button index 1-9
	{
		// Draw center background (80 * 50)
		if (drawSWSidebarBackground)
		{
			backPosition.Y = position.Y - 1;

			if (BSurface* const CameoPCX = pSideExt->SWSidebarBackground_CenterPCX.GetSurface())
			{
				RectangleStruct drawRect { backPosition.X, backPosition.Y, 60, 48 };
				PCX::Instance->BlitToSurface(&drawRect, DSurface::Composite, CameoPCX);
			}
			else
			{
				RectangleStruct backRect { 0, backPosition.Y, 80, 50};
				DSurface::Composite->FillRect(&backRect, COLOR_BLACK);
			}
		}

		// Get SW data
		SuperClass* const pSuper = pHouse->Supers.Items[data[i]];
		SuperWeaponTypeClass* const pSWType = pSuper->Type;
		SWTypeExt::ExtData* const pTypeExt = SWTypeExt::ExtMap.Find(pSWType);

		// Draw cameo
		BSurface* const CameoPCX = pTypeExt->SidebarPCX.GetSurface();

		if (CAN_USE_ARES && AresHelper::CanUseAres && CameoPCX)
		{
			RectangleStruct drawRect { position.X, position.Y, 60, 48 };
			PCX::Instance->BlitToSurface(&drawRect, DSurface::Composite, CameoPCX);
		}
		else if (SHPStruct* const pSHP = pSWType->SidebarImage)
		{
			DSurface::Composite->DrawSHP(FileSystem::CAMEO_PAL, pSHP, 0, &position, &rect,
				BlitterFlags::bf_400, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
		}

		// Flash cameo
		const int delay = pSWType->FlashSidebarTabFrames;

		if (delay > 0 && !pSuper->IsSuspended && (pSuper->IsReady || (pSWType->UseChargeDrain && pSuper->ChargeDrainState != ChargeDrainState::Charging))
			&& ((Unsorted::CurrentFrame - pSuper->ReadyFrame) % (delay << 1)) > delay)
		{
			DSurface::Composite->DrawSHP(FileSystem::SIDEBAR_PAL, Make_Global<SHPStruct*>(0xB07BC0), 0, &position, &rect,
				BlitterFlags(0x406), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
		}

		// SW charge progress
		if (pSuper->ShouldDrawProgress())
		{
			const int frame = pSuper->AnimStage() + 1;

			DSurface::Composite->DrawSHP(FileSystem::SIDEBAR_PAL, Make_Global<SHPStruct*>(0xB0B484), frame, &position, &rect,
				BlitterFlags(0x404), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
		}

		// SW status
		if (const wchar_t* pName = pSuper->NameReadiness())
		{
			Point2D textLocation { 35, position.Y + 1 };
			const TextPrintType printType = TextPrintType::Center | TextPrintType::FullShadow | TextPrintType::Point8;
			RectangleStruct textRect = Drawing::GetTextDimensions(pName, textLocation, static_cast<WORD>(printType), 2, 1);

			// Text black background
			reinterpret_cast<void(__fastcall*)(RectangleStruct*, DSurface*, unsigned short, unsigned char)>(0x621B80)(&textRect, DSurface::Composite, 0, 0xAFu);
			DSurface::Composite->DrawTextA(pName, &rect, &textLocation, static_cast<COLORREF>(color), COLOR_BLACK, printType);
		}

		if (++i == Instance.ButtonIndex)
			recordHeight = position.Y;
	}

	// Draw bottom background (80 * 20)
	if (drawSWSidebarBackground)
	{
		backPosition.Y = position.Y - 1;

		if (BSurface* const CameoPCX = pSideExt->SWSidebarBackground_BottomPCX.GetSurface())
		{
			RectangleStruct drawRect { backPosition.X, backPosition.Y, 60, 48 };
			PCX::Instance->BlitToSurface(&drawRect, DSurface::Composite, CameoPCX);
		}
		else
		{
			RectangleStruct backRect { 0, backPosition.Y, 80, 20};
			DSurface::Composite->FillRect(&backRect, COLOR_BLACK);
		}
	}

	// Draw mouse hover rectangle
	if (!ScenarioClass::Instance->UserInputLocked && recordHeight >= 0)
	{
		position.Y = recordHeight;
		rect.Height = recordHeight + 48;

		RectangleStruct drawRect { 5, position.Y, 60, 48 };
		DSurface::Composite->DrawRectEx(&rect, &drawRect, color);
	}
}

void TacticalButtonClass::RecheckButtonForSW()
{
	Instance.LastPosition = Point2D::Empty;
	HouseClass* const pHouse = HouseClass::CurrentPlayer;
	HouseExt::ExtData* const pHouseExt = HouseExt::ExtMap.Find(pHouse);
	auto& data = pHouseExt->SuperWeaponButtonData;

	for (int i = 0; i < pHouseExt->SuperWeaponButtonCount; ++i)
	{
		if (data[i] >= pHouse->Supers.Count || !pHouse->Supers.Items[data[i]]->IsPresent)
		{
			const int counts = pHouseExt->SuperWeaponButtonCount - 1;

			for (int j = i; j < counts; ++j)
			{
				data[j] = data[j + 1];
			}

			data[--pHouseExt->SuperWeaponButtonCount] = -1;
		}
	}
}

bool TacticalButtonClass::InsertButtonForSW(int& superIndex)
{
	SuperWeaponTypeClass* const pType = SuperWeaponTypeClass::Array->Items[superIndex];
	SWTypeExt::ExtData* const pTypeExt = SWTypeExt::ExtMap.Find(pType);
	bool overflow = true;

	if (pTypeExt->SW_InScreen_Show && !pTypeExt->SW_UseAITargeting)
	{
		HouseClass* const pHouse = HouseClass::CurrentPlayer;
		HouseExt::ExtData* const pHouseExt = HouseExt::ExtMap.Find(pHouse);
		const unsigned int ownerBits = 1u << pHouse->Type->ArrayIndex;

		if (pTypeExt->SW_InScreen_RequiredHouses & ownerBits)
		{
			auto& data = pHouseExt->SuperWeaponButtonData;
			bool move = false;

			for (int i = 0; i < 9; ++i) // 9 buttons at max
			{
				if (move)
				{
					if (data[i] != -1)
					{
						int buffer = data[i];
						data[i] = superIndex;
						superIndex = buffer;
					}
					else
					{
						data[i] = superIndex;
						overflow = false;
						++pHouseExt->SuperWeaponButtonCount;
						break;
					}
				}
				else if (data[i] != -1)
				{
					if (MoveButtonForSW(SuperWeaponTypeClass::Array->Items[data[i]], pType, pTypeExt, ownerBits))
					{
						move = true;
						int buffer = data[i];
						data[i] = superIndex;
						superIndex = buffer;
					}
				}
				else
				{
					data[i] = superIndex;
					overflow = false;
					++pHouseExt->SuperWeaponButtonCount;
					break;
				}
			}
		}
	}

	return overflow;
}

bool TacticalButtonClass::MoveButtonForSW(SuperWeaponTypeClass* pDataType, SuperWeaponTypeClass* pAddType, SWTypeExt::ExtData* pAddTypeExt, unsigned int ownerBits)
{
	SWTypeExt::ExtData* const pDataTypeExt = SWTypeExt::ExtMap.Find(pDataType);

	if ((pDataTypeExt->SW_InScreen_PriorityHouses & ownerBits) && !(pAddTypeExt->SW_InScreen_PriorityHouses & ownerBits))
		return false;
	else if (!(pDataTypeExt->SW_InScreen_PriorityHouses & ownerBits) && (pAddTypeExt->SW_InScreen_PriorityHouses & ownerBits))
		return true;
	else if (pDataTypeExt->CameoPriority > pAddTypeExt->CameoPriority)
		return false;
	else if (pDataTypeExt->CameoPriority < pAddTypeExt->CameoPriority)
		return true;
	else if (pDataType->RechargeTime < pAddType->RechargeTime)
		return false;
	else if (pDataType->RechargeTime > pAddType->RechargeTime)
		return true;

	return wcscmp(pDataType->UIName, pAddType->UIName) > 0;
}

void TacticalButtonClass::TriggerButtonForSW(int buttonIndex)
{
	if (ScenarioClass::Instance->UserInputLocked)
		return;

	HouseClass* const pHouse = HouseClass::CurrentPlayer;
	const int index = HouseExt::ExtMap.Find(pHouse)->SuperWeaponButtonData[buttonIndex - 1];

	if (index < 0) // Keyboard shortcuts may be invalid
		return;

	SuperClass* const pSuper = pHouse->Supers.Items[index];
	SuperWeaponTypeClass* const pType = pSuper->Type;
	SWTypeExt::ExtData* const pTypeExt = SWTypeExt::ExtMap.Find(pType);

	if (CAN_USE_ARES && AresHelper::CanUseAres)
	{
		const bool autoFire = !pTypeExt->SW_ManualFire && pTypeExt->SW_AutoFire;

		if (!pSuper->CanFire() && !autoFire)
		{
			VoxClass::PlayIndex(pTypeExt->EVA_Impatient);
			return;
		}

		if (!pHouse->CanTransactMoney(pTypeExt->Money_Amount))
		{
			if (pTypeExt->EVA_InsufficientFunds != -1)
				VoxClass::PlayIndex(pTypeExt->EVA_InsufficientFunds);
			else
				VoxClass::Play(&Make_Global<const char>(0x819044)); // 0x819044 -> EVA_InsufficientFunds

			const CSFText csf = pTypeExt->Message_InsufficientFunds;

			if (!csf.empty())
			{
				int color = ColorScheme::FindIndex("Gold");

				if (pTypeExt->Message_FirerColor)
				{
					if (pHouse)
						color = pHouse->ColorSchemeIndex;
				}
				else
				{
					if (pTypeExt->Message_ColorScheme > -1)
						color = pTypeExt->Message_ColorScheme;
					else if (pHouse)
						color = pHouse->ColorSchemeIndex;
				}

				MessageListClass::Instance->PrintMessage(csf, RulesClass::Instance->MessageDelay, color);
			}

			return;
		}

		const bool unstoppable = pType->UseChargeDrain && pSuper->ChargeDrainState == ChargeDrainState::Draining
			&& pTypeExt->SW_Unstoppable;

		if (autoFire || unstoppable)
			return;
	}
	else if (!pSuper->CanFire())
	{
		return;
	}

	// Use SW.InScreen.QuickFire here, to some extent to replace SW.UseAITargeting
	// Inevitably ignore various conditions such as range, sight, indicators .etc
	// No check for SW.UseAITargeting, too complicated for me. - CrimRecya
	if (pType->Action == Action::None)
	{
		EventClass event
		(
			pHouse->ArrayIndex,
			EventType::SpecialPlace,
			pType->ArrayIndex,
			CellStruct::Empty
		);
		EventClass::AddEvent(event);
	}
	else if (pTypeExt->SW_InScreen_QuickFire)
	{
		EventClass event
		(
			pHouse->ArrayIndex,
			EventType::SpecialPlace,
			pType->ArrayIndex,
			CellClass::Coord2Cell(TacticalClass::Instance->ClientToCoords(Point2D{ (DSurface::Composite->Width >> 1), (DSurface::Composite->Height >> 1) }))
		);
		EventClass::AddEvent(event);
	}
	else
	{
		MouseClass* const pMouse = MouseClass::Instance;
		pMouse->CurrentBuilding = nullptr;
		pMouse->CurrentBuildingType = nullptr;
		pMouse->unknown_11AC = 0xFFFFFFFF;
		pMouse->SetActiveFoundation(nullptr);
		pMouse->SetRepairMode(0);
		pMouse->SetSellMode(0);
		pMouse->PowerToggleMode = false;
		pMouse->PlanningMode = false;
		pMouse->PlaceBeaconMode = false;
		pMouse->CurrentSWTypeIndex = index;
		pMouse->UnselectAll();

		if (CAN_USE_ARES && AresHelper::CanUseAres && pTypeExt->EVA_SelectTarget != -1)
			VoxClass::PlayIndex(pTypeExt->EVA_SelectTarget);
		else
			VoxClass::Play(&Make_Global<const char>(0x83FB78)); // 0x83FB78 -> EVA_SelectTarget
	}
}

// Hooks

// Mouse trigger hooks
DEFINE_HOOK(0x6931A5, ScrollClass_WindowsProcedure_PressLeftMouseButton, 0x6)
{
	enum { SkipGameCode = 0x6931B4 };

	TacticalButtonClass* const pButtons = &TacticalButtonClass::Instance;

	if (!pButtons->MouseOverTactical())
	{
		pButtons->PressedInButtonsLayer = true;
		pButtons->PressDesignatedButton(0);

		R->Stack(STACK_OFFSET(0x28, 0x8), 0);
		R->EAX(Action::None);
		return SkipGameCode;
	}

	return 0;
}

DEFINE_HOOK(0x693268, ScrollClass_WindowsProcedure_ReleaseLeftMouseButton, 0x5)
{
	enum { SkipGameCode = 0x693276 };

	TacticalButtonClass* const pButtons = &TacticalButtonClass::Instance;

	if (pButtons->PressedInButtonsLayer)
	{
		pButtons->PressedInButtonsLayer = false;
		pButtons->PressDesignatedButton(1);

		R->Stack(STACK_OFFSET(0x28, 0x8), 0);
		R->EAX(Action::None);
		return SkipGameCode;
	}

	return 0;
}

DEFINE_HOOK(0x69330E, ScrollClass_WindowsProcedure_PressRightMouseButton, 0x6)
{
	enum { SkipGameCode = 0x69334A };

	TacticalButtonClass* const pButtons = &TacticalButtonClass::Instance;

	if (!pButtons->MouseOverTactical())
	{
		pButtons->PressedInButtonsLayer = true;
		pButtons->PressDesignatedButton(2);

		return SkipGameCode;
	}

	return 0;
}

DEFINE_HOOK(0x693397, ScrollClass_WindowsProcedure_ReleaseRightMouseButton, 0x6)
{
	enum { SkipGameCode = 0x6933CB };

	TacticalButtonClass* const pButtons = &TacticalButtonClass::Instance;

	if (pButtons->PressedInButtonsLayer)
	{
		pButtons->PressedInButtonsLayer = false;
		pButtons->PressDesignatedButton(3);

		return SkipGameCode;
	}

	return 0;
}

// Mouse suspend hooks
DEFINE_HOOK(0x692F85, ScrollClass_MouseUpdate_SkipMouseLongPress, 0x7)
{
	enum { CheckMousePress = 0x692F8E, CheckMouseNoPress = 0x692FDC, SkipGameCode = 0x692FAE };

	GET(ScrollClass*, pThis, EBX);

	if (pThis->unknown_byte_554A) // 555A: AnyMouseButtonDown
		return !TacticalButtonClass::Instance.PressedInButtonsLayer ? CheckMousePress : SkipGameCode;

	return CheckMouseNoPress;
}

DEFINE_HOOK(0x69300B, ScrollClass_MouseUpdate_SkipMouseActionUpdate, 0x6)
{
	enum { SkipGameCode = 0x69301A };

	const Point2D mousePosition = WWMouseClass::Instance->XY1;
	TacticalButtonClass* const pButtons = &TacticalButtonClass::Instance;
	pButtons->SetMouseButtonIndex(&mousePosition);

	if (pButtons->MouseOverTactical())
		return 0;

	R->Stack(STACK_OFFSET(0x30, -0x24), 0);
	R->EAX(Action::None);
	return SkipGameCode;
}

// Buttons display hooks
DEFINE_HOOK(0x6D4941, TacticalClass_Render_DrawButtonCameo, 0x6)
{
	// TODO New buttons (The later draw, the higher layer)

	TacticalButtonClass::DrawButtonForSW();
	return 0;
}

// SW buttons hooks
DEFINE_HOOK(0x4F9283, HouseClass_Update_RecheckTechTree, 0x5)
{
	TacticalButtonClass::RecheckButtonForSW();
	return 0;
}

DEFINE_HOOK(0x6A6314, SidebarClass_AddCameo_SupportSWButtons, 0x8)
{
	enum { SkipGameCode = 0x6A65F5 };

	GET_STACK(const AbstractType, absType, STACK_OFFSET(0x14, 0x4));
	REF_STACK(int, index, STACK_OFFSET(0x14, 0x8));

	return (absType != AbstractType::Special || SuperWeaponTypeClass::Array->Count <= index || TacticalButtonClass::InsertButtonForSW(index)) ? 0 : SkipGameCode;
}
