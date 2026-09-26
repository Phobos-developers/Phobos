#pragma once

#include "Commands.h"

// Cycles through the objects that were selected when the cycle started, selecting
// one of them at a time and wrapping around at the end of the list.
// Modeled after vanilla's HealthNav (H) / VeterancyNav (V) commands, which use the
// NavCycleMode global to tell an ongoing cycle apart from a fresh key press.
class CycleSelectionCommandClass : public CommandClass
{
public:
	// CommandClass
	virtual const char* GetName() const override;
	virtual const wchar_t* GetUIName() const override;
	virtual const wchar_t* GetUICategory() const override;
	virtual const wchar_t* GetUIDescription() const override;
	virtual void Execute(WWKey eInput) const override;
};
