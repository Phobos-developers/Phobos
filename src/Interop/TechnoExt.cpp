#include "TechnoExt.h"
#include <Ext/Techno/Body.h>
#include <vector>

std::vector<CalculateExtraThreatCallback> TechnoExtInterop::CalculateExtraThreatCallbacks = {};
std::vector<CalculateSightCallback> TechnoExtInterop::CalculateSightCallbacks = {};

DEFINE_EXPORT(bool, ConvertToType_Phobos, FootClass* pThis, TechnoTypeClass* toType)
{
	return TechnoExt::ConvertToType(pThis, toType);
}

DEFINE_EXPORT(void, RegisterCalculateExtraThreatCallback, CalculateExtraThreatCallback callback)
{
	if (callback)
		TechnoExtInterop::CalculateExtraThreatCallbacks.push_back(callback);
}

DEFINE_EXPORT(void, RegisterCalculateSightCallback, CalculateSightCallback callback)
{
	if (callback)
		TechnoExtInterop::CalculateSightCallbacks.push_back(callback);
}
