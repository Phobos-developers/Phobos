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
	GET(const int, nFiringIndex, EBX);
	GET(BuildingClass*, pThis, ESI);

	auto const pType = pThis->Type;

	if (pType->MaxNumberOccupants > 10)
	{
		auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pType);
		R->EAX(&pTypeExt->OccupierMuzzleFlashes[nFiringIndex]);
	}

	return 0;
}

DEFINE_HOOK(0x45387A, BuildingClass_FireOffset_Replace_MuzzleFix, 0xA)
{
	GET(BuildingClass*, pThis, ESI);

	auto const pType = pThis->Type;

	if (pType->MaxNumberOccupants > 10)
	{
		auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pType);
		R->EDX(&pTypeExt->OccupierMuzzleFlashes[pThis->FiringOccupantIndex]);
	}

	return 0;
}

DEFINE_HOOK(0x458623, BuildingClass_KillOccupiers_Replace_MuzzleFix, 0x7)
{
	GET(const int, nFiringIndex, EDI);
	GET(BuildingClass*, pThis, ESI);

	auto const pType = pThis->Type;

	if (pType->MaxNumberOccupants > 10)
	{
		auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pType);
		R->ECX(&pTypeExt->OccupierMuzzleFlashes[nFiringIndex]);
	}

	return 0;
}

DEFINE_HOOK(0x6D528A, TacticalClass_DrawPlacement_PlacementPreview, 0x6)
{
	if (!RulesExt::Global()->PlacementPreview || !Phobos::Config::ShowPlacementPreview)
		return 0;

	const auto pBuilding = specific_cast<BuildingClass*>(DisplayClass::Instance.CurrentBuilding);
	const auto pType = pBuilding ? pBuilding->Type : nullptr;
	const auto pTypeExt = BuildingTypeExt::ExtMap.TryFind(pType);
	const bool isShow = pTypeExt && pTypeExt->PlacementPreview;

	if (isShow)
	{
		CellClass* pCell = nullptr;
		{
			const CellStruct nDisplayCell = Make_Global<CellStruct>(0x88095C);
			const CellStruct nDisplayCell_Offset = Make_Global<CellStruct>(0x880960);

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
			const CoordStruct offset = pTypeExt->PlacementPreview_Offset;
			const int nHeight = offset.Z + pCell->GetFloorHeight({ 0, 0 });
			const CoordStruct coords = CellClass::Cell2Coord(pCell->MapCoords, nHeight);

			point = TacticalClass::Instance->CoordsToClient(coords).first;
			point.X += offset.X;
			point.Y += offset.Y;
		}

		const BlitterFlags blitFlags = pTypeExt->PlacementPreview_Translucency.Get(RulesExt::Global()->PlacementPreview_Translucency) |
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
	const BlitterFlags translucency = (RulesExt::Global()->PlacementPreview && Phobos::Config::ShowPlacementPreview)
		? RulesExt::Global()->PlacementGrid_TranslucencyWithPreview.Get(RulesExt::Global()->PlacementGrid_Translucency)
		: RulesExt::Global()->PlacementGrid_Translucency;

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

	auto const pTmpTypeExt = BuildingTypeExt::ExtMap.Find(ProximityTemp::pType);
	auto const& pBuildingsAllowed = pTmpTypeExt->Adjacent_Allowed;

	if (pBuildingsAllowed.size() > 0 && !pBuildingsAllowed.Contains(pCellBuilding->Type))
		return SkipBuilding;

	auto const& pBuildingsDisallowed = pTmpTypeExt->Adjacent_Disallowed;

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
		GET_STACK(const int, damage, STACK_OFFSET(0xC8, -0x9C));
		R->EAX(static_cast<int>(damage * BuildingTypeExt::ExtMap.Find(Building->Type)->BuildingOccupyDamageMult.Get(RulesClass::Instance->OccupyDamageMultiplier)));
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
		GET_STACK(const int, damage, STACK_OFFSET(0xC8, -0x9C));
		R->EAX(static_cast<int>(damage * BuildingTypeExt::ExtMap.Find(Building->Type)->BuildingBunkerDamageMult.Get(RulesClass::Instance->OccupyDamageMultiplier)));
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
			GET_STACK(const int, rof, STACK_OFFSET(0x10, 0x4));
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
			GET_STACK(const int, rof, STACK_OFFSET(0x10, 0x4));
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

	BuildingClass const* pThis = R->Origin() == BunkerWallDownSound_01
		? R->EDI<BuildingClass*>() : R->ESI<BuildingClass*>();

	BuildingTypeExt::PlayBunkerSound(pThis, R->Origin() == BunkerWallUpSound);

	switch (R->Origin())
	{
	case BunkerWallUpSound:
		return BunkerWallUpSound_Handled_ret;
	case BunkerWallDownSound_01:
		return BunkerWallDownSound_01_Handled_ret;
	case BunkerWallDownSound_02:
		return BunkerWallDownSound_02_Handled_ret;
	default:
		__assume(0); // Just avoiding compilation warnings, shouldn't go this way
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
	GET_STACK(const int, powerMultiplier, STACK_OFFSET(0xC, -0x4));

	const double factor = BuildingTypeExt::ExtMap.Find(pThis->Type)->PowerPlant_DamageFactor;

	if (factor == 1.0)
		R->EAX(Game::F2I(powerMultiplier * pThis->GetHealthPercentage()));
	else if (factor == 0.0)
		R->EAX(powerMultiplier);
	else
		R->EAX(Math::max(Game::F2I(powerMultiplier * (1.0 - factor + factor * pThis->GetHealthPercentage())), 0));

	return Handled;
}

#pragma region WeaponFactoryPath

DEFINE_HOOK(0x73F5A7, UnitClass_IsCellOccupied_UnlimboDirection, 0x8)
{
	enum { NextObject = 0x73FA87, ContinueCheck = 0x73F5AF };

	GET(const bool, notPassable, EAX);

	if (notPassable)
		return ContinueCheck;

	GET(BuildingClass* const, pBuilding, ESI);

	const auto pType = pBuilding->Type;

	if (!pType->WeaponsFactory)
		return NextObject;

	GET(CellClass* const, pCell, EDI);

	if (!RulesExt::Global()->ExtendedWeaponsFactory)
		return pCell->MapCoords.Y == pType->FoundationOutside[10].Y ? NextObject : ContinueCheck;

	auto buffer = CoordStruct::Empty;
	pBuilding->GetExitCoords(&buffer, 0);
	const auto cell = CellClass::Coord2Cell(buffer);
	const bool pathX = (BuildingTypeExt::ExtMap.Find(pType)->WeaponsFactory_Dir.Get() & 2) != 0; // 2,6/0,4
	const bool onPath = pathX ? pCell->MapCoords.Y == cell.Y : pCell->MapCoords.X == cell.X;

	return onPath ? NextObject : ContinueCheck;
}

#pragma endregion

#pragma region WeaponFactoryDirection

DEFINE_HOOK(0x44457B, BuildingClass_KickOutUnit_UnlimboDirection, 0x5)
{
	if (!RulesExt::Global()->ExtendedWeaponsFactory)
		return 0;

	GET(BuildingClass* const, pThis, ESI);
	REF_STACK(DirType, dir, STACK_OFFSET(0x144, -0x144));

	dir = static_cast<DirType>(BuildingTypeExt::ExtMap.Find(pThis->Type)->WeaponsFactory_Dir.Get() << 5);

	return 0;
}

static inline CellStruct GetWeaponFactoryDoor(BuildingClass* pThis)
{
	auto cell = pThis->GetMapCoords();
	auto buffer = CoordStruct::Empty;
	pThis->GetExitCoords(&buffer, 0);
	const auto pType = pThis->Type;

	switch (RulesExt::Global()->ExtendedWeaponsFactory ? BuildingTypeExt::ExtMap.Find(pType)->WeaponsFactory_Dir.Get() : 2)
	{

	case 0:
	{
		cell.X = static_cast<short>(buffer.X / Unsorted::LeptonsPerCell);
		break;
	}

	case 2:
	{
		cell.X += static_cast<short>(pType->GetFoundationWidth() - 1);
		cell.Y = static_cast<short>(buffer.Y / Unsorted::LeptonsPerCell);
		break;
	}

	case 4:
	{
		cell.X = static_cast<short>(buffer.X / Unsorted::LeptonsPerCell);
		cell.Y += static_cast<short>(pType->GetFoundationHeight(false) - 1);
		break;
	}

	case 6:
	{
		cell.Y = static_cast<short>(buffer.Y / Unsorted::LeptonsPerCell);
		break;
	}

	default:
	{
		break;
	}

	}

	return cell;
}

DEFINE_HOOK(0x44955D, BuildingClass_WeaponFactoryOutsideBusy_WeaponFactoryCell, 0x6)
{
	if (!RulesExt::Global()->ExtendedWeaponsFactory)
		return 0;

	enum { SkipGameCode = 0x4495DF };

	GET(BuildingClass* const, pThis, ESI);
	REF_STACK(CoordStruct, coords, STACK_OFFSET(0x30, -0xC));

	const auto cell = GetWeaponFactoryDoor(pThis);
	coords = CellClass::Cell2Coord(cell);

	R->EAX(MapClass::Instance.GetCellAt(cell));

	return SkipGameCode;
}

DEFINE_JUMP(LJMP, 0x44DCC7, 0x44DD3C);

DEFINE_HOOK(0x44E131, BuildingClass_Mission_Unload_WeaponFactoryFix1, 0x5)
{
	enum { SkipGameCode = 0x44E191 };

	GET(BuildingClass* const, pThis, EBP);
	GET(FootClass* const, pLink, EDI);
//	REF_STACK(const CoordStruct, coords, STACK_OFFSET(0x50, -0x1C));

	const auto cell = GetWeaponFactoryDoor(pThis);
	const auto coords = CellClass::Cell2Coord(cell);

	if (RulesExt::Global()->ExtendedWeaponsFactory)
	{
//		const auto pType = pLink->GetTechnoType();
//		const bool isSubterranean = pType->IsSubterranean;
//		pType->IsSubterranean = false;
		pLink->SetDestination(MapClass::Instance.GetCellAt(cell), true);
//		pType->IsSubterranean = isSubterranean;
	}
	else
	{
		pLink->Locomotor->Force_Track(66, coords);
	}

	return SkipGameCode;
}

DEFINE_HOOK(0x44DF72, BuildingClass_Mission_Unload_WeaponFactoryFix2, 0x5)
{
	enum { SkipGameCode = 0x44E1AD };

	GET(BuildingClass* const, pThis, EBP);
	GET_STACK(FootClass* const, pLink, STACK_OFFSET(0x50, -0x30));
//	REF_STACK(const CoordStruct, coords, STACK_OFFSET(0x50, -0x1C));

	const auto cell = GetWeaponFactoryDoor(pThis);
	const auto coords = CellClass::Cell2Coord(cell);

	if (RulesExt::Global()->ExtendedWeaponsFactory)
		pLink->SetDestination(MapClass::Instance.GetCellAt(cell), true);
	else
		pLink->Locomotor->Force_Track(66, coords);

	R->EDI(pLink);

	return SkipGameCode;
}

DEFINE_HOOK(0x44DF1C, BuildingClass_Mission_Unload_WeaponFactoryFix3, 0x7)
{
	if (!RulesExt::Global()->ExtendedWeaponsFactory)
		return 0;

	enum { SkipGameCode = 0x44DF47 };

	GET(BuildingClass* const, pThis, EBP);
	GET_STACK(FootClass* const, pLink, STACK_OFFSET(0x50, -0x30));
	REF_STACK(CellStruct, cell, STACK_OFFSET(0x50, -0x34));
//	REF_STACK(const CoordStruct, coords, STACK_OFFSET(0x50, -0x1C));

	cell = GetWeaponFactoryDoor(pThis);

	R->ESI(pLink);

	return SkipGameCode;
}

DEFINE_HOOK(0x742D98, UnitClass_SetDestination_WeaponFactoryCell, 0x6)
{
	if (!RulesExt::Global()->ExtendedWeaponsFactory)
		return 0;

	enum { SkipGameCode = 0x742DFB };

	GET(BuildingClass* const, pLink, ESI);

	const auto cell = GetWeaponFactoryDoor(pLink);

	R->EAX(MapClass::Instance.GetCellAt(cell));

	return SkipGameCode;
}

DEFINE_HOOK(0x516D3C, HoverLocomotionClass_IsIonSensitive_WeaponFactoryCell, 0x5)
{
	if (!RulesExt::Global()->ExtendedWeaponsFactory)
		return 0;

	enum { Right = 0x516DFF, IsNot = 0x516DF6 };

	GET(BuildingClass* const, pBuilding, EAX);
	GET(ILocomotion* const, iLoco, ESI);

	const auto location = CellClass::Coord2Cell(static_cast<LocomotionClass*>(iLoco)->LinkedTo->Location);
	bool notIon = false;
	auto buffer = CoordStruct::Empty;
	pBuilding->GetExitCoords(&buffer, 0);
	const auto cell = CellClass::Coord2Cell(buffer);
	const auto pType = pBuilding->Type;

	switch (BuildingTypeExt::ExtMap.Find(pType)->WeaponsFactory_Dir.Get())
	{

	case 0:
	{
		notIon |= (cell.X == location.X
			&& cell.Y != location.Y
			&& (pBuilding->Location.Y / Unsorted::LeptonsPerCell) != location.Y);

		break;
	}

	case 2:
	{
		notIon |= (cell.Y == location.Y
			&& cell.X != location.X
			&& (pBuilding->Location.X / Unsorted::LeptonsPerCell + pType->GetFoundationWidth() - 1) != location.X);

		break;
	}

	case 4:
	{
		notIon |= (cell.X == location.X
			&& cell.Y != location.Y
			&& (pBuilding->Location.Y / Unsorted::LeptonsPerCell + pType->GetFoundationHeight(false) - 1) != location.Y);

		break;
	}

	case 6:
	{
		notIon |= (cell.Y == location.Y
			&& cell.X != location.X
			&& (pBuilding->Location.X / Unsorted::LeptonsPerCell) != location.X);

		break;
	}

	default:
	{
		break;
	}

	}

	return notIon ? IsNot : Right;
}

DEFINE_HOOK(0x7443D9, UnitClass_ReadyToNextMission_WeaponFactoryCell, 0x5)
{
	enum { SkipGameCode = 0x744463 };
	return RulesExt::Global()->ExtendedWeaponsFactory ? SkipGameCode : 0;
}

#pragma endregion

#pragma region ImpassableRowsDirection

DEFINE_HOOK(0x458A00, BuildingClass_IsCellNotPassable_ImpassableRowsDirection, 0x6)
{
	enum { SkipGameCode = 0x458A76 };

	GET(BuildingClass* const, pThis, ECX);
	GET_STACK(CellClass* const, pCell, STACK_OFFSET(0x0, 0x4));

	auto isCellNotPassable = [pThis, pCell]() -> bool
	{
		if (pCell->GetBuilding() != pThis)
			return false;

		const auto pType = pThis->Type;

		if (pType->NumberImpassableRows == -1)
			return true;

		if (pType->Bunker && pThis->BunkerLinkedItem)
			return true;

		switch (BuildingTypeExt::ExtMap.Find(pType)->NumberImpassableRows_Dir.Get())
		{

		case 0:
		{
			const int y = pThis->Location.Y / Unsorted::LeptonsPerCell;
			const int maxPassableY = y + pType->GetFoundationHeight(false) - 1 - pType->NumberImpassableRows;
			return pCell->MapCoords.Y > maxPassableY;
		}

		case 2:
		{
			const int x = pThis->Location.X / Unsorted::LeptonsPerCell;
			const int minPassableX = x + pType->NumberImpassableRows;
			return pCell->MapCoords.X < minPassableX;
		}

		case 4:
		{
			const int y = pThis->Location.Y / Unsorted::LeptonsPerCell;
			const int minPassableY = y + pType->NumberImpassableRows;
			return pCell->MapCoords.Y < minPassableY;
		}

		case 6:
		{
			const int x = pThis->Location.X / Unsorted::LeptonsPerCell;
			const int maxPassableX = x + pType->GetFoundationWidth() - 1 - pType->NumberImpassableRows;
			return pCell->MapCoords.X > maxPassableX;
		}

		default:
		{
			return true;
		}

		}
	};

	R->EAX(isCellNotPassable());

	return SkipGameCode;
}

#pragma endregion

#pragma region BibDirection

// The input parameter of GetFoundationHeight is incorrect, and there is no call to input the incorrect parameter, so no need to consider it
DEFINE_HOOK(0x73F7DD, BuildingClass_IsCellNotPassable_BibDirection, 0x8)
{
	enum { SkipGameCode = 0x73F816 };

	GET(CellClass* const, pCell, EDI);
	GET(BuildingTypeClass* const, pType, EAX);

	R->ECX(MapClass::Instance.GetCellAt(Unsorted::AdjacentCell[BuildingTypeExt::ExtMap.Find(pType)->Bib_Dir.Get()] + pCell->MapCoords));

	return SkipGameCode;
}

#pragma endregion
