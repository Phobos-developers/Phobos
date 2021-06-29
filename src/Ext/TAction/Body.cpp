#include "Body.h"

#include <Utilities/SavegameDef.h>

#include <Ext/Scenario/Body.h>

//Static init
template<> const DWORD Extension<TActionClass>::Canary = 0x91919191;
TActionExt::ExtContainer TActionExt::ExtMap;

// =============================
// load / save

template <typename T>
void TActionExt::ExtData::Serialize(T& Stm)
{
	//Stm;
}

void TActionExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<TActionClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void TActionExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<TActionClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool TActionExt::Execute(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject,
	TriggerClass* pTrigger, CellStruct const& location, bool& bHandled)
{
	bHandled = true;
	switch (pThis->ActionKind)
	{
	case TriggerAction::PlaySoundEffectRandom:
		return TActionExt::PlayAudioAtRandomWP(pThis, pHouse, pObject, pTrigger, location);
	default:
		bHandled = false;
		return true;
	};
}

bool TActionExt::PlayAudioAtRandomWP(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	std::vector<int> waypoints;
	waypoints.reserve(ScenarioExt::Global()->Waypoints.size());
	
	auto const pScen = ScenarioClass::Instance();

	for (auto pair : ScenarioExt::Global()->Waypoints)
		if (pScen->IsDefinedWaypoint(pair.first))
			waypoints.push_back(pair.first);

	if (waypoints.size() > 0)
	{
		auto const index = pScen->Random.RandomRanged(0, waypoints.size() - 1);
		auto const luckyWP = waypoints[index];
		auto const cell = pScen->GetWaypointCoords(luckyWP);
		auto const coords = CellClass::Cell2Coord(cell);
		VocClass::PlayIndexAtPos(pThis->Value, coords);
	}

	return true;
}

// =============================
// container

TActionExt::ExtContainer::ExtContainer() : Container("TActionClass") { }

TActionExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

#ifdef MAKE_GAME_SLOWER_FOR_NO_REASON
DEFINE_HOOK(6DD176, TActionClass_CTOR, 5)
{
	GET(TActionClass*, pItem, ESI);

	TActionExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(6E4761, TActionClass_SDDTOR, 6)
{
	GET(TActionClass*, pItem, ESI);

	TActionExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(6E3E30, TActionClass_SaveLoad_Prefix, 8)
DEFINE_HOOK(6E3DB0, TActionClass_SaveLoad_Prefix, 5)
{
	GET_STACK(TActionClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	TActionExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(6E3E29, TActionClass_Load_Suffix, 4)
{
	TActionExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(6E3E4A, TActionClass_Save_Suffix, 3)
{
	TActionExt::ExtMap.SaveStatic();
	return 0;
}
#endif
