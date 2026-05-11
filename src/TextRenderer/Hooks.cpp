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
