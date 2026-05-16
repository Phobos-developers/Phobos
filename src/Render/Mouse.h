#pragma once

#include <GeneralStructures.h>

#include <vector>

#include <Helpers/CompileTime.h>

struct SHPStruct;
class Surface;

class Mouse {
public:
	virtual ~Mouse() {}
	virtual void Set_Cursor(Point2D const& hotspot, SHPStruct const* cursor, int shape) = 0;
	virtual bool Is_Hidden() const = 0;
	virtual void Hide_Mouse() = 0;
	virtual void Show_Mouse() = 0;
	virtual void Release_Mouse() = 0;
	virtual void Capture_Mouse() = 0;
	virtual bool Is_Captured() const = 0;
	virtual void Conditional_Hide_Mouse(RectangleStruct region) = 0;
	virtual void Conditional_Show_Mouse() = 0;
	virtual int Get_Mouse_State() const = 0;
	virtual int Get_Mouse_X() const = 0;
	virtual int Get_Mouse_Y() const = 0;
	virtual Point2D Get_Mouse_Point() const = 0;
	virtual void Set_Mouse_Point(int x, int y) = 0;
	virtual void Draw_Mouse(Surface* scr, bool issidebarsurface = false) = 0;
	virtual void Erase_Mouse(Surface* scr, bool issidebarsurface = false) = 0;
	virtual void Convert_Coordinate(int& x, int& y) const = 0;
};

class DXMouse : public Mouse {
public:
	DEFINE_REFERENCE(DXMouse*, Instance, 0x887640u)

	DXMouse(Surface* surface, HWND hwnd);

	virtual ~DXMouse() override;
	virtual void Set_Cursor(Point2D const& hotspot, SHPStruct const* cursor, int shape) override;
	virtual bool Is_Hidden() const override;
	virtual void Hide_Mouse() override;
	virtual void Show_Mouse() override;
	virtual void Release_Mouse() override;
	virtual void Capture_Mouse() override;
	virtual bool Is_Captured() const override;
	virtual void Conditional_Hide_Mouse(RectangleStruct region) override;
	virtual void Conditional_Show_Mouse() override;
	virtual int Get_Mouse_State() const override;
	virtual int Get_Mouse_X() const override;
	virtual int Get_Mouse_Y() const override;
	virtual Point2D Get_Mouse_Point() const override;
	virtual void Set_Mouse_Point(int x, int y) override;
	virtual void Draw_Mouse(Surface* scr, bool issidebarsurface = false) override;
	virtual void Erase_Mouse(Surface* scr, bool issidebarsurface = false) override;
	virtual void Convert_Coordinate(int& x, int& y) const override;

	void Process_Mouse();
	void Recalc_Capture_Region();
	void Set_Cached_Cursor();

	void Rebuild_Cursor_Image();
private:
	SHPStruct const* MouseShape { nullptr };
	int ShapeNumber { 0 };

	DWORD MousePalette[256] { 0 };

	struct CursorData {
		~CursorData() {
			if (Color) {
				::DeleteObject(Color);
			}
			if (Mask) {
				::DeleteObject(Mask);
			}
		}

		int Width { 0 };
		int Height { 0 };
		HBITMAP Color { nullptr };
		HBITMAP Mask { nullptr };
	};
	std::vector<CursorData> CursorInfo;

	Point2D Hotspot { 0,0 };
	HCURSOR Cursor { nullptr };

	bool IsCaptured { false };
	bool IsVisible { true };

	int MouseX { 0 };
	int MouseY { 0 };

	void Delete_Cursor_Image();
	void Convert_Custor_Image(SHPStruct const* cursor);
	void Shape_To_Cursor(SHPStruct const* cursor, int frame, CursorData& result);
	void Scale_Bitmap_Image(const uint32_t* src_ptr, int src_w, int src_h, uint32_t* dst, int dst_w, int dst_h);
	void Replace_Cursor(HCURSOR cursor);
	void Set_System_Cursor();
	HCURSOR Build_Cursor(const CursorData& data, int hotspot_x, int hotspot_y);

	static int Get_Cursor_Scale();

};
