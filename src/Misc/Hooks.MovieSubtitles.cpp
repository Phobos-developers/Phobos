#include <Phobos.h>
#include <Surface.h>
#include <Helpers/Macro.h>

#include <algorithm>

DEFINE_HOOK(0x432FBC, MoviePlayer_BinkCopyToBuffer_Flags, 0xA)
{
	GET(unsigned int, flags, ECX);
	const bool bSubtitleChanged = (R->EBX<unsigned int>() & 0xFF) != 0;

	if (bSubtitleChanged || Phobos::UI::MovieSubtitles_Background)
		flags |= 0x80000000;

	R->ECX(flags);
	return 0x432FC6;
}

DEFINE_HOOK(0x433320, MoviePlayer_DrawSubtitles_Background, 0x6)
{
	GET(Surface*, pSurface, ECX);
	GET(RectangleStruct*, pDestRect, EAX);

	if (Phobos::UI::MovieSubtitles_Background && pSurface && pDestRect && pDestRect->Width > 0 && pDestRect->Height > 0)
	{
		const int padX = Phobos::UI::MovieSubtitles_BackgroundPaddingX;
		const int padY = Phobos::UI::MovieSubtitles_BackgroundPaddingY;

		RectangleStruct bgRect = {
			std::max(0, pDestRect->X - padX),
			std::max(0, pDestRect->Y - padY),
			pDestRect->Width + padX * 2,
			pDestRect->Height + padY * 2
		};

		pSurface->FillRectTrans(&bgRect, &Phobos::UI::MovieSubtitles_BackgroundColor, Phobos::UI::MovieSubtitles_BackgroundOpacity);
	}

	R->ECX(pSurface);
	R->EAX(pDestRect);
	return 0;
}

DEFINE_HOOK(0x4333C9, MoviePlayer_ClearSubtitles_Padding, 0x6)
{
	if (!Phobos::UI::MovieSubtitles_Background)
		return 0;

	GET_STACK(RectangleStruct*, pClearRect, 0x0);

	if (!pClearRect)
		return 0;

	const int padX = Phobos::UI::MovieSubtitles_BackgroundPaddingX;
	const int padY = Phobos::UI::MovieSubtitles_BackgroundPaddingY;

	pClearRect->X = std::max(0, pClearRect->X - padX);
	pClearRect->Y = std::max(0, pClearRect->Y - padY);
	pClearRect->Width += padX * 2;
	pClearRect->Height += padY * 2;

	return 0;
}
