#pragma once

#include "Commands.h"

class DistributionMode1CommandClass : public CommandClass
{
public:
	static int Mode;

	virtual const char* GetName() const override;
	virtual const wchar_t* GetUIName() const override;
	virtual const wchar_t* GetUICategory() const override;
	virtual const wchar_t* GetUIDescription() const override;
	virtual void Execute(WWKey eInput) const override;
};

class DistributionMode2CommandClass : public CommandClass
{
public:
	static int Mode;

	virtual const char* GetName() const override;
	virtual const wchar_t* GetUIName() const override;
	virtual const wchar_t* GetUICategory() const override;
	virtual const wchar_t* GetUIDescription() const override;
	virtual void Execute(WWKey eInput) const override;
};

class DistributionMode3CommandClass : public CommandClass
{
public:
	static bool Enabled;
	static int ShowTime;

	virtual const char* GetName() const override;
	virtual const wchar_t* GetUIName() const override;
	virtual const wchar_t* GetUICategory() const override;
	virtual const wchar_t* GetUIDescription() const override;
	virtual bool ExtraTriggerCondition(WWKey eInput) const override;
	virtual void Execute(WWKey eInput) const override;
};
