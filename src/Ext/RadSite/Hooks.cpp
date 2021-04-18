#include "Body.h"
#include <BulletClass.h>
#include <HouseClass.h>
#include <InfantryClass.h>
#include <WarheadTypeClass.h>
#include <ScenarioClass.h>

#include "../BuildingType/Body.h"
#include "../Bullet/Body.h"

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

DEFINE_HOOK(469150, B_Detonate_ApplyRad, 5)
{
	GET(BulletClass* const, pThis, ESI);
	GET_BASE(CoordStruct const*, pCoords, 0x8);

	auto const pExt = BulletExt::ExtMap.Find(pThis);
	auto const pWeapon = pThis->GetWeaponType();
	auto const pWH = pThis->WH;

	if (pWeapon && pWeapon->RadLevel > 0)
	{
		auto const cell = CellClass::Coord2Cell(*pCoords);
		auto const spread = static_cast<int>(pWH->CellSpread);
		pExt->ApplyRadiationToCell(cell, spread, pWeapon->RadLevel);
	}

	return 0x46920B;
}
/*
// hack it here so we can use this globally if needed
DEFINE_HOOK(46ADE0, BulletClass_ApplyRadiation, 5)
{
	GET(BulletClass* const, pThis, ECX);
	GET_STACK(CellStruct, location, 0x4);
	GET_STACK(int, spread, 0x8);
	GET_STACK(int, amount, 0xC);

	auto const& Instances = RadSiteExt::RadSiteInstance;
	auto const pWeapon = pThis->GetWeaponType();
	auto const pWeaponExt = WeaponTypeExt::ExtMap.FindOrAllocate(pWeapon);
	auto const pRadType = &pWeaponExt->RadType;
	auto const pThisHouse = pThis->Owner ? pThis->Owner->Owner : nullptr;

	if (Instances.Count > 0) {
		auto const it = std::find_if(Instances.begin(), Instances.end(),
			[=](RadSiteExt::ExtData* const pSite) // Lambda
			{// find
				return pSite->Type == pRadType &&
					pSite->OwnerObject()->BaseCell == location &&
					spread == pSite->OwnerObject()->Spread;
			});

		if (it == Instances.end()) {
			RadSiteExt::CreateInstance(location, spread, amount, pWeaponExt, pThisHouse);
		}
		else {
			auto pRadExt = *it;
			auto pRadSite = pRadExt->OwnerObject();

			if (pRadSite->GetRadLevel() + amount > pRadType->LevelMax) {
				amount = pRadType->LevelMax - pRadSite->GetRadLevel();
			}

			// Handle It
			pRadExt->Add(amount);
		}
	}
	else {
		RadSiteExt::CreateInstance(location, spread, amount, pWeaponExt, pThisHouse);
	}

	return 0x46AE5E;
}
*/
DEFINE_HOOK(5213E3, InfantryClass_AIDeployment_CheckRad, 4)
{
	GET(InfantryClass*, D, ESI);
	GET(int, WeaponRadLevel, EBX);

	auto const& Instances = RadSiteExt::RadSiteInstance;
	auto const pWeapon = D->GetWeapon(1)->WeaponType;

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

	return (RadLevel < WeaponRadLevel) ? 0x5213F4 : 0x521484;
}

// Too OP, be aware
DEFINE_HOOK(43FB23, BuildingClass_AI, 5)
{
	GET(BuildingClass* const, pBuilding, ECX);

	if (!pBuilding->IsIronCurtained() || !pBuilding->Type->ImmuneToRadiation || !pBuilding->InLimbo || !pBuilding->BeingWarpedOut)
	{
		auto const MainCoords = pBuilding->GetMapCoords();
		DynamicVectorClass<RadSiteClass*> eligible;

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
				int RadApplicationDelay = pType->GetBuildingApplicationDelay();
				if ((RadApplicationDelay == 0) || (Unsorted::CurrentFrame % RadApplicationDelay != 0))
					continue;

				// for more precise dmg calculation
				double RadLevel = pRadExt->GetRadLevelAt(CurrentCoord);
				if (RadLevel <= 0.0 || !pType->GetWarhead())
					continue;

				eligible.AddUnique(pRadSite);
			}

			if (eligible.Count > 0)
			{
				auto eligibleRad = eligible.GetItem(ScenarioClass::Instance->Random.Random() % eligible.Count);
				auto eligibleext = RadSiteExt::ExtMap.Find(eligibleRad);
				auto WH = eligibleext->Type->GetWarhead();
				auto absolute = WH->WallAbsoluteDestroyer;
				int Damage = static_cast<int>((eligibleext->GetRadLevelAt(CurrentCoord) / 2) * eligibleext->Type->GetLevelFactor());
				int Distance = static_cast<int>(eligibleRad->BaseCell.DistanceFrom(CurrentCoord));
				auto Radhouse = eligibleext->RadHouse;
				bool ignore = pBuilding->Type->Wall && absolute;

				pBuilding->ReceiveDamage(&Damage, Distance, WH, nullptr, ignore, absolute, Radhouse);
			}
		}
	}

	return 0;
}

// be aware that this function is updated every frame 
// putting debug log here can become mess because it gonna print bunch of debug line
DEFINE_HOOK(4DA554, FootClass_AI_RadSiteClass, 5)
{
	GET(FootClass* const, pFoot, ESI);

	if (!pFoot->IsIronCurtained() && !pFoot->GetTechnoType()->ImmuneToRadiation && !pFoot->InLimbo && !pFoot->IsInAir() && pFoot->IsAlive)
	{
		DynamicVectorClass<RadSiteClass*> eligible;
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

			//put eligible RadSite inside DynamicVector
			eligible.AddUnique(pRadSite);
		}

		//put this outsise the loop
		//fixing bugs that cause multiple rad site on single area generating multiple animtoinf/death anims
		if (eligible.Count > 0)
		{
			auto eligibleRad = eligible.GetItem(ScenarioClass::Instance->Random.Random() % eligible.Count);
			auto eligibleext = RadSiteExt::ExtMap.Find(eligibleRad);
			auto WH = eligibleext->Type->GetWarhead();
			auto absolute = WH->WallAbsoluteDestroyer;
			int Damage = static_cast<int>(eligibleext->GetRadLevelAt(CurrentCoord) * eligibleext->Type->GetLevelFactor());
			int Distance = static_cast<int>(eligibleRad->BaseCell.DistanceFrom(CurrentCoord));
			auto Radhouse = eligibleext->RadHouse;

			pFoot->ReceiveDamage(&Damage, Distance, WH, nullptr, false, absolute, Radhouse);
		}
	}

	if (pFoot->IsAlive)
	{
		return 0x4DA63B;
	}
	else
	{
		R->EAX<DamageState>(DamageState::NowDead);	
		return 0x4DAF00;
	}
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

DEFINE_HOOK(65B5CE, RadSiteClass_Activate_Color, 6)
{
	GET(RadSiteClass* const, pThis, ESI);

	auto pExt = RadSiteExt::ExtMap.Find(pThis);
	ColorStruct pRadColor = pExt->Type->GetColor();

	R->EAX(0);
	R->EDX(0);
	R->EBX(0);

	R->DL(pRadColor.G);
	R->EBP(R->EDX());

	R->BL(pRadColor.B);
	R->AL(pRadColor.R);

	// point out the missing register -Otamaa
	R->EDI(RulesClass::Instance);

	return 0x65B604;
}

DEFINE_HOOK(65B63E, RadSiteClass_Activate_LightFactor, 6)
{
	GET(RadSiteClass* const, Rad, ESI);

	auto pRadExt = RadSiteExt::ExtMap.Find(Rad);
	auto lightFactor = pRadExt->Type->GetLightFactor();

	__asm fmul lightFactor;

	return 0x65B644;
}

DEFINE_HOOK_AGAIN(65B6A0, RadSiteClass_Activate_TintFactor, 6)
DEFINE_HOOK_AGAIN(65B6CA, RadSiteClass_Activate_TintFactor, 6)
DEFINE_HOOK(65B6F2, RadSiteClass_Activate_TintFactor, 6)
{
	GET(RadSiteClass* const, Rad, ESI);

	auto pRadExt = RadSiteExt::ExtMap.Find(Rad);
	double tintFactor = pRadExt->Type->GetTintFactor();

	__asm fmul tintFactor;

	return R->Origin() + 6;
}

DEFINE_HOOK(65B843, RadSiteClass_AI_LevelDelay, 6)
{
	GET(RadSiteClass* const, Rad, ESI);

	auto pRadExt = RadSiteExt::ExtMap.Find(Rad);
	auto delay = pRadExt->Type->GetLevelDelay();

	R->ECX(delay);

	return 0x65B849;
}

DEFINE_HOOK(65B8B9, RadSiteClass_AI_LightDelay, 6)
{
	GET(RadSiteClass* const, Rad, ESI);

	auto pRadExt = RadSiteExt::ExtMap.Find(Rad);
	auto delay = pRadExt->Type->GetLightDelay();

	R->ECX(delay);

	return 0x65B8BF;
}

// Additional Hook below 
DEFINE_HOOK(65BB67, RadSite_Deactivate, 6)
{
	GET(const RadSiteClass*, Rad, ECX);

	auto pRadExt = RadSiteExt::ExtMap.Find(Rad);
	auto delay = pRadExt->Type->GetLevelDelay();

	R->ESI(delay);

	__asm xor edx, edx; // fixing integer overflow crash
	__asm idiv esi;

	return 0x65BB6D;
}