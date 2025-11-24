#include <AircraftTypeClass.h>
#include <SpawnManagerClass.h>
#include <TiberiumClass.h>
#include <TacticalClass.h>
#include "Body.h"

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

	if (pThis && (pThis->IsMouseHovering || TechnoExt::ExtMap.Find(pThis)->TypeExtData->HealthBar_Permanent)
		&& !MapClass::Instance.IsLocationShrouded(pThis->GetCoords()))
	{
		return Permanent;
	}

	return 0;
}

/*DEFINE_HOOK(0x6F64A9, TechnoClass_DrawHealthBar_Hide, 0x5) //replaced with new bars
{
	enum { SkipDraw = 0x6F6AB6 };

	GET(TechnoClass*, pThis, ECX);

	const auto pTypeData = TechnoExt::ExtMap.Find(pThis)->TypeExtData;

	if (pTypeData->HealthBar_Hide)
		return SkipDraw;

	return 0;
}*/

DEFINE_HOOK(0x6F6637, TechnoClass_DrawHealthBar_HideBuildingsPips, 0x5)
{
	enum { SkipDrawPips = 0x6F677D };

	GET(TechnoClass*, pThis, ESI);

	const bool hidePips = TechnoExt::ExtMap.Find(pThis)->TypeExtData->HealthBar_HidePips;

	return hidePips ? SkipDrawPips : 0;
}

DEFINE_HOOK_AGAIN(0x6F6A58, TechnoClass_DrawHealthBar_PermanentPipScale, 0x6)	// DrawOther
DEFINE_HOOK(0x6F67E8, TechnoClass_DrawHealthBar_PermanentPipScale, 0xA)			// DrawBuilding
{
	enum { Permanent = 0x6F6AB6 };

	GET(TechnoClass*, pThis, ESI);

	const bool showPipScale = TechnoExt::ExtMap.Find(pThis)->TypeExtData->HealthBar_Permanent_PipScale;

	return !showPipScale && !pThis->IsMouseHovering && !pThis->IsSelected ? Permanent : 0;
}

DEFINE_HOOK(0x6F65D1, TechnoClass_DrawHealthBar_Buildings, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	GET(const int, length, EBX);
	GET_STACK(RectangleStruct*, pBound, STACK_OFFSET(0x4C, 0x8));

	const auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (pThis->IsSelected && Phobos::Config::EnableSelectBox && !pExt->TypeExtData->HideSelectBox)
	{
		GET_STACK(Point2D*, pLocation, STACK_OFFSET(0x4C, 0x4));
		UNREFERENCED_PARAMETER(pLocation); // choom thought he was clever and recomputed the same shit again and again
		TechnoExt::DrawSelectBox(pThis, pLocation, pBound);
	}

	if (const auto pShieldData = pExt->Shield.get())
	{
		if (pShieldData->IsAvailable() && !pShieldData->IsBrokenAndNonRespawning())
		{
			pShieldData->DrawShieldBar_Building(length, pBound);
		}
	}

	TechnoExt::ProcessDigitalDisplays(pThis);

	return 0;
}

DEFINE_HOOK(0x6F683C, TechnoClass_DrawHealthBar_Units, 0x7)
{
	enum { SkipDrawPips = 0x6F6A58 };

	GET(FootClass*, pThis, ESI);
	GET_STACK(Point2D*, pLocation, STACK_OFFSET(0x4C, 0x4));
	GET_STACK(RectangleStruct*, pBound, STACK_OFFSET(0x4C, 0x8));

	const auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (pThis->IsSelected && Phobos::Config::EnableSelectBox && !pExt->TypeExtData->HideSelectBox)
	{
		UNREFERENCED_PARAMETER(pLocation);
		TechnoExt::DrawSelectBox(pThis, pLocation, pBound);
	}

	if (const auto pShieldData = pExt->Shield.get())
	{
		if (pShieldData->IsAvailable() && !pShieldData->IsBrokenAndNonRespawning())
		{
			const int length = pThis->WhatAmI() == AbstractType::Infantry ? Unsorted::HealthBarSectionsInfantry : Unsorted::HealthBarSectionsOther;
			pShieldData->DrawShieldBar_Other(length, pBound);
		}
	}

	TechnoExt::ProcessDigitalDisplays(pThis);

	if (pExt->TypeExtData->HealthBar_HidePips)
	{
		R->EDI(pLocation);
		return SkipDrawPips;
	}

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

DEFINE_HOOK(0x6F64A9, TechnoClass_DrawHealthBar_HealthAndShieldBar, 0x5)
{
	enum { Skip = 0x6F6AB6 };

	GET(TechnoClass*, pThis, ECX);
	GET_STACK(Point2D*, pLocation, STACK_OFFSET(0x4C, 0x4));
	GET_STACK(RectangleStruct*, pBound, STACK_OFFSET(0x4C, 0x8));

	const auto pType = pThis->GetTechnoType();
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pTypeExt && pTypeExt->HealthBar_Hide)
		return Skip;

	//if (!pTypeExt->HealthBar_BarType)
	//	return 0;

	const auto pExt = TechnoExt::ExtMap.Find(pThis);
	const bool pThisIsInf = pThis->WhatAmI() == AbstractType::Infantry;
	const bool pThisIsBld = pThis->WhatAmI() == AbstractType::Building;
	ValueableVector<BarTypeClass*>* pBarTypes = nullptr;

	Point2D position = *pLocation;

	//if (pThisIsBld)
	//	position.Y -= (static_cast<BuildingClass*>(pThis)->Type->Height + 1) * Unsorted::CellHeightInPixels / 2;
	//else
	//	position.Y -= (pThisIsInf ? Unsorted::HealthBarYOffsetInfantry : Unsorted::HealthBarYOffsetOther) - pThis->GetTechnoType()->PixelSelectionBracketDelta;

	if (!pTypeExt->BarTypes.empty())
	{
		pBarTypes = &pTypeExt->BarTypes;
	}
	else
	{
		switch (pThis->WhatAmI())
		{
		case AbstractType::Building:
		{
			pBarTypes = &RulesExt::Global()->Buildings_DefaultBarTypes;
			const auto pBuildingType = static_cast<BuildingTypeClass*>(pType);
			position.Y -= (pBuildingType->Height + 1) * Unsorted::CellHeightInPixels / 2;
			break;
		}
		case AbstractType::Infantry:
		{
			pBarTypes = &RulesExt::Global()->Infantry_DefaultBarTypes;
			position.Y -= Unsorted::HealthBarYOffsetInfantry - pType->PixelSelectionBracketDelta;
			break;
		}
		case AbstractType::Unit:
		{
			pBarTypes = &RulesExt::Global()->Vehicles_DefaultBarTypes;
			position.Y -= Unsorted::HealthBarYOffsetOther - pType->PixelSelectionBracketDelta;
			break;
		}
		case AbstractType::Aircraft:
		{
			pBarTypes = &RulesExt::Global()->Aircraft_DefaultBarTypes;
			position.Y -= Unsorted::HealthBarYOffsetOther - pType->PixelSelectionBracketDelta;
			break;
		}
		default:
			break;
		}
	}

	if (pBarTypes->empty())
	{
		TechnoExt::ProcessDigitalDisplays(pThis);
		return 0;
	}

	for (BarTypeClass*& pBarType : *pBarTypes)
	{
		//if (HouseClass::IsCurrentPlayerObserver() && !pBarType->VisibleToHouses_Observer)
		//	continue;

		//if (!HouseClass::IsCurrentPlayerObserver() && !EnumFunctions::CanTargetHouse(pBarType->VisibleToHouses, pThis->Owner, HouseClass::CurrentPlayer))
		//	continue;

		//int value = -1;
		//int maxValue = -1;

		//GetValuesForDisplay(pThis, pBarType->InfoType, value, maxValue);

		//if (value == -1 || maxValue == -1)
		//	continue;

		//if (pBarType->ValueScaleDivisor > 1)
		//{
		//	value = Math::max(value / pBarType->ValueScaleDivisor, value != 0 ? 1 : 0);
		//	maxValue = Math::max(maxValue / pBarType->ValueScaleDivisor, maxValue != 0 ? 1 : 0);
		//}

		//const bool isBuilding = pThis->WhatAmI() == AbstractType::Building;
		//const bool isInfantry = pThis->WhatAmI() == AbstractType::Infantry;
		//const bool hasShield = pExt->Shield != nullptr && !pExt->Shield->IsBrokenAndNonRespawning();
		//Point2D position = pThis->WhatAmI() == AbstractType::Building ?
		//	GetBuildingSelectBracketPosition(pThis, pDisplayType->AnchorType_Building)
		//	: GetFootSelectBracketPosition(pThis, pDisplayType->AnchorType);
		//position.Y += pType->PixelSelectionBracketDelta;
		double pValuePercentage = 1.0;
		double pConditionYellow = 0.66;
		double pConditionRed = 0.33;

		switch (pBarType->InfoType)
		{
			case DisplayInfoType::Health:
			{
				pValuePercentage = pThis->GetHealthPercentage();
				pConditionYellow = RulesClass::Instance->ConditionYellow;
				pConditionRed = RulesClass::Instance->ConditionRed;
				break;
			}
			case DisplayInfoType::Shield:
			{
				position.Y += pExt->Shield->GetType()->BracketDelta;
				pValuePercentage = double(pExt->Shield->GetHP()) / pExt->Shield->GetType()->Strength.Get();
				pConditionYellow = pExt->Shield->GetType()->ConditionYellow;
				pConditionRed = pExt->Shield->GetType()->ConditionRed;
				break;
			}
			case DisplayInfoType::Ammo:
			{
				if (pType->Ammo > 0)
					pValuePercentage = double(pThis->Ammo) / pType->Ammo;

				break;
			}
			case DisplayInfoType::MindControl:
			{
				if (pThis->CaptureManager != nullptr)
					pValuePercentage = double(pThis->CaptureManager->ControlNodes.Count) / pThis->CaptureManager->MaxControlNodes;

				break;
			}
			case DisplayInfoType::Spawns:
			{
				if (pThis->SpawnManager == nullptr || pType->Spawns == nullptr || pType->SpawnsNumber <= 0)
					break;

				pValuePercentage = double(pThis->SpawnManager->CountAliveSpawns()) / pType->SpawnsNumber;
				break;
			}
			case DisplayInfoType::Passengers:
			{
				if (pType->Passengers > 0)
					pValuePercentage = double(pThis->Passengers.NumPassengers) / pType->Passengers;

				break;
			}
			case DisplayInfoType::Tiberium:
			{
				if (pType->Storage > 0)
					pValuePercentage = double(pThis->Tiberium.GetTotalAmount()) / pType->Storage;

				break;
			}
			case DisplayInfoType::Experience:
			{
				pValuePercentage = double(pThis->Veterancy.Veterancy * RulesClass::Instance->VeteranRatio * pType->GetCost()) / (2.0 * RulesClass::Instance->VeteranRatio * pType->GetCost());
				break;
			}
			case DisplayInfoType::Occupants:
			{
				if (pThis->WhatAmI() != AbstractType::Building)
					break;

				const auto pBuildingType = abstract_cast<BuildingTypeClass*>(pType);
				const auto pBuilding = abstract_cast<BuildingClass*>(pThis);

				if (!pBuildingType->CanBeOccupied)
					break;

				pValuePercentage = double(pBuilding->Occupants.Count) / pBuildingType->MaxNumberOccupants;
				break;
			}
			case DisplayInfoType::GattlingStage:
			{
				if (!pType->IsGattling)
					break;

				pValuePercentage = double(pThis->CurrentGattlingStage) / pType->WeaponStages;
				break;
			}
			default:
			{
				break;
			}
		}

		TechnoExt::DrawBar(pThis, pBarType, position, pBound, pValuePercentage, pConditionYellow, pConditionRed);
	}

	TechnoExt::ProcessDigitalDisplays(pThis);

	return Skip;
}
