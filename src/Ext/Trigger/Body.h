#pragma once
#include <TriggerClass.h>
#include <timer.h>

#include <map>

#include <Utilities/Container.h>
#include <Utilities/Constructs.h>
#include <Utilities/Template.h>
#include <Utilities/TemplateDef.h>

class TEventClass;

class TriggerExt final : public AbstractExt
{
public:
	using base_type = TriggerClass;

	// deprecated: the pre-rework nested data class is now the extension class itself
	using ExtData [[deprecated("use the extension class itself instead")]] = TriggerExt;

	static constexpr DWORD Canary = 0x73171331;

public:
	// typed owner accessor
	TriggerClass* OwnerObject() const
	{
		return static_cast<TriggerClass*>(this->GetAttachedObject());
	}

	struct EventTimer
	{
		CDTimerClass Timer {};
		int Duration { 0 };
		bool IsRandom { false };
		bool Started { false };

		template <typename T>
		void Serialize(T& Stm)
		{
			Stm
				.Process(this->Timer)
				.Process(this->Duration)
				.Process(this->IsRandom)
				.Process(this->Started);
		}
	};

	PhobosMap<int, EventTimer> EventTimers {};

	TriggerExt(TriggerClass* const OwnerObject) : AbstractExt(OwnerObject)
		, EventTimers {}
	{ }

	virtual ~TriggerExt() = default;

	CDTimerClass* GetTimerForEvent(int eventIndex, TEventClass* pEvent, bool isParallel);
	void ResetAllTimers();

	virtual void LoadFromStream(PhobosStreamReader& Stm) override;
	virtual void SaveToStream(PhobosStreamWriter& Stm) override;

private:
	template <typename T>
	void Serialize(T& Stm);

public:
	class ExtContainer final : public Container<TriggerExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static TriggerExt* Fetch(const TriggerClass* pThis)
	{
		return AbstractExt::Fetch<TriggerExt>(pThis);
	}

	static TriggerExt* TryFetch(const TriggerClass* pThis)
	{
		return AbstractExt::TryFetch<TriggerExt>(pThis);
	}
};
