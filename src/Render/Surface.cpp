#include "Surface.h"

#include <Utilities/Debug.h>

#include <Unsorted.h>
#include <Drawing.h>
#include <Blitters/BlitTrans.h>

#include <algorithm>

void DXSurface::CTOR(int width, int height) {
	Width = width;
	Height = height;
	LockLevel = 0;
	BytesPerPixel = 2; // 16-bit RGB565 color
	*InternalGetPitch() = (width * 2 + 256 - 1) & ~(256 - 1); // Keep rows aligned for texture uploads.
	*InternalGetBuffer() = YRMemory::Allocate(*InternalGetPitch() * height);
	std::memset(*InternalGetBuffer(), 0, *InternalGetPitch() * height);
}

void DXSurface::DTOR() {
	if (*InternalGetBuffer()) {
		YRMemory::Deallocate(*InternalGetBuffer());
		*InternalGetBuffer() = nullptr;
	}
}

DXSurface::DXSurface(int width, int height) : DSurface { noinit_t{} } {
	CTOR(width, height);
}

DXSurface::~DXSurface() {
	DTOR();
}

bool DXSurface::CopyFromWhole(Surface* pSrc, bool transparent, bool) {
	auto sourceRect = RectangleStruct { 0, 0, pSrc->Width, pSrc->Height };
	auto destRect = RectangleStruct { 0, 0, Width, Height };
	return DXSurface::CopyFromPart(&destRect, pSrc, &sourceRect, transparent, false);
}

bool DXSurface::CopyFromPart(RectangleStruct* pClipRect, Surface* pSrc, RectangleStruct* pSrcRect, bool transparent, bool) {
	auto sourceWindow = RectangleStruct { 0, 0, pSrc->Width, pSrc->Height };
	auto destWindow = RectangleStruct { 0, 0, Width, Height };
	return DXSurface::CopyFrom(&destWindow, pClipRect, pSrc, &sourceWindow, pSrcRect, transparent, false);
}

bool DXSurface::CopyFrom(RectangleStruct* dcliprect, RectangleStruct* destrect, Surface* source, RectangleStruct* scliprect, RectangleStruct* sourcerect, bool trans, bool) {
	if (!source->IsDSurface())
	{
		// If source is not a DXSurface, then use vanilla XSurface handler
		return reinterpret_cast<bool(__thiscall*)(XSurface*, RectangleStruct*, Surface*, RectangleStruct*, bool, bool)>(0x7BBB90)(this, destrect, source, sourcerect, trans, true);
	}

	if (trans)
	{
		// XSurface original routine
		RectangleStruct drect = *destrect;
		RectangleStruct srect = *sourcerect;
		if (!Drawing::BlitClip(drect, *dcliprect, srect, *scliprect))
			return false;

		BlitTrans<WORD> blitter;
		return Drawing::BitBlit(this, &drect, source, &srect, &blitter, 0, ZGradient::Deg135, 1000, 0);
	}

	// Handle the untransparent case ourselves, supporting scaling
	RectangleStruct srect = *sourcerect;
	RectangleStruct drect = *destrect;

	RectangleStruct swindow = Drawing::Intersect(*scliprect, source->GetRect());
	RectangleStruct dwindow = Drawing::Intersect(*dcliprect, Surface::GetRect());

	if (!Drawing::BlitClip(drect, dwindow, srect, swindow))
		return false;

	RectangleStruct src { srect.X + swindow.X, srect.Y + swindow.Y, srect.Width, srect.Height };
	RectangleStruct dst { drect.X + dwindow.X, drect.Y + dwindow.Y, drect.Width, drect.Height };
	if (src.Width <= 0 || src.Height <= 0 || dst.Width <= 0 || dst.Height <= 0)
		return false;

	auto src_ptr = reinterpret_cast<DXSurface*>(source)->RawLock(src.X, src.Y);
	auto dst_ptr = this->RawLock(dst.X, dst.Y);
	const auto src_pitch = reinterpret_cast<DXSurface*>(source)->GetPitch();
	const auto dst_pitch = GetPitch();

	// If the source and destination rectangles are the same size, we can do a simple copy.
	if (src.Width == dst.Width && src.Height == dst.Height)
	{
		for (int y = 0; y < dst.Height; ++y)
		{
			std::copy_n(reinterpret_cast<WORD*>(src_ptr), dst.Width, reinterpret_cast<WORD*>(dst_ptr));
			src_ptr = reinterpret_cast<BYTE*>(src_ptr) + src_pitch;
			dst_ptr = reinterpret_cast<BYTE*>(dst_ptr) + dst_pitch;
		}
		return true;
	}

	// Otherwise we need to scale the source to fit the destination.
	const auto incY = (static_cast<unsigned long long>(src.Height) << 16) / dst.Height;
	const auto incX = (static_cast<unsigned long long>(src.Width) << 16) / dst.Width;
	const auto dstGap = dst_pitch - 2 * dst.Width;
	auto posY = incY / 2;
	for (int y = 0; y < dst.Height; ++y)
	{
		const auto srcY = static_cast<int>(posY >> 16);
		const auto pSrcRow = src_ptr + srcY * src_pitch;
		posY += incY;
		auto posX = incX / 2;
		for (int x = 0; x < dst.Width; ++x)
		{
			const auto srcX = 2 * static_cast<int>(posX >> 16);
			posX += incX;
			*reinterpret_cast<WORD*>(dst_ptr) = *reinterpret_cast<const WORD*>(pSrcRow + srcX);
			dst_ptr += 2;
		}
		dst_ptr += dstGap;
	}

	return true;
}

bool DXSurface::FillRectEx(RectangleStruct* pClipRect, RectangleStruct* pFillRect, COLORREF nColor) {
	if (pFillRect->Width <= 0 || pFillRect->Height <= 0)
		return false;

	RectangleStruct rect { 0, 0, Width, Height };
	RectangleStruct windowRect = Drawing::Intersect(*pClipRect, rect);

	rect = *pFillRect;
	rect.X += pClipRect->X;
	rect.Y += pClipRect->Y;
	RectangleStruct clippedRect = Drawing::Intersect(windowRect, rect);
	if (clippedRect.Width <= 0 || clippedRect.Height <= 0)
		return false;

	auto pBuffer = RawLock(clippedRect.X, clippedRect.Y);
	if (!pBuffer)
		return false;

	const int pitch = GetPitch();
	for (int y = 0; y < clippedRect.Height; ++y) {
		std::fill(reinterpret_cast<WORD*>(pBuffer), reinterpret_cast<WORD*>(pBuffer) + clippedRect.Width, static_cast<WORD>(nColor));
		pBuffer = reinterpret_cast<BYTE*>(pBuffer) + pitch;
	}
	return true;
}

bool DXSurface::FillRect(RectangleStruct* pFillRect, COLORREF nColor) {
	auto window = RectangleStruct { 0, 0, Width, Height };
	return DXSurface::FillRectEx(&window, pFillRect, nColor);
}

bool DXSurface::Fill(COLORREF nColor) {
	auto window = RectangleStruct { 0, 0, Width, Height };
	auto clip = RectangleStruct { 0, 0, Width, Height };
	return DXSurface::FillRectEx(&clip, &window, nColor);
}

bool DXSurface::FillRectTrans(RectangleStruct* pClipRect, ColorStruct* pColor, int nOpacity) {
	JMP_THIS(0x4BB830);
}

bool DXSurface::DrawEllipse(int xOffset, int yOffset, int centerX, int centerY, RectangleStruct rect, COLORREF nColor) {
	JMP_THIS(0x7BB350);
}

bool DXSurface::SetPixel(Point2D* pPoint, COLORREF nColor) {
	auto pPixel = RawLock(pPoint->X, pPoint->Y);
	reinterpret_cast<WORD*>(pPixel)[0] = static_cast<WORD>(nColor);
	return true;
}

COLORREF DXSurface::GetPixel(Point2D* pPoint) {
	auto pPixel = RawLock(pPoint->X, pPoint->Y);
	return reinterpret_cast<WORD*>(pPixel)[0];
}

bool DXSurface::DrawLineEx(RectangleStruct* pClipRect, Point2D* pStart, Point2D* pEnd, COLORREF nColor) {
	JMP_THIS(0x7BA610);
}

bool DXSurface::DrawLine(Point2D* pStart, Point2D* pEnd, COLORREF nColor) {
	auto window = RectangleStruct { 0, 0, Width, Height };
	return DXSurface::DrawLineEx(&window, pStart, pEnd, nColor);
}

bool DXSurface::DrawLineColor(RectangleStruct* pRect, Point2D* pStart, Point2D* pEnd, COLORREF nColor, int startZ, int endZ, bool bUnk) {
	JMP_THIS(0x4BFD30);
}

bool DXSurface::DrawMultiplyingLine(RectangleStruct* pRect, Point2D* pStart, Point2D* pEnd, DWORD dwMultiplier, DWORD dwUnk1, DWORD dwUnk2, bool bUnk) {
	JMP_THIS(0x4BBCA0);
}

bool DXSurface::DrawSubtractiveLine(RectangleStruct* pRect, Point2D* pStart, Point2D* pEnd, ColorStruct* pColor, DWORD dwUnk1, DWORD dwUnk2, bool bUnk1, bool bUnk2, bool bUkn3, bool bUkn4, float fUkn) {
	JMP_THIS(0x4BC750);
}

bool DXSurface::DrawRGBMultiplyingLine(RectangleStruct* pRect, Point2D* pStart, Point2D* pEnd, ColorStruct* pColor, float Intensity, int zSource, int zTarget) {
	JMP_THIS(0x4BDF00);
}

bool DXSurface::PlotLine(RectangleStruct* pRect, Point2D* pStart, Point2D* pEnd, bool(__fastcall* AddRedrawPoint)(int*)) {
	JMP_THIS(0x7BAB90);
}

bool DXSurface::DrawDashedLine(Point2D* pStart, Point2D* pEnd, int nColor, bool* Pattern, int nOffset) {
	JMP_THIS(0x7BA8C0);
}

bool DXSurface::DrawDashedLine_(Point2D* pStart, Point2D* pEnd, int nColor, bool* Pattern, int nOffset, bool bUkn) {
	JMP_THIS(0x4C0750);
}

bool DXSurface::DrawLine_(Point2D* pStart, Point2D* pEnd, int nColor, bool bUnk) {
	JMP_THIS(0x4C0E30);
}

bool DXSurface::DrawRectEx(RectangleStruct* pClipRect, RectangleStruct* pDrawRect, int nColor) {
	JMP_THIS(0x7BADC0);
}

bool DXSurface::DrawRect(RectangleStruct* pDrawRect, DWORD dwColor) {
	auto window = RectangleStruct { 0, 0, Width, Height };
	return DXSurface::DrawRectEx(&window, pDrawRect, dwColor);
}

void* DXSurface::Lock(int x, int y) {
	if (x >= 0 && y >= 0) {
		++LockLevel;
		return RawLock(x, y);
	}
	return nullptr;
}

bool DXSurface::Unlock() {
	if (LockLevel > 0) {
		--LockLevel;
		return true;
	}

	return false;
}

bool DXSurface::CanLock(DWORD dwUkn1, DWORD dwUkn2) {
	return true;
}

bool DXSurface::vt_entry_68(DWORD dwUnk1, DWORD dwUnk2) {
	return true;
}

bool DXSurface::IsLocked() {
	return false;
}

int DXSurface::GetBytesPerPixel() {
	return 2;
}

int DXSurface::GetPitch() {
	return *InternalGetPitch();
}

RectangleStruct* DXSurface::GetRect(RectangleStruct* pRect) {
	*pRect = { 0, 0, GetWidth(), GetHeight() };
	return pRect;
}

int DXSurface::GetWidth() {
	return Width;
}

int DXSurface::GetHeight() {
	return Height;
}

bool DXSurface::IsDSurface() {
	return true;
}

bool DXSurface::PutPixelClip(Point2D* pPoint, short color, RectangleStruct* pRect) {
	if (pPoint->X < pRect->X || pPoint->X >= pRect->X + pRect->Width || pPoint->Y < pRect->Y || pPoint->Y >= pRect->Y + pRect->Height) {
		return false;
	}

	auto pPixel = RawLock(pPoint->X, pPoint->Y);
	reinterpret_cast<WORD*>(pPixel)[0] = static_cast<WORD>(color);
	return true;
}

short DXSurface::GetPixelClip(Point2D* pPoint, RectangleStruct* pRect) {
	if (pPoint->X < pRect->X || pPoint->X >= pRect->X + pRect->Width || pPoint->Y < pRect->Y || pPoint->Y >= pRect->Y + pRect->Height) {
		return 0;
	}

	auto pPixel = RawLock(pPoint->X, pPoint->Y);
	return reinterpret_cast<WORD*>(pPixel)[0];
}

bool DXSurface::DrawGradientLine(RectangleStruct* pRect, Point2D* pStart, Point2D* pEnd, ColorStruct* pStartColor, ColorStruct* pEndColor, float fStep, int nColor) {
	JMP_THIS(0x4BF750);
}

bool DXSurface::CanBlit() {
	return true;
}

void* DXSurface::GetBuffer() {
	return *InternalGetBuffer();
}

BYTE* DXSurface::RawLock(int x, int y) {
	return reinterpret_cast<BYTE*>(*InternalGetBuffer()) + y * *InternalGetPitch() + x * GetBytesPerPixel();
}

static __forceinline unsigned int BuildHicolorPixel(unsigned int red, unsigned int green, unsigned int blue) {
	return (red >> Drawing::RedShiftRight << Drawing::RedShiftLeft) |
		(green >> Drawing::GreenShiftRight << Drawing::GreenShiftLeft) |
		(blue >> Drawing::BlueShiftRight << Drawing::BlueShiftLeft);
}

DXSurface* __fastcall DXSurface::CreatePrimary() {
	Drawing::AllowSoftwareBlitFills = false;
	Drawing::AllowSoftwareBlitStretch = false;

	Debug::Log("[RenderDX] D3D11 surface created as primary surface.\n");

	auto surface = new DXSurface(Drawing::RenderWidth, Drawing::RenderHeight);

	Drawing::RedShiftLeft = 11;
	Drawing::RedShiftRight = 3;
	Drawing::GreenShiftLeft = 5;
	Drawing::GreenShiftRight = 2;
	Drawing::BlueShiftLeft = 0;
	Drawing::BlueShiftRight = 3;
	Drawing::ColorMode = RGBMode::RGB565;
	Drawing::HalfbrightMask = static_cast<unsigned short>(BuildHicolorPixel(127, 127, 127));
	Drawing::QuarterbrightMask = static_cast<unsigned short>(BuildHicolorPixel(63, 63, 63));
	Drawing::EighthbrightMask = static_cast<unsigned short>(BuildHicolorPixel(31, 31, 31));

	return surface;
}
