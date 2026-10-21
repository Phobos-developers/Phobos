#include <BitFont.h>
#include <Surface.h>

#include <Phobos.h>

#include <Utilities/Macro.h>

/**
 * The scenario briefing body is painted by the engine helper at 0x621040, which
 * wraps the text into a fixed box and stops drawing once the accumulated line
 * height reaches the box height, so a long localized briefing gets clipped at
 * the bottom of the box.
 *
 * The hook below keeps the engine's own line breaking and layout intact and
 * only lifts the limits of that call: inside the briefing dialog the body is
 * laid out within the main area (everything left of the option menu) the way
 * the dialog defines it, elsewhere the wrapping width is widened to what the
 * current resolution offers. The height/fade limits no longer cut the text off,
 * and the clip rectangle the caller installed is opened up to the whole surface
 * and restored as soon as the engine unlocks the font at the end of the call.
 *
 * Approach based on the briefing text handling of MO Experience Enhancements
 * (https://github.com/MagicShiba/MO_ExperienceEnhancements).
 */

namespace BriefingText
{
	// BitText::DrawText (0x434CD0) takes all ten parameters on the stack. Offsets
	// are relative to the ESP snapshot Syringe takes at the hook point, therefore
	// offset 0 is the return address and the parameters follow.
	enum DrawTextParam
	{
		ParamReturnAddress = 0x0,
		ParamFont = 0x4,
		ParamSurface = 0x8,
		ParamText = 0xC,
		ParamPosX = 0x10,
		ParamPosY = 0x14,
		ParamBoxWidth = 0x18,
		ParamBoxHeight = 0x1C,
		ParamFlags = 0x20,
		ParamFadeStart = 0x24,
		ParamFadeLength = 0x28
	};

	// 0x621040 paints the briefing body and calls DrawText from 0x62113F, the
	// instruction after which is 0x621144. The same helper is used for small
	// controls (buttons, list entries, scrollbar page numbers), which must keep
	// the vanilla box, hence the size limits.
	constexpr DWORD BriefingBodyCaller = 0x621144;
	constexpr int MinBoxWidth = 400;
	constexpr int MinBoxHeight = 100;

	// Kept free on the right side of the surface so that glyphs never end up at
	// the very edge of the pixel buffer.
	constexpr int SurfaceMargin = 8;

	// Percentage of the vanilla side and top margins that is kept inside the
	// briefing dialog. Lower values give the text more room, higher values restore
	// the vanilla spacing.
	constexpr int SideMarginPercent = 80;
	constexpr int TopMarginPercent = 80;

	struct ClipRestore
	{
		BitFont* Target { nullptr };
		LTRBStruct Bounds {};
	};

	ClipRestore PendingClipRestore;
}

DEFINE_HOOK(0x434CD0, BitText_DrawText_UnclipBriefing, 0x6)
{
	using namespace BriefingText;

	// Recover from a call that returned before reaching BitFont::UnLock, so the
	// widened clip rectangle never leaks into unrelated text.
	if (PendingClipRestore.Target)
	{
		PendingClipRestore.Target->Bounds = PendingClipRestore.Bounds;
		PendingClipRestore.Target = nullptr;
	}

	const auto pFont = R->Stack<BitFont*>(ParamFont);
	const auto pSurface = R->Stack<Surface*>(ParamSurface);

	if (!pFont || !pSurface || !R->Stack<const wchar_t*>(ParamText))
		return 0;

	if (R->Stack<DWORD>(ParamReturnAddress) != BriefingBodyCaller)
		return 0;

	const int boxWidth = R->Stack<int>(ParamBoxWidth);
	const int boxHeight = R->Stack<int>(ParamBoxHeight);

	if (boxWidth < MinBoxWidth || boxHeight < MinBoxHeight)
		return 0;

	const int surfaceWidth = pSurface->GetWidth();
	const int surfaceHeight = pSurface->GetHeight();

	// Sanity limits: a wrong surface pointer would otherwise produce a huge clip
	// rectangle.
	if (surfaceWidth < 64 || surfaceWidth > 8192 || surfaceHeight < 64 || surfaceHeight > 8192)
		return 0;

	int posX = R->Stack<int>(ParamPosX);
	int posY = R->Stack<int>(ParamPosY);

	// Default wrapping width: everything from the box origin to the right edge of
	// the surface. Used for the loading screen, where the box comes from the map
	// and must be respected.
	int wrapWidth = surfaceWidth - posX - SurfaceMargin;

	// Inside the briefing dialog the body sits in the main area, i.e. everything
	// left of the option menu, and the vanilla box is centred in it: the left and
	// right margins are equal, so the area is twice the left margin plus the box
	// itself. That keeps the derivation tied to the dialog layout the engine hands
	// over, without any resource lookup or hardcoded panel width, and it scales
	// with the resolution because the dialog does.
	if (Game::SpecialDialog == 9)
	{
		// The main area ends where the option menu begins; the vanilla box is
		// centred in it, so twice its left margin plus the box itself is the area
		// width. Clamped to the surface for safety.
		int mainWidth = posX * 2 + boxWidth;

		if (mainWidth > surfaceWidth)
			mainWidth = surfaceWidth;

		if (mainWidth > 0)
		{
			// Shave the vanilla side and top margins a little so that more text
			// fits, keeping the body horizontally centred in the main area. The
			// bottom is left unbounded further down.
			const int paddingX = posX * SideMarginPercent / 100;

			posX = paddingX;
			posY = posY * TopMarginPercent / 100;
			wrapWidth = mainWidth - paddingX * 2;
		}
	}

	if (wrapWidth > surfaceWidth)
		wrapWidth = surfaceWidth;

	// Never wrap narrower than the vanilla box, so the text can only get shorter.
	if (wrapWidth < boxWidth)
		wrapWidth = boxWidth;

	// Height: DrawText returns once the accumulated line height reaches this
	// value, which is exactly what clips the briefing. Let it run to the bottom
	// of the surface instead.
	int maxHeight = surfaceHeight - posY;

	if (maxHeight < boxHeight)
		maxHeight = boxHeight;

	R->Stack<int>(ParamPosX, posX);
	R->Stack<int>(ParamPosY, posY);
	R->Stack<int>(ParamBoxWidth, wrapWidth);
	R->Stack<int>(ParamBoxHeight, maxHeight);

	// FadeStart makes DrawText drop everything past the given character and fade
	// the tail out; the briefing body is meant to be shown as one block. Zero
	// disables it entirely (and the fade length cannot be reached with it).
	R->Stack<int>(ParamFadeStart, 0);

	// The caller clips the glyphs to the box it installed beforehand. Open the
	// clip rectangle to the whole surface and restore it on unlock.
	PendingClipRestore.Target = pFont;
	PendingClipRestore.Bounds = pFont->Bounds;

	pFont->Bounds.Left = 0;
	pFont->Bounds.Top = 0;
	pFont->Bounds.Right = surfaceWidth - 1;
	pFont->Bounds.Bottom = surfaceHeight - 1;

	return 0;
}

DEFINE_HOOK(0x434990, BitText_UnLock_RestoreBriefingClip, 0x7)
{
	GET(BitFont*, pFont, ECX);

	if (BriefingText::PendingClipRestore.Target != pFont)
		return 0;

	pFont->Bounds = BriefingText::PendingClipRestore.Bounds;
	BriefingText::PendingClipRestore.Target = nullptr;

	return 0;
}
