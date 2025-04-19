#include <AircraftTypeClass.h>
#include <SpawnManagerClass.h>
#include <TiberiumClass.h>
#include <TacticalClass.h>
#include "Body.h"

#include <Utilities/AresHelper.h>

struct DummyBuildingTypeExtHere
{
	char _[0x5D];
	bool Firestorm_Wall;
};

struct DummyTechnoExtHere
{
	char _[0x9C];
	bool DriverKilled;
};

DEFINE_HOOK(0x6F64A0, TechnoClass_DrawHealthBar, 0x5)
{
	enum { SkipDrawCode = 0x6F6ABD };

	GET(TechnoClass*, pThis, ECX);
	GET_STACK(Point2D*, pLocation, 0x4);
	GET_STACK(RectangleStruct*, pBounds, 0x8);
	//GET_STACK(bool, drawFullyHealthBar, 0xC);
	const auto pExt = TechnoExt::ExtMap.Find(pThis);
	const auto pTypeExt = pExt->TypeExtData;

	if (pTypeExt->HealthBar_Hide)
		return SkipDrawCode;

	const auto whatAmI = pThis->WhatAmI();
	const auto pBuilding = whatAmI == BuildingClass::AbsID ? static_cast<BuildingClass*>(pThis) : nullptr;
	Point2D position = *pLocation;
	Point2D pipsAdjust = Point2D::Empty;
	int pipsLength = 0;

	HealthBarTypeClass* pHealthBar = nullptr;

	if (pBuilding)
	{
		if (AresHelper::CanUseAres && reinterpret_cast<DummyBuildingTypeExtHere*>(pBuilding->Type->align_E24)->Firestorm_Wall)
			return SkipDrawCode;

		CoordStruct dimension {};
		pBuilding->Type->Dimension2(&dimension);
		dimension.X /= -2;
		dimension.Y /= 2;

		const auto drawAdjust = TacticalClass::CoordsToScreen(dimension);
		position += drawAdjust;

		dimension.Y = -dimension.Y;
		const auto drawStart = TacticalClass::CoordsToScreen(dimension);

		dimension.Z = 0;
		dimension.Y = -dimension.Y;
		pipsAdjust = TacticalClass::CoordsToScreen(dimension);

		pHealthBar = pTypeExt->HealthBar.Get(RulesExt::Global()->Buildings_DefaultHealthBar);
		pipsLength = (drawAdjust.Y - drawStart.Y) >> 1;
	}
	else
	{
		pipsAdjust = Point2D { -10, 10 };

		pHealthBar = pTypeExt->HealthBar.Get(RulesExt::Global()->DefaultHealthBar);

		constexpr int defaultInfantryPipsLength = 8;
		constexpr int defaultUnitPipsLength = 17;
		pipsLength = pHealthBar->PipsLength.Get(whatAmI == InfantryClass::AbsID ? defaultInfantryPipsLength : defaultUnitPipsLength);
	}

	const auto pOwner = pThis->Owner;
	const bool isAllied = pOwner->IsAlliedWith(HouseClass::CurrentPlayer);

	if (!RulesClass::Instance->EnemyHealth && !HouseClass::IsCurrentPlayerObserver() && !isAllied)
		return SkipDrawCode;

	const auto pShield = pExt->Shield.get();

	if (pShield && pShield->IsAvailable() && !pShield->IsBrokenAndNonRespawning())
	{
		if (pBuilding)
			pShield->DrawShieldBar_Building(pipsLength, &position, pBounds);
		else
			pShield->DrawShieldBar_Other(pipsLength, &position, pBounds);
	}

	if (pBuilding)
		TechnoExt::DrawHealthBar_Building(pThis, pHealthBar, pipsLength, &position, pBounds);
	else
		TechnoExt::DrawHealthBar_Other(pThis, pHealthBar, pipsLength, &position, pBounds);

	TechnoExt::ProcessDigitalDisplays(pThis, &position);

	if (AresHelper::CanUseAres && reinterpret_cast<DummyTechnoExtHere*>(pThis->align_154)->DriverKilled)
		return SkipDrawCode;

	const bool canShowPips = isAllied || pThis->DisplayProductionTo.Contains(HouseClass::CurrentPlayer) || HouseClass::IsCurrentPlayerObserver();

	if (canShowPips || (pBuilding && pBuilding->Type->CanBeOccupied) || pThis->GetTechnoType()->PipsDrawForAll)
	{
		Point2D pipsLocation = *pLocation + pipsAdjust;
		pThis->DrawPipScalePips(&pipsLocation, pLocation, pBounds);
	}

	return SkipDrawCode;
}

DEFINE_HOOK(0x6F534E, TechnoClass_DrawExtras_Insignia, 0x5)
{
	enum { SkipGameCode = 0x6F5388 };

	GET(TechnoClass*, pThis, EBP);
	GET_STACK(Point2D*, pLocation, STACK_OFFSET(0x98, 0x4));
	GET(RectangleStruct*, pBounds, ESI);

	if (pThis->VisualCharacter(false, nullptr) != VisualType::Hidden)
	{
		if (RulesExt::Global()->DrawInsignia_OnlyOnSelected.Get() && !pThis->IsSelected && !pThis->IsMouseHovering)
			return SkipGameCode;
		else
			TechnoExt::DrawInsignia(pThis, pLocation, pBounds);
	}

	return SkipGameCode;
}

DEFINE_HOOK(0x709B2E, TechnoClass_DrawPips_Sizes, 0x5)
{
	GET(TechnoClass*, pThis, ECX);
	REF_STACK(int, pipWidth, STACK_OFFSET(0x74, -0x1C));

	Point2D size;
	bool isBuilding = pThis->WhatAmI() == AbstractType::Building;

	if (pThis->GetTechnoType()->PipScale == PipScale::Ammo)
	{
		if (isBuilding)
			size = RulesExt::Global()->Pips_Ammo_Buildings_Size;
		else
			size = RulesExt::Global()->Pips_Ammo_Size;

		size = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())->AmmoPipSize.Get(size);
	}
	else
	{
		if (isBuilding)
			size = RulesExt::Global()->Pips_Generic_Buildings_Size;
		else
			size = RulesExt::Global()->Pips_Generic_Size;
	}

	pipWidth = size.X;
	R->ESI(size.Y);

	return 0;
}

DEFINE_HOOK(0x709B8B, TechnoClass_DrawPips_Spawns, 0x5)
{
	enum { SkipGameDrawing = 0x709C27 };

	GET(TechnoClass*, pThis, ECX);
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (!pTypeExt->ShowSpawnsPips)
		return SkipGameDrawing;

	LEA_STACK(RectangleStruct*, offset, STACK_OFFSET(0x74, -0x24));
	GET_STACK(RectangleStruct*, rect, STACK_OFFSET(0x74, 0xC));
	GET_STACK(SHPStruct*, shape, STACK_OFFSET(0x74, -0x58));
	GET_STACK(bool, isBuilding, STACK_OFFSET(0x74, -0x61));
	GET(int, maxSpawnsCount, EBX);

	int currentSpawnsCount = pThis->SpawnManager->CountDockedSpawns();
	auto const pipOffset = pTypeExt->SpawnsPipOffset.Get();
	Point2D position = { offset->X + pipOffset.X, offset->Y + pipOffset.Y };
	Point2D size;

	if (isBuilding)
		size = pTypeExt->SpawnsPipSize.Get(RulesExt::Global()->Pips_Generic_Buildings_Size);
	else
		size = pTypeExt->SpawnsPipSize.Get(RulesExt::Global()->Pips_Generic_Size);

	for (int i = 0; i < maxSpawnsCount; i++)
	{
		int frame = i < currentSpawnsCount ? pTypeExt->SpawnsPipFrame : pTypeExt->EmptySpawnsPipFrame;

		DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, shape, frame,
			&position, rect, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);

		position.X += size.X;
		position.Y += size.Y;
	}

	return SkipGameDrawing;
}

DEFINE_HOOK(0x70A36E, TechnoClass_DrawPips_Ammo, 0x6)
{
	enum { SkipGameDrawing = 0x70A4EC };

	GET(TechnoClass*, pThis, ECX);
	LEA_STACK(RectangleStruct*, offset, STACK_OFFSET(0x74, -0x24));
	GET_STACK(RectangleStruct*, rect, STACK_OFFSET(0x74, 0xC));
	GET(int, pipWrap, EBX);
	GET_STACK(int, pipCount, STACK_OFFSET(0x74, -0x54));
	GET_STACK(int, maxPips, STACK_OFFSET(0x74, -0x60));
	GET(int, yOffset, ESI);

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	auto const pipOffset = pTypeExt->AmmoPipOffset.Get();
	Point2D position = { offset->X + pipOffset.X, offset->Y + pipOffset.Y };

	if (pipWrap > 0)
	{
		int levels = maxPips / pipWrap - 1;

		for (int i = 0; i < pipWrap; i++)
		{
			int frame = pTypeExt->AmmoPipWrapStartFrame;

			if (levels >= 0)
			{
				int counter = i + pipWrap * levels;
				int frameCounter = levels;
				bool calculateFrame = true;

				while (counter >= pThis->Ammo)
				{
					frameCounter--;
					counter -= pipWrap;

					if (frameCounter < 0)
					{
						calculateFrame = false;
						break;
					}
				}

				if (calculateFrame)
					frame = frameCounter + frame + 1;
			}

			position.X += offset->Width;
			position.Y += yOffset;

			DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS2_SHP,
				frame, &position, rect, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
		}
	}
	else
	{
		int ammoFrame = pTypeExt->AmmoPipFrame;
		int emptyFrame = pTypeExt->EmptyAmmoPipFrame;

		for (int i = 0; i < maxPips; i++)
		{
			if (i >= pipCount && emptyFrame < 0)
				break;

			int frame = i >= pipCount ? emptyFrame : ammoFrame;

			DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS2_SHP,
				frame, &position, rect, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);

			position.X += offset->Width;
			position.Y += yOffset;
		}
	}

	return SkipGameDrawing;
}

DEFINE_HOOK(0x70A1F6, TechnoClass_DrawPips_Tiberium, 0x6)
{
	enum { SkipGameDrawing = 0x70A4EC };

	GET(TechnoClass*, pThis, ECX);
	LEA_STACK(RectangleStruct*, offset, STACK_OFFSET(0x74, -0x24));
	GET_STACK(RectangleStruct*, rect, STACK_OFFSET(0x74, 0xC));
	GET_STACK(SHPStruct*, shape, STACK_OFFSET(0x74, -0x58));
	GET_STACK(int, maxPips, STACK_OFFSET(0x74, -0x60));
	GET(int, yOffset, ESI);

	Point2D position = { offset->X, offset->Y };
	const int totalStorage = pThis->GetTechnoType()->Storage;

	std::vector<int> pipsToDraw;

	bool isWeeder = false;

	switch (pThis->WhatAmI())
	{
	case AbstractType::Building:
		isWeeder = static_cast<BuildingClass*>(pThis)->Type->Weeder;
		break;
	case AbstractType::Unit:
		isWeeder = static_cast<UnitClass*>(pThis)->Type->Weeder;
		break;
	default:
		break;
	}

	if (isWeeder)
	{
		const int fullWeedFrames = pThis->WhatAmI() == AbstractType::Building ?
			static_cast<int>(pThis->Owner->GetWeedStoragePercentage() * maxPips + 0.5) :
			static_cast<int>(pThis->Tiberium.GetTotalAmount() / totalStorage * maxPips + 0.5);

		for (int i = 0; i < maxPips; i++)
		{
			if (i < fullWeedFrames)
				pipsToDraw.push_back(RulesExt::Global()->Pips_Tiberiums_WeedFrame);
			else
				pipsToDraw.push_back(RulesExt::Global()->Pips_Tiberiums_WeedEmptyFrame);
		}
	}
	else
	{
		std::vector<int> tiberiumPipCounts(TiberiumClass::Array.Count);

		for (size_t i = 0; i < tiberiumPipCounts.size(); i++)
		{
			tiberiumPipCounts[i] = static_cast<int>(pThis->Tiberium.GetAmount(i) / totalStorage * maxPips + 0.5);
		}

		auto const rawPipOrder = RulesExt::Global()->Pips_Tiberiums_DisplayOrder.empty() ? std::vector<int>{ 0, 2, 3, 1 } : RulesExt::Global()->Pips_Tiberiums_DisplayOrder;
		auto const& pipFrames = RulesExt::Global()->Pips_Tiberiums_Frames;
		int const emptyFrame = RulesExt::Global()->Pips_Tiberiums_EmptyFrame;

		std::vector<int> pipOrder;

		// First make a new vector, removing all the duplicate and invalid tiberiums
		for (int index : rawPipOrder)
		{
			if (std::find(pipOrder.begin(), pipOrder.end(), index) == pipOrder.end() &&
				index >= 0 && index < TiberiumClass::Array.Count)
			{
				pipOrder.push_back(index);
			}
		}

		// Then add any tiberium types that are missing
		for (int i = 0; i < TiberiumClass::Array.Count; i++)
		{
			if (std::find(pipOrder.begin(), pipOrder.end(), i) == pipOrder.end())
			{
				pipOrder.push_back(i);
			}
		}

		for (int i = 0; i < maxPips; i++)
		{
			for (const int index : pipOrder)
			{
				if (tiberiumPipCounts[index] > 0)
				{
					tiberiumPipCounts[index]--;

					if (static_cast<size_t>(index) >= pipFrames.size())
						pipsToDraw.push_back(index == 1 ? 5 : 2);
					else
						pipsToDraw.push_back(pipFrames.at(index));

					break;
				}
			}

			if (pipsToDraw.size() <= static_cast<size_t>(i))
				pipsToDraw.push_back(emptyFrame);
		}
	}

	for (int pip : pipsToDraw)
	{
		DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, shape, pip,
			&position, rect, BlitterFlags::Centered | BlitterFlags::bf_400, 0, 0,
			ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);

		position.X += offset->Width;
		position.Y += yOffset;
	}

	return SkipGameDrawing;
}

DEFINE_HOOK(0x70A4FB, TechnoClass_DrawPips_SelfHealGain, 0x5)
{
	enum { SkipGameDrawing = 0x70A6C0 };

	GET(TechnoClass*, pThis, ECX);
	GET_STACK(Point2D*, pLocation, STACK_OFFSET(0x74, 0x4));
	GET_STACK(RectangleStruct*, pBounds, STACK_OFFSET(0x74, 0xC));

	TechnoExt::DrawSelfHealPips(pThis, pLocation, pBounds);

	return SkipGameDrawing;
}
