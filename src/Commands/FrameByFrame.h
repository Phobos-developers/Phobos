#pragma once

#include "Commands.h"

class FrameByFrameCommandClass : public CommandClass
{
public:
	static size_t FrameStepCount;
	static bool FrameStep;

	virtual const char* GetName() const override;
	virtual const wchar_t* GetUIName() const override;
	virtual const wchar_t* GetUICategory() const override;
	virtual const wchar_t* GetUIDescription() const override;
	virtual void Execute(WWKey eInput) const override;

	static bool FrameStepMainLoop();
};
