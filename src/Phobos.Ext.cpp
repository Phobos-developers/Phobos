#include <Phobos.h>

#include <LoadOptionsClass.h>

#include <Ext/Aircraft/Body.h>
#include <Ext/AITriggerType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/Infantry/Body.h>
#include <Ext/Unit/Body.h>
#include <Ext/Cell/Body.h>
#include <Ext/EBolt/Body.h>
#include <Ext/OverlayType/Body.h>
#include <Ext/ParticleSystemType/Body.h>
#include <Ext/RadSite/Body.h>
#include <Ext/Scenario/Body.h>
#include <Ext/Script/Body.h>
#include <Ext/Side/Body.h>
#include <Ext/SWType/NewSWType/NewSWType.h>
#include <Ext/TAction/Body.h>
#include <Ext/TerrainType/Body.h>
#include <Ext/TEvent/Body.h>
#include <Ext/Tiberium/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/WarheadType/Body.h>

#include <New/Type/BannerTypeClass.h>
#include <New/Type/DigitalDisplayTypeClass.h>
#include <New/Type/LaserTrailTypeClass.h>
#include <New/Type/RadTypeClass.h>

#include <New/Entity/BannerClass.h>
#include <New/Type/SelectBoxTypeClass.h>

#include <Utilities/Detach.h>

#include <utility>

#pragma region Implementation details

#pragma region Concepts

// a hack to check if some type can be used as a specialization of a template
template <template <class...> class Template, class... Args>
void DerivedFromSpecialization(const Template<Args...>&);

template <class T, template <class...> class Template>
concept DerivedFromSpecializationOf =
	requires(const T & t) { DerivedFromSpecialization<Template>(t); };

template<typename TExt>
concept HasExtMap = requires { { TExt::ExtMap } -> DerivedFromSpecializationOf<Container>; };

template <typename T>
concept Clearable = requires { T::Clear(); };

template <typename T>
concept PointerInvalidationSubscribable =
	requires (void* ptr, bool removed) { T::PointerGotInvalid(ptr, removed); };

template <typename T>
concept GlobalSaveLoadable = requires
{
	T::LoadGlobals(std::declval<PhobosStreamReader&>());
	T::SaveGlobals(std::declval<PhobosStreamWriter&>());
};

template <typename TAction, typename TProcessed, typename... ArgTypes>
concept DispatchesAction =
	requires (ArgTypes... args) { TAction::template Process<TProcessed>(args...); };

#pragma endregion

// calls:
// T::Clear()
// T::ExtMap.Clear()
struct ClearAction
{
	template <typename T>
	static bool Process()
	{
		if constexpr (Clearable<T>)
			T::Clear();
		else if constexpr (HasExtMap<T>)
			T::ExtMap.Clear();

		return true;
	}
};

// calls:
// T::PointerGotInvalid(void*, bool)
struct InvalidatePointerAction
{
	template <typename T>
	static bool Process(void* ptr, bool removed)
	{
		if constexpr (PointerInvalidationSubscribable<T>)
			T::PointerGotInvalid(ptr, removed);

		return true;
	}
};

// calls:
// T::LoadGlobals(PhobosStreamReader&)
struct LoadGlobalsAction
{
	template <typename T>
	static bool Process(IStream* pStm)
	{
		if constexpr (GlobalSaveLoadable<T>)
		{
			PhobosByteStream stm(0);
			stm.ReadBlockFromStream(pStm);
			PhobosStreamReader reader(stm);

			return T::LoadGlobals(reader) && reader.ExpectEndOfBlock();
		}
		else
		{
			return true;
		}
	}
};

// calls:
// T::SaveGlobals(PhobosStreamWriter&)
struct SaveGlobalsAction
{
	template <typename T>
	static bool Process(IStream* pStm)
	{
		if constexpr (GlobalSaveLoadable<T>)
		{
			PhobosByteStream stm;
			PhobosStreamWriter writer(stm);

			return T::SaveGlobals(writer) && stm.WriteBlockToStream(pStm);
		}
		else
		{
			return true;
		}
	}
};

// calls:
// T::ExtMap.SaveAllToStream(IStream*)
struct SaveExtensionsAction
{
	template <typename T>
	static bool Process(IStream* pStm)
	{
		if constexpr (HasExtMap<T>)
			return T::ExtMap.SaveAllToStream(pStm);
		else
			return true;
	}
};

// calls:
// T::ExtMap.LoadAllFromStream(IStream*)
struct LoadExtensionsAction
{
	template <typename T>
	static bool Process(IStream* pStm)
	{
		if constexpr (HasExtMap<T>)
			return T::ExtMap.LoadAllFromStream(pStm);
		else
			return true;
	}
};

// calls:
// T::ExtMap.RelinkExtensionPointers()
struct RelinkExtensionsAction
{
	template <typename T>
	static bool Process()
	{
		if constexpr (HasExtMap<T>)
			T::ExtMap.RelinkExtensionPointers();

		return true;
	}
};

// calls:
// T::ExtMap.AllocatePendingExtensions()
struct AllocatePendingExtensionsAction
{
	// whether the last sweep created anything; the dispatch folds with && and would
	// short-circuit on a false result, so the outcome travels out of band
	static inline bool AllocatedAny = false;

	template <typename T>
	static bool Process()
	{
		if constexpr (HasExtMap<T>)
		{
			if (T::ExtMap.AllocatePendingExtensions())
				AllocatedAny = true;
		}

		return true;
	}
};

// this is a complicated thing that calls methods on classes. add types to the
// instantiation of this type, and the most appropriate method for each type
// will be called with no overhead of virtual functions.
template <typename... RegisteredTypes>
struct TypeRegistry
{
	__forceinline static void Clear()
	{
		dispatch_mass_action<ClearAction>();
	}

	__forceinline static void InvalidatePointer(void* ptr, bool removed)
	{
		dispatch_mass_action<InvalidatePointerAction>(ptr, removed);
	}

	__forceinline static bool LoadGlobals(IStream* pStm)
	{
		return dispatch_mass_action<LoadGlobalsAction>(pStm);
	}

	__forceinline static bool SaveGlobals(IStream* pStm)
	{
		return dispatch_mass_action<SaveGlobalsAction>(pStm);
	}

	__forceinline static bool SaveExtensions(IStream* pStm)
	{
		return dispatch_mass_action<SaveExtensionsAction>(pStm);
	}

	__forceinline static bool LoadExtensions(IStream* pStm)
	{
		return dispatch_mass_action<LoadExtensionsAction>(pStm);
	}

	__forceinline static void RelinkExtensions()
	{
		dispatch_mass_action<RelinkExtensionsAction>();
	}

	__forceinline static void AllocatePendingExtensions()
	{
		// a sweep can make the game create objects tracked by a container that was
		// already swept, so repeat until one full pass turns up nothing new
		do
		{
			AllocatePendingExtensionsAction::AllocatedAny = false;
			dispatch_mass_action<AllocatePendingExtensionsAction>();
		}
		while (AllocatePendingExtensionsAction::AllocatedAny);
	}

private:
	// TAction: the method dispatcher class to call with each type
	// ArgTypes: the argument types to call the method dispatcher's Process() method
	template <typename TAction, typename... ArgTypes>
		requires (DispatchesAction<TAction, RegisteredTypes, ArgTypes...> && ...)
	__forceinline static bool dispatch_mass_action(ArgTypes... args)
	{
		// (pack expression op ...) is a fold expression which
		// unfolds the parameter pack into a full expression
		return (TAction::template Process<RegisteredTypes>(args...) && ...);
	}
};

#pragma endregion

// Add more class names as you like
// NOTE: the order of extension classes with containers is the savegame stream order - append-only!
using PhobosTypeRegistry = TypeRegistry <
	// Ext classes
	AircraftExt,
	AircraftTypeExt,
	AITriggerTypeExt,
	AnimTypeExt,
	AnimExt,
	BuildingExt,
	BuildingTypeExt,
	BulletExt,
	BulletTypeExt,
	CellExt,
	EBoltExt,
	FootExt,
	HouseExt,
	HouseTypeExt,
	InfantryExt,
	InfantryTypeExt,
	OverlayTypeExt,
	ParticleSystemTypeExt,
	RadSiteExt,
	RulesExt,
	ScenarioExt,
	ScriptExt,
	SideExt,
	SWTypeExt,
	TActionExt,
	TeamExt,
	TeamTypeExt,
	TechnoExt,
	TechnoTypeExt,
	TerrainTypeExt,
	TEventExt,
	TiberiumExt,
	UnitExt,
	UnitTypeExt,
	VoxelAnimExt,
	VoxelAnimTypeExt,
	WarheadTypeExt,
	WeaponTypeExt,
	ParticleTypeExt,
	// New classes
	ShieldTypeClass,
	LaserTrailTypeClass,
	RadTypeClass,
	ShieldClass,
	DigitalDisplayTypeClass,
	BannerTypeClass,
	BannerClass,
	AttachEffectTypeClass,
	AttachEffectClass,
	NewSWType,
	SelectBoxTypeClass
	// other classes
> ;

DEFINE_HOOK(0x7258D0, AnnounceInvalidPointer, 0x6)
{
	GET(AbstractClass* const, pInvalid, ECX);
	GET(bool const, removed, EDX);

	Detach::NotifyAbstract(pInvalid, removed);
	PhobosTypeRegistry::InvalidatePointer(pInvalid, removed);

	if (removed)
	{
		if (auto const pObject = abstract_cast<ObjectClass*>(static_cast<AbstractClass*>(pInvalid)))
			WeaponTypeExt::OnObjectRemoved(pObject);
	}

	return 0;
}

DEFINE_HOOK(0x685659, Scenario_ClearClasses, 0xa)
{
	PhobosTypeRegistry::Clear();
	return 0;
}

// Ares saves its things at the end of the save
// Phobos will save the things at the beginning of the save
// Considering how DTA gets the scenario name, I decided to save it after Rules - secsome

DEFINE_HOOK(0x67D32C, SaveGame_Phobos, 0x5)
{
	GET(IStream*, pStm, ESI);

	if (!PhobosTypeRegistry::SaveGlobals(pStm) || !PhobosTypeRegistry::SaveExtensions(pStm))
		Debug::FatalErrorAndExit("SaveGame - Failed to save Phobos data!\n");

	return 0;
}

DEFINE_HOOK(0x67E826, LoadGame_Phobos, 0x6)
{
	GET(IStream*, pStm, ESI);

	if (!PhobosTypeRegistry::LoadGlobals(pStm) || !PhobosTypeRegistry::LoadExtensions(pStm))
		Debug::FatalErrorAndExit("LoadGame - Failed to load Phobos data!\n");

	return 0;
}

// AbstractClass::Load restores the object image from the savegame as-is, including the
// save-time extension pointer in the inline slot. Clear the slot right after the image
// is read, so anything fetching an extension during the load window (before the
// post-swizzle relink below) sees "no extension" instead of a stale pointer.
DEFINE_HOOK(0x4103D0, AbstractClass_Load_ClearExtensionSlot, 0x5)
{
	GET(AbstractClass*, pThis, ESI);

	AbstractExt::Attach(pThis, nullptr);

	return 0;
}

// First instruction after SwizzleManagerClass::Process has remapped every registered
// pointer: extension owners are valid again, write the extensions back into their
// owners' inline slots (cleared when the owners were loaded).
DEFINE_HOOK(0x67E685, LoadGame_PostSwizzle_Phobos, 0x5)
{
	PhobosTypeRegistry::RelinkExtensions();
	// with the restored extensions in place, the owners left over are the objects the
	// game created itself while loading; they get their extensions now
	PhobosTypeRegistry::AllocatePendingExtensions();
	return 0;
}

DEFINE_HOOK(0x67D04E, GameSave_SavegameInformation, 0x7)
{
	REF_STACK(SavegameInformation, Info, STACK_OFFSET(0x4A4, -0x3F4));

	// Bleeding-edge builds (local and nightly) skip the version offset entirely: each one is a
	// unique snapshot, so version-based filtering would both falsely claim compatibility between
	// same-version nightlies and hide saves from other bleeding-edge snapshots. Only release and
	// pre-release builds get the offset, which makes saves from different Phobos versions filter
	// out of each other's load lists.
#if defined(RELEASE)
	Info.InternalVersion = Info.InternalVersion + SAVEGAME_ID;
#endif
	strncat(Info.ExecutableName.data(),
		" + Phobos " FILE_VERSION_STR,
		Info.ExecutableName.Size - sizeof(" + Phobos " FILE_VERSION_STR)
	);

	return 0;
}

DEFINE_HOOK_AGAIN(0x67FD9D, LoadOptionsClass_GetFileInfo, 0x7)
DEFINE_HOOK(0x67FDB1, LoadOptionsClass_GetFileInfo, 0x7)
{
#if defined(RELEASE)
	GET(SavegameInformation*, Info, ESI);
	Info->InternalVersion = Info->InternalVersion - SAVEGAME_ID;
#endif
	return 0;
}

#ifdef DEBUG

#pragma warning (disable : 4091)
#pragma warning (disable : 4245)

#include <Dbghelp.h>
#include <tlhelp32.h>

/**
 *  Sets up to close Syringe when the game exits.
 *  We don't do it immediately so that the client doesn't think
 *  the game has exited once Syringe closes.
 *
 *  Ported from Vinifera
 *  @author: ZivDero, secsome
 */
static DWORD DebuggerPID = 0;

void _cdecl Kill_Debugger()
{
	if (DebuggerPID != 0)
	{
		HANDLE handle = OpenProcess(PROCESS_TERMINATE, FALSE, DebuggerPID);
		if (handle)
		{
			TerminateProcess(handle, EXIT_SUCCESS);
			CloseHandle(handle);
		}
	}
}

void Setup_Kill_Debugger(DWORD pid)
{
	DebuggerPID = pid;
	atexit(Kill_Debugger);
}

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
					Setup_Kill_Debugger(pid);
					return true;
				}
			}
			NtClose(hDebug);
		}
		FreeLibrary(hModule);
	}

	return false;
}
#endif
