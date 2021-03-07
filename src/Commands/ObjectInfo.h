#pragma once

#include "../Phobos.h"
#include "Commands.h"

#include <TechnoTypeClass.h>
#include <MessageListClass.h>
#include <AITriggerTypeClass.h>
#include <TeamClass.h>
#include <WWMouseClass.h>

#include "../Ext/TechnoType/Body.h"


// #53 New debug feature for AI scripts
class ObjectInfoCommandClass : public PhobosCommandClass
{
public:
	
	virtual const char* GetName() const override
	{
		return "Dump ObjectInfo";
	}

	virtual const wchar_t* GetUIName() const override
	{
		return L"Dump ObjectInfo";
	}

	virtual const wchar_t* GetUICategory() const override
	{
		return L"Development";
	}

	virtual const wchar_t* GetUIDescription() const override
	{
		return L"Dump ObjectInfo to log file and display it.";
	}

	// Just place these two functions here... for now.
	// Will be moved to other places later.
	static size_t __cdecl mbstowcs(wchar_t* lpWideCharStr, const char* lpMultiByteStr, size_t a3)
	{
		JMP_STD(0x7CC2AC);
	}
	static int __cdecl vsprintf(char* a1, const char* a2, va_list a3)
	{
		JMP_STD(0x7CB7BA);
	}

	virtual void Execute(DWORD dwUnk) const override
	{
		if (this->CheckDebugDeactivated())
			return;

		char buffer[0x800] = { 0 };

		auto append = [&buffer](const char* pFormat, ...)
		{
			va_list args;
			va_start(args, pFormat);
			vsprintf(Phobos::readBuffer, pFormat, args);
			va_end(args);
			strcat_s(buffer, Phobos::readBuffer);
		};

		auto getMissionName = [](int mID) {
			switch (mID) {
			case -1:
				return "None";
			case 0:
				return "Sleep";
			case 1:
				return "Attack";
			case 2:
				return "Move";
			case 3:
				return "QMove";
			case 4:
				return "Retreat";
			case 5:
				return "Guard";
			case 6:
				return "Sticky";
			case 7:
				return "Enter";
			case 8:
				return "Capture";
			case 9:
				return "Eaten";
			case 10:
				return "Harvest";
			case 11:
				return "Area_Guard";
			case 12:
				return "Return";
			case 13:
				return "Stop";
			case 14:
				return "Ambush";
			case 15:
				return "Hunt";
			case 16:
				return "Unload";
			case 17:
				return "Sabotage";
			case 18:
				return "Construction";
			case 19:
				return "Selling";
			case 20:
				return "Repair";
			case 21:
				return "Rescue";
			case 22:
				return "Missile";
			case 23:
				return "Harmless";
			case 24:
				return "Open";
			case 25:
				return "Patrol";
			case 26:
				return "ParadropApproach";
			case 27:
				return "ParadropOverfly";
			case 28:
				return "Wait";
			case 29:
				return "AttackMove";
			case 30:
				return "SpyplaneApproach";
			case 31:
				return "SpyplaneOverfly";
			default:
				return "INVALID_MISSION";
			}
		};

		auto display = [&buffer]()
		{
			memset(Phobos::wideBuffer, 0, sizeof Phobos::wideBuffer);
			mbstowcs(Phobos::wideBuffer, buffer, strlen(buffer));
			MessageListClass::Instance->PrintMessage(Phobos::wideBuffer, 600, 5, true);
			Debug::Log("%s\n", buffer);
			buffer[0] = 0;
		};

		auto printFoots = [&append, &display, &getMissionName](FootClass* pFoot) {
			append("[Phobos] Dump ObjectInfo runs.\n");
			auto pType = pFoot->GetTechnoType();
			append("ID = %s, ", pType->ID);
			append("Owner = %s (%s), ", pFoot->Owner->get_ID(), pFoot->Owner->PlainName);
			append("Location = (%d, %d), ", pFoot->GetMapCoords().X, pFoot->GetMapCoords().Y);
			append("Current Mission = %d (%s)\n", pFoot->CurrentMission, getMissionName((int)pFoot->CurrentMission));

			if (pFoot->BelongsToATeam())
			{
				auto pTeam = pFoot->Team;
				append("Team ID = %s, Script ID = %s, Taskforce ID = %s",
					pTeam->Type->ID, pTeam->CurrentScript->Type->get_ID(), pTeam->Type->TaskForce->ID);
				display();
				if (pTeam->CurrentScript->idxCurrentLine >= 0)
					append("Current Script [Line = Action, Argument]: %d = %d,%d", pTeam->CurrentScript->idxCurrentLine, pTeam->CurrentScript->Type->ScriptActions->Action, pTeam->CurrentScript->Type->ScriptActions->Argument);
				else
					append("Current Script [Line = Action, Argument]: %d", pTeam->CurrentScript->idxCurrentLine);
				display();
			}

			append("Current HP = (%d / %d)\n", pFoot->Health, pType->Strength);
			display();
		};

		auto printBuilding = [&append, &display](BuildingClass* pBuilding) {
			append("[Phobos] Dump ObjectInfo runs.\n");
			auto pType = pBuilding->GetTechnoType();
			append("ID = %s, ", pType->ID);
			append("Owner = %s (%s), ", pBuilding->Owner->get_ID(), pBuilding->Owner->PlainName);
			append("Location = (%d, %d)\n", pBuilding->GetMapCoords().X, pBuilding->GetMapCoords().Y);
			append("Current HP = (%d / %d)\n", pBuilding->Health, pBuilding->Type->Strength);
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
				printFoots(abstract_cast<FootClass*>(pObject));
				break;
			case AbstractType::Building:
				printBuilding(abstract_cast<BuildingClass*>(pObject));
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
			if (ObjectClass::CurrentObjects->Count > 0)
			{
				if (ObjectClass::CurrentObjects->Count != 1)
					MessageListClass::Instance->PrintMessage(L"This command will only dump one of these selected object", 600, 5, true);
				dumpInfo(ObjectClass::CurrentObjects->GetItem(ObjectClass::CurrentObjects->Count - 1));
			}
	}
};