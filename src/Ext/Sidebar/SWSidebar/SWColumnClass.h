#pragma once
#include "SWButtonClass.h"
#include <GadgetClass.h>

#include <vector>

class SWColumnClass : public GadgetClass
{
public:
	SWColumnClass() = default;
	SWColumnClass(int maxButtons, int x, int y, int width, int height);

	~SWColumnClass();

	virtual bool Draw(bool forced) override;
	virtual void OnMouseEnter() override;
	virtual void OnMouseLeave() override;
	virtual bool Clicked(DWORD* pKey, GadgetFlag flags, int x, int y, KeyModifier modifier) override;

	bool AddButton(int superIdx);
	bool RemoveButton(int superIdx);
	void ClearButtons(bool remove = true);

	void SetHeight(int height);

	std::vector<SWButtonClass*> Buttons {};
	int MaxButtons { 0 };
};
