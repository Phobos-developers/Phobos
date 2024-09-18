#pragma once
#include "Body.h"
#include <ToggleClass.h>

#include <Utilities/Macro.h>
#include <Ext/SWType/Body.h>

class TacticalButtonClass : public ToggleClass
{
public:
	TacticalButtonClass() = default;
	TacticalButtonClass(unsigned int id, int superIdx, int x, int y, int width, int height);

	~TacticalButtonClass();

	virtual bool Draw(bool forced) override;
	virtual void OnMouseEnter() override;
	virtual void OnMouseLeave() override;
	virtual bool Action(GadgetFlag fags, DWORD* pKey, KeyModifier modifier) override;

	bool LaunchSuper(int superIdx);

public:
	static bool AddButton(int superIdx);
	static bool RemoveButton(int superIdx);
	static void ClearButtons();
	static void SortButtons();

public:
	static std::vector<TacticalButtonClass*> Buttons;
	static bool Initialized;
	static TacticalButtonClass* CurrentButton;

	bool IsHovering { false };
	int SuperIndex { -1 };
};
