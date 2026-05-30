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

	zAdjust += WeaponTypeExt::ExtMap.Find(pWeapon)->LaserZAdjust.Get(RulesExt::Global()->LaserZAdjust);
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

		void Initialize(TechnoClass* pShooter, AbstractClass* pTarget, int weaponIdx, PositionFollow mode, const CoordStruct& initialSource, const CoordStruct& localFLH, int burstIndex)
		{
			if (pShooter)
			{
				if (const auto pBuilding = abstract_cast<BuildingClass*, true>(pShooter))
				{
					if (pBuilding->Type->MaxNumberOccupants > 0)
					{
						if (mode == PositionFollow::Firer)
							mode = PositionFollow::None;
						else if (mode == PositionFollow::All)
							mode = PositionFollow::Target;
					}
				}
			}

			if (pShooter && (mode & PositionFollow::Firer))
			{
				this->Shooter = pShooter;
				this->LocalFLH = localFLH;
				this->FrozenBurstIndex = burstIndex;

				const int savedBurstIndex = pShooter->CurrentBurstIndex;
				pShooter->CurrentBurstIndex = burstIndex;
				CoordStruct worldFLH;
				pShooter->GetFLH(&worldFLH, weaponIdx, localFLH);
				pShooter->CurrentBurstIndex = savedBurstIndex;

				this->SavedOffset = initialSource - worldFLH;
			}

			if (mode & PositionFollow::Target)
				this->Target = abstract_cast<ObjectClass*>(pTarget);

			this->WeaponIndex = weaponIdx;
			this->FollowMode = mode;
		}

		void PointerExpired(void* ptr, bool removed)
		{
			if (!removed)
				return;

			AnnounceInvalidPointer(this->Shooter, ptr);
			AnnounceInvalidPointer(this->Target, ptr);
		}
	};

	std::unordered_map<LaserDrawClass*, TrackingData> TrackingMap;

	void SetLaserTrackingData(LaserDrawClass* pLaser, TechnoClass* pShooter, AbstractClass* pTarget, int weaponIdx, PositionFollow mode, bool ignoreShooter)
	{
		CoordStruct localFLH;
		int burstIndex = 0;
		if (pShooter)
		{
			bool flhFound = false;
			localFLH = TechnoExt::GetBurstFLH(pShooter, weaponIdx, flhFound);
			if (!flhFound)
				localFLH = pShooter->GetWeapon(weaponIdx)->FLH;
			burstIndex = pShooter->CurrentBurstIndex;
		}

		TrackingData data;
		data.Initialize(ignoreShooter ? nullptr : pShooter, pTarget, weaponIdx, mode, pLaser->Source, localFLH, burstIndex);
		TrackingMap[pLaser] = data;
	}

	TechnoClass* Shooter = nullptr;
	AbstractClass* Target = nullptr;
	int WeaponIndex = 0;
	bool IgnoreShooter = false;
	CoordStruct SavedLocalFLH = CoordStruct::Empty;
	int SavedBurstIndex = 0;
}

// container hooks

// IsLaser ctor/dtor/remove hooks
DEFINE_HOOK(0x54FE60, LaserDrawClass_CTOR_Update, 0x5)
{
	GET(LaserDrawClass*, pLaser, ECX);
	LaserRT::TrackingMap[pLaser] = LaserRT::TrackingData {};
	return 0;
}

DEFINE_HOOK_AGAIN(0x5501D7, LaserDrawClass_DTOR_Tracking, 0x5)
DEFINE_HOOK_AGAIN(0x5500EF, LaserDrawClass_DTOR_Tracking, 0x5)
DEFINE_HOOK_AGAIN(0x550016, LaserDrawClass_DTOR_Tracking, 0x6)
DEFINE_HOOK(0x54FFB0, LaserDrawClass_DTOR_Tracking, 0x7) // LaserDrawClass::DTOR
{
	GET(LaserDrawClass*, pLaser, ECX);
	LaserRT::TrackingMap.erase(pLaser);
	return 0;
}

void WeaponTypeExt::LaserTrackingPointerExpired(void* ptr, bool removed)
{
	for (auto& [_, data] : LaserRT::TrackingMap)
		data.PointerExpired(ptr, removed);
}

// hooks

DEFINE_HOOK(0x6FD210, TechnoClass_LaserZap_SetTrackingContext, 0x7)
{
	GET(TechnoClass*, pShooter, ECX);
	GET_STACK(ObjectClass*, pTarget, 0x4);
	GET_STACK(int, weaponIdx, 0x8);

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
	GET(WeaponTypeClass*, pWeapon, ECX);
	const auto pShooter = std::exchange(LaserRT::Shooter, nullptr);
	const auto pTarget = std::exchange(LaserRT::Target, nullptr);
	const int weaponIdx = std::exchange(LaserRT::WeaponIndex, 0);
	const CoordStruct localFLH = std::exchange(LaserRT::SavedLocalFLH, CoordStruct::Empty);
	const int burstIndex = std::exchange(LaserRT::SavedBurstIndex, 0);

	GET(LaserDrawClass*, pLaser, EAX);
	const auto it = LaserRT::TrackingMap.find(pLaser);

	if (it == LaserRT::TrackingMap.cend())
		return 0;

	const auto mode = WeaponTypeExt::ExtMap.Find(pWeapon)->LaserPositionUpdate.Get();

	if (mode == PositionFollow::None)
		return 0;

	it->second.Initialize(pShooter, pTarget, weaponIdx, mode, pLaser->Source, localFLH, burstIndex);
	return 0;
}

static LaserDrawClass* __fastcall Shrapnel_CreateLaser_Wrapper(TechnoClass* pShooter, void*, ObjectClass* pTarget
	, int weaponIdx, WeaponTypeClass* pWeapon, const CoordStruct& sourceCoords)
{
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
	GET(LaserDrawClass*, pLaser, EAX);
	if (!pLaser) return 0;
	const auto it = LaserRT::TrackingMap.find(pLaser);
	if (it == LaserRT::TrackingMap.cend()) return 0;
	GET(DiskLaserClass*, pDiskLaser, ESI);
	const auto pWeapon = pDiskLaser->Weapon;
	if (!pWeapon) return 0;
	const auto mode = WeaponTypeExt::ExtMap.Find(pWeapon)->LaserPositionUpdate.Get();
	if (mode == PositionFollow::None) return 0;
	if (pLaser->Source == pLaser->Target) return 0;

	LaserRT::SetLaserTrackingData(pLaser, pDiskLaser->Owner, pDiskLaser->Target, 0, mode, false);
	return 0;
}

// Per‑frame coordinate update
DEFINE_HOOK(0x550173, LaserDrawClass_Update_Tracking, 0x6)
{
	GET(LaserDrawClass*, pLaser, ESI);
	const auto it = LaserRT::TrackingMap.find(pLaser);

	if (it == LaserRT::TrackingMap.cend())
		return 0;

	const auto& data = it->second;

	if (const auto pShooter = data.Shooter)
	{
		const int savedBurstIndex = pShooter->CurrentBurstIndex;
		pShooter->CurrentBurstIndex = data.FrozenBurstIndex;
		CoordStruct worldFLH;
		pShooter->GetFLH(&worldFLH, data.WeaponIndex, data.LocalFLH);
		pShooter->CurrentBurstIndex = savedBurstIndex;

		pLaser->Source = worldFLH + data.SavedOffset;
	}

	if (const auto pTarget = data.Target)
		pLaser->Target = pTarget->GetTargetCoords();

	return 0;
}

#pragma endregion
