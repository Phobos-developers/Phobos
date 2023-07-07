#pragma once

#include <AbstractClass.h>
#include <GeneralDefinitions.h>
#include <Randomizer.h>

#include <vector>

static constexpr unsigned int RNGCalls_Size = 4096;
static constexpr unsigned int FacingChanges_Size = 1024;
static constexpr unsigned int TargetChanges_Size = 1024;
static constexpr unsigned int DestinationChanges_Size = 1024;
static constexpr unsigned int MissionOverrides_Size = 256;

template <typename T, unsigned int size>
class SyncLogEventTracker
{
	using iterator = typename std::vector<T>::iterator;
	using const_iterator = typename std::vector<T>::const_iterator;
private:
	std::vector<T> Data;
	unsigned int LastPosition;
public:
	SyncLogEventTracker() : Data(size), LastPosition(0) { };

	void Add(T item)
	{
		Data[LastPosition] = item;
		LastPosition++;

		if (LastPosition >= Data.size())
			LastPosition = 0;
	}

	T Get(int index) { return Data[index]; }
	size_t Size() { return Data.size(); }

	iterator begin() { return Data.begin(); }
	iterator end() { return Data.end(); }
	const_iterator begin() const { return Data.begin(); }
	const_iterator end() const { return Data.end(); }
};

struct SyncLogEvent
{
	bool Initialized;
	unsigned int Caller;
	unsigned int Frame;

	SyncLogEvent() = default;

	SyncLogEvent(unsigned int Caller, unsigned int Frame)
		: Caller(Caller), Frame(Frame)
	{
		Initialized = true;
	}
};

struct RNGCallSyncLogEvent : SyncLogEvent
{
	int Type; // 0 = Invalid, 1 = Unranged, 2 = Ranged
	bool IsCritical;
	unsigned int Seed;
	unsigned int Index;
	int Min;
	int Max;

	RNGCallSyncLogEvent() = default;

	RNGCallSyncLogEvent(int Type, bool IsCritical, unsigned int Seed, unsigned int Index, unsigned int Caller, unsigned int Frame, int Min, int Max)
		: Type(Type), IsCritical(IsCritical), Seed(Seed), Index(Index), Min(Min), Max(Max), SyncLogEvent(Caller, Frame)
	{
	}
};

struct FacingChangeSyncLogEvent : SyncLogEvent
{
	unsigned short Facing;

	FacingChangeSyncLogEvent() = default;

	FacingChangeSyncLogEvent(unsigned short Facing, unsigned int Caller, unsigned int Frame)
		: Facing(Facing), SyncLogEvent(Caller, Frame)
	{
	}
};

struct TargetChangeSyncLogEvent : SyncLogEvent
{
	AbstractType Type;
	DWORD ID;
	AbstractType TargetType;
	DWORD TargetID;

	TargetChangeSyncLogEvent() = default;

	TargetChangeSyncLogEvent(const AbstractType& Type, const DWORD& ID, const AbstractType& TargetType, const DWORD& TargetID, unsigned int Caller, unsigned int Frame)
		: Type(Type), ID(ID), TargetType(TargetType), TargetID(TargetID), SyncLogEvent(Caller, Frame)
	{
	}
};

struct MissionOverrideSyncLogEvent : SyncLogEvent
{
	AbstractType Type;
	DWORD ID;
	int Mission;

	MissionOverrideSyncLogEvent() = default;

	MissionOverrideSyncLogEvent(const AbstractType& Type, const DWORD& ID, int Mission, unsigned int Caller, unsigned int Frame)
		: Type(Type), ID(ID), Mission(Mission), SyncLogEvent(Caller, Frame)
	{
	}
};

class SyncLogger
{
private:
	static SyncLogEventTracker<RNGCallSyncLogEvent, RNGCalls_Size> RNGCalls;
	static SyncLogEventTracker<FacingChangeSyncLogEvent, FacingChanges_Size> FacingChanges;
	static SyncLogEventTracker<TargetChangeSyncLogEvent, TargetChanges_Size> TargetChanges;
	static SyncLogEventTracker<TargetChangeSyncLogEvent, DestinationChanges_Size> DestinationChanges;
	static SyncLogEventTracker<MissionOverrideSyncLogEvent, MissionOverrides_Size> MissionOverrides;

	static void WriteRNGCalls(FILE* const pLogFile, int frameDigits);
	static void WriteFacingChanges(FILE* const pLogFile, int frameDigits);
	static void WriteTargetChanges(FILE* const pLogFile, int frameDigits);
	static void WriteDestinationChanges(FILE* const pLogFile, int frameDigits);
	static void WriteMissionOverrides(FILE* const pLogFile, int frameDigits);
public:
	static bool HooksDisabled;

	static void AddRNGCallSyncLogEvent(Randomizer* pRandomizer, int type, unsigned int callerAddress, int min = 0, int max = 0);
	static void AddFacingChangeSyncLogEvent(unsigned short facing, unsigned int callerAddress);
	static void AddTargetChangeSyncLogEvent(AbstractClass* pObject, AbstractClass* pTarget, unsigned int callerAddress);
	static void AddDestinationChangeSyncLogEvent(AbstractClass* pObject, AbstractClass* pTarget, unsigned int callerAddress);
	static void AddMissionOverrideSyncLogEvent(AbstractClass* pObject, int mission, unsigned int callerAddress);
	static void WriteSyncLog(const char* logFilename);
};
