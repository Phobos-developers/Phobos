#include "ZoomManager.h"

#include <GScreenClass.h>
#include <Surface.h>
#include <TacticalClass.h>
#include <Utilities/Macro.h>

#include <algorithm>

DEFINE_HOOK(0x692300, DisplayClass_ProcessClickCoords_TranslateCoordinates, 0x7)
{
	if (!ZoomManager::IsZoomed())
		return 0;

	GET_STACK(Point2D*, pPoint, 0x4);

	if (pPoint)
		*pPoint = ZoomManager::ScreenToTactical(*pPoint);

	return 0;
}

DEFINE_HOOK(0x4AC380, DisplayClass_UpdateDragBand_TranslateCoordinates, 0x6)
{
	if (!ZoomManager::IsZoomed())
		return 0;

	GET_STACK(Point2D*, pPoint, 0x4);

	if (pPoint)
		*pPoint = ZoomManager::ScreenToTactical(*pPoint);

	return 0;
}

DEFINE_HOOK(0x6930A0, ScrollClass_MessageHandler_MiddleClickReset, 0x5)
{
	GET_STACK(const UINT*, pMessage, 0x8);

	if (ZoomManager::IsZoomed() && ZoomManager::WheelEnabled && pMessage && *pMessage == WM_MBUTTONDOWN)
		ZoomManager::ResetZoom();

	return 0;
}

DEFINE_HOOK(0x4F4480, GScreenClass_Render_ZoomUpdate, 0x9)
{
	ZoomManager::Update();
	return 0;
}

DEFINE_HOOK(0x4F44CB, GScreenClass_Render_TacticalAndCommandBar, 0x5)
{
	if (!ZoomManager::IsZoomed() || !TacticalClass::Instance || !DSurface::Alternate || !DSurface::Composite)
		return 0;

	GET(GScreenClass*, pGScreen, ESI);
	GET_STACK(bool, flag, 0x08);
	GET_STACK(bool, bDraw, 0x0C);

	TacticalClass::Instance->Render(DSurface::Composite, flag, 0);

	DSurface* pComposite = DSurface::Composite;
	DSurface::Temp = DSurface::Alternate;

	TacticalClass::Instance->Render(DSurface::Alternate, flag, 1);
	TacticalClass::Instance->Render(DSurface::Alternate, flag, 2);

	DSurface::Temp = pComposite;
	ZoomManager::ApplyTacticalBlit();

	pGScreen->Draw(bDraw ? 1 : 0);

	return 0x4F451B;
}

