#include "TechnoExt.h"
#include <Ext/Techno/Body.h>
#include <vector>

std::vector<CalculateExtraThreatCallback> TechnoExtInterop::CalculateExtraThreatCallbacks = {};
std::vector<CalculateSightCallback> TechnoExtInterop::CalculateSightCallbacks = {};

DEFINE_EXPORT(HRESULT, ConvertToType_Phobos, FootClass* pThis, TechnoTypeClass* toType)
{
	if (!pThis || !toType)
		return E_POINTER;

	if (!TechnoExt::ConvertToType(pThis, toType))
		return E_INVALIDARG;

	return S_OK;
}

DEFINE_EXPORT(HRESULT, RegisterCalculateExtraThreatCallback, CalculateExtraThreatCallback callback)
{
	if (!callback)
		return E_POINTER;

	TechnoExtInterop::CalculateExtraThreatCallbacks.push_back(callback);
	return S_OK;
}

DEFINE_EXPORT(HRESULT, RegisterCalculateSightCallback, CalculateSightCallback callback)
{
	if (!callback)
		return E_POINTER;

	TechnoExtInterop::CalculateSightCallbacks.push_back(callback);
	return S_OK;
}
