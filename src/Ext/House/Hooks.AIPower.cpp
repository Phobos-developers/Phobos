#include "Body.h"

#include <Ext/BuildingType/Body.h>

#include <limits>

DEFINE_HOOK(0x4FE713, HouseClass_AI_BaseConstructionUpdate_AIPower, 0x6)
{
	enum { Produce = 0x4FE953, CheckExemptions = 0x4FE723 };

	GET(HouseClass* const, pThis, EBP);
	GET(BuildingTypeClass* const, pType, EDI);

	auto const& power = BuildingTypeExt::Fetch(pType)->AI_Power;
	auto const delta = HouseExt::Fetch(pThis)->AIPowerDelta;
	if (!power.isset() && delta == 0)
		return 0;

	const auto nodeDrain = Math::max(-BuildingTypeExt::GetAIPower(pType), 0LL);
	const auto available = static_cast<std::int64_t>(pThis->PowerOutput) - pThis->PowerDrain + delta;
	const auto required = nodeDrain + pThis->PowerSurplus;

	// The following exemption chain only tests the sign of ESI. Keep even the
	// magnitude of -INT_MIN safe while comparing the full value above in 64 bits.
	R->ESI(static_cast<int>(Math::min(nodeDrain, static_cast<std::int64_t>(std::numeric_limits<int>::max()))));
	return available >= required ? Produce : CheckExemptions;
}

// AITakeover arranges the initial base plan by its power budget. Recompute only
// the prefix already visited by the engine, never credit a later planned plant.
static bool GetAIPowerPrefix(HouseClass* const pThis, int nodeIndex, std::int64_t& balance)
{
	auto const& nodes = pThis->Base.BaseNodes;
	auto const& types = BuildingTypeClass::Array;
	bool overridden = false;
	balance = 0;

	for (int i = 0; i <= nodeIndex && i < nodes.Count; ++i)
	{
		const int index = nodes.Items[i].BuildingTypeIndex;
		if (index < 0 || index >= types.Count)
			continue;

		auto const pType = types.Items[index];
		if (!pType)
			continue;

		auto const& power = BuildingTypeExt::Fetch(pType)->AI_Power;
		balance += BuildingTypeExt::GetAIPower(pType);
		overridden |= power.isset();
	}

	return overridden;
}

DEFINE_HOOK_AGAIN(0x50AEBD, HouseClass_AITakeover_AIPower, 0x6)
DEFINE_HOOK(0x50ACAF, HouseClass_AITakeover_AIPower, 0x6)
{
	enum { ContinueFirst = 0x50ACBB, InsertFirst = 0x50ACDC,
		ContinueSecond = 0x50AEC9, InsertSecond = 0x50AED9 };

	GET(HouseClass* const, pThis, EBP);
	const bool first = R->Origin() == 0x50ACAF;
	const int nodeIndex = first ? R->EBX<int>() : R->ECX<int>();
	std::int64_t balance;
	if (!GetAIPowerPrefix(pThis, nodeIndex, balance))
		return 0;

	const bool sufficient = balance >= pThis->PowerSurplus;
	return first ? (sufficient ? ContinueFirst : InsertFirst) : (sufficient ? ContinueSecond : InsertSecond);
}
