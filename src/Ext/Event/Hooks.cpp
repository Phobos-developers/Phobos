#include <EventClass.h>

#include <Utilities/Macro.h>

#include <climits>
#include <vector>

namespace EventOutQueue
{
	std::vector<EventClass> Pending;

	constexpr DWORD OutListHeadAddress = 0x00A802CCu;
	constexpr DWORD OutListArrayAddress = 0x00A802D4u;

	int& Head()
	{
		return *reinterpret_cast<int*>(OutListHeadAddress);
	}

	EventClass& Entry(int index)
	{
		return reinterpret_cast<EventClass*>(OutListArrayAddress)[index];
	}

	void MakeRoom()
	{
		auto& outList = EventClass::OutList;

		if (outList.Count < EventClass::MAX_EVENTS)
			return;

		Pending.push_back(Entry(Head()));
		Head() = (Head() + 1) & (EventClass::MAX_EVENTS - 1);
		--outList.Count;
	}

	void Replay()
	{
		if (Pending.empty())
			return;

		size_t replayed = 0;

		while (replayed < Pending.size() && EventClass::OutList.Add(Pending[replayed]))
			++replayed;

		if (replayed)
			Pending.erase(Pending.begin(), Pending.begin() + replayed);
	}
}

DEFINE_HOOK_AGAIN(0x443499, EventClass_OutList_MakeRoom, 0xA)
DEFINE_HOOK_AGAIN(0x4435AD, EventClass_OutList_MakeRoom, 0xA)
DEFINE_HOOK_AGAIN(0x4437F5, EventClass_OutList_MakeRoom, 0xA)
DEFINE_HOOK_AGAIN(0x4439D2, EventClass_OutList_MakeRoom, 0xA)
DEFINE_HOOK_AGAIN(0x443B1C, EventClass_OutList_MakeRoom, 0xA)
DEFINE_HOOK_AGAIN(0x48CA48, EventClass_OutList_MakeRoom, 0xA)
DEFINE_HOOK_AGAIN(0x48CAC9, EventClass_OutList_MakeRoom, 0xA)
DEFINE_HOOK_AGAIN(0x48CBC5, EventClass_OutList_MakeRoom, 0xA)
DEFINE_HOOK_AGAIN(0x4ABB5A, EventClass_OutList_MakeRoom, 0xA)
DEFINE_HOOK_AGAIN(0x4AC18C, EventClass_OutList_MakeRoom, 0xA)
DEFINE_HOOK_AGAIN(0x4AC1B7, EventClass_OutList_MakeRoom, 0xA)
DEFINE_HOOK_AGAIN(0x4AC23F, EventClass_OutList_MakeRoom, 0xA)
DEFINE_HOOK_AGAIN(0x4D7633, EventClass_OutList_MakeRoom, 0xA)
DEFINE_HOOK_AGAIN(0x4E1E59, EventClass_OutList_MakeRoom, 0xA)
DEFINE_HOOK_AGAIN(0x55CFFE, EventClass_OutList_MakeRoom, 0xA)
DEFINE_HOOK_AGAIN(0x5DA898, EventClass_OutList_MakeRoom, 0xA)
DEFINE_HOOK_AGAIN(0x646EF5, EventClass_OutList_MakeRoom, 0xA)
DEFINE_HOOK_AGAIN(0x646FD2, EventClass_OutList_MakeRoom, 0xA)
DEFINE_HOOK_AGAIN(0x647079, EventClass_OutList_MakeRoom, 0xA)
DEFINE_HOOK_AGAIN(0x647129, EventClass_OutList_MakeRoom, 0xA)
DEFINE_HOOK_AGAIN(0x6471F1, EventClass_OutList_MakeRoom, 0xA)
DEFINE_HOOK_AGAIN(0x64A306, EventClass_OutList_MakeRoom, 0xA)
DEFINE_HOOK_AGAIN(0x6641AC, EventClass_OutList_MakeRoom, 0xA)
DEFINE_HOOK_AGAIN(0x6A8EED, EventClass_OutList_MakeRoom, 0xA)
DEFINE_HOOK_AGAIN(0x6AA7EC, EventClass_OutList_MakeRoom, 0xA)
DEFINE_HOOK_AGAIN(0x6AA8D2, EventClass_OutList_MakeRoom, 0xA)
DEFINE_HOOK_AGAIN(0x6AB02F, EventClass_OutList_MakeRoom, 0xA)
DEFINE_HOOK_AGAIN(0x6AB130, EventClass_OutList_MakeRoom, 0xA)
DEFINE_HOOK_AGAIN(0x6AB204, EventClass_OutList_MakeRoom, 0xA)
DEFINE_HOOK_AGAIN(0x6AB236, EventClass_OutList_MakeRoom, 0xA)
DEFINE_HOOK_AGAIN(0x6AB34E, EventClass_OutList_MakeRoom, 0xA)
DEFINE_HOOK_AGAIN(0x6AB401, EventClass_OutList_MakeRoom, 0xA)
DEFINE_HOOK_AGAIN(0x6AB516, EventClass_OutList_MakeRoom, 0xA)
// 0x6AB773 is deliberately not hooked: Ares 3.0p1 hooks the same `cmp` there as
// SelectClass_ProcessInput_ProduceUnsuspended, and the other seven capacity checks of this
// function are already covered above, so letting Ares keep that one costs little.
DEFINE_HOOK_AGAIN(0x730F71, EventClass_OutList_MakeRoom, 0xA)
DEFINE_HOOK_AGAIN(0x73188F, EventClass_OutList_MakeRoom, 0xA)
DEFINE_HOOK_AGAIN(0x7319A1, EventClass_OutList_MakeRoom, 0xA)
// The same check written register-first (`mov eax, dword_A802C8` then `cmp eax, 80h`), including
// the shared enqueue routine at 0x6521C0 that planning mode orders go through.
DEFINE_HOOK_AGAIN(0x4C7D5B, EventClass_OutList_MakeRoom, 0x5)
DEFINE_HOOK_AGAIN(0x64778F, EventClass_OutList_MakeRoom, 0x5)
DEFINE_HOOK_AGAIN(0x647DED, EventClass_OutList_MakeRoom, 0x5)
DEFINE_HOOK_AGAIN(0x647ECE, EventClass_OutList_MakeRoom, 0x5)
DEFINE_HOOK_AGAIN(0x6521C9, EventClass_OutList_MakeRoom, 0x5)
DEFINE_HOOK(0x6FFE55, EventClass_OutList_MakeRoom, 0xA)
{
	EventOutQueue::MakeRoom();

	return 0;
}

DEFINE_HOOK(0x55DE40, UpdateFrame_EventOutQueue_Replay, 0x5)
{
	EventOutQueue::Replay();

	return 0;
}

#pragma region PlanningOrderLimit

// Vanilla refuses a planning order above 128 units; this raises it to 256 rather than removing it.
// EventClass::DoList (0x64C380) moves at most 256 MegaMission events per frame into
// EventClass::MegaMissionList (0xA83ED0), a fixed array with no room to grow in place, so an order
// above that would keep all its planning nodes but only the first 256 would ever act. Refusing it
// whole keeps that half-applied state from happening.
constexpr int MaxPlanningOrderUnits = 256;

DEFINE_HOOK(0x6391A7, PlanningManager_CanAddPlanningNodes_MaxUnits, 0x6)
{
	GET(const int, unitCount, EBP);

	R->EBP(unitCount <= MaxPlanningOrderUnits ? 0 : INT_MAX);

	return 0;
}

#pragma endregion
