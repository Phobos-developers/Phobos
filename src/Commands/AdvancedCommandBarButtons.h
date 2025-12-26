#pragma once

#include <stdlib.h>
#include <vector>

#include <CommandClass.h>
#include <StringTable.h>
#include <MessageListClass.h>
#include <Phobos.h>
#include <Utilities/Debug.h>


class AdvancedCommandBarButton
{
public:
	static std::vector<std::unique_ptr<AdvancedCommandBarButton>> Array;

	static constexpr int MaxButtonCount = 25;
	static constexpr int InUseButtonCount = 11;
	static constexpr int UnusedButtonCount = 1;
	static constexpr int OldButtonCount = InUseButtonCount + UnusedButtonCount;

	static int TotalButtonCount;

	static void RegisterButtons();
	static ShapeButtonClass* GetShapeButton(const char* name);

	virtual const char* GetName() const R0;
	virtual const char* GetTipName() const R0;
	virtual bool CanHoldDown() const R0;
	virtual void Execute(bool isOn) const RX;

	AdvancedCommandBarButton()
	{
		this->ID = TotalButtonCount;
		TotalButtonCount++;
	}

	int ID;
};
