#pragma once
#include <GadgetClass.h>

class ToggleSWButtonClass : public GadgetClass
{
public:
	ToggleSWButtonClass() = default;
	ToggleSWButtonClass(int x, int y, int width, int height);

	~ToggleSWButtonClass() = default;

	virtual bool Draw(bool forced) override;
	virtual void OnMouseEnter() override;
	virtual void OnMouseLeave() override;
	virtual bool Action(GadgetFlag flags, DWORD* pKey, KeyModifier modifier) override;

	void UpdatePosition();

	static bool SwitchSidebar();

public:
	bool IsHovering { false };
	bool IsPressed { false };
};
