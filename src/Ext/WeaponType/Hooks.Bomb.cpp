#include "Body.h"

#include <Utilities/AresFunctions.h>

DEFINE_HOOK(0x469363, BulletClass_DetonateAt_PlantIvanBombs, 0x6)
{
	enum { SkipGameCode = 0x469AA4 };

	GET(BulletClass*, pBullet, ESI);
	GET(TechnoClass*, pTarget, EAX);
	BombListClass::Instance.Plant(pBullet->Owner, pTarget);

	if (const auto pBomb = pTarget->AttachedBomb)
	{
		if (const auto pWeapon = pBullet->GetWeaponType())
			WeaponTypeExt::BombExtMap[pBomb] = WeaponTypeExt::Fetch(pWeapon);
	}

	return SkipGameCode;
}

DEFINE_HOOK(0x4393F2, BombClass_SDDTOR, 5)
{
	if (!AresFunctions::BombExtMap)
	{
		GET(BombClass*, Bomb, ECX);
		WeaponTypeExt::BombExtMap.erase(Bomb);
	}

	return 0;
}

DEFINE_HOOK(0x438D44, BombListClass_AI_Visibility, 0x5)
{
	enum { SkipGameCode = 0x438E2B };

	GET(BombListClass*, pBombList, EDI);
	GET(BombClass*, pBomb, EBX);
	AffectedHouse visibility = AffectedHouse::None;

	if (const auto pWeaponExt = WeaponTypeExt::GetBombExtData(pBomb))
		visibility = pWeaponExt->IvanBomb_Visibility.Get(RulesExt::Global()->IvanBomb_Visibility);
	else
		visibility = RulesExt::Global()->IvanBomb_Visibility;

	const auto pCurrent = HouseClass::CurrentPlayer;
	bool visible = false;

	if (EnumFunctions::CanTargetHouse(visibility, pBomb->OwnerHouse, pCurrent)
		|| std::ranges::any_of(pBombList->Detectors, [=](TechnoClass* pDetector)
		{
			if (!EnumFunctions::CanTargetHouse(visibility, pDetector->Owner, pCurrent))
				return false;

			const int sight = pDetector->GetTechnoType()->BombSight * Unsorted::LeptonsPerCell;
			return pDetector->GetCoords().DistanceFromSquared(pBomb->Target->GetCoords()) <= static_cast<double>(sight) * sight;
		})
	)
	{
		visible = true;
	}

	R->AL(visible);
	return SkipGameCode;
}
