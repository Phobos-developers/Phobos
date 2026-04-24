
#include <utility>
#include <Helpers/Macro.h>
#include "TextRenderer.h"
#include <CCINIClass.h>
#include <BitFont.h>
#include <BitText.h>
#include <Phobos.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/Debug.h>

static bool IsTTFEnabled()
{
	static CCINIClass ini_uimd;
	ini_uimd.LoadFromFile(GameStrings::UIMD_INI);
	return ini_uimd.ReadBool("Render", "EnableTTF", false);
}
// Best fix Unicode
DEFINE_HOOK(0x5D3BA0, sub_433F50, 7)
{

	GET_STACK(const wchar_t*, pText, 0xC);
	std::wstring wtext = TextRenderer::FixUtf8InWchar(pText);
	R->Stack(0xC, wtext.c_str());

	return 0;

}

// 43393A
DEFINE_HOOK(0x433880, BitFont_CTOR, 8)
{

	if (IsTTFEnabled())
		return 0;
	GET(BitFont*, pFont, ECX);
	CCINIClass ini_uimd {};
	ini_uimd.LoadFromFile(GameStrings::UIMD_INI);
	ini_uimd.ReadString("Font", "FileName", "default.ttf", Phobos::readBuffer);
	std::string fontPath = std::string("Fonts\\") + Phobos::readBuffer;
	pFont = TextRenderer::BitFont_CTOR_(pFont, fontPath.c_str());

	return  pFont ? 0x43393A : 0;
}


DEFINE_HOOK(0x433CF0, BitFont_GetTextDimension, 8)
{
	if (IsTTFEnabled())
		return 0;
	GET(BitFont*, pFont, ECX);
	GET_STACK(const wchar_t*, pText, 0x4);
	GET_STACK(int*, pWidth, 0x8);
	GET_STACK(int*, pHeight, 0xC);
	GET_STACK(int, nMaxWidth, 0x10);
	std::wstring arabicShaped;


	R->EAX((DWORD)TextRenderer::BitFont_GetTextDimension_(pFont, pText, pWidth, pHeight, nMaxWidth));

	return 0x433EA2;
}
DEFINE_HOOK(0x434CD0, BitText_DrawText, 10)
{
	if (IsTTFEnabled())
		return 0;
	GET_STACK(BitFont*, pFont, 0x4);
	GET_STACK(Surface*, pSurface, 0x8);
	GET_STACK(const wchar_t*, pWideString, 0xC);
	GET_STACK(int, X, 0x10);
	GET_STACK(int, Y, 0x14);
	GET_STACK(int, W, 0x18);
	GET_STACK(int, H, 0x1C);
	GET_STACK(int, a8, 0x20);
	GET_STACK(int, a9, 0x24);
	GET_STACK(int, nColorAdjust, 0x28);

	bool handled = TextRenderer::BitText_DrawText_(pFont, pSurface, pWideString, X, Y, W, H, a8, a9, nColorAdjust);

	return handled ? 0x435310 : 0;
}

DEFINE_HOOK(0x434500, sub_434500, 7)
{
	if (IsTTFEnabled())
		return 0;
	GET(BitFont*, pFont, ECX);
	GET_STACK(wchar_t*, pText, 0x4);
	GET_STACK(int, xLeft, 0x8);
	GET_STACK(int, yTop, 0xC);
	GET_STACK(int, charCount, 0x10);
	GET_STACK(int, nColorAdjust, 0x14);

	if (!TextRenderer::GetFTFace(pFont))
		return 0;

	R->EAX(TextRenderer::BitFont_434500_(pFont, pText, xLeft, yTop, charCount, nColorAdjust));
	return 0x4346B4;
}



