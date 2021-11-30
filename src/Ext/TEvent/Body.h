#pragma once

#include <Utilities/Container.h>
#include <Utilities/Template.h>

#include <Helpers/Template.h>

#include <TEventClass.h>

class HouseClass;

enum PhobosTriggerEvent
{
	LocalVariableGreaterThan = 500,
	LocalVariableLessThan,
	LocalVariableEqualsTo,
	LocalVariableGreaterThanOrEqualsTo,
	LocalVariableLessThanOrEqualsTo,
	LocalVariableAndIsTrue,
	GlobalVariableGreaterThan,
	GlobalVariableLessThan,
	GlobalVariableEqualsTo,
	GlobalVariableGreaterThanOrEqualsTo,
	GlobalVariableLessThanOrEqualsTo,
	GlobalVariableAndIsTrue,
	LocalVariableGreaterThanLocalVariable,
	LocalVariableLessThanLocalVariable,
	LocalVariableEqualsToLocalVariable,
	LocalVariableGreaterThanOrEqualsToLocalVariable,
	LocalVariableLessThanOrEqualsToLocalVariable,
	LocalVariableAndIsTrueLocalVariable,
	GlobalVariableGreaterThanLocalVariable,
	GlobalVariableLessThanLocalVariable,
	GlobalVariableEqualsToLocalVariable,
	GlobalVariableGreaterThanOrEqualsToLocalVariable,
	GlobalVariableLessThanOrEqualsToLocalVariable,
	GlobalVariableAndIsTrueLocalVariable,
	LocalVariableGreaterThanGlobalVariable,
	LocalVariableLessThanGlobalVariable,
	LocalVariableEqualsToGlobalVariable,
	LocalVariableGreaterThanOrEqualsToGlobalVariable,
	LocalVariableLessThanOrEqualsToGlobalVariable,
	LocalVariableAndIsTrueGlobalVariable,
	GlobalVariableGreaterThanGlobalVariable,
	GlobalVariableLessThanGlobalVariable,
	GlobalVariableEqualsToGlobalVariable,
	GlobalVariableGreaterThanOrEqualsToGlobalVariable,
	GlobalVariableLessThanOrEqualsToGlobalVariable,
	GlobalVariableAndIsTrueGlobalVariable,
};

class TEventExt
{
public:
	using base_type = TEventClass;

	class ExtData final : public Extension<TEventClass>
	{
	public:
		ExtData(TEventClass* const OwnerObject) : Extension<TEventClass>(OwnerObject)
		{ }

		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	static bool Execute(TEventClass* pThis, int iEvent, HouseClass* pHouse, ObjectClass* pObject,
					TimerStruct* pTimer, bool* isPersitant, TechnoClass* pSource, bool& bHandled);

	template<bool IsGlobal, typename _Pr>
	static bool VariableCheck(TEventClass* pThis);
	template<bool IsSrcGlobal, bool IsGlobal, typename _Pr>
	static bool VariableCheckBinary(TEventClass* pThis);

	class ExtContainer final : public Container<TEventExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
};
