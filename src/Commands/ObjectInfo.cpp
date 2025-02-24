#include "ObjectInfo.h"

#include <Utilities/GeneralUtils.h>
#include <BuildingClass.h>
#include <InfantryClass.h>
#include <FootClass.h>
#include <TeamClass.h>
#include <HouseClass.h>
#include <ScriptClass.h>
#include <AITriggerTypeClass.h>
#include <Helpers/Enumerators.h>
#include <CRT.h>

#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>

const char* ObjectInfoCommandClass::GetName() const
{
	return "Dump ObjectInfo";
}

const wchar_t* ObjectInfoCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DUMP_OBJECT_INFO", L"Dump Object Info");
}

const wchar_t* ObjectInfoCommandClass::GetUICategory() const
{
	return CATEGORY_DEVELOPMENT;
}

const wchar_t* ObjectInfoCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DUMP_OBJECT_INFO_DESC", L"Dump ObjectInfo to log file and display it.");
}

void ObjectInfoCommandClass::Execute(WWKey eInput) const
{
	char buffer[0x800] = { 0 };

	auto append = [&buffer](const char* pFormat, ...)
	{
		va_list args;
		va_start(args, pFormat);
		vsprintf_s(Phobos::readBuffer, pFormat, args);
		va_end(args);
		strcat_s(buffer, Phobos::readBuffer);
	};

	auto display = [&buffer]()
	{
		memset(Phobos::wideBuffer, 0, sizeof Phobos::wideBuffer);
		CRT::mbstowcs(Phobos::wideBuffer, buffer, strlen(buffer));
		MessageListClass::Instance->PrintMessage(Phobos::wideBuffer, 600, 5, true);
		Debug::Log("%s\n", buffer);
		buffer[0] = 0;
	};

	auto getTargetInfo = [](TechnoClass* pCurrent, AbstractClass* pTarget, int& distance, const char*& ID, CellStruct& mapCoords)
	{
		if (auto const pObject = abstract_cast<ObjectClass*>(pTarget))
		{
			mapCoords = pObject->GetMapCoords();
			ID = pObject->GetType()->get_ID();
		}
		else if (auto const pCell = abstract_cast<CellClass*>(pTarget))
		{
			mapCoords = pCell->MapCoords;
			ID = "Cell";
		}

		distance = pCurrent->DistanceFrom(pTarget) / Unsorted::LeptonsPerCell;
	};

	auto printFoots = [&append, &display, &getTargetInfo](FootClass* pFoot)
	{
		append("[Phobos] Dump ObjectInfo runs.\n");
		auto pType = pFoot->GetTechnoType();
		append("ID = %s, ", pType->ID);
		append("Owner = %s (%s), ", pFoot->Owner->get_ID(), pFoot->Owner->PlainName);
		append("Location = (%d, %d), ", pFoot->GetMapCoords().X, pFoot->GetMapCoords().Y);
		append("Mission = %d (%s), Status = %d\n", pFoot->CurrentMission, MissionControlClass::FindName(pFoot->CurrentMission), pFoot->MissionStatus);

		if (pFoot->BelongsToATeam())
		{
			auto pTeam = pFoot->Team;

			auto pTeamType = pFoot->Team->Type;
			bool found = false;
			for (int i = 0; i < AITriggerTypeClass::Array->Count && !found; i++)
			{
				auto pTriggerTeam1Type = AITriggerTypeClass::Array->GetItem(i)->Team1;
				auto pTriggerTeam2Type = AITriggerTypeClass::Array->GetItem(i)->Team2;

				if (pTeamType && ((pTriggerTeam1Type && pTriggerTeam1Type == pTeamType) || (pTriggerTeam2Type && pTriggerTeam2Type == pTeamType)))
				{
					found = true;
					auto pTriggerType = AITriggerTypeClass::Array->GetItem(i);
					append("Trigger ID = %s, weights [Current, Min, Max]: %f, %f, %f", pTriggerType->ID, pTriggerType->Weight_Current, pTriggerType->Weight_Minimum, pTriggerType->Weight_Maximum);
				}
			}
			display();

			append("Team ID = %s, Script ID = %s, Taskforce ID = %s",
				pTeam->Type->ID, pTeam->CurrentScript->Type->get_ID(), pTeam->Type->TaskForce->ID);
			display();

			if (pTeam->CurrentScript->CurrentMission >= 0)
				append("Current Script [Line = Action, Argument]: %d = %d,%d", pTeam->CurrentScript->CurrentMission, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Action, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument);
			else
				append("Current Script [Line = Action, Argument]: %d", pTeam->CurrentScript->CurrentMission);

			display();
		}

		if (pFoot->Passengers.NumPassengers > 0)
		{
			FootClass* pCurrent = pFoot->Passengers.FirstPassenger;
			append("%d Passengers: %s", pFoot->Passengers.NumPassengers, pCurrent->GetTechnoType()->ID);
			for (pCurrent = abstract_cast<FootClass*>(pCurrent->NextObject); pCurrent; pCurrent = abstract_cast<FootClass*>(pCurrent->NextObject))
				append(", %s", pCurrent->GetTechnoType()->ID);
			append("\n");
		}

		if (pFoot->Target)
		{
			auto mapCoords = CellStruct::Empty;
			auto ID = "N/A";
			int distance = 0;
			getTargetInfo(pFoot, pFoot->Target, distance, ID, mapCoords);
			append("Target = %s, Distance = %d, Location = (%d, %d)\n", ID, distance, mapCoords.X, mapCoords.Y);
		}

		if (pFoot->Destination)
		{
			auto mapCoords = CellStruct::Empty;
			auto ID = "N/A";
			int distance = 0;
			getTargetInfo(pFoot, pFoot->Destination, distance, ID, mapCoords);
			append("Destination = %s, Distance = %d, Location = (%d, %d)\n", ID, distance, mapCoords.X, mapCoords.Y);
		}

		append("Current HP = (%d / %d)", pFoot->Health, pType->Strength);

		auto pTechnoExt = TechnoExt::ExtMap.Find(pFoot);
		auto pShieldData = pTechnoExt->Shield.get();

		if (pTechnoExt->CurrentShieldType && pShieldData)
			append(", Current Shield HP = (%d / %d)", pShieldData->GetHP(), pTechnoExt->CurrentShieldType->Strength);

		if (pType->Ammo > 0)
			append(", Ammo = (%d / %d)", pFoot->Ammo, pType->Ammo);

		append("\n");
		display();
	};

	auto printBuilding = [&append, &display, &getTargetInfo](BuildingClass* pBuilding)
	{
		append("[Phobos] Dump ObjectInfo runs.\n");
		auto pType = pBuilding->Type;
		append("ID = %s, ", pType->ID);
		append("Owner = %s (%s), ", pBuilding->Owner->get_ID(), pBuilding->Owner->PlainName);
		append("Location = (%d, %d), ", pBuilding->GetMapCoords().X, pBuilding->GetMapCoords().Y);
		append("Mission = %d (%s), Status = %d\n", pBuilding->CurrentMission, MissionControlClass::FindName(pBuilding->CurrentMission), pBuilding->MissionStatus);

		if (pBuilding->Factory && pBuilding->Factory->Object)
		{
			append("Production: %s (%d%%)\n", pBuilding->Factory->Object->GetTechnoType()->ID, (pBuilding->Factory->GetProgress() * 100 / 54));
		}

		if (pBuilding->Type->Refinery || pBuilding->Type->ResourceGatherer)
		{
			append("Money: %d\n", pBuilding->Owner->Available_Money());
		}

		if (pBuilding->Occupants.Count > 0)
		{
			append("Occupants: %s", pBuilding->Occupants.GetItem(0)->Type->ID);
			for (int i = 1; i < pBuilding->Occupants.Count; i++)
			{
				append(", %s", pBuilding->Occupants.GetItem(i)->Type->ID);
			}
			append("\n");
		}

		if (pBuilding->Type->Upgrades)
		{
			append("Upgrades (%d/%d): ", pBuilding->UpgradeLevel, pBuilding->Type->Upgrades);
			for (int i = 0; i < 3; i++)
			{
				if (i != 0)
					append(", ");

				append("Slot %d = %s", i + 1, pBuilding->Upgrades[i] ? pBuilding->Upgrades[i]->get_ID() : "<none>");
			}
			append("\n");
		}

		if (pBuilding->Type->Ammo > 0)
			append("Ammo = (%d / %d)\n", pBuilding->Ammo, pBuilding->Type->Ammo);

		if (pBuilding->Target)
		{
			auto mapCoords = CellStruct::Empty;
			auto ID = "N/A";
			int distance = 0;
			getTargetInfo(pBuilding, pBuilding->Target, distance, ID, mapCoords);
			append("Target = %s, Distance = %d, Location = (%d, %d)\n", ID, distance, mapCoords.X, mapCoords.Y);
		}

		append("Current HP = (%d / %d)\n", pBuilding->Health, pBuilding->Type->Strength);

		auto pTechnoExt = TechnoExt::ExtMap.Find(pBuilding);
		auto pShieldData = pTechnoExt->Shield.get();

		if (pTechnoExt->CurrentShieldType && pShieldData)
			append("Current Shield HP = (%d / %d)\n", pShieldData->GetHP(), pTechnoExt->CurrentShieldType->Strength);

		display();
	};

	bool dumped = false;
	auto dumpInfo = [&printFoots, &printBuilding, &append, &display, &dumped](ObjectClass* pObject)
	{
		switch (pObject->WhatAmI())
		{
		case AbstractType::Infantry:
		case AbstractType::Unit:
		case AbstractType::Aircraft:
			printFoots(static_cast<FootClass*>(pObject));
			break;
		case AbstractType::Building:
			printBuilding(static_cast<BuildingClass*>(pObject));
			break;
		default:
			append("INVALID ITEM!");
			display();
			break;
		}
		dumped = true;
	};

	for (auto pTechno : *TechnoClass::Array)
	{
		if (dumped) break;
		if (pTechno->IsMouseHovering)
			dumpInfo(pTechno);
	}

	if (!dumped)
	{
		if (ObjectClass::CurrentObjects->Count > 0)
		{
			if (ObjectClass::CurrentObjects->Count != 1)
				MessageListClass::Instance->PrintMessage(L"This command will only dump one of these selected object", 600, 5, true);

			dumpInfo(ObjectClass::CurrentObjects->GetItem(ObjectClass::CurrentObjects->Count - 1));
		}
	}
}
