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

//unused function , safeguard
DEFINE_HOOK(0x46ADE0, BulletClass_ApplyRadiation_UnUsed, 0x5)
{
	Debug::Log("[" __FUNCTION__ "] Called ! , You are not suppose to be here ! \n");
	return 0x46AE5E;
}

// Fix for desolator
DEFINE_HOOK(0x5213E3, InfantryClass_AIDeployment_CheckRad, 0x4)
{
	GET(InfantryClass*, pInf, ESI);
	GET(int, weaponRadLevel, EBX);

	auto const pWeapon = pInf->GetDeployWeapon()->WeaponType;

	int radLevel = 0;
	if (RadSiteExt::Array.Count > 0 && pWeapon)
	{
		auto const pWeaponExt = WeaponTypeExt::ExtMap.FindOrAllocate(pWeapon);
		auto const pRadType = pWeaponExt->RadType;
		auto const warhead = pWeapon->Warhead;
		auto currentCoord = pInf->GetCell()->MapCoords;

		auto const it = std::find_if(RadSiteExt::Array.begin(), RadSiteExt::Array.end(),
			[=](RadSiteExt::ExtData* const pSite)
			{
				return
					pSite->Type == pRadType &&
					pSite->OwnerObject()->BaseCell == currentCoord &&
					pSite->OwnerObject()->Spread == Game::F2I(warhead->CellSpread)
					;
			});

		if (it != RadSiteExt::Array.end())
		{
			auto pRadExt = *it;
			auto pRadSite = pRadExt->OwnerObject();
			radLevel = pRadSite->GetRadLevel();
		}
	}

	return (!radLevel || (radLevel < weaponRadLevel / 3)) ?
		0x5213F4 : 0x521484;
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

	if (pBuilding->IsIronCurtained() || pBuilding->Type->ImmuneToRadiation || pBuilding->InLimbo || pBuilding->BeingWarpedOut || pBuilding->TemporalTargetingMe)
		return 0;

	int radDelay = RulesExt::Global()->RadApplicationDelay_Building;

	if (RulesExt::Global()->UseGlobalRadApplicationDelay &&
		(radDelay == 0 || Unsorted::CurrentFrame % radDelay != 0))
	{
		return 0;
	}

	auto const buildingCoords = pBuilding->GetMapCoords();
	for (auto pFoundation = pBuilding->GetFoundationData(false); *pFoundation != CellStruct { 0x7FFF, 0x7FFF }; ++pFoundation)
	{
		CellStruct nCurrentCoord = buildingCoords + *pFoundation;

		for (auto& pRadExt : RadSiteExt::Array)
		{
			RadSiteClass* pRadSite = pRadExt->OwnerObject();
			RadTypeClass* pType = pRadExt->Type;

			// Check the distance, if not in range, just skip this one
			double orDistance = pRadSite->BaseCell.DistanceFrom(nCurrentCoord);

			if (pRadSite->Spread < orDistance - 0.5)
				continue;

			if (!RulesExt::Global()->UseGlobalRadApplicationDelay)
			{
				int delay = pType->GetBuildingApplicationDelay();

				if ((delay == 0) || (Unsorted::CurrentFrame % delay != 0))
					continue;
			}

			if (RadSiteExt::GetRadLevelAt(pRadSite, nCurrentCoord) <= 0.0 || !pType->GetWarhead())
				continue;

			auto damage = Game::F2I((RadSiteExt::GetRadLevelAt(pRadSite, nCurrentCoord) / 2) * pType->GetLevelFactor());

			if (pBuilding->IsAlive) // simple fix for previous issues
			{
				if (!pRadExt->ApplyRadiationDamage(pBuilding, damage, Game::F2I(orDistance)))
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
	GET(FootClass* const, pFoot, ESI);

	int radDelay = RulesClass::Instance->RadApplicationDelay;

	if (!pFoot->IsIronCurtained() && pFoot->IsInPlayfield && !pFoot->TemporalTargetingMe &&
		(!RulesExt::Global()->UseGlobalRadApplicationDelay || Unsorted::CurrentFrame % radDelay == 0))
	{
		CellStruct CurrentCoord = pFoot->GetCell()->MapCoords;

		// Loop for each different radiation stored in the RadSites container
		for (auto& pRadExt : RadSiteExt::Array)
		{
			RadSiteClass* pRadSite = pRadExt->OwnerObject();

			// Check the distance, if not in range, just skip this one
			double orDistance = pRadSite->BaseCell.DistanceFrom(CurrentCoord);

			if (pRadSite->Spread < orDistance - 0.7)
				continue;

			RadTypeClass* pType = pRadExt->Type;

			if (!RulesExt::Global()->UseGlobalRadApplicationDelay)
			{
				int delay = pType->GetApplicationDelay();

				if ((delay == 0) || (Unsorted::CurrentFrame % delay != 0))
					continue;
			}

			// for more precise dmg calculation
			double nRadLevel = RadSiteExt::GetRadLevelAt(pRadSite, CurrentCoord);

			if (nRadLevel <= 0.0 || !pType->GetWarhead())
				continue;

			int damage = Game::F2I(nRadLevel * pType->GetLevelFactor());

			if (pFoot->IsAlive || !pFoot->IsSinking)
			{
				if (!pRadExt->ApplyRadiationDamage(pFoot, damage, Game::F2I(orDistance)))
					break;
			}
		}
	}

	return pFoot->IsAlive ? 0x4DA63B : 0x4DAF00;
}

#define GET_RADSITE(reg, value)\
	GET(RadSiteClass* const, pThis, reg);\
	RadSiteExt::ExtData* pExt = RadSiteExt::ExtMap.Find(pThis);\
	auto output = pExt->Type->## value ##;

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
