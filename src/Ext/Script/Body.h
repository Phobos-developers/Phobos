#pragma once

#include <ScriptClass.h>
#include <ScriptTypeClass.h>
#include <TeamClass.h>
#include <AITriggerTypeClass.h>

#include <HouseClass.h>
#include <AircraftClass.h>
#include <MapClass.h>
#include <BulletClass.h>
#include <Helpers/Enumerators.h>
#include <WarheadTypeClass.h>
#include <SpawnManagerClass.h>

#include <Ext/Team/Body.h>
#include <Utilities/Container.h>
#include <Phobos.h>

class ScriptExt
{
public:
	using base_type = ScriptClass;

	class ExtData final : public Extension<ScriptClass>
	{
	public:
		// Nothing yet

		ExtData(ScriptClass* OwnerObject) : Extension<ScriptClass>(OwnerObject)
			// Nothing yet
		{ }

		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override {}

		virtual void LoadFromStream(PhobosStreamReader& Stm);
		virtual void SaveToStream(PhobosStreamWriter& Stm);

	};

	class ExtContainer final : public Container<ScriptExt> {
	public:
		ExtContainer();
		~ExtContainer();
	};

	static void ProcessAction(TeamClass * pTeam);
	static void ExecuteTimedAreaGuardAction(TeamClass * pTeam);
	static void LoadIntoTransports(TeamClass * pTeam);
	static void WaitUntilFullAmmoAction(TeamClass * pTeam);
	static void Mission_Gather_NearTheLeader(TeamClass *pTeam, int countdown);
	static void Mission_Attack(TeamClass* pTeam, bool repeatAction, int calcThreatMode, int attackAITargetType, int IdxAITargetTypeItem);
	static TechnoClass* GreatestThreat(TechnoClass* pTechno, int method, int calcThreatMode, HouseClass* onlyTargetThisHouseEnemy, int attackAITargetType, int idxAITargetTypeItem, bool agentMode);
	static bool EvaluateObjectWithMask(TechnoClass* pTechno, int mask, int attackAITargetType, int idxAITargetTypeItem, TechnoClass *pTeamLeader);

	static void DecreaseCurrentTriggerWeight(TeamClass* pTeam, bool forceJumpLine, double modifier);
	static void IncreaseCurrentTriggerWeight(TeamClass* pTeam, bool forceJumpLine, double modifier);
	static void WaitIfNoTarget(TeamClass *pTeam, int attempts);

	static void Mission_Attack_List(TeamClass *pTeam, bool repeatAction, int calcThreatMode, int attackAITargetType);

	static ExtContainer ExtMap;

private:
	static void ModifyCurrentTriggerWeight(TeamClass* pTeam, bool forceJumpLine, double modifier);
};
