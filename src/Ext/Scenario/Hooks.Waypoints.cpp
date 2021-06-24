#include <Helpers/Macro.h>

#include "Body.h"

#include <MapClass.h>

#include <string>

DEFINE_HOOK(68BCC0, ScenarioClass_Get_Waypoint_Location, B)
{
	GET_STACK(CellStruct*, pCell, 0x4);
	GET_STACK(int, nWaypoint, 0x8);

	*pCell = ScenarioExt::Global()->Waypoints[nWaypoint];

	R->EAX(pCell);

	return 0x68BCD1;
}

DEFINE_HOOK(68BCE4, ScenarioClass_Get_Waypoint_Cell_0, 7)
{
	GET_STACK(int, nWaypoint, 0x4);

	R->ECX(&ScenarioExt::Global()->Waypoints[nWaypoint]);

	return 0x68BCEB;
}

DEFINE_HOOK(68BD08, ScenarioClass_Get_Waypoint, 7)
{
	GET_STACK(int, nWaypoint, STACK_OFFS(0x10, -0x8));

	R->ECX(&ScenarioExt::Global()->Waypoints[nWaypoint]);

	return 0x68BD0F;
}

DEFINE_HOOK(68BD60, ScenarioClass_Clear_All_Waypoints, 6)
{
	ScenarioExt::Global()->Waypoints.clear();

	return 0x68BD79;
}

DEFINE_HOOK(68BD80, ScenarioClass_Is_Waypoint_Valid, 5)
{
	GET_STACK(int, nWaypoint, 0x4);
	auto const& waypoints = ScenarioExt::Global()->Waypoints;

	R->AL(nWaypoint >= 0 && waypoints.find(nWaypoint) != waypoints.end());

	return 0x68BDB3;
}

DEFINE_HOOK(68BDC0, ScenarioClass_ReadWaypoints, 8)
{
	GET_STACK(INIClass* const, pINI, 0x4);

	CellStruct buffer;

	for (int i = 0; i < pINI->GetKeyCount("Waypoints"); ++i)
	{
		const auto pName = pINI->GetKeyName("Waypoints", i);
		int id;
		if (sscanf_s(pName, "%d", &id) != 1 || id < 0)
			Debug::Log("[Developer Warning] Failed to parse waypoint %s.\n", pName);
		int nCoord = pINI->ReadInteger("Waypoints", pName, 0);
		
		if (nCoord)
		{
			buffer.X = static_cast<short>(nCoord % 1000);
			buffer.Y = static_cast<short>(nCoord / 1000);
			if (auto pCell = MapClass::Instance->TryGetCellAt(buffer))
				pCell->Flags |= cf_IsWaypoint;
			else
				Debug::Log("[Developer Warning] Can not get waypoint %d : [%d, %d]!\n", id, buffer.X, buffer.Y);
			ScenarioExt::Global()->Waypoints[id] = buffer;
		}
		else
			Debug::Log("[Developer Warning] Invalid waypoint %d!\n", id);
	}

	return 0x68BE8C;
}

DEFINE_HOOK(68BE90, ScenarioClass_Write_Waypoints, 5)
{
	GET_STACK(INIClass*, pINI, 0x4);

	pINI->Clear("Waypoints", nullptr);

	for (const auto& pair : ScenarioExt::Global()->Waypoints)
	{
		char buffer[32];
		sprintf_s(buffer, "%d", pair.first);
		pINI->WriteInteger("Waypoints", buffer, pair.second.X + 1000 * pair.second.Y, false);
	}
	
	return 0x68BF1F;
}

//DEFINE_HOOK(68BF30, ScenarioClass_Set_Default_Waypoint, A)
//{
//	Debug::Log(__FUNCTION__" called! Caller = %p\n", R->Stack32(0x0));
//	return 0;
//}

DEFINE_HOOK(68BF50, ScenarioClass_Set_Waypoint, 8)
{
	GET_STACK(int, nWaypoint, 0x4);
	GET_STACK(CellStruct, cell, 0x8);

	ScenarioExt::Global()->Waypoints[nWaypoint] = cell;

	return 0x68BF5F;
}

DEFINE_HOOK(68BF74, ScenarioClass_Get_Waypoint_Cell, 7)
{
	GET_STACK(int, nWaypoint, 0x4);

	R->ECX(&ScenarioExt::Global()->Waypoints[nWaypoint]);

	return 0x68BF7B;
}

DEFINE_HOOK(763610, Waypoint_To_String, 5)
{
	static char buffer[8] { '\0' };
	GET(int, nWaypoint, ECX);

	if (nWaypoint < 0)
		R->EAX("0");
	else if (nWaypoint == INT_MAX)
		R->EAX("FXSHRXX");
	else
	{
		++nWaypoint;
		int pos = 7;
		while (nWaypoint > 0)
		{
			--pos;
			char m = nWaypoint % 26;
			if (m == 0) m = 26;
			buffer[pos] = m + '@'; // '@' = 'A' - 1
			nWaypoint = (nWaypoint - m) / 26;
		}
		R->EAX(buffer + pos);
	}
	return 0x763621;
}

DEFINE_HOOK(763690, String_To_Waypoint, 7)
{
	GET(char*, pString, ECX);

	int n = 0;
	int len = strlen(pString);
	for (int i = len - 1, j = 1; i >= 0; i--, j *= 26)
	{
		int c = toupper(pString[i]);
		if (c < 'A' || c > 'Z') return 0;
		n += ((int)c - 64) * j;
	}
	R->EAX(n - 1);

	return 0x7636DF;
}

// This function is really strange, it returns empty string if a wp is valid, but why? - secsome
DEFINE_HOOK(68BF90, ScenarioClass_Get_Waypoint_As_String, 6)
{
	R->EAX(0x889F64);
	return 0x68BFD7;
}

DEFINE_HOOK(6883B7, ScenStruct_ScenStruct_1, 6)
{
	GET(int, nCount, ESI);

	// Waypoint 0-7 are used as multiplayer starting location
	for (int i = 0; i < 8; ++i)
	{
		if (!ScenarioClass::Instance->IsDefinedWaypoint(i))
			break;
		else
			++nCount;
	}

	R->ESI(nCount);

	return 0x6883EB;
}

DEFINE_HOOK(68843B, ScenStruct_ScenStruct_2, 6)
{
	REF_STACK(DynamicVectorClass<CellStruct>, waypoints, STACK_OFFS(0x40, 0x18));
	REF_STACK(CellStruct, buffer, STACK_OFFS(0x40, 0x20));
	GET(int, i, ESI);

	if (ScenarioClass::Instance->IsDefinedWaypoint(i))
	{
		waypoints.AddItem(ScenarioExt::Global()->Waypoints[i]);
		Debug::Log("Multiplayer start waypoint found at cell %d,%d\n", buffer.X, buffer.Y);
	}
	return 0x6884EF;
}

//DEFINE_HOOK(6D6070, Tactical_SetTacticalPosition, 5)
//{
//	GET_STACK(DWORD, dwCaller, 0x0);
//	GET_STACK(CoordStruct*, pCoord, 0x4);
//
//	CellStruct cell = CellClass::Coord2Cell(*pCoord);
//	Debug::Log(__FUNCTION__ "Caller = %p Cell = (%d,%d)\n", dwCaller, cell.X, cell.Y);
//
//	return 0;
//}

DEFINE_HOOK(684CB7, Scen_Waypoint_Call_1, 7)
{
	GET(int, nWaypoint, EAX);

	CellStruct cell = ScenarioExt::Global()->Waypoints[nWaypoint];

	R->EAX(*(int*)&cell);
	return 0x684CBE;
}

DEFINE_HOOK(6855E4, Scen_Waypoint_Call_2, 5)
{
	ScenarioExt::Global()->Waypoints.clear();

	return 0x6855FC;
}

DEFINE_HOOK(68AFE7, Scen_Waypoint_Call_3, 5)
{
	GET(int, nWaypoint, EDI);

	CellStruct cell = ScenarioExt::Global()->Waypoints[nWaypoint];

	R->EDX(*(int*)&cell);
	return 0x68AFEE;
}

DEFINE_HOOK(68AF45, Scen_Waypoint_Call_4, 6)
{
	int nStartingPoints = 0;
	for (int i = 0; i < 8; ++i)
	{
		if (ScenarioClass::Instance->IsDefinedWaypoint(i))
			++nStartingPoints;
		else
			break;
	}
	
	R->EDX(nStartingPoints);
	return 0x68AF86;
}