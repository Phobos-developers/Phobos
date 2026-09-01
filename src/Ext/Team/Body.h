#pragma once
#include <TeamClass.h>
#include <AITriggerTypeClass.h>

#include <Ext/TeamType/Body.h>
#include <Ext/HouseType/Body.h>
#include <Ext/House/Body.h>
#include <Ext/Rules/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Phobos.h>

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

	static bool HouseOwns(AITriggerTypeClass* pThis, HouseClass* pHouse, bool allies, TechnoTypeClass* pItem);
	static bool HouseOwns(AITriggerTypeClass* pThis, HouseClass* pHouse, bool allies, const std::vector<TechnoTypeClass*>& list);
	static bool HouseOwnsAll(AITriggerTypeClass* pThis, HouseClass* pHouse, const std::vector<TechnoTypeClass*>& list);
	static bool EnemyOwns(AITriggerTypeClass* pThis, HouseClass* pHouse, HouseClass* pEnemy, bool onlySelectedEnemy, TechnoTypeClass* pItem);
	static bool EnemyOwns(AITriggerTypeClass* pThis, HouseClass* pHouse, HouseClass* pEnemy, bool onlySelectedEnemy, const std::vector<TechnoTypeClass*>& list);
	static bool EnemyOwnsAll(AITriggerTypeClass* pThis, HouseClass* pHouse, HouseClass* pEnemy, const std::vector<TechnoTypeClass*>& list);
	static bool NeutralOwns(AITriggerTypeClass* pThis, TechnoTypeClass* pItem);
	static bool NeutralOwns(AITriggerTypeClass* pThis, const std::vector<TechnoTypeClass*>& list);
	static bool NeutralOwnsAll(AITriggerTypeClass* pThis, const std::vector<TechnoTypeClass*>& list);
	static bool CountConditionMet(AITriggerTypeClass* pThis, int nObjects);
	static bool EvaluateTriggerCondition(
		AITriggerTypeClass* pTrigger,
		HouseClass* pHouse,
		HouseClass* pTargetHouse = nullptr,
		bool hasReachedMaxDefensiveTeamsLimit = false,
		int destroyedBridgesCount = 0,
		int undamagedBridgesCount = 0);

	static TeamExt* Fetch(const TeamClass* pThis)
	{
		return AbstractExt::Fetch<TeamExt>(pThis);
	}

	static TeamExt* TryFetch(const TeamClass* pThis)
	{
		return AbstractExt::TryFetch<TeamExt>(pThis);
	}

};
