#include "Mouse.h"

#include <Unsorted.h>
#include <FileFormats/SHP.h>
#include <FileSystem.h>

#include "Functions.h"

#include <cmath>

#include <Utilities/Debug.h>

DXMouse::DXMouse(Surface* surface, HWND hwnd) {}

DXMouse::~DXMouse() {
	Delete_Cursor_Image();
	if (Cursor) {
		::DestroyCursor(Cursor);
		Cursor = nullptr;
	}
}

void DXMouse::Set_Cursor(Point2D const& hotspot, SHPStruct const* cursor, int shape) {
	if (cursor == nullptr || shape < 0 || shape >= cursor->Frames) {
		Delete_Cursor_Image();
		Set_System_Cursor();
		return;
	}

	if (MouseShape == cursor && ShapeNumber == shape) {
		return; // No change needed
	}

	if (cursor != MouseShape) {
		Delete_Cursor_Image();
		Convert_Custor_Image(cursor);
	}

	MouseShape = cursor;
	ShapeNumber = shape;

	const auto& info = CursorInfo[shape];

	Hotspot = hotspot;
	Point2D scaled_hotspot;
	scaled_hotspot.X = std::clamp(Hotspot.X * Get_Cursor_Scale(), 0, info.Width - 1);
	scaled_hotspot.Y = std::clamp(Hotspot.Y * Get_Cursor_Scale(), 0, info.Height - 1);

	Replace_Cursor(Build_Cursor(info, scaled_hotspot.X, scaled_hotspot.Y));
}

bool DXMouse::Is_Hidden() const {
	return !IsVisible;
}

void DXMouse::Hide_Mouse() {
	Debug::Log("Hiding mouse cursor\n");

	if (!IsVisible)
		return;

	::SetCursor(nullptr);
	IsVisible = false;
}

void DXMouse::Show_Mouse() {
	Debug::Log("Showing mouse cursor\n");

	if (IsVisible)
		return;

	::SetCursor(Cursor);
	IsVisible = true;
}

void DXMouse::Release_Mouse() {
	if (!IsCaptured)
		return;

	::ClipCursor(nullptr);
	IsCaptured = false;
}

void DXMouse::Capture_Mouse() {
	if (IsCaptured)
		return;

	RECT client_rect;
	::GetClientRect(Game::hWnd, &client_rect);
	::MapWindowPoints(Game::hWnd, nullptr, reinterpret_cast<LPPOINT>(&client_rect), 2);
	::ClipCursor(&client_rect);

	IsCaptured = true;
}

bool DXMouse::Is_Captured() const {
	return IsCaptured;
}

void DXMouse::Conditional_Hide_Mouse(RectangleStruct region) {
	Hide_Mouse();
}

void DXMouse::Conditional_Show_Mouse() {
	Show_Mouse();
}

int DXMouse::Get_Mouse_State() const {
	return IsVisible ? 0 : -1;
}

int DXMouse::Get_Mouse_X() const {
	return MouseX;
}

int DXMouse::Get_Mouse_Y() const {
	return MouseY;
}

Point2D DXMouse::Get_Mouse_Point() const {
	return Point2D { MouseX, MouseY };
}

void DXMouse::Set_Mouse_Point(int x, int y) {
	MouseX = x;
	MouseY = y;
}

// Hardware cursor drawing is handled by the OS, so these functions are no-ops.
void DXMouse::Draw_Mouse(Surface* scr, bool issidebarsurface) {}

void DXMouse::Erase_Mouse(Surface* scr, bool issidebarsurface) {}

// Coordinate conversion is not needed when using hardware cursor, so this is a no-op.
void DXMouse::Convert_Coordinate(int& x, int& y) const {}

void DXMouse::Process_Mouse() {
	// Update mouse position via GetCursorPos and ScreenToClient
	if (!Unsorted::GameInFocus)
		return;

	POINT pt;
	if (!::GetCursorPos(&pt)) {
		return;
	}

	if (!::ScreenToClient(Game::hWnd, &pt)) {
		return;
	}

	if (RenderDX::ShouldScale()) {
		MouseX = RenderDX::ClientToRenderX(pt.x);
		MouseY = RenderDX::ClientToRenderY(pt.y);
	}
	else {
		MouseX = pt.x;
		MouseY = pt.y;
	}

}

void DXMouse::Recalc_Capture_Region() {
	if (Is_Captured()) {
		Release_Mouse();
		Capture_Mouse();
	}
}

void DXMouse::Set_Cached_Cursor() {
	if (IsVisible)
		::SetCursor(Cursor);
	else
		::SetCursor(nullptr);
}

void DXMouse::Rebuild_Cursor_Image() {
	SHPStruct const* shape = MouseShape;
	int number = ShapeNumber;

	Delete_Cursor_Image();
	Set_Cursor(Hotspot, shape, number);
}

void DXMouse::Delete_Cursor_Image() {
	CursorInfo.clear();

	MouseShape = nullptr;
	ShapeNumber = 0;
}

void DXMouse::Convert_Custor_Image(SHPStruct const* cursor) {
	if (!cursor || cursor->Frames <= 0)
		return;

	for (int i = 0; i < 256; ++i) {
		const auto color = static_cast<uint16_t*>(FileSystem::MOUSE_PAL->PaletteData)[i];
		auto clr = ColorStruct { color };
		MousePalette[i] = ((i == 0 ? 0 : 255) << 24) | (clr.R << 16) | (clr.G << 8) | clr.B;
	}

	CursorInfo.resize(cursor->Frames);
	for (int i = 0; i < cursor->Frames; ++i)
		Shape_To_Cursor(cursor, i, CursorInfo[i]);
}

void DXMouse::Shape_To_Cursor(SHPStruct const* cursor, int frame, CursorData& result) {
	int width = cursor->Width;
	int height = cursor->Height;

	std::vector<uint32_t> original_colors;
	original_colors.resize(width * height);

	int scaled_width = static_cast<int>(width * Get_Cursor_Scale());
	int scaled_height = static_cast<int>(height * Get_Cursor_Scale());

	BITMAPV5HEADER bi {};
	bi.bV5Size = sizeof(BITMAPV5HEADER);
	bi.bV5Width = scaled_width;
	bi.bV5Height = -scaled_height; // Negative height for top-down bitmap
	bi.bV5Planes = 1;
	bi.bV5BitCount = 32;
	bi.bV5Compression = BI_BITFIELDS;
	bi.bV5RedMask = 0x00FF0000;
	bi.bV5GreenMask = 0x0000FF00;
	bi.bV5BlueMask = 0x000000FF;
	bi.bV5AlphaMask = 0xFF000000;

	HDC hdc = ::GetDC(nullptr);
	void* dst = nullptr;
	HBITMAP bitmap = ::CreateDIBSection(hdc, reinterpret_cast<const BITMAPINFO*>(&bi), DIB_RGB_COLORS, &dst, nullptr, 0);
	::ReleaseDC(nullptr, hdc);

	if (!dst || !bitmap) {
		return;
	}

	const auto* src = static_cast<const uint8_t*>(cursor->GetPixels(frame));
	const auto r = cursor->GetFrameBounds(frame);

	if (cursor->HasCompression(frame)) {
		const uint8_t* psrc = src;
		for (int y = 0; y < r.Height; ++y) {
			uint32_t* dst_row = original_colors.data() + (r.Y + y) * width + r.X;
			int len = psrc[0] | (psrc[1] << 8);
			int pos = 0;
			for (int k = 2; k < len; ++k) {
				uint8_t b = psrc[k];
				if (b == 0) {
					uint8_t count = psrc[++k];
					for (int i = 0; i < count; ++i) {
						dst_row[pos++] = MousePalette[0];
					}
				}
				else
					dst_row[pos++] = MousePalette[b];
			}
			psrc += len;
		}
	}
	else {
		for (int y = 0; y < r.Height; ++y) {
			uint32_t* dst_row = original_colors.data() + (r.Y + y) * width + r.X;
			const uint8_t* src_row = src + y * r.Width;
			for (int x = 0; x < r.Width; ++x) {
				const auto color = MousePalette[src_row[x]];
				dst_row[x] = color;
			}
		}
	}

	Scale_Bitmap_Image(original_colors.data(), width, height, static_cast<uint32_t*>(dst), scaled_width, scaled_height);

	HBITMAP mask = ::CreateBitmap(scaled_width, scaled_height, 1, 1, nullptr);
	
	result.Width = scaled_width;
	result.Height = scaled_height;
	result.Color = bitmap;
	result.Mask = mask;
}

void DXMouse::Scale_Bitmap_Image(const uint32_t* src_ptr, int src_w, int src_h, uint32_t* dst, int dst_w, int dst_h) {
	// Using nearest neighbor scaling for simplicity
	const uint64_t inc_y = (static_cast<uint64_t>(src_h) << 16) / dst_h;
	const uint64_t inc_x = (static_cast<uint64_t>(src_w) << 16) / dst_w;

	uint64_t pos_y = inc_y / 2;

	for (int y = 0; y < dst_h; ++y) {
		const uint64_t src_y = pos_y >> 16;
		const uint32_t* src_row = src_ptr + src_y * src_w;

		pos_y += inc_y;

		uint64_t pos_x = inc_x / 2;

		for (int x = 0; x < dst_w; ++x) {
			const uint64_t src_x = pos_x >> 16;
			pos_x += inc_x;
			*dst++ = src_row[src_x];
		}
	}
}


void DXMouse::Replace_Cursor(HCURSOR cursor) {
	auto old_cursor = std::exchange(Cursor, cursor);
	::SetCursor(Cursor);
	if (old_cursor) {
		::DestroyCursor(old_cursor);
	}
}

void DXMouse::Set_System_Cursor() { Replace_Cursor(::LoadCursorA(nullptr, IDC_ARROW)); }

HCURSOR DXMouse::Build_Cursor(const CursorData& data, int hotspot_x, int hotspot_y) {
	ICONINFO ii {};
	ii.fIcon = FALSE;
	ii.xHotspot = static_cast<DWORD>(hotspot_x);
	ii.yHotspot = static_cast<DWORD>(hotspot_y);
	ii.hbmColor = data.Color;
	ii.hbmMask = data.Mask;

	return static_cast<HCURSOR>(::CreateIconIndirect(&ii));
}

int DXMouse::Get_Cursor_Scale() {
	if (!RenderDX::ShouldScale()) {
		return 1;
	}

	return std::max(1, static_cast<int>(std::round(1.0f / RenderDX::GetYScale())));
}
