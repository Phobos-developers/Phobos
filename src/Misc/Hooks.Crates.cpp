#include <Ext/UnitType/Body.h>
#include <Ext/House/Body.h>
#include <New/Type/ResourceTypeClass.h>
#include <AnimClass.h>
#include <AnimTypeClass.h>
#include <VocClass.h>
#include <MessageListClass.h>

DEFINE_HOOK(0x56BD8B, MapClass_PlaceRandomCrate_Sampling, 0x5)
{
	enum { SpawnCrate = 0x56BE7B, SkipSpawn = 0x56BE91 };

	const int XP = 2 * MapClass::Instance.VisibleRect.X - MapClass::Instance.MapRect.Width
		+ ScenarioClass::Instance->Random.RandomRanged(0, 2 * MapClass::Instance.VisibleRect.Width);

	const int YP = 2 * MapClass::Instance.VisibleRect.Y + MapClass::Instance.MapRect.Width
		+ ScenarioClass::Instance->Random.RandomRanged(0, 2 * MapClass::Instance.VisibleRect.Height + 2);

	const CellStruct candidate { (short)((XP + YP) / 2),(short)((YP - XP) / 2) };
	const auto pCell = MapClass::Instance.TryGetCellAt(candidate);

	if (!pCell)
		return SkipSpawn;

	if (!MapClass::Instance.IsWithinUsableArea(pCell, true))
		return SkipSpawn;

	const bool isWater = pCell->LandType == LandType::Water;

	if (isWater && RulesExt::Global()->CrateOnlyOnLand.Get())
		return SkipSpawn;

	REF_STACK(CellStruct, cell, STACK_OFFSET(0x28, -0x18));

	cell = MapClass::Instance.NearByLocation(pCell->MapCoords,
		isWater ? SpeedType::Float : SpeedType::Track,
		-1, MovementZone::Normal, false, 1, 1, false, false, false, true, CellStruct::Empty, false, false);

	if (cell == CellStruct::Empty)
		return SkipSpawn;

	R->EAX(&cell);

	return SpawnCrate;
}

// Change RulesClass->FreeMCV default from 0 to 1.
DEFINE_PATCH(0x6656B3, 0x89, 0x4E);

DEFINE_HOOK(0x481BB8, CellClass_GoodieCheck_FreeMCV, 0x6)
{
	enum { SkipForcedMCV = 0x481C03, EnableForcedMCV = 0x481BF6 };

	GET(HouseClass*, pHouse, EDI);
	GET_STACK(UnitTypeClass*, pBaseUnit, STACK_OFFSET(0x188, -0x138));

	if (RulesClass::Instance->FreeMCV
		&& pHouse->Available_Money() > RulesExt::Global()->FreeMCV_CreditsThreshold
		&& SessionClass::Instance.Config.Bases
		&& !pHouse->OwnedBuildings
		&& !pHouse->CountOwnedNow(pBaseUnit))
	{
		return EnableForcedMCV;
	}

	return SkipForcedMCV;
}

DEFINE_HOOK(0x481C27, CellClass_GoodieCheck_UnitCrateVehicleCap, 0x0)
{
	enum { Capped = 0x481C44, NotCapped = 0x481C4A };

	GET(HouseClass*, pHouse, EDX);

	if (RulesExt::Global()->UnitCrateVehicleCap < 0 || pHouse->OwnedUnits <= RulesExt::Global()->UnitCrateVehicleCap)
		return NotCapped;

	return Capped;
}

DEFINE_HOOK(0x4821BD, CellClass_GoodieCheck_CrateGoodie, 0x6)
{
	enum { SkipGameCode = 0x4821C3 };

	GET(UnitTypeClass*, pUnitType, EDI);

	bool crateGoodie = pUnitType->CrateGoodie;

	if (crateGoodie)
	{
		auto const pTypeExt = UnitTypeExt::Fetch(pUnitType);

		if (pTypeExt->CrateGoodie_RerollChance > 0.0)
			crateGoodie = pTypeExt->CrateGoodie_RerollChance < ScenarioClass::Instance->Random.RandomDouble();
	}

	R->CL(crateGoodie);

	return SkipGameCode;
}

DEFINE_HOOK(0x481F9D, CellClass_SpringCrate_RevealMap, 0x8)
{
	GET_BASE(FootClass*, pFoot, 0x8);

	auto pOwner = pFoot->Owner;

	if (SessionClass::IsCampaign() && (pOwner->IsHumanPlayer || pOwner->IsInPlayerControl))
		pOwner = HouseClass::CurrentPlayer;

	MapClass::Instance.Reveal(pOwner);

	return 0x481FC8;
}

DEFINE_HOOK(0x4824CA, CellClass_GoodieCheck_MoneyCrate_CustomResources, 0xF)
{
	GET_BASE(FootClass*, pUnit, 0x8);
	GET(int, moneyAmount, EDI);
	GET(CellClass*, pCell, ESI);

	if (pUnit && pUnit->Owner)
	{
		const auto pOwner = pUnit->Owner;
		const auto pHouseExt = HouseExt::TryFetch(pOwner);

		// Build lottery candidates with equal weight
		// Candidate -1: Vanilla Money
		std::vector<int> candidates;
		candidates.push_back(-1);

		for (size_t i = 0; i < ResourceTypeClass::Array.size(); ++i)
		{
			const auto& pRes = ResourceTypeClass::Array[i];
			if (pRes && !pRes->IsMoneyResource() && !pRes->IsPowerResource() && pRes->Crate_Amount.Get() > 0)
			{
				if (!pHouseExt || pHouseExt->IsResourceEnabled(static_cast<int>(i)))
				{
					candidates.push_back(static_cast<int>(i));
				}
			}
		}

		int chosenIdx = -1;
		if (candidates.size() > 1)
		{
			const int roll = ScenarioClass::Instance->Random.RandomRanged(0, static_cast<int>(candidates.size() - 1));
			chosenIdx = candidates[roll];
		}

		if (chosenIdx == -1)
		{
			// Vanilla Money won
			if (moneyAmount > 0)
			{
				pOwner->GiveMoney(moneyAmount);
			}
		}
		else if (pHouseExt)
		{
			// Custom resource won
			const auto& pRes = ResourceTypeClass::Array[chosenIdx];
			const int amount = pRes->Crate_Amount.Get();
			pHouseExt->UpdateResourceAmount(chosenIdx, amount);

			const CoordStruct coords = pCell->GetCenterCoords();

			// Play custom sound if specified
			if (pRes->Crate_Sound.Get() >= 0)
			{
				VocClass::PlayAt(pRes->Crate_Sound.Get(), coords);
			}

			// Spawn custom animation if specified
			if (pRes->Crate_Anim.Get() >= 0)
			{
				if (const auto pAnimType = AnimTypeClass::Array.GetItemOrDefault(pRes->Crate_Anim.Get()))
				{
					GameCreate<AnimClass>(pAnimType, coords);
				}
			}
		}
	}

	return 0x4824D9;
}
