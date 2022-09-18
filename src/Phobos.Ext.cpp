#include <Phobos.h>

#include <Ext/Aircraft/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/House/Body.h>
#include <Ext/RadSite/Body.h>
#include <Ext/Rules/Body.h>
#include <Ext/Scenario/Body.h>
#include <Ext/Script/Body.h>
#include <Ext/Side/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/TAction/Body.h>
#include <Ext/Team/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/TerrainType/Body.h>
#include <Ext/Tiberium/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/VoxelAnimType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>

#include <New/Type/RadTypeClass.h>
#include <New/Type/LaserTrailTypeClass.h>
#include <New/Type/DigitalDisplayTypeClass.h>

#include <utility>

#pragma region Implementation details

// this can be implicitly constructed from int,
// which can make selecting an overload unattractive,
// because it's a user-defined conversion. the more
// conversions, the less attractive
struct Dummy
{
	Dummy(int a) { };
};

// this is a typed nothing: a type list type
template <typename...>
struct DummyTypes { };

// calls:
// T::Clear()
// T::ExtMap.Clear()
struct ClearHelper
{
	template <typename T>
	static bool Process()
	{
		clear<T>(0, 0);

		return true;
	}

private:
	template <typename T>
	static auto clear(int, int) -> decltype(T::Clear())
	{
		T::Clear();
	}

	template <typename T>
	static auto clear(Dummy, int) -> decltype(T::ExtMap.Clear())
	{
		T::ExtMap.Clear();
	}

	template <typename T>
	static auto clear(Dummy, Dummy) -> void
	{
		// do nothing
	}
};

// calls:
// T::PointerGotInvalid(void*, bool)
// T::ExtMap.PointerGotInvalid(void*, bool)
struct InvalidatePointerHelper
{
	template <typename T>
	static bool Process(void* ptr, bool removed)
	{
		invalidpointer<T>(0, 0, ptr, removed);

		return true;
	}

private:
	template <typename T>
	static auto invalidpointer(int, int, void* ptr, bool removed) -> decltype(T::PointerGotInvalid(ptr, removed))
	{
		T::PointerGotInvalid(ptr, removed);
	}

	template <typename T>
	static auto invalidpointer(Dummy, int, void* ptr, bool removed) -> decltype(T::ExtMap.PointerGotInvalid(ptr, removed))
	{
		T::ExtMap.PointerGotInvalid(ptr, removed);
	}

	template <typename T>
	static auto invalidpointer(Dummy, Dummy, void* ptr, bool removed) -> void
	{
		// do nothing
	}
};

// calls:
// T::LoadGlobals(PhobosStreamReader&)
struct LoadHelper
{
	template <typename T>
	static bool Process(IStream* pStm)
	{
		return load<T>(0, pStm);
	}

private:
	template <typename T>
	static auto load(int, IStream* pStm) -> decltype(T::LoadGlobals(std::declval<PhobosStreamReader&>()))
	{
		PhobosByteStream Stm(0);
		Stm.ReadBlockFromStream(pStm);
		PhobosStreamReader Reader(Stm);

		return T::LoadGlobals(Reader) && Reader.ExpectEndOfBlock();
	}

	template <typename T>
	static auto load(Dummy, IStream* pStm) -> bool
	{
		// do nothing
		return true;
	}
};

// calls:
// T::SaveGlobals(PhobosStreamWriter&)
struct SaveHelper
{
	template <typename T>
	static bool Process(IStream* pStm)
	{
		return save<T>(0, pStm);
	}

private:
	template <typename T>
	static auto save(int, IStream* pStm) -> decltype(T::SaveGlobals(std::declval<PhobosStreamWriter&>()))
	{
		PhobosByteStream Stm;
		PhobosStreamWriter Writer(Stm);

		return T::SaveGlobals(Writer) && Stm.WriteBlockToStream(pStm);
	}

	template <typename T>
	static auto save(Dummy, IStream* pStm) -> bool
	{
		// do nothing
		return true;
	}
};

// this is a complicated thing that calls methods on classes. add types to the
// instantiation of this type, and the most appropriate method for each type
// will be called with no overhead of virtual functions.
template <typename... Ts>
struct MassAction
{
	__forceinline void Clear()
	{
		process<ClearHelper>(DummyTypes<Ts...>());
	}

	__forceinline void InvalidPointer(void* ptr, bool removed)
	{
		process<InvalidatePointerHelper>(DummyTypes<Ts...>(), ptr, removed);
	}

	__forceinline bool Load(IStream* pStm)
	{
		return process<LoadHelper>(DummyTypes<Ts...>(), pStm);
	}

	__forceinline bool Save(IStream* pStm)
	{
		return process<SaveHelper>(DummyTypes<Ts...>(), pStm);
	}

private:
	// T: the method dispatcher class to call with each type
	// TArgs: the arguments to call the method dispatcher's Process() method
	// TType and TTypes: setup for recursion. TType is the first type, the one
	// to handle now. TTypes is the tail that is recursively went into.

	// this is the base case, no more types, nothing to call
	template <typename T, typename... TArgs>
	bool process(DummyTypes<>, TArgs... args)
	{
		return true;
	}

	// this is the recursion part: invoke T:Process() for first type, then
	// recurse with the remaining types
	template <typename T, typename... TArgs, typename TType, typename... TTypes>
	__forceinline bool process(DummyTypes<TType, TTypes...>, TArgs... args)
	{
		if (!T::Process<TType>(args...))
			return false;

		return process<T>(DummyTypes<TTypes...>(), args...);
	}
};

#pragma endregion

// Add more class names as you like
auto MassActions = MassAction <
	// Ext classes
	AircraftExt,
	AnimTypeExt,
	AnimExt,
	BuildingExt,
	BuildingTypeExt,
	BulletExt,
	BulletTypeExt,
	HouseExt,
	RadSiteExt,
	RulesExt,
	ScenarioExt,
	ScriptExt,
	SideExt,
	SWTypeExt,
	TActionExt,
	TeamExt,
	TechnoExt,
	TechnoTypeExt,
	TerrainTypeExt,
	TiberiumExt,
	VoxelAnimExt,
	VoxelAnimTypeExt,
	WarheadTypeExt,
	WeaponTypeExt,
	// New classes
	ShieldTypeClass,
	LaserTrailTypeClass,
	RadTypeClass,
	ShieldClass,
	DigitalDisplayTypeClass
	// other classes
> ();

DEFINE_HOOK(0x7258D0, AnnounceInvalidPointer, 0x6)
{
	GET(AbstractClass* const, pInvalid, ECX);
	GET(bool const, removed, EDX);

	Phobos::PointerGotInvalid(pInvalid, removed);

	return 0;
}

DEFINE_HOOK(0x685659, Scenario_ClearClasses, 0xa)
{
	Phobos::Clear();
	return 0;
}

void Phobos::Clear()
{
	MassActions.Clear();
}

void Phobos::PointerGotInvalid(AbstractClass* const pInvalid, bool const removed)
{
	MassActions.InvalidPointer(pInvalid, removed);
}

HRESULT Phobos::SaveGameData(IStream* pStm)
{
	Debug::Log("Saving global Phobos data\n");

	if (!MassActions.Save(pStm))
		return E_FAIL;

	Debug::Log("Finished saving the game\n");

	return S_OK;
}

void Phobos::LoadGameData(IStream* pStm)
{
	Debug::Log("Loading global Phobos data\n");

	if (!MassActions.Load(pStm))
		Debug::Log("Error loading the game\n");
	else
		Debug::Log("Finished loading the game\n");
}

#ifdef DEBUG

#pragma warning (disable : 4091)
#pragma warning (disable : 4245)

#include <Dbghelp.h>
#include <tlhelp32.h>

bool Phobos::DetachFromDebugger()
{
	auto GetDebuggerProcessId = [](DWORD dwSelfProcessId) -> DWORD
	{
		DWORD dwParentProcessId = -1;
		HANDLE hSnapshot = CreateToolhelp32Snapshot(2, 0);
		PROCESSENTRY32 pe32;
		pe32.dwSize = sizeof(PROCESSENTRY32);
		Process32First(hSnapshot, &pe32);
		do
		{
			if (pe32.th32ProcessID == dwSelfProcessId)
			{
				dwParentProcessId = pe32.th32ParentProcessID;
				break;
			}
		}
		while (Process32Next(hSnapshot, &pe32));
		CloseHandle(hSnapshot);
		return dwParentProcessId;
	};

	HMODULE hModule = LoadLibrary("ntdll.dll");
	if (hModule != NULL)
	{
		auto const NtRemoveProcessDebug =
			(NTSTATUS(__stdcall*)(HANDLE, HANDLE))GetProcAddress(hModule, "NtRemoveProcessDebug");
		auto const NtSetInformationDebugObject =
			(NTSTATUS(__stdcall*)(HANDLE, ULONG, PVOID, ULONG, PULONG))GetProcAddress(hModule, "NtSetInformationDebugObject");
		auto const NtQueryInformationProcess =
			(NTSTATUS(__stdcall*)(HANDLE, ULONG, PVOID, ULONG, PULONG))GetProcAddress(hModule, "NtQueryInformationProcess");
		auto const NtClose =
			(NTSTATUS(__stdcall*)(HANDLE))GetProcAddress(hModule, "NtClose");

		HANDLE hDebug;
		HANDLE hCurrentProcess = GetCurrentProcess();
		NTSTATUS status = NtQueryInformationProcess(hCurrentProcess, 30, &hDebug, sizeof(HANDLE), 0);
		if (0 <= status)
		{
			ULONG killProcessOnExit = FALSE;
			status = NtSetInformationDebugObject(
				hDebug,
				1,
				&killProcessOnExit,
				sizeof(ULONG),
				NULL
			);
			if (0 <= status)
			{
				const auto pid = GetDebuggerProcessId(GetProcessId(hCurrentProcess));
				status = NtRemoveProcessDebug(hCurrentProcess, hDebug);
				if (0 <= status)
				{
					HANDLE hDbgProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
					if (INVALID_HANDLE_VALUE != hDbgProcess)
					{
						BOOL ret = TerminateProcess(hDbgProcess, EXIT_SUCCESS);
						CloseHandle(hDbgProcess);
						return ret;
					}
				}
			}
			NtClose(hDebug);
		}
		FreeLibrary(hModule);
	}

	return false;
}
#endif
