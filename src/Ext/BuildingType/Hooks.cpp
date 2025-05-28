#include "Body.h"

#include <TacticalClass.h>
#include <Ext/Rules/Body.h>

#include <Utilities/Macro.h>
#include <Utilities/EnumFunctions.h>

DEFINE_HOOK(0x460285, BuildingTypeClass_LoadFromINI_Muzzle, 0x6)
{
	enum { Skip = 0x460388, Read = 0x460299 };

	GET(BuildingTypeClass*, pThis, EBP);

	// Restore overriden instructions
	R->Stack(STACK_OFFSET(0x368, -0x358), 0);
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

DEFINE_HOOK(0x6D528A, TacticalClass_DrawPlacement_PlacementPreview, 0x6)
{
	auto pRules = RulesExt::Global();

	if (!pRules->PlacementPreview || !Phobos::Config::ShowPlacementPreview)
		return 0;

	auto pBuilding = specific_cast<BuildingClass*>(DisplayClass::Instance.CurrentBuilding);
	auto pType = pBuilding ? pBuilding->Type : nullptr;
	auto pTypeExt = pType ? BuildingTypeExt::ExtMap.Find(pType) : nullptr;
	bool isShow = pTypeExt && pTypeExt->PlacementPreview;

	if (isShow)
	{
		CellClass* pCell = nullptr;
		{
			CellStruct nDisplayCell = Make_Global<CellStruct>(0x88095C);
			CellStruct nDisplayCell_Offset = Make_Global<CellStruct>(0x880960);

			pCell = MapClass::Instance.TryGetCellAt(nDisplayCell + nDisplayCell_Offset);
			if (!pCell)
				return 0;
		}

		int nImageFrame = 0;
		SHPStruct* pImage = pTypeExt->PlacementPreview_Shape.GetSHP();
		{
			if (!pImage)
			{
				pImage = pType->LoadBuildup();
				if (pImage)
					nImageFrame = ((pImage->Frames / 2) - 1);
				else
					pImage = pType->GetImage();

				if (!pImage)
					return 0;
			}

			nImageFrame = Math::clamp(pTypeExt->PlacementPreview_ShapeFrame.Get(nImageFrame), 0, (int)pImage->Frames);
		}


		Point2D point;
		{
			CoordStruct offset = pTypeExt->PlacementPreview_Offset;
			int nHeight = offset.Z + pCell->GetFloorHeight({ 0, 0 });
			CoordStruct coords = CellClass::Cell2Coord(pCell->MapCoords, nHeight);

			point = TacticalClass::Instance->CoordsToClient(coords).first;
			point.X += offset.X;
			point.Y += offset.Y;
		}

		BlitterFlags blitFlags = pTypeExt->PlacementPreview_Translucency.Get(pRules->PlacementPreview_Translucency) |
			BlitterFlags::Centered | BlitterFlags::Nonzero | BlitterFlags::MultiPass;

		ConvertClass* pPalette = pTypeExt->PlacementPreview_Remap.Get()
			? pBuilding->GetDrawer()
			: pTypeExt->PlacementPreview_Palette.GetOrDefaultConvert(FileSystem::UNITx_PAL);

		DSurface* pSurface = DSurface::Temp;
		RectangleStruct rect = pSurface->GetRect();
		rect.Height -= 32; // account for bottom bar

		CC_Draw_Shape(pSurface, pPalette, pImage, nImageFrame, &point, &rect, blitFlags,
			0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
	}

	return 0;
}

DEFINE_HOOK(0x47EFAE, CellClass_Draw_It_SetPlacementGridTranslucency, 0x6)
{
	auto pRules = RulesExt::Global();
	BlitterFlags translucency = (pRules->PlacementPreview && Phobos::Config::ShowPlacementPreview)
		? pRules->PlacementGrid_TranslucencyWithPreview.Get(pRules->PlacementGrid_Translucency)
		: pRules->PlacementGrid_Translucency;

	if (translucency != BlitterFlags::None)
	{
		LEA_STACK(BlitterFlags*, blitFlags, STACK_OFFSET(0x68, -0x58));
		*blitFlags |= translucency;
	}

	return 0;
}

// Rewritten
DEFINE_HOOK(0x465D40, BuildingTypeClass_IsVehicle, 0x6)
{
	enum { ReturnFromFunction = 0x465D6A };

	GET(BuildingTypeClass*, pThis, ECX);

	const auto pExt = BuildingTypeExt::ExtMap.Find(pThis);

	if (pExt->ConsideredVehicle.isset())
	{
		R->EAX(pExt->ConsideredVehicle.Get());
		return ReturnFromFunction;
	}

	return 0;
}

DEFINE_HOOK(0x5F5416, ObjectClass_ReceiveDamage_CanC4DamageRounding, 0x6)
{
	enum { SkipGameCode = 0x5F5456 };

	GET(ObjectClass*, pThis, ESI);
	GET(int*, pDamage, EDI);

	if (*pDamage == 0 && pThis->WhatAmI() == AbstractType::Building)
	{
		auto const pType = static_cast<BuildingClass*>(pThis)->Type;

		if (!pType->CanC4)
		{
			auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pType);

			if (!pTypeExt->CanC4_AllowZeroDamage)
				*pDamage = 1;
		}
	}

	return SkipGameCode;
}

#pragma region BuildingProximity

namespace ProximityTemp
{
	BuildingTypeClass* pType = nullptr;
}

DEFINE_HOOK(0x4A8F20, DisplayClass_BuildingProximityCheck_SetContext, 0x5)
{
	GET(BuildingTypeClass*, pType, ESI);

	ProximityTemp::pType = pType;

	return 0;
}

DEFINE_HOOK(0x4A8FD7, DisplayClass_BuildingProximityCheck_BuildArea, 0x6)
{
	enum { SkipBuilding = 0x4A902C };

	GET(BuildingClass*, pCellBuilding, ESI);

	auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pCellBuilding->Type);

	if (pTypeExt->NoBuildAreaOnBuildup && pCellBuilding->CurrentMission == Mission::Construction)
		return SkipBuilding;

	auto const& pBuildingsAllowed = BuildingTypeExt::ExtMap.Find(ProximityTemp::pType)->Adjacent_Allowed;

	if (pBuildingsAllowed.size() > 0 && !pBuildingsAllowed.Contains(pCellBuilding->Type))
		return SkipBuilding;

	auto const& pBuildingsDisallowed = BuildingTypeExt::ExtMap.Find(ProximityTemp::pType)->Adjacent_Disallowed;

	if (pBuildingsDisallowed.size() > 0 && pBuildingsDisallowed.Contains(pCellBuilding->Type))
		return SkipBuilding;

	return 0;
}

#pragma endregion

DEFINE_HOOK(0x6FE3F1, TechnoClass_FireAt_OccupyDamageBonus, 0xB)
{
	enum { ApplyDamageBonus = 0x6FE405 };

	GET(TechnoClass* const, pThis, ESI);

	if (const auto Building = specific_cast<BuildingClass*>(pThis))
	{
		GET_STACK(int, damage, STACK_OFFSET(0xC8, -0x9C));
		R->EAX(Game::F2I(damage * BuildingTypeExt::ExtMap.Find(Building->Type)->BuildingOccupyDamageMult.Get(RulesClass::Instance->OccupyDamageMultiplier)));
		return ApplyDamageBonus;
	}

	return 0;
}

DEFINE_HOOK(0x6FE421, TechnoClass_FireAt_BunkerDamageBonus, 0xB)
{
	enum { ApplyDamageBonus = 0x6FE435 };

	GET(TechnoClass* const, pThis, ESI);

	if (const auto Building = specific_cast<BuildingClass*>(pThis->BunkerLinkedItem))
	{
		GET_STACK(int, damage, STACK_OFFSET(0xC8, -0x9C));
		R->EAX(Game::F2I(damage * BuildingTypeExt::ExtMap.Find(Building->Type)->BuildingBunkerDamageMult.Get(RulesClass::Instance->OccupyDamageMultiplier)));
		return ApplyDamageBonus;
	}

	return 0;
}

DEFINE_HOOK(0x6FD183, TechnoClass_RearmDelay_BuildingOccupyROFMult, 0xC)
{
	enum { ApplyRofMod = 0x6FD1AB, SkipRofMod = 0x6FD1B1 };

	GET(TechnoClass*, pThis, ESI);

	if (const auto Building = specific_cast<BuildingClass*>(pThis))
	{
		const auto multiplier = BuildingTypeExt::ExtMap.Find(Building->Type)->BuildingOccupyROFMult.Get(RulesClass::Instance->OccupyROFMultiplier);

		if (multiplier > 0.0f)
		{
			GET_STACK(int, rof, STACK_OFFSET(0x10, 0x4));
			R->EAX(Game::F2I(static_cast<double>(rof) / multiplier));
			return ApplyRofMod;
		}

		return SkipRofMod;
	}

	return 0;
}

DEFINE_HOOK(0x6FD1C7, TechnoClass_RearmDelay_BuildingBunkerROFMult, 0xC)
{
	enum { ApplyRofMod = 0x6FD1EF, SkipRofMod = 0x6FD1F1 };

	GET(TechnoClass*, pThis, ESI);

	if (const auto Building = specific_cast<BuildingClass*>(pThis->BunkerLinkedItem))
	{
		const auto multiplier = BuildingTypeExt::ExtMap.Find(Building->Type)->BuildingBunkerROFMult.Get(RulesClass::Instance->BunkerROFMultiplier);

		if (multiplier > 0.0f)
		{
			GET_STACK(int, rof, STACK_OFFSET(0x10, 0x4));
			R->EAX(Game::F2I(static_cast<double>(rof) / multiplier));
			return ApplyRofMod;
		}

		return SkipRofMod;
	}

	return 0;
}

DEFINE_HOOK_AGAIN(0x45933D, BuildingClass_BunkerSound, 0x5)
DEFINE_HOOK_AGAIN(0x4595D9, BuildingClass_BunkerSound, 0x5)
DEFINE_HOOK(0x459494, BuildingClass_BunkerSound, 0x5)
{
	enum
	{
		BunkerWallUpSound = 0x45933D,
		BunkerWallUpSound_Handled_ret = 0x459374,

		BunkerWallDownSound_01 = 0x4595D9,
		BunkerWallDownSound_01_Handled_ret = 0x459612,

		BunkerWallDownSound_02 = 0x459494,
		BunkerWallDownSound_02_Handled_ret = 0x4594CD
	};

	BuildingClass const* pThis = R->Origin() == BunkerWallDownSound_01 ?
		R->EDI<BuildingClass*>() : R->ESI<BuildingClass*>();

	BuildingTypeExt::PlayBunkerSound(pThis, R->Origin() == BunkerWallUpSound);

	switch (R->Origin())
	{
	case BunkerWallUpSound:
		return BunkerWallUpSound_Handled_ret;
	case BunkerWallDownSound_01:
		return BunkerWallDownSound_01_Handled_ret;
	case BunkerWallDownSound_02:
		return BunkerWallDownSound_02_Handled_ret;
	}
}

DEFINE_HOOK_AGAIN(0x4426DB, BuildingClass_ReceiveDamage_DisableDamageSound, 0x8)
DEFINE_HOOK_AGAIN(0x702777, BuildingClass_ReceiveDamage_DisableDamageSound, 0x8)
DEFINE_HOOK(0x70272E, BuildingClass_ReceiveDamage_DisableDamageSound, 0x8)
{
	enum
	{
		BuildingClass_TakeDamage_DamageSound = 0x4426DB,
		BuildingClass_TakeDamage_DamageSound_Handled_ret = 0x44270B,

		TechnoClass_TakeDamage_Building_DamageSound_01 = 0x702777,
		TechnoClass_TakeDamage_Building_DamageSound_01_Handled_ret = 0x7027AE,

		TechnoClass_TakeDamage_Building_DamageSound_02 = 0x70272E,
		TechnoClass_TakeDamage_Building_DamageSound_02_Handled_ret = 0x702765,
	};

	GET(TechnoClass*, pThis, ESI);

	if (auto const pBuilding = specific_cast<BuildingClass*>(pThis))
	{
		if (BuildingTypeExt::ExtMap.Find(pBuilding->Type)->DisableDamageSound)
		{
			switch (R->Origin())
			{
			case BuildingClass_TakeDamage_DamageSound:
				return BuildingClass_TakeDamage_DamageSound_Handled_ret;
			case TechnoClass_TakeDamage_Building_DamageSound_01:
				return TechnoClass_TakeDamage_Building_DamageSound_01_Handled_ret;
			case TechnoClass_TakeDamage_Building_DamageSound_02:
				return TechnoClass_TakeDamage_Building_DamageSound_02_Handled_ret;
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x44E85F, BuildingClass_Power_DamageFactor, 0x7)
{
	enum { Handled = 0x44E86F };

	GET(BuildingClass*, pThis, ESI);
	GET_STACK(int, powerMultiplier, STACK_OFFSET(0xC, -0x4));

	const double factor = BuildingTypeExt::ExtMap.Find(pThis->Type)->PowerPlant_DamageFactor;
	R->EAX(Math::max(Game::F2I(powerMultiplier * (1.0 - factor + factor * pThis->GetHealthPercentage())), 0));

	return Handled;
}
