#pragma once
#include <MessageListClass.h>

#include <ObjectClass.h>
#include <FootClass.h>
#include <Ext/Techno/Body.h>

#include "Commands.h"
#include <Utilities/Debug.h>

class DummyCommandClass : public PhobosCommandClass
{
public:
	// CommandClass
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
		MessageListClass::Instance->PrintMessage(L"[Phobos] Dummy command runs");

		for (auto pObject : ObjectClass::CurrentObjects())
		{
			if (auto pTechno = abstract_cast<TechnoClass*>(pObject))
			{
				TechnoExt::AddExtraTint(pTechno, 150, 191, 98, 10, 1);
			}
		}
	}
};
