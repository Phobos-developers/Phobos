#include "Body.h"

#include <TacticalClass.h>
#include <SpawnManagerClass.h>

#include <Utilities/EnumFunctions.h>

void TechnoExt::DrawSelfHealPips(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds)
{
	if (!RulesExt::Global()->GainSelfHealAllowMultiplayPassive && pThis->Owner->Type->MultiplayPassive)
		return;

	bool drawPip = false;
	bool isInfantryHeal = false;
	int selfHealFrames = 0;

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pTypeExt->SelfHealGainType.isset() && pTypeExt->SelfHealGainType.Get() == SelfHealGainType::NoHeal)
		return;

	bool hasInfantrySelfHeal = pTypeExt->SelfHealGainType.isset() && pTypeExt->SelfHealGainType.Get() == SelfHealGainType::Infantry;
	bool hasUnitSelfHeal = pTypeExt->SelfHealGainType.isset() && pTypeExt->SelfHealGainType.Get() == SelfHealGainType::Units;
	bool isOrganic = false;

	if (pThis->WhatAmI() == AbstractType::Infantry || (pThis->GetTechnoType()->Organic && pThis->WhatAmI() == AbstractType::Unit))
		isOrganic = true;

	if (pThis->Owner->InfantrySelfHeal > 0 && (hasInfantrySelfHeal || (isOrganic && !hasUnitSelfHeal)))
	{
		drawPip = true;
		selfHealFrames = RulesClass::Instance->SelfHealInfantryFrames;
		isInfantryHeal = true;
	}
	else if (pThis->Owner->UnitsSelfHeal > 0 && (hasUnitSelfHeal || (pThis->WhatAmI() == AbstractType::Unit && !isOrganic)))
	{
		drawPip = true;
		selfHealFrames = RulesClass::Instance->SelfHealUnitFrames;
	}

	if (drawPip)
	{
		Valueable<Point2D> pipFrames;
		bool isSelfHealFrame = false;
		int xOffset = 0;
		int yOffset = 0;

		if (Unsorted::CurrentFrame % selfHealFrames <= 5
			&& pThis->Health < pThis->GetTechnoType()->Strength)
		{
			isSelfHealFrame = true;
		}

		if (pThis->WhatAmI() == AbstractType::Unit || pThis->WhatAmI() == AbstractType::Aircraft)
		{
			auto& offset = RulesExt::Global()->Pips_SelfHeal_Units_Offset.Get();
			pipFrames = RulesExt::Global()->Pips_SelfHeal_Units;
			xOffset = offset.X;
			yOffset = offset.Y + pThis->GetTechnoType()->PixelSelectionBracketDelta;
		}
		else if (pThis->WhatAmI() == AbstractType::Infantry)
		{
			auto& offset = RulesExt::Global()->Pips_SelfHeal_Infantry_Offset.Get();
			pipFrames = RulesExt::Global()->Pips_SelfHeal_Infantry;
			xOffset = offset.X;
			yOffset = offset.Y + pThis->GetTechnoType()->PixelSelectionBracketDelta;
		}
		else
		{
			auto pType = static_cast<BuildingClass*>(pThis)->Type;
			int fHeight = pType->GetFoundationHeight(false);
			int yAdjust = -Unsorted::CellHeightInPixels / 2;

			auto& offset = RulesExt::Global()->Pips_SelfHeal_Buildings_Offset.Get();
			pipFrames = RulesExt::Global()->Pips_SelfHeal_Buildings;
			xOffset = offset.X + Unsorted::CellWidthInPixels / 2 * fHeight;
			yOffset = offset.Y + yAdjust * fHeight + pType->Height * yAdjust;
		}

		int pipFrame = isInfantryHeal ? pipFrames.Get().X : pipFrames.Get().Y;

		Point2D position = { pLocation->X + xOffset, pLocation->Y + yOffset };

		auto flags = BlitterFlags::bf_400 | BlitterFlags::Centered;

		if (isSelfHealFrame)
			flags = flags | BlitterFlags::Darken;

		DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS_SHP,
		pipFrame, &position, pBounds, flags, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}
}

void TechnoExt::DrawInsignia(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds)
{
	Point2D offset = *pLocation;

	SHPStruct* pShapeFile = FileSystem::PIPS_SHP;
	int defaultFrameIndex = -1;

	auto pTechnoType = pThis->GetTechnoType();
	auto pOwner = pThis->Owner;

	if (pThis->IsDisguised() && !pThis->IsClearlyVisibleTo(HouseClass::CurrentPlayer) && !(HouseClass::IsCurrentPlayerObserver()
		|| EnumFunctions::CanTargetHouse(RulesExt::Global()->DisguiseBlinkingVisibility, HouseClass::CurrentPlayer, pOwner)))
	{
		if (auto const pType = TechnoTypeExt::GetTechnoType(pThis->Disguise))
		{
			pTechnoType = pType;
			pOwner = pThis->DisguisedAsHouse;
		}
	}

	TechnoTypeExt::ExtData* pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);

	bool isVisibleToPlayer = (pOwner && pOwner->IsAlliedWith(HouseClass::CurrentPlayer))
		|| HouseClass::IsCurrentPlayerObserver()
		|| pTechnoTypeExt->Insignia_ShowEnemy.Get(RulesExt::Global()->EnemyInsignia);

	if (!isVisibleToPlayer)
		return;

	bool isCustomInsignia = false;

	if (SHPStruct* pCustomShapeFile = pTechnoTypeExt->Insignia.Get(pThis))
	{
		pShapeFile = pCustomShapeFile;
		defaultFrameIndex = 0;
		isCustomInsignia = true;
	}

	VeterancyStruct* pVeterancy = &pThis->Veterancy;
	auto insigniaFrames = pTechnoTypeExt->InsigniaFrames.Get();
	int insigniaFrame = insigniaFrames.X;
	int frameIndex = pTechnoTypeExt->InsigniaFrame.Get(pThis);

	if (pTechnoType->Gunner)
	{
		int weaponIndex = pThis->CurrentWeaponNumber;

		if (auto const pCustomShapeFile = pTechnoTypeExt->Insignia_Weapon[weaponIndex].Get(pThis))
		{
			pShapeFile = pCustomShapeFile;
			defaultFrameIndex = 0;
			isCustomInsignia = true;
		}

		int frame = pTechnoTypeExt->InsigniaFrame_Weapon[weaponIndex].Get(pThis);

		if (frame != -1)
			frameIndex = frame;

		auto& frames = pTechnoTypeExt->InsigniaFrames_Weapon[weaponIndex];

		if (frames != Vector3D<int>(-1, -1, -1))
			insigniaFrames = frames;
	}

	if (pVeterancy->IsVeteran())
	{
		defaultFrameIndex = !isCustomInsignia ? 14 : defaultFrameIndex;
		insigniaFrame = insigniaFrames.Y;
	}
	else if (pVeterancy->IsElite())
	{
		defaultFrameIndex = !isCustomInsignia ? 15 : defaultFrameIndex;
		insigniaFrame = insigniaFrames.Z;
	}

	frameIndex = frameIndex == -1 ? insigniaFrame : frameIndex;

	if (frameIndex == -1)
		frameIndex = defaultFrameIndex;

	if (frameIndex != -1 && pShapeFile)
	{
		switch (pThis->WhatAmI())
		{
		case AbstractType::Infantry:
			offset += RulesExt::Global()->DrawInsignia_AdjustPos_Infantry;
			break;
		case AbstractType::Building:
			if (RulesExt::Global()->DrawInsignia_AdjustPos_BuildingsAnchor.isset())
				offset = GetBuildingSelectBracketPosition(static_cast<BuildingClass*>(pThis)->Type, pLocation, RulesExt::Global()->DrawInsignia_AdjustPos_BuildingsAnchor) + RulesExt::Global()->DrawInsignia_AdjustPos_Buildings;
			else
				offset += RulesExt::Global()->DrawInsignia_AdjustPos_Buildings;
			break;
		default:
			offset += RulesExt::Global()->DrawInsignia_AdjustPos_Units;
			break;
		}

		DSurface::Temp->DrawSHP(
			FileSystem::PALETTE_PAL, pShapeFile, frameIndex, &offset, pBounds, BlitterFlags(0xE00), 0, -2, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}

	return;
}




Point2D TechnoExt::GetScreenLocation(TechnoClass* pThis)
{
	return TacticalClass::Instance->CoordsToClient(pThis->GetCoords()).first;
}

Point2D TechnoExt::GetFootSelectBracketPosition(int length, Point2D* pLocation, Anchor anchor)
{
	Point2D position = *pLocation;

	RectangleStruct bracketRect =
	{
		position.X - length + (length == 8) + 1,
		position.Y - 28 + (length == 8),
		length * 2,
		length * 3
	};

	return anchor.OffsetPosition(bracketRect);
}

Point2D TechnoExt::GetBuildingSelectBracketPosition(BuildingTypeClass* pType, Point2D* pLocation, BuildingSelectBracketPosition bracketPosition)
{
	Point2D position = *pLocation;
	CoordStruct dim2 = CoordStruct::Empty;
	pType->Dimension2(&dim2);
	dim2 = { -dim2.X / 2, dim2.Y / 2, dim2.Z };
	Point2D positionFix = TacticalClass::CoordsToScreen(dim2);

	const int foundationWidth = pType->GetFoundationWidth();
	const int foundationHeight = pType->GetFoundationHeight(false);
	const int height = pType->Height * 12;
	const int lengthW = foundationWidth * 7 + foundationWidth / 2;
	const int lengthH = foundationHeight * 7 + foundationHeight / 2;

	position.X += positionFix.X + 3 + lengthH * 4;
	position.Y += positionFix.Y + 4 - lengthH * 2;

	switch (bracketPosition)
	{
	case BuildingSelectBracketPosition::Top:
		break;
	case BuildingSelectBracketPosition::LeftTop:
		position.X -= lengthH * 4;
		position.Y += lengthH * 2;
		break;
	case BuildingSelectBracketPosition::LeftBottom:
		position.X -= lengthH * 4;
		position.Y += lengthH * 2 + height;
		break;
	case BuildingSelectBracketPosition::Bottom:
		position.Y += lengthW * 2 + lengthH * 2 + height;
		break;
	case BuildingSelectBracketPosition::RightBottom:
		position.X += lengthW * 4;
		position.Y += lengthW * 2 + height;
		break;
	case BuildingSelectBracketPosition::RightTop:
		position.X += lengthW * 4;
		position.Y += lengthW * 2;
	default:
		break;
	}

	return position;
}

int TechnoExt::HealthBar_GetPip(Vector3D<int> const& pips, double percentage, const bool isBuilding)
{
	if (percentage > RulesClass::Instance->ConditionYellow && pips.X != -1)
		return pips.X;
	else if (percentage <= RulesClass::Instance->ConditionYellow && percentage > RulesClass::Instance->ConditionRed && (pips.Y != -1 || pips.X != -1))
		return pips.Y == -1 ? pips.X : pips.Y;
	else if (pips.Z != -1 || pips.X != -1)
		return pips.Z == -1 ? pips.X : pips.Z;

	return isBuilding ? 5 : 16;
}

int TechnoExt::HealthBar_GetPipAmount(double percentage, int pipsLength)
{
	return std::clamp(static_cast<int>(percentage * pipsLength), 1, pipsLength);
}

void TechnoExt::DrawBuildingBar(ConvertClass* pPalette, SHPStruct* pShape, Point2D* pLocation, RectangleStruct* pBounds, Point2D interval, const int pipsTotal, const int pipsLength, const int emptyFrame, const int frame)
{
	if (pipsTotal > 0)
	{
		Point2D drawPoint = *pLocation + Point2D { 3 + 4 * pipsLength, 4 - 2 * pipsLength };
		BlitterFlags blitterFlags = BlitterFlags::Centered | BlitterFlags::bf_400;

		for (int idx = pipsTotal; idx; --idx, drawPoint += interval)
			DSurface::Composite->DrawSHP(pPalette, pShape, frame, &drawPoint, pBounds, blitterFlags, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}

	if (pipsTotal < pipsLength)
	{
		int idx = pipsLength - pipsTotal, deltaX = 4 * pipsTotal, deltaY = -2 * pipsTotal;
		Point2D drawPoint = *pLocation + Point2D { 3 + 4 * pipsLength - deltaX, 4 - 2 * pipsLength - deltaY };
		BlitterFlags blitterFlags = BlitterFlags::Centered | BlitterFlags::bf_400;

		for (; idx; --idx, drawPoint += interval)
			DSurface::Composite->DrawSHP(pPalette, pShape, emptyFrame, &drawPoint, pBounds, blitterFlags, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}
}

void TechnoExt::DrawOtherBar(ConvertClass* pBrdPalette, SHPStruct* pBrdShape, ConvertClass* pPipsPalette, SHPStruct* pPipsShape, Point2D* pLocation, RectangleStruct* pBounds, const int brdXOffset, Point2D interval, const int pipsTotal, const int brdFrame, const int frame)
{
	if (brdFrame >= 0)
	{
		Point2D drawPoint = *pLocation + Point2D { brdXOffset + 16, -1 };
		BlitterFlags blitterFlags = BlitterFlags::Centered | BlitterFlags::bf_400 | BlitterFlags::Alpha;

		DSurface::Composite->DrawSHP(pBrdPalette, pBrdShape, brdFrame, &drawPoint, pBounds, blitterFlags, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}

	Point2D drawPoint = *pLocation;
	BlitterFlags blitterFlags = BlitterFlags::Centered | BlitterFlags::bf_400;

	for (int idx = 0; idx < pipsTotal; ++idx, drawPoint += interval)
		DSurface::Composite->DrawSHP(pPipsPalette, pPipsShape, frame, &drawPoint, pBounds, blitterFlags, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
}

void TechnoExt::DrawHealthBar_Building(TechnoClass* pThis, HealthBarTypeClass* pHealthBar, int pipsLength, Point2D* pLocation, RectangleStruct* pBounds)
{
	const auto pPipsPalette = pHealthBar->PipsPalette.GetOrDefaultConvert(FileSystem::PALETTE_PAL);
	const auto pPipsShape = pHealthBar->PipsShape.Get();

	auto position = *pLocation;

	const auto interval = pHealthBar->PipsInterval_Building.Get();
	const int emptyFrame = pHealthBar->PipsEmpty.Get(RulesExt::Global()->Pips_Building_Empty);

	const auto& pips = pHealthBar->Pips_Building.Get(RulesExt::Global()->Pips_Building);

	const double percentage = pThis->GetHealthPercentage();
	const int frame = TechnoExt::HealthBar_GetPip(pips, percentage, true);
	const int pipsTotal = percentage > 0.0 ? TechnoExt::HealthBar_GetPipAmount(percentage, pipsLength) : 0;
	TechnoExt::DrawBuildingBar(pPipsPalette, pPipsShape, &position, pBounds, interval, pipsTotal, pipsLength, emptyFrame, frame);
}

void TechnoExt::DrawHealthBar_Other(TechnoClass* pThis, HealthBarTypeClass* pHealthBar, int pipsLength, Point2D* pLocation, RectangleStruct* pBounds)
{
	const auto pBrdPalette = pHealthBar->PipBrdPalette.GetOrDefaultConvert(FileSystem::PALETTE_PAL);
	const auto pBrdShape = pHealthBar->PipBrdShape.Get(FileSystem::PIPBRD_SHP);
	const auto pPipsPalette = pHealthBar->PipsPalette.GetOrDefaultConvert(FileSystem::PALETTE_PAL);
	const auto pPipsShape = pHealthBar->PipsShape.Get();

	const auto pipsInterval = pHealthBar->PipsInterval.Get();
	auto position = *pLocation + Point2D { pHealthBar->XOffset - pipsLength * pipsInterval.X / 2, pThis->GetTechnoType()->PixelSelectionBracketDelta - 24 };

	const auto whatAmI = pThis->WhatAmI();

	if (whatAmI != InfantryClass::AbsID)
		position += { 2, -1 };

	const int xOffset = pHealthBar->PipBrdXOffset.Get();

	const auto& pips = pHealthBar->Pips.Get(RulesExt::Global()->Pips);

	const double percentage = pThis->GetHealthPercentage();
	const int brdFrame = pThis->IsSelected ? pHealthBar->PipBrd.Get(whatAmI == InfantryClass::AbsID ? 1 : 0) : -1;

	const int pipsTotal = percentage > 0.0 ? TechnoExt::HealthBar_GetPipAmount(percentage, pipsLength) : 0;
	const int frame = TechnoExt::HealthBar_GetPip(pips, percentage, false);
	TechnoExt::DrawOtherBar(pBrdPalette, pBrdShape, pPipsPalette, pPipsShape, &position, pBounds, xOffset, pipsInterval, pipsTotal, brdFrame, frame);
}

void TechnoExt::ProcessDigitalDisplays(TechnoClass* pThis, Point2D* pLocation)
{
	if (!Phobos::Config::DigitalDisplay_Enable)
		return;

	const auto pType = pThis->GetTechnoType();
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	if (pTypeExt->DigitalDisplay_Disable)
		return;

	const auto pExt = TechnoExt::ExtMap.Find(pThis);
	int length = 17;
	ValueableVector<DigitalDisplayTypeClass*>* pDisplayTypes = nullptr;

	if (!pTypeExt->DigitalDisplayTypes.empty())
	{
		pDisplayTypes = &pTypeExt->DigitalDisplayTypes;
	}
	else
	{
		switch (pThis->WhatAmI())
		{
		case AbstractType::Building:
		{
			pDisplayTypes = &RulesExt::Global()->Buildings_DefaultDigitalDisplayTypes;
			const auto pBuildingType = static_cast<BuildingTypeClass*>(pThis->GetTechnoType());
			const int height = pBuildingType->GetFoundationHeight(false);
			length = height * 7 + height / 2;
			break;
		}
		case AbstractType::Infantry:
		{
			pDisplayTypes = &RulesExt::Global()->Infantry_DefaultDigitalDisplayTypes;
			length = 8;
			break;
		}
		case AbstractType::Unit:
		{
			pDisplayTypes = &RulesExt::Global()->Vehicles_DefaultDigitalDisplayTypes;
			break;
		}
		case AbstractType::Aircraft:
		{
			pDisplayTypes = &RulesExt::Global()->Aircraft_DefaultDigitalDisplayTypes;
			break;
		}
		default:
			return;
		}
	}

	for (DigitalDisplayTypeClass*& pDisplayType : *pDisplayTypes)
	{
		if (HouseClass::IsCurrentPlayerObserver() && !pDisplayType->VisibleToHouses_Observer)
			continue;

		if (!HouseClass::IsCurrentPlayerObserver() && !EnumFunctions::CanTargetHouse(pDisplayType->VisibleToHouses, pThis->Owner, HouseClass::CurrentPlayer))
			continue;

		int value = -1;
		int maxValue = -1;

		GetValuesForDisplay(pThis, pDisplayType->InfoType, value, maxValue);

		if (value == -1 || maxValue == -1)
			continue;

		if (pDisplayType->ValueScaleDivisor > 1)
		{
			value = Math::max(value / pDisplayType->ValueScaleDivisor, value != 0 ? 1 : 0);
			maxValue = Math::max(maxValue / pDisplayType->ValueScaleDivisor, maxValue != 0 ? 1 : 0);
		}

		const bool isBuilding = pThis->WhatAmI() == AbstractType::Building;
		const bool isInfantry = pThis->WhatAmI() == AbstractType::Infantry;
		const bool hasShield = pExt->Shield != nullptr && !pExt->Shield->IsBrokenAndNonRespawning();
		Point2D position = pThis->WhatAmI() == AbstractType::Building ?
			GetBuildingSelectBracketPosition(static_cast<BuildingClass*>(pThis)->Type, pLocation, pDisplayType->AnchorType_Building)
			: GetFootSelectBracketPosition(length, pLocation, pDisplayType->AnchorType);
		position.Y += pType->PixelSelectionBracketDelta;

		if (pDisplayType->InfoType == DisplayInfoType::Shield)
			position.Y += pExt->Shield->GetType()->BracketDelta;

		pDisplayType->Draw(position, length, value, maxValue, isBuilding, isInfantry, hasShield);
	}
}

void TechnoExt::GetValuesForDisplay(TechnoClass* pThis, DisplayInfoType infoType, int& value, int& maxValue)
{
	const auto pType = pThis->GetTechnoType();
	const auto pExt = TechnoExt::ExtMap.Find(pThis);

	switch (infoType)
	{
	case DisplayInfoType::Health:
	{
		value = pThis->Health;
		maxValue = pType->Strength;
		break;
	}
	case DisplayInfoType::Shield:
	{
		if (pExt->Shield == nullptr || pExt->Shield->IsBrokenAndNonRespawning())
			return;

		value = pExt->Shield->GetHP();
		maxValue = pExt->Shield->GetType()->Strength.Get();
		break;
	}
	case DisplayInfoType::Ammo:
	{
		if (pType->Ammo <= 0)
			return;

		value = pThis->Ammo;
		maxValue = pType->Ammo;
		break;
	}
	case DisplayInfoType::MindControl:
	{
		if (pThis->CaptureManager == nullptr)
			return;

		value = pThis->CaptureManager->ControlNodes.Count;
		maxValue = pThis->CaptureManager->MaxControlNodes;
		break;
	}
	case DisplayInfoType::Spawns:
	{
		if (pThis->SpawnManager == nullptr || pType->Spawns == nullptr || pType->SpawnsNumber <= 0)
			return;

		value = pThis->SpawnManager->CountAliveSpawns();
		maxValue = pType->SpawnsNumber;
		break;
	}
	case DisplayInfoType::Passengers:
	{
		if (pType->Passengers <= 0)
			return;

		value = pThis->Passengers.NumPassengers;
		maxValue = pType->Passengers;
		break;
	}
	case DisplayInfoType::Tiberium:
	{
		if (pType->Storage <= 0)
			return;

		value = static_cast<int>(pThis->Tiberium.GetTotalAmount());
		maxValue = pType->Storage;
		break;
	}
	case DisplayInfoType::Experience:
	{
		value = static_cast<int>(pThis->Veterancy.Veterancy * RulesClass::Instance->VeteranRatio * pType->GetCost());
		maxValue = static_cast<int>(2.0 * RulesClass::Instance->VeteranRatio * pType->GetCost());
		break;
	}
	case DisplayInfoType::Occupants:
	{
		if (pThis->WhatAmI() != AbstractType::Building)
			return;

		const auto pBuildingType = abstract_cast<BuildingTypeClass*>(pType);
		const auto pBuilding = abstract_cast<BuildingClass*>(pThis);

		if (!pBuildingType->CanBeOccupied)
			return;

		value = pBuilding->Occupants.Count;
		maxValue = pBuildingType->MaxNumberOccupants;
		break;
	}
	case DisplayInfoType::GattlingStage:
	{
		if (!pType->IsGattling)
			return;

		value = pThis->CurrentGattlingStage;
		maxValue = pType->WeaponStages;
		break;
	}
	default:
	{
		value = pThis->Health;
		maxValue = pType->Strength;
		break;
	}
	}
}
