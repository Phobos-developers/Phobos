#pragma once

#include <GeneralStructures.h>

#include <vector>

#include <Helpers/CompileTime.h>

struct SHPStruct;
class Surface;

class Mouse {
public:
	virtual ~Mouse() {}
	virtual void SetCursor(Point2D const& hotspot, SHPStruct const* pCursor, int shape) = 0;
	virtual bool IsHidden() const = 0;
	virtual void HideMouse() = 0;
	virtual void ShowMouse() = 0;
	virtual void ReleaseMouse() = 0;
	virtual void CaptureMouse() = 0;
	virtual bool IsCaptured() const = 0;
	virtual void ConditionalHideMouse(RectangleStruct region) = 0;
	virtual void ConditionalShowMouse() = 0;
	virtual int GetMouseState() const = 0;
	virtual int GetMouseX() const = 0;
	virtual int GetMouseY() const = 0;
	virtual Point2D GetMousePoint() const = 0;
	virtual void SetMousePoint(int x, int y) = 0;
	virtual void DrawMouse(Surface* pSurface, bool isSidebarSurface = false) = 0;
	virtual void EraseMouse(Surface* pSurface, bool isSidebarSurface = false) = 0;
	virtual void ConvertCoordinate(int& x, int& y) const = 0;
};

class DXMouse : public Mouse {
public:
	DEFINE_REFERENCE(DXMouse*, Instance, 0x887640u)

	DXMouse(Surface* pSurface, HWND hWnd);

	virtual ~DXMouse() override;
	virtual void SetCursor(Point2D const& hotspot, SHPStruct const* pCursor, int shape) override;
	virtual bool IsHidden() const override;
	virtual void HideMouse() override;
	virtual void ShowMouse() override;
	virtual void ReleaseMouse() override;
	virtual void CaptureMouse() override;
	virtual bool IsCaptured() const override;
	virtual void ConditionalHideMouse(RectangleStruct region) override;
	virtual void ConditionalShowMouse() override;
	virtual int GetMouseState() const override;
	virtual int GetMouseX() const override;
	virtual int GetMouseY() const override;
	virtual Point2D GetMousePoint() const override;
	virtual void SetMousePoint(int x, int y) override;
	virtual void DrawMouse(Surface* pSurface, bool isSidebarSurface = false) override;
	virtual void EraseMouse(Surface* pSurface, bool isSidebarSurface = false) override;
	virtual void ConvertCoordinate(int& x, int& y) const override;

	void ProcessMouse();
	void RecalcCaptureRegion();
	void SetCachedCursor();

	void RebuildCursorImage();
private:
	SHPStruct const* MouseShape { nullptr }; // Current SHP cursor data.
	int ShapeNumber { 0 }; // Current cursor frame index.

	DWORD MousePalette[256] { 0 }; // ARGB palette converted from mouse.pal.

	struct CursorData {
		~CursorData() {
			if (Color) {
				::DeleteObject(Color);
			}
			if (Mask) {
				::DeleteObject(Mask);
			}
		}

		int Width { 0 }; // Cursor bitmap width.
		int Height { 0 }; // Cursor bitmap height.
		HBITMAP Color { nullptr }; // Color bitmap handle.
		HBITMAP Mask { nullptr }; // Mask bitmap handle.
	};
	std::vector<CursorData> CursorInfo; // Cached cursor frames.

	Point2D Hotspot { 0, 0 }; // Cursor hotspot in render coordinates.
	HCURSOR Cursor { nullptr }; // Current Win32 cursor handle.

	bool Captured { false }; // Whether the cursor is clipped to the game window.
	bool Visible { true }; // Whether the cursor should be displayed.

	int MouseX { 0 }; // Current render-space cursor X.
	int MouseY { 0 }; // Current render-space cursor Y.

	void DeleteCursorImage();
	void ConvertCursorImage(SHPStruct const* pCursor);
	void ShapeToCursor(SHPStruct const* pCursor, int frame, CursorData& result);
	void ScaleBitmapImage(const uint32_t* pSource, int sourceWidth, int sourceHeight, uint32_t* pDest, int destWidth, int destHeight);
	void ReplaceCursor(HCURSOR cursor);
	void SetSystemCursor();
	HCURSOR BuildCursor(const CursorData& data, int hotspotX, int hotspotY);

	static int GetCursorScale();

};
