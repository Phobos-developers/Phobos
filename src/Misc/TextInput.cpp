#include <Utilities\Macro.h>

// Allow message entry in Skirmish
// DEFINE_JUMP(LJMP, 0x55E484, 0x55E48D);

wchar_t* IMEBuffer = reinterpret_cast<wchar_t*>(0xB730EC);

UINT GetCurentCodepage()
{
	char szLCData[6 + 1];
	WORD lang = LOWORD(GetKeyboardLayout(NULL));
	LCID locale = MAKELCID(lang, SORT_DEFAULT);
	GetLocaleInfoA(locale, LOCALE_IDEFAULTANSICODEPAGE, szLCData, _countof(szLCData));

	return atoi(szLCData);
}

wchar_t LocalizeCharacter(char character)
{
	wchar_t result;
	UINT codepage = GetCurentCodepage();
	MultiByteToWideChar(codepage, MB_USEGLYPHCHARS, &character, 1, &result, 1);
	return result;
}

DEFINE_HOOK(0x5D46C7, MessageListClass_Input, 0x5)
{
	if (!IMEBuffer[0])
		R->EBX<wchar_t>(LocalizeCharacter(R->EBX<char>()));

	return 0;
}

DEFINE_HOOK(0x61510E, WWUI_NewEditCtrl, 0x7)
{
	R->EDI<wchar_t>(LocalizeCharacter(R->EBX<char>()));
	return 0x615226;
}

// It is required to add Imm32.lib to AdditionalDependencies
/*
HIMC& IMEContext = *reinterpret_cast<HIMC*>(0xB7355C);
wchar_t* IMECompositionString = reinterpret_cast<wchar_t*>(0xB73318);

DEFINE_HOOK(0x777F15, IMEUpdateCompositionString, 0x7)
{
	IMECompositionString[0] = 0;
	ImmGetCompositionStringW(IMEContext, GCS_COMPSTR, IMECompositionString, 256);

	return 0;
}
*/
