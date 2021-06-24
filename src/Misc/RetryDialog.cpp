#include <Helpers/Macro.h>
#include <WWMessageBox.h>
#include <LoadOptionsClass.h>
#include <Themes.h>

DEFINE_HOOK(686092, RetryDialog_SaveLoad, 7)
{
	// GET_STACK(WWMessageBox, messageBox, STACK_OFFS(0x98, 0x84));

	while (true)
	{
		// WWMessageBox
		// Button1
		// Button3
		// Button2
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
			if (!LoadOptionsClass().LoadDialog())
				continue;
			ThemePlayer::Instance->Stop();
			break;
		}
		break;
	}

	PUSH_IMM(1);

	return 0x686395;
}