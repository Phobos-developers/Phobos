#pragma once
#include <ControlClass.h>

class ToggleSWButtonClass : public ControlClass
{
public:
	ToggleSWButtonClass() = default;
	ToggleSWButtonClass(unsigned int id, int x, int y, int width, int height);

	~ToggleSWButtonClass() = default;

	virtual bool Draw(bool forced) override;
	virtual void OnMouseEnter() override;
	virtual void OnMouseLeave() override;
	virtual bool Action(GadgetFlag fags, DWORD* pKey, KeyModifier modifier) override;

	void UpdatePosition();

	static bool SwitchSidebar();

public:
	bool IsHovering { false };
	bool IsPressed { false };
};
