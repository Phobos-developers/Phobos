#include "OwnerDraw.Internal.h"

static UINT GetStaticAnimationTimerInterval(HWND parentHwnd, HWND controlHwnd)
{
	int dialogId = 0;
	if (auto pParentData = FindOwnerDrawData(parentHwnd))
		dialogId = pParentData->DialogID;

	const int controlId = ::GetDlgCtrlID(controlHwnd);
	constexpr int AnimatedDialogIds[] =
	{
		148, 216, 245, 226, 213, 257, 297, 215, 187, 256,
		214, 293, 290, 274, 231, 278, 285, 284, 254, 271,
		279, 276, 230, 243, 244, 700, 270, 264, 3014, 3015
	};

	if (controlId == 1820
		&& std::find(std::begin(AnimatedDialogIds), std::end(AnimatedDialogIds), dialogId) != std::end(AnimatedDialogIds))
	{
		return 100;
	}

	if (dialogId == 148 && (controlId == 1770 || controlId == 1771 || controlId == 1772))
		return 100;

	if ((dialogId == 259 || dialogId == 3015) && controlId == 1835)
		return 100;

	if (dialogId == 196 && controlId == 1961)
		return 50;

	return 0;
}

static void DestroyStaticMovie(OwnerDrawDialogElement& data)
{
	auto pMovie = data.AsStatic().MovieHandle();
	if (!pMovie)
		return;

	if (pMovie->VTable && pMovie->VTable->Destructor)
		pMovie->VTable->Destructor(pMovie, 1);

	data.AsStatic().MovieHandle() = nullptr;
}

static void DestroyStaticMovieAux(OwnerDrawDialogElement& data)
{
	DeleteUnknownGameObject(data.AsStatic().MovieAuxHandle());
}

static void DetachStaticMovie(HWND hWnd, OwnerDrawDialogElement& data)
{
	DestroyStaticMovie(data);
	::KillTimer(hWnd, 0x65);
	DestroyStaticMovieAux(data);
}

static void PrepareStaticMovieLoad(HWND hWnd, OwnerDrawDialogElement& data)
{
	DestroyStaticMovie(data);
	::KillTimer(hWnd, 0x65);
	DestroyStaticMovieAux(data);
}

static LRESULT LoadStaticMovie(HWND hWnd, OwnerDrawDialogElement& data, const char* pMovieName)
{
	PrepareStaticMovieLoad(hWnd, data);

	RECT ownerRect {};
	OwnerDraw::GetRectangle(hWnd, &ownerRect);

	auto pMovie = OwnerDraw::InitMovieHandle(pMovieName, DSurface::Alternate, nullptr);
	data.AsStatic().MovieHandle() = pMovie;
	if (pMovie)
	{
		if (pMovie->VTable && pMovie->VTable->SetPosition)
			pMovie->VTable->SetPosition(pMovie, ownerRect.left, ownerRect.top);

		::MoveWindow(hWnd, ownerRect.left, ownerRect.top, pMovie->Width, pMovie->Height, FALSE);
		::SetTimer(hWnd, 0x65, 0x22, nullptr);
		return 0;
	}

	::KillTimer(hWnd, 0x65);
	DestroyStaticMovieAux(data);
	return 0;
}

static const char* GetStaticMovieName(int index)
{
	if (index < 0 || index >= MovieInfo::Array.Count)
		return nullptr;

	return MovieInfo::Array.Items[index].Name;
}

static bool EnsureStaticBackground(HWND hWnd, OwnerDrawDialogElement& data)
{
	if (data.AsStatic().CachedBackground() || !DSurface::Alternate)
		return data.AsStatic().CachedBackground() != nullptr;

	RECT ownerRect {};
	RECT clientRect {};
	OwnerDraw::GetRectangle(hWnd, &ownerRect);
	::GetClientRect(hWnd, &clientRect);

	const int width = clientRect.right + 1;
	const int height = clientRect.bottom + 1;
	if (width <= 0 || height <= 0)
		return false;

	data.AsStatic().CachedBackground() = GameCreate<BSurface>(width, height);
	if (!data.AsStatic().CachedBackground())
		return false;

	++OwnerDraw::CachedSurfaceCount;

	RectangleStruct destRect { 0, 0, width, height };
	RectangleStruct sourceRect { ownerRect.left, ownerRect.top, width, height };
	CopySurfacePart(data.AsStatic().CachedBackground(), destRect, DSurface::Alternate, sourceRect);
	return true;
}

static void RestoreStaticBackground(HWND hWnd, OwnerDrawDialogElement& data, bool inclusiveBounds)
{
	if (!data.AsStatic().CachedBackground() || !DSurface::Alternate)
		return;

	RECT ownerRect {};
	RECT clientRect {};
	OwnerDraw::GetRectangle(hWnd, &ownerRect);
	::GetClientRect(hWnd, &clientRect);

	const int width = clientRect.right + (inclusiveBounds ? 1 : 0);
	const int height = clientRect.bottom + (inclusiveBounds ? 1 : 0);
	if (width <= 0 || height <= 0)
		return;

	RectangleStruct destRect { ownerRect.left, ownerRect.top, width, height };
	RectangleStruct sourceRect { 0, 0, width, height };
	CopySurfacePart(DSurface::Alternate, destRect, data.AsStatic().CachedBackground(), sourceRect);
}

static void ResetStaticBackground(HWND hWnd, OwnerDrawDialogElement& data)
{
	if (data.AsStatic().CachedBackground())
		DeleteSurfaceObject(data.AsStatic().CachedBackground());

	::InvalidateRect(hWnd, nullptr, FALSE);
}

static void DrawStaticText(HWND hWnd, OwnerDrawDialogElement& data, const RECT& ownerRect)
{
	const auto pText = data.AsStatic().Text();
	if (!pText)
		return;

	const auto drawMode = data.AsStatic().DrawMode();
	if (drawMode != WWUIStaticDrawMode::Text && !data.AsStatic().AnimationRunning())
		return;

	const LONG windowStyle = ::GetWindowLongA(hWnd, GWL_STYLE);
	int textDrawStyle = 16;
	if (windowStyle & SS_CENTER)
		textDrawStyle = 17;
	else if (windowStyle & SS_RIGHT)
		textDrawStyle = 18;

	const COLORREF textColor = (windowStyle & WS_DISABLED)
		? Phobos::UI::ColorDisabledLabel
		: data.AsStatic().TextColor();

	RECT textRect = ownerRect;
	OwnerDraw::DrawWideText(
		DSurface::Alternate,
		pText,
		&textRect,
		data.AsStatic().Font(),
		textColor,
		textDrawStyle,
		data.AsStatic().TextFlags(),
		0,
		data.AsStatic().TextRevealCount(),
		data.AsStatic().ColorAdjust());

	if (data.AsStatic().SoundIndex() != -1)
		VocClass::PlayGlobal(data.AsStatic().SoundIndex(), 0x2000, 1.0f);

	if (drawMode == WWUIStaticDrawMode::TypewriterText)
	{
		const int revealLimit = static_cast<int>(std::wcslen(pText)) + data.AsStatic().ColorAdjust() + 1;
		if (data.AsStatic().TextRevealCount() < revealLimit)
		{
			data.AsStatic().TextRevealCount() += data.AsStatic().TextRevealStep();
			if (data.AsStatic().TextRevealCount() >= revealLimit)
				::KillTimer(hWnd, 0);
		}
	}
}

static bool DrawStaticPCX(HWND hWnd, OwnerDrawDialogElement& data, RectangleStruct imageRect)
{
	auto pImage = static_cast<BSurface*>(data.AsStatic().ImageSurface());
	if (!pImage)
		return false;

	const int imageWidth = pImage->GetWidth();
	const int imageHeight = pImage->GetHeight();
	if (imageRect.Width > imageWidth)
	{
		imageRect.X += (imageRect.Width - imageWidth) / 2;
		imageRect.Width = imageWidth;
	}

	if (imageRect.Height > imageHeight)
	{
		imageRect.Y += (imageRect.Height - imageHeight) / 2;
		imageRect.Height = imageHeight;
	}

	PCX::Instance.BlitToSurface(
		&imageRect,
		DSurface::Alternate,
		pImage,
		static_cast<WORD>(ConvertRGBToSurfaceColor(RGB(255, 0, 255))));

	::ValidateRect(hWnd, nullptr);
	return true;
}

static void DrawStaticShape(HWND hWnd, OwnerDrawDialogElement& data, const RectangleStruct& imageRect, bool animate)
{
	auto pShape = data.AsStatic().Shape();
	if (!pShape)
		return;

	int shapeX = imageRect.X;
	int shapeY = imageRect.Y;
	if (imageRect.Width > pShape->Width)
		shapeX += (imageRect.Width - pShape->Width) / 2;

	if (imageRect.Height > pShape->Height)
		shapeY += (imageRect.Height - pShape->Height) / 2;

	Point2D position { shapeX, shapeY };
	RectangleStruct bounds = DSurface::Alternate->GetRect();
	const int currentFrame = data.AsStatic().CurrentFrame();
	CC_Draw_Shape(
		DSurface::Alternate,
		data.AsStatic().ShapeDrawer(),
		pShape,
		currentFrame,
		&position,
		&bounds,
		BlitterFlags::bf_400,
		0,
		0,
		ZGradient::Ground,
		1000,
		0,
		nullptr,
		0,
		0,
		0);

	if (animate && data.AsStatic().AnimationRunning())
	{
		int nextFrame = currentFrame + 1;
		if (nextFrame >= data.AsStatic().FrameCount())
			nextFrame = 0;

		data.AsStatic().CurrentFrame() = nextFrame;
		if (data.AsStatic().FrameNotifyHwnd())
		{
			::SendMessageA(data.AsStatic().FrameNotifyHwnd(), WW_STATIC_ANIMFRAMENOTIFY, nextFrame, reinterpret_cast<LPARAM>(hWnd));
			::ValidateRect(hWnd, nullptr);
		}
	}
}

static LRESULT PaintStatic(HWND hWnd, OwnerDrawDialogElement& data)
{
	if (data.AsStatic().SuppressPaint() || data.AsStatic().MovieHandle())
	{
		::ValidateRect(hWnd, nullptr);
		return 0;
	}

	EnsureStaticBackground(hWnd, data);

	RECT ownerRect {};
	OwnerDraw::GetRectangle(hWnd, &ownerRect);

	if (data.AsStatic().FillBackground())
	{
		RectangleStruct fillRect
		{
			ownerRect.left,
			ownerRect.top,
			ownerRect.right - ownerRect.left,
			ownerRect.bottom - ownerRect.top
		};
		DSurface::Alternate->FillRect(&fillRect, ConvertRGBToSurfaceColor(data.AsStatic().FillColor()));
	}

	const auto drawMode = data.AsStatic().DrawMode();
	if (drawMode < WWUIStaticDrawMode::PCX)
	{
		DrawStaticText(hWnd, data, ownerRect);
	}
	else if ((drawMode == WWUIStaticDrawMode::PCX
		|| drawMode == WWUIStaticDrawMode::Shape
		|| drawMode == WWUIStaticDrawMode::AnimatedShape)
		&& static_cast<int>(::GetTickCount() - data.AsStatic().LastFrameTick()) > data.AsStatic().FrameDelayMs())
	{
		RestoreStaticBackground(hWnd, data, false);

		RectangleStruct imageRect
		{
			ownerRect.left,
			ownerRect.top,
			ownerRect.right - ownerRect.left,
			ownerRect.bottom - ownerRect.top
		};

		if (drawMode == WWUIStaticDrawMode::PCX)
		{
			if (DrawStaticPCX(hWnd, data, imageRect))
				return 0;
		}
		else
		{
			DrawStaticShape(hWnd, data, imageRect, drawMode == WWUIStaticDrawMode::AnimatedShape);
		}
	}

	::ValidateRect(hWnd, nullptr);
	return 0;
}

static void DestroyStaticResources(HWND hWnd, OwnerDrawDialogElement& data)
{
	if (data.AsStatic().CachedBackground())
		DeleteSurfaceObject(data.AsStatic().CachedBackground());

	if (data.AsStatic().OwnsShape() && data.AsStatic().Shape())
	{
		YRMemory::Deallocate(data.AsStatic().Shape());
		data.AsStatic().Shape() = nullptr;
	}

	const auto drawMode = data.AsStatic().DrawMode();
	if ((drawMode == WWUIStaticDrawMode::TypewriterText && data.AsStatic().AnimationRunning())
		|| drawMode == WWUIStaticDrawMode::AnimatedShape)
	{
		::KillTimer(hWnd, 0);
	}

	DestroyStaticMovie(data);
	::KillTimer(hWnd, 0x65);
	DestroyStaticMovieAux(data);
}

static LRESULT HandleStaticMovieTimer(HWND hWnd, OwnerDrawDialogElement& data)
{
	auto pMovie = data.AsStatic().MovieHandle();
	if (!pMovie)
		return 0;

	if (pMovie->VTable && pMovie->VTable->AdvanceFrame && pMovie->VTable->AdvanceFrame(pMovie))
		::InvalidateRect(hWnd, nullptr, FALSE);

	if (pMovie->VTable && pMovie->VTable->FramesLeft && pMovie->VTable->FramesLeft(pMovie))
	{
		if (data.AsStatic().LoopMovie())
		{
			if (pMovie->VTable->SeekToFrame)
				pMovie->VTable->SeekToFrame(pMovie, 1);

			return 0;
		}

		DetachStaticMovie(hWnd, data);
	}

	return 0;
}

static LRESULT HandleStaticVisualTimer(HWND hWnd, OwnerDrawDialogElement& data)
{
	const auto drawMode = data.AsStatic().DrawMode();
	if (drawMode == WWUIStaticDrawMode::TypewriterText)
	{
		::InvalidateRect(hWnd, nullptr, TRUE);
		return 0;
	}

	const bool singleShotVisualTimer = drawMode == WWUIStaticDrawMode::PCX || drawMode == WWUIStaticDrawMode::Shape;
	const bool animatedShapeTimer = drawMode == WWUIStaticDrawMode::AnimatedShape;
	if (!singleShotVisualTimer && !animatedShapeTimer)
		return 0;

	if (static_cast<int>(::GetTickCount() - data.AsStatic().LastFrameTick()) > data.AsStatic().FrameDelayMs())
	{
		if (singleShotVisualTimer || (animatedShapeTimer && data.AsStatic().AnimationRunning()))
		{
			::InvalidateRect(hWnd, nullptr, TRUE);
			if (singleShotVisualTimer)
			{
				::KillTimer(hWnd, 0);
				return 0;
			}
		}

		if (!data.AsStatic().FrameCount())
		{
			::KillTimer(hWnd, 0);
			if (auto pShape = data.AsStatic().Shape())
			{
				data.AsStatic().FrameCount() = pShape->Frames;
				::SetTimer(hWnd, 0, pShape->Frames, nullptr);
			}
		}
	}

	return 0;
}

LRESULT CALLBACK WWUI::StaticCtrl(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	auto pData = FindOwnerDrawData(hWnd);
	if (!pData)
		return 0;

	auto& data = *pData;
	auto forwardOriginal = [&]() -> LRESULT
	{
		const auto pOriginalWndProc = FindWindowProc(OwnerDraw::DialogProcs, hWnd);
		return CallSelectedHandler(pOriginalWndProc, hWnd, message, wParam, lParam);
	};

	switch (message)
	{
	case WW_INITDIALOG:
		data.AsStatic().DrawMode() = WWUIStaticDrawMode::Text;
		data.AsStatic().TextFlags() = 12;
		data.AsStatic().TextColor() = Phobos::UI::ColorTextLabel;
		return 0;

	case WM_PAINT:
		return PaintStatic(hWnd, data);

	case WM_MOVE:
	case WM_SIZE:
	case WM_WINDOWPOSCHANGED:
		ResetStaticBackground(hWnd, data);
		return 0;

	case WM_DESTROY:
		DestroyStaticResources(hWnd, data);
		return forwardOriginal();

	case WM_TIMER:
		if (wParam == 0x65)
			return HandleStaticMovieTimer(hWnd, data);

		return HandleStaticVisualTimer(hWnd, data);

	case WW_SETCOLOR:
	{
		const COLORREF newTextColor = lParam == static_cast<LPARAM>(-1)
			? Phobos::UI::ColorTextLabel
			: static_cast<COLORREF>(lParam);

		if (newTextColor != data.AsStatic().TextColor())
			::InvalidateRect(hWnd, nullptr, FALSE);

		data.AsStatic().TextColor() = newTextColor;
		return 0;
	}

	case WW_SETFILLCOLOR:
		data.AsStatic().FillBackground() = true;
		data.AsStatic().FillColor() = static_cast<COLORREF>(lParam);
		return 0;

	case WW_SETTEXTW:
	case WW_SETTEXTA:
		RestoreStaticBackground(hWnd, data, true);
		if (data.AsStatic().CachedBackground())
			::InvalidateRect(hWnd, nullptr, FALSE);

		return 1;

	case WW_RESETANIMTIMER:
		if (!data.AsStatic().Shape()
			|| data.AsStatic().DrawMode() != WWUIStaticDrawMode::AnimatedShape
			|| data.AsStatic().AnimationRunning())
		{
			return 0;
		}

		data.AsStatic().AnimationRunning() = true;
		::SetTimer(hWnd, 0, GetStaticAnimationTimerInterval(::GetParent(hWnd), hWnd), nullptr);
		return 0;

	case WW_STATIC_STOPANIM:
		if (data.AsStatic().DrawMode() == WWUIStaticDrawMode::AnimatedShape && data.AsStatic().AnimationRunning())
		{
			::KillTimer(hWnd, 0);
			data.AsStatic().AnimationRunning() = false;
		}
		return 0;

	case WW_STATIC_SETANIMFRAME:
		if (data.AsStatic().DrawMode() == WWUIStaticDrawMode::AnimatedShape)
		{
			data.AsStatic().CurrentFrame() = static_cast<int>(lParam);
			::InvalidateRect(hWnd, nullptr, TRUE);
		}
		return 0;

	case WW_STATIC_GETANIMFRAME:
		return data.AsStatic().DrawMode() == WWUIStaticDrawMode::AnimatedShape ? data.AsStatic().CurrentFrame() : -1;

	case WW_STATIC_SETANIMFRAMENOTIFYHWND:
		if (data.AsStatic().DrawMode() == WWUIStaticDrawMode::AnimatedShape)
			data.AsStatic().FrameNotifyHwnd() = reinterpret_cast<HWND>(lParam);
		return 0;

	case WW_STATIC_SETCURRENTMOVIEBYINDEX:
		if (const char* pMovieName = GetStaticMovieName(static_cast<int>(wParam)))
			return LoadStaticMovie(hWnd, data, pMovieName);

		return 0;

	case WW_STATIC_SETCURRENTMOVIEBYNAME:
		return LoadStaticMovie(hWnd, data, reinterpret_cast<const char*>(lParam));

	case WW_STATIC_PAUSEMOVIE:
		if (auto pMovie = data.AsStatic().MovieHandle())
		{
			if (pMovie->VTable && pMovie->VTable->Pause)
				pMovie->VTable->Pause(pMovie, 1);
		}
		return 0;

	case WW_STATIC_CONTINUEMOVIE:
		if (auto pMovie = data.AsStatic().MovieHandle())
		{
			if (pMovie->VTable && pMovie->VTable->Pause)
				pMovie->VTable->Pause(pMovie, 0);
		}
		return 0;

	case WW_STATIC_DETACHMOVIE:
		DetachStaticMovie(hWnd, data);
		return 0;

	case WW_STATIC_SETLOOPMOVIE:
		data.AsStatic().LoopMovie() = static_cast<int>(wParam);
		return 0;

	case WW_STATIC_REVEALTEXTS:
		if (data.AsStatic().DrawMode() != WWUIStaticDrawMode::TypewriterText || data.AsStatic().AnimationRunning())
			return 0;

		data.AsStatic().AnimationRunning() = true;
		data.AsStatic().TextRevealCount() = 1;
		::SetTimer(hWnd, 0, data.AsStatic().TextRevealDelay(), nullptr);
		::InvalidateRect(hWnd, nullptr, FALSE);
		return 0;

	case WW_STATIC_BLITMOVIE:
		if (auto pMovie = data.AsStatic().MovieHandle())
		{
			if (pMovie->VTable && pMovie->VTable->Blit)
				pMovie->VTable->Blit(pMovie);
		}
		return 0;

	default:
		return forwardOriginal();
	}
}
