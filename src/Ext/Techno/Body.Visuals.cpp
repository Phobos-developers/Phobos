#include "Body.h"

#include <TacticalClass.h>
#include <SpawnManagerClass.h>
#include <FactoryClass.h>
#include <SuperClass.h>
#include <Ext/SWType/Body.h>
#include <Ext/House/Body.h>
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

		if (!pDisplayType->VisibleInSpecialState && (pThis->TemporalTargetingMe || pThis->IsIronCurtained()))
			continue;

		int value = -1;
		int maxValue = 0;

		GetValuesForDisplay(pThis, pDisplayType->InfoType, value, maxValue);

		if (value <= -1 || maxValue <= 0)
			continue;

		const auto divisor = pDisplayType->ValueScaleDivisor.Get(pDisplayType->ValueAsTimer ? 15 : 1);

		if (divisor > 1)
		{
			value = Math::max(value / divisor, value ? 1 : 0);
			maxValue = Math::max(maxValue / divisor, 1);
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
		const auto pShield = pExt->Shield.get();

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

		value = pCaptureManager->ControlNodes.Count;
		maxValue = pCaptureManager->MaxControlNodes;
		break;
	}
	case DisplayInfoType::Spawns:
	{
		const auto pSpawnManager = pThis->SpawnManager;

		if (!pSpawnManager || !pType->Spawns)
			return;

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

		value = (pThis->Ammo >= pType->Ammo) ? 0 : pThis->ReloadTimer.GetTimeLeft();
		maxValue = (pThis->Ammo || pType->EmptyReload <= 0) ? pType->Reload : pType->EmptyReload;
		break;
	}
	case DisplayInfoType::SpawnTimer:
	{
		const auto pSpawnManager = pThis->SpawnManager;

		if (!pSpawnManager || !pType->Spawns || pType->SpawnsNumber <= 0)
			return;

		value = pSpawnManager->RegenRate;
		maxValue = value;

		for (int i = 0; i < pType->SpawnsNumber; ++i)
		{
			const auto pSpawnNode = pSpawnManager->SpawnedNodes[i];

			if (pSpawnNode->Status == SpawnNodeStatus::Dead)
				value = Math::min(value, pSpawnNode->SpawnTimer.GetTimeLeft());
		}

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
		if (!pExt->TypeExtData->PassengerDeletionType)
			return;

		const auto& timer = pExt->PassengerDeletionTimer;
		value = timer.GetTimeLeft();
		maxValue = timer.TimeLeft;
		break;
	}
	case DisplayInfoType::AutoDeath:
	{
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

		const auto pHouse = pThis->Owner;
		const auto pBuildingType = static_cast<BuildingTypeClass*>(pType);
		auto getSuperTimer = [pHouse, pBuildingType]() -> CDTimerClass*
		{
			if (pBuildingType->SuperWeapon != -1)
				return &pHouse->Supers.GetItem(pBuildingType->SuperWeapon)->RechargeTimer;
			else if (pBuildingType->SuperWeapon2 != -1)
				return &pHouse->Supers.GetItem(pBuildingType->SuperWeapon2)->RechargeTimer;

			const auto& superWeapons = BuildingTypeExt::ExtMap.Find(pBuildingType)->SuperWeapons;
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
	default:
	{
		value = pThis->Health;
		maxValue = pType->Strength;
		break;
	}
	}
}
