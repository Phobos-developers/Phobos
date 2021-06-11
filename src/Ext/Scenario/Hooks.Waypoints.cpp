#include <Helpers/Macro.h>

#include "Body.h"

#include <MapClass.h>

#include <string>

DEFINE_HOOK(68BCC0, ScenarioClass_Get_Waypoint_Location, B)
{
	// GET_STACK(CellStruct*, pCell, 0x4); in fact
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
	GET_STACK(int, nWaypoint, STACK_OFFS(0x10, 0x8));

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
	GET(int, nWaypoint, EAX);
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
		if (sscanf_s(pName, "%d", &id) != 1)
			Debug::FatalErrorAndExit("[Fatal Error] Failed to parse waypoint %s.\n", pName);
		int nCoord = pINI->ReadInteger("Waypoints", pName, 0);
		
		if (nCoord)
		{
			buffer.X = static_cast<short>(nCoord % 1000);
			buffer.Y = static_cast<short>(nCoord / 1000);
			if (auto pCell = MapClass::Instance->TryGetCellAt(buffer))
				pCell->Flags |= cf_IsWaypoint;
			else
				Debug::FatalErrorAndExit("[Fatal Error] Waypoint %d : [%d, %d] out of the map!\n", id, buffer.X, buffer.Y);
			ScenarioExt::Global()->Waypoints[id] = buffer;
		}
		else
			Debug::FatalErrorAndExit("[Fatal Error] Invalid waypoint %d!\n", id);
	}

	return 0x68BE8C;
}

DEFINE_HOOK(68BE90, ScenarioClass_Write_Waypoints, 5)
{
	GET_STACK(INIClass*, pINI, 0x4);

	pINI->Clear("Waypoints", nullptr);

	for (size_t i = 0; i < ScenarioExt::Global()->Waypoints.size(); ++i)
	{
		const auto& cell = ScenarioExt::Global()->Waypoints[i];
		char buffer[32];
		sprintf_s(buffer, "%d", i);
		pINI->WriteInteger("Waypoints", buffer, cell.X + 1000 * cell.Y, false);
	}

	return 0x68BF1F;
}

DEFINE_HOOK(68BF30, ScenarioClass_Set_Default_Waypoint, A)
{
	Debug::Log(__FUNCTION__" called! Caller = %p\n", R->Stack32(0x0));
	return 0;
}

DEFINE_HOOK(68BF50, ScenarioClass_Set_Waypoint, 8)
{
	GET_STACK(int, nWaypoint, 0x4);
	GET_STACK(CellStruct, cell, 0x8);

	ScenarioExt::Global()->Waypoints[nWaypoint] = cell;

	return 0x68BF5F;
}

DEFINE_HOOK(68BF70, ScenarioClass_Get_Waypoint_Cell, B)
{
	Debug::Log(__FUNCTION__" called! Caller = %p\n", R->Stack32(0x0));
	return 0;
}

DEFINE_HOOK(763610, Waypoint_To_String, 5)
{
	static std::string buffer;
	GET(int, nWaypoint, ECX);
	buffer.clear();

	if (nWaypoint == -1)
		buffer = "0";
	else
	{
		++nWaypoint;
		while (nWaypoint > 0)
		{
			int m = nWaypoint % 26;
			if (m == 0) m = 26;
			buffer = (char)(m + 64) + buffer;
			nWaypoint = (nWaypoint - m) / 26;
		}
	}

	R->EAX(buffer.c_str());
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

// shouldn't be invalid, I think - secsome
DEFINE_HOOK(68BF90, ScenarioClass_Get_Waypoint_As_String, 6)
{
	R->EAX(0x889F64);
	return 0x68BFD7;
}