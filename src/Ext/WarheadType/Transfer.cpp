#include "Body.h"

#include <Utilities/Enum.h>
#include <Utilities/Helpers.Alex.h>
#include <Utilities/GeneralUtils.h>
#include <TechnoClass.h>
#include <TechnoTypeClass.h>
#include <Misc/FlyingStrings.h>
#include <type_traits>


double VeterancyMultiplier(TechnoClass* pTechno, Ability ability)
{
	if (pTechno == nullptr)
		return 1.0;
	double result = 1.0;
	double bonus = 1.0;
	bool veteranBonus = false;
	bool eliteBonus = false;
	TechnoTypeClass* pType = pTechno->GetTechnoType();

	switch (ability)
	{
	case Ability::Firepower:
		veteranBonus = pType->VeteranAbilities.FIREPOWER;
		eliteBonus = pType->EliteAbilities.FIREPOWER;
		bonus = RulesClass::Instance->VeteranCombat;
		break;
	case Ability::ROF:
		veteranBonus = pType->VeteranAbilities.ROF;
		eliteBonus = pType->EliteAbilities.ROF;
		bonus = RulesClass::Instance->VeteranROF;
		break;
	case Ability::Stronger:
		veteranBonus = pType->VeteranAbilities.STRONGER;
		eliteBonus = pType->EliteAbilities.STRONGER;
		bonus = RulesClass::Instance->VeteranArmor;
		break;
	case Ability::Faster:
		veteranBonus = pType->VeteranAbilities.FASTER;
		eliteBonus = pType->EliteAbilities.FASTER;
		bonus = RulesClass::Instance->VeteranSpeed;
		break;
	case Ability::Sight:
		veteranBonus = pType->VeteranAbilities.SIGHT;
		eliteBonus = pType->EliteAbilities.SIGHT;
		bonus = RulesClass::Instance->VeteranSight;
		break;
	default:
		return result;
	}

	if (pTechno->Veterancy.Veterancy >= 1.0f && veteranBonus)
		result *= bonus;
	if (pTechno->Veterancy.Veterancy >= 2.0f && eliteBonus)
		result *= bonus;

	return result;
}

double AttributeValue(TechnoClass* pTechno, TransferTypeResource attr, bool current = true, bool stage = false)
{
	if (pTechno == nullptr)
		return 0.0;
	auto pType = pTechno->GetTechnoType();
	double veterancyStage = pType->Cost * RulesClass::Instance->VeteranRatio;
	double currentStageVeterancy = veterancyStage * (pTechno->Veterancy.Veterancy - (int)pTechno->Veterancy.Veterancy);
	int gatling_stage = pTechno->CurrentGattlingStage;
	int gatling_value = pTechno->GattlingValue;
	int gatling_current_max = pTechno->Veterancy.IsElite()
		? pType->EliteStage[gatling_stage]
		: pType->WeaponStage[gatling_stage];
	int gatling_current_min = 0;
	if (gatling_stage > 0)
		gatling_current_min = pTechno->Veterancy.IsElite()
		? pType->EliteStage[gatling_stage - 1]
		: pType->WeaponStage[gatling_stage - 1];

	switch (attr)
	{
	case TransferTypeResource::Experience:
		if (current)
			if (stage)
				return currentStageVeterancy;
			else
				return veterancyStage * pTechno->Veterancy.Veterancy;
		else
			if (stage)
				return veterancyStage;
			else
				return veterancyStage * 2.0f;

	case TransferTypeResource::Money:
		if (current)
			return (double)pType->Cost;
		else
			return (double)pTechno->Owner->Available_Money();

	case TransferTypeResource::Health:
		if (current)
			return (double)pTechno->Health;
		else
			return (double)pType->Strength;

	case TransferTypeResource::Ammo:
		if (current)
			return (double)pTechno->Ammo;
		else
			return (double)pType->Ammo;

	case TransferTypeResource::GatlingRate:
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
	default:
		return 0.0;
	}
}

struct TransferUnit
{
	TechnoClass* Techno;
	HouseClass* House;

	double Value;
	double Multiplier;

	double CurrentAttributeValue;
	double TotalAttributeValue;

	TransferUnit(TechnoClass* pUnit, TransferTypeResource attr): Techno { pUnit }
		, House { pUnit != nullptr ? pUnit->Owner : nullptr }
		, Value { 0.0 }
		, Multiplier { 1.0 }
		, CurrentAttributeValue { AttributeValue(pUnit, attr) }
		, TotalAttributeValue { AttributeValue(pUnit, attr, false) }
	{ }
	TransferUnit(): TransferUnit(nullptr, TransferTypeResource::Experience) { }
};

class TransferDetails
{
	int AddValue(TransferUnit* pValues)
	{
		if (pValues->Value == 0)
			return 0;
		TechnoClass* pTechno = pValues->Techno;
		TechnoTypeClass* pType = nullptr;

		if (pTechno == nullptr && Options->Receive_Resource != TransferTypeResource::Money)
			return 0;
		if (pTechno != nullptr)
			pType = pTechno->GetTechnoType();

		int value = (int)(pValues->Value + 0.5);
		switch (Options->Receive_Resource)
		{
		case TransferTypeResource::Experience:
			if (pTechno->Veterancy.IsElite() || !pType->Trainable)
			{
				return 0;
			}
			else if (pValues->CurrentAttributeValue + value >= pValues->TotalAttributeValue)
			{
				pTechno->Veterancy.SetElite();
				return (int)(pValues->TotalAttributeValue - pValues->CurrentAttributeValue);
			}
			pTechno->Veterancy.Add(pType->Cost, value);
			return value;
		case TransferTypeResource::Money:
			pValues->House->GiveMoney(value);
			return value;
		case TransferTypeResource::Health:
			if (pTechno->Health + value > pType->Strength)
			{
				pTechno->Health = pType->Strength;
				return pType->Strength - pTechno->Health;
			}
			pTechno->Health += value;
			return value;
		case TransferTypeResource::Ammo:
			if (pType->Ammo <= 0)
				return 0;
			if (pTechno->Ammo + value > pType->Ammo)
			{
				pTechno->Ammo = pType->Ammo;
				return pType->Ammo - pTechno->Ammo;
			}
			pTechno->Ammo += value;
			return value;
		case TransferTypeResource::GatlingRate:
			if (!pType->IsGattling || pType->WeaponStages <= 1)
				return 0;
			int CurrentStage = pTechno->CurrentGattlingStage;
			if (pTechno->GattlingValue + value >= pValues->TotalAttributeValue)
			{
				pTechno->GattlingValue = (int)pValues->TotalAttributeValue;
				pTechno->CurrentGattlingStage = pType->WeaponStages-1;
				return pType->WeaponStages-1 - CurrentStage;
			}
			pTechno->GattlingValue += value;
			if (pTechno->GattlingValue < (pTechno->Veterancy.IsElite() ? pType->EliteStage[0] : pType->WeaponStage[0]))
			{
				pTechno->CurrentGattlingStage = CurrentStage;
				return CurrentStage;
			}
			for (int Stage = 1; Stage < pType->WeaponStages; Stage++)
			{
				if (pTechno->Veterancy.IsElite())
				{
					if (pTechno->GattlingValue >= pType->EliteStage[Stage-1]
						&& pTechno->GattlingValue < pType->EliteStage[Stage] && Stage < CurrentStage)
					{
						pTechno->CurrentGattlingStage = Stage;
						return Stage - CurrentStage;
					}
				}
				else
				{
					if (pTechno->GattlingValue >= pType->WeaponStage[Stage-1]
						&& pTechno->GattlingValue < pType->WeaponStage[Stage] && Stage < CurrentStage)
					{
						pTechno->CurrentGattlingStage = Stage;
						return Stage - CurrentStage;
					}
				}
			}
			return 0;
		}
		return 0;
	}

	int SubValue(TransferUnit* pValues)
	{
		if (pValues->Value == 0)
			return 0;
		TechnoClass* pTechno = pValues->Techno;
		TechnoTypeClass* pType = nullptr;

		if (pTechno == nullptr && Options->Send_Resource != TransferTypeResource::Money)
			return 0;
		if (pTechno != nullptr)
			pType = pTechno->GetTechnoType();

		int value = (int)(pValues->Value + 0.5);
		switch (Options->Send_Resource)
		{
		case TransferTypeResource::Experience:
			if (pTechno->Veterancy.Veterancy <= 0.0f || !pType->Trainable)
			{
				return 0;
			}
			else if (pValues->CurrentAttributeValue - value <= 0)
			{
				pTechno->Veterancy.Reset();
				return (int)(pValues->CurrentAttributeValue);
			}
			pTechno->Veterancy.Veterancy = (float)((pValues->CurrentAttributeValue - (double)value) / pValues->TotalAttributeValue);
			return value;
		case TransferTypeResource::Money:
			if (pValues->House->Available_Money() < value)
				value = pValues->House->Available_Money();
			pValues->House->TakeMoney(value);
			return value;
		case TransferTypeResource::Health:
			if (pTechno->Health - value <= 0)
			{
				pTechno->Health = 1;
				return pTechno->Health - 1;
			}
			pTechno->Health -= value;
			return value;
		case TransferTypeResource::Ammo:
			if (pType->Ammo <= 0)
				return 0;
			if (pTechno->Ammo - value < 0)
			{
				pTechno->Ammo = 0;
				return pTechno->Ammo;
			}
			pTechno->Ammo -= value;
			return value;
		case TransferTypeResource::GatlingRate:
			if (!pType->IsGattling || pType->WeaponStages <= 1)
				return 0;
			int CurrentStage = pTechno->CurrentGattlingStage;
			if (pTechno->GattlingValue - value <= 0)
			{
				pTechno->GattlingValue = 0;
				pTechno->CurrentGattlingStage = 0;
				return CurrentStage;
			}
			pTechno->GattlingValue -= value;
			if (pTechno->GattlingValue < (pTechno->Veterancy.IsElite() ? pType->EliteStage[0] : pType->WeaponStage[0]))
			{
				pTechno->CurrentGattlingStage = CurrentStage;
				return CurrentStage;
			}
			for (int Stage = 1; Stage < pType->WeaponStages; Stage++)
			{
				if (pTechno->Veterancy.IsElite())
				{
					if (pTechno->GattlingValue >= pType->EliteStage[Stage-1]
						&& pTechno->GattlingValue < pType->EliteStage[Stage] && Stage < CurrentStage)
					{
						pTechno->CurrentGattlingStage = Stage;
						return CurrentStage - Stage;
					}
				}
				else
				{
					if (pTechno->GattlingValue >= pType->WeaponStage[Stage-1]
						&& pTechno->GattlingValue < pType->WeaponStage[Stage] && Stage < CurrentStage)
					{
						pTechno->CurrentGattlingStage = Stage;
						return CurrentStage - Stage;
					}
				}
			}
			return 0;
		}
		return 0;
	}

	enum LogType {
		None = 0,
		Src = 1,
		Trg = 2,
		Both = Src | Trg,
		War = 4,
		All = Both | War
	};

	void LogDetails(const wchar_t* step, LogType ltype = LogType::All, TransferUnit* pTUnit = nullptr)
	{
		// return;
		Debug::Log("TransferDetails at %ls\n", step);
		int i = 0;
		if (ltype == LogType::All)
		{
			Debug::Log("TransferType: %s\n", Options->Name.data());
		}
		if (ltype & LogType::War)
		{
			Debug::Log("WarheadID: %s\n", SourceWarhead->ID);
		}
		if (ltype & LogType::Src)
		{
			if (Source.Techno == nullptr)
			{
				Debug::Log("HouseID: %s\n", Source.House->Type->ID);
			}
			else
			{
				Debug::Log("SourceID: %s Value: %.1f x %.3f Current: %.1f/%.1f\n", Source.Techno->GetTechnoType()->ID,
					Source.Value, Source.Multiplier, Source.CurrentAttributeValue, Source.TotalAttributeValue);
			}
		}
		if (ltype & LogType::Trg)
		{
			if (pTUnit == nullptr)
			{
				if (Targets.size() == 0)
				{
					return;
				}
				for (auto Target : Targets)
				{
					i++;
					if (Target.Techno != nullptr)
					{
						Debug::Log("%d - TargetID: %s Value: %.1f x %.3f Current: %.1f/%.1f\n", i, Target.Techno->GetTechnoType()->ID,
							Target.Value, Target.Multiplier, Target.CurrentAttributeValue, Target.TotalAttributeValue);
					}
				}
			}
			else
			{
				Debug::Log("TargetID: %s Value: %.1f x %.3f Current: %.1f/%.1f\n", pTUnit->Techno->GetTechnoType()->ID,
					pTUnit->Value, pTUnit->Multiplier, pTUnit->CurrentAttributeValue, pTUnit->TotalAttributeValue);
			}
		}
	}

public:
	TransferUnit Source;
	std::vector<TransferUnit> Targets;
	TransferTypeClass* Options;
	WarheadTypeClass* SourceWarhead;
	CoordStruct DetonationCoords;

	TransferDetails(TechnoClass* pUnit, HouseClass* pHouse, WarheadTypeClass* pWarhead, std::vector<TechnoClass*> pTechnoList, TransferTypeClass* pTType, CoordStruct coords)
	{
		this->Source.Techno = pUnit;
		this->Source.House = pHouse;
		this->Options = pTType;
		if (this->Options->TargetToSource)
		{			
			this->Source.CurrentAttributeValue = AttributeValue(pUnit, pTType->Receive_Resource);
			this->Source.TotalAttributeValue = AttributeValue(pUnit, pTType->Receive_Resource, false);
			for (auto pTechno : pTechnoList)
			{
				this->Targets.push_back(TransferUnit(pTechno, pTType->Send_Resource));
			}
		}
		else
		{
			this->Source.CurrentAttributeValue = AttributeValue(pUnit, pTType->Send_Resource);
			this->Source.TotalAttributeValue = AttributeValue(pUnit, pTType->Send_Resource, false);
			for (auto pTechno : pTechnoList)
			{
				this->Targets.push_back(TransferUnit(pTechno, pTType->Receive_Resource));
			}
		}
		this->SourceWarhead = pWarhead;
		this->DetonationCoords = coords;

	}
	TransferDetails() = delete;

	bool CalculateValues()
	{
		// LogDetails(L"CalculationStart");
		WarheadTypeClass* TargetWarhead = Options->Target_Warhead;
		if (!Options->Target_ConsiderArmor || TargetWarhead == nullptr)
			TargetWarhead = SourceWarhead;
		// LogDetails(L"SourceInside", LogType::Src);
		if (Options->TargetToSource ? Options->Receive_ConsiderSourceVeterancy : Options->Send_ConsiderSourceVeterancy)
		{
			Source.Multiplier *= VeterancyMultiplier(Source.Techno, Ability::Firepower);
		}
		// LogDetails(L"SourceVeterancy", LogType::Src);
		int TargetCount = Targets.size();
		for (auto Target = Targets.begin(); Target != Targets.end(); Target++)
		{
			if (!Options->Target_Spread_CountUnaffected)
			{
				// LogDetails(L"TargetConsiderArmor", LogType::Trg, &(*Target));
				if (Options->Target_ConsiderArmor)
					Target->Multiplier *= GeneralUtils::GetWarheadVersusArmor(TargetWarhead,
						Target->Techno->GetTechnoType()->Armor);
				// LogDetails(L"TargetVersusArmor", LogType::Trg, &(*Target));
				bool IsHouseAffected = Options->Target_AffectedHouses == AffectedHouse::All || (Source.House
					&& EnumFunctions::CanTargetHouse(Options->Target_AffectedHouses, Source.House, HouseClass::CurrentPlayer));
				if ((Options->Target_ConsiderArmor && Target->Multiplier == 0.0) || !IsHouseAffected)
				{
					Targets.erase(Target--);
					continue;
				}
				// LogDetails(L"TargetAffected", LogType::None);
			}

			if (Options->TargetToSource ? Options->Send_ConsiderSourceVeterancy : Options->Receive_ConsiderSourceVeterancy)
				Target->Multiplier *= VeterancyMultiplier(this->Source.Techno, Ability::Firepower);
			// LogDetails(L"TargetVeterancy", LogType::Trg, &(*Target));

			if (Options->Target_Spread_Distribution == SpreadDistribution::ByProximity)
				Target->Multiplier *= 1.0f - (1.0f - SourceWarhead->PercentAtMax)
					* (DetonationCoords.DistanceFrom(Target->Techno->GetCoords())
					/ Unsorted::LeptonsPerCell / SourceWarhead->CellSpread);
			// LogDetails(L"TargetDistribution:ByProximity", LogType::Trg, &(*Target));

			if (Options->TargetToSource)
			{
				Target->Value = Options->Send_Value;
				// LogDetails(L"TargetToSource", LogType::Trg, &(*Target));
				if (Options->Send_Value_IsPercent)
					Target->Value *= Options->Send_Value_PercentOfTotal ? Target->TotalAttributeValue : Target->CurrentAttributeValue;
				// LogDetails(L"TargetPercent", LogType::Trg, &(*Target));
			}
			else
			{
				Target->Value = Options->Receive_Value;
				// LogDetails(L"SourceToTarget", LogType::Trg, &(*Target));
				if (!Options->Receive_Value_ProportionalToSent && Options->Receive_Value_IsPercent)
					Target->Value *= Options->Receive_Value_PercentOfTotal ? Target->TotalAttributeValue : Target->CurrentAttributeValue;
				// LogDetails(L"TargetPercent", LogType::Trg, &(*Target));
			}
		}

		if (SourceWarhead->CellSpread && !Options->Target_Spread_CountUnaffected)
			TargetCount = Targets.size();
		// LogDetails(L"CalculationStartEnd", LogType::Both);

		if (Options->TargetToSource)
		{
			// LogDetails(L"TargetToSource", LogType::None);

			double LargestValue = 0;
			double ValueSum = 0.0;
			for (auto Target = Targets.begin(); Target != Targets.end(); Target++)
			{
				// LogDetails(L"TargetBeforeMultiplier", LogType::Trg, &(*Target));
				Target->Value *= Target->Multiplier
					/ (Options->Target_Spread_Distribution == SpreadDistribution::Equally ? TargetCount : 1);
				// LogDetails(L"TargetAfterMultiplier", LogType::Trg, &(*Target));

				if (Options->Send_Value_FlatMinimum && Target->Multiplier && Target->Value < Options->Send_Value_FlatMinimum)
					Target->Value = Options->Send_Value_FlatMinimum;
				// LogDetails(L"TargetFlatMinimum", LogType::Trg, &(*Target));
				if (Options->Send_Value_FlatMaximum && Target->Value > Options->Send_Value_FlatMaximum)
					Target->Value = Options->Send_Value_FlatMaximum;
				// LogDetails(L"TargetFlatMaximum", LogType::Trg, &(*Target));

				if (Options->Send_RequireRealResource && Target->Value > Target->CurrentAttributeValue)
					Target->Value = Target->CurrentAttributeValue;
				// LogDetails(L"TargetRealResource", LogType::Trg, &(*Target));

				if (Options->Send_Resource == TransferTypeResource::Experience && !Options->Send_Experience_AllowDemote
					&& Options->Send_RequireRealResource)
				{
					// LogDetails(L"TargetExperienceDemote", LogType::Trg, &(*Target));
					if (Target->Techno->Veterancy.IsElite())
					{
						Target->Value = 0.0;
						// LogDetails(L"TargetElite", LogType::Trg, &(*Target));
					}
					else if (Target->Techno->Veterancy.IsVeteran() && ((Target->CurrentAttributeValue - Target->Value)
						< (Target->TotalAttributeValue / 2.0f)))
					{
						Target->Value = Target->CurrentAttributeValue - Target->TotalAttributeValue / 2.0f;
						// LogDetails(L"TargetVeteran", LogType::Trg, &(*Target));
					}
				}

				if (Target->Value > LargestValue)
					LargestValue = Target->Value;

				ValueSum += Target->Value;
			}
			// LogDetails(L"CalculationMiddleEnd", LogType::Both);

			Source.Value = Options->Receive_Value * Source.Multiplier;
			// LogDetails(L"SourceReceive", LogType::Src);
			if (Options->Receive_Value_ProportionalToSent)
			{
				// LogDetails(L"SourceProportional", LogType::Src);
				if (Options->Receive_Value_IsPercent)
				{
					if (Options->Receive_Value_PercentOfTotal)
						Source.Value *= ValueSum;
					else
						Source.Value *= LargestValue;
					// LogDetails(L"SourceIsPercent", LogType::Src);
				}
				else
				{
					Source.Value *= TargetCount;
					// LogDetails(L"SourceIsNotPercent", LogType::Src);
				}
			}
			else
			{
				// LogDetails(L"SourceNotProportional", LogType::Src);
				if (Options->Receive_Value_IsPercent)
				{
					if (Options->Receive_Value_PercentOfTotal)
						Source.Value *= Source.TotalAttributeValue;
					else
						Source.Value *= Source.CurrentAttributeValue;
					// LogDetails(L"SourceIsPercent", LogType::Src);
				}
			}

			if (Options->Receive_Value_FlatMinimum && Source.Value < Options->Receive_Value_FlatMinimum)
				Source.Value = Options->Receive_Value_FlatMinimum;
			// LogDetails(L"SourceFlatMinimum", LogType::Src);
			if (Options->Receive_Value_FlatMaximum && Source.Value > Options->Receive_Value_FlatMaximum)
				Source.Value = Options->Receive_Value_FlatMaximum;
			// LogDetails(L"SourceFlatMaximum", LogType::Src);

			if (Options->Receive_RefuseOverflow && Options->Receive_Resource != TransferTypeResource::Money
				&& Source.Value + Source.CurrentAttributeValue > Source.TotalAttributeValue)
			{
				double Overflow = Source.Value + Source.CurrentAttributeValue - Source.TotalAttributeValue;
				for (auto Target = Targets.begin(); Target != Targets.end(); Target++)
				{
					Target->Value -= Target->Value / ValueSum * Overflow;
				}
			}
		}
		else
		{
			// LogDetails(L"SourceToTarget", LogType::None);
			Source.Value = Options->Send_Value;
			// LogDetails(L"SourceSend", LogType::Src);
			if (Options->Send_Value_IsPercent)
				Source.Value *= Options->Send_Value_PercentOfTotal ? Source.TotalAttributeValue : Source.CurrentAttributeValue;
			// LogDetails(L"SourcePercent", LogType::Src);

			Source.Value *= Source.Multiplier;
			// LogDetails(L"SourceMultiplier", LogType::Src);

			if (Options->Send_Value_FlatMinimum && Source.Multiplier && Source.Value < Options->Send_Value_FlatMinimum)
				Source.Value = Options->Send_Value_FlatMinimum;
			// LogDetails(L"SourceFlatMinimum", LogType::Src);
			if (Options->Send_Value_FlatMaximum && Source.Value > Options->Send_Value_FlatMaximum)
				Source.Value = Options->Send_Value_FlatMaximum;
			// LogDetails(L"SourceFlatMaximum", LogType::Src);

			if (Options->Send_RequireRealResource && Source.Value > Source.CurrentAttributeValue)
				Source.Value = Source.CurrentAttributeValue;
			// LogDetails(L"SourceRealResource", LogType::Src);

			if (!Options->Receive_RefuseOverflow && Source.Value > Source.CurrentAttributeValue)
				Source.Value = Source.CurrentAttributeValue;
			// LogDetails(L"SourceRealResource", LogType::Src);

			if (Options->Send_Resource == TransferTypeResource::Experience && !Options->Send_Experience_AllowDemote
				&& Options->Send_RequireRealResource)
			{
				// LogDetails(L"SourceExperienceDemote", LogType::Src);
				if (Source.Techno != nullptr)
				{
					if (Source.Techno->Veterancy.IsElite())
					{
						Source.Value = 0.0;
						// LogDetails(L"SourceElite", LogType::Src);
					}
					else if (Source.Techno->Veterancy.IsVeteran() && ((Source.CurrentAttributeValue - Source.Value)
						< (Source.TotalAttributeValue / 2.0f)))
					{
						Source.Value = Source.CurrentAttributeValue - Source.TotalAttributeValue / 2.0f;
						// LogDetails(L"SourceVeteran", LogType::Src);
					}
				}
			}

			double OverflowSum = 0;
			for (auto Target = Targets.begin(); Target != Targets.end(); Target++)
			{
				// LogDetails(L"TargetStart", LogType::Trg, &(*Target));
				Target->Value *= Target->Multiplier
					/ (Options->Target_Spread_Distribution == SpreadDistribution::Equally ? TargetCount : 1);
				// LogDetails(L"TargetMultiplier", LogType::Trg, &(*Target));
				if (Options->Receive_Value_ProportionalToSent)
				{
					// LogDetails(L"TargetProportional", LogType::Trg, &(*Target));
					if (Options->Receive_Value_IsPercent)
					{
						// LogDetails(L"TargetPercent", LogType::Trg, &(*Target));
						if (Options->Receive_Value_PercentOfTotal)
							Target->Value *= Source.Value;
						else
							Target->Value *= Source.Value / TargetCount;
						// LogDetails(L"TargetPercentTotal", LogType::Trg, &(*Target));
					}
				}

				if (Options->Receive_Value_FlatMinimum && Target->Value < Options->Receive_Value_FlatMinimum)
					Target->Value = Options->Receive_Value_FlatMinimum;
				// LogDetails(L"TargetFlatMinimum", LogType::Trg, &(*Target));
				if (Options->Receive_Value_FlatMaximum && Target->Value > Options->Receive_Value_FlatMaximum)
					Target->Value = Options->Receive_Value_FlatMaximum;
				// LogDetails(L"TargetFlatMaximum", LogType::Trg, &(*Target));

				if (Options->Receive_RefuseOverflow && Options->Receive_Resource != TransferTypeResource::Money
					&& Target->Value + Target->CurrentAttributeValue > Target->TotalAttributeValue)
				{
					OverflowSum += (Target->Value + Target->CurrentAttributeValue - Target->TotalAttributeValue)
						/ Target->Value * Source.Value / TargetCount;
				}
			}
			Source.Value -= OverflowSum;
		}
		// LogDetails(L"CalculationEnd", LogType::Both);
		return true;
	}

	bool ExecuteTransfer()
	{
		if (Options->TargetToSource)
		{
			int val = AddValue(&Source);
			if (val)
				Debug::Log("SourceID: %s Value: +%d\n", Source.Techno->GetTechnoType()->ID, val);
			if (Options->Receive_Text && val)
				FlyingStrings::AddNumberString(val, Source.House, Options->Receive_Text_Houses,
					Options->Receive_Text_Color, Source.Techno->Location, Options->Receive_Text_Offset,
					Options->Receive_Text_ShowSign, Options->Receive_Resource == TransferTypeResource::Money ? Phobos::UI::CostLabel : L"");
			for (auto Target = Targets.begin(); Target != Targets.end(); Target++)
			{
				val = SubValue(&(*Target));
				if (val)
					Debug::Log("TargetID: %s Value: -%d\n", Target->Techno->GetTechnoType()->ID, val);
				if (Options->Send_Text && val)
					FlyingStrings::AddNumberString(-val, Target->House, Options->Send_Text_Houses,
						Options->Send_Text_Color, Target->Techno->Location, Options->Send_Text_Offset,
						Options->Send_Text_ShowSign, Options->Send_Resource == TransferTypeResource::Money ? Phobos::UI::CostLabel : L"");
			}
		}
		else
		{
			int val = SubValue(&Source);
			if (Options->Send_Text && val)
				FlyingStrings::AddNumberString(-val, Source.House, Options->Send_Text_Houses,
					Options->Send_Text_Color, Source.Techno->Location, Options->Send_Text_Offset,
					Options->Send_Text_ShowSign, Options->Send_Resource == TransferTypeResource::Money ? Phobos::UI::CostLabel : L"");
			if (val)
				Debug::Log("SourceID: %s Value: -%d\n", Source.Techno->GetTechnoType()->ID, val);
			for (auto Target = Targets.begin(); Target != Targets.end(); Target++)
			{
				val = AddValue(&(*Target));
				if (val)
					Debug::Log("TargetID: %s Value: +%d\n", Target->Techno->GetTechnoType()->ID, val);
				if (Options->Receive_Text && val)
					FlyingStrings::AddNumberString(val, Target->House, Options->Receive_Text_Houses,
						Options->Receive_Text_Color, Target->Techno->Location, Options->Receive_Text_Offset,
						Options->Receive_Text_ShowSign, Options->Receive_Resource == TransferTypeResource::Money ? Phobos::UI::CostLabel : L"");
			}
		}
		return true;
	}
};

void WarheadTypeExt::ExtData::TransferWithGroup(TechnoClass* pSourceTechno, HouseClass* pSourceHouse, std::vector<TechnoClass*> pTargets, CoordStruct coords)
{
	for (auto transferType : this->Transfer_Types)
	{
		TransferDetails transfer(pSourceTechno, pSourceHouse, this->OwnerObject(), pTargets, transferType, coords);
		transfer.CalculateValues();
		transfer.ExecuteTransfer();
	}
}

void WarheadTypeExt::ExtData::TransferWithUnit(TechnoClass* pSourceTechno, HouseClass* pSourceHouse, TechnoClass* pTargetTechno, CoordStruct coords)
{
	std::vector<TechnoClass*> Target = {pTargetTechno};
	for (auto transferType : this->Transfer_Types)
	{
		TransferDetails transfer(pSourceTechno, pSourceHouse, this->OwnerObject(), Target, transferType, coords);
		transfer.CalculateValues();
		transfer.ExecuteTransfer();
	}
}