#include <Utilities\Macro.h>
#include <Utilities\Debug.h>

DEFINE_LJMP(0x55E484, 0x55E48D); // Allow message entry in Skirmish

UINT GetCodepage()
{
	char szLCData[6 + 1];
	WORD lang = LOWORD(GetKeyboardLayout(NULL));
	LCID locale = MAKELCID(lang, SORT_DEFAULT);
	GetLocaleInfoA(locale, LOCALE_IDEFAULTANSICODEPAGE, szLCData, _countof(szLCData));

	return atoi(szLCData);
}

wchar_t LocalizeSymbol(char character)
{
	wchar_t result;
	UINT codepage = GetCodepage();
	MultiByteToWideChar(codepage, MB_USEGLYPHCHARS, &character, 1, &result, 1);
	return result;
}

DEFINE_HOOK(0x5D46C7, MessageListClass_Input, 5)
{
	R->EBX<wchar_t>(LocalizeSymbol(R->EBX<char>()));
	return 0;
}

DEFINE_HOOK(0x61526C, WWUI__NewEditCtrl, 5)
{
	R->EDI<wchar_t>(LocalizeSymbol(R->EDI<char>()));
	return 0;
}
