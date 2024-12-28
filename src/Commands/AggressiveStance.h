#pragma once

#include "Commands.h"

// Select next idle harvester
class AggressiveStanceClass : public CommandClass
{
public:
	// CommandClass
	virtual const char* GetName() const override;
	virtual const wchar_t* GetUIName() const override;
	virtual const wchar_t* GetUICategory() const override;
	virtual const wchar_t* GetUIDescription() const override;
	virtual const wchar_t* GetToggleOnPopupMessage() const;
	virtual const wchar_t* GetToggleOffPopupMessage() const;
	virtual void Execute(WWKey eInput) const override;
};
