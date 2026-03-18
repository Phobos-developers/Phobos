#pragma once

#include "Commands.h"

// Re-queue the last produced item from a specific production tab.
// TabIndex: 0 = Power/Infrastructure, 1 = Defense/Combat, 2 = Infantry, 3 = Vehicles/Aircraft
template<int TabIndex>
class BuildLastOfTabCommandClass : public CommandClass
{
public:
	virtual const char* GetName() const override;
	virtual const wchar_t* GetUIName() const override;
	virtual const wchar_t* GetUICategory() const override;
	virtual const wchar_t* GetUIDescription() const override;
	virtual void Execute(WWKey eInput) const override;
};
