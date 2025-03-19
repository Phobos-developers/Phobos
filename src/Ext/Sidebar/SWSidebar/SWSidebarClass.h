#pragma once
#include "SWButtonClass.h"
#include "SWColumnClass.h"
#include "ToggleSWButtonClass.h"
#include <Ext/Scenario/Body.h>
#include <CommandClass.h>

class SWSidebarClass
{
public:
	bool AddColumn();
	bool RemoveColumn();

	void InitClear();
	void InitIO();

	bool AddButton(int superIdx);
	void SortButtons();

	int GetMaximumButtonCount();

	static bool IsEnabled();
	static void RecheckCameo();

	static SWSidebarClass Instance;

public:
	std::vector<SWColumnClass*> Columns {};
	SWColumnClass* CurrentColumn { nullptr };
	SWButtonClass* CurrentButton { nullptr };
	ToggleSWButtonClass* ToggleButton { nullptr };
	bool DisableEntry { false };

	static CommandClass* Commands[10];
};
