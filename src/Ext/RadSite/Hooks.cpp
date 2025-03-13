#include "Body.h"

#include <BulletClass.h>
#include <HouseClass.h>
#include <InfantryClass.h>
#include <WarheadTypeClass.h>
#include <ScenarioClass.h>

#include <Ext/BuildingType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/Rules/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/Cell/Body.h>

#include <Utilities/Macro.h>
/*
	Custom Radiations
	Worked out from old uncommented Ares RadSite Hook , adding some more hook
	and rewriting some in order to make this working perfecly
	Credit : Ares Team , for unused/uncommented source of Hook.RadSite
						,RulesData_LoadBeforeTypeData Hook
			Alex-B : GetRadSiteAt ,Helper that used at FootClass_AI & BuildingClass_AI
					Radiate , Uncommented
			me(Otamaa) adding some more stuffs and rewriting hook that cause crash

*/

DEFINE_HOOK(0x469150, BulletClass_Detonate_ApplyRadiation, 0x5)
{
	GET(BulletClass* const, pThis, ESI);
	GET_BASE(CoordStruct const*, pCoords, 0x8);

	auto const pWeapon = pThis->GetWeaponType();

	if (pWeapon && pWeapon->RadLevel > 0 && MapClass::Instance->IsWithinUsableArea((*pCoords)))
	{
		auto const pExt = BulletExt::ExtMap.Find(pThis);
		auto const pWH = pThis->WH;
		auto const cell = CellClass::Coord2Cell(*pCoords);
		auto const spread = Game::F2I(pWH->CellSpread);

		pExt->ApplyRadiationToCell(cell, spread, pWeapon->RadLevel);
	}

	return 0x46920B;
}
#ifndef __clang__
//unused function , safeguard
DEFINE_HOOK(0x46ADE0, BulletClass_ApplyRadiation_Unused, 0x5)
{
	Debug::Log(__FUNCTION__ " called ! , You are not supposed to be here!\n");
	return 0x46AE5E;
}
#endif
// Fix for desolator
DEFINE_HOOK(0x5213B4, InfantryClass_AIDeployment_CheckRad, 0x7)
{
	enum { FireCheck = 0x5213F4, SetMissionRate = 0x521484 };

	GET(InfantryClass*, pInfantry, ESI);
	GET(int, weaponRadLevel, EBX);
	auto const pCell = pInfantry->GetCell();
	auto const pCellExt = CellExt::ExtMap.Find(pCell);
	int radLevel = 0;

	if (!pCellExt->RadSites.empty())
	{
		if (auto const pWeapon = pInfantry->GetDeployWeapon()->WeaponType)
		{
			auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
			auto const pRadType = pWeaponExt->RadType;
			auto const warhead = pWeapon->Warhead;

			for (const auto radSite : pCellExt->RadSites)
			{
				if (radSite->Spread == static_cast<int>(warhead->CellSpread) && RadSiteExt::ExtMap.Find(radSite)->Type == pRadType)
				{
					radLevel = radSite->GetRadLevel();
					break;
				}
			}
		}
	}

	return (!radLevel || (radLevel < weaponRadLevel / 3)) ?
		FireCheck : SetMissionRate;
}

// Fix for desolator unable to fire his deploy weapon when cloaked
DEFINE_HOOK(0x521478, InfantryClass_AIDeployment_FireNotOKCloakFix, 0x4)
{
	GET(InfantryClass* const, pThis, ESI);

	auto const pWeapon = pThis->GetDeployWeapon()->WeaponType;
	AbstractClass* pTarget = nullptr; //default WWP nullptr

	if (pWeapon
		&& pWeapon->DecloakToFire
		&& (pThis->CloakState == CloakState::Cloaked || pThis->CloakState == CloakState::Cloaking))
	{
		// FYI this are hack to immedietely stop the Cloaking
		// since this function is always failing to decloak and set target when cell is occupied
		// something is wrong somewhere  # Otamaa
		auto nDeployFrame = pThis->Type->Sequence->GetSequence(Sequence::DeployedFire).CountFrames;
		pThis->CloakDelayTimer.Start(nDeployFrame);

		pTarget = MapClass::Instance->TryGetCellAt(pThis->GetCoords());
	}

	pThis->SetTarget(pTarget); //Here we go

	return 0x521484;
}

// Too OP, be aware
DEFINE_HOOK(0x43FB23, BuildingClass_AI_Radiation, 0x5)
{
	GET(BuildingClass* const, pBuilding, ECX);

	if (pBuilding->Type->ImmuneToRadiation || pBuilding->InLimbo || pBuilding->BeingWarpedOut || pBuilding->TemporalTargetingMe)
		return 0;

	int radDelay = RulesExt::Global()->RadApplicationDelay_Building;

	if (RulesExt::Global()->UseGlobalRadApplicationDelay &&
		(radDelay == 0 || Unsorted::CurrentFrame % radDelay != 0))
	{
		return 0;
	}

	auto const buildingCoords = pBuilding->GetMapCoords();
	std::unordered_map<RadSiteClass*, int> damageCounts;

	for (auto pFoundation = pBuilding->GetFoundationData(false); *pFoundation != CellStruct { 0x7FFF, 0x7FFF }; ++pFoundation)
	{
		CellStruct nCurrentCoord = buildingCoords + *pFoundation;
		const auto pCell = MapClass::Instance->TryGetCellAt(nCurrentCoord);

		if (!pCell)
			continue;

		const auto pCellExt = CellExt::ExtMap.Find(pCell);

		for (const auto& [pRadSite, radLevel] : pCellExt->RadLevels)
		{
			if (radLevel <= 0)
				continue;

			auto const pRadExt = RadSiteExt::ExtMap.Find(pRadSite);
			RadTypeClass* pType = pRadExt->Type;
			int maxDamageCount = pType->GetBuildingDamageMaxCount();

			if (maxDamageCount > 0 && damageCounts[pRadSite] >= maxDamageCount)
				continue;

			if (!pType->GetWarhead())
				continue;

			if (!RulesExt::Global()->UseGlobalRadApplicationDelay)
			{
				int delay = pType->GetBuildingApplicationDelay();

				if ((delay == 0) || (Unsorted::CurrentFrame % delay != 0))
					continue;
			}

			if (pBuilding->IsAlive) // simple fix for previous issues
			{
				int damage = Game::F2I(radLevel * pType->GetLevelFactor());

				if (maxDamageCount > 0)
					damageCounts[pRadSite]++;

				if (!pRadExt->ApplyRadiationDamage(pBuilding, damage, 0))
					break;
			}
		}
	}

	return 0;
}

// skip Frame % RadApplicationDelay
DEFINE_JUMP(LJMP, 0x4DA554, 0x4DA56E);

// Hook Adjusted to support Ares RadImmune Ability check
DEFINE_HOOK(0x4DA59F, FootClass_AI_Radiation, 0x5)
{
	enum { Continue = 0x4DA63B, ReturnFromFunction = 0x4DAF00 };

	GET(FootClass* const, pFoot, ESI);

	if (pFoot->IsInPlayfield && !pFoot->TemporalTargetingMe &&
		(!RulesExt::Global()->UseGlobalRadApplicationDelay || Unsorted::CurrentFrame % RulesClass::Instance->RadApplicationDelay == 0))
	{
		if (auto const pCell = pFoot->GetCell())
		{
			auto const pCellExt = CellExt::ExtMap.Find(pCell);

			for (const auto& [pRadSite, radLevel] : pCellExt->RadLevels)
			{
				if (radLevel <= 0)
					continue;

				auto const pRadExt = RadSiteExt::ExtMap.Find(pRadSite);
				RadTypeClass* pType = pRadExt->Type;

				if (!pType->GetWarhead())
					continue;

				if (!RulesExt::Global()->UseGlobalRadApplicationDelay)
				{
					int delay = pType->GetApplicationDelay();

					if ((delay == 0) || (Unsorted::CurrentFrame % delay != 0))
						continue;
				}

				if (pFoot->IsAlive || !pFoot->IsSinking)
				{
					int damage = Game::F2I(radLevel * pType->GetLevelFactor());

					if (!pRadExt->ApplyRadiationDamage(pFoot, damage, 0))
						break;
				}
			}
		}
	}

	return pFoot->IsAlive ? Continue : ReturnFromFunction;
}

#define GET_RADSITE(reg, value)\
	GET(RadSiteClass* const, pThis, reg);\
	RadSiteExt::ExtData* pExt = RadSiteExt::ExtMap.Find(pThis);\
	auto output = pExt->Type-> value ;

/*
//All part of 0x65B580 Hooks is here
DEFINE_HOOK(65B593, RadSiteClass_Activate_Delay, 6)
{
	GET(RadSiteClass* const, pThis, ECX);
	auto const pExt = RadSiteExt::ExtMap.Find(pThis);

	auto const currentLevel = pThis->GetRadLevel();
	auto levelDelay = pExt->Type->GetLevelDelay();
	auto lightDelay = pExt->Type->GetLightDelay();

	if (currentLevel < levelDelay)
	{
		levelDelay = currentLevel;
		lightDelay = currentLevel;
	}

	R->ECX(levelDelay);
	R->EAX(lightDelay);

	return 0x65B59F;
}

DEFINE_HOOK(65B5CE, RadSiteClass_Activate_Color, 6)
{
	GET_RADSITE(ESI, GetColor());

	R->EAX(0);
	R->EDX(0);
	R->EBX(0);

	R->DL(output.G);
	R->EBP(R->EDX());

	R->BL(output.B);
	R->AL(output.R);

	// point out the missing register - Otamaa
	R->EDI(pThis);

	return 0x65B604;
}

DEFINE_HOOK(0x65B63E, RadSiteClass_Activate_LightFactor, 0x6)
{
	GET_RADSITE(ESI, GetLightFactor());

	__asm fmul output;

	return 0x65B644;
}

DEFINE_HOOK_AGAIN(0x65B6A0, RadSiteClass_Activate_TintFactor, 0x6)
DEFINE_HOOK_AGAIN(0x65B6CA, RadSiteClass_Activate_TintFactor, 0x6)
DEFINE_HOOK(0x65B6F2, RadSiteClass_Activate_TintFactor, 0x6)
{
	GET_RADSITE(ESI, GetTintFactor());

	__asm fmul output;

	return R->Origin() + 6;
}
*/

DEFINE_HOOK(0x65B843, RadSiteClass_AI_LevelDelay, 0x6)
{
	GET_RADSITE(ESI, GetLevelDelay());

	R->ECX(output);

	return 0x65B849;
}

DEFINE_HOOK(0x65B8B9, RadSiteClass_AI_LightDelay, 0x6)
{
	GET_RADSITE(ESI, GetLightDelay());

	R->ECX(output);

	return 0x65B8BF;
}

// Additional Hook below
DEFINE_HOOK(0x65BB67, RadSite_Deactivate, 0x6)
{
	GET_RADSITE(ECX, GetLevelDelay());
	GET(int, val, EAX);

	R->EAX(val / output);
	R->EDX(val % output);

	return 0x65BB6D;
}

DEFINE_HOOK(0x65BAC1, RadSiteClass_Radiate_Increase, 0x8)
{
	enum { SkipGameCode = 0x65BB11 };

	GET(RadSiteClass*, pThis, EDX);
	GET(int, distance, EAX);
	const int max = pThis->SpreadInLeptons;

	if (distance > max)
		return SkipGameCode;

	LEA_STACK(CellStruct*, cell, STACK_OFFSET(0x60, -0x4C));
	const auto pCell = MapClass::Instance->TryGetCellAt(*cell);

	if (!pCell)
		return SkipGameCode;

	const auto pCellExt = CellExt::ExtMap.Find(pCell);
	auto& radLevels = pCellExt->RadLevels;

	const auto it = std::find_if(radLevels.begin(), radLevels.end(), [pThis](std::pair<RadSiteClass*, int> const& item) { return item.first == pThis; });
	const int amount = Game::F2I(static_cast<double>(max - distance) / max * pThis->RadLevel);

	if (it != radLevels.end())
		it->Level += amount;
	else
		radLevels.emplace_back(pThis, amount);

	return SkipGameCode;
}

DEFINE_HOOK(0x65BC6E, RadSiteClass_Deactivate_Decrease, 0x6)
{
	enum { SkipGameCode = 0x65BCBD };

	GET(RadSiteClass*, pThis, EDX);
	GET(int, distance, EAX);
	const int max = pThis->SpreadInLeptons;

	if (distance > max)
		return SkipGameCode;

	LEA_STACK(CellStruct*, cell, STACK_OFFSET(0x70, -0x5C));
	const auto pCell = MapClass::Instance->TryGetCellAt(*cell);

	if (!pCell)
		return SkipGameCode;

	const auto pCellExt = CellExt::ExtMap.Find(pCell);
	auto& radLevels = pCellExt->RadLevels;

	const auto it = std::find_if(radLevels.begin(), radLevels.end(), [pThis](std::pair<RadSiteClass*, int> const& item) { return item.first == pThis; });

	if (it != radLevels.end())
	{
		const int amount = Game::F2I(static_cast<double>(max - distance) / max * pThis->RadLevel / pThis->LevelSteps);
		it->Level -= amount;
	}

	return SkipGameCode;
}

DEFINE_HOOK(0x65BE01, RadSiteClass_DecreaseRadiation_Decrease, 0x6)
{
	enum { SkipGameCode = 0x65BE4C };

	GET(RadSiteClass*, pThis, EDX);
	GET(int, distance, EAX);
	const int max = pThis->SpreadInLeptons;

	if (distance > max)
		return SkipGameCode;

	LEA_STACK(CellStruct*, cell, STACK_OFFSET(0x60, -0x50));
	const auto pCell = MapClass::Instance->TryGetCellAt(*cell);

	if (!pCell)
		return SkipGameCode;

	const auto pCellExt = CellExt::ExtMap.Find(pCell);
	auto& radLevels = pCellExt->RadLevels;

	const auto it = std::find_if(radLevels.begin(), radLevels.end(), [pThis](std::pair<RadSiteClass*, int> const& item) { return item.first == pThis; });

	if (it != radLevels.end())
	{
		const int amount = Game::F2I(static_cast<double>(max - distance) / max * pThis->RadLevel / pThis->LevelSteps);
		it->Level -= amount;
	}

	return SkipGameCode;
}
