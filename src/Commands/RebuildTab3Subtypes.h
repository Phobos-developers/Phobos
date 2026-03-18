#pragma once

#include "Commands.h"

// Dedicated rebuild commands for tab 3 subtypes.
// RebuildVehicle: re-queues the last non-naval UnitType.
// RebuildAircraft: re-queues the last AircraftType.
// RebuildNaval: re-queues the last naval UnitType.

class RebuildVehicleCommandClass : public CommandClass
{
public:
	virtual const char* GetName() const override;
	virtual const wchar_t* GetUIName() const override;
	virtual const wchar_t* GetUICategory() const override;
	virtual const wchar_t* GetUIDescription() const override;
	virtual void Execute(WWKey eInput) const override;
};

class RebuildAircraftCommandClass : public CommandClass
{
public:
	virtual const char* GetName() const override;
	virtual const wchar_t* GetUIName() const override;
	virtual const wchar_t* GetUICategory() const override;
	virtual const wchar_t* GetUIDescription() const override;
	virtual void Execute(WWKey eInput) const override;
};

class RebuildNavalCommandClass : public CommandClass
{
public:
	virtual const char* GetName() const override;
	virtual const wchar_t* GetUIName() const override;
	virtual const wchar_t* GetUICategory() const override;
	virtual const wchar_t* GetUIDescription() const override;
	virtual void Execute(WWKey eInput) const override;
};
