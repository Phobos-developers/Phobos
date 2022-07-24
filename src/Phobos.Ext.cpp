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

#include <utility>

#pragma region Implementation details

#pragma region Concepts

// a hack to check if some type can be used as a specialization of a template
template <template <class...> class Template, class... Args>
void DerivedFromSpecialization(const Template<Args...>&);

template <class T, template <class...> class Template>
concept DerivedFromSpecializationOf =
	requires(const T & t) { DerivedFromSpecialization<Template>(t); };

template<typename T>
concept HasExtMap = requires { { T::ExtMap } -> DerivedFromSpecializationOf<Container>; };

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

template <typename THelper, typename TProcessable, typename... TProcessArgs>
concept DispatchesAction =
	requires (TProcessArgs... args) { THelper::template Process<TProcessable>(args...); };

#pragma endregion

// this is a typed nothing: a type list type
template <typename...>
struct PhobosTypeList { };

// calls:
// T::Clear()
// T::ExtMap.Clear()
struct ClearHelper
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
// T::ExtMap.PointerGotInvalid(void*, bool)
struct InvalidatePointerHelper
{
	template <typename T>
	static bool Process(void* ptr, bool removed)
	{
		if constexpr (PointerInvalidationSubscribable<T>)
			T::PointerGotInvalid(ptr, removed);
		else if constexpr (HasExtMap<T>)
			T::ExtMap.PointerGotInvalid(ptr, removed);

		return true;
	}
};

// calls:
// T::LoadGlobals(PhobosStreamReader&)
struct LoadHelper
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
struct SaveHelper
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

// this is a complicated thing that calls methods on classes. add types to the
// instantiation of this type, and the most appropriate method for each type
// will be called with no overhead of virtual functions.
template <typename... Ts>
struct MassAction
{
	__forceinline void Clear()
	{
		process<ClearHelper>(PhobosTypeList<Ts...>());
	}

	__forceinline void InvalidPointer(void* ptr, bool removed)
	{
		process<InvalidatePointerHelper>(PhobosTypeList<Ts...>(), ptr, removed);
	}

	__forceinline bool Load(IStream* pStm)
	{
		return process<LoadHelper>(PhobosTypeList<Ts...>(), pStm);
	}

	__forceinline bool Save(IStream* pStm)
	{
		return process<SaveHelper>(PhobosTypeList<Ts...>(), pStm);
	}

private:
	// THelper: the method dispatcher class to call with each type
	// TArgs: the arguments to call the method dispatcher's Process() method
	// TTypes: the classes to process.
	template <typename THelper, typename... TArgs, typename... TTypes>
	requires (DispatchesAction<THelper, TTypes, TArgs...> && ...)
	__forceinline bool process(PhobosTypeList<TTypes...>, TArgs... args)
	{
		// (pack expression op ...) is a fold expression which
		// unfolds the parameter pack into a full expression
		return (THelper::template Process<TTypes>(args...) && ...);
	}
};

#pragma endregion

// Add more class names as you like
using PhobosMassAction = MassAction<
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
	RadTypeClass
	// other classes
>;

auto MassActions = PhobosMassAction();

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
					sprintf_s(Phobos::readBuffer, "taskkill /F /PID %d", pid);
					WinExec(Phobos::readBuffer, SW_HIDE);
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
