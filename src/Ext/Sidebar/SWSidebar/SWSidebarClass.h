#pragma once
#include "TacticalButtonClass.h"
#include "SWColumnClass.h"
#include <Ext/Sidebar/Body.h>

class SWSidebarClass
{
public:
	bool AddColumn();
	bool RemoveColumn();

	void Init_Clear();

	bool AddButton(int superIdx);
	void SortButtons();

	int GetMaximumButtonCount();

	static bool IsEnabled();

private:
	static std::unique_ptr<SWSidebarClass> Instance;

public:
	static void Allocate();
	static void Remove();

	static SWSidebarClass* Global()
	{
		return Instance.get();
	}

	static void Clear()
	{
		Allocate();
	}

public:
	std::vector<SWColumnClass*> Columns {};
	SWColumnClass* CurrentColumn { nullptr };
	TacticalButtonClass* CurrentButton { nullptr };
};
