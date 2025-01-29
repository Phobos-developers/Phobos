#pragma once

#include "Commands.h"
#include <New/Type/EventTypeClass.h>

// Select next idle harvester
class TriggerCustomHotkeyClass : public CommandClass
{
public:
	int NumeralSequence;
	char ButtonName[32];
	char UIName [32];
	char UIDescription [32];
	wchar_t UINameFallback[32];
	wchar_t UIDescriptionFallback[32];

	void SetNumeralSequence(int numeralSequence);

	// CommandClass
	virtual const char* GetName() const override;
	virtual const wchar_t* GetUIName() const override;
	virtual const wchar_t* GetUICategory() const override;
	virtual const wchar_t* GetUIDescription() const override;
	virtual void Execute(WWKey eInput) const override;
};
