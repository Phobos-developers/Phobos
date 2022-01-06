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

enum PhobosScripts
{
	// FS-21 please finish your stuff here, thanks
	LocalVariableSet = 500,
	LocalVariableAdd,
	LocalVariableMinus,
	LocalVariableMultiply,
	LocalVariableDivide,
	LocalVariableMod,
	LocalVariableLeftShift,
	LocalVariableRightShift,
	LocalVariableReverse,
	LocalVariableXor,
	LocalVariableOr,
	LocalVariableAnd,
	GlobalVariableSet,
	GlobalVariableAdd,
	GlobalVariableMinus,
	GlobalVariableMultiply,
	GlobalVariableDivide,
	GlobalVariableMod,
	GlobalVariableLeftShift,
	GlobalVariableRightShift,
	GlobalVariableReverse,
	GlobalVariableXor,
	GlobalVariableOr,
	GlobalVariableAnd,
	LocalVariableSetByLocal,
	LocalVariableAddByLocal,
	LocalVariableMinusByLocal,
	LocalVariableMultiplyByLocal,
	LocalVariableDivideByLocal,
	LocalVariableModByLocal,
	LocalVariableLeftShiftByLocal,
	LocalVariableRightShiftByLocal,
	LocalVariableReverseByLocal,
	LocalVariableXorByLocal,
	LocalVariableOrByLocal,
	LocalVariableAndByLocal,
	GlobalVariableSetByLocal,
	GlobalVariableAddByLocal,
	GlobalVariableMinusByLocal,
	GlobalVariableMultiplyByLocal,
	GlobalVariableDivideByLocal,
	GlobalVariableModByLocal,
	GlobalVariableLeftShiftByLocal,
	GlobalVariableRightShiftByLocal,
	GlobalVariableReverseByLocal,
	GlobalVariableXorByLocal,
	GlobalVariableOrByLocal,
	GlobalVariableAndByLocal,
	LocalVariableSetByGlobal,
	LocalVariableAddByGlobal,
	LocalVariableMinusByGlobal,
	LocalVariableMultiplyByGlobal,
	LocalVariableDivideByGlobal,
	LocalVariableModByGlobal,
	LocalVariableLeftShiftByGlobal,
	LocalVariableRightShiftByGlobal,
	LocalVariableReverseByGlobal,
	LocalVariableXorByGlobal,
	LocalVariableOrByGlobal,
	LocalVariableAndByGlobal,
	GlobalVariableSetByGlobal,
	GlobalVariableAddByGlobal,
	GlobalVariableMinusByGlobal,
	GlobalVariableMultiplyByGlobal,
	GlobalVariableDivideByGlobal,
	GlobalVariableModByGlobal,
	GlobalVariableLeftShiftByGlobal,
	GlobalVariableRightShiftByGlobal,
	GlobalVariableReverseByGlobal,
	GlobalVariableXorByGlobal,
	GlobalVariableOrByGlobal,
	GlobalVariableAndByGlobal,
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
	static void Mission_Attack_List1Random(TeamClass *pTeam, bool repeatAction, int calcThreatMode, int attackAITargetType);
	static void Mission_Move_List(TeamClass *pTeam, int calcThreatMode, bool pickAllies, int attackAITargetType);
	static void Mission_Move_List1Random(TeamClass *pTeam, int calcThreatMode, bool pickAllies, int attackAITargetType, int idxAITargetTypeItem);
	static void SetCloseEnoughDistance(TeamClass *pTeam, double distance);
	static void SetMoveMissionEndMode(TeamClass* pTeam, int mode);
	static void SkipNextAction(TeamClass* pTeam, int successPercentage);

	static void VariablesHandler(TeamClass* pTeam, PhobosScripts eAction, int nArg);
	template<bool IsGlobal, class _Pr>
	static void VariableOperationHandler(TeamClass* pTeam, int nVariable, int Number);
	template<bool IsSrcGlobal, bool IsGlobal, class _Pr>
	static void VariableBinaryOperationHandler(TeamClass* pTeam, int nVariable, int nVarToOperate);
	static FootClass* FindTheTeamLeader(TeamClass* pTeam);

	static void LocalVariableAdd(TeamClass* pTeam, int nVariable, int Number);
	static void LocalVariableMultiply(TeamClass* pTeam, int nVariable, int Number);
	static void LocalVariableDivide(TeamClass* pTeam, int nVariable, int Number);
	static void LocalVariableMod(TeamClass* pTeam, int nVariable, int Number);
	static void LocalVariableLeftShift(TeamClass* pTeam, int nVariable, int Number);
	static void LocalVariableRightShift(TeamClass* pTeam, int nVariable, int Number);
	static void LocalVariableReverse(TeamClass* pTeam, int nVariable, int Number);
	static void LocalVariableXor(TeamClass* pTeam, int nVariable, int Number);
	static void LocalVariableOr(TeamClass* pTeam, int nVariable, int Number);
	static void LocalVariableAnd(TeamClass* pTeam, int nVariable, int Number);
	static void GlobalVariableAdd(TeamClass* pTeam, int nVariable, int Number);
	static void GlobalVariableMultiply(TeamClass* pTeam, int nVariable, int Number);
	static void GlobalVariableDivide(TeamClass* pTeam, int nVariable, int Number);
	static void GlobalVariableMod(TeamClass* pTeam, int nVariable, int Number);
	static void GlobalVariableLeftShift(TeamClass* pTeam, int nVariable, int Number);
	static void GlobalVariableRightShift(TeamClass* pTeam, int nVariable, int Number);
	static void GlobalVariableReverse(TeamClass* pTeam, int nVariable, int Number);
	static void GlobalVariableXor(TeamClass* pTeam, int nVariable, int Number);
	static void GlobalVariableOr(TeamClass* pTeam, int nVariable, int Number);
	static void GlobalVariableAnd(TeamClass* pTeam, int nVariable, int Number);

	static ExtContainer ExtMap;

private:
	static void ModifyCurrentTriggerWeight(TeamClass* pTeam, bool forceJumpLine, double modifier);
	static bool MoveMissionEndStatus(TeamClass* pTeam, TechnoClass* pFocus, FootClass* pLeader, int mode);
};
