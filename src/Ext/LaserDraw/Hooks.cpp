#include <Ext/LaserDraw/Body.h>

#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>
#include <DiskLaserClass.h>
#include <Helpers/Macro.h>
#include <Utilities/Patch.h>

static void NukeTrackingHooks()
{
	Patch::Apply_RAW(0x54FE60, { 0x56, 0x8B, 0xF1, 0x33, 0xC0 }); // LaserDrawClass_CTOR_Update
	Patch::Apply_RAW(0x550016, { 0x8B, 0x15, 0x78, 0xC8, 0xAB, 0x00 }); // LaserDrawClass_RemoveTracking
	Patch::Apply_RAW(0x5501D7, { 0xA1, 0x78, 0xC8, 0xAB, 0x00 }); // LaserDrawClass_RemoveTracking
	Patch::Apply_RAW(0x6FD210, { 0x83, 0xEC, 0x38, 0x8B, 0x44, 0x24, 0x44 }); // TechnoClass_LaserZap_SetTrackingContext
	Patch::Apply_RAW(0x6FD446, { 0x5F, 0x5E, 0x5D, 0x5B, 0x83, 0xC4, 0x38 }); // TechnoClass_LaserZap_Tracking
	Patch::Apply_RAW(0x46A8AC, { 0xE8, 0x5F, 0x29, 0x29, 0x00 }); // Shrapnel_CreateLaser_Wrapper
	Patch::Apply_RAW(0x46AD81, { 0xE8, 0x8A, 0x24, 0x29, 0x00 }); // Shrapnel_CreateLaser_Wrapper
	Patch::Apply_RAW(0x4A7696, { 0x8B, 0x46, 0x2C, 0x8B, 0x56, 0x24 }); // DiskLaser_Update_ActivateMainBeam_Tracking
	Patch::Apply_RAW(0x550173, { 0x8B, 0x56, 0x08, 0x8B, 0x46, 0x10 }); // LaserDrawClass_Update_Tracking
}

namespace LaserRT
{
	TechnoClass* Shooter = nullptr;
	AbstractClass* Target = nullptr;
	int WeaponIndex = 0;
	bool IgnoreShooter = false;
	CoordStruct SavedLocalFLH = CoordStruct::Empty;
	int SavedBurstIndex = 0;

	void SetLaserTrackingData(LaserDrawClass* pLaser, TechnoClass* pShooter, AbstractClass* pTarget, int weaponIdx, PositionFollow mode, bool ignoreShooter)
	{
		CoordStruct localFLH;
		int burstIndex = 0;
		bool stopOnFirerConvert = false;

		if (pShooter)
		{
			bool flhFound = false;
			localFLH = TechnoExt::GetBurstFLH(pShooter, weaponIdx, flhFound);

			if (!flhFound)
				localFLH = pShooter->GetWeapon(weaponIdx)->FLH;

			burstIndex = pShooter->CurrentBurstIndex;

			if (const auto pWeapon = pShooter->GetWeapon(weaponIdx)->WeaponType)
			{
				const auto pWeaponExt = WeaponTypeExt::Fetch(pWeapon);
				stopOnFirerConvert = pWeaponExt->LaserPositionUpdate_StopOnFirerConvert.Get(RulesExt::Global()->LaserPositionUpdate_StopOnFirerConvert);
			}
		}

		auto* pExt = LaserDrawExt::Find(pLaser);
		if (!pExt)
			pExt = LaserDrawExt::Allocate(pLaser);
		if (!pExt)
			return;

		pExt->Unregister();
		pExt->Initialize(ignoreShooter ? nullptr : pShooter, pTarget, weaponIdx, mode, pLaser->Source, localFLH, burstIndex, stopOnFirerConvert);
		pExt->Register();
	}
}

// container hooks

// The CTOR must attach an extension even with empty tracking data - removing it
// would break DiskLaser's charging ring (start point on ring A, end point on ring B).
DEFINE_HOOK(0x54FE60, LaserDrawClass_CTOR_Update, 0x5)
{
	GET(LaserDrawClass*, pLaser, ECX);

	LaserDrawExt::ResetPointer(pLaser);

	if (Phobos::Optimizations::DisableLaserTracking)
	{
		// no weapon uses LaserPositionUpdate: drop every tracking hook for good
		NukeTrackingHooks();
		return 0;
	}

	LaserDrawExt::Allocate(pLaser);

	return 0;
}

static void RemoveLaserFromTracking(LaserDrawClass* pLaser)
{
	if (auto* pExt = LaserDrawExt::Find(pLaser))
	{
		pExt->Unregister();
		LaserDrawExt::Release(pLaser);
	}
}

DEFINE_HOOK_AGAIN(0x5501D7, LaserDrawClass_RemoveTracking, 0x5)
DEFINE_HOOK(0x550016, LaserDrawClass_RemoveTracking, 0x6)
{
	GET(LaserDrawClass*, pLaser, ESI);
	RemoveLaserFromTracking(pLaser);
	return 0;
}

void WeaponTypeExt::OnObjectRemoved(ObjectClass* pObject)
{
	auto itShoot = LaserDrawExt::ShooterToLasers.find(pObject);
	if (itShoot != LaserDrawExt::ShooterToLasers.end())
	{
		for (auto pLaser : itShoot->second)
		{
			if (auto* pExt = LaserDrawExt::Find(pLaser))
			{
				if (pExt->Shooter == pObject)
					pExt->Shooter = nullptr;

				if (!pExt->Shooter && !pExt->Target)
					LaserDrawExt::Release(pLaser);
			}
		}
		LaserDrawExt::ShooterToLasers.erase(itShoot);
	}
	LaserDrawExt::ShooterToLasers.erase(pObject);

	auto itTarget = LaserDrawExt::TargetToLasers.find(pObject);
	if (itTarget != LaserDrawExt::TargetToLasers.end())
	{
		for (auto pLaser : itTarget->second)
		{
			if (auto* pExt = LaserDrawExt::Find(pLaser))
			{
				if (pExt->Target == pObject)
					pExt->Target = nullptr;

				if (!pExt->Shooter && !pExt->Target)
					LaserDrawExt::Release(pLaser);
			}
		}
		LaserDrawExt::TargetToLasers.erase(itTarget);
	}
	LaserDrawExt::TargetToLasers.erase(pObject);
}

// hooks

DEFINE_HOOK(0x6FD210, TechnoClass_LaserZap_SetTrackingContext, 0x7)
{
	if (Phobos::Optimizations::DisableLaserTracking)
		return 0;

	GET(TechnoClass*, pShooter, ECX);
	GET_STACK(ObjectClass*, pTarget, 0x4);
	GET_STACK(const int, weaponIdx, 0x8);

	LaserRT::Shooter = LaserRT::IgnoreShooter ? nullptr : pShooter;
	LaserRT::Target = pTarget;
	LaserRT::WeaponIndex = weaponIdx;

	LaserRT::SavedBurstIndex = pShooter->CurrentBurstIndex;
	bool flhFound = false;
	LaserRT::SavedLocalFLH = TechnoExt::GetBurstFLH(pShooter, weaponIdx, flhFound);

	if (!flhFound)
	{
		LaserRT::SavedLocalFLH = pShooter->GetWeapon(weaponIdx)->FLH;

		if (LaserRT::SavedBurstIndex % 2 != 0)
			LaserRT::SavedLocalFLH.Y = -LaserRT::SavedLocalFLH.Y;
	}
	return 0;
}

DEFINE_HOOK(0x6FD446, TechnoClass_LaserZap_Tracking, 0x7)
{
	if (Phobos::Optimizations::DisableLaserTracking)
		return 0;

	GET(WeaponTypeClass*, pWeapon, ECX);
	GET(LaserDrawClass*, pLaser, EAX);
	const auto mode = WeaponTypeExt::Fetch(pWeapon)->LaserPositionUpdate.Get();

	if (mode == PositionFollow::None)
		return 0;

	const auto pShooter = std::exchange(LaserRT::Shooter, nullptr);
	const auto pTarget = std::exchange(LaserRT::Target, nullptr);
	const int weaponIdx = std::exchange(LaserRT::WeaponIndex, 0);

	// The current implementation no longer requires storing into a variable, but resetting operations still need to be handled.
	std::exchange(LaserRT::SavedLocalFLH, CoordStruct::Empty);
	std::exchange(LaserRT::SavedBurstIndex, 0);

	LaserRT::SetLaserTrackingData(pLaser, pShooter, pTarget, weaponIdx, mode, false);
	return 0;
}

static LaserDrawClass* __fastcall Shrapnel_CreateLaser_Wrapper(TechnoClass* pShooter, void*, ObjectClass* pTarget, int weaponIdx, WeaponTypeClass* pWeapon, const CoordStruct& sourceCoords)
{
	const auto mode = WeaponTypeExt::Fetch(pWeapon)->LaserPositionUpdate.Get();

	if (mode == PositionFollow::None)
		return pShooter->CreateLaser(pTarget, weaponIdx, pWeapon, sourceCoords);

	LaserRT::IgnoreShooter = true;
	const auto pLaser = pShooter->CreateLaser(pTarget, weaponIdx, pWeapon, sourceCoords);
	LaserRT::IgnoreShooter = false;
	return pLaser;
}
DEFINE_FUNCTION_JUMP(CALL, 0x46A8AC, Shrapnel_CreateLaser_Wrapper)
DEFINE_FUNCTION_JUMP(CALL, 0x46AD81, Shrapnel_CreateLaser_Wrapper)

// DiskLaser main beam activation
DEFINE_HOOK(0x4A7696, DiskLaser_Update_ActivateMainBeam_Tracking, 0x6)
{
	if (Phobos::Optimizations::DisableLaserTracking)
		return 0;

	GET(LaserDrawClass*, pLaser, EAX);

	if (!pLaser)
		return 0;

	GET(DiskLaserClass*, pDiskLaser, ESI);
	const auto pWeapon = pDiskLaser->Weapon;

	if (!pWeapon)
		return 0;

	const auto mode = WeaponTypeExt::Fetch(pWeapon)->LaserPositionUpdate.Get();

	if (mode == PositionFollow::None)
		return 0;

	if (pLaser->Source == pLaser->Target)
		return 0;

	LaserRT::SetLaserTrackingData(pLaser, pDiskLaser->Owner, pDiskLaser->Target, 0, mode, false);
	return 0;
}

// Per‑frame coordinate update
DEFINE_HOOK(0x550173, LaserDrawClass_Update_Tracking, 0x6)
{
	if (LaserDrawExt::ExtMap.size() == 0)
		return 0;

	GET(LaserDrawClass*, pLaser, ESI);
	auto* pExt = LaserDrawExt::Find(pLaser);

	if (!pExt)
		return 0;

	if (const auto pShooter = pExt->Shooter)
	{
		if (pExt->StopOnFirerConvert && pExt->OriginalType)
		{
			if (pShooter->GetTechnoType() != pExt->OriginalType)
				pExt->Shooter = nullptr;
		}

		if (pExt->Shooter)
		{
			const int savedBurstIndex = pShooter->CurrentBurstIndex;
			pShooter->CurrentBurstIndex = pExt->FrozenBurstIndex;
			const CoordStruct worldFLH = pShooter->GetFLH(pExt->WeaponIndex, CoordStruct::Empty);
			pShooter->CurrentBurstIndex = savedBurstIndex;

			pLaser->Source = worldFLH + pExt->SavedOffset;
		}
	}

	if (const auto pTarget = pExt->Target)
		pLaser->Target = pTarget->GetTargetCoords();

	return 0;
}
