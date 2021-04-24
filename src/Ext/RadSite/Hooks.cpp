#include "Body.h"
#include <BulletClass.h>
#include <HouseClass.h>
#include <InfantryClass.h>
#include <WarheadTypeClass.h>
#include <ScenarioClass.h>

#include "../BuildingType/Body.h"
#include "../Bullet/Body.h"
#include "../Rules/Body.h"
#include "../Techno/Body.h"

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

DEFINE_HOOK(469150, BulletClass_Detonate_ApplyRadiation, 5)
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
DEFINE_HOOK(46ADE0, BulletClass_ApplyRadiation, 5)
{
	GET_STACK(CellStruct, location, 0x4);
	GET_STACK(int, spread, 0x8);
	GET_STACK(int, amount, 0xC);

	auto const& Instances = RadSiteExt::RadSiteInstance;
	auto const pType = RadType::FindOrAllocate("Radiation");

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

//Desolator cannot fire his deploy weapon when cloaked 
DEFINE_HOOK(5213E3, InfantryClass_AIDeployment_CheckRad, 4)
{
	GET(InfantryClass*, D, ESI);
	GET(int, WeaponRadLevel, EBX);

	auto const& Instances = RadSiteExt::RadSiteInstance;
	auto const pWeapon = D->GetDeployWeapon()->WeaponType;

	int RadLevel = 0;
	if (Instances.Count > 0 && pWeapon)
	{
		auto const pWeaponExt = WeaponTypeExt::ExtMap.FindOrAllocate(pWeapon);
		auto const pRadType = pWeaponExt->RadType;
		auto const WH = pWeapon->Warhead;
		auto CurrentCoord = D->GetCell()->MapCoords;

		auto const it = std::find_if(Instances.begin(), Instances.end(),
			[=](RadSiteExt::ExtData* const pSite) // Lambda
		{// find 
			return
				pSite->Type == pRadType &&
				pSite->OwnerObject()->BaseCell == CurrentCoord &&
				pSite->OwnerObject()->Spread == static_cast<int>(WH->CellSpread)
				;
		});

		if (it != Instances.end())
		{
			auto pRadExt = *it;
			auto pRadSite = pRadExt->OwnerObject();
			RadLevel = pRadSite->GetRadLevel();
		}
	}

	if (D->CloakState == CloakState::Cloaked)
	{
		D->Uncloak(false);
		D->Cloakable = false;
		D->NeedsRedraw = true;
		auto Text = TechnoExt::ExtMap.Find(D);
		Text->WasCloaked = true;
	}

	return (RadLevel < WeaponRadLevel) ? 0x5213F4 : 0x521484;
}

// Too OP, be aware
DEFINE_HOOK(43FB23, BuildingClass_AI, 5)
{
	GET(BuildingClass* const, pBuilding, ECX);

	if (pBuilding->IsIronCurtained() || pBuilding->Type->ImmuneToRadiation || pBuilding->InLimbo || pBuilding->BeingWarpedOut)
	{
		return 0;
	}

	auto const MainCoords = pBuilding->GetMapCoords();

	for (auto pFoundation = pBuilding->GetFoundationData(false); *pFoundation != CellStruct{ 0x7FFF, 0x7FFF }; ++pFoundation)
	{
		CellStruct CurrentCoord = MainCoords + *pFoundation;

		for (auto pRadExt : RadSiteExt::RadSiteInstance)
		{
			RadSiteClass* pRadSite = pRadExt->OwnerObject();

			// Check the distance, if not in range, just skip this one
			double orDistance = pRadSite->BaseCell.DistanceFrom(CurrentCoord);
			if (pRadSite->Spread < orDistance - 0.5)
				continue;

			RadType* pType = pRadExt->Type;
			int RadApplicationDelay = pType->BuildingApplicationDelay.Get(RulesExt::Global()->RadApplDelayBuilding);
			if ((RadApplicationDelay == 0) || (Unsorted::CurrentFrame % RadApplicationDelay != 0))
				continue;

			if (pRadExt->GetRadLevelAt(CurrentCoord) <= 0.0 || !pType->GetWarhead())
				continue;

			auto WH = pType->GetWarhead();
			auto absolute = WH->WallAbsoluteDestroyer;
			bool ignore = pBuilding->Type->Wall && absolute;
			auto damage = static_cast<int>((pRadExt->GetRadLevelAt(CurrentCoord) / 2) * pType->GetLevelFactor());

			if (pBuilding->IsAlive)// simple fix for previous issues
				pBuilding->ReceiveDamage(&damage, static_cast<int>(orDistance), WH, nullptr, ignore, absolute, pRadExt->RadHouse.Get());
		}
	}

	return 0;
}

// be aware that this function is updated every frame 
// putting debug log here can become mess because it gonna print bunch of debug line
DEFINE_HOOK(4DA554, FootClass_AI_RadSiteClass, 5)
{
	GET(FootClass* const, pFoot, ESI);

	auto state = static_cast<DamageState>(R->AL());

	if (!pFoot->IsIronCurtained() && !pFoot->GetTechnoType()->ImmuneToRadiation && !pFoot->InLimbo && !pFoot->IsInAir())
	{
		CellStruct CurrentCoord = pFoot->GetCell()->MapCoords;

		// Loop for each different radiation stored in the RadSites container
		for (auto const pRadExt : RadSiteExt::RadSiteInstance)
		{

			RadSiteClass* pRadSite = pRadExt->OwnerObject();

			// Check the distance, if not in range, just skip this one
			double orDistance = pRadSite->BaseCell.DistanceFrom(CurrentCoord);
			if (pRadSite->Spread < orDistance - 0.7)
				continue;

			RadType* pType = pRadExt->Type;
			int RadApplicationDelay = pType->GetApplicationDelay();
			if ((RadApplicationDelay == 0) || (Unsorted::CurrentFrame % RadApplicationDelay != 0))
				continue;

			// for more precise dmg calculation
			double RadLevel = pRadExt->GetRadLevelAt(CurrentCoord);
			if (RadLevel <= 0.0 || !pType->GetWarhead())
				continue;

			int Damage = static_cast<int>(RadLevel* pType->GetLevelFactor());
			int Distance = static_cast<int>(orDistance);
			auto WH = pType->GetWarhead();
			auto absolute = WH->WallAbsoluteDestroyer;

			if (pFoot->IsAlive || !pFoot->IsSinking)// simple fix for previous issues
				state = pFoot->ReceiveDamage(&Damage, Distance, WH, nullptr, false, absolute, pRadExt->RadHouse.Get());
		}
	}

	R->EAX<DamageState>(state);
	return pFoot->IsAlive ? 0x4DA63B : 0x4DAF00;
}

DEFINE_HOOK(65B593, RadSiteClass_Activate_Delay, 6)
{
	GET(RadSiteClass* const, pThis, ECX);
	auto const pExt = RadSiteExt::ExtMap.Find(pThis);

	auto const CurrentLevel = pThis->GetRadLevel();
	auto LevelDelay = pExt->Type->GetLevelDelay();
	auto LightDelay = pExt->Type->GetLightDelay();

	if (CurrentLevel < LevelDelay)
	{
		LevelDelay = CurrentLevel;
		LightDelay = CurrentLevel;
	}

	R->ECX(LevelDelay);
	R->EAX(LightDelay);

	return 0x65B59F;
}

#define GET_RADSITE(reg , value)\
	GET(RadSiteClass* const, pThis, reg);\
	RadSiteExt::ExtData *pExt = RadSiteExt::ExtMap.Find(pThis);\
	auto Output = pExt->Type->## value ##;

DEFINE_HOOK(65B5CE, RadSiteClass_Activate_Color, 6)
{
	GET_RADSITE(ESI, GetColor());

	R->EAX(0);
	R->EDX(0);
	R->EBX(0);

	R->DL(Output.G);
	R->EBP(R->EDX());

	R->BL(Output.B);
	R->AL(Output.R);

	// point out the missing register -Otamaa
	R->EDI(pThis);

	return 0x65B604;
}

DEFINE_HOOK(65B63E, RadSiteClass_Activate_LightFactor, 6)
{
	GET_RADSITE(ESI, GetLightFactor());

	__asm fmul Output;

	return 0x65B644;
}

DEFINE_HOOK_AGAIN(65B6A0, RadSiteClass_Activate_TintFactor, 6)
DEFINE_HOOK_AGAIN(65B6CA, RadSiteClass_Activate_TintFactor, 6)
DEFINE_HOOK(65B6F2, RadSiteClass_Activate_TintFactor, 6)
{
	GET_RADSITE(ESI, GetTintFactor());

	__asm fmul Output;

	return R->Origin() + 6;
}

DEFINE_HOOK(65B843, RadSiteClass_AI_LevelDelay, 6)
{
	GET_RADSITE(ESI, GetLevelDelay());

	R->ECX(Output);

	return 0x65B849;
}

DEFINE_HOOK(65B8B9, RadSiteClass_AI_LightDelay, 6)
{
	GET_RADSITE(ESI, GetLightDelay());

	R->ECX(Output);

	return 0x65B8BF;
}

// Additional Hook below 
DEFINE_HOOK(65BB67, RadSite_Deactivate, 6)
{
	GET_RADSITE(ECX, GetLevelDelay());

	R->ESI(Output);

	__asm xor edx, edx; // fixing integer overflow crash
	__asm idiv esi;

	return 0x65BB6D;
}