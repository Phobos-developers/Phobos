#pragma once
#include "TacticalButtonClass.h"
#include "SWColumnClass.h"
#include <Ext/Sidebar/Body.h>

class SWSidebarClass
{
public:
	bool AddColumn();
	bool RemoveColumn();

	void InitClear();

	bool AddButton(int superIdx);
	void SortButtons();

	void RecordHotkey(int buttonIndex, int key);
	int GetMaximumButtonCount();

	static bool IsEnabled();

private:
	static std::unique_ptr<SWSidebarClass> Instance;
	static PhobosMap<int, const wchar_t*> KeyboardCodeTextMap;

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
	ToggleSWButtonClass* ToggleButton { nullptr };

	std::wstring KeyCodeText[10] {};
	int KeyCodeData[10] {};
};
