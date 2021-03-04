#pragma once

#include "../Phobos.h"
#include "Commands.h"

#include <HouseClass.h>
#include <MessageListClass.h>
#include <AITriggerTypeClass.h>
#include <TeamClass.h>

class TeamsCommandClass : public PhobosCommandClass
{
public:
	//CommandClass
	virtual const char* GetName() const override
	{
		return "Dump Teams";
	}

	virtual const wchar_t* GetUIName() const override
	{
		return L"Dump Teams";
	}

	virtual const wchar_t* GetUICategory() const override
	{
		return L"Development";
	}

	virtual const wchar_t* GetUIDescription() const override
	{
		return L"Dump Teams to log file.";
	}

	virtual void Execute(DWORD dwUnk) const override
	{
		Debug::Log("[Phobos] DumpTeams command runs.\n");
		MessageListClass::Instance->PrintMessage(L"[Phobos] DumpTeams command runs.");
		
		
	}
};
