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
	if (this->SuperVisible && pMousePosition->X < 65 && pMousePosition->X >= 5) // Button index 1-10 : Super weapons buttons
	{
		const int currentCounts = this->SWButtonData.size();
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

	// TODO New buttons (Start from index = 11)

	if (CheckMouseOverBackground(pMousePosition))
		return 0; // Button index 0 : Background

	return -1;
}

bool TacticalButtonClass::CheckMouseOverBackground(const Point2D* pMousePosition)
{
	if (RulesExt::Global()->SWSidebarBackground && this->SuperVisible)
	{
		if (const int currentCounts = this->SWButtonData.size())
		{
			if (pMousePosition->X < 80 && pMousePosition->X >= 0)
			{
				const int height = DSurface::Composite->GetHeight();
				const int checkHight = (height - 32 - 48 * currentCounts - 2 * (currentCounts - 1)) / 2 - 21;

				if (pMousePosition->Y >= checkHight && pMousePosition->Y < (checkHight + currentCounts * 50 + 40))
					return true;
			}
		}
	}

	// TODO New button backgrounds

	return false;
}

// Inline functions
inline bool TacticalButtonClass::MouseIsOverButtons()
{
	return this->ButtonIndex > 0;
}

inline bool TacticalButtonClass::MouseIsOverTactical()
{
	return this->ButtonIndex < 0;
}

// Cite functions
int TacticalButtonClass::GetButtonIndex()
{
	return this->ButtonIndex;
}

void TacticalButtonClass::RecheckButtonIndex()
{
	this->LastPosition = Point2D { -1, -1 };
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
		const int index = Instance.SWButtonData[this->ButtonIndex - 1];
		SuperClass* const pSuper = pHouse->Supers.Items[index];

		if (pSuper != this->RecordSuper)
		{
			PhobosToolTip::Instance.HelpText(pSuper);
			this->RecordSuper = pSuper;
		}
	}
	else if (this->RecordSuper)
	{
		this->RecordSuper = nullptr;
	}
}

void TacticalButtonClass::PressDesignatedButton(int triggerIndex)
{
	const int buttonIndex = this->ButtonIndex;

	if (buttonIndex <= 0) // In buttons background
		return;

	if (buttonIndex <= 10) // Button index 1-10 : Super weapons buttons
	{
		if (!triggerIndex)
			TriggerButtonForSW(buttonIndex);
		else if (triggerIndex == 2)
			DisplayClass::Instance->CurrentSWTypeIndex = -1;
	}
/*	else if (?) // TODO New buttons (Start from index = 11)
	{
		;
	}*/
}

// SW buttons functions
void TacticalButtonClass::DrawButtonForSW()
{
	if (!this->SuperVisible)
		return;

	const int currentCounts = this->SWButtonData.size();

	if (!currentCounts)
		return;

	HouseClass* const pHouse = HouseClass::CurrentPlayer;
	SideExt::ExtData* const pSideExt = SideExt::ExtMap.Find(SideClass::Array->GetItem(pHouse->SideIndex));
	const bool drawSWSidebarBackground = RulesExt::Global()->SWSidebarBackground && pSideExt;

	auto& data = this->SWButtonData;
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
	for (int i = 0; i < currentCounts; position.Y += 50, rect.Height += 50) // Button index 1-10
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

		// Draw cameo
		if (SWTypeExt::ExtData* const pTypeExt = SWTypeExt::ExtMap.Find(pSWType))
		{
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

		if (++i == this->ButtonIndex)
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
	RecheckButtonIndex();
	HouseClass* const pHouse = HouseClass::CurrentPlayer;
	auto& data = this->SWButtonData;

	for (auto it = data.begin(); it != data.end();)
	{
		const int superIndex = *it;

		if (superIndex >= pHouse->Supers.Count || !pHouse->Supers.Items[superIndex]->IsPresent)
			it = data.erase(it);
		else
			++it;
	}
}

bool TacticalButtonClass::InsertButtonForSW(int& superIndex)
{
	SuperWeaponTypeClass* const pType = SuperWeaponTypeClass::Array->Items[superIndex];
	SWTypeExt::ExtData* const pTypeExt = SWTypeExt::ExtMap.Find(pType);
	bool overflow = true;

	if (pTypeExt && pTypeExt->SW_InScreen_Show)
	{
		const unsigned int ownerBits = 1u << HouseClass::CurrentPlayer->Type->ArrayIndex;

		if (pTypeExt->SW_InScreen_RequiredHouses & ownerBits)
		{
			const int currentCounts = this->SWButtonData.size();
			auto& data = this->SWButtonData;
			bool move = false;

			for (int i = 0; i < 10; ++i) // 10 buttons at max
			{
				if (move)
				{
					if (i < currentCounts)
					{
						int buffer = data[i];
						data[i] = superIndex;
						superIndex = buffer;
					}
					else
					{
						data.push_back(superIndex);
						overflow = false;
						break;
					}
				}
				else if (i < currentCounts)
				{
					if (SortButtonForSW(SuperWeaponTypeClass::Array->Items[data[i]], pType, pTypeExt, ownerBits))
					{
						move = true;
						int buffer = data[i];
						data[i] = superIndex;
						superIndex = buffer;
					}
				}
				else
				{
					data.push_back(superIndex);
					overflow = false;
					break;
				}
			}
		}
	}

	return overflow;
}

bool TacticalButtonClass::SortButtonForSW(SuperWeaponTypeClass* pDataType, SuperWeaponTypeClass* pAddType, SWTypeExt::ExtData* pAddTypeExt, unsigned int ownerBits)
{
	SWTypeExt::ExtData* const pDataTypeExt = SWTypeExt::ExtMap.Find(pDataType);

	if (pDataTypeExt && pAddTypeExt)
	{
		if ((pDataTypeExt->SW_InScreen_PriorityHouses & ownerBits) && !(pAddTypeExt->SW_InScreen_PriorityHouses & ownerBits))
			return false;

		if (!(pDataTypeExt->SW_InScreen_PriorityHouses & ownerBits) && (pAddTypeExt->SW_InScreen_PriorityHouses & ownerBits))
			return true;

		if (pDataTypeExt->CameoPriority > pAddTypeExt->CameoPriority)
			return false;

		if (pDataTypeExt->CameoPriority < pAddTypeExt->CameoPriority)
			return true;
	}

	if (pDataType->RechargeTime < pAddType->RechargeTime)
		return false;

	if (pDataType->RechargeTime > pAddType->RechargeTime)
		return true;

	return wcscmp(pDataType->UIName, pAddType->UIName) > 0;
}

void TacticalButtonClass::TriggerButtonForSW(int buttonIndex)
{
	if (ScenarioClass::Instance->UserInputLocked || !this->SuperVisible)
		return;

	const int superIndex = this->SWButtonData[buttonIndex - 1];

	if (superIndex < 0)
		return;

	SidebarClass* const pSidebar = SidebarClass::Instance;
	this->DummyAction = true;

	DummySelectClass pButton;
	pButton.LinkTo = &pSidebar->Tabs[pSidebar->ActiveTabIndex];
	pButton.unknown_int_30 = 0x7FFFFFFF;
	pButton.SWIndex = superIndex;

	DWORD KeyNum = 0;
	reinterpret_cast<bool(__thiscall*)(DummySelectClass*, GadgetFlag, DWORD*, KeyModifier)>(0x6AAD00)(&pButton, GadgetFlag::LeftPress, &KeyNum, KeyModifier::None); // SelectClass_Action
}

// Hooks

// Mouse trigger hooks
DEFINE_HOOK(0x6931A5, ScrollClass_WindowsProcedure_PressLeftMouseButton, 0x6)
{
	enum { SkipGameCode = 0x6931B4 };

	TacticalButtonClass* const pButtons = &TacticalButtonClass::Instance;

	if (!pButtons->MouseIsOverTactical())
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

	if (!pButtons->MouseIsOverTactical())
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

	if (pButtons->MouseIsOverTactical())
		return 0;

	R->Stack(STACK_OFFSET(0x30, -0x24), 0);
	R->EAX(Action::None);
	return SkipGameCode;
}

// Buttons display hooks
DEFINE_HOOK(0x6D4941, TacticalClass_Render_DrawButtonCameo, 0x6)
{
	// TODO New buttons (The later draw, the higher layer)

	TacticalButtonClass::Instance.DrawButtonForSW();
	return 0;
}

// SW buttons hooks
DEFINE_HOOK(0x4F9283, HouseClass_Update_RecheckTechTree, 0x5)
{
	TacticalButtonClass::Instance.RecheckButtonForSW();
	return 0;
}

DEFINE_HOOK(0x6A6314, SidebarClass_AddCameo_SupportSWButtons, 0x8)
{
	enum { SkipGameCode = 0x6A65F5 };

	GET_STACK(const AbstractType, absType, STACK_OFFSET(0x14, 0x4));
	REF_STACK(int, index, STACK_OFFSET(0x14, 0x8));

	return (absType != AbstractType::Special || SuperWeaponTypeClass::Array->Count <= index || TacticalButtonClass::Instance.InsertButtonForSW(index)) ? 0 : SkipGameCode;
}

DEFINE_HOOK(0x6AAF46, SelectClass_Action_ButtonClick1, 0x6)
{
	enum { SkipGameCode = 0x6AB95A };

	GET(const int, index, ESI);

	SuperWeaponTypeClass* const pType = SuperWeaponTypeClass::Array->Items[index];
	SWTypeExt::ExtData* const pTypeExt = SWTypeExt::ExtMap.Find(pType);
	TacticalButtonClass* const pButtons = &TacticalButtonClass::Instance;
	bool keyboardCall = false;

	if (pButtons->KeyboardCall)
	{
		pButtons->KeyboardCall = false;
		keyboardCall = true;
	}

	if (pTypeExt)
	{
		if (pTypeExt->SW_QuickFireAtMouse && keyboardCall)
		{
			const CoordStruct mouseCoords = TacticalClass::Instance->ClientToCoords(WWMouseClass::Instance->XY1);

			if (mouseCoords != CoordStruct::Empty)
			{
				EventClass event
				(
					HouseClass::CurrentPlayer->ArrayIndex,
					EventType::SpecialPlace,
					pType->ArrayIndex,
					CellClass::Coord2Cell(mouseCoords)
				);
				EventClass::AddEvent(event);

				return SkipGameCode;
			}
		}
		else if (!pTypeExt->SW_QuickFireInScreen)
		{
			return 0;
		}

		EventClass event
		(
			HouseClass::CurrentPlayer->ArrayIndex,
			EventType::SpecialPlace,
			pType->ArrayIndex,
			CellClass::Coord2Cell(TacticalClass::Instance->ClientToCoords(Point2D{ (DSurface::Composite->Width >> 1), (DSurface::Composite->Height >> 1) }))
		);
		EventClass::AddEvent(event);

		return SkipGameCode;
	}

	return 0;
}

DEFINE_HOOK_AGAIN(0x6AAD2F, SelectClass_Action_ButtonClick2, 0x7)
DEFINE_HOOK(0x6AB94F, SelectClass_Action_ButtonClick2, 0xB)
{
	enum { SkipGameCode = 0x6AAE7C };

	if (!TacticalButtonClass::Instance.DummyAction)
		return 0;

	GET(TacticalButtonClass::DummySelectClass* const , pThis, EDI);

	R->Stack(STACK_OFFSET(0xAC, -0x98), pThis->SWIndex);
	return SkipGameCode;
}

DEFINE_HOOK(0x6AB961, SelectClass_Action_ButtonClick3, 0x7)
{
	enum { SkipGameCode = 0x6AB975 };

	TacticalButtonClass* const pButtons = &TacticalButtonClass::Instance;

	if (!pButtons->DummyAction)
		return 0;

	pButtons->DummyAction = false;

	return SkipGameCode;
}
