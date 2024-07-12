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
	// Range 10000-10999 are team (aka ingame) actions
	// Sub-range 10000-10049 is for "attack" actions
	RepeatAttackCloser = 10000,
	SingleAttackCloser = 10001,
	RepeatAttackTypeCloser = 10002,
	SingleAttackTypeCloser = 10003,
	RandomAttackTypeCloser = 10004,
	RepeatAttackFarther = 10005,
	SingleAttackFarther = 10006,
	RepeatAttackTypeFarther = 10007,
	SingleAttackTypeFarther = 10008,
	RandomAttackTypeFarther = 10009,
	RepeatAttackCloserThreat = 10010,
	SingleAttackCloserThreat = 10011,
	RepeatAttackTypeCloserThreat = 10012,
	SingleAttackTypeCloserThreat = 10013,
	RepeatAttackFartherThreat = 10014,
	SingleAttackFartherThreat = 10015,
	RepeatAttackTypeFartherThreat = 10016,
	SingleAttackTypeFartherThreat = 10017,
	// Sub-range 10050-10099 is for "Move to" actions
	MoveToEnemyCloser = 10050,
	MoveToTypeEnemyCloser = 10051,
	RandomMoveToTypeEnemyCloser = 10052,
	MoveToFriendlyCloser = 10053,
	MoveToTypeFriendlyCloser = 10054,
	RandomMoveToTypeFriendlyCloser = 10055,
	MoveToEnemyFarther = 10056,
	MoveToTypeEnemyFarther = 10057,
	RandomMoveToTypeEnemyFarther = 10058,
	MoveToFriendlyFarther = 10059,
	MoveToTypeFriendlyFarther = 10060,
	RandomMoveToTypeFriendlyFarther = 10061,
	// Sub-range 10100-10999 is for "general purpose" actions
	TimedAreaGuard = 10100,
	WaitUntilFullAmmo = 10101,
	GatherAroundLeader = 10102,
	LoadIntoTransports = 10103,
	ChronoshiftToEnemyBase = 10104,

	// Range 12000-12999 are suplementary/setup pre-actions
	WaitIfNoTarget = 12000,
	ModifyTargetDistance = 12001,
	SetMoveMissionEndMode = 12002,

	// Range 14000-14999 are utility actions (angernodes manipulation, Team manipulation, etc)
	TeamWeightReward = 14000,
	IncreaseCurrentAITriggerWeight = 14001,
	DecreaseCurrentAITriggerWeight = 14002,
	UnregisterGreatSuccess = 14003,

	// Range 16000-16999 are flow control actions (jumps, change script, loops, breaks, etc)
	SameLineForceJumpCountdown = 16000,
	NextLineForceJumpCountdown = 16001,
	StopForceJumpCountdown = 16002,
	RandomSkipNextAction = 16003,
	PickRandomScript = 16004,
	JumpBackToPreviousScript = 16005,

	// Range 18000-18999 are variable actions
	LocalVariableSet = 18000,
	LocalVariableAdd = 18001,
	LocalVariableMinus = 18002,
	LocalVariableMultiply = 18003,
	LocalVariableDivide = 18004,
	LocalVariableMod = 18005,
	LocalVariableLeftShift = 18006,
	LocalVariableRightShift = 18007,
	LocalVariableReverse = 18008,
	LocalVariableXor = 18009,
	LocalVariableOr = 18010,
	LocalVariableAnd = 18011,
	GlobalVariableSet = 18012,
	GlobalVariableAdd = 18013,
	GlobalVariableMinus = 18014,
	GlobalVariableMultiply = 18015,
	GlobalVariableDivide = 18016,
	GlobalVariableMod = 18017,
	GlobalVariableLeftShift = 18018,
	GlobalVariableRightShift = 18019,
	GlobalVariableReverse = 18020,
	GlobalVariableXor = 18021,
	GlobalVariableOr = 18022,
	GlobalVariableAnd = 18023,
	LocalVariableSetByLocal = 18024,
	LocalVariableAddByLocal = 18025,
	LocalVariableMinusByLocal = 18026,
	LocalVariableMultiplyByLocal = 18027,
	LocalVariableDivideByLocal = 18028,
	LocalVariableModByLocal = 18029,
	LocalVariableLeftShiftByLocal = 18030,
	LocalVariableRightShiftByLocal = 18031,
	LocalVariableReverseByLocal = 18032,
	LocalVariableXorByLocal = 18033,
	LocalVariableOrByLocal = 18034,
	LocalVariableAndByLocal = 18035,
	GlobalVariableSetByLocal = 18036,
	GlobalVariableAddByLocal = 18037,
	GlobalVariableMinusByLocal = 18038,
	GlobalVariableMultiplyByLocal = 18039,
	GlobalVariableDivideByLocal = 18040,
	GlobalVariableModByLocal = 18041,
	GlobalVariableLeftShiftByLocal = 18042,
	GlobalVariableRightShiftByLocal = 18043,
	GlobalVariableReverseByLocal = 18044,
	GlobalVariableXorByLocal = 18045,
	GlobalVariableOrByLocal = 18046,
	GlobalVariableAndByLocal = 18047,
	LocalVariableSetByGlobal = 18048,
	LocalVariableAddByGlobal = 18049,
	LocalVariableMinusByGlobal = 18050,
	LocalVariableMultiplyByGlobal = 18051,
	LocalVariableDivideByGlobal = 18052,
	LocalVariableModByGlobal = 18053,
	LocalVariableLeftShiftByGlobal = 18054,
	LocalVariableRightShiftByGlobal = 18055,
	LocalVariableReverseByGlobal = 18056,
	LocalVariableXorByGlobal = 18057,
	LocalVariableOrByGlobal = 18058,
	LocalVariableAndByGlobal = 18059,
	GlobalVariableSetByGlobal = 18060,
	GlobalVariableAddByGlobal = 18061,
	GlobalVariableMinusByGlobal = 18062,
	GlobalVariableMultiplyByGlobal = 18063,
	GlobalVariableDivideByGlobal = 18064,
	GlobalVariableModByGlobal = 18065,
	GlobalVariableLeftShiftByGlobal = 18066,
	GlobalVariableRightShiftByGlobal = 18067,
	GlobalVariableReverseByGlobal = 18068,
	GlobalVariableXorByGlobal = 18069,
	GlobalVariableOrByGlobal = 18070,
	GlobalVariableAndByGlobal = 18071

	// Range 19000-19999 are miscellanous/uncategorized actions
};

class ScriptExt
{
public:
	using base_type = ScriptClass;

	static constexpr DWORD Canary = 0x3B3B3B3B;

	class ExtData final : public Extension<ScriptClass>
	{
	public:
		// Nothing yet

		ExtData(ScriptClass* OwnerObject) : Extension<ScriptClass>(OwnerObject)
			// Nothing yet
		{
		}

		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	};

	class ExtContainer final : public Container<ScriptExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static void ProcessAction(TeamClass* pTeam);
	static void ExecuteTimedAreaGuardAction(TeamClass* pTeam);
	static void LoadIntoTransports(TeamClass* pTeam);
	static void WaitUntilFullAmmoAction(TeamClass* pTeam);
	static void Mission_Gather_NearTheLeader(TeamClass* pTeam, int countdown);
	static void DecreaseCurrentTriggerWeight(TeamClass* pTeam, bool forceJumpLine, double modifier);
	static void IncreaseCurrentTriggerWeight(TeamClass* pTeam, bool forceJumpLine, double modifier);
	static void WaitIfNoTarget(TeamClass* pTeam, int attempts);
	static void TeamWeightReward(TeamClass* pTeam, double award);
	static void PickRandomScript(TeamClass* pTeam, int idxScriptsList);
	static void UnregisterGreatSuccess(TeamClass* pTeam);
	static void SetCloseEnoughDistance(TeamClass* pTeam, double distance);
	static void SetMoveMissionEndMode(TeamClass* pTeam, int mode);
	static void SkipNextAction(TeamClass* pTeam, int successPercentage);
	static FootClass* FindTheTeamLeader(TeamClass* pTeam);
	static void Set_ForceJump_Countdown(TeamClass* pTeam, bool repeatLine, int count);
	static void Stop_ForceJump_Countdown(TeamClass* pTeam);
	static void JumpBackToPreviousScript(TeamClass* pTeam);
	static void ChronoshiftToEnemyBase(TeamClass* pTeam, int extraDistance);

	static bool IsExtVariableAction(int action);
	static void VariablesHandler(TeamClass* pTeam, PhobosScripts eAction, int nArg);
	template<bool IsGlobal, class _Pr>
	static void VariableOperationHandler(TeamClass* pTeam, int nVariable, int Number);
	template<bool IsSrcGlobal, bool IsGlobal, class _Pr>
	static void VariableBinaryOperationHandler(TeamClass* pTeam, int nVariable, int nVarToOperate);
	static bool IsUnitAvailable(TechnoClass* pTechno, bool checkIfInTransportOrAbsorbed);
	static void Log(const char* pFormat, ...);
	// Mission.Attack.cpp
	static void Mission_Attack(TeamClass* pTeam, bool repeatAction, int calcThreatMode, int attackAITargetType, int IdxAITargetTypeItem);
	static TechnoClass* GreatestThreat(TechnoClass* pTechno, int method, int calcThreatMode, HouseClass* onlyTargetThisHouseEnemy, int attackAITargetType, int idxAITargetTypeItem, bool agentMode);
	static bool EvaluateObjectWithMask(TechnoClass* pTechno, int mask, int attackAITargetType, int idxAITargetTypeItem, TechnoClass* pTeamLeader);
	static void Mission_Attack_List(TeamClass* pTeam, bool repeatAction, int calcThreatMode, int attackAITargetType);
	static void Mission_Attack_List1Random(TeamClass* pTeam, bool repeatAction, int calcThreatMode, int attackAITargetType);
	static void CheckUnitTargetingCapabilities(TechnoClass* pTechno, bool& hasAntiGround, bool& hasAntiAir, bool agentMode);
	static bool IsUnitArmed(TechnoClass* pTechno);
	static bool IsUnitMindControlledFriendly(HouseClass* pHouse, TechnoClass* pTechno);

	// Mission.Move.cpp
	static void Mission_Move(TeamClass* pTeam, int calcThreatMode, bool pickAllies, int attackAITargetType, int idxAITargetTypeItem);
	static TechnoClass* FindBestObject(TechnoClass* pTechno, int method, int calcThreatMode, bool pickAllies, int attackAITargetType, int idxAITargetTypeItem);
	static void Mission_Move_List(TeamClass* pTeam, int calcThreatMode, bool pickAllies, int attackAITargetType);
	static void Mission_Move_List1Random(TeamClass* pTeam, int calcThreatMode, bool pickAllies, int attackAITargetType, int idxAITargetTypeItem);

private:
	static void ModifyCurrentTriggerWeight(TeamClass* pTeam, bool forceJumpLine, double modifier);
	static bool MoveMissionEndStatus(TeamClass* pTeam, TechnoClass* pFocus, FootClass* pLeader, int mode);
	static void ChronoshiftTeamToTarget(TeamClass* pTeam, TechnoClass* pTeamLeader, AbstractClass* pTarget);
};
