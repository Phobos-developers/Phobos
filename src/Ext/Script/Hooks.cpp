#include "Body.h"

#include <MapClass.h>
#include <ThemeClass.h>

#include <Ext/House/Body.h>
#include <Helpers/Macro.h>

enum class BuildingWithProperty : unsigned int
{
	LeastThreat = 0,
	HighestThreat = 65536,
	Nearest = 131072,
	Farthest = 196608
};

DEFINE_HOOK(0x6E9443, TeamClass_AI, 0x8)
{
	GET(TeamClass*, pTeam, ESI);

	auto const pTeamData = TeamExt::ExtMap.Find(pTeam);

	// Force a line jump. This should support vanilla YR Actions
	if (pTeamData->ForceJump_InitialCountdown > 0 && pTeamData->ForceJump_Countdown.Expired())
	{
		auto const pScript = pTeam->CurrentScript;

		if (pTeamData->ForceJump_RepeatMode)
		{
			pScript->CurrentMission--;
			pTeam->Focus = nullptr;
			pTeam->QueuedFocus = nullptr;
			//pTeamData->selectedTarget; // TO-DO When a specific PR is merged
			//ScriptExt::Log("AI Scripts - Team Update: [%s] [%s](line: %d = %d,%d): Jump to the same line -> (Reason: Timed Jump loop)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission + 1, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Action, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Argument);

			if (pTeamData->ForceJump_InitialCountdown > 0)
			{
				pTeamData->ForceJump_Countdown.Start(pTeamData->ForceJump_InitialCountdown);
				pTeamData->ForceJump_RepeatMode = true;
			}
		}
		else
		{
			pTeamData->ForceJump_InitialCountdown = -1;
			//ScriptExt::Log("AI Scripts - Team Update: [%s] [%s](line: %d = %d,%d): Jump to line: %d = %d,%d -> (Reason: Timed Jump)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, pScript->CurrentMission + 1, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Action, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Argument);
		}

		for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		{
			if (ScriptExt::IsUnitAvailable(pUnit, false))
			{
				pUnit->SetTarget(nullptr);
				pUnit->SetDestination(nullptr, false);
				pUnit->QueueMission(Mission::Guard, true);
			}
		}

		pTeam->StepCompleted = true;
		return 0;
	}

	ScriptExt::ProcessAction(pTeam);

	return 0;
}

// Take NewINIFormat into account just like the other classes does
// Author: secsome
DEFINE_HOOK(0x6E95B3, TeamClass_AI_MoveToCell, 0x6)
{
	if (!R->BL())
		return 0x6E95A4;

	GET(const int, nCoord, ECX);
	REF_STACK(CellStruct, cell, STACK_OFFSET(0x38, -0x28));

	// if ( NewINIFormat < 4 ) then divide 128
	// in other times we divide 1000
	const int nDivisor = ScenarioClass::NewINIFormat < 4 ? 128 : 1000;
	cell.X = static_cast<short>(nCoord % nDivisor);
	cell.Y = static_cast<short>(nCoord / nDivisor);

	R->EAX(MapClass::Instance.GetCellAt(cell));
	return 0x6E959C;
}

DEFINE_HOOK(0x6EFEFB, TMission_ChronoShiftToBuilding_SuperWeapons, 0x6)
{
	enum { SkipGameCode = 0x6EFF22 };

	GET(HouseClass*, pHouse, EBP);

	SuperClass* pSuperCSphere = nullptr;
	SuperClass* pSuperCWarp = nullptr;
	HouseExt::GetAIChronoshiftSupers(pHouse, pSuperCSphere, pSuperCWarp);
	R->ESI(pSuperCSphere);
	R->EBX(pSuperCWarp);

	return SkipGameCode;
}

DEFINE_HOOK(0x6F01B0, TMission_ChronoShiftToTarget_SuperWeapons, 0x6)
{
	enum { SkipGameCode = 0x6F01D9 };

	GET(HouseClass*, pHouse, EDI);
	REF_STACK(SuperClass*, pSuperCWarp, STACK_OFFSET(0x30, -0x1C));

	SuperClass* pSuperCSphere = nullptr;
	HouseExt::GetAIChronoshiftSupers(pHouse, pSuperCSphere, pSuperCWarp);
	R->EBX(pSuperCSphere);

	return SkipGameCode;
}

DEFINE_HOOK(0x723CA1, TeamMissionClass_FillIn_StringsSupport_and_id_masks, 0xB)
{
	enum { SkipCode = 0x723CD2 };

	GET(ScriptActionNode*, node, ECX);
	GET_STACK(char*, scriptActionLine, 0x8);

	int action = 0;
	int argument = 0;
	char* endptr;

	if (sscanf(scriptActionLine, "%d,%[^\n]", &action, Phobos::readBuffer) != 2)
	{
		node->Action = action;
		node->Argument = argument;
		R->ECX(node);

		return SkipCode;
	}

	long val = strtol(Phobos::readBuffer, &endptr, 10);

	if (*endptr == '\0'
		&& val >= std::numeric_limits<int>::min()
		&& val <= std::numeric_limits<int>::max())
	{
		// Integer case (the classic).
		argument = static_cast<int>(val);
	}
	else
	{
		// New strings case
		char textArgument[sizeof(Phobos::readBuffer)] = { 0 };

		action = action;
		strcpy_s(textArgument, Phobos::readBuffer);

		// Action masks: These actions translate IDs into indices while preserving the original action values.
		// The reason for using these masks is that some ScriptType actions rely on fixed indices rather than ID labels.
		// When these lists change, there's a high probability of breaking the original index of the pointed element
		char id[sizeof(AbstractTypeClass::ID)] = { 0 };
		char bwp[20] = { 0 };
		char* context = nullptr;
		int index = 0;
		int prefixIndex = 0;

		switch (static_cast<PhobosScripts>(action))
		{
		case PhobosScripts::ChangeToScriptByID:
			action = 17;
			index = ScriptTypeClass::FindIndex(textArgument);
			break;
		case PhobosScripts::ChangeToTeamTypeByID:
			action = 18;
			index = TeamTypeClass::FindIndex(textArgument);
			break;
		case PhobosScripts::ChangeToHouseByID:
			action = 20;
			index = HouseClass::FindIndexByName(textArgument);

			if (index < 0)
				ScriptExt::Log("AI Scripts - TeamMissionClass_FillIn_StringsSupport: Invalid House [%s]\n", textArgument);
			break;
		case PhobosScripts::PlaySpeechByID: // Note: PR 1900 needs to be merged into develop
			action = static_cast<int>(PhobosScripts::PlaySpeech);
			index = VoxClass::FindIndex(textArgument);
			break;
		case PhobosScripts::PlaySoundByID:
			action = 25;
			index = VocClass::FindIndex(textArgument);
			break;
		case PhobosScripts::PlayMovieByID:
			// Note: action "26" is currently impossible without an expert Phobos developer declaring the Movies class... in that case I could code the right FindIndex(textArgument) so sadly I'll skip "26" for now :-(
			action = 26;
			index = 0;
			break;
		case PhobosScripts::PlayThemeByID:
			action = 27;
			index = ThemeClass::Instance.FindIndex(textArgument);
			break;
		case PhobosScripts::PlayAnimationByID:
			action = 51;
			index = AnimTypeClass::FindIndex(textArgument);
			break;
		case PhobosScripts::AttackEnemyStructureByID:
		case PhobosScripts::MoveToEnemyStructureByID:
		case PhobosScripts::ChronoshiftTaskForceToStructureByID:
		case PhobosScripts::MoveToFriendlyStructureByID:
			if (PhobosScripts::AttackEnemyStructureByID == static_cast<PhobosScripts>(action))
				action = 46;
			else if (PhobosScripts::MoveToEnemyStructureByID == static_cast<PhobosScripts>(action))
				action = 47;
			else if (PhobosScripts::ChronoshiftTaskForceToStructureByID == static_cast<PhobosScripts>(action))
				action = 56;
			else if (PhobosScripts::MoveToFriendlyStructureByID == static_cast<PhobosScripts>(action))
				action = 58;

			/* BwP check:
			Information from https://modenc.renegadeprojects.com/ScriptTypes/ScriptActions
			Computed Value                           Description
			-------------------------------------		-------------------------------------------------------
			0 (Hex 0x0) + Building Index          -> Index of the instance of the building with least threat
			65536 (Hex 0x10000) + Building Index  -> Index of the instance of the building with highest threat
			131072 (Hex 0x20000) + Building Index -> Index of the instance of the building which is nearest
			196608 (Hex 0x30000) + Building Index -> Index of the instance of the building which is farthest
			*/

			//strcpy_s(id, strtok_s(textArgument, ",", &context));
			//_snprintf_s(bwp, sizeof(bwp), context);
			//strcpy_s(bwp, context);
			//context = nullptr;
			//strcpy_s(bwp, strtok_s(textArgument, ",", &context));

			if (sscanf(textArgument, "%[^,],%s", id, bwp) == 2)
			{
				index = BuildingTypeClass::FindIndex(id);

				if (index >= 0)
				{
					if (_strcmpi(bwp, "highestthreat") == 0)
						prefixIndex = static_cast<int>(BuildingWithProperty::HighestThreat);
					else if (_strcmpi(bwp, "nearest") == 0)
						prefixIndex = static_cast<int>(BuildingWithProperty::Nearest);
					else if (_strcmpi(bwp, "farthest") == 0)
						prefixIndex = static_cast<int>(BuildingWithProperty::Farthest);
				}
			}
			break;
		default:
			index = 0;
			break;
		}

		if (index >= 0)
			argument = prefixIndex + index;
	}

	node->Action = action;
	node->Argument = argument;
	R->ECX(node);

	return SkipCode;
}
