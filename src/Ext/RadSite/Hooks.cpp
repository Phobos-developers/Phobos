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

/*
	Custom Radiations
	Worked out from old uncommented Ares RadSite Hook , adding some more hook
	and rewriting some in order to make this working perfecly
	Credit : Ares Team , for unused/uncommented source of Hook.RadSite
						,RulesData_LoadBeforeTypeData Hook
			 Alex-B : GetRadSiteAt ,Helper that used at FootClass_AI & BuildingClass_AI
					  Radiate , Uncommented
			 me(Otamaa) adding some more stuffs and rewriting hook that cause crash
	TODO : -Testings
	// Do this Ares Hook will cause a problem ?
	//4DA584 = FootClass_AI_RadImmune, 7
*/

DEFINE_HOOK(0x469150, BulletClass_Detonate_ApplyRadiation, 0x5)
{
	GET(BulletClass* const, pThis, ESI);
	GET_BASE(CoordStruct const*, pCoords, 0x8);

	auto const pWeapon = pThis->GetWeaponType();

	if (pWeapon && pWeapon->RadLevel > 0)
	{
		auto const pExt = BulletExt::ExtMap.Find(pThis);
		auto const pWH = pThis->WH;
		auto const cell = CellClass::Coord2Cell(*pCoords);
		auto const spread = static_cast<int>(pWH->CellSpread);

		pExt->ApplyRadiationToCell(cell, spread, pWeapon->RadLevel);
	}

	return 0x46920B;
}

/*
// hack it here so we can use this globally if needed
// *Prototype*
//able to manually set the RadType instead rely on Weapon RadType
//Break InfantryClass_AIDeployment_CheckRad atm , will fix later
DEFINE_HOOK(0x46ADE0, BulletClass_ApplyRadiation, 0x5)
{
	GET_STACK(CellStruct, location, 0x4);
	GET_STACK(int, spread, 0x8);
	GET_STACK(int, amount, 0xC);

	auto const& Instances = RadSiteExt::Array;
	auto const pType = RadTypeClass::FindOrAllocate("Radiation");

	if (Instances.Count > 0)
	{
		auto const it = std::find_if(Instances.begin(), Instances.end(),
			[=](RadSiteExt::ExtData* const pSite) // Lambda
		{// find
			return
				pSite->Type == pType &&
				pSite->OwnerObject()->BaseCell == location &&
				spread == pSite->OwnerObject()->Spread;
		});

		if (it == Instances.end())
		{
			RadSiteExt::CreateInstance(location, spread, amount, pType, nullptr);
		}
		else
		{
			auto pRadExt = *it;
			auto pRadSite = pRadExt->OwnerObject();

			if (pRadSite->GetRadLevel() + amount > pType->GetLevelMax())
			{
				amount = pType->GetLevelMax() - pRadSite->GetRadLevel();
			}

			// Handle It
			pRadExt->Add(amount);
		}
	}
	else
	{
		RadSiteExt::CreateInstance(location, spread, amount, pType, nullptr);
	}

	return 0x46AE5E;
}
*/

// Fix for desolator unable to fire his deploy weapon when cloaked 
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
		auto const wh = pWeapon->Warhead;
		auto currentCoord = pInf->GetCell()->MapCoords;

		auto const it = std::find_if(RadSiteExt::Array.begin(), RadSiteExt::Array.end(),
			[=](RadSiteExt::ExtData* const pSite)
		{
			return
				pSite->Type == pRadType &&
				pSite->OwnerObject()->BaseCell == currentCoord &&
				pSite->OwnerObject()->Spread == static_cast<int>(wh->CellSpread)
				;
		});

		if (it != RadSiteExt::Array.end())
		{
			auto pRadExt = *it;
			auto pRadSite = pRadExt->OwnerObject();
			radLevel = pRadSite->GetRadLevel();
		}
	}

	if (pInf->CloakState == CloakState::Cloaked)
	{
		pInf->Uncloak(false);
		pInf->Cloakable = false;
		pInf->NeedsRedraw = true;
		auto pExt = TechnoExt::ExtMap.Find(pInf);
		pExt->WasCloaked = true;
	}

	return (!radLevel || (radLevel < weaponRadLevel / 3)) ?
		0x5213F4 : 0x521484;
}

// Too OP, be aware
DEFINE_HOOK(0x43FB23, BuildingClass_AI, 0x5)
{
	GET(BuildingClass* const, pBuilding, ECX);

	if (pBuilding->IsIronCurtained() || pBuilding->Type->ImmuneToRadiation || pBuilding->InLimbo || pBuilding->BeingWarpedOut)
		return 0;

	auto const buildingCoords = pBuilding->GetMapCoords();
	for (auto pFoundation = pBuilding->GetFoundationData(false); *pFoundation != CellStruct { 0x7FFF, 0x7FFF }; ++pFoundation)
	{
		CellStruct CurrentCoord = buildingCoords + *pFoundation;

		for (auto pRadExt : RadSiteExt::Array)
		{
			RadSiteClass* pRadSite = pRadExt->OwnerObject();
			RadTypeClass* pType = pRadExt->Type;

			// Check the distance, if not in range, just skip this one
			double orDistance = pRadSite->BaseCell.DistanceFrom(CurrentCoord);
			if (pRadSite->Spread < orDistance - 0.5)
				continue;

			int delay = pType->GetBuildingApplicationDelay();
			if ((delay == 0) || (Unsorted::CurrentFrame % delay != 0))
				continue;

			if (pRadExt->GetRadLevelAt(CurrentCoord) <= 0.0 || !pType->GetWarhead())
				continue;

			auto wh = pType->GetWarhead();
			auto absolute = wh->WallAbsoluteDestroyer;
			bool ignore = pBuilding->Type->Wall && absolute;
			auto damage = static_cast<int>((pRadExt->GetRadLevelAt(CurrentCoord) / 2) * pType->GetLevelFactor());

			if (pBuilding->IsAlive) // simple fix for previous issues
				pBuilding->ReceiveDamage(&damage, static_cast<int>(orDistance), wh, nullptr, ignore, absolute, pRadExt->RadHouse.Get());
		}
	}

	return 0;
}

// be aware that this function is updated every frame 
// putting debug log here can become mess because it gonna print bunch of debug line
DEFINE_HOOK(0x4DA554, FootClass_AI_RadSiteClass, 0x5)
{
	GET(FootClass* const, pFoot, ESI);
	auto state = static_cast<DamageState>(R->AL());

	if (!pFoot->IsIronCurtained() && !pFoot->GetTechnoType()->ImmuneToRadiation && !pFoot->InLimbo && !pFoot->IsInAir())
	{
		CellStruct CurrentCoord = pFoot->GetCell()->MapCoords;

		// Loop for each different radiation stored in the RadSites container
		for (auto const pRadExt : RadSiteExt::Array)
		{
			RadSiteClass* pRadSite = pRadExt->OwnerObject();

			// Check the distance, if not in range, just skip this one
			double orDistance = pRadSite->BaseCell.DistanceFrom(CurrentCoord);
			if (pRadSite->Spread < orDistance - 0.7)
				continue;

			RadTypeClass* pType = pRadExt->Type;
			int RadApplicationDelay = pType->GetApplicationDelay();
			if ((RadApplicationDelay == 0) || (Unsorted::CurrentFrame % RadApplicationDelay != 0))
				continue;

			// for more precise dmg calculation
			double RadLevel = pRadExt->GetRadLevelAt(CurrentCoord);
			if (RadLevel <= 0.0 || !pType->GetWarhead())
				continue;

			int damage = static_cast<int>(RadLevel * pType->GetLevelFactor());
			int distance = static_cast<int>(orDistance);
			auto wh = pType->GetWarhead();
			auto absolute = wh->WallAbsoluteDestroyer;

			if (pFoot->IsAlive || !pFoot->IsSinking)// simple fix for previous issues
				state = pFoot->ReceiveDamage(&damage, distance, wh, nullptr, false, absolute, pRadExt->RadHouse.Get());
		}
	}

	R->EAX<DamageState>(state);

	return pFoot->IsAlive ? 0x4DA63B : 0x4DAF00;
}

DEFINE_HOOK(0x65B593, RadSiteClass_Activate_Delay, 0x6)
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

#define GET_RADSITE(reg, value)\
	GET(RadSiteClass* const, pThis, reg);\
	RadSiteExt::ExtData* pExt = RadSiteExt::ExtMap.Find(pThis);\
	auto output = pExt->Type->## value ##;

DEFINE_HOOK(0x65B5CE, RadSiteClass_Activate_Color, 0x6)
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

	R->ESI(output);

	__asm xor  edx, edx; // fixing integer overflow crash
	__asm idiv esi;

	return 0x65BB6D;
}