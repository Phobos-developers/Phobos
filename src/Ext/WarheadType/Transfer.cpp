#include "Body.h"

#include <Utilities/Enum.h>
#include <Utilities/Helpers.Alex.h>
#include <Utilities/GeneralUtils.h>
#include <TechnoClass.h>
#include <TechnoTypeClass.h>
#include <Misc/FlyingStrings.h>

bool ColorAnyNot(ColorStruct color, BYTE range)
{
	return color.R != range && color.G != range && color.B != range;
}

int ResourceValue(TechnoClass* pTechno, HouseClass* pHouse, TransferTypeResource attr, bool current = true, bool stage = false)
{
	if (pTechno == nullptr && !(attr == TransferTypeResource::Money && pHouse != nullptr && current))
		return 0;

	TechnoTypeClass* pType = nullptr;
	if (pTechno)
	{
		pType = pTechno->GetTechnoType();
	}

	switch (attr)
	{
	case TransferTypeResource::Experience:
	{
		int veterancyStage = (int)(pType->Cost * RulesClass::Instance->VeteranRatio);
		int currentStageVeterancy = (int)(veterancyStage * (pTechno->Veterancy.Veterancy - (int)pTechno->Veterancy.Veterancy));
		if (current)
			if (stage)
				return currentStageVeterancy;
			else
				return (int)(veterancyStage * pTechno->Veterancy.Veterancy);
		else
			if (stage)
				return veterancyStage;
			else
				return veterancyStage * 2;
	}
	case TransferTypeResource::Money:
		if (current)
			return pHouse->Available_Money();
		else
			return pType->Cost;

	case TransferTypeResource::Health:
		if (current)
			return pTechno->Health;
		else
			return pType->Strength;

	case TransferTypeResource::Ammo:
		if (current)
			return pTechno->Ammo;
		else
			return pType->Ammo;

	case TransferTypeResource::GatlingRate:
	{
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

struct TransferUnit
{
	TechnoClass* Techno;
	HouseClass* House;

	double Value;
	double Modifier;

	/// @brief Current resource value
	double Current;
	/// @brief Total resource value
	double Total;
	TransferTypeResource Resource;

	TransferUnit(TechnoClass* pUnit, TransferTypeResource attr): Techno { pUnit }
		, House { pUnit != nullptr ? pUnit->Owner : nullptr }
		, Value { 0.0 }
		, Modifier { 1.0 }
		, Current { (double)ResourceValue(pUnit, pUnit != nullptr ? pUnit->Owner : nullptr, attr) }
		, Total { (double)ResourceValue(pUnit, pUnit != nullptr ? pUnit->Owner : nullptr, attr, false) }
		, Resource { attr }
	{ }
	TransferUnit(): TransferUnit(nullptr, TransferTypeResource::Health) { }
};

class TransferDetails
{
public:
	TransferUnit Source;
	TechnoClass* BulletTargetTechno;
	std::vector<TransferUnit> Targets;
	TransferTypeClass* Options;
	WarheadTypeClass* SourceWarhead;
	CoordStruct DetonationCoords;

	TransferDetails(TechnoClass* pSTechno, HouseClass* pHouse, WarheadTypeClass* pWarhead, TechnoClass* pTTechno, std::vector<TechnoClass*> pTechnoList, TransferTypeClass* pTType, CoordStruct coords)
	{
		this->Source.Techno = pSTechno;
		this->Source.House = pHouse;
		this->BulletTargetTechno = pTTechno;
		this->Options = pTType;

		if (this->Options->TargetToSource)
		{
			this->Source.Current = (double)ResourceValue(pSTechno, pHouse, pTType->Receive_Resource);
			this->Source.Total = (double)ResourceValue(pSTechno, pHouse, pTType->Receive_Resource, false);
			this->Source.Resource = pTType->Receive_Resource;
			for (auto pTechno : pTechnoList)
			{
				this->Targets.push_back(TransferUnit(pTechno, pTType->Send_Resource));
			}
		}
		else
		{
			this->Source.Current = (double)ResourceValue(pSTechno, pHouse, pTType->Send_Resource);
			this->Source.Total = (double)ResourceValue(pSTechno, pHouse, pTType->Send_Resource, false);
			this->Source.Resource = pTType->Send_Resource;
			for (auto pTechno : pTechnoList)
			{
				this->Targets.push_back(TransferUnit(pTechno, pTType->Receive_Resource));
			}
		}

		this->SourceWarhead = pWarhead;
		this->DetonationCoords = coords;
	}

	TransferDetails() = delete;

private:
	enum LogType
	{
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
			Debug::Log("TransferType: %s\n", Options->Name.data());
		if (ltype & LogType::War)
			Debug::Log("WarheadID: %s\n", SourceWarhead->ID);
		if (ltype & LogType::Src)
		{
			if (Source.Techno == nullptr)
				Debug::Log("HouseID: %s\n", Source.House->Type->ID);
			else
			{
				Debug::Log("SourceID: %s Value: %.1f x %.3f Current: %.1f/%.1f\n", Source.Techno->GetTechnoType()->ID,
					Source.Value, Source.Modifier, Source.Current, Source.Total);
			}
		}
		if (ltype & LogType::Trg)
		{
			if (pTUnit == nullptr)
			{
				if (Targets.size() == 0)
					return;
				for (auto Target : Targets)
				{
					i++;
					if (Target.Techno != nullptr)
					{
						Debug::Log("%d - TargetID: %s Value: %.1f x %.3f Current: %.1f/%.1f\n", i, Target.Techno->GetTechnoType()->ID,
							Target.Value, Target.Modifier, Target.Current, Target.Total);
					}
				}
			}
			else
			{
				Debug::Log("TargetID: %s Value: %.1f x %.3f Current: %.1f/%.1f\n", pTUnit->Techno->GetTechnoType()->ID,
					pTUnit->Value, pTUnit->Modifier, pTUnit->Current, pTUnit->Total);
			}
		}
	}

	int AddValue(TransferUnit* pValues)
	{
		if (pValues->Value == 0)
			return 0;
		TechnoClass* pTechno = pValues->Techno;
		TechnoTypeClass* pType = nullptr;

		if (pTechno == nullptr && pValues->Resource != TransferTypeResource::Money)
			return 0;
		if (pTechno != nullptr)
			pType = pTechno->GetTechnoType();

		int value = (int)(pValues->Value + 0.5);
		switch (pValues->Resource)
		{
		case TransferTypeResource::Experience:
			if (pTechno->Veterancy.IsElite() || !pType->Trainable)
				return 0;
			else if (pValues->Current + value >= pValues->Total)
			{
				pTechno->Veterancy.SetElite();
				return (int)(pValues->Total - pValues->Current);
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
			pTechno->Flashing.DurationRemaining += 10;
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
			if (pTechno->GattlingValue + value >= pValues->Total && pType)
			{
				pTechno->GattlingValue = (int)pValues->Total;
				pTechno->CurrentGattlingStage = pType->WeaponStages - 1;
				return pType->WeaponStages - 1 - CurrentStage;
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
					if (pTechno->GattlingValue >= pType->EliteStage[Stage - 1]
						&& pTechno->GattlingValue < pType->EliteStage[Stage] && Stage < CurrentStage)
					{
						pTechno->CurrentGattlingStage = Stage;
						return Stage - CurrentStage;
					}
				}
				else
				{
					if (pTechno->GattlingValue >= pType->WeaponStage[Stage - 1]
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

		if (pTechno == nullptr && pValues->Resource != TransferTypeResource::Money)
			return 0;
		if (pTechno != nullptr)
			pType = pTechno->GetTechnoType();

		int value = (int)(-pValues->Value + 0.5);
		switch (pValues->Resource)
		{
		case TransferTypeResource::Experience:
			if (pTechno->Veterancy.Veterancy <= 0.0f || !pType->Trainable)
				return 0;

			if (!Options->Decrease_Experience_AllowDemote)
			{
				if (pTechno->Veterancy.IsElite() || pTechno->Veterancy.Veterancy == 1.0f)
					return 0;
				else if (pTechno->Veterancy.IsVeteran() && pValues->Current - value < pValues->Total / 2)
					value = (int)(pValues->Total / 2.0 - pValues->Current);
			}
			if ((double)value > pValues->Current)
			{
				pTechno->Veterancy.Reset();
				return (int)(pValues->Current);
			}
			pTechno->Veterancy.Veterancy = (float)((pValues->Current - (double)value) / pValues->Total * 2);
			return value;
		case TransferTypeResource::Money:
			if (pValues->House->Available_Money() < value)
				value = pValues->House->Available_Money();
			pValues->House->TakeMoney(value);
			return value;
		case TransferTypeResource::Health:
			if (!Options->Decrease_Health_AllowKill && value > pTechno->Health)
				value = pTechno->Health - 1;
			pTechno->ReceiveDamage(&value, 0, SourceWarhead, Source.Techno, true, false, Source.House);
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
					if (pTechno->GattlingValue >= pType->EliteStage[Stage - 1]
						&& pTechno->GattlingValue < pType->EliteStage[Stage] && Stage < CurrentStage)
					{
						pTechno->CurrentGattlingStage = Stage;
						return CurrentStage - Stage;
					}
				}
				else
				{
					if (pTechno->GattlingValue >= pType->WeaponStage[Stage - 1]
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

	void AssessTargets()
	{
		WarheadTypeClass* TargetWarhead = Options->Target_Warhead;
		if (TargetWarhead == nullptr)
			TargetWarhead = SourceWarhead;

		for (auto Target = Targets.begin(); Target != Targets.end(); Target++)
		{
			bool IsHouseAffected = Options->Target_AffectHouses == AffectedHouse::All
				|| EnumFunctions::CanTargetHouse(Options->Target_AffectHouses, Source.House, Target->House);

			if (!IsHouseAffected || (SourceWarhead->CellSpread > 0
				&& ((Options->Target_Spread_IgnoreSelf && Target->Techno == Source.Techno)
				|| (Options->Target_Spread_Distribution == SpreadDistribution::None && Target->Techno != BulletTargetTechno))))
			{
				Target->Modifier = 0.0f;
				if (!Options->Target_Spread_CountUnaffected)
					Targets.erase(Target--);
				continue;
			}

			if (Options->Target_ConsiderArmor)
			{
				Target->Modifier *= GeneralUtils::GetWarheadVersusArmor(TargetWarhead
					, Target->Techno->GetTechnoType()->Armor);
			}

			if (Options->Target_Spread_Distribution == SpreadDistribution::Distance)
			{
				Target->Modifier *= 1.0f - (1.0f - SourceWarhead->PercentAtMax)
					* (DetonationCoords.DistanceFrom(Target->Techno->GetCoords())
					/ Unsorted::LeptonsPerCell / SourceWarhead->CellSpread);
			}

			if (!Options->Target_Spread_CountUnaffected && Target->Modifier == 0.0)
			{
				Targets.erase(Target--);
				continue;
			}
		}
	}

	void InitializeValues()
	{
		if (Options->TargetToSource)
		{
			Source.Value = Options->Receive_Value;

			if (Options->Receive_Value_Type != TechnoValueType::Fixed)
			{
				switch (Options->Receive_Value_Type)
				{
				case TechnoValueType::Current:
					Source.Value *= Source.Current;
					break;
				case TechnoValueType::Missing:
					if (Options->Receive_Resource == TransferTypeResource::Money)
					{
						if (Source.Techno)
							Source.Value *= Source.Techno->GetTechnoType()->Soylent;
						else
							Source.Value = 0;
					}
					else
						Source.Value *= Source.Total - Source.Current;
					break;
				case TechnoValueType::Total:
					Source.Value *= Source.Total;
					break;
				}
			}

			if (Source.Techno && Source.Techno->Veterancy.Veterancy >= 1.0f)
			{
				Source.Value *= Options->Receive_Value_SourceVeterancyMultiplier.Get().X;
				if (Source.Techno->Veterancy.Veterancy >= 2.0f)
					Source.Value *= Options->Receive_Value_SourceVeterancyMultiplier.Get().Y;
			}

			for (auto Target = Targets.begin(); Target != Targets.end(); Target++)
			{
				Target->Value = Options->Send_Value;

				if (Options->Send_Value_Type != TechnoValueType::Fixed)
				{
					switch (Options->Send_Value_Type)
					{
					case TechnoValueType::Current:
						Target->Value *= Target->Current;
						break;
					case TechnoValueType::Missing:
						if (Options->Send_Resource == TransferTypeResource::Money)
							Target->Value *= Target->Techno->GetTechnoType()->Soylent;
						else
							Target->Value *= Target->Total - Target->Current;
						break;
					case TechnoValueType::Total:
						Target->Value *= Target->Total;
						break;
					}
				}

				if (Target->Techno->Veterancy.Veterancy >= 1.0f)
				{
					Target->Value *= Options->Send_Value_SourceVeterancyMultiplier.Get().X;
					if (Target->Techno->Veterancy.Veterancy >= 2.0f)
						Target->Value *= Options->Send_Value_SourceVeterancyMultiplier.Get().Y;
				}

				if (Options->Target_Spread_Distribution == SpreadDistribution::Split)
					Target->Value /= (double)Targets.size();

				Target->Value *= Target->Modifier;
			}
		}
		else
		{
			Source.Value = Options->Send_Value;

			if (Options->Send_Value_Type != TechnoValueType::Fixed)
			{
				switch (Options->Send_Value_Type)
				{
				case TechnoValueType::Current:
					Source.Value *= Source.Current;
					break;
				case TechnoValueType::Missing:
					if (Options->Receive_Resource == TransferTypeResource::Money)
					{
						if (Source.Techno)
							Source.Value *= Source.Techno->GetTechnoType()->Soylent;
						else
							Source.Value = 0;
					}
					else
						Source.Value *= Source.Total - Source.Current;
					break;
				case TechnoValueType::Total:
					Source.Value *= Source.Total;
					break;
				}
			}

			if (Source.Techno && Source.Techno->Veterancy.Veterancy >= 1.0f)
			{
				Source.Value *= Options->Send_Value_SourceVeterancyMultiplier.Get().X;
				if (Source.Techno->Veterancy.Veterancy >= 2.0f)
					Source.Value *= Options->Send_Value_SourceVeterancyMultiplier.Get().Y;
			}

			for (auto Target = Targets.begin(); Target != Targets.end(); Target++)
			{
				Target->Value = Options->Receive_Value;

				if (Options->Receive_Value_Type != TechnoValueType::Fixed)
				{
					switch (Options->Receive_Value_Type)
					{
					case TechnoValueType::Current:
						Target->Value *= Target->Current;
						break;
					case TechnoValueType::Missing:
						if (Options->Receive_Resource == TransferTypeResource::Money)
							Target->Value *= Target->Techno->GetTechnoType()->Soylent;
						else
							Target->Value *= Target->Total - Target->Current;
						break;
					case TechnoValueType::Total:
						Target->Value *= Target->Total;
						break;
					}
				}

				if (Target->Techno->Veterancy.Veterancy >= 1.0f)
				{
					Target->Value *= Options->Receive_Value_SourceVeterancyMultiplier.Get().X;
					if (Target->Techno->Veterancy.Veterancy >= 2.0f)
						Target->Value *= Options->Receive_Value_SourceVeterancyMultiplier.Get().Y;
				}

				if (Options->Target_Spread_Distribution == SpreadDistribution::Split)
					Target->Value /= (double)Targets.size();

				Target->Value *= Target->Modifier;
			}
		}
	}

	void FinalizeCalculations()
	{
		if (Options->TargetToSource)
		{
			double Highest = 0.0;
			double PositiveSum = 0.0;
			double NegativeSum = 0.0;

			for (auto Target = Targets.begin(); Target != Targets.end(); Target++)
			{
				if (Target->Modifier == 0)
					continue;

				if (Target->Modifier > 0)
				{
					if (Options->Send_Value_FlatLimits.Get().X && Target->Value < Options->Send_Value_FlatLimits.Get().X)
						Target->Value = Options->Send_Value_FlatLimits.Get().X;
					if (Options->Send_Value_FlatLimits.Get().Y && Target->Value > Options->Send_Value_FlatLimits.Get().Y)
						Target->Value = Options->Send_Value_FlatLimits.Get().Y;
				}
				else
				{
					if (Options->Send_Value_FlatLimits.Get().Y && Target->Value < -Options->Send_Value_FlatLimits.Get().Y)
						Target->Value = -Options->Send_Value_FlatLimits.Get().Y;
					if (Options->Send_Value_FlatLimits.Get().X && Target->Value > -Options->Send_Value_FlatLimits.Get().X)
						Target->Value = -Options->Send_Value_FlatLimits.Get().X;
				}

				if (Options->Send_PreventOverflow && Options->Send_Resource != TransferTypeResource::Money
					&& Target->Value + Target->Current > Target->Total)
				{
					Target->Value = Target->Total - Target->Current;
				}

				if (Options->Send_PreventUnderflow)
				{
					if (Options->Send_Resource == TransferTypeResource::Experience && !Options->Decrease_Experience_AllowDemote)
					{
						if (Target->Techno->Veterancy.IsElite() || Target->Techno->Veterancy.Veterancy == 1.0f)
							Target->Value = 0;
						else if (Target->Techno->Veterancy.IsVeteran() && ((Target->Current + Target->Value) < (Target->Total / 2)))
							Target->Value = Target->Total / 2 - Target->Current;
						else if (Target->Current < -Target->Value)
							Target->Value = -Target->Current;
					}
					else if (Target->Current < -Target->Value)
					{
						Target->Value = -Target->Current;

						if (Options->Send_Resource == TransferTypeResource::Health && !Options->Decrease_Health_AllowKill)
							Target->Value += 1.0;
					}
				}

				if (Target->Value < 0)
				{
					NegativeSum += Target->Value;

					if (Options->Send_Value < 0 && Target->Value < Highest)
						Highest = Target->Value;
				}
				else
				{
					PositiveSum += Target->Value;

					if (Options->Send_Value > 0 && Target->Value > Highest)
						Highest = Target->Value;
				}
			}

			switch (Options->Receive_Multiplier)
			{
			case Multiplier::Highest:
				Source.Value *= Highest;
				break;
			case Multiplier::Sum:
				Source.Value *= PositiveSum + NegativeSum;
				break;
			case Multiplier::Tally:
				Source.Value *= (double)Targets.size();
				break;
			default:
				break;
			}

			if (Options->Receive_Value_FlatLimits.Get().X && Source.Value < Options->Receive_Value_FlatLimits.Get().X)
				Source.Value = Options->Receive_Value_FlatLimits.Get().X;
			if (Options->Receive_Value_FlatLimits.Get().Y && Source.Value > Options->Receive_Value_FlatLimits.Get().Y)
				Source.Value = Options->Receive_Value_FlatLimits.Get().Y;

			if (Options->Receive_Resource != TransferTypeResource::Money && Options->Receive_ReturnOverflow
				&& Source.Total - Source.Current < Source.Value && Options->Receive_Multiplier != Multiplier::None
				&& Options->Receive_Multiplier != Multiplier::Tally)
			{
				double Overflow = Source.Value - (Source.Total - Source.Current);

				if (-NegativeSum > PositiveSum)
				{
					Overflow /= -NegativeSum;
					for (auto Target = Targets.begin(); Target != Targets.end(); Target++)
					{
						if (Target->Modifier == 0)
							continue;

						if (Target->Value < 0)
							Target->Value -= Target->Value * Overflow;
					}
				}
				else
				{
					Overflow /= PositiveSum;
					for (auto Target = Targets.begin(); Target != Targets.end(); Target++)
					{
						if (Target->Modifier == 0)
							continue;

						if (Target->Value > 0)
							Target->Value -= Target->Value * Overflow;
					}
				}

				Source.Value = Source.Total - Source.Current;
			}
		}
		else
		{
			if (Options->Send_Value_FlatLimits.Get().X && Source.Value < Options->Send_Value_FlatLimits.Get().X)
				Source.Value = Options->Send_Value_FlatLimits.Get().X;
			if (Options->Send_Value_FlatLimits.Get().Y && Source.Value > Options->Send_Value_FlatLimits.Get().Y)
				Source.Value = Options->Send_Value_FlatLimits.Get().Y;

			if (Options->Send_PreventOverflow && Options->Send_Resource != TransferTypeResource::Money
				&& (Source.Value + Source.Current) > Source.Total)
			{
				Source.Value = Source.Total - Source.Current;
			}

			if (Options->Send_PreventUnderflow)
			{
				if (Source.Techno && Options->Send_Resource == TransferTypeResource::Experience && !Options->Decrease_Experience_AllowDemote)
				{
					if (Source.Techno->Veterancy.IsElite() || Source.Techno->Veterancy.Veterancy == 1.0f)
						Source.Value = 0;
					else if (Source.Techno->Veterancy.IsVeteran() && Source.Current + Source.Value < Source.Total / 2)
						Source.Value = Source.Total / 2 - Source.Current;
				}
				else if (Source.Current < -Source.Value)
				{
					Source.Value = -Source.Current;

					if (Options->Send_Resource == TransferTypeResource::Health && !Options->Decrease_Health_AllowKill)
						Source.Value += 1.0;
				}
			}

			double OverflowSum = 0.0;
			for (auto Target = Targets.begin(); Target != Targets.end(); Target++)
			{
				switch (Options->Receive_Multiplier)
				{
				case Multiplier::Highest:
				case Multiplier::Sum:
					Target->Value *= Source.Value;
					break;
				case Multiplier::Tally:
					Target->Value *= (double)Targets.size();
					break;
				default:
					break;
				}

				if (Options->Receive_Value_FlatLimits.Get().X && Target->Value < Options->Receive_Value_FlatLimits.Get().X)
					Target->Value = Options->Receive_Value_FlatLimits.Get().X;
				if (Options->Receive_Value_FlatLimits.Get().Y && Target->Value > Options->Receive_Value_FlatLimits.Get().Y)
					Target->Value = Options->Receive_Value_FlatLimits.Get().Y;

				double Overflow = Target->Value - (Target->Total - Target->Current);
				if (Options->Receive_Resource != TransferTypeResource::Money && Options->Receive_ReturnOverflow
					&& Overflow > 0 && Options->Receive_Multiplier != Multiplier::None
					&& Options->Receive_Multiplier != Multiplier::Tally)
				{
					OverflowSum += Overflow / Target->Value * Source.Value / (double)(Targets.size());
					Target->Value = Target->Total - Target->Current;
				}
			}

			Source.Value -= OverflowSum;
		}
	}

	void ApplyChanges()
	{
		int SourceChange = 0;
		if (Source.Value < 0)
			SourceChange = -SubValue(&Source);
		else if (Source.Value > 0)
			SourceChange = AddValue(&Source);

		for (auto Target = Targets.begin(); Target != Targets.end(); Target++)
		{
			int TargetChange = 0;
			if (Target->Value < 0)
				TargetChange = -SubValue(&(*Target));
			else if (Target->Value > 0)
				TargetChange = AddValue(&(*Target));
		}

		LogDetails(L"Transfer complete", LogType::All);
	}

public:
	void ExecuteTransfer()
	{
		AssessTargets();
		InitializeValues();
		FinalizeCalculations();
		ApplyChanges();
	}
};

void WarheadTypeExt::ExtData::TransferWithGroup(TechnoClass* pSourceTechno, HouseClass* pSourceHouse, TechnoClass* pTargetTechno, std::vector<TechnoClass*> pTargets, CoordStruct coords)
{
	for (auto transferType : this->Transfer_Types)
	{
		TransferDetails transfer(pSourceTechno, pSourceHouse, this->OwnerObject(), pTargetTechno, pTargets, transferType, coords);
		transfer.ExecuteTransfer();
	}
}

void WarheadTypeExt::ExtData::TransferWithUnit(TechnoClass* pSourceTechno, HouseClass* pSourceHouse, TechnoClass* pTargetTechno, CoordStruct coords)
{
	std::vector<TechnoClass*> Target = { pTargetTechno };
	for (auto transferType : this->Transfer_Types)
	{
		TransferDetails transfer(pSourceTechno, pSourceHouse, this->OwnerObject(), pTargetTechno, Target, transferType, coords);
		transfer.ExecuteTransfer();
	}
}
