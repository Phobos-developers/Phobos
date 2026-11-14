#include "ZoomCommands.h"

#include <Misc/ZoomManager.h>
#include <Utilities/GeneralUtils.h>

const char* ZoomInCommandClass::GetName() const
{
	return "Zoom In";
}

const wchar_t* ZoomInCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_ZOOM_IN", L"Zoom In");
}

const wchar_t* ZoomInCommandClass::GetUICategory() const
{
	return CATEGORY_INTERFACE;
}

const wchar_t* ZoomInCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_ZOOM_IN_DESC", L"Magnifies the tactical battlefield view.");
}

void ZoomInCommandClass::Execute(WWKey eInput) const
{
	if (!ZoomManager::KeyEnabled || !ZoomManager::CanPlayerZoom())
		return;

	ZoomManager::ZoomIn();
}

const char* ZoomOutCommandClass::GetName() const
{
	return "Zoom Out";
}

const wchar_t* ZoomOutCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_ZOOM_OUT", L"Zoom Out");
}

const wchar_t* ZoomOutCommandClass::GetUICategory() const
{
	return CATEGORY_INTERFACE;
}

const wchar_t* ZoomOutCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_ZOOM_OUT_DESC", L"Reduces the tactical battlefield magnification.");
}

void ZoomOutCommandClass::Execute(WWKey eInput) const
{
	if (!ZoomManager::KeyEnabled || !ZoomManager::CanPlayerZoom())
		return;

	ZoomManager::ZoomOut();
}

const char* ResetZoomCommandClass::GetName() const
{
	return "Reset Zoom";
}

const wchar_t* ResetZoomCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_RESET_ZOOM", L"Reset Zoom");
}

const wchar_t* ResetZoomCommandClass::GetUICategory() const
{
	return CATEGORY_INTERFACE;
}

const wchar_t* ResetZoomCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_RESET_ZOOM_DESC", L"Restores the tactical view to normal magnification.");
}

void ResetZoomCommandClass::Execute(WWKey eInput) const
{
	if (!ZoomManager::KeyEnabled || !ZoomManager::CanPlayerZoom())
		return;

	ZoomManager::ResetZoom();
}
