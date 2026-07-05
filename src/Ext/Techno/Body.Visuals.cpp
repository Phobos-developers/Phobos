#include "Body.h"

#include <Ext/Anim/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/House/Body.h>
#include <ObjectClass.h>
#include <algorithm>

void TechnoExt::DrawSelfHealPips(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds)
{
	if (!RulesExt::Global()->GainSelfHealAllowMultiplayPassive && pThis->Owner->Type->MultiplayPassive)
		return;

	auto const pTypeExt = TechnoExt::ExtMap.Find(pThis)->TypeExtData;

	if (pTypeExt->SelfHealGainType.isset() && pTypeExt->SelfHealGainType.Get() == SelfHealGainType::NoHeal)
		return;

	bool drawPip = false;
	bool isInfantryHeal = false;
	int selfHealFrames = 0;
	const bool hasInfantrySelfHeal = pTypeExt->SelfHealGainType.isset() && pTypeExt->SelfHealGainType.Get() == SelfHealGainType::Infantry;
	const bool hasUnitSelfHeal = pTypeExt->SelfHealGainType.isset() && pTypeExt->SelfHealGainType.Get() == SelfHealGainType::Units;
	auto const whatAmI = pThis->WhatAmI();
	auto const pType = pTypeExt->OwnerObject();
	const bool isOrganic = (whatAmI == AbstractType::Infantry || (pType->Organic && whatAmI == AbstractType::Unit));

	auto hasSelfHeal = [pThis](const bool infantryHeal)
		{
			auto const pOwner = pThis->Owner;

			auto haveHeal = [infantryHeal](HouseClass* pHouse)
				{
					return (infantryHeal ? pHouse->InfantrySelfHeal > 0 : pHouse->UnitsSelfHeal > 0);
				};

			if (haveHeal(pOwner))
				return true;

			const bool isCampaign = SessionClass::IsCampaign();
			const bool fromPlayer = RulesExt::Global()->GainSelfHealFromPlayerControl && isCampaign && (pOwner->IsHumanPlayer || pOwner->IsInPlayerControl);
			const bool fromAllies = RulesExt::Global()->GainSelfHealFromAllies;

			if (fromPlayer || fromAllies)
			{
				auto checkHouse = [fromPlayer, fromAllies, isCampaign, pOwner](HouseClass* pHouse)
					{
						if (pHouse == pOwner)
							return false;

						return (fromPlayer && (pHouse->IsHumanPlayer || pHouse->IsInPlayerControl)) // pHouse->IsControlledByCurrentPlayer()
							|| (fromAllies && (!isCampaign || (!pHouse->IsHumanPlayer && !pHouse->IsInPlayerControl)) && pHouse->IsAlliedWith(pOwner));
					};

				for (auto const pHouse : HouseClass::Array)
				{
					if (checkHouse(pHouse) && haveHeal(pHouse))
						return true;
				}
			}

			return false;
		};

	if ((hasInfantrySelfHeal || (isOrganic && !hasUnitSelfHeal)) && hasSelfHeal(true))
	{
		drawPip = true;
		selfHealFrames = RulesClass::Instance->SelfHealInfantryFrames;
		isInfantryHeal = true;
	}
	else if ((hasUnitSelfHeal || (whatAmI == AbstractType::Unit && !isOrganic)) && hasSelfHeal(false))
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
			&& pThis->Health < pType->Strength)
		{
			isSelfHealFrame = true;
		}

		if (whatAmI == AbstractType::Unit || whatAmI == AbstractType::Aircraft)
		{
			auto& offset = RulesExt::Global()->Pips_SelfHeal_Units_Offset.Get();
			pipFrames = RulesExt::Global()->Pips_SelfHeal_Units;
			xOffset = offset.X;
			yOffset = offset.Y + pType->PixelSelectionBracketDelta;
		}
		else if (whatAmI == AbstractType::Infantry)
		{
			auto& offset = RulesExt::Global()->Pips_SelfHeal_Infantry_Offset.Get();
			pipFrames = RulesExt::Global()->Pips_SelfHeal_Infantry;
			xOffset = offset.X;
			yOffset = offset.Y + pType->PixelSelectionBracketDelta;
		}
		else
		{
			const auto pBldType = static_cast<BuildingClass*>(pThis)->Type;
			const int fHeight = pBldType->GetFoundationHeight(false);
			const int yAdjust = -Unsorted::CellHeightInPixels / 2;

			auto& offset = RulesExt::Global()->Pips_SelfHeal_Buildings_Offset.Get();
			pipFrames = RulesExt::Global()->Pips_SelfHeal_Buildings;
			xOffset = offset.X + Unsorted::CellWidthInPixels / 2 * fHeight;
			yOffset = offset.Y + yAdjust * fHeight + pBldType->Height * yAdjust;
		}

		const int pipFrame = isInfantryHeal ? pipFrames.Get().X : pipFrames.Get().Y;

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
	auto pTechnoTypeExt = TechnoExt::ExtMap.Find(pThis)->TypeExtData;
	auto pTechnoType = pTechnoTypeExt->OwnerObject();
	auto pOwner = pThis->Owner;
	const bool isObserver = HouseClass::IsCurrentPlayerObserver();

	if (pThis->IsDisguised() && !pThis->IsClearlyVisibleTo(HouseClass::CurrentPlayer) && !(isObserver
		|| EnumFunctions::CanTargetHouse(RulesExt::Global()->DisguiseBlinkingVisibility, HouseClass::CurrentPlayer, pOwner)))
	{
		if (auto const pType = TechnoTypeExt::GetTechnoType(pThis->Disguise))
		{
			pTechnoType = pType;
			pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pType);
			pOwner = pThis->DisguisedAsHouse;
		}
		else if (pThis->Disguise->WhatAmI() == AbstractType::TerrainType && (!isObserver && !pOwner->IsAlliedWith(HouseClass::CurrentPlayer)))
		{
			return;
		}
	}

	const bool isVisibleToPlayer = (pOwner && pOwner->IsAlliedWith(HouseClass::CurrentPlayer))
		|| isObserver || pTechnoTypeExt->Insignia_ShowEnemy.Get(RulesExt::Global()->EnemyInsignia);

	if (!isVisibleToPlayer)
		return;

	Point2D offset = *pLocation;
	SHPStruct* pShapeFile = FileSystem::PIPS_SHP;
	int defaultFrameIndex = -1;
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

	if (pTechnoType->Passengers > 0 && pTechnoTypeExt->Insignia_Passengers.size() > 0)
	{
		int passengersIndex = pTechnoTypeExt->Passengers_BySize ? pThis->Passengers.GetTotalSize() : pThis->Passengers.NumPassengers;
		passengersIndex = Math::min(passengersIndex, pTechnoType->Passengers);

		if (auto const pCustomShapeFile = pTechnoTypeExt->Insignia_Passengers[passengersIndex].Get(pThis))
		{
			pShapeFile = pCustomShapeFile;
			defaultFrameIndex = 0;
			isCustomInsignia = true;
		}

		const int frame = pTechnoTypeExt->InsigniaFrame_Passengers[passengersIndex].Get(pThis);

		if (frame != -1)
			frameIndex = frame;

		auto const& frames = pTechnoTypeExt->InsigniaFrames_Passengers[passengersIndex];

		if (frames != Vector3D<int>(-1, -1, -1))
			insigniaFrames = frames.Get();
	}

	if (pTechnoType->Gunner && pTechnoTypeExt->Insignia_Weapon.size() > 0)
	{
		const int weaponIndex = pThis->CurrentWeaponNumber;

		if (auto const pCustomShapeFile = pTechnoTypeExt->Insignia_Weapon[weaponIndex].Get(pThis))
		{
			pShapeFile = pCustomShapeFile;
			defaultFrameIndex = 0;
			isCustomInsignia = true;
		}

		const int frame = pTechnoTypeExt->InsigniaFrame_Weapon[weaponIndex].Get(pThis);

		if (frame != -1)
			frameIndex = frame;

		auto const& frames = pTechnoTypeExt->InsigniaFrames_Weapon[weaponIndex];

		if (frames != Vector3D<int>(-1, -1, -1))
			insigniaFrames = frames.Get();
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

		offset.Y += RulesExt::Global()->DrawInsignia_UsePixelSelectionBracketDelta ? pTechnoType->PixelSelectionBracketDelta : 0;

		DSurface::Temp->DrawSHP(
			FileSystem::PALETTE_PAL, pShapeFile, frameIndex, &offset, pBounds, BlitterFlags(0xE00), 0, -2, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}

	return;
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
	const Point2D positionFix = TacticalClass::CoordsToScreen(dim2);

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

void TechnoExt::DrawSelectBox(TechnoClass* pThis, const Point2D* pLocation, const RectangleStruct* pBounds, bool drawBefore)
{
	const auto whatAmI = pThis->WhatAmI();
	const auto pTypeExt = TechnoExt::ExtMap.Find(pThis)->TypeExtData;
	const auto pType = pTypeExt->OwnerObject();
	SelectBoxTypeClass* pSelectBox = nullptr;

	if (pTypeExt->SelectBox.isset())
		pSelectBox = pTypeExt->SelectBox.Get();
	else if (whatAmI == InfantryClass::AbsID)
		pSelectBox = RulesExt::Global()->DefaultInfantrySelectBox.Get();
	else if (whatAmI != BuildingClass::AbsID)
		pSelectBox = RulesExt::Global()->DefaultUnitSelectBox.Get();

	if (!pSelectBox || pSelectBox->DrawAboveTechno == drawBefore)
		return;

	const bool canSee = HouseClass::IsCurrentPlayerObserver() ? pSelectBox->VisibleToHouses_Observer : EnumFunctions::CanTargetHouse(pSelectBox->VisibleToHouses, pThis->Owner, HouseClass::CurrentPlayer);

	if (!canSee)
		return;

	const double healthPercentage = pThis->GetHealthPercentage();
	const auto defaultFrame = whatAmI == InfantryClass::AbsID ? Vector3D<int> { 1, 1, 1 } : Vector3D<int> { 0, 0, 0 };

	const auto pSurface = DSurface::Temp;
	const auto flags = (drawBefore ? BlitterFlags::Flat | BlitterFlags::Alpha : BlitterFlags::Nonzero | BlitterFlags::MultiPass) | BlitterFlags::Centered | pSelectBox->Translucency;
	const int zAdjust = drawBefore ? pThis->GetZAdjustment() - 2 : 0;
	const auto pGroundShape = pSelectBox->GroundShape.Get();

	if ((pGroundShape || pSelectBox->GroundLine) && whatAmI != BuildingClass::AbsID && (pSelectBox->Ground_AlwaysDraw || pThis->IsInAir()))
	{
		auto [point, visible] = TacticalClass::Instance->CoordsToClient(pThis->GetRenderCoords());
		const auto pFoot = static_cast<FootClass*>(pThis);
		if(pThis->WhatAmI()==AbstractType::Aircraft)
			point.Y += TacticalClass::AdjustForZ(pFoot->GetHeight());
		else
			point += pFoot->Locomotor->Shadow_Point();
		if (visible && pGroundShape)
		{
			const auto pPalette = pSelectBox->GroundPalette.GetOrDefaultConvert(FileSystem::PALETTE_PAL);

			const Vector3D<int> frames = pSelectBox->GroundFrames.Get(defaultFrame);
			const int frame = healthPercentage > RulesClass::Instance->ConditionYellow ? frames.X : healthPercentage > RulesClass::Instance->ConditionRed ? frames.Y : frames.Z;

			const Point2D drawPoint = point + pSelectBox->GroundOffset;
			pSurface->DrawSHP(pPalette, pGroundShape, frame, &drawPoint, pBounds, flags, 0, zAdjust, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
		}

		if (pSelectBox->GroundLine)
		{
			Point2D start = *pLocation; // Copy to prevent be modified
			const int color = Drawing::RGB_To_Int(pSelectBox->GroundLineColor.Get(healthPercentage));

			if (pSelectBox->GroundLine_Dashed)
				pSurface->DrawDashed(&start, &point, color, 0);
			else if (Line_In_Bounds(&start, &point, &DSurface::ViewBounds))
				pSurface->DrawLine(&start, &point, color);
		}
	}

	if (const auto pShape = pSelectBox->Shape.Get())
	{
		const auto pPalette = pSelectBox->Palette.GetOrDefaultConvert(FileSystem::PALETTE_PAL);

		const Vector3D<int> frames = pSelectBox->Frames.Get(defaultFrame);
		const int frame = healthPercentage > RulesClass::Instance->ConditionYellow ? frames.X : healthPercentage > RulesClass::Instance->ConditionRed ? frames.Y : frames.Z;

		const Point2D offset = whatAmI == InfantryClass::AbsID ? Point2D { 8, -3 } : Point2D { 1, -4 };
		Point2D drawPoint = *pLocation + offset + pSelectBox->Offset;

		if (pSelectBox->DrawAboveTechno)
			drawPoint.Y += pType->PixelSelectionBracketDelta;

		pSurface->DrawSHP(pPalette, pShape, frame, &drawPoint, pBounds, flags, 0, zAdjust, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
	}
}

void TechnoExt::ProcessDigitalDisplays(TechnoClass* pThis)
{
	if (!Phobos::Config::DigitalDisplay_Enable)
		return;

	const auto pTypeExt = TechnoExt::ExtMap.Find(pThis)->TypeExtData;

	if (pTypeExt->DigitalDisplay_Disable)
		return;

	int length = 17;
	ValueableVector<DigitalDisplayTypeClass*>* pDisplayTypes = nullptr;
	const auto whatAmI = pThis->WhatAmI();

	if (!pTypeExt->DigitalDisplayTypes.empty())
	{
		pDisplayTypes = &pTypeExt->DigitalDisplayTypes;
	}
	else
	{
		switch (whatAmI)
		{
		case AbstractType::Building:
		{
			pDisplayTypes = &RulesExt::Global()->Buildings_DefaultDigitalDisplayTypes;
			const auto pBuildingType = static_cast<BuildingTypeClass*>(pTypeExt->OwnerObject());
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

	const auto pType = pTypeExt->OwnerObject();
	const auto pShield = TechnoExt::ExtMap.Find(pThis)->Shield.get();
	const bool hasShield = pShield && !pShield->IsBrokenAndNonRespawning();
	const bool isBuilding = whatAmI == AbstractType::Building;
	const bool isInfantry = whatAmI == AbstractType::Infantry;

	for (DigitalDisplayTypeClass*& pDisplayType : *pDisplayTypes)
	{
		if (!pDisplayType->CanShow(pThis))
			continue;

		int value = -1;
		int maxValue = 0;

		GetValuesForDisplay(pThis, pType, pDisplayType->InfoType, value, maxValue, pDisplayType->InfoIndex);

		if (value <= -1 || maxValue <= 0)
			continue;

		const auto divisor = pDisplayType->ValueScaleDivisor.Get(pDisplayType->ValueAsTimer ? 15 : 1);

		if (divisor > 1)
		{
			value = Math::max(value / divisor, value ? 1 : 0);
			maxValue = Math::max(maxValue / divisor, 1);
		}

		Point2D position = whatAmI == AbstractType::Building
			? GetBuildingSelectBracketPosition(pThis, pDisplayType->AnchorType_Building)
			: GetFootSelectBracketPosition(pThis, pDisplayType->AnchorType);
		position.Y += pType->PixelSelectionBracketDelta;

		if (pDisplayType->InfoType == DisplayInfoType::Shield)
			position.Y += pShield->GetType()->BracketDelta;

		pDisplayType->Draw(position, length, value, maxValue, isBuilding, isInfantry, hasShield);
	}
}

void TechnoExt::GetValuesForDisplay(TechnoClass* pThis, TechnoTypeClass* pType, DisplayInfoType infoType, int& value, int& maxValue, int infoIndex)
{
	switch (infoType)
	{
	case DisplayInfoType::Health:
	{
		value = pThis->Health;
		maxValue = pType->Strength;

		if (pThis->Disguised && !pThis->Owner->IsAlliedWith(HouseClass::CurrentPlayer))
			GetDigitalDisplayFakeHealth(pThis, value, maxValue);

		break;
	}
	case DisplayInfoType::Shield:
	{
		const auto pShield = TechnoExt::ExtMap.Find(pThis)->Shield.get();

		if (!pShield || pShield->IsBrokenAndNonRespawning())
			return;

		value = pShield->GetHP();
		maxValue = pShield->GetType()->Strength.Get();
		break;
	}
	case DisplayInfoType::Ammo:
	{
		value = pThis->Ammo;
		maxValue = pType->Ammo;
		break;
	}
	case DisplayInfoType::MindControl:
	{
		const auto pCaptureManager = pThis->CaptureManager;

		if (!pCaptureManager)
			return;

		value = pCaptureManager->GetControlledCount();
		maxValue = pCaptureManager->MaxControlNodes;
		break;
	}
	case DisplayInfoType::Spawns:
	{
		const auto pSpawnManager = pThis->SpawnManager;

		if (!pSpawnManager || !pType->Spawns)
			return;

		if (infoIndex == 1)
			value = pSpawnManager->CountDockedSpawns();
		else if (infoIndex == 2)
			value = pSpawnManager->CountLaunchingSpawns();
		else
			value = pSpawnManager->CountAliveSpawns();

		maxValue = pType->SpawnsNumber;
		break;
	}
	case DisplayInfoType::Passengers:
	{
		value = pThis->Passengers.NumPassengers;
		maxValue = pType->Passengers;
		break;
	}
	case DisplayInfoType::Tiberium:
	{
		if (infoIndex && infoIndex <= TiberiumClass::Array.Count)
			value = static_cast<int>(pThis->Tiberium.GetAmount(infoIndex - 1));
		else
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

		const auto pBuildingType = static_cast<BuildingTypeClass*>(pType);

		if (!pBuildingType->CanBeOccupied)
			return;

		value = static_cast<BuildingClass*>(pThis)->Occupants.Count;
		maxValue = pBuildingType->MaxNumberOccupants;
		break;
	}
	case DisplayInfoType::GattlingStage:
	{
		if (!pType->IsGattling)
			return;

		value = pThis->GattlingValue ? pThis->CurrentGattlingStage + 1 : 0;
		maxValue = pType->WeaponStages;
		break;
	}
	case DisplayInfoType::ROF:
	{
		if (!pThis->IsArmed())
			return;

		const auto& timer = pThis->RearmTimer;
		value = timer.GetTimeLeft();
		maxValue = timer.TimeLeft;
		break;
	}
	case DisplayInfoType::Reload:
	{
		if (pType->Ammo <= 0)
			return;

		const auto& timer = pThis->ReloadTimer;
		value = (pThis->Ammo >= pType->Ammo) ? 0 : timer.GetTimeLeft();
		maxValue = timer.TimeLeft ? timer.TimeLeft : ((pThis->Ammo || pType->EmptyReload <= 0) ? pType->Reload : pType->EmptyReload);
		break;
	}
	case DisplayInfoType::SpawnTimer:
	{
		const auto pSpawnManager = pThis->SpawnManager;

		if (!pSpawnManager || !pType->Spawns || pType->SpawnsNumber <= 0)
			return;

		if (infoIndex && infoIndex <= pSpawnManager->SpawnedNodes.Count)
		{
			value = pSpawnManager->SpawnedNodes[infoIndex - 1]->SpawnTimer.GetTimeLeft();
		}
		else
		{
			for (int i = 0; i < pSpawnManager->SpawnedNodes.Count; ++i)
			{
				const auto pSpawnNode = pSpawnManager->SpawnedNodes[i];

				if (pSpawnNode->Status == SpawnNodeStatus::Dead)
				{
					const int time = pSpawnNode->SpawnTimer.GetTimeLeft();

					if (!value || time < value)
						value = time;
				}
			}
		}

		maxValue = pSpawnManager->RegenRate;
		break;
	}
	case DisplayInfoType::GattlingTimer:
	{
		if (!pType->IsGattling)
			return;

		const auto thisStage = pThis->CurrentGattlingStage;
		const auto& stage = pThis->Veterancy.IsElite() ? pType->EliteStage : pType->WeaponStage;

		value = pThis->GattlingValue;
		maxValue = stage[thisStage];

		if (thisStage > 0)
		{
			value -= stage[thisStage - 1];
			maxValue -= stage[thisStage - 1];
		}

		break;
	}
	case DisplayInfoType::ProduceCash:
	{
		if (pThis->WhatAmI() != AbstractType::Building || static_cast<BuildingTypeClass*>(pType)->ProduceCashAmount <= 0)
			return;

		const auto& timer = static_cast<BuildingClass*>(pThis)->CashProductionTimer;
		value = timer.GetTimeLeft();
		maxValue = timer.TimeLeft;
		break;
	}
	case DisplayInfoType::PassengerKill:
	{
		const auto pExt = TechnoExt::ExtMap.Find(pThis);

		if (!pExt->TypeExtData->PassengerDeletionType)
			return;

		const auto& timer = pExt->PassengerDeletionTimer;
		value = timer.GetTimeLeft();
		maxValue = timer.TimeLeft;
		break;
	}
	case DisplayInfoType::AutoDeath:
	{
		const auto pExt = TechnoExt::ExtMap.Find(pThis);
		const auto pTypeExt = pExt->TypeExtData;

		if (!pTypeExt->AutoDeath_Behavior.isset())
			return;

		if (pTypeExt->AutoDeath_AfterDelay > 0)
		{
			const auto& timer = pExt->AutoDeathTimer;
			value = timer.GetTimeLeft();
			maxValue = timer.TimeLeft;
		}
		else if (pTypeExt->AutoDeath_OnAmmoDepletion)
		{
			value = pThis->Ammo;
			maxValue = pType->Ammo;
		}

		break;
	}
	case DisplayInfoType::SuperWeapon:
	{
		if (pThis->WhatAmI() != AbstractType::Building)
			return;

		auto getSuperTimer = [pThis, pType, infoIndex]() -> CDTimerClass*
		{
			const auto pHouse = pThis->Owner;
			const auto pBuildingType = static_cast<BuildingTypeClass*>(pType);
			const auto pBuildingTypeExt = BuildingTypeExt::ExtMap.Find(pBuildingType);

			if (infoIndex && infoIndex <= pBuildingTypeExt->GetSuperWeaponCount())
			{
				if (infoIndex == 1)
				{
					if (pBuildingType->SuperWeapon != -1)
						return &pHouse->Supers.GetItem(pBuildingType->SuperWeapon)->RechargeTimer;
				}
				else if (infoIndex == 2)
				{
					if (pBuildingType->SuperWeapon2 != -1)
						return &pHouse->Supers.GetItem(pBuildingType->SuperWeapon2)->RechargeTimer;
				}
				else
				{
					const auto& superWeapons = pBuildingTypeExt->SuperWeapons;
					return &pHouse->Supers.GetItem(superWeapons[infoIndex - 3])->RechargeTimer;
				}

				return nullptr;
			}

			if (pBuildingType->SuperWeapon != -1)
				return &pHouse->Supers.GetItem(pBuildingType->SuperWeapon)->RechargeTimer;
			else if (pBuildingType->SuperWeapon2 != -1)
				return &pHouse->Supers.GetItem(pBuildingType->SuperWeapon2)->RechargeTimer;

			const auto& superWeapons = pBuildingTypeExt->SuperWeapons;
			return superWeapons.size() > 0 ? &pHouse->Supers.GetItem(superWeapons[0])->RechargeTimer : nullptr;
		};
		if (const auto pTimer = getSuperTimer())
		{
			value = pTimer->GetTimeLeft();
			maxValue = pTimer->TimeLeft;
		}

		break;
	}
	case DisplayInfoType::IronCurtain:
	{
		if (!pThis->IsIronCurtained())
			return;

		const auto& timer = pThis->IronCurtainTimer;
		value = timer.GetTimeLeft();
		maxValue = timer.TimeLeft;
		break;
	}
	case DisplayInfoType::TemporalLife:
	{
		const auto pTemporal = pThis->TemporalTargetingMe;

		if (!pTemporal)
			return;

		value = pTemporal->WarpRemaining;
		maxValue = pType->Strength * 10;
		break;
	}
	case DisplayInfoType::FactoryProcess:
	{
		if (pThis->WhatAmI() != AbstractType::Building)
			return;

		auto getFactory = [pThis, pType, infoIndex]() -> FactoryClass*
		{
			const auto pHouse = pThis->Owner;
			const auto pBuildingType = static_cast<BuildingTypeClass*>(pType);

			if (infoIndex == 1)
			{
				if (!pHouse->IsControlledByHuman())
					return static_cast<BuildingClass*>(pThis)->Factory;
				else if (pThis->IsPrimaryFactory)
					return pHouse->GetPrimaryFactory(pBuildingType->Factory, pBuildingType->Naval, BuildCat::DontCare);
			}
			else if (infoIndex == 2)
			{
				if (pHouse->IsControlledByHuman() && pThis->IsPrimaryFactory && pBuildingType->Factory == AbstractType::BuildingType)
					return pHouse->Primary_ForDefenses;
			}
			else if (!pHouse->IsControlledByHuman())
			{
				return static_cast<BuildingClass*>(pThis)->Factory;
			}
			else if (pThis->IsPrimaryFactory)
			{
				const auto pFactory = pHouse->GetPrimaryFactory(pBuildingType->Factory, pBuildingType->Naval, BuildCat::DontCare);

				if (pFactory && pFactory->Object)
					return pFactory;
				else if (pBuildingType->Factory == AbstractType::BuildingType)
					return pHouse->Primary_ForDefenses;
			}

			return nullptr;
		};
		if (const auto pFactory = getFactory())
		{
			if (pFactory->Object)
			{
				value = pFactory->GetProgress();
				maxValue = 54;
			}
		}

		break;
	}
	default:
	{
		value = pThis->Health;
		maxValue = pType->Strength;

		if (pThis->Disguised && !pThis->Owner->IsAlliedWith(HouseClass::CurrentPlayer))
			GetDigitalDisplayFakeHealth(pThis, value, maxValue);

		break;
	}
	}
}

void TechnoExt::GetDigitalDisplayFakeHealth(TechnoClass* pThis, int& value, int& maxValue)
{
	if (TechnoExt::ExtMap.Find(pThis)->TypeExtData->DigitalDisplay_Health_FakeAtDisguise)
	{
		if (const auto pType = TechnoTypeExt::GetTechnoType(pThis->Disguise))
		{
			const int newMaxValue = pType->Strength;
			const double ratio = static_cast<double>(value) / maxValue;
			value = static_cast<int>(ratio * newMaxValue);
			maxValue = newMaxValue;
		}
	}
}

void TechnoExt::ShowPromoteAnim(TechnoClass* pThis)
{
	auto const pTypeExt = TechnoExt::ExtMap.Find(pThis)->TypeExtData;
	auto const& veteranAnims = !pTypeExt->Promote_VeteranAnimation.empty() ? pTypeExt->Promote_VeteranAnimation : RulesExt::Global()->Promote_VeteranAnimation;
	auto const& eliteAnims = !pTypeExt->Promote_EliteAnimation.empty() ? pTypeExt->Promote_EliteAnimation : RulesExt::Global()->Promote_EliteAnimation;

	if (pThis->Veterancy.GetRemainingLevel() == Rank::Veteran && !veteranAnims.empty())
		AnimExt::CreateRandomAnim(veteranAnims, pThis->GetCenterCoords(), pThis, pThis->Owner, true, true);
	else if (!eliteAnims.empty())
		AnimExt::CreateRandomAnim(eliteAnims, pThis->GetCenterCoords(), pThis, pThis->Owner, true, true);
}

void TechnoExt::DrawCameos(TechnoClass* pThis)
{
	if (ObjectClass::CurrentObjects.Count != 1)
		return;

	if (ObjectClass::CurrentObjects.GetItem(0) != pThis)
		return;

	const auto whatAmI = pThis->WhatAmI();

	const auto pType = pThis->GetTechnoType();
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	const auto pRulesExt = RulesExt::Global();

	// Per-unit explicitly disabled - skip entirely
	if (pTypeExt->ShowCameo.isset() && !pTypeExt->ShowCameo.Get())
		return;

	// Determine if permanent mode is on (permanent mode bypasses master switch and toggleable)
	const bool permanentMode = pTypeExt->ShowCameo.isset()
		? pTypeExt->ShowCameo.Get()
		: pRulesExt->ShowCameo;

	// Master switch check (skipped for permanent mode)
	if (!permanentMode && !Phobos::Config::ShowCameo_Enable)
		return;

	// Toggleable check (skipped for permanent mode)
	if (!permanentMode && !pRulesExt->ShowCameo_Toggleable)
		return;

	// Collect cameos: passengers + occupants
	// Passengers are stored in LIFO linked list; for buildings we reverse to FIFO
	// so that Y flip produces correct top-to-bottom layout.
	// Occupants are stored in FIFO DynamicVectorClass and do NOT need reversing.
	std::vector<std::pair<TechnoTypeClass*, int>> cameoCounts;

	// Step 1: Collect passengers into a temp vector (LIFO order)
	std::vector<std::pair<TechnoTypeClass*, int>> passengerCounts;
	for (auto pUnit = pThis->Passengers.GetFirstPassenger(); pUnit; pUnit = abstract_cast<FootClass*>(pUnit->NextObject))
	{
		const auto pUnitType = pUnit->GetTechnoType();
		auto it = std::find_if(passengerCounts.begin(), passengerCounts.end(),
			[pUnitType](const auto& pair) { return pair.first == pUnitType; });
		if (it != passengerCounts.end())
			it->second++;
		else
			passengerCounts.emplace_back(pUnitType, 1);
	}

	// For buildings, reverse passenger order: LIFO -> FIFO
	if (whatAmI == AbstractType::Building)
		std::reverse(passengerCounts.begin(), passengerCounts.end());

	// Merge passengers into cameoCounts
	for (const auto& item : passengerCounts)
		cameoCounts.emplace_back(item.first, item.second);

	// Step 2: Collect occupants (FIFO order, no reverse needed)
	if (whatAmI == AbstractType::Building)
	{
		const auto pBuilding = static_cast<BuildingClass*>(pThis);
		for (int i = 0; i < pBuilding->Occupants.Count; i++)
		{
			const auto pOccupant = pBuilding->Occupants.GetItem(i);
			const auto pOccupantType = pOccupant->GetTechnoType();
			auto it = std::find_if(cameoCounts.begin(), cameoCounts.end(),
				[pOccupantType](const auto& pair) { return pair.first == pOccupantType; });
			if (it != cameoCounts.end())
				it->second++;
			else
				cameoCounts.emplace_back(pOccupantType, 1);
		}
	}

	if (cameoCounts.empty())
		return;

	const auto bracketPos = whatAmI == AbstractType::Building
		? GetBuildingSelectBracketPosition(pThis, BuildingSelectBracketPosition::Top)
		: GetFootSelectBracketPosition(pThis, Anchor(HorizontalPosition::Center, VerticalPosition::Top));
	Point2D offset = pTypeExt->ShowCameo_BottomOffset.isset()
		? pTypeExt->ShowCameo_BottomOffset.Get()
		: pRulesExt->ShowCameo_BottomOffset.Get();
	Point2D basePos = bracketPos;
	basePos.X += offset.X;
	basePos.Y += offset.Y + pType->PixelSelectionBracketDelta;

	const int perRow = pTypeExt->ShowCameo_PerRow.isset()
		? pTypeExt->ShowCameo_PerRow.Get()
		: pRulesExt->ShowCameo_PerRow.Get();
	const int count = static_cast<int>(cameoCounts.size());
	const int rows = (count + perRow - 1) / perRow;

	// Get overlap/offset/overlapPrimary settings
	const Point2D overlapXY = pTypeExt->ShowCameo_OverlapXY.isset()
		? pTypeExt->ShowCameo_OverlapXY.Get()
		: pRulesExt->ShowCameo_OverlapXY.Get();
	const Point2D cameoOffsetXY = pTypeExt->ShowCameo_OffsetXY.isset()
		? pTypeExt->ShowCameo_OffsetXY.Get()
		: pRulesExt->ShowCameo_OffsetXY.Get();
	const bool overlapPrimary = pTypeExt->ShowCameo_OverlapPrimary.isset()
		? pTypeExt->ShowCameo_OverlapPrimary.Get()
		: pRulesExt->ShowCameo_OverlapPrimary.Get();
	// overlapPrimary=true: primary (earlier) covers secondary (later) (前面覆盖后面)
	// overlapPrimary=false: secondary (later) covers primary (earlier) (后面覆盖前面)

	// Pre-compute canvas sizes for each cameo (standard 60x48, or custom SHP dimensions)
	struct CameoCanvas { int width; int height; };
	std::vector<CameoCanvas> canvases;
	canvases.reserve(count);
	for (const auto& item : cameoCounts)
	{
		const auto pCameoType = item.first;
		const auto pCameoTypeExt = TechnoTypeExt::ExtMap.Find(pCameoType);
		int w = 60, h = 48;
		if (SHPStruct* pCustomCameo = pCameoTypeExt->ShowCameo_CustomShape)
		{
			w = pCustomCameo->Width;
			h = pCustomCameo->Height;
		}
		canvases.push_back({ w, h });
	}

	// Step 1-2-3: Calculate positions sequentially (row layout + OverlapXY + OffsetXY)
	// Overlap formula: overlap = coveredCanvasSize * (OverlapXY% / 100) + OffsetXY
	// OverlapPrimary=true: overlap based on secondary cameo's size (secondary is covered)
	// OverlapPrimary=false: overlap based on primary cameo's size (primary is covered)
	// Base gap between adjacent icons: 2 pixels (hGap / vGap)
	const int hGap = 2;
	const int vGap = 2;
	// Iterate from bottom row to top row, so that each row can reference the row below it
	std::vector<Point2D> cameoPositions(count);
	for (int row = 0; row < rows; row++)
	{
		const int firstIdx = row * perRow;
		const int lastIdx = Math::min(firstIdx + perRow - 1, count - 1);
		for (int idx = firstIdx; idx <= lastIdx; idx++)
		{
			const int col = idx - firstIdx;
			const int curW = canvases[idx].width;
			const int curH = canvases[idx].height;

			if (row == 0)
			{
				// Bottom row: anchor at base position (bottom edge fixed at basePos.Y)
				if (col == 0)
				{
					cameoPositions[idx].X = basePos.X;
					cameoPositions[idx].Y = basePos.Y - curH;
				}
				else
				{
					// Same row: position to the right of the cameo directly to the left
					const int prevIdx = idx - 1;
					const int prevX = cameoPositions[prevIdx].X;
					const int prevW = canvases[prevIdx].width;

					const int overlapX = overlapPrimary
						? (curW * overlapXY.X / 100 + cameoOffsetXY.X)
						: (prevW * overlapXY.X / 100 + cameoOffsetXY.X);

					cameoPositions[idx].X = prevX + prevW + hGap - overlapX;
					cameoPositions[idx].Y = cameoPositions[prevIdx].Y;
				}
			}
			else
			{
				if (col == 0)
				{
					// First in row: position above the cameo directly below
					const int belowIdx = (row - 1) * perRow;
					const int belowY = cameoPositions[belowIdx].Y;
					const int belowH = canvases[belowIdx].height;

					const int overlapY = overlapPrimary
						? (curH * overlapXY.Y / 100 + cameoOffsetXY.Y)
						: (belowH * overlapXY.Y / 100 + cameoOffsetXY.Y);

					cameoPositions[idx].X = basePos.X;
					cameoPositions[idx].Y = belowY - curH - vGap + overlapY;
				}
				else
				{
					// Same row: position to the right of the cameo directly to the left
					const int prevIdx = idx - 1;
					const int prevX = cameoPositions[prevIdx].X;
					const int prevW = canvases[prevIdx].width;

					const int overlapX = overlapPrimary
						? (curW * overlapXY.X / 100 + cameoOffsetXY.X)
						: (prevW * overlapXY.X / 100 + cameoOffsetXY.X);

					cameoPositions[idx].X = prevX + prevW + hGap - overlapX;
					cameoPositions[idx].Y = cameoPositions[prevIdx].Y;
				}
			}
		}
	}

	// Step 4: Center each row horizontally
	for (int row = 0; row < rows; row++)
	{
		const int firstIdx = row * perRow;
		const int lastIdx = Math::min(firstIdx + perRow - 1, count - 1);
		const int rowWidth = cameoPositions[lastIdx].X + canvases[lastIdx].width - cameoPositions[firstIdx].X;
		const int centerOffset = basePos.X - rowWidth / 2 - cameoPositions[firstIdx].X;
		for (int i = firstIdx; i <= lastIdx; i++)
			cameoPositions[i].X += centerOffset;
	}

	// For buildings, flip Y axis: first-entered goes to top, last-entered to bottom
	if (whatAmI == AbstractType::Building)
	{
		int topY = INT_MAX;
		for (int i = 0; i < count; i++)
			topY = Math::min(topY, cameoPositions[i].Y);

		for (int i = 0; i < count; i++)
			cameoPositions[i].Y = basePos.Y - canvases[i].height - (cameoPositions[i].Y - topY);
	}

	const auto pSurface = DSurface::Composite;
	RectangleStruct bounds = DSurface::Composite->GetRect();

	// Determine BlitterFlags for translucency
	const TranslucencyLevel translucency = pTypeExt->ShowCameo_Translucency.isset()
		? pTypeExt->ShowCameo_Translucency.Get()
		: pRulesExt->ShowCameo_Translucency;
	const BlitterFlags translucencyFlag = translucency.GetBlitterFlags();

	int idx = 0;
	for (const auto& item : cameoCounts)
	{
		const auto pCameoType = item.first;
		const int cameoCount = item.second;
		const int curW = canvases[idx].width;
		const int curH = canvases[idx].height;

		Point2D iconPos = cameoPositions[idx];

		// Skip icons whose any part exceeds the screen top boundary (Y < 0) to prevent crash
		// Only the top boundary matters because cameos are always above the selection bracket
		if (iconPos.Y < 0)
		{
			idx++;
			continue;
		}

		bool drawn = false;

		const auto pCameoTypeExt = TechnoTypeExt::ExtMap.Find(pCameoType);

		// Use custom ShowCameo SHP if specified
		if (SHPStruct* pCustomCameo = pCameoTypeExt->ShowCameo_CustomShape)
		{
			const auto pCameoConvert = pCameoTypeExt->CameoPalette.GetOrDefaultConvert(FileSystem::CAMEO_PAL);
			const auto pConvert = pCameoTypeExt->ShowCameo_CustomPalette.GetOrDefaultConvert(pCameoConvert);
			const int frameCount = pCustomCameo->Frames > 0 ? pCustomCameo->Frames : 1;
			const int frame = (Unsorted::CurrentFrame % frameCount);
			pSurface->DrawSHP(
				pConvert,
				pCustomCameo,
				frame,
				&iconPos,
				&bounds,
				BlitterFlags::bf_400 | BlitterFlags::Alpha | translucencyFlag,
				0, 0,
				ZGradient::Ground,
				1000, 0, nullptr, 0, 0, 0
			);
			drawn = true;
		}

		if (!drawn)
		{
			if (const auto pCameoPCX = pCameoTypeExt->CameoPCX.GetSurface())
			{
				RectangleStruct pcxBounds = { iconPos.X, iconPos.Y, curW, curH };
				PCX::Instance.BlitToSurface(&pcxBounds, pSurface, pCameoPCX);
				drawn = true;
			}
		}

		if (!drawn)
		{
			SHPStruct* pCameo = pCameoType->GetCameo();
			if (pCameo)
			{
				const auto pConvert = pCameoTypeExt->CameoPalette.GetOrDefaultConvert(FileSystem::CAMEO_PAL);
				pSurface->DrawSHP(
					pConvert,
					pCameo,
					0,
					&iconPos,
					&bounds,
					BlitterFlags::bf_400 | BlitterFlags::Alpha | translucencyFlag,
					0, 0,
					ZGradient::Ground,
					1000, 0, nullptr, 0, 0, 0
				);
				drawn = true;
			}
		}

		if (drawn)
		{
			wchar_t countText[16];
			_snwprintf_s(countText, _countof(countText), _TRUNCATE, L"%d", cameoCount);

			Point2D textPos = { iconPos.X + curW - 4, iconPos.Y + 2 };

			pSurface->DrawTextA(
				countText,
				&bounds,
				&textPos,
				Drawing::RGB_To_Int({ 0, 255, 0 }),
				0,
				TextPrintType::Center | TextPrintType::FullShadow
			);
		}

		idx++;
	}
}
