#pragma once

#include "Commands.h"

class DistributionModeSpreadCommandClass : public CommandClass
{
public:
	virtual const char* GetName() const override;
	virtual const wchar_t* GetUIName() const override;
	virtual const wchar_t* GetUICategory() const override;
	virtual const wchar_t* GetUIDescription() const override;
	virtual void Execute(WWKey eInput) const override;
};

class DistributionModeFilterCommandClass : public CommandClass
{
public:
	virtual const char* GetName() const override;
	virtual const wchar_t* GetUIName() const override;
	virtual const wchar_t* GetUICategory() const override;
	virtual const wchar_t* GetUIDescription() const override;
	virtual void Execute(WWKey eInput) const override;
};

class DistributionModeHoldDownCommandClass : public CommandClass
{
public:
	static bool Enabled;
	static bool OnMessageShowed;
	static bool OffMessageShowed;
	static int ShowTime;

	virtual const char* GetName() const override;
	virtual const wchar_t* GetUIName() const override;
	virtual const wchar_t* GetUICategory() const override;
	virtual const wchar_t* GetUIDescription() const override;
	virtual bool ExtraTriggerCondition(WWKey eInput) const override;
	virtual void Execute(WWKey eInput) const override;

	static void DistributionSpreadModeExpand();
	static void DistributionSpreadModeReduce();
};
