#pragma once
#include "SWButtonClass.h"
#include "SWColumnClass.h"
#include "ToggleSWButtonClass.h"
#include <Ext/Sidebar/Body.h>
#include <CommandClass.h>

class SWSidebarClass
{
public:
	bool AddColumn();
	bool RemoveColumn();

	void InitClear();

	bool AddButton(int superIdx);
	void SortButtons();

	int GetMaximumButtonCount();

	static bool IsEnabled();

	static SWSidebarClass Instance;

public:
	static void Allocate();
	static void Remove();

	static void Clear()
	{
		Allocate();
	}

public:
	std::vector<SWColumnClass*> Columns {};
	SWColumnClass* CurrentColumn { nullptr };
	SWButtonClass* CurrentButton { nullptr };
	ToggleSWButtonClass* ToggleButton { nullptr };

	static CommandClass* Commands[10];
};
