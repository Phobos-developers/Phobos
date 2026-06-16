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

DEFINE_HOOK_AGAIN(0x6D9134, TacticalClass_RenderLayers_DrawBefore, 0x5)// BuildingClass
DEFINE_HOOK(0x6D9076, TacticalClass_RenderLayers_DrawBefore, 0x5)// FootClass
{
	GET(TechnoClass*, pTechno, ESI);

	if (pTechno->IsSelected && Phobos::Config::EnableSelectBox)
	{
		const auto pTypeExt = TechnoExt::ExtMap.Find(pTechno)->TypeExtData;

		if (!pTypeExt->HealthBar_Hide && !pTypeExt->HideSelectBox)
		{
			GET(Point2D*, pLocation, EAX);
			TechnoExt::DrawSelectBox(pTechno, pLocation, &DSurface::ViewBounds, true);
		}
	}

	return 0;
}

DEFINE_HOOK(0x6F5E37, TechnoClass_DrawExtras_DrawHealthBar, 0x6)
{
	enum { Permanent = 0x6F5E41 };

	GET(TechnoClass*, pThis, EBP);

	if (pThis
		&& (pThis->IsMouseHovering || TechnoExt::ExtMap.Find(pThis)->TypeExtData->HealthBar_Permanent)
		&& !MapClass::Instance.IsLocationShrouded(pThis->GetCoords()))
	{
		return Permanent;
	}

	return 0;
}

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
	constexpr int defaultInfantryPipsLength = 8;
	constexpr int defaultUnitPipsLength = 17;

	HealthBarTypeClass* pHealthBar = nullptr;
	bool drawBuildingHealthBar = false;

	if (pBuilding)
	{
		if (AresHelper::CanUseAres && reinterpret_cast<DummyBuildingTypeExtHere*>(pBuilding->Type->align_E24)->Firestorm_Wall)
			return SkipDrawCode;

		if (pThis->IsSelected && Phobos::Config::EnableSelectBox && !pExt->TypeExtData->HideSelectBox)
			TechnoExt::DrawSelectBox(pThis, pLocation, pBounds);

		CoordStruct dimension {};
		pBuilding->Type->Dimension2(&dimension);
		dimension.X /= -2;
		dimension.Y /= 2;

		const auto drawAdjust = TacticalClass::CoordsToScreen(dimension);
		pHealthBar = pTypeExt->HealthBar.Get(RulesExt::Global()->Buildings_DefaultHealthBar);
		drawBuildingHealthBar = !pHealthBar->PipBrdShape.isset() && !pHealthBar->IsAnimated;

		if (drawBuildingHealthBar)
			position += drawAdjust;
		else
			position.Y += drawAdjust.Y / 2;

		dimension.Y = -dimension.Y;
		const auto drawStart = TacticalClass::CoordsToScreen(dimension);

		dimension.Z = 0;
		dimension.Y = -dimension.Y;
		pipsAdjust = TacticalClass::CoordsToScreen(dimension);

		if (drawBuildingHealthBar)
			pipsLength = (drawAdjust.Y - drawStart.Y) >> 1;
		else
			pipsLength = pHealthBar->PipsLength.Get(defaultUnitPipsLength);
	}
	else
	{
		pHealthBar = pTypeExt->HealthBar.Get(RulesExt::Global()->DefaultHealthBar);

		pipsAdjust = Point2D { -10, 10 };
		pipsLength = pHealthBar->PipsLength.Get(whatAmI == InfantryClass::AbsID ? defaultInfantryPipsLength : defaultUnitPipsLength);
	}

	__assume(pThis != nullptr);
	const auto pOwner = pThis->Owner;
	const bool isAllied = pOwner->IsAlliedWith(HouseClass::CurrentPlayer);

	if (!RulesClass::Instance->EnemyHealth && !HouseClass::IsCurrentPlayerObserver() && !isAllied)
		return SkipDrawCode;

	if (!pTypeExt->HealthBar_HidePips)
	{
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
	}

	TechnoExt::ProcessDigitalDisplays(pThis, pipsLength, pLocation);

	if (AresHelper::CanUseAres && reinterpret_cast<DummyTechnoExtHere*>(pThis->align_154)->DriverKilled)
		return SkipDrawCode;

	const bool showPipScale = TechnoExt::ExtMap.Find(pThis)->TypeExtData->HealthBar_Permanent_PipScale;

	if (showPipScale || pThis->IsMouseHovering || pThis->IsSelected)
	{
		const bool canShowPips = isAllied || pThis->DisplayProductionTo.Contains(HouseClass::CurrentPlayer) || HouseClass::IsCurrentPlayerObserver();

		if (canShowPips || (pBuilding && pBuilding->Type->CanBeOccupied) || pThis->GetTechnoType()->PipsDrawForAll)
		{
			Point2D pipsLocation = *pLocation + pipsAdjust;
			pThis->DrawPipScalePips(&pipsLocation, pLocation, pBounds);
		}
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
	const bool isBuilding = pThis->WhatAmI() == AbstractType::Building;
	auto const pType = pThis->GetTechnoType();

	if (pType->PipScale == PipScale::Ammo)
	{
		if (isBuilding)
			size = RulesExt::Global()->Pips_Ammo_Buildings_Size;
		else
			size = RulesExt::Global()->Pips_Ammo_Size;

		size = TechnoTypeExt::ExtMap.Find(pType)->AmmoPipSize.Get(size);
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
	auto const pTypeExt = TechnoExt::ExtMap.Find(pThis)->TypeExtData;

	if (!pTypeExt->ShowSpawnsPips)
		return SkipGameDrawing;

	LEA_STACK(RectangleStruct*, offset, STACK_OFFSET(0x74, -0x24));
	GET_STACK(RectangleStruct*, rect, STACK_OFFSET(0x74, 0xC));
	GET_STACK(SHPStruct*, shape, STACK_OFFSET(0x74, -0x58));
	GET_STACK(const bool, isBuilding, STACK_OFFSET(0x74, -0x61));
	GET(const int, maxSpawnsCount, EBX);

	const int currentSpawnsCount = pThis->SpawnManager->CountDockedSpawns();
	auto const pipOffset = pTypeExt->SpawnsPipOffset.Get();
	Point2D position = { offset->X + pipOffset.X, offset->Y + pipOffset.Y };
	Point2D size;

	if (isBuilding)
		size = pTypeExt->SpawnsPipSize.Get(RulesExt::Global()->Pips_Generic_Buildings_Size);
	else
		size = pTypeExt->SpawnsPipSize.Get(RulesExt::Global()->Pips_Generic_Size);

	for (int i = 0; i < maxSpawnsCount; i++)
	{
		const int frame = i < currentSpawnsCount ? pTypeExt->SpawnsPipFrame : pTypeExt->EmptySpawnsPipFrame;

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
	GET(const int, pipWrap, EBX);
	GET_STACK(const int, pipCount, STACK_OFFSET(0x74, -0x54));
	GET_STACK(const int, maxPips, STACK_OFFSET(0x74, -0x60));
	GET(const int, yOffset, ESI);

	auto const pTypeExt = TechnoExt::ExtMap.Find(pThis)->TypeExtData;
	auto const pipOffset = pTypeExt->AmmoPipOffset.Get();
	const int offsetWidth = offset->Width;
	Point2D position = { offset->X + pipOffset.X, offset->Y + pipOffset.Y };

	if (pipWrap > 0)
	{
		const int ammo = pThis->Ammo;
		const int levels = maxPips / pipWrap - 1;
		const int startFrame = pTypeExt->AmmoPipWrapStartFrame;

		for (int i = 0; i < pipWrap; i++)
		{
			int frame = startFrame;

			if (levels >= 0)
			{
				int counter = i + pipWrap * levels;
				int frameCounter = levels;
				bool calculateFrame = true;

				while (counter >= ammo)
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

			position.X += offsetWidth;
			position.Y += yOffset;

			DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS2_SHP,
				frame, &position, rect, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
		}
	}
	else
	{
		const int ammoFrame = pTypeExt->AmmoPipFrame;
		const int emptyFrame = pTypeExt->EmptyAmmoPipFrame;

		for (int i = 0; i < maxPips; i++)
		{
			if (i >= pipCount && emptyFrame < 0)
				break;

			const int frame = i >= pipCount ? emptyFrame : ammoFrame;

			DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS2_SHP,
				frame, &position, rect, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);

			position.X += offsetWidth;
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
	GET_STACK(const int, maxPips, STACK_OFFSET(0x74, -0x60));
	GET(const int, yOffset, ESI);

	Point2D position = { offset->X, offset->Y };
	const int totalStorage = pThis->GetTechnoType()->Storage;

	std::vector<int> pipsToDraw;
	pipsToDraw.reserve(maxPips);

	bool isWeeder = false;
	auto const whatAmI = pThis->WhatAmI();

	switch (whatAmI)
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
		const int fullWeedFrames = whatAmI == AbstractType::Building
			? static_cast<int>(pThis->Owner->GetWeedStoragePercentage() * maxPips + 0.5)
			: static_cast<int>(pThis->Tiberium.GetTotalAmount() / totalStorage * maxPips + 0.5);

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
		const int count = TiberiumClass::Array.Count;
		std::vector<int> tiberiumPipCounts(count);

		for (size_t i = 0; i < tiberiumPipCounts.size(); i++)
		{
			tiberiumPipCounts[i] = static_cast<int>(pThis->Tiberium.GetAmount(i) / totalStorage * maxPips + 0.5);
		}

		auto const rawPipOrder = RulesExt::Global()->Pips_Tiberiums_DisplayOrder.empty() ? std::vector<int>{ 0, 2, 3, 1 } : RulesExt::Global()->Pips_Tiberiums_DisplayOrder;
		auto const& pipFrames = RulesExt::Global()->Pips_Tiberiums_Frames;
		int const emptyFrame = RulesExt::Global()->Pips_Tiberiums_EmptyFrame;

		std::vector<int> pipOrder;
		pipOrder.reserve(count);

		// First make a new vector, removing all the duplicate and invalid tiberiums
		for (int index : rawPipOrder)
		{
			if (std::find(pipOrder.begin(), pipOrder.end(), index) == pipOrder.end()
				&& index >= 0 && index < count)
			{
				pipOrder.push_back(index);
			}
		}

		// Then add any tiberium types that are missing
		for (int i = 0; i < count; i++)
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

	const int offsetWidth = offset->Width;

	for (const int pip : pipsToDraw)
	{
		DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, shape, pip,
			&position, rect, BlitterFlags::Centered | BlitterFlags::bf_400, 0, 0,
			ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);

		position.X += offsetWidth;
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
