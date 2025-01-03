#include "HandlerEffectClass.h"
#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Misc/FlyingStrings.h>

HandlerEffectClass::HandlerEffectClass()
	: Weapon {}
	, Weapon_Firer_Scope {}
	, Weapon_Firer_ExtScope {}
	, Weapon_SpawnProj { false }
	, Convert_Pairs {}
	, Soylent_Mult {}
	, Soylent_IncludePassengers { false }
	, Soylent_Scope {}
	, Soylent_ExtScope {}
	, Soylent_Display { true }
	, Soylent_Display_Houses { AffectedHouse::All }
	, Soylent_Display_Offset { { 0, 0 } }
	, Passengers_Eject { false }
	, Passengers_Kill { false }
	, Passengers_Kill_Score { false }
	, Passengers_Kill_Score_Scope {}
	, Passengers_Kill_Score_ExtScope {}
	, Passengers_Create_Types {}
	, Passengers_Create_Nums {}
	, Passengers_Create_Owner_Scope {}
	, Passengers_Create_Owner_ExtScope {}
	, Veterancy_Set {}
	, Veterancy_Add {}
	, Voice {}
	, Voice_Persist { false }
	, Voice_Global { false }
	, EVA {}
{ }

std::unique_ptr<HandlerEffectClass> HandlerEffectClass::Parse(INI_EX& exINI, const char* pSection, const char* scopeName, const char* effectName)
{
	auto effect = std::make_unique<HandlerEffectClass>();
	effect.get()->LoadFromINI(exINI, pSection, scopeName, effectName);
	if (effect.get()->IsDefined())
	{
		return effect;
	}
	else
	{
		effect.reset();
		return nullptr;
	}
}

void HandlerEffectClass::LoadFromINI(INI_EX& exINI, const char* pSection, const char* scopeName, const char* effectName)
{
	char tempBuffer[64];

	// Weapon Detonation
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Weapon", scopeName, effectName);
	Weapon.Read(exINI, pSection, tempBuffer);
	if (Weapon.isset())
	{
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Weapon.Firer.Scope", scopeName, effectName);
		Weapon_Firer_Scope.Read(exINI, pSection, tempBuffer);
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Weapon.Firer.ExtScope", scopeName, effectName);
		Weapon_Firer_ExtScope.Read(exINI, pSection, tempBuffer);
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Weapon.SpawnProj", scopeName, effectName);
		Weapon_SpawnProj.Read(exINI, pSection, tempBuffer);
	}

	// Type Conversion
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Convert", scopeName, effectName);
	TypeConvertGroup::Parse(this->Convert_Pairs, exINI, pSection, AffectedHouse::All, tempBuffer);

	// Soylent Bounty
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Soylent.Mult", scopeName, effectName);
	Soylent_Mult.Read(exINI, pSection, tempBuffer);
	if (Soylent_Mult.isset())
	{
		if (Soylent_Mult.Get() == 0)
		{
			Soylent_Mult.Reset();
		}
		else
		{
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Soylent.IncludePassengers", scopeName, effectName);
			Soylent_IncludePassengers.Read(exINI, pSection, tempBuffer);
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Soylent.Scope", scopeName, effectName);
			Soylent_Scope.Read(exINI, pSection, tempBuffer);
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Soylent.ExtScope", scopeName, effectName);
			Soylent_ExtScope.Read(exINI, pSection, tempBuffer);
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Soylent.Display", scopeName, effectName);
			Soylent_Display.Read(exINI, pSection, tempBuffer);
			if (Soylent_Display.Get())
			{
				_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Soylent.Display.Houses", scopeName, effectName);
				Soylent_Display_Houses.Read(exINI, pSection, tempBuffer);
				_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Soylent.Display.Offset", scopeName, effectName);
				Soylent_Display_Offset.Read(exINI, pSection, tempBuffer);
			}
		}
	}

	// Passenger Ejection
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Passengers.Eject", scopeName, effectName);
	Passengers_Eject.Read(exINI, pSection, tempBuffer);

	// Passenger Removal
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Passengers.Kill", scopeName, effectName);
	Passengers_Kill.Read(exINI, pSection, tempBuffer);
	if (Passengers_Kill.Get())
	{
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Passengers.Kill.Score", scopeName, effectName);
		Passengers_Kill_Score.Read(exINI, pSection, tempBuffer);
		if (Passengers_Kill_Score.Get())
		{
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Passengers.Kill.Score.Scope", scopeName, effectName);
			Passengers_Kill_Score_Scope.Read(exINI, pSection, tempBuffer);
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Passengers.Kill.Score.ExtScope", scopeName, effectName);
			Passengers_Kill_Score_ExtScope.Read(exINI, pSection, tempBuffer);
		}
	}

	// Passenger Creation
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Passengers.Create.Types", scopeName, effectName);
	Passengers_Create_Types.Read(exINI, pSection, tempBuffer);
	if (!Passengers_Create_Types.empty())
	{
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Passengers.Create.Nums", scopeName, effectName);
		Passengers_Create_Nums.Read(exINI, pSection, tempBuffer);
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Passengers.Create.Owner.Scope", scopeName, effectName);
		Passengers_Create_Owner_Scope.Read(exINI, pSection, tempBuffer);
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Passengers.Create.Owner.ExtScope", scopeName, effectName);
		Passengers_Create_Owner_ExtScope.Read(exINI, pSection, tempBuffer);
	}

	// Veterancy
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Veterancy.Set", scopeName, effectName);
	Veterancy_Set.Read(exINI, pSection, tempBuffer);
	if (Veterancy_Set.isset() && Veterancy_Set.Get() != VeterancyType::Rookie
		&& Veterancy_Set.Get() != VeterancyType::Veteran
		&& Veterancy_Set.Get() != VeterancyType::Elite)
	{
		Veterancy_Set.Reset();
	}
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Veterancy.Add", scopeName, effectName);
	Veterancy_Add.Read(exINI, pSection, tempBuffer);
	if (Veterancy_Add.isset() && Veterancy_Add.Get() == 0)
	{
		Veterancy_Add.Reset();
	}

	// Voice
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Voice", scopeName, effectName);
	Voice.Read(exINI, pSection, tempBuffer);
	if (Voice.isset())
	{
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Voice.Persist", scopeName, effectName);
		Voice_Persist.Read(exINI, pSection, tempBuffer);
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Voice.Global", scopeName, effectName);
		Voice_Global.Read(exINI, pSection, tempBuffer);
	}
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.EVA", scopeName, effectName);
	EVA.Read(exINI, pSection, tempBuffer);
}

void HandlerEffectClass::Execute(std::map<EventScopeType, TechnoClass*>* pParticipants, TechnoClass* pOwner, TechnoClass* pTarget) const
{
	// Weapon Detonation
	if (Weapon.isset())
	{
		auto pWeapon = Weapon.Get();
		TechnoClass* pFirer = pOwner;
		if (Weapon_Firer_Scope.isset())
		{
			pFirer = HandlerCompClass::GetTrueTarget(pParticipants->at(Weapon_Firer_Scope.Get()), Weapon_Firer_ExtScope);
		}
		if (pFirer)
		{
			if (Weapon_SpawnProj.Get())
			{
				if (BulletClass* pBullet = pWeapon->Projectile->CreateBullet(pTarget, pFirer,
					pWeapon->Damage, pWeapon->Warhead, pWeapon->Speed, pWeapon->Bright))
				{
					pBullet->WeaponType = pWeapon;
					pBullet->MoveTo(pTarget->Location, BulletVelocity::Empty);
					BulletExt::ExtMap.Find(pBullet)->FirerHouse = pFirer->Owner;
				}
			}
			else
			{
				WeaponTypeExt::DetonateAt(pWeapon, pTarget->GetCoords(), pFirer, pWeapon->Damage, pFirer->GetOwningHouse());
			}
		}
	}

	// Type Conversion
	if (!Convert_Pairs.empty())
	{
		TypeConvertGroup::Convert(static_cast<FootClass*>(pTarget), this->Convert_Pairs, pOwner->Owner);
	}

	// Soylent Bounty
	if (Soylent_Mult.isset())
	{
		auto pReceptant = pOwner;
		if (Soylent_Scope.isset())
		{
			pReceptant = HandlerCompClass::GetTrueTarget(pParticipants->at(Soylent_Scope.Get()), Soylent_ExtScope);
		}

		// if we don't have a proper receptant then don't bother to do at all
		if (pReceptant)
		{
			auto multiplier = Soylent_Mult.Get();
			int nMoneyToGive = (int)(pTarget->GetTechnoType()->GetRefund(pTarget->Owner, true) * multiplier);
			if (Soylent_IncludePassengers.Get() && pTarget->Passengers.NumPassengers > 0)
			{
				nMoneyToGive += TechnoTypeExt::GetTotalSoylentOfPassengers(multiplier, pTarget);
			}

			if (nMoneyToGive != 0)
			{
				pReceptant->Owner->TransactMoney(nMoneyToGive);
				if (Soylent_Display.Get())
				{
					FlyingStrings::AddMoneyString(nMoneyToGive, pReceptant->Owner,
						Soylent_Display_Houses, pReceptant->Location, Soylent_Display_Offset);
				}
			}
		}
	}

	// Passenger Ejection
	if (Passengers_Eject.Get())
	{
		if (pTarget->Passengers.NumPassengers > 0)
		{
			auto const openTopped = pTarget->GetTechnoType()->OpenTopped;

			while (pTarget->Passengers.FirstPassenger)
			{
				FootClass* pPassenger = pTarget->Passengers.RemoveFirstPassenger();

				auto const coords = pTarget->GetCoords();
				auto const pCell = MapClass::Instance->GetCellAt(coords);
				auto const isBridge = pCell->ContainsBridge();
				auto const nCell = MapClass::Instance->NearByLocation(pCell->MapCoords,
					SpeedType::Wheel, -1, MovementZone::Normal, isBridge, 1, 1, true,
					false, false, isBridge, CellStruct::Empty, false, false);

				pPassenger->Unlimbo(MapClass::Instance->TryGetCellAt(nCell)->GetCoords(), static_cast<DirType>(32 * ScenarioClass::Instance->Random.RandomRanged(0, 7)));

				if (openTopped)
				{
					pTarget->ExitedOpenTopped(pPassenger);
				}
			}
		}
	}

	// Passenger Removal
	if (Passengers_Kill.Get())
	{
		TechnoClass* pScorer = nullptr;
		if (Passengers_Kill_Score.Get())
		{
			pScorer = pOwner;
			if (Passengers_Kill_Score_Scope.isset())
			{
				pScorer = HandlerCompClass::GetTrueTarget(pParticipants->at(Passengers_Kill_Score_Scope.Get()), Passengers_Kill_Score_ExtScope);
			}
		}

		pTarget->KillPassengers(pScorer);
	}

	// Passenger Creation
	if (!Passengers_Create_Types.empty())
	{
		TechnoClass* pPassengerOwnerScope = pTarget;
		if (Passengers_Kill_Score.Get())
		{
			pPassengerOwnerScope = pOwner;
			if (Passengers_Kill_Score_Scope.isset())
			{
				pPassengerOwnerScope = HandlerCompClass::GetTrueTarget(pParticipants->at(Passengers_Kill_Score_Scope.Get()), Passengers_Kill_Score_ExtScope);
			}
		}

		if (pPassengerOwnerScope)
		{
			this->CreatePassengers(pTarget, pPassengerOwnerScope);
		}
	}

	// Veterancy
	if (pTarget->GetTechnoType()->Trainable)
	{
		if (Veterancy_Set.isset())
		{
			switch (Veterancy_Set.Get())
			{
			case VeterancyType::Rookie:
				pTarget->Veterancy.Reset();
				break;
			case VeterancyType::Veteran:
				pTarget->Veterancy.SetVeteran();
				break;
			case VeterancyType::Elite:
				pTarget->Veterancy.SetElite();
				break;
			}
		}

		if (Veterancy_Add.isset())
		{
			pTarget->Veterancy.Add(Veterancy_Add.Get());
		}
	}

	// Voice
	if (Voice.isset())
	{
		if (Voice_Global.Get())
		{
			VocClass::PlayAt(Voice.Get(), pTarget->Location);
		}
		else if (pTarget->Owner->IsControlledByCurrentPlayer())
		{
			if (Voice_Persist.Get())
				VocClass::PlayAt(Voice.Get(), pTarget->Location);
			else
				pTarget->QueueVoice(Voice.Get());
		}
	}

	// EVA
	if (EVA.isset())
	{
		if (pTarget->Owner->IsControlledByCurrentPlayer())
		{
			VoxClass::PlayIndex(EVA.Get());
		}
	}
}

// Basically copied from Ares "TechnoExt::ExtData::CreateInitialPayload()".
void HandlerEffectClass::CreatePassengers(TechnoClass* pToWhom, TechnoClass* pPassengerOwnerScope) const
{
	auto const pType = pToWhom->GetTechnoType();

	auto const pBld = abstract_cast<BuildingClass*>(pToWhom);
	auto const pBldType = pBld ? pBld->Type : nullptr;

	auto freeSlots = (pBld && pBldType->CanBeOccupied)
		? pBldType->MaxNumberOccupants - pBld->GetOccupantCount()
		: pType->Passengers - pToWhom->Passengers.NumPassengers;

	auto const sizePayloadNum = this->Passengers_Create_Types.size();

	for (auto i = 0u; i < Passengers_Create_Types.size(); ++i)
	{
		auto const pPayloadType = Passengers_Create_Types[i];

		if (!pPayloadType)
		{
			continue;
		}

		// buildings and aircraft aren't valid payload, and building payload
		// can only be infantry
		auto const absPayload = pPayloadType->WhatAmI();
		if (absPayload == AbstractType::BuildingType
			|| absPayload == AbstractType::AircraftType
			|| (pBld && absPayload != AbstractType::InfantryType))
		{
			continue;
		}

		// if there are no nums, index gets huge and invalid, which means 1
		auto const idxPayloadNum = Math::min(i + 1, sizePayloadNum) - 1;
		auto const payloadNum = (idxPayloadNum < sizePayloadNum)
			? Passengers_Create_Nums[idxPayloadNum] : 1;

		// never fill in more than allowed
		auto const count = Math::min(payloadNum, freeSlots);
		freeSlots -= count;

		for (auto j = 0; j < count; ++j)
		{
			auto const pObject = pPayloadType->CreateObject(pPassengerOwnerScope->Owner);

			if (pBld)
			{
				// buildings only allow infantry payload, so this in infantry
				auto const pPayload = static_cast<InfantryClass*>(pObject);

				if (pBldType->CanBeOccupied)
				{
					pBld->Occupants.AddItem(pPayload);

					auto const pCell = pToWhom->GetCell();
					pToWhom->UpdateThreatInCell(pCell);
				}
				else
				{
					pPayload->Limbo();

					if (pBldType->InfantryAbsorb)
					{
						pPayload->Absorbed = true;

						if (pPayload->CountedAsOwnedSpecial)
						{
							--pPayload->Owner->OwnedInfantry;
							pPayload->CountedAsOwnedSpecial = false;
						}

						if (pBldType->ExtraPowerBonus > 0)
						{
							pBld->Owner->RecheckPower = true;
						}
					}
					else
					{
						pPayload->SendCommand(RadioCommand::RequestLink, pBld);
					}

					pBld->Passengers.AddPassenger(pPayload);
					pPayload->AbortMotion();
				}

			}
			else
			{
				auto const pPayload = static_cast<FootClass*>(pObject);
				pPayload->SetLocation(pToWhom->Location);
				pPayload->Limbo();

				if (pType->OpenTopped)
				{
					pToWhom->EnteredOpenTopped(pPayload);
				}

				pPayload->Transporter = pToWhom;

				auto const old = std::exchange(VocClass::VoicesEnabled(), false);
				pToWhom->AddPassenger(pPayload);
				VocClass::VoicesEnabled = old;
			}
		}
	}
}

bool HandlerEffectClass::IsDefined() const
{
	return Weapon.isset()
		|| !Convert_Pairs.empty()
		|| Soylent_Mult.isset()
		|| Passengers_Eject.Get()
		|| Passengers_Kill.Get()
		|| !Passengers_Create_Types.empty()
		|| Veterancy_Set.isset()
		|| Veterancy_Add.isset()
		|| Voice.isset()
		|| EVA.isset();
}

bool HandlerEffectClass::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return this->Serialize(stm);
}

bool HandlerEffectClass::Save(PhobosStreamWriter& stm) const
{
	return const_cast<HandlerEffectClass*>(this)->Serialize(stm);
}

template<typename T>
bool HandlerEffectClass::Serialize(T& stm)
{
	return stm
		.Process(this->Weapon)
		.Process(this->Weapon_Firer_Scope)
		.Process(this->Weapon_Firer_ExtScope)
		.Process(this->Weapon_SpawnProj)
		.Process(this->Convert_Pairs)
		.Process(this->Soylent_Mult)
		.Process(this->Soylent_IncludePassengers)
		.Process(this->Soylent_Scope)
		.Process(this->Soylent_ExtScope)
		.Process(this->Soylent_Display)
		.Process(this->Soylent_Display_Houses)
		.Process(this->Soylent_Display_Offset)
		.Process(this->Passengers_Eject)
		.Process(this->Passengers_Kill)
		.Process(this->Passengers_Kill_Score)
		.Process(this->Passengers_Kill_Score_Scope)
		.Process(this->Passengers_Kill_Score_ExtScope)
		.Process(this->Passengers_Create_Types)
		.Process(this->Passengers_Create_Nums)
		.Process(this->Passengers_Create_Owner_Scope
		.Process(this->Passengers_Create_Owner_ExtScope)
		.Process(this->Veterancy_Set)
		.Process(this->Veterancy_Add)
		.Process(this->Voice)
		.Process(this->Voice_Persist)
		.Process(this->Voice_Global)
		.Process(this->EVA)
		.Success();
}
