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

class LaserDrawExt
{
public:
	using base_type = LaserDrawClass;

	static constexpr DWORD Canary = 0x4C617365; // "Lase"

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

		~ExtData() override
		{
			this->Unregister();
			LaserDrawExt::ClearPointer(this->OwnerObject());
		}

		void Initialize(TechnoClass* pShooter, AbstractClass* pTarget, int weaponIdx, PositionFollow mode,
			const CoordStruct& initialSource, const CoordStruct& localFLH, int burstIndex, bool stopOnFirerConvert);

		void Register();
		void Unregister();
	};

	static constexpr uintptr_t PadHighOffset = 0x22; // align_22
	static constexpr uintptr_t PadLowOffset = 0x4A;  // align_4A

	static ExtData* Find(LaserDrawClass* pLaser);
	static ExtData* Allocate(LaserDrawClass* pLaser);
	static void Release(LaserDrawClass* pLaser);
	static void ResetPointer(LaserDrawClass* pLaser);
	static void ClearPointer(LaserDrawClass* pLaser);

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
