#include "Body.h"

#include <BuildingClass.h>

DEFINE_HOOK(0x460285, BuildingTypeClass_LoadFromINI_Muzzle, 0x6)
{
	enum { Skip = 0x460388, Read = 0x460299 };

	GET(BuildingTypeClass*, pThis, EBP);

	// Restore overriden instructions
	R->Stack(STACK_OFFS(0x368, 0x358), 0);
	R->EDX(0);

	// Disable Vanilla Muzzle flash when MaxNumberOccupants is 0 or more than 10
	return !pThis->MaxNumberOccupants || pThis->MaxNumberOccupants > 10
		? Skip : Read;
}

DEFINE_HOOK(0x44043D, BuildingClass_AI_Temporaled_Chronosparkle_MuzzleFix, 0x8)
{
	GET(BuildingClass*, pThis, ESI);

	auto pType = pThis->Type;
	if (pType->MaxNumberOccupants > 10)
	{
		GET(int, nFiringIndex, EBX);
		auto pTypeExt = BuildingTypeExt::ExtMap.Find(pType);
		R->EAX(&pTypeExt->OccupierMuzzleFlashes[nFiringIndex]);
	}

	return 0;
}

DEFINE_HOOK(0x45387A, BuildingClass_FireOffset_Replace_MuzzleFix, 0xA)
{
	GET(BuildingClass*, pThis, ESI);

	auto pType = pThis->Type;
	if (pType->MaxNumberOccupants > 10)
	{
		auto pTypeExt = BuildingTypeExt::ExtMap.Find(pType);
		R->EDX(&pTypeExt->OccupierMuzzleFlashes[pThis->FiringOccupantIndex]);
	}

	return 0;
}

DEFINE_HOOK(0x458623, BuildingClass_KillOccupiers_Replace_MuzzleFix, 0x7)
{
	GET(BuildingClass*, pThis, ESI);

	auto pType = pThis->Type;
	if (pType->MaxNumberOccupants > 10)
	{
		GET(int, nFiringIndex, EDI);
		auto pTypeExt = BuildingTypeExt::ExtMap.Find(pType);
		R->ECX(&pTypeExt->OccupierMuzzleFlashes[nFiringIndex]);
	}

	return 0;
}

DEFINE_HOOK(0x6FE3F1, TechnoClass_Fireat_OccupyDamageBonus, 0xB)
{
	GET(TechnoClass* const, pThis, ESI);

	if (auto const Building = specific_cast<BuildingClass*>(pThis))
	{
		if (auto const TypeExt = BuildingTypeExt::ExtMap.Find(Building->Type))
		{
			GET_STACK(int, nDamage, 0x2C);
			R->EAX(Game::F2I(nDamage * TypeExt->BuildingOccupyDamageMult.Get(RulesClass::Instance->OccupyDamageMultiplier)));
			return 0x6FE405;
		}
	}

	return 0;
}

DEFINE_HOOK(0x6FE421, TechnoClass_Fireat_BunkerDamageBonus, 0xB)
{
	GET(TechnoClass* const, pThis, ESI);

	if (auto const Building = specific_cast<BuildingClass*>(pThis->BunkerLinkedItem))
	{
		if (auto const TypeExt = BuildingTypeExt::ExtMap.Find(Building->Type))
		{
			GET_STACK(int, nDamage, 0x2C);
			R->EAX(Game::F2I(nDamage * TypeExt->BuildingBunkerDamageMult.Get(RulesClass::Instance->OccupyDamageMultiplier)));
			return 0x6FE435;
		}
	}

	return 0;
}

DEFINE_HOOK(0x6FD183, TechnoClass_RearmDelay_BuildingOccupyROFMult, 0xC)
{
	GET(TechnoClass*, pThis, ESI);

	if (auto const Building = specific_cast<BuildingClass*>(pThis))
	{
		if (auto const TypeExt = BuildingTypeExt::ExtMap.Find(Building->Type))
		{
			auto const nMult = TypeExt->BuildingOccupyROFMult.Get(RulesClass::Instance->OccupyROFMultiplier);
			if (nMult > 0.0f)
			{
				GET_STACK(int, nROF, STACK_OFFS(0x10, -0x4));
				R->EAX(Game::F2I(((double)nROF) / nMult));
				return 0x6FD1AB;
			}
			return 0x6FD1B1;
		}
	}

	return 0x0;
}

DEFINE_HOOK(0x6FD1C7, TechnoClass_RearmDelay_BuildingBunkerROFMult, 0xC)
{
	GET(TechnoClass*, pThis, ESI);

	if (auto const Building = specific_cast<BuildingClass*>(pThis->BunkerLinkedItem))
	{
		if (auto const TypeExt = BuildingTypeExt::ExtMap.Find(Building->Type))
		{
			auto const nMult = TypeExt->BuildingBunkerROFMult.Get(RulesClass::Instance->BunkerROFMultiplier);
			if (nMult > 0.0f)
			{
				GET_STACK(int, nROF, STACK_OFFS(0x10, -0x4));
				R->EAX(Game::F2I(((double)nROF) / nMult));
				return 0x6FD1EF;
			}
			return 0x6FD1F1;
		}
	}

	return 0x0;
}

static void BunkerPlaySound(BuildingClass const* pThis, bool bUp = false)
{
	auto nSound = bUp ? RulesClass::Instance->BunkerWallsUpSound : RulesClass::Instance->BunkerWallsDownSound;

	if (auto const TypeExt = BuildingTypeExt::ExtMap.Find(pThis->Type))
		nSound = bUp ? TypeExt->BunkerWallsUpSound.Get(nSound) : TypeExt->BunkerWallsDownSound.Get(nSound);

	if (nSound != -1)
		VocClass::PlayAt(nSound, pThis->Location);
}

DEFINE_HOOK_AGAIN(0x45933D, BuildingClass_BunkerSound, 0x5)
DEFINE_HOOK_AGAIN(0x4595D9, BuildingClass_BunkerSound, 0x5)
DEFINE_HOOK(0x459494, BuildingClass_BunkerSound, 0x5)
{
	BuildingClass const* pThis = R->Origin() == 0x4595D9 ?
		R->EDI<BuildingClass*>() : R->ESI<BuildingClass*>();

	BunkerPlaySound(pThis, R->Origin() != 0x459494);

	switch (R->Origin())
	{
	case 0x45933D:
		return 0x459374;
	case 0x4595D9:
		return 0x459612;
	}

	return 0x4594CD;
}

// not working  ?
// hook itself is fine , but it not play globally as it should :s
DEFINE_HOOK(0x44A86A, BuildingClass_Mi_Selling_PackupSound, 0x6)
{
	GET(BuildingClass* const, pThis, EBP);
	CoordStruct nBuffer;
	pThis->GetCenterCoord(&nBuffer);
	auto const pExt = BuildingTypeExt::ExtMap.Find(pThis->Type);
	VocClass::PlayIndexAtPos(pThis->Type->PackupSound,nBuffer, pExt && pExt->PackupSound_PlayGlobal.Get());
	return 0x44A89E;
}

DEFINE_HOOK_AGAIN(0x4426DB, BuildingClass_ReceiveDamage_DisableDamageSound, 0x8)
DEFINE_HOOK_AGAIN(0x702777, BuildingClass_ReceiveDamage_DisableDamageSound, 0x8)
DEFINE_HOOK(0x70272E, BuildingClass_ReceiveDamage_DisableDamageSound, 0x8)
{
	GET(TechnoClass*, pThis, ESI);

	if (auto const pBuilding = specific_cast<BuildingClass*>(pThis))
	{
		auto const pExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type);
		if(pExt && pExt->DisableDamageSound.Get())
		{
			switch (R->Origin())
			{
			case 0x70272E:
				return 0x702765;
			case 0x702777:
				return 0x7027AE;
			case 0x4426DB:
				return 0x44270B;
			}
		}
	}

	return 0x0;
}

DEFINE_HOOK(0x44E85F, BuildingClass_Power_DegradeWithHealth, 0x7)
{
	GET(BuildingClass*, pThis, ESI);
	GET_STACK(int, nPowMult, STACK_OFFS(0xC, 0x4));

	auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pThis->Type);

	R->EAX(pTypeExt->Power_DegradeWithHealth.Get() ? Game::F2I(nPowMult * pThis->GetHealthPercentage()) : (nPowMult));

	return 0x44E86F;
}