#include <Helpers/Macro.h>
#include "TextRenderer.h"
#include <BitFont.h>
#include <BitText.h>
#include <Surface.h>
#include <Utilities/Debug.h>
#include <MessageListClass.h>
#include <TextLabelClass.h>
#include <ColorScheme.h>

DEFINE_HOOK(0x433CF0, BitFont_GetTextDimension, 8)
{
    GET(BitFont*, pFont, ECX);
    GET_STACK(const wchar_t*, pText, 0x4);
    GET_STACK(int*, pWidth, 0x8);
    GET_STACK(int*, pHeight, 0xC);
    GET_STACK(int, nMaxWidth, 0x10);
    if (TextRenderer::GetTextDimension(pFont, pText, pWidth, pHeight, nMaxWidth))
    { R->EAX(1); return 0x433EA2; }
    return 0;
}

DEFINE_HOOK(0x434CD0, BitText_DrawText, 10)
{
    GET_STACK(BitFont*, pFont, 0x4);
    GET_STACK(Surface*, pSurface, 0x8);
    GET_STACK(const wchar_t*, pWideString, 0xC);
    GET_STACK(int, X, 0x10); GET_STACK(int, Y, 0x14);
    GET_STACK(int, W, 0x18); GET_STACK(int, H, 0x1C);
    GET_STACK(int, a8, 0x20);
    if (TextRenderer::DrawText(pFont, pSurface, pWideString, X, Y, W, H, a8))
        return 0x435310;
    return 0;
}

DEFINE_HOOK(0x5D49A0, MessageListClass_Draw_TTF, 6)
{
    GET(MessageListClass*, pThis, ECX);

    if (!pThis || !pThis->MessageList)
        return 0x5D4AA0;

    BitFont* pFont = BitFont::Instance;
    if (!pFont) return 0x5D4AA0;

    for (TextLabelClass* pLabel = pThis->MessageList; pLabel;
         pLabel = static_cast<TextLabelClass*>(pLabel->Next))
    {
        if (!pLabel->Text || !*pLabel->Text || pLabel->SkipDraw)
            continue;

        // Get color
        pFont->Color = 0x7FFF;
        if (ColorScheme* pScheme = ColorScheme::Array.GetItem(pLabel->ColorSchemeIndex))
            pFont->Color = Drawing::RGB_To_Int(pScheme->BaseColor.R, pScheme->BaseColor.G, pScheme->BaseColor.B);

        // Background
        int tw = 0, th = 0;
        TextRenderer::GetTextDimension(pFont, pLabel->Text, &tw, &th, 0);
        if (tw > 0 && th > 0)
        {
            RectangleStruct bg = { pLabel->X - 4, pLabel->Y - 2, tw + 12, th + 8 };
            DSurface::Composite->FillRect(&bg, COLOR_BLACK);
        }

        // Draw TTF text
        TextRenderer::DrawText(pFont, DSurface::Composite, pLabel->Text, pLabel->X, pLabel->Y, 0, 0, 0);
    }

    return 0x5D4AA0;
}
