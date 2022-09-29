#include "Body.h"

#include <ScenarioClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/VoxelAnimType/Body.h>
#include <Ext/WarheadType/Body.h>

DEFINE_HOOK(0x74A70E, VoxelAnimClass_AI_Additional, 0xC)
{
	GET(VoxelAnimClass* const, pThis, EBX);

	//auto pTypeExt = VoxelAnimTypeExt::ExtMap.Find(pThis->Type);
	auto pThisExt = VoxelAnimExt::ExtMap.Find(pThis);

	if (!pThisExt->LaserTrails.empty())
	{
		CoordStruct location = pThis->GetCoords();
		CoordStruct drawnCoords = location;

		for (auto const& trail : pThisExt->LaserTrails)
		{
			if (!trail->LastLocation.isset())
				trail->LastLocation = location;

			trail->Visible = pThis->IsVisible;
			trail->Update(drawnCoords);

		}
	}

	return 0;
}

DEFINE_HOOK(0x74A027, VoxelAnimClass_AI_Expired, 0x6)
{
	enum { SkipGameCode = 0x74A22A };

	GET(VoxelAnimClass* const, pThis, EBX);
	GET(int, flag, EAX);

	bool heightFlag = flag & 0xFF;

	if (!pThis || !pThis->Type)
		return SkipGameCode;

	CoordStruct nLocation = pThis->Location;
	auto const pOwner = pThis->OwnerHouse;
	auto const pTypeExt = VoxelAnimTypeExt::ExtMap.Find(pThis->Type);
	AnimTypeClass* pSplashAnim = nullptr;

	if (pThis->GetCell()->LandType != LandType::Water || heightFlag || pTypeExt->ExplodeOnWater)
	{
		if (auto const pWarhead = pThis->Type->Warhead)
		{
			auto const nDamage = pThis->Type->Damage;

			if (pTypeExt->Warhead_Detonate)
			{
				WarheadTypeExt::DetonateAt(pWarhead, nLocation, nullptr, nDamage);
			}
			else
			{
				MapClass::DamageArea(nLocation, nDamage, nullptr, pWarhead, pWarhead->Tiberium, pOwner);
				MapClass::FlashbangWarheadAt(nDamage, pWarhead, nLocation);
			}
		}

		if (auto const pExpireAnim = pThis->Type->ExpireAnim)
		{
			if (auto pAnim = GameCreate<AnimClass>(pExpireAnim, nLocation, 0, 1, 0x2600u, 0, 0))
				AnimExt::SetAnimOwnerHouseKind(pAnim, pOwner, nullptr, false);
		}
	}
	else
	{
		TypeList<AnimTypeClass*> defaultSplashAnims;

		if (!pThis->Type->IsMeteor)
		{
			defaultSplashAnims = TypeList<AnimTypeClass*>();
			defaultSplashAnims.AddItem(RulesClass::Instance->Wake);
		}
		else
		{
			defaultSplashAnims = RulesClass::Instance->SplashList;
		}

		auto const splash = pTypeExt->SplashAnims.GetElements(defaultSplashAnims);

		if (splash.size() > 0)
		{
			auto nIndexR = (splash.size() - 1);
			auto nIndex = pTypeExt->SplashAnims_PickRandom ?
				ScenarioClass::Instance->Random.RandomRanged(0, nIndexR) : nIndexR;

			pSplashAnim = splash.at(nIndex);
		}
	}

	if (pSplashAnim)
	{
		if (auto const pSplashAnimCreated = GameCreate<AnimClass>(pSplashAnim, nLocation, 0, 1, 0x600u, false))
			AnimExt::SetAnimOwnerHouseKind(pSplashAnimCreated, pOwner, nullptr, false);
	}

	return SkipGameCode;
}
