
#include "AdvancedCommandBarButtons.h"


#include "DistributionMode.h"

#include <ShapeButtonClass.h>
#include "Commands.h"

#include <Utilities/Macro.h>

void AdvancedCommandBarButton::RegisterButtons()
{
	// Vanilla buttons
	AdvancedCommandBarButton::TotalButtonCount = AdvancedCommandBarButton::OldButtonCount;

	// New button register here
	AdvancedCommandBarButton::Array.push_back(std::make_unique<DistributionModeHoldDownButtonClass>());
}

ShapeButtonClass* AdvancedCommandBarButton::GetShapeButton(const char* name)
{
	for (const auto& ptr : AdvancedCommandBarButton::Array)
	{
		AdvancedCommandBarButton* pNewButton = ptr.get();
		if (_strcmpi(pNewButton->GetName(), name) == 0)
		{
			return ShapeButtonClass::GetButton(pNewButton->ID);
		}
	}

	return nullptr;
}

#pragma region Hooks

DEFINE_HOOK(0x6CFD08, AdvancedCommandBarClass_GetButtonIdxByName_NewButton, 0x5)
{
	enum { SetButtonIndex = 0x6CFD0D };

	GET(const char*, name, ECX);

	for (const auto& ptr : AdvancedCommandBarButton::Array)
	{
		AdvancedCommandBarButton* pButton = ptr.get();
		if (_strcmpi(name, pButton->GetName()) == 0)
		{
			R->EAX(pButton->ID);
			return SetButtonIndex;
		}
	}

	return 0;
}

DEFINE_HOOK(0x6D0233, AdvancedCommandBarClass_Init_InitNewButtonIndex, 0x6)
{
	AdvancedCommandBarButton::RegisterButtons();
	return 0;
}

DEFINE_HOOK(0x6D0827, AdvancedCommandBarClass_Update_UpdateNewButton, 0x6)
{
	GET(const int, index, EAX);

	for (const auto& ptr : AdvancedCommandBarButton::Array)
	{
		AdvancedCommandBarButton* pNewButton = ptr.get();

		if (pNewButton->ID == index)
		{
			pNewButton->Execute(ShapeButtonClass::GetButton(index)->IsOn);
			break;
		}
	}

	return 0;
}

DEFINE_HOOK(0x6D10DF, AdvancedCommandBarClass_InitButtonIO_InitNewHoldDownButton, 0x6)
{
	for (const auto& ptr : AdvancedCommandBarButton::Array)
	{
		AdvancedCommandBarButton* pNewButton = ptr.get();

		if (!pNewButton->CanHoldDown())
			continue;

		if (auto pButton = ShapeButtonClass::GetButton(pNewButton->ID))
		{
			pButton->ToggleType = 1;
			pButton->UseFlash = true;
		}
	}

	return 0;
}

DEFINE_HOOK(0x6D14DD, AdvancedCommandBarClass_InitToolTip_InitNewButtonToolTip, 0x5)
{
	for (const auto& ptr : AdvancedCommandBarButton::Array)
	{
		AdvancedCommandBarButton* pNewButton = ptr.get();
		ShapeButtonClass::SetToolTip(ShapeButtonClass::GetButton(pNewButton->ID), pNewButton->GetTipName());
	}

	return 0;
}

#pragma endregion
