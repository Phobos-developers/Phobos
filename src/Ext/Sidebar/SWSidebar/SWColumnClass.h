#pragma once
#include "SWButtonClass.h"
#include <ControlClass.h>

#include <vector>

class SWColumnClass : public ControlClass
{
public:
	SWColumnClass() = default;
	SWColumnClass(unsigned int id, int maxButtons, int x, int y, int width, int height);

	~SWColumnClass() = default;

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
