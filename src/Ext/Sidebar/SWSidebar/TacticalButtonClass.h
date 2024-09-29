#pragma once
#include <ControlClass.h>

class TacticalButtonClass : public ControlClass
{
public:
	TacticalButtonClass() = default;
	TacticalButtonClass(unsigned int id, int superIdx, int x, int y, int width, int height);

	~TacticalButtonClass() = default;

	virtual bool Draw(bool forced) override;
	virtual void OnMouseEnter() override;
	virtual void OnMouseLeave() override;
	virtual bool Action(GadgetFlag fags, DWORD* pKey, KeyModifier modifier) override;

	void SetColumn(int column);
	bool LaunchSuper() const;

public:
	static constexpr int StartID = 2200;

	bool IsHovering { false };
	int ColumnIndex { -1 };
	int SuperIndex { -1 };
};

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
