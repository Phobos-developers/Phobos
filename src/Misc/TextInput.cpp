#include <Utilities\Macro.h>
#include <Utilities\Debug.h>

DEFINE_LJMP(0x55E484, 0x55E48D); // Allow message entry in Skirmish

wchar_t ToCyrillic(wchar_t result)
{
	int lang = LOWORD(GetKeyboardLayout(NULL));

	if (lang == 0x0419 || lang == 0x0422)
	{
		if (result >= 0xC0 && result <= 0xFF) // A-я
			result += 0x350;

		if (result == 0xA8) // Ё
			result = 0x401;

		if (result == 0xB8) // ё
			result = 0x451;
	}

	return result;
}

DEFINE_HOOK(0x5D46C7, MessageListClass_Input, 5)
{
	R->EBX(ToCyrillic(R->EBX()));
	return 0;
}

DEFINE_HOOK(0x61526C, WWUI__NewEditCtrl, 5)
{
	R->EDI(ToCyrillic(R->EDI()));
	return 0;
}
