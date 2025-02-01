#pragma once

#include "Commands.h"
#include <BuildingClass.h>
#include <TechnoClass.h>
#include "Ext/Techno/Body.h"
#include <unordered_map>

class AutoLoadCommandClass : public CommandClass
{
public:
	// CommandClass
	virtual const char *GetName() const override;
	virtual const wchar_t *GetUIName() const override;
	virtual const wchar_t *GetUICategory() const override;
	virtual const wchar_t *GetUIDescription() const override;
	virtual void Execute(WWKey eInput) const override;

	static int GetBuildingPassengerBudget(BuildingClass* pBuilding);
	static bool CanBeBuildingPassenger(TechnoClass* pPassenger);
	static std::set<TechnoClass*> SpreadPassengersToTransports(std::vector<TechnoClass*>& passengers, std::vector<std::pair<TechnoClass*, int>>& transports);
};
