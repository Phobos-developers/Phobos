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

	ToggleAggressiveStance = 0x80,

	FIRST = ToggleAggressiveStance,
	LAST = ToggleAggressiveStance
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

		struct ToggleAggressiveStance
		{
			TargetClass Who;
		} ToggleAggressiveStance;
	};

	bool AddEvent();
	void RespondEvent();

	static void RaiseToggleAggressiveStance(TechnoClass* pTechno);
	void RespondToToggleAggressiveStance();

	static size_t GetDataSize(EventTypeExt type);
	static bool IsValidType(EventTypeExt type);
};

static_assert(sizeof(EventExt) == 111);
static_assert(offsetof(EventExt, DataBuffer) == 7);
#pragma pack(pop)

