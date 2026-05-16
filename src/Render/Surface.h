#pragma once

#include <Surface.h>

// CPU render
class DXSurfaceImpl;
class DXSurface : public DSurface {
private:
	DXSurfaceImpl* Impl() const {
		return reinterpret_cast<DXSurfaceImpl*>(Buffer);
	}

	DXSurfaceImpl*& ImplRef() {
		return reinterpret_cast<DXSurfaceImpl*&>(Buffer);
	}

public:
	static DXSurface* __fastcall CreatePrimary();

	void CTOR(int width, int height);
	void DTOR();

	DXSurface(int width, int height);

	virtual ~DXSurface() override;

	//Surface
	virtual bool CopyFromWhole(Surface* pSrc, bool trans, bool same_copy_cpu) override;

	virtual bool CopyFromPart(
		RectangleStruct* pClipRect, //ignored and retrieved again...
		Surface* pSrc,
		RectangleStruct* pSrcRect,	//desired source rect of pSrc ?
		bool trans,
		bool same_copy_cpu) override;

	virtual bool CopyFrom(
		RectangleStruct* pClipRect,
		RectangleStruct* pClipRect2,	//again? hmm
		Surface* pSrc,
		RectangleStruct* pDestRect,	//desired dest rect of pSrc ? (stretched? clipped?)
		RectangleStruct* pSrcRect,	//desired source rect of pSrc ?
		bool trans,
		bool same_copy_cpu) override;

	virtual bool FillRectEx(RectangleStruct* pClipRect, RectangleStruct* pFillRect, COLORREF nColor) override;
	virtual bool FillRect(RectangleStruct* pFillRect, COLORREF nColor) override;
	virtual bool Fill(COLORREF nColor) override;
	virtual bool FillRectTrans(RectangleStruct* pClipRect, ColorStruct* pColor, int Opacity) override;
	virtual bool DrawEllipse(int XOff, int YOff, int CenterX, int CenterY, RectangleStruct Rect, COLORREF nColor) override;
	virtual bool SetPixel(Point2D* pPoint, COLORREF nColor) override;
	virtual COLORREF GetPixel(Point2D* pPoint) override;
	virtual bool DrawLineEx(RectangleStruct* pClipRect, Point2D* pStart, Point2D* pEnd, COLORREF nColor) override;
	virtual bool DrawLine(Point2D* pStart, Point2D* pEnd, COLORREF nColor) override;
	virtual bool DrawLineColor(
		RectangleStruct* pRect, Point2D* pStart, Point2D* pEnd, COLORREF nColor,
		int startZ, int endZ, bool bUnk) override;

	virtual bool DrawMultiplyingLine(
		RectangleStruct* pRect, Point2D* pStart, Point2D* pEnd, DWORD dwMultiplier,
		DWORD dwUnk1, DWORD dwUnk2, bool bUnk) override;

	virtual bool DrawSubtractiveLine(
		RectangleStruct* pRect, Point2D* pStart, Point2D* pEnd, ColorStruct* pColor,
		DWORD dwUnk1, DWORD dwUnk2, bool bUnk1, bool bUnk2,
		bool bUkn3, bool bUkn4, float fUkn) override;

	virtual bool DrawRGBMultiplyingLine(
		RectangleStruct* pRect, Point2D* pStart, Point2D* pEnd, ColorStruct* pColor,
		float Intensity, int zSource, int zTarget) override;

	virtual bool PlotLine(
		RectangleStruct* pRect, Point2D* pStart, Point2D* pEnd, bool(__fastcall* fpDrawCallback)(int*)) override;
	virtual bool DrawDashedLine(
		Point2D* pStart, Point2D* pEnd, int nColor, bool* Pattern, int nOffset) override;
	virtual bool DrawDashedLine_(
		Point2D* pStart, Point2D* pEnd, int nColor, bool* Pattern, int nOffset, bool bUkn) override;
	virtual bool DrawLine_(Point2D* pStart, Point2D* pEnd, int nColor, bool bUnk) override;
	virtual bool DrawRectEx(RectangleStruct* pClipRect, RectangleStruct* pDrawRect, int nColor) override;
	virtual bool DrawRect(RectangleStruct* pDrawRect, DWORD dwColor) override;
	virtual void* Lock(int X, int Y) override;
	virtual bool Unlock() override;
	virtual bool CanLock(DWORD dwUkn1 = 0, DWORD dwUkn2 = 0) override;
	virtual bool vt_entry_68(DWORD dwUnk1, DWORD dwUnk2) override;
	virtual bool IsLocked() override;
	virtual int GetBytesPerPixel() override;
	virtual int GetPitch() override;
	virtual RectangleStruct* GetRect(RectangleStruct* pRect) override;
	virtual int GetWidth() override;
	virtual int GetHeight() override;
	virtual bool IsDSurface() override;
	virtual bool PutPixelClip(Point2D* pPoint, short nUkn, RectangleStruct* pRect) override;
	virtual short GetPixelClip(Point2D* pPoint, RectangleStruct* pRect) override;
	virtual bool DrawGradientLine(RectangleStruct* pRect, Point2D* pStart, Point2D* pEnd,
		ColorStruct* pStartColor, ColorStruct* pEndColor, float fStep, int nColor) override;
	virtual bool CanBlit() override;

	void* GetBuffer();
};
