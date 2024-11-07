#include "Body.h"

#include <TacticalClass.h>
#include <SpawnManagerClass.h>
#include <FactoryClass.h>
#include <SuperClass.h>
#include <Ext/SWType/Body.h>

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
				offset = GetBuildingSelectBracketPosition(pThis, RulesExt::Global()->DrawInsignia_AdjustPos_BuildingsAnchor) + RulesExt::Global()->DrawInsignia_AdjustPos_Buildings;
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

void TechnoExt::DrawFactoryProgress(BuildingClass* pThis, RectangleStruct* pBounds, Point2D basePosition)
{
	const RulesExt::ExtData* const pRulesExt = RulesExt::Global();

	if (!pRulesExt->FactoryProgressDisplay)
		return;

	const auto pType = pThis->Type;
	const auto pHouse = pThis->Owner;
	FactoryClass* pPrimaryFactory = nullptr;
	FactoryClass* pSecondaryFactory = nullptr;

	if (pHouse->IsControlledByHuman())
	{
		if (!pThis->IsPrimaryFactory)
			return;

		switch (pType->Factory)
		{
		case AbstractType::BuildingType:
			pPrimaryFactory = pHouse->GetPrimaryFactory(AbstractType::BuildingType, false, BuildCat::DontCare);
			pSecondaryFactory = pHouse->GetPrimaryFactory(AbstractType::BuildingType, false, BuildCat::Combat);
			break;
		case AbstractType::InfantryType:
			pPrimaryFactory = pHouse->GetPrimaryFactory(AbstractType::InfantryType, false, BuildCat::Combat);
			break;
		case AbstractType::UnitType:
			pPrimaryFactory = pHouse->GetPrimaryFactory(AbstractType::UnitType, pType->Naval, BuildCat::Combat);
			break;
		case AbstractType::AircraftType:
			pPrimaryFactory = pHouse->GetPrimaryFactory(AbstractType::AircraftType, false, BuildCat::Combat);
			break;
		default:
			return;
		}
	}
	else // AIs have no Primary factories
	{
		pPrimaryFactory = pThis->Factory;

		if (!pPrimaryFactory)
			return;
	}

	const bool havePrimary = pPrimaryFactory && pPrimaryFactory->Object;
	const bool haveSecondary = pSecondaryFactory && pSecondaryFactory->Object;

	if (!havePrimary && !haveSecondary)
		return;

	const auto maxLength = pType->GetFoundationHeight(false) * 15 >> 1;
	const auto location = basePosition + Point2D { 6, (3 + pType->PixelSelectionBracketDelta) } + pRulesExt->FactoryProgressDisplay_Offset.Get();

	if (havePrimary)
	{
		auto position = location;

		DrawFrameStruct pDraw
		{
			static_cast<int>((static_cast<double>(pPrimaryFactory->GetProgress()) / 54) * maxLength),
			pRulesExt->FactoryProgressDisplay_Pips,
			pRulesExt->ProgressDisplay_Buildings_PipsShape.Get(),
			0,
			-1,
			nullptr,
			maxLength,
			0,
			&position,
			pBounds
		};

		TechnoExt::DrawVanillaStyleBuildingBar(&pDraw);
	}

	if (haveSecondary)
	{
		auto position = havePrimary ? location + Point2D { 6, 3 } : location;

		DrawFrameStruct pDraw
		{
			static_cast<int>((static_cast<double>(pSecondaryFactory->GetProgress()) / 54) * maxLength),
			pRulesExt->FactoryProgressDisplay_Pips,
			pRulesExt->ProgressDisplay_Buildings_PipsShape.Get(),
			0,
			-1,
			nullptr,
			maxLength,
			0,
			&position,
			pBounds
		};

		TechnoExt::DrawVanillaStyleBuildingBar(&pDraw);
	}
}

void TechnoExt::DrawSuperProgress(BuildingClass* pThis, RectangleStruct* pBounds, Point2D basePosition)
{
	const auto pRulesExt = RulesExt::Global();

	if (!pRulesExt->MainSWProgressDisplay)
		return;

	const auto pType = pThis->Type;
	const auto pOwner = pThis->Owner;
	const auto superIndex = pType->SuperWeapon;
	const auto pSuper = (superIndex != -1) ? pOwner->Supers.GetItem(superIndex) : nullptr;

	if (!pSuper || !SWTypeExt::ExtMap.Find(pSuper->Type)->IsAvailable(pOwner))
		return;

	const auto maxLength = pType->GetFoundationHeight(false) * 15 >> 1;
	auto position = basePosition + Point2D { 6, (3 + pType->PixelSelectionBracketDelta) } + pRulesExt->MainSWProgressDisplay_Offset.Get();

	DrawFrameStruct pDraw
	{
		static_cast<int>((static_cast<double>(pSuper->AnimStage()) / 54) * maxLength),
		pRulesExt->MainSWProgressDisplay_Pips,
		pRulesExt->ProgressDisplay_Buildings_PipsShape.Get(),
		0,
		-1,
		nullptr,
		maxLength,
		0,
		&position,
		pBounds
	};

	TechnoExt::DrawVanillaStyleBuildingBar(&pDraw);
}

void TechnoExt::DrawIronCurtainProgress(TechnoClass* pThis, RectangleStruct* pBounds, Point2D basePosition, bool isBuilding, bool isInfantry)
{
	if (!pThis->IsIronCurtained())
		return;

	const auto pRulesExt = RulesExt::Global();

	if (!pRulesExt->InvulnerableDisplay)
		return;

	const auto timer = &pThis->IronCurtainTimer;

	if (isBuilding)
	{
		const auto pBuilding = static_cast<BuildingClass*>(pThis);
		const auto pType = pBuilding->Type;
		const auto maxLength = pType->GetFoundationHeight(false) * 15 >> 1;
		const auto offset = pRulesExt->InvulnerableDisplay_Buildings_Offset.Get();
		auto position = basePosition + Point2D { offset.X, pType->PixelSelectionBracketDelta + offset.Y };

		DrawFrameStruct pDraw
		{
			static_cast<int>((static_cast<double>(timer->GetTimeLeft()) / timer->TimeLeft) * maxLength + 0.99),
			(pThis->ForceShielded ? pRulesExt->InvulnerableDisplay_Buildings_Pips.Get().X : pRulesExt->InvulnerableDisplay_Buildings_Pips.Get().Y),
			pRulesExt->ProgressDisplay_Buildings_PipsShape.Get(),
			0,
			-1,
			nullptr,
			maxLength,
			-1,
			&position,
			pBounds
		};

		if (offset == Point2D::Empty && (pThis->IsSelected || pThis->IsMouseHovering)) // Layer fix
		{
			RulesClass* const pRules = RulesClass::Instance;
			const auto ratio = pBuilding->GetHealthPercentage();
			pDraw.MidLength = static_cast<int>(ratio * maxLength);
			pDraw.MidFrame = (ratio > pRules->ConditionYellow) ? 1 : (ratio > pRules->ConditionRed ? 2 : 4);
			pDraw.MidPipSHP = FileSystem::PIPS_SHP;
			pDraw.BrdFrame = 0;
		}

		TechnoExt::DrawVanillaStyleBuildingBar(&pDraw);
	}
	else
	{
		const int maxLength = isInfantry ? 8 : 17;
		auto position = basePosition + Point2D { 0, pThis->GetTechnoType()->PixelSelectionBracketDelta + 2 } + pRulesExt->InvulnerableDisplay_Others_Offset.Get();

		DrawFrameStruct pDraw
		{
			static_cast<int>((static_cast<double>(timer->GetTimeLeft()) / timer->TimeLeft) * maxLength + 0.99),
			(pThis->ForceShielded ? pRulesExt->InvulnerableDisplay_Others_Pips.Get().X : pRulesExt->InvulnerableDisplay_Others_Pips.Get().Y),
			pRulesExt->ProgressDisplay_Others_PipsShape.Get(),
			0,
			-1,
			nullptr,
			maxLength,
			-1,
			&position,
			pBounds
		};

		TechnoExt::DrawVanillaStyleFootBar(&pDraw);
	}
}

void TechnoExt::DrawTemporalProgress(TechnoClass* pThis, RectangleStruct* pBounds, Point2D basePosition, bool isBuilding, bool isInfantry)
{
	const auto pTemporal = pThis->TemporalTargetingMe;

	if (!pTemporal)
		return;

	const auto pRulesExt = RulesExt::Global();

	if (!pRulesExt->TemporalLifeDisplay)
		return;

	if (isBuilding)
	{
		const auto pBuilding = static_cast<BuildingClass*>(pThis);
		const auto pType = pBuilding->Type;
		const auto maxLength = pType->GetFoundationHeight(false) * 15 >> 1;
		const auto offset = pRulesExt->TemporalLifeDisplay_Buildings_Offset.Get();
		auto position = basePosition + Point2D { offset.X, pType->PixelSelectionBracketDelta + offset.Y };

		DrawFrameStruct pDraw
		{
			static_cast<int>((static_cast<double>(pTemporal->WarpRemaining) / (pType->Strength * 10)) * maxLength + 0.99),
			pRulesExt->TemporalLifeDisplay_Buildings_Pips,
			pRulesExt->ProgressDisplay_Buildings_PipsShape.Get(),
			0,
			-1,
			nullptr,
			maxLength,
			-1,
			&position,
			pBounds
		};

		if (offset == Point2D::Empty && (pThis->IsSelected || pThis->IsMouseHovering)) // Layer fix
		{
			RulesClass* const pRules = RulesClass::Instance;
			const auto ratio = pBuilding->GetHealthPercentage();
			pDraw.MidLength = static_cast<int>(ratio * maxLength);
			pDraw.MidFrame = (ratio > pRules->ConditionYellow) ? 1 : (ratio > pRules->ConditionRed ? 2 : 4);
			pDraw.MidPipSHP = FileSystem::PIPS_SHP;
			pDraw.BrdFrame = 0;
		}

		TechnoExt::DrawVanillaStyleBuildingBar(&pDraw);
	}
	else
	{
		const int maxLength = isInfantry ? 8 : 17;
		const auto pType = pThis->GetTechnoType();
		auto position = basePosition + Point2D { 0, (pType->PixelSelectionBracketDelta + 2) } + pRulesExt->TemporalLifeDisplay_Others_Offset.Get();

		DrawFrameStruct pDraw
		{
			static_cast<int>((static_cast<double>(pTemporal->WarpRemaining) / (pType->Strength * 10)) * maxLength + 0.99),
			pRulesExt->TemporalLifeDisplay_Others_Pips,
			pRulesExt->ProgressDisplay_Others_PipsShape.Get(),
			0,
			-1,
			nullptr,
			maxLength,
			-1,
			&position,
			pBounds
		};

		TechnoExt::DrawVanillaStyleFootBar(&pDraw);
	}
}

void TechnoExt::DrawVanillaStyleFootBar(DrawFrameStruct* pDraw)
{
	const auto pLocation = pDraw->Location;
	const auto pBounds = pDraw->Bounds;

	if (pDraw->BrdFrame >= 0)
	{
		pLocation->X += 17;
		DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPBRD_SHP, pDraw->BrdFrame, pLocation, pBounds, BlitterFlags(0xE00), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
		pLocation->X -= 15;
	}
	else
	{
		pLocation->X += 2;
	}

	pLocation->Y += 1;

	const auto topLength = pDraw->TopLength;
	const auto midLength = pDraw->MidLength;
	const auto maxLength = pDraw->MaxLength;

	auto length = topLength > maxLength ? maxLength : topLength;

	if (pDraw->TopFrame >= 0 && pDraw->TopPipSHP)
	{
		for (auto drawIdx = length; drawIdx > 0 ; --drawIdx, pLocation->X += 2)
			DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, pDraw->TopPipSHP, pDraw->TopFrame, pLocation, pBounds, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}

	length = midLength > maxLength ? maxLength - length : midLength - length;

	if (pDraw->MidFrame >= 0 && pDraw->MidPipSHP)
	{
		for (auto drawIdx = length; drawIdx > 0 ; --drawIdx, pLocation->X += 2)
			DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, pDraw->MidPipSHP, pDraw->MidFrame, pLocation, pBounds, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}
}

void TechnoExt::DrawVanillaStyleBuildingBar(DrawFrameStruct* pDraw)
{
	const auto pLocation = pDraw->Location;
	++pLocation->X;
	const auto pBounds = pDraw->Bounds;

	const auto topLength = pDraw->TopLength;
	const auto midLength = pDraw->MidLength;
	const auto maxLength = pDraw->MaxLength;

	auto length = topLength > maxLength ? maxLength : topLength;

	if (pDraw->TopFrame >= 0 && pDraw->TopPipSHP)
	{
		for (auto drawIdx = length; drawIdx > 0 ; --drawIdx, pLocation->X -= 4, pLocation->Y += 2)
			DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, pDraw->TopPipSHP, pDraw->TopFrame, pLocation, pBounds, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}

	length = midLength > maxLength ? maxLength - length : midLength - length;

	if (pDraw->MidFrame >= 0 && pDraw->MidPipSHP)
	{
		for (auto drawIdx = length; drawIdx > 0 ; --drawIdx, pLocation->X -= 4, pLocation->Y += 2)
			DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, pDraw->MidPipSHP, pDraw->MidFrame, pLocation, pBounds, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}

	length = length >= 0 ? maxLength - midLength : maxLength - topLength;

	if (pDraw->BrdFrame >= 0)
	{
		for (auto drawIdx = length; drawIdx > 0 ; --drawIdx, pLocation->X -= 4, pLocation->Y += 2)
			DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS_SHP, pDraw->BrdFrame, pLocation, pBounds, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}
}

Point2D TechnoExt::GetScreenLocation(TechnoClass* pThis)
{
	return TacticalClass::Instance->CoordsToClient(pThis->GetCoords()).first;
}

Point2D TechnoExt::GetFootSelectBracketPosition(TechnoClass* pThis, Anchor anchor)
{
	int length = 17;
	Point2D position = GetScreenLocation(pThis);

	if (pThis->WhatAmI() == AbstractType::Infantry)
		length = 8;

	RectangleStruct bracketRect =
	{
		position.X - length + (length == 8) + 1,
		position.Y - 28 + (length == 8),
		length * 2,
		length * 3
	};

	return anchor.OffsetPosition(bracketRect);
}

Point2D TechnoExt::GetBuildingSelectBracketPosition(TechnoClass* pThis, BuildingSelectBracketPosition bracketPosition)
{
	const auto pBuildingType = static_cast<BuildingTypeClass*>(pThis->GetTechnoType());
	Point2D position = GetScreenLocation(pThis);
	CoordStruct dim2 = CoordStruct::Empty;
	pBuildingType->Dimension2(&dim2);
	dim2 = { -dim2.X / 2, dim2.Y / 2, dim2.Z };
	Point2D positionFix = TacticalClass::CoordsToScreen(dim2);

	const int foundationWidth = pBuildingType->GetFoundationWidth();
	const int foundationHeight = pBuildingType->GetFoundationHeight(false);
	const int height = pBuildingType->Height * 12;
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

void TechnoExt::ProcessDigitalDisplays(TechnoClass* pThis)
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
			GetBuildingSelectBracketPosition(pThis, pDisplayType->AnchorType_Building)
			: GetFootSelectBracketPosition(pThis, pDisplayType->AnchorType);
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
