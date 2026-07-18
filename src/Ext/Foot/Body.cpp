#include "Body.h"

#include <JumpjetLocomotionClass.h>

#include <Ext/Anim/Body.h>
#include <Misc/FlyingStrings.h>

void FootExt::UpdateTiberiumEater()
{
	const auto pEaterType = this->TypeExtData->TiberiumEaterType.get();

	if (!pEaterType)
		return;

	const int transDelay = pEaterType->TransDelay;

	if (transDelay && this->TiberiumEater_Timer.InProgress())
		return;

	const auto pThis = this->OwnerObject();
	const auto pOwner = pThis->Owner;
	bool active = false;
	const bool displayCash = pEaterType->Display && pThis->IsClearlyVisibleTo(HouseClass::CurrentPlayer);
	int facing = pThis->PrimaryFacing.Current().GetFacing<8>();

	if (facing >= 7)
		facing = 0;
	else
		facing++;

	const int cellCount = static_cast<int>(pEaterType->Cells.size());
	const int locationZ = pThis->Location.Z;
	const int numOrePurifiers = pOwner->NumOrePurifiers;
	const float cashMultiplier = pEaterType->CashMultiplier;
	const float purifierBonus = RulesClass::Instance->PurifierBonus;
	const bool animMove = pEaterType->AnimMove;
	const auto displayToHouse = pEaterType->DisplayToHouse;
	const auto amountPerCell = pEaterType->AmountPerCell;
	const auto displayOffset = pEaterType->DisplayOffset;
	const auto& animsAll = pEaterType->Anims;
	auto* scenarioRandom = &ScenarioClass::Instance->Random;

	for (int idx = 0; idx < cellCount; idx++)
	{
		const auto& cellOffset = pEaterType->Cells[idx];
		const auto pos = TechnoExt::GetFLHAbsoluteCoords(pThis, CoordStruct { cellOffset.X, cellOffset.Y, 0 }, false);
		const auto pCell = MapClass::Instance.TryGetCellAt(pos);

		if (!pCell)
			continue;

		if (const int contained = pCell->GetContainedTiberiumValue())
		{
			const int tiberiumIdx = pCell->GetContainedTiberiumIndex();
			const int tiberiumValue = TiberiumClass::Array[tiberiumIdx]->Value;
			const int tiberiumAmount = static_cast<int>(static_cast<double>(contained) / tiberiumValue);
			const int amount = amountPerCell > 0 ? std::min(amountPerCell.Get(), tiberiumAmount) : tiberiumAmount;
			pCell->ReduceTiberium(amount);
			const float multiplier = cashMultiplier * (1.0f + numOrePurifiers * purifierBonus);
			const int value = static_cast<int>(std::round(amount * tiberiumValue * multiplier));
			pOwner->TransactMoney(value);
			active = true;

			if (displayCash)
			{
				auto cellCoords = pCell->GetCoords();
				cellCoords.Z = std::max(locationZ, cellCoords.Z);
				FlyingStrings::AddMoneyString(value, pThis, pOwner, displayToHouse, cellCoords, displayOffset);
			}

			const auto& anims = pEaterType->Anims_Tiberiums[tiberiumIdx].GetElements(animsAll);
			const int animCount = static_cast<int>(anims.size());

			if (animCount == 0)
				continue;

			AnimTypeClass* pAnimType = nullptr;

			switch (animCount)
			{
			case 1:
				pAnimType = anims[0];
				break;

			case 8:
				pAnimType = anims[facing];
				break;

			default:
				pAnimType = anims[scenarioRandom->RandomRanged(0, animCount - 1)];
				break;
			}

			if (pAnimType)
			{
				const auto pAnim = GameCreate<AnimClass>(pAnimType, pos);
				AnimExt::SetAnimOwnerHouseKind(pAnim, pThis->Owner, nullptr, false, true);

				if (animMove)
					pAnim->SetOwnerObject(pThis);
			}
		}
	}

	if (active && transDelay)
		this->TiberiumEater_Timer.Start(pEaterType->TransDelay);
}

void FootExt::UpdateOnTunnelEnter()
{
	if (!this->IsInTunnel)
	{
		if (const auto pShieldData = this->Shield.get())
			pShieldData->SetAnimationVisibility(false);

		for (const auto& pTrail : this->LaserTrails)
		{
			pTrail->Visible = false;
			pTrail->LastLocation = { };
		}

		this->IsInTunnel = true;
	}
}

void FootExt::UpdateOnTunnelExit()
{
	this->IsInTunnel = false;

	if (const auto pShieldData = this->Shield.get())
		pShieldData->SetAnimationVisibility(true);
}

void FootExt::UpdateTypeData_Foot()
{
	auto const pThis = this->OwnerObject();
	auto const pOldType = this->PreviousType;
	auto const pCurrentType = this->TypeExtData->OwnerObject();
	auto const abs = pThis->WhatAmI();
	//auto const pOldTypeExt = TechnoTypeExt::Fetch(pOldType);

	// Update movement sound if still moving while type changed.
	if (pThis->IsMoveSoundPlaying && pThis->Locomotor->Is_Moving())
	{
		if (pCurrentType->MoveSound != pOldType->MoveSound)
		{
			// End the old sound.
			pThis->MoveSoundAudioController.End();

			if (auto const count = pCurrentType->MoveSound.Count)
			{
				// Play a new sound.
				const int soundIndex = pCurrentType->MoveSound[Randomizer::Global.Random() % count];
				VocClass::PlayAt(soundIndex, pThis->Location, &pThis->MoveSoundAudioController);
				pThis->IsMoveSoundPlaying = true;
			}
			else
			{
				pThis->IsMoveSoundPlaying = false;
			}

			pThis->MoveSoundDelay = 0;
		}
	}

	if (abs == AbstractType::Infantry)
	{
		auto const pInf = static_cast<InfantryClass*>(pThis);

		// It's still not recommended to have such idea, please avoid using this
		if (static_cast<InfantryTypeClass*>(pOldType)->Deployer && !static_cast<InfantryTypeClass*>(pCurrentType)->Deployer)
		{
			switch (pInf->SequenceAnim)
			{
			case Sequence::Deploy:
			case Sequence::Deployed:
			case Sequence::DeployedIdle:
				pInf->PlayAnim(Sequence::Ready, true);
				break;
			case Sequence::DeployedFire:
				pInf->PlayAnim(Sequence::FireUp, true);
				break;
			default:
				break;
			}
		}
	}

	if (pOldType->Locomotor == LocomotionClass::CLSIDs::Teleport && pCurrentType->Locomotor != LocomotionClass::CLSIDs::Teleport && pThis->WarpingOut)
		this->HasRemainingWarpInDelay = true;

	// Update open topped state of potential passengers if transport's OpenTopped value changes.
	// OpenTopped does not work properly with buildings to begin with which is why this is here rather than in the Techno update one.
	if (pThis->Passengers.NumPassengers > 0)
	{
		const bool toOpenTopped = pCurrentType->OpenTopped;
		FootClass* pFirstPassenger = pThis->Passengers.GetFirstPassenger();

		while (true)
		{
			if (toOpenTopped)
			{
				pFirstPassenger->SetLocation(pThis->Location);
				// Add passengers to the logic layer.
				pThis->EnteredOpenTopped(pFirstPassenger);
			}
			else
			{
				// Lose target & destination
				pFirstPassenger->SetTarget(nullptr);
				pFirstPassenger->SetCurrentWeaponStage(0);
				pFirstPassenger->AbortMotion();
				pThis->ExitedOpenTopped(pFirstPassenger);

				// OpenTopped adds passengers to logic layer when enabled. Under normal conditions this does not need to be removed since
				// OpenTopped state does not change while passengers are still in transport but in case of type conversion that can happen.
				LogicClass::Instance.RemoveObject(pFirstPassenger);
			}

			pFirstPassenger->Transporter = pThis;

			if (const auto pNextPassenger = abstract_cast<FootClass*>(pFirstPassenger->NextObject))
				pFirstPassenger = pNextPassenger;
			else
				break;
		}

		if (pCurrentType->Gunner)
			pThis->ReceiveGunner(pFirstPassenger);
	}
	else if (pCurrentType->Gunner)
	{
		pThis->RemoveGunner(nullptr);
	}

	if (!pCurrentType->CanDisguise || (!pThis->Disguise && pCurrentType->PermaDisguise))
	{
		// When it can't disguise or has lost its disguise, update its disguise.
		pThis->ClearDisguise();
	}

	if (abs != AbstractType::Aircraft)
	{
		auto const pLocomotorType = pCurrentType->Locomotor;

		// The Hover movement pattern allows for self-landing.
		if (pLocomotorType != LocomotionClass::CLSIDs::Fly && pLocomotorType != LocomotionClass::CLSIDs::Hover)
		{
			const bool isinAir = pThis->IsInAir() && !pThis->LocomotorSource;

			if (auto const pJJLoco = locomotion_cast<JumpjetLocomotionClass*>(pThis->Locomotor))
			{
				const int turnrate = pCurrentType->JumpjetTurnRate >= 127 ? 127 : pCurrentType->JumpjetTurnRate;
				pJJLoco->Speed = pCurrentType->JumpjetSpeed;
				pJJLoco->Climb = pCurrentType->JumpjetClimb;
				pJJLoco->Accel = pCurrentType->JumpjetAccel;
				pJJLoco->Crash = pCurrentType->JumpjetCrash;
				pJJLoco->Deviation = pCurrentType->JumpjetDeviation;
				pJJLoco->NoWobbles = pCurrentType->JumpjetNoWobbles;
				pJJLoco->Wobbles = pCurrentType->JumpjetWobbles;
				pJJLoco->TurnRate = turnrate;
				pJJLoco->CurrentHeight = pCurrentType->JumpjetHeight;
				pJJLoco->Height = pCurrentType->JumpjetHeight;
				pJJLoco->LocomotionFacing.SetROT(turnrate);

				if (isinAir)
				{
					if (pCurrentType->BalloonHover)
					{
						// Makes the jumpjet think it is hovering without actually moving.
						pJJLoco->State = JumpjetLocomotionClass::State::Hovering;
						pJJLoco->IsMoving = true;

						if (!pJJLoco->Is_Moving_Now())
							pJJLoco->DestinationCoords = pThis->Location;
					}
					else if (!pJJLoco->Is_Moving_Now())
					{
						pJJLoco->Move_To(pThis->Location);
					}
				}
			}
			else if (isinAir)
			{
				// Let it go into free fall.
				pThis->IsFallingDown = true;

				const auto pCell = MapClass::Instance.TryGetCellAt(pThis->Location);

				if (pCell && !pCell->IsClearToMove(pCurrentType->SpeedType, true, true,
					-1, pCurrentType->MovementZone, pCell->GetLevel(), pCell->ContainsBridge()))
				{
					// If it's landing position cannot be moved, then it is granted a crash death.
					pThis->IsABomb = true;
				}
				else
				{
					// If it's gonna land on the bridge, then it needs this.
					pThis->OnBridge = pCell ? pCell->ContainsBridge() : false;
					this->OnParachuted = true;
				}

				if (abs == AbstractType::Infantry)
				{
					// Infantry changed to parachute status (not required).
					static_cast<InfantryClass*>(pThis)->PlayAnim(Sequence::Paradrop, true, false);
				}
			}
		}

		if (abs == AbstractType::Unit)
		{
			// Yes, synchronize its turret facing or it will turn strangely.
			if (pOldType->Turret != pCurrentType->Turret)
			{
				const auto primaryFacing = pThis->PrimaryFacing.Current();
				auto& secondaryFacing = pThis->SecondaryFacing;

				secondaryFacing.SetCurrent(primaryFacing);
				secondaryFacing.SetDesired(primaryFacing);
			}
		}
	}

	this->PreviousType = nullptr;
}

void FootExt::UpdateWarpInDelay()
{
	if (this->HasRemainingWarpInDelay)
	{
		if (this->LastWarpInDelay)
		{
			this->LastWarpInDelay--;
		}
		else
		{
			this->HasRemainingWarpInDelay = false;
			this->IsBeingChronoSphered = false;
			this->OwnerObject()->WarpingOut = false;
		}
	}
}

// =============================
// load / save

template <typename T>
void FootExt::Serialize(T& Stm)
{
	Stm
		.Process(this->LastKillWasTeamTarget)
		.Process(this->LastWarpDistance)
		.Process(this->JumpjetSpeed)
		.Process(this->IsInTunnel)
		.Process(this->OriginalPassengerOwner)
		.Process(this->HasRemainingWarpInDelay)
		.Process(this->LastWarpInDelay)
		.Process(this->IsBeingChronoSphered)
		.Process(this->LastSensorsMapCoords)
		.Process(this->TiberiumEater_Timer)
		.Process(this->ResetLocomotor)
		.Process(this->JumpjetStraightAscend)
		;
}

void FootExt::LoadFromStream(PhobosStreamReader& Stm)
{
	TechnoExt::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void FootExt::SaveToStream(PhobosStreamWriter& Stm)
{
	TechnoExt::SaveToStream(Stm);
	this->Serialize(Stm);
}
