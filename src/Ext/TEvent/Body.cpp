#include "Body.h"

#include <Utilities/SavegameDef.h>

#include <Ext/Scenario/Body.h>
#include <BuildingClass.h>
#include <InfantryClass.h>
#include <UnitClass.h>
#include <AircraftClass.h>

//Static init
template<> const DWORD Extension<TEventClass>::Canary = 0x91919191;
TEventExt::ExtContainer TEventExt::ExtMap;

// =============================
// load / save

template <typename T>
void TEventExt::ExtData::Serialize(T& Stm)
{
	//Stm;
}

void TEventExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<TEventClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void TEventExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<TEventClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool TEventExt::Execute(TEventClass* pThis, int iEvent, HouseClass* pHouse, ObjectClass* pObject,
	TimerStruct* pTimer, bool* isPersitant, TechnoClass* pSource, bool& bHandled)
{
	bHandled = true;
	switch (static_cast<PhobosTriggerEvent>(pThis->EventKind))
	{
		// helper struct
		struct and_with { bool operator()(int a, int b) { return a & b; } };

	case PhobosTriggerEvent::LocalVariableGreaterThan:
		return TEventExt::VariableCheck<false, std::greater<int>>(pThis);
	case PhobosTriggerEvent::LocalVariableLessThan:
		return TEventExt::VariableCheck<false, std::less<int>>(pThis);
	case PhobosTriggerEvent::LocalVariableEqualsTo:
		return TEventExt::VariableCheck<false, std::equal_to<int>>(pThis);
	case PhobosTriggerEvent::LocalVariableGreaterThanOrEqualsTo:
		return TEventExt::VariableCheck<false, std::greater_equal<int>>(pThis);
	case PhobosTriggerEvent::LocalVariableLessThanOrEqualsTo:
		return TEventExt::VariableCheck<false, std::less_equal<int>>(pThis);
	case PhobosTriggerEvent::LocalVariableAndIsTrue:
		return TEventExt::VariableCheck<false, and_with>(pThis);
	case PhobosTriggerEvent::GlobalVariableGreaterThan:
		return TEventExt::VariableCheck<true, std::greater<int>>(pThis);
	case PhobosTriggerEvent::GlobalVariableLessThan:
		return TEventExt::VariableCheck<true, std::less<int>>(pThis);
	case PhobosTriggerEvent::GlobalVariableEqualsTo:
		return TEventExt::VariableCheck<true, std::equal_to<int>>(pThis);
	case PhobosTriggerEvent::GlobalVariableGreaterThanOrEqualsTo:
		return TEventExt::VariableCheck<true, std::greater_equal<int>>(pThis);
	case PhobosTriggerEvent::GlobalVariableLessThanOrEqualsTo:
		return TEventExt::VariableCheck<true, std::less_equal<int>>(pThis);
	case PhobosTriggerEvent::GlobalVariableAndIsTrue:
		return TEventExt::VariableCheck<true, and_with>(pThis);

	case PhobosTriggerEvent::LocalVariableGreaterThanLocalVariable:
		return TEventExt::VariableCheckBinary<false, false, std::greater<int>>(pThis);
	case PhobosTriggerEvent::LocalVariableLessThanLocalVariable:
		return TEventExt::VariableCheckBinary<false, false, std::less<int>>(pThis);
	case PhobosTriggerEvent::LocalVariableEqualsToLocalVariable:
		return TEventExt::VariableCheckBinary<false, false, std::equal_to<int>>(pThis);
	case PhobosTriggerEvent::LocalVariableGreaterThanOrEqualsToLocalVariable:
		return TEventExt::VariableCheckBinary<false, false, std::greater_equal<int>>(pThis);
	case PhobosTriggerEvent::LocalVariableLessThanOrEqualsToLocalVariable:
		return TEventExt::VariableCheckBinary<false, false, std::less_equal<int>>(pThis);
	case PhobosTriggerEvent::LocalVariableAndIsTrueLocalVariable:
		return TEventExt::VariableCheckBinary<false, false, and_with>(pThis);
	case PhobosTriggerEvent::GlobalVariableGreaterThanLocalVariable:
		return TEventExt::VariableCheckBinary<false, true, std::greater<int>>(pThis);
	case PhobosTriggerEvent::GlobalVariableLessThanLocalVariable:
		return TEventExt::VariableCheckBinary<false, true, std::less<int>>(pThis);
	case PhobosTriggerEvent::GlobalVariableEqualsToLocalVariable:
		return TEventExt::VariableCheckBinary<false, true, std::equal_to<int>>(pThis);
	case PhobosTriggerEvent::GlobalVariableGreaterThanOrEqualsToLocalVariable:
		return TEventExt::VariableCheckBinary<false, true, std::greater_equal<int>>(pThis);
	case PhobosTriggerEvent::GlobalVariableLessThanOrEqualsToLocalVariable:
		return TEventExt::VariableCheckBinary<false, true, std::less_equal<int>>(pThis);
	case PhobosTriggerEvent::GlobalVariableAndIsTrueLocalVariable:
		return TEventExt::VariableCheckBinary<false, true, and_with>(pThis);

	case PhobosTriggerEvent::LocalVariableGreaterThanGlobalVariable:
		return TEventExt::VariableCheckBinary<true, false, std::greater<int>>(pThis);
	case PhobosTriggerEvent::LocalVariableLessThanGlobalVariable:
		return TEventExt::VariableCheckBinary<true, false, std::less<int>>(pThis);
	case PhobosTriggerEvent::LocalVariableEqualsToGlobalVariable:
		return TEventExt::VariableCheckBinary<true, false, std::equal_to<int>>(pThis);
	case PhobosTriggerEvent::LocalVariableGreaterThanOrEqualsToGlobalVariable:
		return TEventExt::VariableCheckBinary<true, false, std::greater_equal<int>>(pThis);
	case PhobosTriggerEvent::LocalVariableLessThanOrEqualsToGlobalVariable:
		return TEventExt::VariableCheckBinary<true, false, std::less_equal<int>>(pThis);
	case PhobosTriggerEvent::LocalVariableAndIsTrueGlobalVariable:
		return TEventExt::VariableCheckBinary<true, false, and_with>(pThis);
	case PhobosTriggerEvent::GlobalVariableGreaterThanGlobalVariable:
		return TEventExt::VariableCheckBinary<true, true, std::greater<int>>(pThis);
	case PhobosTriggerEvent::GlobalVariableLessThanGlobalVariable:
		return TEventExt::VariableCheckBinary<true, true, std::less<int>>(pThis);
	case PhobosTriggerEvent::GlobalVariableEqualsToGlobalVariable:
		return TEventExt::VariableCheckBinary<true, true, std::equal_to<int>>(pThis);
	case PhobosTriggerEvent::GlobalVariableGreaterThanOrEqualsToGlobalVariable:
		return TEventExt::VariableCheckBinary<true, true, std::greater_equal<int>>(pThis);
	case PhobosTriggerEvent::GlobalVariableLessThanOrEqualsToGlobalVariable:
		return TEventExt::VariableCheckBinary<true, true, std::less_equal<int>>(pThis);
	case PhobosTriggerEvent::GlobalVariableAndIsTrueGlobalVariable:
		return TEventExt::VariableCheckBinary<true, true, and_with>(pThis);

	default:
		bHandled = false;
		return true;
	};
}

template<bool IsGlobal, class _Pr>
bool TEventExt::VariableCheck(TEventClass* pThis)
{
	auto itr = ScenarioExt::Global()->Variables[IsGlobal].find(pThis->Value);

	if (itr != ScenarioExt::Global()->Variables[IsGlobal].end())
	{
		// We uses TechnoName for our operator number
		int nOpt = atoi(pThis->TechnoName);
		return _Pr()(itr->second.Value, nOpt);
	}

	return false;
}

template<bool IsSrcGlobal, bool IsGlobal, class _Pr>
bool TEventExt::VariableCheckBinary(TEventClass* pThis)
{
	auto itr = ScenarioExt::Global()->Variables[IsGlobal].find(pThis->Value);

	if (itr != ScenarioExt::Global()->Variables[IsGlobal].end())
	{
		// We uses TechnoName for our src variable index
		int nSrcVariable = atoi(pThis->TechnoName);
		auto itrsrc = ScenarioExt::Global()->Variables[IsSrcGlobal].find(nSrcVariable);
		
		if (itrsrc != ScenarioExt::Global()->Variables[IsSrcGlobal].end())
			return _Pr()(itr->second.Value, itrsrc->second.Value);
	}

	return false;
}

// =============================
// container

TEventExt::ExtContainer::ExtContainer() : Container("TEventClass") { }

TEventExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

#ifdef MAKE_GAME_SLOWER_FOR_NO_REASON
DEFINE_HOOK(0x6DD176, TActionClass_CTOR, 0x5)
{
	GET(TActionClass*, pItem, ESI);

	TActionExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(0x6E4761, TActionClass_SDDTOR, 0x6)
{
	GET(TActionClass*, pItem, ESI);

	TActionExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x6E3E30, TActionClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x6E3DB0, TActionClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(TActionClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	TActionExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x6E3E29, TActionClass_Load_Suffix, 0x4)
{
	TActionExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x6E3E4A, TActionClass_Save_Suffix, 0x3)
{
	TActionExt::ExtMap.SaveStatic();
	return 0;
}
#endif
