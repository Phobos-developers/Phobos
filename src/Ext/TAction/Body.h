#pragma once

#include <Utilities/Container.h>
#include <Utilities/Template.h>

#include <Helpers/Template.h>

#include <TActionClass.h>

#include <string>
#include <optional>

class HouseClass;

enum class PhobosTriggerAction : unsigned int
{
	SaveGame = 500,
	EditVariable,
	GenerateRandomNumber,
	PrintVariableValue,
	BinaryOperation,
};

class TActionExt
{
public:
	using base_type = TActionClass;

	class ExtData final : public Extension<TActionClass>
	{
	public:
		ExtData(TActionClass* const OwnerObject) : Extension<TActionClass>(OwnerObject)
		{ }

		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	static bool Execute(TActionClass* pThis, HouseClass* pHouse,
			ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location, bool& bHandled);

#pragma push_macro("ACTION_FUNC")
#define ACTION_FUNC(name) \
	static bool name(TActionClass* pThis, HouseClass* pHouse, \
		ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)

	ACTION_FUNC(PlayAudioAtRandomWP);
	ACTION_FUNC(SaveGame);
	ACTION_FUNC(EditVariable);
	ACTION_FUNC(GenerateRandomNumber);
	ACTION_FUNC(PrintVariableValue);
	ACTION_FUNC(BinaryOperation);

#undef ACTION_FUNC
#pragma pop_macro("ACTION_FUNC")

	static std::optional<std::wstring> PendingSaveGameByTrigger;

	class ExtContainer final : public Container<TActionExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
};
