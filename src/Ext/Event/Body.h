#pragma once

#include <cstddef>
#include <stdint.h>
#include <TechnoClass.h>
#include <TargetClass.h>

enum class EventTypeExt : uint8_t
{
	// Vanilla game used Events from 0x00 to 0x2F
	// CnCNet reserved Events from 0x30 to 0x3F
	// Ares used Events 0x60 and 0x61

	TriggerCustomHotkey = 0x70,

	FIRST = TriggerCustomHotkey,
	LAST = TriggerCustomHotkey
};

#pragma pack(push, 1)
class EventExt
{
public:
	EventTypeExt Type;
	bool IsExecuted;
	char HouseIndex;
	uint32_t Frame;
	union
	{
		char DataBuffer[104];

		struct TriggerCustomHotkey
		{
			TargetClass Who;
			int NumeralSequence;
		} TriggerCustomHotkey;
	};

	bool AddEvent();
	void RespondEvent();

	static void RaiseTriggerCustomHotkey(AbstractClass* pTarget, HouseClass* pHouse, int numeralSequence);
	void RespondToTriggerCustomHotkey();

	static size_t GetDataSize(EventTypeExt type);
	static bool IsValidType(EventTypeExt type);
};

static_assert(sizeof(EventExt) == 111);
static_assert(offsetof(EventExt, DataBuffer) == 7);
#pragma pack(pop)
