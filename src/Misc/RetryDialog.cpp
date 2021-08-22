#include <Helpers/Macro.h>
#include <WWMessageBox.h>
#include <LoadOptionsClass.h>
#include <ThemeClass.h>

namespace RetryDialogFlag
{
	bool IsCalledFromRetryDialog = false;
}

DEFINE_HOOK(0x686092, DoLose_RetryDialogForCampaigns, 0x7)
{
	while (true)
	{
		// WWMessageBox buttons look like below:
		// Button1
		// Button3
		// Button2
		// I prefer to put the loadgame to the center of them - secsome
		switch (WWMessageBox::Instance().Process(
			StringTable::LoadString("TXT_TO_REPLAY"),
			StringTable::LoadString("TXT_OK"),
			StringTable::LoadString("GUI:LOADGAME"),
			StringTable::LoadString("TXT_CANCEL"))
		)
		{
		case WWMessageBox::Result::Button1: // ok
			return 0x6860F6;
		default:
		case WWMessageBox::Result::Button3: // cancel
			return 0x6860EE;
		case WWMessageBox::Result::Button2: // load game
			auto pDialog = GameCreate<LoadOptionsClass>();
			RetryDialogFlag::IsCalledFromRetryDialog = true;
			const bool bIsAboutToLoad = pDialog->LoadDialog();
			RetryDialogFlag::IsCalledFromRetryDialog = false;
			GameDelete(pDialog);

			if (!bIsAboutToLoad)
				continue;
			
			ThemeClass::Instance->Stop();
			break;
		}
		break;
	}

	PUSH_IMM(1); // For the stack

	return 0x686395;
}

DEFINE_HOOK(0x558F4E, LoadOptionClass_Dialog_CenterListBox, 0x5)
{
	if (RetryDialogFlag::IsCalledFromRetryDialog)
	{
		GET(HWND, hListBox, EAX);
		GET(HWND, hDialog, EDI);

		HWND hLoadButton = GetDlgItem(hDialog, 1039);

		RECT buttonRect;
		GetWindowRect(hLoadButton, &buttonRect);

		float scaleX = static_cast<float>(buttonRect.right - buttonRect.left) / 108;
		float scaleY = static_cast<float>(buttonRect.bottom - buttonRect.top) / 22;
		int X = buttonRect.left - static_cast<int>(346 * scaleX);
		int Y = buttonRect.top - static_cast<int>(44 * scaleY);

		SetWindowPos(hListBox, NULL, X, Y, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER);
	}

	return 0;
}