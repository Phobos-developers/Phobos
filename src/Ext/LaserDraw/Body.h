#pragma once
#include <LaserDrawClass.h>

#include <AbstractClass.h>
#include <BuildingClass.h>
#include <ObjectClass.h>
#include <TechnoClass.h>
#include <TechnoTypeClass.h>

#include <Utilities/Container.h>
#include <Utilities/Enum.h>

#include <unordered_map>
#include <vector>

// O(1) per-laser tracking data for LaserPositionUpdate.
//
// The extension pointer is stored split across the two unused 16-bit padding
// words of LaserDrawClass (align_22 @ 0x22 = high half, align_4A @ 0x4A = low
// half), giving O(1) access on the per-frame hot path instead of the previous
// std::unordered_map lookup. LaserDrawClass is not AbstractClass-derived, so it
// has no unified 0x18 extension slot - the padding split is the only spare space.
class LaserDrawExt
{
public:
	using base_type = LaserDrawClass;

	static constexpr DWORD Canary = 0x4C617365; // "Lase"

	// reverse maps: an ObjectClass being removed -> the lasers tracking it, for
	// pointer invalidation on object removal (not per-frame hot)
	static std::unordered_map<ObjectClass*, std::vector<LaserDrawClass*>> ShooterToLasers;
	static std::unordered_map<ObjectClass*, std::vector<LaserDrawClass*>> TargetToLasers;

	class ExtData final : public Extension<LaserDrawClass>
	{
	public:
		TechnoClass* Shooter = nullptr;
		ObjectClass* Target = nullptr;
		int WeaponIndex = 0;
		PositionFollow FollowMode = PositionFollow::None;
		CoordStruct SavedOffset = CoordStruct::Empty;
		CoordStruct LocalFLH = CoordStruct::Empty;
		int FrozenBurstIndex = 0;
		bool StopOnFirerConvert = false;
		const TechnoTypeClass* OriginalType = nullptr;

		ExtData(LaserDrawClass* pOwner) : Extension<LaserDrawClass>(pOwner) { }

		virtual ~ExtData() = default;

		void Initialize(TechnoClass* pShooter, AbstractClass* pTarget, int weaponIdx, PositionFollow mode,
			const CoordStruct& initialSource, const CoordStruct& localFLH, int burstIndex, bool stopOnFirerConvert);

		void Register();
		void Unregister();
	};

	// --- O(1) padding-pointer access (the per-frame hot path) ---
	static constexpr uintptr_t PadHighOffset = 0x22; // align_22
	static constexpr uintptr_t PadLowOffset = 0x4A;  // align_4A

	static ExtData* Find(LaserDrawClass* pLaser);
	static ExtData* Allocate(LaserDrawClass* pLaser);
	static void Release(LaserDrawClass* pLaser);
	static void ResetPointer(LaserDrawClass* pLaser);

	class ExtContainer final : public Container<LaserDrawExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};

using LaserDrawExtension = LaserDrawExt::ExtData;
