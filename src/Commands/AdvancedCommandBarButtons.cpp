
#include "AdvancedCommandBarButtons.h"

#include "Commands.h"
#include "DistributionMode.h"

#include <ShapeButtonClass.h>

#include <Utilities/Macro.h>

#pragma region ShapeButton

int ShapeButtonHelper::NewButtonIndexes[ShapeButtonHelper::NewButtonCount] =
{
	-1 // DistributionMode
	// New button initialize here
};

DEFINE_HOOK(0x6CFD08, ShapeButtonClass_FindIndex_FindNewButton, 0x5)
{
	enum { SetButtonIndex = 0x6CFD0D };

	GET(const char*, name, ECX);

	for (int i = 0; i < ShapeButtonHelper::NewButtonCount; ++i)
	{
		if (_strcmpi(name, ShapeButtonHelper::NewButtonNames[i]) == 0)
		{
			R->EAX(i + ShapeButtonHelper::OldButtonCount);
			return SetButtonIndex;
		}
	}

	return 0;
}

DEFINE_HOOK(0x6D0233, TabClass_Init_InitNewButtonIndex, 0x6)
{
	for (int i = 0; i < ShapeButtonHelper::NewButtonCount; ++i)
		ShapeButtonHelper::NewButtonIndexes[i] = ShapeButtonClass::FindIndex(ShapeButtonHelper::NewButtonNames[i]);

	return 0;
}

DEFINE_HOOK(0x6D0827, TabClass_Update_UpdateNewButton, 0x6)
{
	GET(const int, index, EAX);

	if (ShapeButtonHelper::NewButtonIndexes[0] == index)
	{
		if (ShapeButtonClass::GetButton(index)->IsOn)
			DistributionModeHoldDownCommandClass::DistributionModeOn();
		else
			DistributionModeHoldDownCommandClass::DistributionModeOff();
	}

	// New button trigger here

	return 0;
}

DEFINE_HOOK(0x6D10DF, TabClass_InitButtonIO_InitNewHoldDownButton, 0x6)
{
	const int distributionModeButtonIndex = ShapeButtonHelper::NewButtonIndexes[0];

	if (distributionModeButtonIndex != -1)
	{
		if (const auto pButton = ShapeButtonClass::GetButton(distributionModeButtonIndex))
		{
			// Clicking the button is different from holding down the hotkey
			pButton->ToggleType = 1;
			pButton->UseFlash = true;
		}
	}

	return 0;
}

DEFINE_HOOK(0x6D14DD, TabClass_InitToolTip_InitNewButtonToolTip, 0x5)
{
	for (int i = 0; i < ShapeButtonHelper::NewButtonCount; ++i)
		ShapeButtonClass::SetToolTip(ShapeButtonClass::GetButton(ShapeButtonHelper::NewButtonIndexes[i]), ShapeButtonHelper::NewButtonTipNames[i]);

	return 0;
}

#pragma endregion
