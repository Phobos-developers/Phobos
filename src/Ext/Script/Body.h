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

enum class PhobosScripts : unsigned int
{
	TimedAreaGuard = 71,
	LoadIntoTransports = 72,
	WaitUntilFullAmmo = 73,
	RepeatAttackCloserThreat = 74,
	RepeatAttackFartherThreat = 75,
	RepeatAttackCloser = 76,
	RepeatAttackFarther = 77,
	SingleAttackCloserThreat = 78,
	SingleAttackFartherThreat = 79,
	SingleAttackCloser = 80,
	SingleAttackFarther = 81,
	DecreaseCurrentAITriggerWeight = 82,
	IncreaseCurrentAITriggerWeight = 83,
	RepeatAttackTypeCloserThreat = 84,
	RepeatAttackTypeFartherThreat = 85,
	RepeatAttackTypeCloser = 86,
	RepeatAttackTypeFarther = 87,
	SingleAttackTypeCloserThreat = 88,
	SingleAttackTypeFartherThreat = 89,
	SingleAttackTypeCloser = 90,
	SingleAttackTypeFarther = 91,
	WaitIfNoTarget = 92,
	TeamWeightReward = 93,
	PickRandomScript = 94,
	MoveToEnemyCloser = 95,
	MoveToEnemyFarther = 96,
	MoveToFriendlyCloser = 97,
	MoveToFriendlyFarther = 98,
	MoveToTypeEnemyCloser = 99,
	MoveToTypeEnemyFarther = 100,
	MoveToTypeFriendlyCloser = 101,
	MoveToTypeFriendlyFarther = 102,
	ModifyTargetDistance = 103,
	RandomAttackTypeCloser = 104,
	RandomAttackTypeFarther = 105,
	RandomMoveToTypeEnemyCloser = 106,
	RandomMoveToTypeEnemyFarther = 107,
	RandomMoveToTypeFriendlyCloser = 108,
	RandomMoveToTypeFriendlyFarther = 109,
	SetMoveMissionEndMode = 110,
	UnregisterGreatSuccess = 111,
	GatherAroundLeader = 112,
	RandomSkipNextAction = 113,
	StopForceJumpCountdown = 124,
	NextLineForceJumpCountdown = 125,
	SameLineForceJumpCountdown = 126,
	UnloadFromTransports = 135,

	// Variables
	LocalVariableSet = 500,
	LocalVariableAdd = 501,
	LocalVariableMinus = 502,
	LocalVariableMultiply = 503,
	LocalVariableDivide = 504,
	LocalVariableMod = 505,
	LocalVariableLeftShift = 506,
	LocalVariableRightShift = 507,
	LocalVariableReverse = 508,
	LocalVariableXor = 509,
	LocalVariableOr = 510,
	LocalVariableAnd = 511,
	GlobalVariableSet = 512,
	GlobalVariableAdd = 513,
	GlobalVariableMinus = 514,
	GlobalVariableMultiply = 515,
	GlobalVariableDivide = 516,
	GlobalVariableMod = 517,
	GlobalVariableLeftShift = 518,
	GlobalVariableRightShift = 519,
	GlobalVariableReverse = 520,
	GlobalVariableXor = 521,
	GlobalVariableOr = 522,
	GlobalVariableAnd = 523,
	LocalVariableSetByLocal = 524,
	LocalVariableAddByLocal = 525,
	LocalVariableMinusByLocal = 526,
	LocalVariableMultiplyByLocal = 527,
	LocalVariableDivideByLocal = 528,
	LocalVariableModByLocal = 529,
	LocalVariableLeftShiftByLocal = 530,
	LocalVariableRightShiftByLocal = 531,
	LocalVariableReverseByLocal = 532,
	LocalVariableXorByLocal = 533,
	LocalVariableOrByLocal = 534,
	LocalVariableAndByLocal = 535,
	GlobalVariableSetByLocal = 536,
	GlobalVariableAddByLocal = 537,
	GlobalVariableMinusByLocal = 538,
	GlobalVariableMultiplyByLocal = 539,
	GlobalVariableDivideByLocal = 540,
	GlobalVariableModByLocal = 541,
	GlobalVariableLeftShiftByLocal = 542,
	GlobalVariableRightShiftByLocal = 543,
	GlobalVariableReverseByLocal = 544,
	GlobalVariableXorByLocal = 545,
	GlobalVariableOrByLocal = 546,
	GlobalVariableAndByLocal = 547,
	LocalVariableSetByGlobal = 548,
	LocalVariableAddByGlobal = 549,
	LocalVariableMinusByGlobal = 550,
	LocalVariableMultiplyByGlobal = 551,
	LocalVariableDivideByGlobal = 552,
	LocalVariableModByGlobal = 553,
	LocalVariableLeftShiftByGlobal = 554,
	LocalVariableRightShiftByGlobal = 555,
	LocalVariableReverseByGlobal = 556,
	LocalVariableXorByGlobal = 557,
	LocalVariableOrByGlobal = 558,
	LocalVariableAndByGlobal = 559,
	GlobalVariableSetByGlobal = 560,
	GlobalVariableAddByGlobal = 561,
	GlobalVariableMinusByGlobal = 562,
	GlobalVariableMultiplyByGlobal = 563,
	GlobalVariableDivideByGlobal = 564,
	GlobalVariableModByGlobal = 565,
	GlobalVariableLeftShiftByGlobal = 566,
	GlobalVariableRightShiftByGlobal = 567,
	GlobalVariableReverseByGlobal = 568,
	GlobalVariableXorByGlobal = 569,
	GlobalVariableOrByGlobal = 570,
	GlobalVariableAndByGlobal = 571,
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
	static FootClass* FindTheTeamLeader(TeamClass* pTeam);
	static void Set_ForceJump_Countdown(TeamClass *pTeam, bool repeatLine, int count);
	static void Stop_ForceJump_Countdown(TeamClass *pTeam);

	static bool IsExtVariableAction(int action);
	static void VariablesHandler(TeamClass* pTeam, PhobosScripts eAction, int nArg);
	template<bool IsGlobal, class _Pr>
	static void VariableOperationHandler(TeamClass* pTeam, int nVariable, int Number);
	template<bool IsSrcGlobal, bool IsGlobal, class _Pr>
	static void VariableBinaryOperationHandler(TeamClass* pTeam, int nVariable, int nVarToOperate);

	static void UnloadFromTransports(TeamClass* pTeam);

	static ExtContainer ExtMap;

private:
	static void ModifyCurrentTriggerWeight(TeamClass* pTeam, bool forceJumpLine, double modifier);
	static bool MoveMissionEndStatus(TeamClass* pTeam, TechnoClass* pFocus, FootClass* pLeader, int mode);
};
