#include "Mouse.h"

#include <Unsorted.h>
#include <FileFormats/SHP.h>
#include <FileSystem.h>

#include "Functions.h"

#include <algorithm>
#include <cmath>
#include <utility>

#include <Utilities/Debug.h>

DXMouse::DXMouse(Surface* pSurface, HWND hWnd) {}

DXMouse::~DXMouse() {
	DeleteCursorImage();
	if (Cursor) {
		::DestroyCursor(Cursor);
		Cursor = nullptr;
	}
}

void DXMouse::SetCursor(Point2D const& hotspot, SHPStruct const* pCursor, int shape) {
	if (pCursor == nullptr || shape < 0 || shape >= pCursor->Frames) {
		DeleteCursorImage();
		SetSystemCursor();
		return;
	}

	if (MouseShape == pCursor && ShapeNumber == shape)
		return;

	if (pCursor != MouseShape) {
		DeleteCursorImage();
		ConvertCursorImage(pCursor);
	}

	MouseShape = pCursor;
	ShapeNumber = shape;

	const auto& info = CursorInfo[shape];

	Hotspot = hotspot;
	Point2D scaledHotspot;
	scaledHotspot.X = std::clamp(Hotspot.X * GetCursorScale(), 0, info.Width - 1);
	scaledHotspot.Y = std::clamp(Hotspot.Y * GetCursorScale(), 0, info.Height - 1);

	ReplaceCursor(BuildCursor(info, scaledHotspot.X, scaledHotspot.Y));
}

bool DXMouse::IsHidden() const {
	return !Visible;
}

void DXMouse::HideMouse() {
	Debug::Log("Hiding mouse cursor\n");

	if (!Visible)
		return;

	::SetCursor(nullptr);
	Visible = false;
}

void DXMouse::ShowMouse() {
	Debug::Log("Showing mouse cursor\n");

	if (Visible)
		return;

	::SetCursor(Cursor);
	Visible = true;
}

void DXMouse::ReleaseMouse() {
	if (!Captured)
		return;

	::ClipCursor(nullptr);
	Captured = false;
}

void DXMouse::CaptureMouse() {
	if (Captured)
		return;

	RECT clientRect;
	::GetClientRect(Game::hWnd, &clientRect);
	::MapWindowPoints(Game::hWnd, nullptr, reinterpret_cast<LPPOINT>(&clientRect), 2);
	::ClipCursor(&clientRect);

	Captured = true;
}

bool DXMouse::IsCaptured() const {
	return Captured;
}

void DXMouse::ConditionalHideMouse(RectangleStruct region) {
	HideMouse();
}

void DXMouse::ConditionalShowMouse() {
	ShowMouse();
}

int DXMouse::GetMouseState() const {
	return Visible ? 0 : -1;
}

int DXMouse::GetMouseX() const {
	return MouseX;
}

int DXMouse::GetMouseY() const {
	return MouseY;
}

Point2D DXMouse::GetMousePoint() const {
	return Point2D { MouseX, MouseY };
}

void DXMouse::SetMousePoint(int x, int y) {
	MouseX = x;
	MouseY = y;
}

// Hardware cursor drawing is handled by the OS, so these functions are no-ops.
void DXMouse::DrawMouse(Surface* pSurface, bool isSidebarSurface) {}

void DXMouse::EraseMouse(Surface* pSurface, bool isSidebarSurface) {}

// Coordinate conversion is not needed when using hardware cursor, so this is a no-op.
void DXMouse::ConvertCoordinate(int& x, int& y) const {}

void DXMouse::ProcessMouse() {
	if (!Unsorted::GameInFocus)
		return;

	POINT pt;
	if (!::GetCursorPos(&pt))
		return;

	if (!RenderDX::ScreenToRenderPoint(&pt, true))
		return;

	MouseX = pt.x;
	MouseY = pt.y;
}

void DXMouse::RecalcCaptureRegion() {
	if (IsCaptured()) {
		ReleaseMouse();
		CaptureMouse();
	}
}

void DXMouse::SetCachedCursor() {
	if (Visible)
		::SetCursor(Cursor);
	else
		::SetCursor(nullptr);
}

void DXMouse::RebuildCursorImage() {
	SHPStruct const* shape = MouseShape;
	int number = ShapeNumber;

	DeleteCursorImage();
	SetCursor(Hotspot, shape, number);
}

void DXMouse::DeleteCursorImage() {
	CursorInfo.clear();

	MouseShape = nullptr;
	ShapeNumber = 0;
}

void DXMouse::ConvertCursorImage(SHPStruct const* pCursor) {
	if (!pCursor || pCursor->Frames <= 0)
		return;

	for (int i = 0; i < 256; ++i) {
		const auto color = static_cast<uint16_t*>(FileSystem::MOUSE_PAL->PaletteData)[i];
		auto clr = ColorStruct { static_cast<WORD>(color) };
		MousePalette[i] = ((i == 0 ? 0 : 255) << 24) | (clr.R << 16) | (clr.G << 8) | clr.B;
	}

	CursorInfo.resize(pCursor->Frames);
	for (int i = 0; i < pCursor->Frames; ++i)
		ShapeToCursor(pCursor, i, CursorInfo[i]);
}

void DXMouse::ShapeToCursor(SHPStruct const* pCursor, int frame, CursorData& result) {
	int width = pCursor->Width;
	int height = pCursor->Height;

	std::vector<uint32_t> originalColors;
	originalColors.resize(width * height);

	int scaledWidth = static_cast<int>(width * GetCursorScale());
	int scaledHeight = static_cast<int>(height * GetCursorScale());

	BITMAPV5HEADER bi {};
	bi.bV5Size = sizeof(BITMAPV5HEADER);
	bi.bV5Width = scaledWidth;
	bi.bV5Height = -scaledHeight; // Negative height creates a top-down bitmap.
	bi.bV5Planes = 1;
	bi.bV5BitCount = 32;
	bi.bV5Compression = BI_BITFIELDS;
	bi.bV5RedMask = 0x00FF0000;
	bi.bV5GreenMask = 0x0000FF00;
	bi.bV5BlueMask = 0x000000FF;
	bi.bV5AlphaMask = 0xFF000000;

	HDC hDC = ::GetDC(nullptr);
	void* pDestPixels = nullptr;
	HBITMAP bitmap = ::CreateDIBSection(hDC, reinterpret_cast<const BITMAPINFO*>(&bi), DIB_RGB_COLORS, &pDestPixels, nullptr, 0);
	::ReleaseDC(nullptr, hDC);

	if (!pDestPixels || !bitmap)
		return;

	const auto* pSource = static_cast<const uint8_t*>(pCursor->GetPixels(frame));
	const auto rect = pCursor->GetFrameBounds(frame);

	if (pCursor->HasCompression(frame)) {
		const uint8_t* pSrc = pSource;
		for (int y = 0; y < rect.Height; ++y) {
			uint32_t* pDestRow = originalColors.data() + (rect.Y + y) * width + rect.X;
			int length = pSrc[0] | (pSrc[1] << 8);
			int pos = 0;
			for (int k = 2; k < length; ++k) {
				uint8_t value = pSrc[k];
				if (value == 0) {
					uint8_t count = pSrc[++k];
					for (int i = 0; i < count; ++i)
						pDestRow[pos++] = MousePalette[0];
				}
				else
					pDestRow[pos++] = MousePalette[value];
			}
			pSrc += length;
		}
	}
	else {
		for (int y = 0; y < rect.Height; ++y) {
			uint32_t* pDestRow = originalColors.data() + (rect.Y + y) * width + rect.X;
			const uint8_t* pSourceRow = pSource + y * rect.Width;
			for (int x = 0; x < rect.Width; ++x) {
				const auto color = MousePalette[pSourceRow[x]];
				pDestRow[x] = color;
			}
		}
	}

	ScaleBitmapImage(originalColors.data(), width, height, static_cast<uint32_t*>(pDestPixels), scaledWidth, scaledHeight);

	HBITMAP mask = ::CreateBitmap(scaledWidth, scaledHeight, 1, 1, nullptr);

	result.Width = scaledWidth;
	result.Height = scaledHeight;
	result.Color = bitmap;
	result.Mask = mask;
}

void DXMouse::ScaleBitmapImage(const uint32_t* pSource, int sourceWidth, int sourceHeight, uint32_t* pDest, int destWidth, int destHeight) {
	const uint64_t yStep = (static_cast<uint64_t>(sourceHeight) << 16) / destHeight;
	const uint64_t xStep = (static_cast<uint64_t>(sourceWidth) << 16) / destWidth;

	uint64_t yPosition = yStep / 2;

	for (int y = 0; y < destHeight; ++y) {
		const uint64_t sourceY = yPosition >> 16;
		const uint32_t* pSourceRow = pSource + sourceY * sourceWidth;

		yPosition += yStep;

		uint64_t xPosition = xStep / 2;

		for (int x = 0; x < destWidth; ++x) {
			const uint64_t sourceX = xPosition >> 16;
			xPosition += xStep;
			*pDest++ = pSourceRow[sourceX];
		}
	}
}

void DXMouse::ReplaceCursor(HCURSOR cursor) {
	auto oldCursor = std::exchange(Cursor, cursor);
	::SetCursor(Cursor);
	if (oldCursor)
		::DestroyCursor(oldCursor);
}

void DXMouse::SetSystemCursor() {
	ReplaceCursor(::LoadCursorA(nullptr, IDC_ARROW));
}

HCURSOR DXMouse::BuildCursor(const CursorData& data, int hotspotX, int hotspotY) {
	ICONINFO ii {};
	ii.fIcon = FALSE;
	ii.xHotspot = static_cast<DWORD>(hotspotX);
	ii.yHotspot = static_cast<DWORD>(hotspotY);
	ii.hbmColor = data.Color;
	ii.hbmMask = data.Mask;

	return static_cast<HCURSOR>(::CreateIconIndirect(&ii));
}

int DXMouse::GetCursorScale() {
	return std::max(1, static_cast<int>(std::round(1.0f / RenderDX::GetYScale())));
}
