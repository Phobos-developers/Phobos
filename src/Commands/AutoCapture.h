#pragma once

#include "Commands.h"

class AutoCaptureCommandClass : public CommandClass
{
public:
	bool TechBuildingsOnly;

	// CommandClass
	virtual const char* GetName() const override;
	virtual const wchar_t* GetUIName() const override;
	virtual const wchar_t* GetUICategory() const override;
	virtual const wchar_t* GetUIDescription() const override;
	virtual void Execute(WWKey eInput) const override;
};
