#pragma once

#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <TEventClass.h>

class HouseClass;

enum PhobosTriggerEvent
{
	LocalVariableGreaterThan = 500,
	LocalVariableLessThan = 501,
	LocalVariableEqualsTo = 502,
	LocalVariableGreaterThanOrEqualsTo = 503,
	LocalVariableLessThanOrEqualsTo = 504,
	LocalVariableAndIsTrue = 505,
	GlobalVariableGreaterThan = 506,
	GlobalVariableLessThan = 507,
	GlobalVariableEqualsTo = 508,
	GlobalVariableGreaterThanOrEqualsTo = 509,
	GlobalVariableLessThanOrEqualsTo = 510,
	GlobalVariableAndIsTrue = 511,
	LocalVariableGreaterThanLocalVariable = 512,
	LocalVariableLessThanLocalVariable = 513,
	LocalVariableEqualsToLocalVariable = 514,
	LocalVariableGreaterThanOrEqualsToLocalVariable = 515,
	LocalVariableLessThanOrEqualsToLocalVariable = 516,
	LocalVariableAndIsTrueLocalVariable = 517,
	GlobalVariableGreaterThanLocalVariable = 518,
	GlobalVariableLessThanLocalVariable = 519,
	GlobalVariableEqualsToLocalVariable = 520,
	GlobalVariableGreaterThanOrEqualsToLocalVariable = 521,
	GlobalVariableLessThanOrEqualsToLocalVariable = 522,
	GlobalVariableAndIsTrueLocalVariable = 523,
	LocalVariableGreaterThanGlobalVariable = 524,
	LocalVariableLessThanGlobalVariable = 525,
	LocalVariableEqualsToGlobalVariable = 526,
	LocalVariableGreaterThanOrEqualsToGlobalVariable = 527,
	LocalVariableLessThanOrEqualsToGlobalVariable = 528,
	LocalVariableAndIsTrueGlobalVariable = 529,
	GlobalVariableGreaterThanGlobalVariable = 530,
	GlobalVariableLessThanGlobalVariable = 531,
	GlobalVariableEqualsToGlobalVariable = 532,
	GlobalVariableGreaterThanOrEqualsToGlobalVariable = 533,
	GlobalVariableLessThanOrEqualsToGlobalVariable = 534,
	GlobalVariableAndIsTrueGlobalVariable = 535,

	ShieldBroken = 600,
	HouseOwnsTechnoType = 601,
	HouseDoesntOwnTechnoType = 602,
	CellHasTechnoType = 604,
	CellHasAnyTechnoTypeFromList = 605,
	AttachedIsUnderAttachedEffect = 606,
	AttachedIsUnderWebby = 607,

	_DummyMaximum,
};

class TEventExt final : public AbstractExt
{
public:
	using base_type = TEventClass;

	// deprecated: the pre-rework nested data class is now the extension class itself
	using ExtData [[deprecated("use the extension class itself instead")]] = TEventExt;

	static constexpr DWORD Canary = 0x91919191;

public:
	// typed owner accessor
	TEventClass* OwnerObject() const
	{
		return static_cast<TEventClass*>(this->GetAttachedObject());
	}

	TEventExt(TEventClass* const OwnerObject) : AbstractExt(OwnerObject)
	{ }

	virtual ~TEventExt() = default;

	virtual void LoadFromStream(PhobosStreamReader& Stm) override;
	virtual void SaveToStream(PhobosStreamWriter& Stm) override;

private:
	template <typename T>
	void Serialize(T& Stm);

public:

	static int GetFlags(int iEvent);

	static std::optional<bool> Execute(TEventClass* pThis, int iEvent, HouseClass* pHouse,
		ObjectClass* pObject, CDTimerClass* pTimer, bool* isPersitant, TechnoClass* pSource);

	template<bool IsGlobal, typename _Pr>
	static bool VariableCheck(TEventClass* pThis);
	template<bool IsSrcGlobal, bool IsGlobal, typename _Pr>
	static bool VariableCheckBinary(TEventClass* pThis);

	static bool HouseOwnsTechnoTypeTEvent(TEventClass* pThis);
	static bool HouseDoesntOwnTechnoTypeTEvent(TEventClass* pThis);

	static bool CellHasAnyTechnoTypeFromListTEvent(TEventClass* pThis, ObjectClass* pObject, HouseClass* pHouse);
	static bool CellHasTechnoTypeTEvent(TEventClass* pThis, ObjectClass* pObject, HouseClass* pHouse);
	static bool AttachedIsUnderWebbyTEvent(ObjectClass* pObject);

	static bool AttachedIsUnderAttachedEffectTEvent(TEventClass* pThis, ObjectClass* pObject);


public:
	class ExtContainer final : public Container<TEventExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static TEventExt* Fetch(const TEventClass* pThis)
	{
		return AbstractExt::Fetch<TEventExt>(pThis);
	}

	static TEventExt* TryFetch(const TEventClass* pThis)
	{
		return AbstractExt::TryFetch<TEventExt>(pThis);
	}
};

