#pragma once

#include "OwnerDraw.h"

#include "../Render/Functions.h"

#include <CommCtrl.h>
#include <windowsx.h>

#include <BitFont.h>
#include <BitText.h>
#include <OwnerDraw.h>
#include <Drawing.h>
#include <PCX.h>
#include <Phobos.h>
#include <RulesClass.h>
#include <ScenarioClass.h>
#include <SessionClass.h>
#include <StringTable.h>
#include <UI.h>
#include <Unsorted.h>
#include <VocClass.h>

#include <FileFormats/SHP.h>
#include <Memory.h>
#include <Surface.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cwchar>
#include <cstring>
#include <mbstring.h>
#include <iterator>
#include <new>
#include <string>
#include <vector>

inline constexpr int DialogProcWindowLongIndex = 4;
inline constexpr int PaintStateExtraIndex = 61;
inline constexpr int SavedFontExtraIndex = 56;
inline constexpr int SavedBkModeExtraIndex = 57;
inline constexpr int SavedBkColorExtraIndex = 58;
inline constexpr int SavedTextColorExtraIndex = 59;

WWWinData* FindOwnerDrawData(HWND hWnd);

WNDPROC FindWindowProc(OwnerDraw::HwndProcDict& procs, HWND hWnd);

bool IsEmpty(const WideWstring& text);

const wchar_t* GetWideTextBuffer(const WideWstring& text);

void DeleteSurfaceObject(Surface*& pSurface);

void ResetOwnerDrawCachedSurface(OwnerDrawDialogElement& data);

void ResetOwnerDrawCachedSurfaceTree(HWND rootHwnd);

void DeleteUnknownGameObject(void*& pObject);

void InsetSurfaceRect(RectangleStruct& rect, int x, int y);

int ConvertRGBToSurfaceColor(COLORREF color);

COLORREF AverageColor(COLORREF first, COLORREF second);

WORD BlendSurfacePixelTowardMasks(WORD destination, int alpha);

void BlendFillRect(const RectangleStruct& rect, Surface* pSurface, WORD color, int alpha);

void BlendGradientRect(const RectangleStruct& rect, Surface* pSurface, WORD color, int widthScale);

bool DrawAlphaLine(DSurface* pSurface, Point2D start, Point2D end, WORD color, BYTE alpha);

bool DrawAlphaBeveledRect(
	DSurface* pSurface,
	const RectangleStruct& rect,
	bool raised,
	int thickness,
	BYTE leftAlpha,
	BYTE topAlpha,
	BYTE rightAlpha,
	BYTE bottomAlpha)
;

int DrawBeveledBorder(Surface* pSurface, const RectangleStruct& rect, int thickness, int color);

BSurface* GetPCXSurface(const char* pFilename);

bool BlitTiledPCX(const RectangleStruct& rect, Surface* pDestination, Surface* pSource, int offsetX, int offsetY);

bool CopySurfacePart(Surface* pDestination, const RectangleStruct& toRect, Surface* pSource, const RectangleStruct& fromRect);

void DrawPCXCopy(Surface* pDestination, const RectangleStruct& rect, BSurface* pPCX);

LRESULT CallSelectedHandler(WNDPROC pSelectedWndProc, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void CleanupDestroyedWindow(HWND hWnd);

void DrawScrollArrow(Surface* pSurface, const RectangleStruct& rect, bool isUp, bool pressed, bool useGreyArt);

void EnsureScrollBarCache(
	OwnerDrawDialogElement& data,
	OwnerDrawDialogElement* pParentData,
	const RectangleStruct& localRect,
	const RectangleStruct& parentSourceRect)
;

int BitFontHeight(BitFont* pFont);

void CharToWideString(wchar_t* pBuffer, int capacity, const char* pText);

void WideToCharString(char* pBuffer, int capacity, const wchar_t* pText);
