#include "Body.h"

#include <Kamikaze.h>

#include <JumpjetLocomotionClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/House/Body.h>
#include <Ext/Scenario/Body.h>
#include <Ext/Unit/Body.h>
#include <Misc/FlyingStrings.h>
#include <Utilities/AresFunctions.h>

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

void FootExt::UpdateTypeData(TechnoTypeClass* pCurrentType)
{
	auto const pThis = this->OwnerObject();
	auto const pOldType = this->TypeExtData->OwnerObject();
	auto const pOldTypeExt = TechnoTypeExt::Fetch(pOldType);
	auto const pOwner = pThis->Owner;
	auto& pSlaveManager = pThis->SlaveManager;
	auto& pSpawnManager = pThis->SpawnManager;
	auto& pCaptureManager = pThis->CaptureManager;
	auto& pTemporalImUsing = pThis->TemporalImUsing;
	auto& pAirstrike = pThis->Airstrike;

	auto const pNewTypeExt = TechnoTypeExt::Fetch(pCurrentType);
	this->TypeExtData = pNewTypeExt;

	this->UpdateSelfOwnedAttachEffects();

	if (auto const pShield = this->Shield.get())
		pShield->ConvertCheck(pCurrentType);

	// Recalculate and redraw
	pThis->MarkForRedraw();
	this->UpdateTintValues();

	// Recreate Laser Trails
	if (const size_t trailCount = this->LaserTrails.size())
	{
		std::vector<std::unique_ptr<LaserTrailClass>> addition;
		addition.reserve(trailCount);

		for (auto& pTrail : this->LaserTrails)
		{
			if (!pTrail->Intrinsic)
				addition.emplace_back(std::move(pTrail));
		}

		this->LaserTrails.clear();
		this->LaserTrails.reserve(this->TypeExtData->LaserTrailData.size() + addition.size());

		for (const auto& entry : this->TypeExtData->LaserTrailData)
			this->LaserTrails.emplace_back(std::make_unique<LaserTrailClass>(entry.GetType(), pOwner, entry.FLH, entry.IsOnTurret));

		for (auto& pTrail : addition)
			this->LaserTrails.emplace_back(std::move(pTrail));
	}
	else if (const size_t trailSize = pNewTypeExt->LaserTrailData.size())
	{
		this->LaserTrails.reserve(trailSize);

		for (const auto& entry : pNewTypeExt->LaserTrailData)
			this->LaserTrails.emplace_back(std::make_unique<LaserTrailClass>(entry.GetType(), pOwner, entry.FLH, entry.IsOnTurret));
	}

	// Reset AutoDeath Timer if new techno type doesn't have timed AutoDeath
	if (this->AutoDeathTimer.HasStarted() && pNewTypeExt->AutoDeath_AfterDelay <= 0)
		this->AutoDeathTimer.Stop();

	// Reset PassengerDeletion Timer
	if (this->PassengerDeletionTimer.HasStarted() && pNewTypeExt->PassengerDeletionType && pNewTypeExt->PassengerDeletionType->Rate <= 0)
		this->PassengerDeletionTimer.Stop();

	// Remove from tracked AutoDeath objects if no longer has AutoDeath
	if (pOldTypeExt->AutoDeath_Behavior.isset() && !pNewTypeExt->AutoDeath_Behavior.isset())
	{
		auto& vec = ScenarioExt::Global()->AutoDeathObjects;
		vec.erase(std::remove(vec.begin(), vec.end(), this), vec.end());
	}

	// Remove from harvesters list if no longer a harvester.
	if (pOldTypeExt->Harvester_Counted)
	{
		if (!pNewTypeExt->Harvester_Counted)
		{
			auto& vec = HouseExt::Fetch(pOwner)->OwnedCountedHarvesters;
			vec.erase(std::remove(vec.begin(), vec.end(), pThis), vec.end());
		}
	}
	// Add to harvesters list if it's a harvester.
	else if (pNewTypeExt->Harvester_Counted)
	{
		HouseExt::Fetch(pOwner)->OwnedCountedHarvesters.push_back(pThis);
	}

	// Remove from limbo reloaders if no longer applicable
	if (pOldType->Ammo > 0 && pOldTypeExt->ReloadInTransport.Get(RulesExt::Global()->ReloadInTransport) && !pNewTypeExt->ReloadInTransport.Get(RulesExt::Global()->ReloadInTransport))
	{
		auto& vec = ScenarioExt::Global()->TransportReloaders;
		vec.erase(std::remove(vec.begin(), vec.end(), this), vec.end());
	}

	// Powered by ststl-s, Fly-Star
	if (pCurrentType->Enslaves && pCurrentType->SlavesNumber > 0)
	{
		// SlaveManager does not exist or they have different slaves.
		if (!pSlaveManager || pSlaveManager->SlaveType != pCurrentType->Enslaves)
		{
			if (pSlaveManager)
			{
				// Slaves are not the same, so clear out.
				pSlaveManager->Killed(nullptr);
				GameDelete(pSlaveManager);
				pSlaveManager = nullptr;
			}

			pSlaveManager = GameCreate<SlaveManagerClass>(pThis, pCurrentType->Enslaves, pCurrentType->SlavesNumber, pCurrentType->SlaveRegenRate, pCurrentType->SlaveReloadRate);
		}
		else if (pSlaveManager->SlaveCount != pCurrentType->SlavesNumber)
		{
			// Additions/deletions made when quantities are inconsistent.
			if (pSlaveManager->SlaveCount < pCurrentType->SlavesNumber)
			{
				// There are too few slaves here. More are needed.
				const int count = pCurrentType->SlavesNumber - pSlaveManager->SlaveCount;

				for (int i = 0; i < count; i++)
				{
					const auto pSlaveNode = GameCreate<SlaveManagerClass::SlaveControl>();
					pSlaveNode->Slave = nullptr;
					pSlaveNode->State = SlaveControlStatus::Dead;
					pSlaveNode->RespawnTimer.Start(pCurrentType->SlaveRegenRate);
					pSlaveManager->SlaveNodes.AddItem(pSlaveNode);
				}
			}
			else
			{
				// Remove excess slaves
				for (int i = pSlaveManager->SlaveCount - 1; i >= pCurrentType->SlavesNumber; --i)
				{
					if (const auto pSlaveNode = pSlaveManager->SlaveNodes.GetItem(i))
					{
						if (const auto pSlave = pSlaveNode->Slave)
						{
							if (pSlave->InLimbo)
							{
								// He wasn't killed, just erased.
								pSlave->RegisterDestruction(pThis);
								pSlave->UnInit();
							}
							else
							{
								// Oh, my God, he's been killed.
								pSlave->ReceiveDamage(&pSlave->Health, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, nullptr);
							}
						}

						// Unlink
						pSlaveNode->Slave = nullptr;
						pSlaveNode->State = SlaveControlStatus::Dead;
						GameDelete(pSlaveNode);
					}

					// Remove it
					pSlaveManager->SlaveNodes.RemoveItem(i);
				}
			}

			pSlaveManager->SlaveCount = pCurrentType->SlavesNumber;
		}
	}
	else if (pSlaveManager)
	{
		pSlaveManager->Killed(nullptr);
		GameDelete(pSlaveManager);
		pSlaveManager = nullptr;
	}

	if (pCurrentType->Spawns && pCurrentType->SpawnsNumber > 0)
	{
		// No SpawnManager exists, or their SpawnType is inconsistent.
		if (!pSpawnManager || pCurrentType->Spawns != pSpawnManager->SpawnType)
		{
			if (pSpawnManager)
			{
				// It may be odd that AircraftType is different, I chose to reset it.
				pSpawnManager->KillNodes();
				GameDelete(pSpawnManager);
			}

			pSpawnManager = GameCreate<SpawnManagerClass>(pThis, pCurrentType->Spawns, pCurrentType->SpawnsNumber, pCurrentType->SpawnRegenRate, pCurrentType->SpawnReloadRate);
		}
		else if (pSpawnManager->SpawnCount != pCurrentType->SpawnsNumber)
		{
			// Additions/deletions made when quantities are inconsistent.
			if (pSpawnManager->SpawnCount < pCurrentType->SpawnsNumber)
			{
				const int count = pCurrentType->SpawnsNumber - pSpawnManager->SpawnCount;

				// Add the missing Spawns, but don't intend for them to be born right away.
				for (int i = 0; i < count; i++)
				{
					const auto pSpawnNode = GameCreate<SpawnControl>();
					pSpawnNode->Unit = nullptr;
					pSpawnNode->Status = SpawnNodeStatus::Dead;
					pSpawnNode->SpawnTimer.Start(pCurrentType->SpawnRegenRate);
					pSpawnNode->IsSpawnMissile = false;
					pSpawnManager->SpawnedNodes.AddItem(pSpawnNode);
				}
			}
			else
			{
				// Remove excess spawns
				for (int i = pSpawnManager->SpawnCount - 1; i >= pCurrentType->SpawnsNumber; --i)
				{
					if (const auto pSpawnNode = pSpawnManager->SpawnedNodes.GetItem(i))
					{
						auto& pStatus = pSpawnNode->Status;

						// Spawns that don't die get killed.
						if (const auto pAircraft = pSpawnNode->Unit)
						{
							pAircraft->SpawnOwner = nullptr;

							if (pAircraft->InLimbo
								|| pStatus == SpawnNodeStatus::Idle
								|| pStatus == SpawnNodeStatus::Reloading
								|| pStatus == SpawnNodeStatus::TakeOff)
							{
								if (pStatus == SpawnNodeStatus::TakeOff)
									Kamikaze::Instance.Remove(pAircraft);

								pAircraft->UnInit();
							}
							else if (pSpawnNode->IsSpawnMissile)
							{
								pAircraft->ReceiveDamage(&pAircraft->Health, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, nullptr);
							}
							else
							{
								pAircraft->Crash(nullptr);
							}
						}

						// Unlink
						pSpawnNode->Unit = nullptr;
						pStatus = SpawnNodeStatus::Dead;
						GameDelete(pSpawnNode);
					}

					// Remove it
					pSpawnManager->SpawnedNodes.RemoveItem(i);
				}
			}

			pSpawnManager->SpawnCount = pCurrentType->SpawnsNumber;
		}
	}
	else if (pSpawnManager)
	{
		// Reset the target.
		pSpawnManager->ResetTarget();

		// pSpawnManager->KillNodes() kills all Spawns, but it is not necessary to kill the parts that are not performing tasks.
		for (const auto pSpawnNode : pSpawnManager->SpawnedNodes)
		{
			const auto pAircraft = pSpawnNode->Unit;
			auto& pStatus = pSpawnNode->Status;

			// A dead or idle Spawn is not killed.
			if (!pAircraft
				|| pStatus == SpawnNodeStatus::Dead
				|| pStatus == SpawnNodeStatus::Idle
				|| pStatus == SpawnNodeStatus::Reloading)
			{
				continue;
			}

			pAircraft->SpawnOwner = nullptr;

			if (pStatus == SpawnNodeStatus::TakeOff)
			{
				Kamikaze::Instance.Remove(pAircraft);
				pAircraft->UnInit();
			}
			else if (pSpawnNode->IsSpawnMissile)
			{
				pAircraft->ReceiveDamage(&pAircraft->Health, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, nullptr);
			}
			else
			{
				pAircraft->Crash(nullptr);
			}

			pSpawnNode->Unit = nullptr;
			pStatus = SpawnNodeStatus::Dead;
			pSpawnNode->IsSpawnMissile = false;
			pSpawnNode->SpawnTimer.Start(pSpawnManager->RegenRate);
		}
	}

	// Prepare the variables.
	int maxCapture = 0;
	bool infiniteCapture = false;
	bool hasTemporal = false;
	bool hasAirstrike = false;
	bool hasLocomotor = false;
	bool hasParasite = false;

	auto checkWeapon = [&maxCapture, &infiniteCapture, &hasTemporal,
		&hasAirstrike, &hasLocomotor, &hasParasite](WeaponTypeClass* pWeaponType)
		{
			if (!pWeaponType)
				return;

			const auto pWH = pWeaponType->Warhead;

			if (pWH->MindControl)
			{
				if (pWeaponType->Damage > maxCapture)
					maxCapture = pWeaponType->Damage;

				if (pWeaponType->InfiniteMindControl)
					infiniteCapture = true;
			}

			if (pWH->Temporal)
				hasTemporal = true;

			if (pWH->Airstrike)
				hasAirstrike = true;

			if (pWH->IsLocomotor)
				hasLocomotor = true;

			if (pWH->Parasite)
				hasParasite = true;
		};

	for (int i = 0; i < TechnoTypeClass::MaxWeapons; i++)
	{
		checkWeapon(pThis->GetWeapon(i)->WeaponType);
	}

	if (maxCapture > 0)
	{
		if (!pCaptureManager)
		{
			// Rebuild a CaptureManager
			pCaptureManager = GameCreate<CaptureManagerClass>(pThis, maxCapture, infiniteCapture);
		}
		else if (pOldTypeExt->Convert_ResetMindControl.Get(RulesExt::Global()->Convert_ResetMindControl))
		{
			if (!infiniteCapture && pCaptureManager->GetControlledCount() > maxCapture)
			{
				// Remove excess nodes.
				for (int i = pCaptureManager->ControlNodes.Count - 1; i >= maxCapture; --i)
				{
					auto const pControlNode = pCaptureManager->ControlNodes.GetItem(i);
					pCaptureManager->FreeUnit(pControlNode->Unit);
				}
			}

			pCaptureManager->MaxControlNodes = maxCapture;
			pCaptureManager->InfiniteMindControl = infiniteCapture;
		}
	}
	else if (pCaptureManager && pOldTypeExt->Convert_ResetMindControl.Get(RulesExt::Global()->Convert_ResetMindControl))
	{
		// Remove CaptureManager completely
		pCaptureManager->FreeAll();
		GameDelete(pCaptureManager);
		pCaptureManager = nullptr;
	}

	if (hasTemporal)
	{
		if (!pTemporalImUsing)
		{
			// Rebuild a TemporalClass
			pTemporalImUsing = GameCreate<TemporalClass>(pThis);
		}
	}
	else if (pTemporalImUsing)
	{
		if (pTemporalImUsing->Target)
		{
			// Free this afflicted man.
			pTemporalImUsing->LetGo();
		}

		// Delete it
		GameDelete(pTemporalImUsing);
		pTemporalImUsing = nullptr;
	}

	if (hasAirstrike && pCurrentType->AirstrikeTeam > 0)
	{
		if (!pAirstrike)
		{
			// Rebuild a AirstrikeClass
			pAirstrike = GameCreate<AirstrikeClass>(pThis);
		}
		else
		{
			// Modify the parameters of AirstrikeClass.
			pAirstrike->AirstrikeTeam = pCurrentType->AirstrikeTeam;
			pAirstrike->EliteAirstrikeTeam = pCurrentType->EliteAirstrikeTeam;
			pAirstrike->AirstrikeTeamType = pCurrentType->AirstrikeTeamType;
			pAirstrike->EliteAirstrikeTeamType = pCurrentType->EliteAirstrikeTeamType;
			pAirstrike->AirstrikeRechargeTime = pCurrentType->AirstrikeRechargeTime;
			pAirstrike->EliteAirstrikeRechargeTime = pCurrentType->EliteAirstrikeRechargeTime;
		}
	}
	else if (pAirstrike)
	{
		pAirstrike->InvalidatePointer(pThis);
		GameDelete(pAirstrike);
		pAirstrike = nullptr;
	}

	if (!hasLocomotor && pThis->LocomotorTarget)
	{
		pThis->ReleaseLocomotor(pThis->Target == pThis->LocomotorTarget);
		pThis->LocomotorTarget->LocomotorSource = nullptr;
		pThis->LocomotorTarget = nullptr;
	}

	if (pOldType->BombSight && !pCurrentType->BombSight)
		BombListClass::Instance.RemoveDetector(pThis);
	else if (!pOldType->BombSight && pCurrentType->BombSight)
		BombListClass::Instance.AddDetector(pThis);

	auto& pParasiteImUsing = pThis->ParasiteImUsing;

	if (hasParasite)
	{
		if (!pParasiteImUsing)
		{
			// Rebuild a ParasiteClass
			pParasiteImUsing = GameCreate<ParasiteClass>(pThis);
		}
	}
	else if (pParasiteImUsing)
	{
		if (pParasiteImUsing->Victim)
		{
			// Release of victims.
			pParasiteImUsing->ExitUnit();
		}

		// Delete it
		GameDelete(pParasiteImUsing);
		pParasiteImUsing = nullptr;
	}

	auto const abs = pThis->WhatAmI();

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

			// FireAngle
			pThis->BarrelFacing.SetCurrent(DirStruct(0x4000 - (pCurrentType->FireAngle << 8)));

			// Reset recoil data
			static_cast<UnitExt*>(this)->InitializeRecoilData();
			{
				auto& turretRecoil = pThis->TurretRecoil.Turret;
				const auto& turretAnimData = pCurrentType->TurretAnimData;
				turretRecoil.Travel = turretAnimData.Travel;
				turretRecoil.CompressFrames = turretAnimData.CompressFrames;
				turretRecoil.RecoverFrames = turretAnimData.RecoverFrames;
				turretRecoil.HoldFrames = turretAnimData.HoldFrames;
				auto& barrelRecoil = pThis->BarrelRecoil.Turret;
				const auto& barrelAnimData = pCurrentType->BarrelAnimData;
				barrelRecoil.Travel = barrelAnimData.Travel;
				barrelRecoil.CompressFrames = barrelAnimData.CompressFrames;
				barrelRecoil.RecoverFrames = barrelAnimData.RecoverFrames;
				barrelRecoil.HoldFrames = barrelAnimData.HoldFrames;
			}
		}
	}

	// handle AutoTargetOwnPosition
	if (pOldTypeExt->AutoTargetOwnPosition && !pNewTypeExt->AutoTargetOwnPosition)
		pThis->SetTarget(nullptr);

	// Clear AlphaImage
	if (const auto pAlphaMap = AresFunctions::AlphaExtMap)
	{
		if (const auto pAlpha = pAlphaMap->get_or_default(pThis))
			GameDelete(pAlpha);
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
		.Process(this->AttackMoveFollowerTempCount)
		//.Process(this->IsOwnerChangeFromRevertOnExit) Temporary flag, does not need to be serialized.
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
