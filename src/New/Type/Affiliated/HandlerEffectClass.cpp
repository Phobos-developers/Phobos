#include "HandlerEffectClass.h"
#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Misc/FlyingStrings.h>
#include <Utilities/Helpers.Alex.h>
#include <AircraftClass.h>

#include <Ext/House/Body.h>
HandlerEffectClass::HandlerEffectClass()
	: HasAnyTechnoEffect { false }

#pragma region TechnoEffects
	, Weapon { }
	, Weapon_Firer {}
	, Weapon_FirerExt {}
	, Weapon_SpawnProj { false }
	, Convert_Pairs {}
	, Soylent_Mult {}
	, Soylent_IncludePassengers { false }
	, Soylent_Receptant {}
	, Soylent_ReceptantExt {}
	, Soylent_Display { true }
	, Soylent_Display_Houses { AffectedHouse::All }
	, Soylent_Display_Offset { { 0, 0 } }
	, Passengers_Eject { false }
	, Passengers_Kill { false }
	, Passengers_Kill_Score { false }
	, Passengers_Kill_Scorer {}
	, Passengers_Kill_ScorerExt {}
	, Passengers_Create_Types {}
	, Passengers_Create_Nums {}
	, Passengers_Create_Owner {}
	, Passengers_Create_OwnerExt {}
	, Veterancy_Set {}
	, Veterancy_Add {}
	, Voice {}
	, Voice_Persist { false }
	, Voice_Global { false }
	, Command {}
	, Command_Target {}
	, Command_TargetExt {}
#pragma endregion

#pragma region HouseEffects
	, HasAnyHouseEffect { false }
	, EVA {}
	, PlayerEmblem_AttachTypes {}
	, PlayerEmblem_RemoveTypes {}
#pragma endregion

#pragma region GenericEffects
	, HasAnyGenericEffect { false }
	, EventInvokers {}
	, Scope_Radius {}
	, Scope_MapWide { false }
	, Scope_Abstract {}
	, Scope_House {}
	, Scope_AirIncluded { false }
	, Scope_TechnoTypes {}
	, Scope_EventInvokers {}
#pragma endregion
{ }

std::unique_ptr<HandlerEffectClass> HandlerEffectClass::Parse(INI_EX& exINI, const char* pSection, const char* actorName, const char* effectName)
{
	auto effect = std::make_unique<HandlerEffectClass>();
	effect.get()->LoadFromINI(exINI, pSection, actorName, effectName);
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

void HandlerEffectClass::LoadFromINI(INI_EX& exINI, const char* pSection, const char* actorName, const char* effectName)
{
	char tempBuffer[64];

#pragma region TechnoEffects
	// Weapon Detonation
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Weapon", actorName, effectName);
	Weapon.Read(exINI, pSection, tempBuffer);
	if (Weapon.isset())
	{
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Weapon.Firer", actorName, effectName);
		Weapon_Firer.Read(exINI, pSection, tempBuffer);
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Weapon.FirerExt", actorName, effectName);
		Weapon_FirerExt.Read(exINI, pSection, tempBuffer);
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Weapon.SpawnProj", actorName, effectName);
		Weapon_SpawnProj.Read(exINI, pSection, tempBuffer);
	}

	// Type Conversion
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Convert", actorName, effectName);
	TypeConvertGroup::Parse(this->Convert_Pairs, exINI, pSection, AffectedHouse::All, tempBuffer);

	// Soylent Bounty
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Soylent.Mult", actorName, effectName);
	Soylent_Mult.Read(exINI, pSection, tempBuffer);
	if (Soylent_Mult.isset())
	{
		if (Soylent_Mult.Get() == 0)
		{
			Soylent_Mult.Reset();
		}
		else
		{
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Soylent.IncludePassengers", actorName, effectName);
			Soylent_IncludePassengers.Read(exINI, pSection, tempBuffer);
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Soylent.Receptant", actorName, effectName);
			Soylent_Receptant.Read(exINI, pSection, tempBuffer);
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Soylent.ReceptantExt", actorName, effectName);
			Soylent_ReceptantExt.Read(exINI, pSection, tempBuffer);
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Soylent.Display", actorName, effectName);
			Soylent_Display.Read(exINI, pSection, tempBuffer);
			if (Soylent_Display.Get())
			{
				_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Soylent.Display.Houses", actorName, effectName);
				Soylent_Display_Houses.Read(exINI, pSection, tempBuffer);
				_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Soylent.Display.Offset", actorName, effectName);
				Soylent_Display_Offset.Read(exINI, pSection, tempBuffer);
			}
		}
	}

	// Passenger Ejection
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Passengers.Eject", actorName, effectName);
	Passengers_Eject.Read(exINI, pSection, tempBuffer);

	// Passenger Removal
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Passengers.Kill", actorName, effectName);
	Passengers_Kill.Read(exINI, pSection, tempBuffer);
	if (Passengers_Kill.Get())
	{
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Passengers.Kill.Score", actorName, effectName);
		Passengers_Kill_Score.Read(exINI, pSection, tempBuffer);
		if (Passengers_Kill_Score.Get())
		{
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Passengers.Kill.Scorer", actorName, effectName);
			Passengers_Kill_Scorer.Read(exINI, pSection, tempBuffer);
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Passengers.Kill.ScorerExt", actorName, effectName);
			Passengers_Kill_ScorerExt.Read(exINI, pSection, tempBuffer);
		}
	}

	// Passenger Creation
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Passengers.Create.Types", actorName, effectName);
	Passengers_Create_Types.Read(exINI, pSection, tempBuffer);
	if (!Passengers_Create_Types.empty())
	{
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Passengers.Create.Nums", actorName, effectName);
		Passengers_Create_Nums.Read(exINI, pSection, tempBuffer);
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Passengers.Create.Owner", actorName, effectName);
		Passengers_Create_Owner.Read(exINI, pSection, tempBuffer);
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Passengers.Create.OwnerExt", actorName, effectName);
		Passengers_Create_OwnerExt.Read(exINI, pSection, tempBuffer);
	}

	// Veterancy
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Veterancy.Set", actorName, effectName);
	Veterancy_Set.Read(exINI, pSection, tempBuffer);
	if (Veterancy_Set.isset() && Veterancy_Set.Get() != VeterancyType::Rookie
		&& Veterancy_Set.Get() != VeterancyType::Veteran
		&& Veterancy_Set.Get() != VeterancyType::Elite)
	{
		Veterancy_Set.Reset();
	}
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Veterancy.Add", actorName, effectName);
	Veterancy_Add.Read(exINI, pSection, tempBuffer);
	if (Veterancy_Add.isset() && Veterancy_Add.Get() == 0)
	{
		Veterancy_Add.Reset();
	}

	// Voice
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Voice", actorName, effectName);
	Voice.Read(exINI, pSection, tempBuffer);
	if (Voice.isset())
	{
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Voice.Persist", actorName, effectName);
		Voice_Persist.Read(exINI, pSection, tempBuffer);
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Voice.Global", actorName, effectName);
		Voice_Global.Read(exINI, pSection, tempBuffer);
	}

	// Transfer to House
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Transfer.To.House", actorName, effectName);
	Transfer_To_House.Read(exINI, pSection, tempBuffer);
	if (Transfer_To_House.isset())
	{
		switch (Transfer_To_House.Get())
		{
		case OwnerHouseKind::Civilian:
		case OwnerHouseKind::Neutral:
		case OwnerHouseKind::Special:
			break;
		default:
			Transfer_To_House.Reset();
		}
	}

	// Transfer to Actor
	if (!Transfer_To_House.isset())
	{
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Transfer.To.Actor", actorName, effectName);
		Transfer_To_Actor.Read(exINI, pSection, tempBuffer);
		if (Transfer_To_Actor.isset())
		{
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Transfer.To.ActorExt", actorName, effectName);
			Transfer_To_ActorExt.Read(exINI, pSection, tempBuffer);
		}
	}

	// Command
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Command", actorName, effectName);
	Command.Read(exINI, pSection, tempBuffer);
	if (Command.isset())
	{
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Command.Target", actorName, effectName);
		Command_Target.Read(exINI, pSection, tempBuffer);
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Command.TargetExt", actorName, effectName);
		Command_TargetExt.Read(exINI, pSection, tempBuffer);
	}

	// defined flag
	HasAnyTechnoEffect = IsDefinedAnyTechnoEffect();
#pragma endregion

#pragma region HouseEffects
	// EVA
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.EVA", actorName, effectName);
	EVA.Read(exINI, pSection, tempBuffer);

	// Player Emblems
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.PlayerEmblem.AttachTypes", actorName, effectName);
	PlayerEmblem_AttachTypes.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.PlayerEmblem.RemoveTypes", actorName, effectName);
	PlayerEmblem_RemoveTypes.Read(exINI, pSection, tempBuffer);

	// defined flag
	HasAnyHouseEffect = IsDefinedAnyHouseEffect();
#pragma endregion

#pragma region GenericEffects
	// Event Invoker
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.EventInvoker", actorName, effectName);
	EventInvokerTypeClass::LoadTypeListFromINI(exINI, pSection, tempBuffer, &this->EventInvokers);

	// Area search
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Scope.Radius", actorName, effectName);
	Scope_Radius.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Scope.MapWide", actorName, effectName);
	Scope_MapWide.Read(exINI, pSection, tempBuffer);

	if (Scope_MapWide.Get() || Scope_Radius.isset())
	{
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Scope.Abstract", actorName, effectName);
		Scope_Abstract.Read(exINI, pSection, tempBuffer);
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Scope.House", actorName, effectName);
		Scope_House.Read(exINI, pSection, tempBuffer);
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Scope.AirIncluded", actorName, effectName);
		Scope_AirIncluded.Read(exINI, pSection, tempBuffer);
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Scope.TechnoTypes", actorName, effectName);
		Scope_TechnoTypes.Read(exINI, pSection, tempBuffer);

		// all filters must be specified
		if (Scope_Abstract.isset() && Scope_House.isset() && !Scope_TechnoTypes.empty())
		{
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Scope.EventInvoker", actorName, effectName);
			EventInvokerTypeClass::LoadTypeListFromINI(exINI, pSection, tempBuffer, &this->Scope_EventInvokers);
		}

		if (Scope_EventInvokers.empty())
		{
			Scope_Radius.Reset();
			Scope_MapWide = false;
			Scope_Abstract.Reset();
			Scope_House.Reset();
			Scope_AirIncluded = false;
			Scope_TechnoTypes.clear();
		}
	}

	// defined flag
	HasAnyGenericEffect = IsDefinedAnyGenericEffect();
#pragma endregion
}

void HandlerEffectClass::Execute(std::map<EventActorType, AbstractClass*>* pParticipants, AbstractClass* pTarget) const
{
	if (!pTarget)
		return;

	auto pOwner = pParticipants->at(EventActorType::Me);
	auto pOwnerHouse = HandlerCompClass::GetOwningHouseOfActor(pOwner);

	if (HasAnyTechnoEffect)
	{
		if (auto pTargetTechno = abstract_cast<TechnoClass*>(pTarget))
		{
			ExecuteForTechno(pOwner, pOwnerHouse, pParticipants, pTargetTechno);
		}
	}

	if (HasAnyHouseEffect)
	{
		auto pTargetHouse = HandlerCompClass::GetOwningHouseOfActor(pTarget);
		ExecuteForHouse(pOwner, pOwnerHouse, pParticipants, pTargetHouse);
	}

	if (HasAnyGenericEffect)
	{
		ExecuteGeneric(pOwner, pOwnerHouse, pParticipants, pTarget);
	}
}

void HandlerEffectClass::ExecuteForTechno(AbstractClass* pOwner, HouseClass* pOwnerHouse, std::map<EventActorType, AbstractClass*>* pParticipants, TechnoClass* pTarget) const
{
	// Weapon Detonation
	if (Weapon.isset())
	{
		auto pWeapon = Weapon.Get();
		AbstractClass* pFirer = pOwner;
		if (Weapon_Firer.isset())
		{
			pFirer = HandlerCompClass::GetTrueTarget(pParticipants->at(Weapon_Firer.Get()), Weapon_FirerExt);
		}
		if (pFirer)
		{
			auto pFirerTechno = abstract_cast<TechnoClass*>(pFirer);
			auto pFirerHouse = HandlerCompClass::GetOwningHouseOfActor(pFirer);
			if (pFirerTechno && Weapon_SpawnProj.Get())
			{
				if (BulletClass* pBullet = pWeapon->Projectile->CreateBullet(pTarget, pFirerTechno,
					pWeapon->Damage, pWeapon->Warhead, pWeapon->Speed, pWeapon->Bright))
				{
					pBullet->WeaponType = pWeapon;
					pBullet->MoveTo(pTarget->Location, BulletVelocity::Empty);
					BulletExt::ExtMap.Find(pBullet)->FirerHouse = pFirerHouse;
				}
			}
			else
			{
				WeaponTypeExt::DetonateAt(pWeapon, pTarget->GetCoords(), pFirerTechno, pWeapon->Damage, pFirerHouse);
			}
		}
	}

	// Type Conversion
	if (!Convert_Pairs.empty())
	{
		TypeConvertGroup::Convert(static_cast<FootClass*>(pTarget), this->Convert_Pairs, pOwnerHouse);
	}

	// Soylent Bounty
	if (Soylent_Mult.isset())
	{
		auto pReceptant = pOwner;
		if (Soylent_Receptant.isset())
		{
			pReceptant = HandlerCompClass::GetTrueTarget(pParticipants->at(Soylent_Receptant.Get()), Soylent_ReceptantExt);
		}

		// if we don't have a proper receptant then don't bother to do at all
		if (pReceptant)
		{
			auto pReceptantTechno = abstract_cast<TechnoClass*>(pReceptant);
			auto pReceptantHouse = HandlerCompClass::GetOwningHouseOfActor(pReceptant);

			auto multiplier = Soylent_Mult.Get();
			int nMoneyToGive = (int)(pTarget->GetTechnoType()->GetRefund(pTarget->Owner, true) * multiplier);

			if (Soylent_IncludePassengers.Get())
			{
				nMoneyToGive += TechnoTypeExt::GetTotalSoylentOfPassengers(multiplier, pTarget);
			}

			if (nMoneyToGive != 0)
			{
				pReceptantHouse->TransactMoney(nMoneyToGive);
				if (pReceptantTechno && Soylent_Display.Get())
				{
					FlyingStrings::AddMoneyString(nMoneyToGive, pReceptantHouse,
						Soylent_Display_Houses, pReceptantTechno->Location, Soylent_Display_Offset);
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

			// Remove each passenger and unlimbo (place) it at a random place nearby.
			// This supports transport vehicles and Bio Reactors.
			while (pTarget->Passengers.FirstPassenger)
			{
				FootClass* pPassenger = pTarget->Passengers.RemoveFirstPassenger();
				auto const pPassExt = TechnoExt::ExtMap.Find(pPassenger);
				auto const coords = pTarget->GetCoords();
				pPassExt->UnlimboAtRandomPlaceNearby(&coords);
				if (openTopped)
				{
					pTarget->ExitedOpenTopped(pPassenger);
				}
			}
		}
		else if (pTarget->GetOccupantCount() > 0)
		{
			auto pTargetBld = reinterpret_cast<BuildingClass*>(pTarget);
			// BuildingClass* -> RemoveOccupants
			reinterpret_cast<void(__thiscall*)(BuildingClass*, int, int)>(0x457DE0)(pTargetBld, 0, 0);
		}
	}

	// Passenger Removal
	if (Passengers_Kill.Get())
	{
		AbstractClass* pScorer = nullptr;
		if (Passengers_Kill_Score.Get())
		{
			if (Passengers_Kill_Scorer.isset())
				pScorer = HandlerCompClass::GetTrueTarget(pParticipants->at(Passengers_Kill_Scorer.Get()), Passengers_Kill_ScorerExt);
			else
				pScorer = pOwner;
		}
		auto pScorerTechno = abstract_cast<TechnoClass*>(pScorer);
		if (pTarget->Passengers.NumPassengers > 0)
		{
			pTarget->KillPassengers(pScorerTechno);
		}
		else if (pTarget->GetOccupantCount() > 0)
		{
			// If it is garrisoned, it must be a garrisonable structure, so it is safe to use "reinterpret_cast" here.
			auto pTargetBld = reinterpret_cast<BuildingClass*>(pTarget);
			pTargetBld->KillOccupants(pScorerTechno);
		}
	}

	// Passenger Creation
	if (!Passengers_Create_Types.empty())
	{
		if (Passengers_Create_Owner.isset())
		{
			auto pPassengerOwnerActor = HandlerCompClass::GetTrueTarget(pParticipants->at(Passengers_Create_Owner.Get()), Passengers_Create_OwnerExt);
			if (pPassengerOwnerActor)
			{
				this->CreatePassengers(pTarget, HandlerCompClass::GetOwningHouseOfActor(pPassengerOwnerActor));
			}
		}
		else
		{
			this->CreatePassengers(pTarget, pTarget->Owner);
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

	// Transfer
	if (Transfer_To_House.isset() || Transfer_To_Actor.isset())
	{
		HouseClass* pTransferToHouse = nullptr;
		if (Transfer_To_House.isset())
		{
			switch (Transfer_To_House.Get())
			{
			case OwnerHouseKind::Civilian:
				pTransferToHouse = HouseClass::FindCivilianSide();
				break;
			case OwnerHouseKind::Neutral:
				pTransferToHouse = HouseClass::FindNeutral();
				break;
			case OwnerHouseKind::Special:
				pTransferToHouse = HouseClass::FindSpecial();
				break;
			}
		}
		if (Transfer_To_Actor.isset())
		{
			auto pTransferTo = HandlerCompClass::GetTrueTarget(pParticipants->at(Transfer_To_Actor.Get()), Transfer_To_ActorExt);
			if (pTransferTo)
			{
				pTransferToHouse = HandlerCompClass::GetOwningHouseOfActor(pTransferTo);
			}
		}
		if (pTransferToHouse && pTransferToHouse != pTarget->Owner)
		{
			this->TransferOwnership(pTarget, pTransferToHouse);
		}
	}

	// Command
	if (Command.isset())
	{
		TechnoClass* pCommandTarget = nullptr;
		bool isDestination = false;
		switch (Command.Get())
		{
			// These command types require an explicit target or it doesn't count.
		case Mission::Enter:
		case Mission::Attack:
			if (Command_Target.isset())
				pCommandTarget = abstract_cast<TechnoClass*>(HandlerCompClass::GetTrueTarget(pParticipants->at(Command_Target.Get()), Command_TargetExt));
			if (!pCommandTarget)
				goto _EndCommand_;
			isDestination = Command.Get() == Mission::Enter;
			break;
			// These command types do not require a target.
		case Mission::Guard:
		case Mission::Unload:
			break;
		default:
			goto _EndCommand_;
		}

		pTarget->QueueMission(Command.Get(), false);
		if (pCommandTarget)
		{
			if (isDestination)
			{
				pTarget->SetTarget(nullptr);
				pTarget->SetDestination(pCommandTarget, true);
			}
			else
			{
				pTarget->SetTarget(pCommandTarget);
			}
		}

	_EndCommand_:;
	}
}

void HandlerEffectClass::ExecuteForHouse(AbstractClass* pOwner, HouseClass* pOwnerHouse, std::map<EventActorType, AbstractClass*>* pParticipants, HouseClass* pTarget) const
{
	// EVA
	if (EVA.isset())
	{
		if (pTarget->IsControlledByCurrentPlayer())
		{
			VoxClass::PlayIndex(EVA.Get());
		}
	}

	// Player Emblems
	if (!PlayerEmblem_AttachTypes.empty() || !PlayerEmblem_RemoveTypes.empty())
	{
		auto const pTargetExt = HouseExt::ExtMap.Find(pTarget);

		bool refreshBuildOptionsFlag = false;

		if (!PlayerEmblem_AttachTypes.empty())
		{
			for (auto const pEmblemType : PlayerEmblem_AttachTypes)
			{
				if (!pTargetExt->PlayerEmblems.contains(pEmblemType))
				{
					pTargetExt->PlayerEmblems.insert(pEmblemType);
					pEmblemType->InvokeEventHandlers(EventTypeClass::WhenAttach, pTarget);
					refreshBuildOptionsFlag = refreshBuildOptionsFlag || pEmblemType->AlterBuildOptions();
				}
			}
		}

		if (!PlayerEmblem_RemoveTypes.empty())
		{
			for (auto const pEmblemType : PlayerEmblem_AttachTypes)
			{
				if (pTargetExt->PlayerEmblems.contains(pEmblemType))
				{
					pTargetExt->PlayerEmblems.erase(pEmblemType);
					pEmblemType->InvokeEventHandlers(EventTypeClass::WhenDetach, pTarget);
					refreshBuildOptionsFlag = refreshBuildOptionsFlag || pEmblemType->AlterBuildOptions();
				}
			}
		}

		if (refreshBuildOptionsFlag)
		{
			pTargetExt->UpdatePlayerEmblemBuildOptions();
		}
	}
}

void HandlerEffectClass::ExecuteGeneric(AbstractClass* pOwner, HouseClass* pOwnerHouse, std::map<EventActorType, AbstractClass*>*pParticipants, AbstractClass* pTarget) const
{
	std::map<EventActorType, AbstractClass*> participants = {
		{ EventActorType::Me, nullptr },
		{ EventActorType::They, nullptr },
		{ EventActorType::Scoper, pParticipants->operator[](EventActorType::Scoper) },
		{ EventActorType::Enchanter, pParticipants->operator[](EventActorType::Enchanter) },
	};

	// Event Invoker
	if (!EventInvokers.empty())
	{
		participants[EventActorType::Me] = pTarget;
		participants[EventActorType::They] = pOwner;

		for (auto pEventInvokerType : EventInvokers)
		{
			pEventInvokerType->TryExecute(pOwnerHouse, &participants);
		}
	}

	// Area Search
	if (!Scope_EventInvokers.empty())
	{
		participants[EventActorType::Me] = nullptr;
		participants[EventActorType::They] = pOwner;
		participants[EventActorType::Scoper] = pTarget;

		std::function<void(TechnoClass*)> tryInvoke = [this, pOwnerHouse, &participants](TechnoClass* pItem)
			{
				if (IsEligibleForAreaSearch(pItem, pOwnerHouse))
				{
					participants.operator[](EventActorType::Me) = pItem;
					for (auto pEventInvokerType : Scope_EventInvokers)
					{
						pEventInvokerType->TryExecute(pOwnerHouse, &participants);
					}
				}
			};

		if (Scope_MapWide.Get())
		{
			auto copy_dvc = []<typename T>(const DynamicVectorClass<T>&dvc)
			{
				std::vector<T> vec(dvc.Count);
				std::copy(dvc.begin(), dvc.end(), vec.begin());
				return vec;
			};

			if ((Scope_Abstract & AffectedTarget::Aircraft) != AffectedTarget::None)
			{
				auto const aircraft = copy_dvc(*AircraftClass::Array);

				for (auto pAircraft : aircraft)
					tryInvoke(pAircraft);
			}

			if ((Scope_Abstract & AffectedTarget::Building) != AffectedTarget::None)
			{
				auto const buildings = copy_dvc(*BuildingClass::Array);

				for (auto pBuilding : buildings)
					tryInvoke(pBuilding);
			}

			if ((Scope_Abstract & AffectedTarget::Infantry) != AffectedTarget::None)
			{
				auto const infantry = copy_dvc(*InfantryClass::Array);

				for (auto pInf : infantry)
					tryInvoke(pInf);
			}

			if ((Scope_Abstract & AffectedTarget::Unit) != AffectedTarget::None)
			{
				auto const units = copy_dvc(*UnitClass::Array);

				for (auto const pUnit : units)
					tryInvoke(pUnit);
			}
		}
		else
		{
			if (auto pTargetObj = abstract_cast<ObjectClass*>(pTarget))
			{
				const auto pItems = Helpers::Alex::getCellSpreadItems(pTargetObj->Location, Scope_Radius.Get(), Scope_AirIncluded);
				for (auto pItem : pItems)
				{
					tryInvoke(pItem);
				}
			}
		}
	}
}

bool HandlerEffectClass::IsEligibleForAreaSearch(TechnoClass* pTechno, HouseClass* pOwner) const
{
	if (!pTechno || !pTechno->IsOnMap || !pTechno->IsAlive || pTechno->InLimbo || pTechno->IsSinking)
		return false;

	if (pOwner && !EnumFunctions::CanTargetHouse(this->Scope_House, pOwner, pTechno->Owner))
		return false;

	if (pTechno->IsInAir() && !Scope_AirIncluded)
		return false;

	return Scope_TechnoTypes.Contains(pTechno->GetTechnoType());
}

// Basically copied from Ares "TechnoExt::ExtData::CreateInitialPayload()".
// This supports transport vehicles, Bio Reactors, and garrisonable structures.
void HandlerEffectClass::CreatePassengers(TechnoClass* pToWhom, HouseClass* pPassengerOwner) const
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
			auto const pObject = pPayloadType->CreateObject(pPassengerOwner);

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

void HandlerEffectClass::TransferOwnership(TechnoClass* pTarget, HouseClass* pNewOnwer) const
{
	// if MCed, release from MC
	if (auto const pController = pTarget->MindControlledBy)
	{
		pController->CaptureManager->FreeUnit(pTarget);
	}

	// remove perma control ring effect
	pTarget->MindControlledByAUnit = false;
	if (pTarget->MindControlRingAnim)
	{
		pTarget->MindControlRingAnim->UnInit();
		pTarget->MindControlRingAnim = nullptr;
	}

	// transfer ownership and cancel mission
	pTarget->SetOwningHouse(pNewOnwer, false);
	pTarget->QueueMission(Mission::Guard, false);

	// reboot the slave manager
	if (pTarget->SlaveManager)
	{
		pTarget->SlaveManager->ResumeWork();
	}
}

bool HandlerEffectClass::IsDefined() const
{
	return HasAnyTechnoEffect
		|| HasAnyHouseEffect
		|| HasAnyGenericEffect;
}

bool HandlerEffectClass::IsDefinedAnyTechnoEffect() const
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
		|| Transfer_To_House.isset()
		|| Transfer_To_Actor.isset()
		|| Command.isset();
}

bool HandlerEffectClass::IsDefinedAnyHouseEffect() const
{
	return EVA.isset()
		|| !PlayerEmblem_AttachTypes.empty()
		|| !PlayerEmblem_RemoveTypes.empty();
}

bool HandlerEffectClass::IsDefinedAnyGenericEffect() const
{
	return !EventInvokers.empty()
		|| !Scope_EventInvokers.empty();
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
#pragma region TechnoEffects
		.Process(this->HasAnyTechnoEffect)
		.Process(this->Weapon)
		.Process(this->Weapon_Firer)
		.Process(this->Weapon_FirerExt)
		.Process(this->Weapon_SpawnProj)
		.Process(this->Convert_Pairs)
		.Process(this->Soylent_Mult)
		.Process(this->Soylent_IncludePassengers)
		.Process(this->Soylent_Receptant)
		.Process(this->Soylent_ReceptantExt)
		.Process(this->Soylent_Display)
		.Process(this->Soylent_Display_Houses)
		.Process(this->Soylent_Display_Offset)
		.Process(this->Passengers_Eject)
		.Process(this->Passengers_Kill)
		.Process(this->Passengers_Kill_Score)
		.Process(this->Passengers_Kill_Scorer)
		.Process(this->Passengers_Kill_ScorerExt)
		.Process(this->Passengers_Create_Types)
		.Process(this->Passengers_Create_Nums)
		.Process(this->Passengers_Create_Owner)
		.Process(this->Passengers_Create_OwnerExt)
		.Process(this->Veterancy_Set)
		.Process(this->Veterancy_Add)
		.Process(this->Voice)
		.Process(this->Voice_Persist)
		.Process(this->Voice_Global)
		.Process(this->Transfer_To_House)
		.Process(this->Transfer_To_Actor)
		.Process(this->Transfer_To_ActorExt)
		.Process(this->Command)
		.Process(this->Command_Target)
		.Process(this->Command_TargetExt)
#pragma endregion
#pragma region HouseEffects
		.Process(this->HasAnyHouseEffect)
		.Process(this->EVA)
		.Process(this->PlayerEmblem_AttachTypes)
		.Process(this->PlayerEmblem_RemoveTypes)
#pragma endregion
#pragma region GenericEffects
		.Process(this->HasAnyGenericEffect)
		.Process(this->EventInvokers)
		.Process(this->Scope_Radius)
		.Process(this->Scope_MapWide)
		.Process(this->Scope_Abstract)
		.Process(this->Scope_House)
		.Process(this->Scope_AirIncluded)
		.Process(this->Scope_TechnoTypes)
		.Process(this->Scope_EventInvokers)
#pragma endregion
		.Success();
}
