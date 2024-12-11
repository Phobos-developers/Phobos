#include <AircraftTypeClass.h>
#include <SpawnManagerClass.h>
#include <TiberiumClass.h>
#include "Body.h"

DEFINE_HOOK(0x6F64A9, TechnoClass_DrawHealthBar_Hide, 0x5)
{
	GET(TechnoClass*, pThis, ECX);
	auto pTypeData = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	if (pTypeData->HealthBar_Hide)
		return 0x6F6AB6;

	return 0;
}

DEFINE_HOOK(0x6F65D1, TechnoClass_DrawHealthBar_Buildings, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	GET(int, length, EBX);
	GET_STACK(Point2D*, pLocation, STACK_OFFSET(0x4C, 0x4));
	UNREFERENCED_PARAMETER(pLocation); // choom thought he was clever and recomputed the same shit again and again
	GET_STACK(RectangleStruct*, pBound, STACK_OFFSET(0x4C, 0x8));

	const auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (const auto pShieldData = pExt->Shield.get())
	{
		if (pShieldData->IsAvailable() && !pShieldData->IsBrokenAndNonRespawning())
			pShieldData->DrawShieldBar_Building(length, pBound);
	}

	TechnoExt::ProcessDigitalDisplays(pThis);

	return 0;
}

DEFINE_HOOK(0x6F683C, TechnoClass_DrawHealthBar_Units, 0x7)
{
	GET(FootClass*, pThis, ESI);
	GET_STACK(Point2D*, pLocation, STACK_OFFSET(0x4C, 0x4));
	UNREFERENCED_PARAMETER(pLocation);
	GET_STACK(RectangleStruct*, pBound, STACK_OFFSET(0x4C, 0x8));

	const auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (const auto pShieldData = pExt->Shield.get())
	{
		if (pShieldData->IsAvailable() && !pShieldData->IsBrokenAndNonRespawning())
		{
			const int length = pThis->WhatAmI() == AbstractType::Infantry ? 8 : 17;
			pShieldData->DrawShieldBar_Other(length, pBound);
		}
	}

	TechnoExt::ProcessDigitalDisplays(pThis);

	return 0;
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
		std::vector<int> tiberiumPipCounts(TiberiumClass::Array->Count);

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
				index >= 0 && index < TiberiumClass::Array->Count)
			{
				pipOrder.push_back(index);
			}
		}

		// Then add any tiberium types that are missing
		for (int i = 0; i < TiberiumClass::Array->Count; i++)
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
