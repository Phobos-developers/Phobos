#pragma once
#include <GadgetClass.h>

class SWButtonClass : public GadgetClass
{
public:
	SWButtonClass() = default;
	SWButtonClass(int superIdx, int x, int y, int width, int height);

	~SWButtonClass();

	virtual bool Draw(bool forced) override;
	virtual void OnMouseEnter() override;
	virtual void OnMouseLeave() override;
	virtual bool Action(GadgetFlag flags, DWORD* pKey, KeyModifier modifier) override;

	void SetColumn(int column);
	bool LaunchSuper() const;

public:
	bool IsHovering { false };
	int ColumnIndex { -1 };
	int SuperIndex { -1 };
};
