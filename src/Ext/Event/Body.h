#pragma once
#include <cstddef>
#include <stdint.h>
#include <TechnoClass.h>
#include <FootClass.h>

enum class EventTypeExt : uint8_t
{
	// Vanilla game used Events from 0x00 to 0x2F
	// CnCNet reserved Events from 0x30 to 0x3F
	// Ares used Events 0x60 and 0x61

	SyncStopRandomTargetTarNav = 0x49,

	FIRST = SyncStopRandomTargetTarNav,
	LAST = SyncStopRandomTargetTarNav
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

		struct SyncStopRandomTargetTarNav
		{
			int TechnoUniqueID;
		} SyncStopRandomTargetTarNav;
	};

	bool AddEvent();
	void RespondEvent();

	static size_t GetDataSize(EventTypeExt type);
	static bool IsValidType(EventTypeExt type);
};

static_assert(sizeof(EventExt) == 111);
static_assert(offsetof(EventExt, DataBuffer) == 7);
#pragma pack(pop)
