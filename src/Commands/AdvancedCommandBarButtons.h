#pragma once

#include <stdlib.h>

#include <CommandClass.h>
#include <StringTable.h>
#include <MessageListClass.h>
#include <Phobos.h>
#include <Utilities/Debug.h>

class ShapeButtonHelper
{
public:
	static constexpr int MaxButtonCount = 25;
	static constexpr int InUseButtonCount = 11;
	static constexpr int UnusedButtonCount = 1;
	static constexpr int OldButtonCount = InUseButtonCount + UnusedButtonCount;
	static constexpr int NewButtonCount = std::min(1, (MaxButtonCount - OldButtonCount));
	//  1. Team01
	//  2. Team02
	//  3. Team03
	//  4. TypeSelect
	//  5. Deploy
	//  6. AttackMove
	//  7. Guard
	//  8. Beacon
	//  9. Stop
	// 10. PlanningMode
	// 11. Cheer
	// 12. MoveToDeploy
	static constexpr const char* NewButtonNames[NewButtonCount] =
	{
		/* 13. */ "DistributionMode"
		/* New button name here */
	};
	static constexpr const char* NewButtonTipNames[NewButtonCount] =
	{
		"Tip:DistributionMode"
		// New button tip here
	};
	static int NewButtonIndexes[NewButtonCount];
};

class AdvancedCommandBarButton
{
public:
	static std::map<int, AdvancedCommandBarButton> IndexMap;

	virtual const char* GetName() const R0;
	virtual const char* GetTipName() const R0;
	virtual void Execute() const RX;
};
