#pragma once

#include <CommandClass.h>
#include <StringTable.h>
#include <MessageListClass.h>
#include <Phobos.h>
#include <Utilities/Debug.h>

template <typename T>
T* MakeCommand()
{
	T* command = GameCreate<T>();
	CommandClass::Array.AddItem(command);
	return command;
};

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

#define CATEGORY_TEAM StringTable::LoadString(GameStrings::TXT_TEAM)
#define CATEGORY_INTERFACE StringTable::LoadString(GameStrings::TXT_INTERFACE)
#define CATEGORY_TAUNT StringTable::LoadString(GameStrings::TXT_TAUNT)
#define CATEGORY_SELECTION StringTable::LoadString(GameStrings::TXT_SELECTION)
#define CATEGORY_CONTROL StringTable::LoadString(GameStrings::TXT_CONTROL)
#define CATEGORY_DEBUG L"Debug"
#define CATEGORY_GUIDEBUG StringTable::LoadString(GameStrings::GUI_DEBUG)
#define CATEGORY_DEVELOPMENT StringTable::LoadString("TXT_DEVELOPMENT")
