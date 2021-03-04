#pragma once

#include "../Phobos.h"
#include "Commands.h"

#include <BuildingTypeClass.h>
#include <HouseClass.h>
#include <MessageListClass.h>

class DummyCommandClass : public PhobosCommandClass
{
public:
	//CommandClass
	virtual const char* GetName() const override
	{
		return "Phobos Dummy";
	}

	virtual const wchar_t* GetUIName() const override
	{
		return L"Phobos Dummy";
	}

	virtual const wchar_t* GetUICategory() const override
	{
		return L"Development";
	}

	virtual const wchar_t* GetUIDescription() const override
	{
		return L"Dummy";
	}

	virtual void Execute(DWORD dwUnk) const override
	{
		Debug::Log("[Phobos] Dummy command runs.\n");
		MessageListClass::Instance->PrintMessage(L"[Phobos] Dummy command rums");
	}
};
