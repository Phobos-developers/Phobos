#pragma once
#include <TriggerClass.h>
#include <timer.h>

#include <map>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/Constructs.h>
#include <Utilities/Template.h>

class TriggerExt
{
public:
	using base_type = TriggerClass;

	static constexpr DWORD Canary = 0x73171331;

	class ExtData final : public Extension<TriggerClass>
	{
	public:
		std::vector<TEventClass*> SortedEventsList;
		PhobosMap<int, CDTimerClass> SequentialTimers;
		std::map<int, int> SequentialTimersOriginalValue;
		PhobosMap<int, CDTimerClass> ParallelTimers;
		std::map<int, int> ParallelTimersOriginalValue;
		int SequentialSwitchModeIndex = -1;

		ExtData(TriggerClass* OwnerObject) : Extension<TriggerClass>(OwnerObject)
			, SortedEventsList {}
			, SequentialTimers {}
			, SequentialTimersOriginalValue {}
			, ParallelTimers {}
			, ParallelTimersOriginalValue {}
			, SequentialSwitchModeIndex { -1 }
		{ }

		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<TriggerExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
};
