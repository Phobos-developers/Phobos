#pragma once
#include "TacticalButtonClass.h"
#include "SWColumnClass.h"
#include <Ext/Sidebar/Body.h>

class SWSidebarClass
{
public:
	bool AddColumn();
	bool RemoveColumn();
	void ClearColumns();

	bool AddButton(int superIdx);
	void SortButtons();

	int GetMaximumButtonCount();

	static bool IsEnabled();

	static SWSidebarClass Instance;

	bool Initialized { false };
	std::vector<SWColumnClass*> Columns {};
	SWColumnClass* CurrentColumn { nullptr };
	TacticalButtonClass* CurrentButton { nullptr };
};
