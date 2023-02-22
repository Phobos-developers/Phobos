#include "TransferClass.h"

#include <Utilities/Helpers.Alex.h>
#include <Misc/FlyingStrings.h>

int ModFrom1(const int& lvalue, const int& rvalue)
{
	return lvalue < 0 ? ((lvalue + 1) % rvalue - 1) : ((lvalue - 1) % rvalue + 1);
}

int ResourceValue(TechnoClass* pTechno, HouseClass* pHouse, TransferResource attr, bool current = true, bool stage = false)
{
	if (pTechno == nullptr && !(attr == TransferResource::Money && pHouse != nullptr))
		return 0;

	TechnoTypeClass* pType = nullptr;
	if (pTechno)
	{
		pType = pTechno->GetTechnoType();
	}

	switch (attr)
	{
	case TransferResource::Health:
		if (current)
			return pTechno->Health;
		else
			return pType->Strength;

	case TransferResource::Experience:
	{
		int veterancyStage = static_cast<int>(pType->Cost * RulesClass::Instance->VeteranRatio);
		int currentStageVeterancy = static_cast<int>(veterancyStage * (pTechno->Veterancy.Veterancy - static_cast<int>(pTechno->Veterancy.Veterancy)));
		if (current)
			if (stage)
				return currentStageVeterancy;
			else
				return static_cast<int>(veterancyStage * pTechno->Veterancy.Veterancy);
		else
			if (stage)
				return veterancyStage;
			else
				return veterancyStage * 2;
	}
	case TransferResource::Money:
		if (current)
			return pHouse->Available_Money();
		else
			return pType ? pType->Cost : 0;

	case TransferResource::Ammo:
		if (current)
			return pTechno->Ammo;
		else
			return pType->Ammo;

	case TransferResource::GatlingRate:
	{
		int gatling_stage = pTechno->CurrentGattlingStage;
		int gatling_value = pTechno->GattlingValue;
		int gatling_current_max = pTechno->Veterancy.IsElite()
			? pType->EliteStage[gatling_stage]
			: pType->WeaponStage[gatling_stage];
		int gatling_current_min = 0;
		if (gatling_stage > 0)
		{
			gatling_current_min = pTechno->Veterancy.IsElite()
				? pType->EliteStage[gatling_stage - 1]
				: pType->WeaponStage[gatling_stage - 1];
		}
		if (current)
			if (stage)
				return gatling_value - gatling_current_min;
			else
				return gatling_value;
		else
			if (stage)
				return gatling_current_max - gatling_current_min;
			else
				return pTechno->Veterancy.IsElite()
				? pType->EliteStage[pType->WeaponStages - 1]
				: pType->WeaponStage[pType->WeaponStages - 1];
	}
	default:
		return 0;
	}
}

TransferUnit::TransferUnit(TechnoClass* pTechno, TransferResource attr)
{
	if (pTechno == nullptr)
		return;

	this->Techno = pTechno;
	this->House = pTechno->Owner;
	this->Value = 0.0;
	this->Modifier = 1.0;
	this->Current = static_cast<double>(ResourceValue(pTechno, pTechno->Owner, attr));
	this->Total = static_cast<double>(ResourceValue(pTechno, pTechno->Owner, attr, false));
	this->Resource = attr;
}

TransferUnit::TransferUnit(HouseClass* pHouse)
{
	if (pHouse == nullptr)
		return;

	this->Techno = nullptr;
	this->House = pHouse;
	this->Value = 0.0;
	this->Modifier = 1.0;
	this->Current = static_cast<double>(ResourceValue(nullptr, pHouse, TransferResource::Money));
	this->Total = static_cast<double>(ResourceValue(nullptr, pHouse, TransferResource::Money));
	this->Resource = TransferResource::Money;
}

TransferClass::TransferClass(TechnoClass* pSTechno, HouseClass* pHouse, WarheadTypeClass* pWarhead, TechnoClass* pTTechno, TransferTypeClass* pTType, CoordStruct coords)
{
	this->SourceTechno = pSTechno;
	this->SourceHouse = pHouse;
	this->SourceWarhead = pWarhead;
	this->BulletTargetTechno = pTTechno;
	this->DetonationCoords = coords;
	this->Type = pTType;

	this->IsCellSpread = pWarhead->CellSpread > 0.0;
}

int TransferClass::ChangeHealth(TechnoClass* pTechno, int value, TechnoClass* pSource, HouseClass* pHouse, WarheadTypeClass* pWarhead, bool killable)
{
	value = -value;
	if (pTechno->Health - value <= 0 && !killable)
		value = pTechno->Health - 1;

	if (value == 0)
		return 0;
	pTechno->ReceiveDamage(&value, 0, pWarhead, pSource, true, false, pHouse);

	return -value;
}

int TransferClass::ChangeExperience(TechnoClass* pTechno, int value, bool demotable)
{
	if (!pTechno->GetTechnoType()->Trainable)
		return 0;
	int cost = pTechno->GetTechnoType()->Cost;
	double veterancyStage = static_cast<double>(cost) * RulesClass::Instance->VeteranRatio;
	int experience = static_cast<int>(pTechno->Veterancy.Veterancy * veterancyStage);

	if (value < 0 && !demotable)
	{
		if (pTechno->Veterancy.IsElite() || pTechno->Veterancy.Veterancy == 1.0f)
			return 0;
		else if (pTechno->Veterancy.IsVeteran() && (experience + value) < veterancyStage)
			value = static_cast<int>(veterancyStage) - experience;
	}

	pTechno->Veterancy.Add(cost, value);
	if (pTechno->Veterancy.IsNegative())
		pTechno->Veterancy.Reset();

	return static_cast<int>(pTechno->Veterancy.Veterancy * veterancyStage) - experience;
}

int TransferClass::ChangeMoney(HouseClass* pHouse, int value)
{
	if (value > 0)
	{
		pHouse->GiveMoney(value);
	}
	else
	{
		value = -value;
		if (pHouse->Available_Money() < value)
			value = pHouse->Available_Money();
		pHouse->TakeMoney(value);
		value = -value;
	}
	return value;
}

int TransferClass::ChangeAmmo(TechnoClass* pTechno, int value)
{
	int ammo = pTechno->Ammo;
	pTechno->Ammo = std::clamp(ammo + value, 0, pTechno->GetTechnoType()->Ammo);
	return pTechno->Ammo - ammo;
}

int TransferClass::ChangeGatlingRate(TechnoClass* pTechno, int value, int changeLimit, bool canCycle, bool change)
{
	if (pTechno == nullptr)
	{
		return 0;
	}

	TechnoTypeClass* pType = pTechno->GetTechnoType();
	if (!pType->IsGattling || pType->WeaponStages <= 1)
	{
		return 0;
	}

	int stageCount = pType->WeaponStages;
	if (stageCount > 6)
		stageCount = 6;
	if (changeLimit > stageCount - 1)
		changeLimit = stageCount - 1;

	std::vector<int> weaponStages = {};

	if (pTechno->Veterancy.IsElite())
		weaponStages.insert(weaponStages.end(), pType->EliteStage, pType->EliteStage + stageCount);
	else
		weaponStages.insert(weaponStages.end(), pType->WeaponStage, pType->WeaponStage + stageCount);

	std::vector<int>::iterator finder;

	int rate = pTechno->GattlingValue + value;
	int stage = pTechno->CurrentGattlingStage;
	int resultStage;

	if (canCycle)
	{
		rate = ModFrom1(rate, weaponStages[stageCount - 1]);
		if (rate < 0)
			finder = std::upper_bound(weaponStages.begin(), weaponStages.end(), rate + weaponStages[stageCount - 1]);
		else
			finder = std::upper_bound(weaponStages.begin(), weaponStages.end(), rate);

		resultStage = finder - weaponStages.begin();
		if (rate < 0)
			resultStage -= stageCount;
		if (pTechno->GattlingValue + value >= weaponStages[stageCount - 1])
			resultStage += stageCount;
		int stageChange = resultStage - stage;
		if (changeLimit >= 0 && std::abs(stageChange) > changeLimit)
		{
			if (stageChange < 0)
			{
				resultStage = stage - changeLimit;
				if (resultStage < 0)
				{
					resultStage += stageCount;
				}

				if (resultStage == 0)
				{
					rate = 0;
				}
				else
				{
					rate = weaponStages[resultStage - 1];
				}
			}
			else
			{
				resultStage = stage + changeLimit;
				if (resultStage >= stageCount)
				{
					resultStage -= stageCount;
				}
				rate = weaponStages[resultStage] - 1;
			}
		}
		if (rate < 0)
		{
			rate += weaponStages[stageCount - 1];
			resultStage += stageCount;
		}
		if (resultStage == stageCount)
			resultStage--;
		stage = resultStage;
	}
	else
	{
		rate = std::clamp(rate, 0, weaponStages[stageCount - 1]);
		finder = std::upper_bound(weaponStages.begin(), weaponStages.end(), rate);
		resultStage = finder - weaponStages.begin();

		int stageChange = resultStage - stage;
		if (changeLimit >= 0 && std::abs(stageChange) > changeLimit)
		{
			resultStage = stage + (stageChange < 0 ? -changeLimit : changeLimit);
			resultStage = std::clamp(resultStage, 0, stageCount - 1);
			if (stageChange < 0)
			{
				if (resultStage == 0)
				{
					rate = 0;
				}
				else
				{
					rate = weaponStages[resultStage - 1];
				}
			}
			else
			{
				rate = weaponStages[resultStage] - 1;
			}
		}
	}

	if (resultStage >= stageCount)
		resultStage = stageCount - 1;

	stage = resultStage;

	int diff = rate - pTechno->GattlingValue;

	if (change)
	{
		pTechno->GattlingValue = rate;
		pTechno->CurrentGattlingStage = stage;
	}

	return diff;
}

int TransferClass::AlterResource(TransferUnit* pValues)
{
	if (pValues->Value == 0)
		return 0;
	TechnoClass* pTechno = pValues->Techno;

	if (pTechno == nullptr && pValues->Resource != TransferResource::Money)
		return 0;

	int value = static_cast<int>(std::round(pValues->Value));

	switch (pValues->Resource)
	{
	case TransferResource::Health:
		return ChangeHealth(pTechno, value, SourceTechno, SourceHouse, SourceWarhead, !Type->Health_PreventKill);
	case TransferResource::Experience:
		return ChangeExperience(pTechno, value, !Type->Experience_PreventDemote);
	case TransferResource::Money:
		return ChangeMoney(pValues->House, value);
	case TransferResource::Ammo:
		return ChangeAmmo(pTechno, value);
	case TransferResource::GatlingRate:
		return ChangeGatlingRate(pTechno, value, Type->GatlingRate_LimitStageChange,
			TechnoTypeExt::ExtMap.Find(pTechno->GetTechnoType())->Gattling_Cycle);
	}
	return 0;
}

bool TransferClass::DetermineSides()
{
	bool SendOnlyHouseIsNeeded = Type->Send_Resource == TransferResource::Money
		&& (Type->Send_Value_Type == TechnoValueType::Fixed || Type->Send_Value_Type == TechnoValueType::Current);
	bool ReceiveOnlyHouseIsNeeded = Type->Receive_Resource == TransferResource::Money
		&& (Type->Receive_Value_Type == TechnoValueType::Fixed || Type->Receive_Value_Type == TechnoValueType::Current);

	WarheadTypeClass* extraWarhead = Type->Extra_Warhead;
	if (extraWarhead == nullptr)
	{
		extraWarhead = SourceWarhead;
	}

	switch (Type->Direction)
	{
	case TransferDirection::SourceToTarget:
	{
		if (SourceTechno == nullptr && !SendOnlyHouseIsNeeded)
		{
			FailureMessage = "Failure on DetermineSides(): no Source Techno for SourceToTarget.\n";
			return false;
		}

		if (SourceTechno == nullptr)
		{
			Senders.emplace_back(SourceHouse);
		}
		else if (SourceTechno)
		{
			Senders.emplace_back(SourceTechno, Type->Send_Resource);
		}

		if (IsCellSpread)
		{
			auto targetList = Helpers::Alex::getCellSpreadItems(DetonationCoords, SourceWarhead->CellSpread, true);
			for (auto pTarget : targetList)
			{
				Receivers.emplace_back(pTarget, Type->Receive_Resource);
			}
		}
		else if (BulletTargetTechno)
		{
			Receivers.emplace_back(BulletTargetTechno, Type->Receive_Resource);
		}

		IsReceiverTarget = true;
		return true;
	}
	case TransferDirection::SourceToSource:
	{
		if (SourceTechno == nullptr && !(SendOnlyHouseIsNeeded && ReceiveOnlyHouseIsNeeded))
		{
			FailureMessage = "Failure on DetermineSides(): no Source Techno for SourceToSource.\n";
			return false;
		}

		if (SourceTechno == nullptr)
		{
			Senders.emplace_back(SourceHouse);
			Receivers.emplace_back(SourceHouse);
		}
		else
		{
			Senders.emplace_back(SourceTechno, Type->Send_Resource);
			Receivers.emplace_back(SourceTechno, Type->Receive_Resource);
		}

		return true;
	}
	case TransferDirection::TargetToSource:
	{
		if (SourceTechno == nullptr && !ReceiveOnlyHouseIsNeeded)
		{
			FailureMessage = "Failure on DetermineSides(): no Source Techno for TargetToSource.\n";
			return false;
		}

		if (IsCellSpread)
		{
			auto targetList = Helpers::Alex::getCellSpreadItems(DetonationCoords, SourceWarhead->CellSpread, true);
			for (auto pTarget : targetList)
			{
				Senders.emplace_back(pTarget, Type->Send_Resource);
			}
		}
		else if (BulletTargetTechno)
		{
			Senders.emplace_back(BulletTargetTechno, Type->Send_Resource);
		}

		if (SourceTechno == nullptr)
		{
			Receivers.emplace_back(SourceHouse);
		}
		else if (SourceTechno)
		{
			Receivers.emplace_back(SourceTechno, Type->Receive_Resource);
		}

		IsSenderTarget = true;
		return true;
	}
	case TransferDirection::TargetToExtra:
	{
		if (Type->Extra_Spread_EpicenterIsSource && SourceTechno == nullptr)
		{
			FailureMessage = "Failure on DetermineSides(): no Source Techno for Extra.Spread.EpicenterIsSource.\n";
			return false;
		}

		if (IsCellSpread)
		{
			auto targetList = Helpers::Alex::getCellSpreadItems(DetonationCoords, SourceWarhead->CellSpread, true);
			for (auto pTarget : targetList)
			{
				Senders.emplace_back(pTarget, Type->Send_Resource);
			}
		}
		else if (BulletTargetTechno)
		{
			Senders.emplace_back(BulletTargetTechno, Type->Send_Resource);
		}

		CoordStruct extraEpicenter = DetonationCoords;
		if (Type->Extra_Spread_EpicenterIsSource)
		{
			extraEpicenter = SourceTechno->Location;
		}
		auto extraList = Helpers::Alex::getCellSpreadItems(extraEpicenter, extraWarhead->CellSpread, true);

		for (auto pExtra : extraList)
		{
			Receivers.emplace_back(pExtra, Type->Receive_Resource);
		}

		IsSenderTarget = true;
		IsReceiverExtra = true;
		return true;
	}
	case TransferDirection::TargetToTarget:
	{
		if (IsCellSpread)
		{
			auto targetList = Helpers::Alex::getCellSpreadItems(DetonationCoords, SourceWarhead->CellSpread, true);
			for (auto pTarget : targetList)
			{
				Senders.emplace_back(pTarget, Type->Send_Resource);
				Receivers.emplace_back(pTarget, Type->Receive_Resource);
			}
		}
		else if (BulletTargetTechno)
		{
			Senders.emplace_back(BulletTargetTechno, Type->Send_Resource);
			Receivers.emplace_back(BulletTargetTechno, Type->Receive_Resource);
		}

		IsSenderTarget = true;
		IsReceiverTarget = true;
		return true;
	}
	case TransferDirection::ExtraToTarget:
	{
		if (Type->Extra_Spread_EpicenterIsSource && SourceTechno == nullptr)
		{
			FailureMessage = "Failure on DetermineSides(): no Source Techno for Extra.Spread.EpicenterIsSource.\n";
			return false;
		}

		CoordStruct extraEpicenter = DetonationCoords;
		if (Type->Extra_Spread_EpicenterIsSource)
		{
			extraEpicenter = SourceTechno->Location;
		}
		auto extraList = Helpers::Alex::getCellSpreadItems(extraEpicenter, extraWarhead->CellSpread, true);

		for (auto pExtra : extraList)
		{
			Senders.emplace_back(pExtra, Type->Send_Resource);
		}

		if (IsCellSpread)
		{
			auto targetList = Helpers::Alex::getCellSpreadItems(DetonationCoords, SourceWarhead->CellSpread, true);
			for (auto pTarget : targetList)
			{
				Receivers.emplace_back(pTarget, Type->Receive_Resource);
			}
		}
		else if (BulletTargetTechno)
		{
			Receivers.emplace_back(BulletTargetTechno, Type->Send_Resource);
		}

		IsSenderExtra = true;
		IsReceiverTarget = true;
		return true;
	}
	}

	FailureMessage = "Failure on DetermineSides(): \"successfully\" reached end\n";
	return false;
}

bool TransferClass::ApplyModifiers()
{
	WarheadTypeClass* targetWarhead = Type->Target_VersusWarhead;
	if (targetWarhead == nullptr)
	{
		targetWarhead = SourceWarhead;
	}

	WarheadTypeClass* extraWarhead = Type->Extra_Warhead;
	if (extraWarhead == nullptr)
	{
		extraWarhead = SourceWarhead;
	}

	for (auto Sender = Senders.begin(); Sender != Senders.end(); Sender++)
	{
		if (IsSenderTarget)
		{
			bool IsHouseAffected = Type->Target_AffectHouses == AffectedHouse::All
				|| EnumFunctions::CanTargetHouse(Type->Target_AffectHouses, SourceHouse, Sender->House);

			if (!IsHouseAffected || (IsCellSpread
				&& Type->Target_Spread_IgnoreSelf && Sender->Techno == SourceTechno))
			{
				Senders.erase(Sender--);
				continue;
			}

			if (Type->Target_ConsiderArmor)
			{
				Sender->Modifier *= GeneralUtils::GetWarheadVersusArmor(targetWarhead, Sender->Techno->GetTechnoType()->Armor);
			}

			if (IsCellSpread)
			{
				Sender->Modifier *= 1.0f - (1.0f - SourceWarhead->PercentAtMax)
					* DetonationCoords.DistanceFrom(Sender->Techno->GetCoords())
					/ Unsorted::LeptonsPerCell / SourceWarhead->CellSpread;
			}

			if (Sender->Techno->Veterancy.IsVeteran())
				Sender->Modifier *= Type->VeterancyMultiplier_TargetOverTarget.Get().X;
			else if (Sender->Techno->Veterancy.IsElite())
				Sender->Modifier *= Type->VeterancyMultiplier_TargetOverTarget.Get().Y;

			if (Sender->Modifier == 0.0)
			{
				Senders.erase(Sender--);
				continue;
			}
		}
		else if (IsSenderExtra)
		{
			bool IsHouseAffected = Type->Extra_AffectHouses == AffectedHouse::All
				|| EnumFunctions::CanTargetHouse(Type->Extra_AffectHouses, SourceHouse, Sender->House);

			if (!IsHouseAffected || (IsCellSpread && Type->Extra_Spread_IgnoreEpicenter
				&& Sender->Techno == (Type->Extra_Spread_EpicenterIsSource ? SourceTechno : BulletTargetTechno)))
			{
				Senders.erase(Sender--);
				continue;
			}

			if (Type->Extra_ConsiderArmor)
			{
				Sender->Modifier *= GeneralUtils::GetWarheadVersusArmor(extraWarhead, Sender->Techno->GetTechnoType()->Armor);
			}

			CoordStruct extraEpicenter = DetonationCoords;
			if (SourceTechno && Type->Extra_Spread_EpicenterIsSource)
			{
				extraEpicenter = SourceTechno->Location;
			}

			if (IsCellSpread)
			{
				Sender->Modifier *= 1.0f - (1.0f - extraWarhead->PercentAtMax)
					* extraEpicenter.DistanceFrom(Sender->Techno->GetCoords())
					/ Unsorted::LeptonsPerCell / extraWarhead->CellSpread;
			}

			if (Sender->Techno->Veterancy.IsVeteran())
				Sender->Modifier *= Type->VeterancyMultiplier_ExtraOverExtra.Get().X;
			else if (Sender->Techno->Veterancy.IsElite())
				Sender->Modifier *= Type->VeterancyMultiplier_ExtraOverExtra.Get().Y;

			if (Sender->Modifier == 0.0)
			{
				Senders.erase(Sender--);
				continue;
			}
		}

		if (SourceTechno && SourceTechno->Veterancy.Veterancy >= 1.0f)
		{
			if (SourceTechno->Veterancy.IsVeteran())
				Sender->Modifier *= Type->VeterancyMultiplier_SourceOverSender.Get().X;
			else if (SourceTechno->Veterancy.IsElite())
				Sender->Modifier *= Type->VeterancyMultiplier_SourceOverSender.Get().Y;
		}
	}

	for (auto Receiver = Receivers.begin(); Receiver != Receivers.end(); Receiver++)
	{
		if (IsReceiverTarget)
		{
			bool IsHouseAffected = Type->Target_AffectHouses == AffectedHouse::All
				|| EnumFunctions::CanTargetHouse(Type->Target_AffectHouses, SourceHouse, Receiver->House);

			if (!IsHouseAffected || (IsCellSpread
				&& Type->Target_Spread_IgnoreSelf && Receiver->Techno == SourceTechno))
			{
				Receivers.erase(Receiver--);
				continue;
			}

			if (Type->Target_ConsiderArmor)
			{
				Receiver->Modifier *= GeneralUtils::GetWarheadVersusArmor(targetWarhead, Receiver->Techno->GetTechnoType()->Armor);
			}

			if (IsCellSpread)
			{
				Receiver->Modifier *= 1.0f - (1.0f - SourceWarhead->PercentAtMax)
					* DetonationCoords.DistanceFrom(Receiver->Techno->GetCoords())
					/ Unsorted::LeptonsPerCell / SourceWarhead->CellSpread;
			}

			if (Receiver->Techno->Veterancy.IsVeteran())
				Receiver->Modifier *= Type->VeterancyMultiplier_TargetOverTarget.Get().X;
			else if (Receiver->Techno->Veterancy.IsElite())
				Receiver->Modifier *= Type->VeterancyMultiplier_TargetOverTarget.Get().Y;

			if (Receiver->Modifier == 0.0)
			{
				Receivers.erase(Receiver--);
				continue;
			}
		}
		else if (IsReceiverExtra)
		{
			bool IsHouseAffected = Type->Extra_AffectHouses == AffectedHouse::All
				|| EnumFunctions::CanTargetHouse(Type->Extra_AffectHouses, SourceHouse, Receiver->House);

			if (!IsHouseAffected || (IsCellSpread && Type->Extra_Spread_IgnoreEpicenter
				&& Receiver->Techno == (Type->Extra_Spread_EpicenterIsSource ? SourceTechno : BulletTargetTechno)))
			{
				Receivers.erase(Receiver--);
				continue;
			}

			if (Type->Extra_ConsiderArmor)
			{
				Receiver->Modifier *= GeneralUtils::GetWarheadVersusArmor(extraWarhead, Receiver->Techno->GetTechnoType()->Armor);
			}

			CoordStruct extraEpicenter = DetonationCoords;
			if (SourceTechno && Type->Extra_Spread_EpicenterIsSource)
			{
				extraEpicenter = SourceTechno->Location;
			}

			if (IsCellSpread)
			{
				Receiver->Modifier *= 1.0f - (1.0f - extraWarhead->PercentAtMax)
					* extraEpicenter.DistanceFrom(Receiver->Techno->GetCoords())
					/ Unsorted::LeptonsPerCell / extraWarhead->CellSpread;
			}

			if (Receiver->Techno->Veterancy.IsVeteran())
				Receiver->Modifier *= Type->VeterancyMultiplier_ExtraOverExtra.Get().X;
			else if (Receiver->Techno->Veterancy.IsElite())
				Receiver->Modifier *= Type->VeterancyMultiplier_ExtraOverExtra.Get().Y;

			if (Receiver->Modifier == 0.0)
			{
				Receivers.erase(Receiver--);
				continue;
			}
		}

		if (SourceTechno && SourceTechno->Veterancy.Veterancy >= 1.0f)
		{
			if (SourceTechno->Veterancy.IsVeteran())
				Receiver->Modifier *= Type->VeterancyMultiplier_SourceOverReceiver.Get().X;
			else if (SourceTechno->Veterancy.IsElite())
				Receiver->Modifier *= Type->VeterancyMultiplier_SourceOverReceiver.Get().Y;
		}
	}

	return true;
}

bool TransferClass::RegisterValues()
{
	for (auto Sender = Senders.begin(); Sender != Senders.end(); Sender++)
	{
		Sender->Value = Type->Send_Value;

		if (Type->Send_Value_Type != TechnoValueType::Fixed)
		{
			switch (Type->Send_Value_Type)
			{
			case TechnoValueType::Current:
				Sender->Value *= Sender->Current;
				break;
			case TechnoValueType::Missing:
				if (Type->Send_Resource == TransferResource::Money)
					Sender->Value *= Sender->Techno ? Sender->Techno->GetTechnoType()->Soylent : 0;
				else
					Sender->Value *= Sender->Total - Sender->Current;
				break;
			case TechnoValueType::Total:
				Sender->Value *= Sender->Total;
				break;
			}
		}

		Sender->Value *= Sender->Modifier;
	}

	for (auto Receiver = Receivers.begin(); Receiver != Receivers.end(); Receiver++)
	{
		Receiver->Value = Type->Receive_Value;

		if (Type->Receive_Value_Type != TechnoValueType::Fixed)
		{
			switch (Type->Receive_Value_Type)
			{
			case TechnoValueType::Current:
				Receiver->Value *= Receiver->Current;
				break;
			case TechnoValueType::Missing:
				if (Type->Receive_Resource == TransferResource::Money)
					Receiver->Value *= Receiver->Techno ? Receiver->Techno->GetTechnoType()->Soylent : 0;
				else
					Receiver->Value *= Receiver->Total - Receiver->Current;
				break;
			case TechnoValueType::Total:
				Receiver->Value *= Receiver->Total;
				break;
			}
		}

		Receiver->Value *= Receiver->Modifier;
	}

	return true;
}

bool TransferClass::ValidateLimits()
{
	double Highest = 0.0;
	double PositiveSum = 0.0;
	double NegativeSum = 0.0;

	std::map<HouseClass*, double> SentResourceSharedByHouse;

	for (auto Sender = Senders.begin(); Sender != Senders.end(); Sender++)
	{
		if (Sender->Modifier == 0)
			continue;

		if (Sender->Modifier > 0)
		{
			if (Type->Send_Value_FlatLimits.Get().X && Sender->Value < Type->Send_Value_FlatLimits.Get().X)
				Sender->Value = Type->Send_Value_FlatLimits.Get().X;
			if (Type->Send_Value_FlatLimits.Get().Y && Sender->Value > Type->Send_Value_FlatLimits.Get().Y)
				Sender->Value = Type->Send_Value_FlatLimits.Get().Y;
		}
		else
		{
			if (Type->Send_Value_FlatLimits.Get().Y && Sender->Value < -Type->Send_Value_FlatLimits.Get().Y)
				Sender->Value = -Type->Send_Value_FlatLimits.Get().Y;
			if (Type->Send_Value_FlatLimits.Get().X && Sender->Value > -Type->Send_Value_FlatLimits.Get().X)
				Sender->Value = -Type->Send_Value_FlatLimits.Get().X;
		}

		if (Type->Send_PreventOverflow && Type->Send_Resource != TransferResource::Money
			&& Sender->Value + Sender->Current > Sender->Total)
		{
			Sender->Value = Sender->Total - Sender->Current;
		}

		if (Type->Send_PreventUnderflow && Sender->Value < 0)
		{
			if (Type->Send_Resource == TransferResource::Experience && Type->Experience_PreventDemote)
			{
				if (Sender->Techno->Veterancy.IsElite() || Sender->Techno->Veterancy.Veterancy == 1.0f)
					Sender->Value = 0;
				else if (Sender->Techno->Veterancy.IsVeteran() && ((Sender->Current + Sender->Value) < (Sender->Total / 2)))
					Sender->Value = Sender->Total / 2 - Sender->Current;
				else if (Sender->Current < -Sender->Value)
					Sender->Value = -Sender->Current;
			}
			else if (Sender->Current < -Sender->Value)
			{
				Sender->Value = -Sender->Current;

				if (Type->Send_Resource == TransferResource::Health && Type->Health_PreventKill)
					Sender->Value += 1.0;
			}

			if (Type->Send_Resource == TransferResource::Money)
			{
				SentResourceSharedByHouse[Sender->House] += Sender->Value;
			}
		}
	}

	for (auto Sender = Senders.begin(); Sender != Senders.end(); Sender++)
	{
		if (Type->Send_PreventUnderflow && Type->Send_Resource == TransferResource::Money)
		{
			if (-SentResourceSharedByHouse[Sender->House] > Sender->Current && Sender->Value < 0)
			{
				Sender->Value -= Sender->Value * Sender->Value / SentResourceSharedByHouse[Sender->House];
			}
		}

		if (Sender->Value < 0)
		{
			NegativeSum += Sender->Value;

			if (Type->Send_Value < 0 && Sender->Value < Highest)
				Highest = Sender->Value;
		}
		else
		{
			PositiveSum += Sender->Value;

			if (Type->Send_Value > 0 && Sender->Value > Highest)
				Highest = Sender->Value;
		}
	}

	double Factor = 1.0;

	if (Type->Receive_SentFactor != TransferFactor::None && Senders.size() == 0)
	{
		Factor = 0.0;
	}
	else
	{
		switch (Type->Receive_SentFactor)
		{
		case TransferFactor::Highest:
			Factor = Highest;
			break;
		case TransferFactor::Sum:
			Factor = PositiveSum + NegativeSum;
			break;
		case TransferFactor::Average:
			Factor = (PositiveSum + NegativeSum) / static_cast<double>(Senders.size());
			break;
		case TransferFactor::Count:
			Factor = static_cast<double>(Senders.size());
			break;
		default:
			break;
		}
	}

	int idx = -1;
	for (auto Receiver = Receivers.begin(); Receiver != Receivers.end(); Receiver++)
	{
		idx++;
		if (Receiver->Modifier == 0)
			continue;

		if (Type->Direction == TransferDirection::TargetToTarget
			&& Type->Receive_SentFactor == TransferFactor::Average && Type->Receive_SentSplit)
		{
			Receiver->Value *= Senders[idx].Value;
		}
		else
		{
			Receiver->Value *= Factor;

			if (Type->Receive_SentSplit)
			{
				Receiver->Value /= static_cast<double>(Receivers.size());
			}
		}

		if (Receiver->Modifier > 0)
		{
			if (Type->Receive_Value_FlatLimits.Get().X && Receiver->Value < Type->Receive_Value_FlatLimits.Get().X)
				Receiver->Value = Type->Receive_Value_FlatLimits.Get().X;
			if (Type->Receive_Value_FlatLimits.Get().Y && Receiver->Value > Type->Receive_Value_FlatLimits.Get().Y)
				Receiver->Value = Type->Receive_Value_FlatLimits.Get().Y;
		}
		else
		{
			if (Type->Receive_Value_FlatLimits.Get().Y && Receiver->Value < -Type->Receive_Value_FlatLimits.Get().Y)
				Receiver->Value = -Type->Receive_Value_FlatLimits.Get().Y;
			if (Type->Receive_Value_FlatLimits.Get().X && Receiver->Value > -Type->Receive_Value_FlatLimits.Get().X)
				Receiver->Value = -Type->Receive_Value_FlatLimits.Get().X;
		}
	}

	return true;
}

bool TransferClass::EnforceChanges()
{
	int i = 0;
	for (auto Sender = Senders.begin(); Sender != Senders.end(); Sender++)
	{
		int SenderChange = AlterResource(&(*Sender));
		// Debug::Log("Sender[%d] change: %d\n", i++, SenderChange);
		if (Type->Send_Resource == TransferResource::Money && Type->Money_Display_Sender && Sender->Techno)
		{
			FlyingStrings::AddMoneyString(SenderChange, Sender->House,
				Type->Money_Display_Sender_Houses, Sender->Techno->Location,
				Type->Money_Display_Sender_Offset);
		}
	}

	i = 0;
	for (auto Receiver = Receivers.begin(); Receiver != Receivers.end(); Receiver++)
	{
		int ReceiverChange = AlterResource(&(*Receiver));
		// Debug::Log("Receiver[%d] change: %d\n", i++, ReceiverChange);
		if (Type->Receive_Resource == TransferResource::Money && Type->Money_Display_Receiver && Receiver->Techno)
		{
			FlyingStrings::AddMoneyString(ReceiverChange, Receiver->House,
				Type->Money_Display_Receiver_Houses, Receiver->Techno->Location,
				Type->Money_Display_Receiver_Offset);
		}
	}

	return true;
}

bool TransferClass::PerformTransfer()
{
	if (DetermineSides())
		if (ApplyModifiers())
			if (RegisterValues())
				if (ValidateLimits())
					if (EnforceChanges())
						return true;
	return false;
}
