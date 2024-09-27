#pragma once
#include <ControlClass.h>

class TacticalButtonClass : public ControlClass
{
public:
	TacticalButtonClass() = default;
	TacticalButtonClass(unsigned int id, int superIdx, int x, int y, int width, int height);

	~TacticalButtonClass();

	virtual bool Draw(bool forced) override;
	virtual void OnMouseEnter() override;
	virtual void OnMouseLeave() override;
	virtual bool Action(GadgetFlag fags, DWORD* pKey, KeyModifier modifier) override;

	void SetColumn(int column);
	bool LaunchSuper() const;

public:
	bool IsHovering { false };
	bool IsPressed { false };
	int ColumnIndex { -1 };
	int SuperIndex { -1 };
};
