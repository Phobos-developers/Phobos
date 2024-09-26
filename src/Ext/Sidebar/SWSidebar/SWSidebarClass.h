#pragma once
#include "TacticalButtonClass.h"

#include <vector>

class SWSidebarClass
{
public:
	bool AddButton(int superIdx);
	bool RemoveButton(int superIdx);
	void ClearButtons();
	void SortButtons();

	static SWSidebarClass Instance;

	std::vector<TacticalButtonClass*> Buttons {};
	bool Initialized { false };
	TacticalButtonClass* CurrentButton { nullptr };
};
