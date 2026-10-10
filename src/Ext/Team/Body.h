#pragma once
#include <TeamClass.h>

#include <Ext/TeamType/Body.h>

#include <Utilities/Container.h>
#include <Utilities/Detach.h>
#include <Utilities/TemplateDef.h>

class TeamExt final : public AbstractExt, public Detach::Listener<FootClass>
{
public:
	using base_type = TeamClass;

	// deprecated: the pre-rework nested data class is now the extension class itself
	using ExtData [[deprecated("use the extension class itself instead")]] = TeamExt;

	static constexpr DWORD Canary = 0x414B4B41;

public:
	// typed owner accessor
	TeamClass* OwnerObject() const
	{
		return static_cast<TeamClass*>(this->GetAttachedObject());
	}

	int WaitNoTargetAttempts;
	double NextSuccessWeightAward;
	int IdxSelectedObjectFromAIList;
	double CloseEnough;
	int Countdown_RegroupAtLeader;
	int MoveMissionEndMode;
	int WaitNoTargetCounter;
	CDTimerClass WaitNoTargetTimer;
	CDTimerClass ForceJump_Countdown;
	int ForceJump_InitialCountdown;
	bool ForceJump_RepeatMode;
	FootClass* TeamLeader;
	std::vector<ScriptClass*> PreviousScriptList;
	bool ConditionalJump_Evaluation;
	int ConditionalJump_ComparatorMode;
	int ConditionalJump_ComparatorValue;
	int ConditionalJump_Counter;
	int ConditionalJump_Index;
	bool AbortActionAfterKilling;
	bool ConditionalJump_EnabledKillsCount;
	bool ConditionalJump_ResetVariablesIfJump;

	TeamExt(TeamClass* OwnerObject) : AbstractExt(OwnerObject)
		, WaitNoTargetAttempts { 0 }
		, NextSuccessWeightAward { 0 }
		, IdxSelectedObjectFromAIList { -1 }
		, CloseEnough { -1 }
		, Countdown_RegroupAtLeader { -1 }
		, MoveMissionEndMode { 0 }
		, WaitNoTargetCounter { 0 }
		, WaitNoTargetTimer { }
		, ForceJump_Countdown { }
		, ForceJump_InitialCountdown { -1 }
		, ForceJump_RepeatMode { false }
		, TeamLeader { nullptr }
		, PreviousScriptList { }
		, ConditionalJump_Evaluation { false }
		, ConditionalJump_ComparatorMode { 3 }
		, ConditionalJump_ComparatorValue { 1 }
		, ConditionalJump_Counter { 0 }
		, ConditionalJump_Index { -1 }
		, AbortActionAfterKilling { false }
		, ConditionalJump_EnabledKillsCount { false }
		, ConditionalJump_ResetVariablesIfJump { true }
	{ }

	virtual ~TeamExt() = default;

	virtual void OnDetach(FootClass* pTarget, bool removed) override;

	virtual void LoadFromStream(PhobosStreamReader& Stm) override;
	virtual void SaveToStream(PhobosStreamWriter& Stm) override;

private:
	template <typename T>
	void Serialize(T& Stm);

public:
	class ExtContainer final : public Container<TeamExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static TeamExt* Fetch(const TeamClass* pThis)
	{
		return AbstractExt::Fetch<TeamExt>(pThis);
	}

	static TeamExt* TryFetch(const TeamClass* pThis)
	{
		return AbstractExt::TryFetch<TeamExt>(pThis);
	}

};

