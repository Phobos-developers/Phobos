#include <Ext\WeaponType\Body.h>
#include <Ext/Techno/Body.h>
#include <Helpers/Macro.h>
#include <Utilities/GeneralUtils.h>
#include <unordered_map>

namespace LaserDrawTemp
{
	ColorStruct maxColor;
}

DEFINE_HOOK(0x550D1F, LaserDrawClass_DrawInHouseColor_Context_Set, 0x6)
{
	LaserDrawTemp::maxColor = ColorStruct(*R->lea_Stack<ColorStruct*>(0x14));
	return 0;
}

//Enables proper laser thickness and falloff of it
DEFINE_HOOK(0x550F47, LaserDrawClass_DrawInHouseColor_BetterDrawing, 0x0)
{
	// Restore overridden code that's needed - Kerbiter
	GET_STACK(const bool, noQuickDraw, 0x13);
	R->ESI(noQuickDraw ? 8u : 64u);

	GET(LaserDrawClass*, pThis, EBX);
	GET_STACK(const int, currentThickness, 0x5C);

	double mult = 1.0;

	if (pThis->Thickness > 1)
	{
		const double falloffStep = 1.0 / pThis->Thickness;
		const double falloffMult = GeneralUtils::FastPow(1.0 - falloffStep, currentThickness);
		mult = (1.0 - falloffStep * currentThickness) * falloffMult;
	}

	const unsigned int r = (unsigned int)(mult * LaserDrawTemp::maxColor.R);
	const unsigned int g = (unsigned int)(mult * LaserDrawTemp::maxColor.G);
	const unsigned int b = (unsigned int)(mult * LaserDrawTemp::maxColor.B);

	R->EAX(r);
	R->ECX(g);
	R->EDX(b);

	return 0x550F9D;
}

DEFINE_HOOK(0x6FD3FD, TechnoClass_LaserZap_ZAdjust, 0x5)
{
	GET_STACK(WeaponTypeClass*, pWeapon, STACK_OFFSET(0x6C, 0xC));
	GET(int, zAdjust, EAX);

	zAdjust += WeaponTypeExt::Fetch(pWeapon)->LaserZAdjust.Get(RulesExt::Global()->LaserZAdjust);
	R->EAX(zAdjust);

	return 0;
}

#pragma region LaserPositionUpdate

namespace LaserRT
{
	struct TrackingData
	{
		TechnoClass* Shooter { nullptr };
		ObjectClass* Target { nullptr };
		int WeaponIndex { 0 };
		PositionFollow FollowMode { PositionFollow::None };
		CoordStruct SavedOffset { CoordStruct::Empty };
		CoordStruct LocalFLH { CoordStruct::Empty };
		int FrozenBurstIndex { 0 };
		bool StopOnFirerConvert { false };
		const TechnoTypeClass* OriginalType { nullptr };

		void Initialize(TechnoClass* pShooter, AbstractClass* pTarget, int weaponIdx, PositionFollow mode, const CoordStruct& initialSource, const CoordStruct& localFLH, int burstIndex, bool stopOnFirerConvert)
		{
			const auto pShooterBuilding = abstract_cast<BuildingClass*>(pShooter);

			if (pShooterBuilding && pShooterBuilding->Type->MaxNumberOccupants > 0)
				mode &= ~PositionFollow::Firer;

			if (pShooter && (mode & PositionFollow::Firer))
			{
				this->Shooter = pShooter;
				this->LocalFLH = localFLH;
				this->FrozenBurstIndex = burstIndex;
				this->StopOnFirerConvert = stopOnFirerConvert;

				if (stopOnFirerConvert)
					this->OriginalType = pShooter->GetTechnoType();

				const int savedBurstIndex = pShooter->CurrentBurstIndex;
				pShooter->CurrentBurstIndex = burstIndex;
				const CoordStruct worldFLH = pShooter->GetFLH(weaponIdx, localFLH);
				pShooter->CurrentBurstIndex = savedBurstIndex;

				this->SavedOffset = initialSource - worldFLH;
			}

			if (mode & PositionFollow::Target)
				this->Target = abstract_cast<ObjectClass*>(pTarget);

			this->WeaponIndex = weaponIdx;
			this->FollowMode = mode;
		}
	};

	std::unordered_map<LaserDrawClass*, TrackingData> TrackingMap;

	std::unordered_map<ObjectClass*, std::vector<LaserDrawClass*>> ShooterToLasers;
	std::unordered_map<ObjectClass*, std::vector<LaserDrawClass*>> TargetToLasers;

	static void RegisterTracking(LaserDrawClass* pLaser, const TrackingData& data)
	{
		if (data.Shooter && (data.FollowMode & PositionFollow::Firer))
			ShooterToLasers[data.Shooter].push_back(pLaser);
		if (data.Target && (data.FollowMode & PositionFollow::Target))
			TargetToLasers[data.Target].push_back(pLaser);
	}

	static void UnregisterTracking(LaserDrawClass* pLaser, const TrackingData& data)
	{
		if (data.Shooter)
		{
			auto it = ShooterToLasers.find(data.Shooter);
			if (it != ShooterToLasers.end())
			{
				auto& vec = it->second;
				vec.erase(std::remove(vec.begin(), vec.end(), pLaser), vec.end());
				if (vec.empty())
					ShooterToLasers.erase(it);
			}
		}
		if (data.Target)
		{
			auto it = TargetToLasers.find(data.Target);
			if (it != TargetToLasers.end())
			{
				auto& vec = it->second;
				vec.erase(std::remove(vec.begin(), vec.end(), pLaser), vec.end());
				if (vec.empty())
					TargetToLasers.erase(it);
			}
		}
	}

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

		TrackingData data;
		data.Initialize(ignoreShooter ? nullptr : pShooter, pTarget, weaponIdx, mode, pLaser->Source, localFLH, burstIndex, stopOnFirerConvert);

		auto it = TrackingMap.find(pLaser);
		if (it != TrackingMap.end())
			UnregisterTracking(pLaser, it->second);

		TrackingMap[pLaser] = data;
		RegisterTracking(pLaser, data);
	}

	TechnoClass* Shooter = nullptr;
	AbstractClass* Target = nullptr;
	int WeaponIndex = 0;
	bool IgnoreShooter = false;
	CoordStruct SavedLocalFLH = CoordStruct::Empty;
	int SavedBurstIndex = 0;
}

// container hooks

// IsLaser this is no longer necessary, but the handling of DiskLaser is more complex, and keeping the CTOR is currently the most cost-effective solution.
DEFINE_HOOK(0x54FE60, LaserDrawClass_CTOR_Update, 0x5)
{
	if (!Phobos::Optimizations::DisableLaserTracking)
	{
		GET(LaserDrawClass*, pLaser, ECX);
		LaserRT::TrackingMap[pLaser] = LaserRT::TrackingData {};
	}
	return 0;
}

DEFINE_HOOK_AGAIN(0x5501D7, LaserDrawClass_DTOR_Tracking, 0x5)
DEFINE_HOOK_AGAIN(0x5500EF, LaserDrawClass_DTOR_Tracking, 0x5)
DEFINE_HOOK_AGAIN(0x550016, LaserDrawClass_DTOR_Tracking, 0x6)
DEFINE_HOOK(0x54FFB0, LaserDrawClass_DTOR_Tracking, 0x7) // LaserDrawClass::DTOR
{
	GET(LaserDrawClass*, pLaser, ECX);

	auto it = LaserRT::TrackingMap.find(pLaser);
	if (it != LaserRT::TrackingMap.end())
	{
		LaserRT::UnregisterTracking(pLaser, it->second);
		LaserRT::TrackingMap.erase(it);
	}

	return 0;
}

void WeaponTypeExt::OnObjectRemoved(ObjectClass* pObject)
{
	auto itShoot = LaserRT::ShooterToLasers.find(pObject);
	if (itShoot != LaserRT::ShooterToLasers.end())
	{
		for (auto pLaser : itShoot->second)
		{
			auto dataIt = LaserRT::TrackingMap.find(pLaser);
			if (dataIt != LaserRT::TrackingMap.end())
			{
				auto& data = dataIt->second;
				if (data.Shooter == pObject)
					data.Shooter = nullptr;
				if (!data.Shooter && !data.Target)
				{
					LaserRT::TrackingMap.erase(dataIt);
				}
			}
		}
		LaserRT::ShooterToLasers.erase(itShoot);
	}
	LaserRT::ShooterToLasers.erase(pObject);

	auto itTarget = LaserRT::TargetToLasers.find(pObject);
	if (itTarget != LaserRT::TargetToLasers.end())
	{
		for (auto pLaser : itTarget->second)
		{
			auto dataIt = LaserRT::TrackingMap.find(pLaser);
			if (dataIt != LaserRT::TrackingMap.end())
			{
				auto& data = dataIt->second;
				if (data.Target == pObject)
					data.Target = nullptr;
				if (!data.Shooter && !data.Target)
					LaserRT::TrackingMap.erase(dataIt);
			}
		}
		LaserRT::TargetToLasers.erase(itTarget);
	}
	LaserRT::TargetToLasers.erase(pObject);
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
	if (LaserRT::TrackingMap.empty())
		return 0;

	GET(LaserDrawClass*, pLaser, ESI);
	const auto it = LaserRT::TrackingMap.find(pLaser);

	if (it == LaserRT::TrackingMap.cend())
		return 0;

	auto& data = it->second;

	if (const auto pShooter = data.Shooter)
	{
		if (data.StopOnFirerConvert && data.OriginalType)
		{
			if (pShooter->GetTechnoType() != data.OriginalType)
				data.Shooter = nullptr;
		}

		if (data.Shooter)
		{
			const int savedBurstIndex = pShooter->CurrentBurstIndex;
			pShooter->CurrentBurstIndex = data.FrozenBurstIndex;
			const CoordStruct worldFLH = pShooter->GetFLH(data.WeaponIndex, data.LocalFLH);
			pShooter->CurrentBurstIndex = savedBurstIndex;

			pLaser->Source = worldFLH + data.SavedOffset;
		}
	}

	if (const auto pTarget = data.Target)
		pLaser->Target = pTarget->GetTargetCoords();

	return 0;
}

#pragma endregion
