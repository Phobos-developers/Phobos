#include "Surface.h"

#include <Utilities/Debug.h>

#include <Unsorted.h>
#include <Drawing.h>

class DXSurfaceImpl
{
public:
	DXSurfaceImpl(int width, int height);
	~DXSurfaceImpl();

	int Pitch;
	std::unique_ptr<BYTE[]> Buffer;
};

void DXSurface::CTOR(int width, int height)
{
	this->Width = width;
	this->Height = height;
	this->LockLevel = 0;
	this->BytesPerPixel = 2;
	ImplRef() = new DXSurfaceImpl(width, height);
}

void DXSurface::DTOR()
{
	if (ImplRef())
	{
		delete ImplRef();
		ImplRef() = nullptr;
	}
}

DXSurface::DXSurface(int width, int height) : DSurface { noinit_t{} }
{
	CTOR(width, height);
}

DXSurface::~DXSurface()
{
	DTOR();
}

bool DXSurface::CopyFromWhole(Surface* pSrc, bool trans, bool same_copy_cpu)
{
	JMP_THIS(0x7BBAF0);
}

bool DXSurface::CopyFromPart(RectangleStruct* pClipRect, Surface* pSrc, RectangleStruct* pSrcRect, bool trans, bool same_copy_cpu)
{
	JMP_THIS(0x7BBB90);
}

bool DXSurface::CopyFrom(RectangleStruct* pClipRect, RectangleStruct* pClipRect2, Surface* pSrc, RectangleStruct* pDestRect, RectangleStruct* pSrcRect, bool trans, bool same_copy_cpu)
{
	JMP_THIS(0x7BBCF0);
}

bool DXSurface::FillRectEx(RectangleStruct* pClipRect, RectangleStruct* pFillRect, COLORREF nColor)
{
	JMP_THIS(0x7BB050);
}

bool DXSurface::FillRect(RectangleStruct* pFillRect, COLORREF nColor)
{
	JMP_THIS(0x7BB020);
}

bool DXSurface::Fill(COLORREF nColor)
{
	JMP_THIS(0x7BBAB0);
}

bool DXSurface::FillRectTrans(RectangleStruct* pClipRect, ColorStruct* pColor, int nOpacity)
{
	JMP_THIS(0x4BB830);
}

bool DXSurface::DrawEllipse(int XOff, int YOff, int CenterX, int CenterY, RectangleStruct Rect, COLORREF nColor)
{
	JMP_THIS(0x7BB350);
}

bool DXSurface::SetPixel(Point2D* pPoint, COLORREF nColor)
{
	JMP_THIS(0x7BAEB0);
}

COLORREF DXSurface::GetPixel(Point2D* pPoint)
{
	JMP_THIS(0x7BAE60);
}

bool DXSurface::DrawLineEx(RectangleStruct* pClipRect, Point2D* pStart, Point2D* pEnd, COLORREF nColor)
{
	JMP_THIS(0x7BA610);
}

bool DXSurface::DrawLine(Point2D* pStart, Point2D* pEnd, COLORREF nColor)
{
	JMP_THIS(0x7BA5E0);
}

bool DXSurface::DrawLineColor(RectangleStruct* pRect, Point2D* pStart, Point2D* pEnd, COLORREF nColor, int startZ, int endZ, bool bUnk)
{
	JMP_THIS(0x4BFD30);
}

bool DXSurface::DrawMultiplyingLine(RectangleStruct* pRect, Point2D* pStart, Point2D* pEnd, DWORD dwMultiplier, DWORD dwUnk1, DWORD dwUnk2, bool bUnk)
{
	JMP_THIS(0x4BBCA0);
}

bool DXSurface::DrawSubtractiveLine(RectangleStruct* pRect, Point2D* pStart, Point2D* pEnd, ColorStruct* pColor, DWORD dwUnk1, DWORD dwUnk2, bool bUnk1, bool bUnk2, bool bUkn3, bool bUkn4, float fUkn)
{
	JMP_THIS(0x4BC750);
}

bool DXSurface::DrawRGBMultiplyingLine(RectangleStruct* pRect, Point2D* pStart, Point2D* pEnd, ColorStruct* pColor, float Intensity, int zSource, int zTarget)
{
	JMP_THIS(0x4BDF00);
}

bool DXSurface::PlotLine(RectangleStruct* pRect, Point2D* pStart, Point2D* pEnd, bool(__fastcall* fpDrawCallback)(int*))
{
	JMP_THIS(0x7BAB90);
}

bool DXSurface::DrawDashedLine(Point2D* pStart, Point2D* pEnd, int nColor, bool* Pattern, int nOffset)
{
	JMP_THIS(0x7BA8C0);
}

bool DXSurface::DrawDashedLine_(Point2D* pStart, Point2D* pEnd, int nColor, bool* Pattern, int nOffset, bool bUkn)
{
	JMP_THIS(0x4C0750);
}

bool DXSurface::DrawLine_(Point2D* pStart, Point2D* pEnd, int nColor, bool bUnk)
{
	JMP_THIS(0x4C0E30);
}

bool DXSurface::DrawRectEx(RectangleStruct* pClipRect, RectangleStruct* pDrawRect, int nColor)
{
	JMP_THIS(0x7BADC0);
}

bool DXSurface::DrawRect(RectangleStruct* pDrawRect, DWORD dwColor)
{
	JMP_THIS(0x7BAD90);
}

void* DXSurface::Lock(int X, int Y)
{
	if (X >= 0 && Y >= 0)
	{
		++LockLevel;
		return Impl()->Buffer.get() + Y * Impl()->Pitch + X * GetBytesPerPixel();
	}
	return nullptr;
}

bool DXSurface::Unlock()
{
	if (LockLevel > 0)
	{
		--LockLevel;
		return true;
	}

	return false;
}

bool DXSurface::CanLock(DWORD dwUkn1, DWORD dwUkn2)
{
	return true;
}

bool DXSurface::vt_entry_68(DWORD dwUnk1, DWORD dwUnk2)
{
	return true;
}

bool DXSurface::IsLocked()
{
	return false;
}

int DXSurface::GetBytesPerPixel()
{
	return 2;
}

int DXSurface::GetPitch()
{
	return Impl()->Pitch;
}

RectangleStruct* DXSurface::GetRect(RectangleStruct* pRect)
{
	*pRect = { 0, 0, GetWidth(), GetHeight() };
	return pRect;
}

int DXSurface::GetWidth()
{
	return Width;
}

int DXSurface::GetHeight()
{
	return Height;
}

bool DXSurface::IsDSurface()
{
	return true;
}

bool DXSurface::PutPixelClip(Point2D* pPoint, short nUkn, RectangleStruct* pRect)
{
	JMP_THIS(0x7BAF90);
}

short DXSurface::GetPixelClip(Point2D* pPoint, RectangleStruct* pRect)
{
	JMP_THIS(0x7BAF10);
}

bool DXSurface::DrawGradientLine(RectangleStruct* pRect, Point2D* pStart, Point2D* pEnd, ColorStruct* pStartColor, ColorStruct* pEndColor, float fStep, int nColor)
{
	JMP_THIS(0x4BF750);
}

bool DXSurface::CanBlit()
{
	return true;
}

void* DXSurface::GetBuffer()
{
	return Impl()->Buffer.get();
}

DXSurfaceImpl::DXSurfaceImpl(int width, int height)
{
	const int sourceRowBytes = width * 2; // 2 bytes per pixel for 16-bit color
	Pitch = (sourceRowBytes + 256 - 1) & ~(256 - 1); // Align up to D3D12_TEXTURE_DATA_PITCH_ALIGNMENT
	Buffer.reset(new BYTE[Pitch * height]);
}

DXSurfaceImpl::~DXSurfaceImpl() {}

static __forceinline unsigned int Build_Hicolor_Pixel(unsigned int r, unsigned int g, unsigned int b)
{
	return (r >> Drawing::RedShiftRight << Drawing::RedShiftLeft) |
		(g >> Drawing::GreenShiftRight << Drawing::GreenShiftLeft) |
		(b >> Drawing::BlueShiftRight << Drawing::BlueShiftLeft);
}

DXSurface* __fastcall DXSurface::CreatePrimary()
{
	Drawing::AllowSoftwareBlitFills = false;
	Drawing::AllowSoftwareBlitStretch = false;

	Debug::Log("[RenderDX] D3D12 surface created as primary surface.\n");

	auto surface = new DXSurface(Drawing::RenderWidth, Drawing::RenderHeight);

	// RGB565 color shifts
	Drawing::RedShiftLeft = 11;
	Drawing::RedShiftRight = 3;
	Drawing::GreenShiftLeft = 5;
	Drawing::GreenShiftRight = 2;
	Drawing::BlueShiftLeft = 0;
	Drawing::BlueShiftRight = 3;
	Drawing::ColorMode = RGBMode::RGB565;
	Drawing::HalfbrightMask = static_cast<unsigned short>(Build_Hicolor_Pixel(127, 127, 127));
	Drawing::QuarterbrightMask = static_cast<unsigned short>(Build_Hicolor_Pixel(63, 63, 63));
	Drawing::EighthbrightMask = static_cast<unsigned short>(Build_Hicolor_Pixel(31, 31, 31));

	return surface;
}
