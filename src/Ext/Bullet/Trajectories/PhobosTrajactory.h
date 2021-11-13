#pragma once

#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Savegame.h>

class PhobosTrajactoryType
{
public:
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) = 0;
	virtual bool Save(PhobosStreamWriter& Stm) const = 0;

	virtual void LoadFromINIFile(CCINIClass* const pINI) = 0;
	virtual void LoadFromINIFile(INI_EX& const exINI) = 0;
};

class PhobosTrajactory
{
public:
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) = 0;
	virtual bool Save(PhobosStreamWriter& Stm) const = 0;

	virtual void OnUnlimbo() = 0;
	virtual void OnAI() = 0;
};