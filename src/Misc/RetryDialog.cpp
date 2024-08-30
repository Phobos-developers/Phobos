#include <Helpers/Macro.h>
#include <WWMessageBox.h>
#include <LoadOptionsClass.h>
#include <ThemeClass.h>
#include <CCToolTip.h>
#include <GameOptionsClass.h>
#include <GScreenClass.h>
#include <EvadeClass.h>

namespace RetryDialogFlag
{
	bool IsCalledFromRetryDialog = false;
}

DEFINE_HOOK(0x686092, DoLose_RetryDialogForCampaigns, 0x7)
{
	enum { OK = 0x6860F6, Cancel = 0x6860EE, LoadGame = 0x686231 };

	while (true)
	{
		// WWMessageBox buttons look like below:
		// Button1
		// Button3
		// Button2
		// I prefer to put the loadgame to the center of them - secsome
		// Did you??? NO, YOU DIDN'T. Bruhhhh
		switch (WWMessageBox::Instance->Process(
			GameStrings::TXT_TO_REPLAY,
			"GUI:LOADGAME",
			GameStrings::TXT_CANCEL,
			GameStrings::TXT_OK))
		{
		case WWMessageBox::Result::Button3:
			return OK;

		default:
		case WWMessageBox::Result::Button2:
			return Cancel;

		case WWMessageBox::Result::Button1:
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

	EvadeClass::Instance->Do();

	if (CCToolTip::Instance())
		CCToolTip::Instance->SetState(GameOptionsClass::Instance->Tooltips);

	GScreenClass::Instance->Render();

	return LoadGame;
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
