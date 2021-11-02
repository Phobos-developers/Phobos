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

enum class PhobosScripts : int
{
	// FS-21 please finish your stuff here, thanks
	LocalVariableAdd = 500,
	LocalVariableMultiply,
	LocalVariableDivide,
	LocalVariableMod,
	LocalVariableLeftShift,
	LocalVariableRightShift,
	LocalVariableReverse,
	LocalVariableXor,
	LocalVariableOr,
	LocalVariableAnd,
	GlobalVariableAdd,
	GlobalVariableMultiply,
	GlobalVariableDivide,
	GlobalVariableMod,
	GlobalVariableLeftShift,
	GlobalVariableRightShift,
	GlobalVariableReverse,
	GlobalVariableXor,
	GlobalVariableOr,
	GlobalVariableAnd,

};

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
	static void TeamWeightReward(TeamClass *pTeam, double award);
	static void PickRandomScript(TeamClass * pTeam, int idxScriptsList);
	static void Mission_Move(TeamClass* pTeam, int calcThreatMode, bool pickAllies, int attackAITargetType, int idxAITargetTypeItem);
	static TechnoClass* FindBestObject(TechnoClass *pTechno, int method, int calcThreatMode, bool pickAllies, int attackAITargetType, int idxAITargetTypeItem);
	static void UnregisterGreatSuccess(TeamClass * pTeam);

	static void Mission_Attack_List(TeamClass *pTeam, bool repeatAction, int calcThreatMode, int attackAITargetType);

	static void LocalVariableAdd(int nVariable, int Number);
	static void LocalVariableMultiply(int nVariable, int Number);
	static void LocalVariableDivide(int nVariable, int Number);
	static void LocalVariableMod(int nVariable, int Number);
	static void LocalVariableLeftShift(int nVariable, int Number);
	static void LocalVariableRightShift(int nVariable, int Number);
	static void LocalVariableReverse(int nVariable, int Number);
	static void LocalVariableXor(int nVariable, int Number);
	static void LocalVariableOr(int nVariable, int Number);
	static void LocalVariableAnd(int nVariable, int Number);
	static void GlobalVariableAdd(int nVariable, int Number);
	static void GlobalVariableMultiply(int nVariable, int Number);
	static void GlobalVariableDivide(int nVariable, int Number);
	static void GlobalVariableMod(int nVariable, int Number);
	static void GlobalVariableLeftShift(int nVariable, int Number);
	static void GlobalVariableRightShift(int nVariable, int Number);
	static void GlobalVariableReverse(int nVariable, int Number);
	static void GlobalVariableXor(int nVariable, int Number);
	static void GlobalVariableOr(int nVariable, int Number);
	static void GlobalVariableAnd(int nVariable, int Number);

	static ExtContainer ExtMap;

private:
	static void ModifyCurrentTriggerWeight(TeamClass* pTeam, bool forceJumpLine, double modifier);
};
