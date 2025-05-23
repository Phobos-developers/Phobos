#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/TemplateDef.h>

class InsigniaTypeClass final : public Enumerable<InsigniaTypeClass>
{
public:
	Promotable<SHPStruct*> Insignia;
	Promotable<int> InsigniaFrame;

	InsigniaTypeClass(const char* const pTitle) : Enumerable<InsigniaTypeClass>(pTitle)
		, Insignia { }
		, InsigniaFrame { -1 }
	{ }

	void LoadFromINI(CCINIClass* pINI);
	// No need to save and load as it's only for parsing
};
