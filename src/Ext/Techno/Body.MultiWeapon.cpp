#include "Body.h"

#include <InfantryClass.h>
#include <HouseClass.h>
#include <OverlayTypeClass.h>
#include <TerrainClass.h>
#include <SpawnManagerClass.h>
#include <SlaveManagerClass.h>
#include <AirstrikeClass.h>

#include <Ext/BulletType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/EnumFunctions.h>

// Powered by ststl-s
void TechnoExt::FixManagers(TechnoClass* const pThis)
{
	TechnoTypeClass* pType = pThis->GetTechnoType();
	FootClass* pFoot = nullptr;

	if (pThis->WhatAmI() != AbstractType::Building)
	{
		pFoot = static_cast<FootClass*>(pThis);

		if (!pType->CanDisguise || (!pFoot->Disguise && pType->PermaDisguise))
			pThis->ClearDisguise();

		if (pThis->Passengers.NumPassengers > 0)
		{
			if (const auto pPass = pFoot->Passengers.GetFirstPassenger())
			{
				FootClass* pFirstPassenger = pPass;

				while (true)
				{
					if (pType->OpenTopped)
					{
						pFoot->EnteredOpenTopped(pFirstPassenger);
					}
					else
					{
						// Lose target & destination
						pFirstPassenger->SetTarget(nullptr);
						pFirstPassenger->SetCurrentWeaponStage(0);
						pFirstPassenger->AbortMotion();
						pFoot->ExitedOpenTopped(pFirstPassenger);

						// OpenTopped adds passengers to logic layer when enabled. Under normal conditions this does not need to be removed since
						// OpenTopped state does not change while passengers are still in transport but in case of type conversion that can happen.
						LogicClass::Instance.RemoveObject(pFirstPassenger);
					}

					pFirstPassenger->Transporter = pFoot;
					const auto pNextPassenger = abstract_cast<FootClass*>(pFirstPassenger->NextObject);

					if (pNextPassenger)
						pFirstPassenger = pNextPassenger;
					else
						break;
				}

				if (pType->Gunner)
					pFoot->ReceiveGunner(pFirstPassenger);
				else
					pFoot->RemoveGunner(pFirstPassenger);
			}
		}
	}

	const auto pOwner = pThis->Owner;
	auto& pSlaveManager = pThis->SlaveManager;

	if (pType->Enslaves && pType->SlavesNumber > 0)
	{
		if (!pSlaveManager || pSlaveManager->SlaveType != pType->Enslaves)
		{
			if (pSlaveManager && pSlaveManager->SlaveType != pType->Enslaves)
			{
				pSlaveManager->Killed(nullptr);
				GameDelete(pSlaveManager);
			}

			pSlaveManager = GameCreate<SlaveManagerClass>(pThis, pType->Enslaves, pType->SlavesNumber, pType->SlaveRegenRate, pType->SlaveReloadRate);
		}
		else if (pSlaveManager->SlaveCount != pType->SlavesNumber)
		{
			if (pSlaveManager->SlaveCount < pType->SlavesNumber)
			{
				int count = pType->SlavesNumber - pSlaveManager->SlaveCount;

				for (int index = 0; index < count; index++)
				{
					if (auto pSlaveNode = GameCreate<SlaveManagerClass::SlaveControl>())
					{
						pSlaveNode->Slave = nullptr;
						pSlaveNode->State = SlaveControlStatus::Dead;
						pSlaveNode->RespawnTimer.Start(pType->SlaveRegenRate);
						pSlaveManager->SlaveNodes.AddItem(pSlaveNode);
					}
				}
			}
			else
			{
				for (int index = pSlaveManager->SlaveCount - 1; index >= pType->SlavesNumber; --index)
				{
					if (auto pSlaveNode = pSlaveManager->SlaveNodes.GetItem(index))
					{
						const auto pSlave = pSlaveNode->Slave;

						if (pSlave && pSlaveNode->State != SlaveControlStatus::Dead)
						{
							if (pSlave->InLimbo)
							{
								pSlave->RegisterDestruction(pThis);
								pSlave->UnInit();
								pSlaveManager->LostSlave(pSlave);
							}
							else
							{
								pSlave->ReceiveDamage(&pSlave->Health, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, pOwner);
							}
						}

						GameDelete(pSlaveNode);
					}

					pSlaveManager->SlaveNodes.RemoveItem(index);
				}
			}

			pSlaveManager->SlaveCount = pType->SlavesNumber;
		}
	}
	else if (pSlaveManager)
	{
		pSlaveManager->Killed(nullptr);
		GameDelete(pSlaveManager);
		pSlaveManager = nullptr;
	}

	auto& pSpawnManager = pThis->SpawnManager;
	if (pType->Spawns && pType->SpawnsNumber > 0)
	{
		if (!pSpawnManager || pType->Spawns != pSpawnManager->SpawnType)
		{
			if (pSpawnManager && pType->Spawns != pSpawnManager->SpawnType)
			{
				pSpawnManager->KillNodes();
				GameDelete(pSpawnManager);
			}

			pSpawnManager = GameCreate<SpawnManagerClass>(pThis, pType->Spawns, pType->SpawnsNumber, pType->SpawnRegenRate, pType->SpawnReloadRate);
		}
		else if (pSpawnManager->SpawnCount != pType->SpawnsNumber)
		{
			if (pSpawnManager->SpawnCount < pType->SpawnsNumber)
			{
				int count = pType->SpawnsNumber - pSpawnManager->SpawnCount;

				for (int index = 0; index < count; index++)
				{
					if (auto pSpawnNode = GameCreate<SpawnControl>())
					{
						pSpawnNode->Unit = nullptr;
						pSpawnNode->Status = SpawnNodeStatus::Dead;
						pSpawnNode->SpawnTimer.Start(pType->SpawnRegenRate);
						pSpawnNode->IsSpawnMissile = false;
						pSpawnManager->SpawnedNodes.AddItem(pSpawnNode);
					}
				}
			}
			else
			{
				for (int index = pSpawnManager->SpawnCount - 1; index >= pType->SpawnsNumber; --index)
				{
					if (auto pSpawnNode = pSpawnManager->SpawnedNodes.GetItem(index))
					{
						const auto pAircraft = pSpawnNode->Unit;
						const auto pStatus = pSpawnNode->Status;

						if (pAircraft && pStatus != SpawnNodeStatus::Dead)
						{
							if (pAircraft->InLimbo || pStatus == SpawnNodeStatus::Idle ||
								pStatus == SpawnNodeStatus::Reloading || pStatus == SpawnNodeStatus::TakeOff)
							{
								pAircraft->RegisterDestruction(pThis);
								pAircraft->UnInit();
							}
							else
							{
								pAircraft->ReceiveDamage(&pAircraft->Health, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, pOwner);
							}

						}

						pSpawnManager->UnlinkPointer(pAircraft);
						GameDelete(pSpawnNode);
					}

					pSpawnManager->SpawnedNodes.RemoveItem(index);
				}
			}

			pSpawnManager->SpawnCount = pType->SpawnsNumber;
		}
	}
	else if (pSpawnManager)
	{
		pSpawnManager->KillNodes();
		GameDelete(pSpawnManager);
		pSpawnManager = nullptr;
	}

	auto& pAirstrike = pThis->Airstrike;
	if (pType->AirstrikeTeam > 0)
	{
		if (!pAirstrike)
		{
			pAirstrike = GameCreate<AirstrikeClass>(pThis);
		}
		else
		{
			pAirstrike->AirstrikeTeam = pType->AirstrikeTeam;
			pAirstrike->EliteAirstrikeTeam = pType->EliteAirstrikeTeam;
			pAirstrike->AirstrikeTeamType = pType->AirstrikeTeamType;
			pAirstrike->EliteAirstrikeTeamType = pType->EliteAirstrikeTeamType;
			pAirstrike->AirstrikeRechargeTime = pType->AirstrikeRechargeTime;
			pAirstrike->EliteAirstrikeRechargeTime = pType->EliteAirstrikeRechargeTime;
		}
	}
	else if (pAirstrike)
	{
		pAirstrike->StartMission(nullptr);
		GameDelete(pAirstrike);
		pAirstrike = nullptr;
	}

	std::vector<WeaponTypeClass*> vWeapons;
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	bool isElite = pThis->Veterancy.IsElite();

	if (pType->WeaponCount > 0 && (pType->HasMultipleTurrets() || pTypeExt->MultiWeapon.Get()))
	{
		for (int index = 0; index < TechnoTypeClass::MaxWeapons; index++)
		{
			const auto pWeaponType = pType->GetWeapon(index, isElite).WeaponType;

			if (pWeaponType)
				vWeapons.push_back(pWeaponType);
		}
	}
	else
	{
		const auto primary = pType->GetWeapon(0, isElite).WeaponType;
		const auto secondary = pType->GetWeapon(1, isElite).WeaponType;

		if (primary)
			vWeapons.push_back(primary);

		if (secondary)
			vWeapons.push_back(secondary);
	}

	int maxCapture = 0;
	bool infiniteCapture = false;
	bool hasTemporal = false;
	bool hasParasite = false;

	if (!vWeapons.empty())
	{
		for (const auto pWeaponType : vWeapons)
		{
			const auto pWH = pWeaponType->Warhead;

			if (pWH->MindControl)
			{
				if (pWeaponType->Damage > maxCapture)
					maxCapture = pWeaponType->Damage;

				if (pWeaponType->InfiniteMindControl)
					infiniteCapture = true;
			}

			if (pWH->Temporal)
			{
				hasTemporal = true;
			}

			if (pWH->Parasite)
			{
				hasParasite = true;
			}
		}
	}

	auto& pCaptureManager = pThis->CaptureManager;

	if (maxCapture > 0)
	{
		if (!pCaptureManager)
		{
			pCaptureManager = GameCreate<CaptureManagerClass>(pThis, maxCapture, infiniteCapture);
		}
		else
		{
			if (pCaptureManager->ControlNodes.Count > maxCapture)
			{
				for (int index = pCaptureManager->ControlNodes.Count - 1; index >= maxCapture; --index)
				{
					auto pControlNode = pCaptureManager->ControlNodes.GetItem(index);
					pCaptureManager->FreeUnit(pControlNode->Unit);
				}
			}

			pCaptureManager->MaxControlNodes = maxCapture;
			pCaptureManager->InfiniteMindControl = infiniteCapture;
		}
	}
	else if (pCaptureManager)
	{
		pCaptureManager->FreeAll();
		GameDelete(pCaptureManager);
		pCaptureManager = nullptr;
	}

	auto& pTemporalImUsing = pThis->TemporalImUsing;
	if (hasTemporal)
	{
		if (!pTemporalImUsing)
			pTemporalImUsing = GameCreate<TemporalClass>(pThis);
	}
	else if (pTemporalImUsing)
	{
		if (pTemporalImUsing->Target)
			pTemporalImUsing->Detach();

		GameDelete(pTemporalImUsing);
		pTemporalImUsing = nullptr;
	}

	if (pFoot)
	{
		auto& pParasiteImUsing = pFoot->ParasiteImUsing;
		if (hasParasite)
		{
			if (!pParasiteImUsing)
				pParasiteImUsing = GameCreate<ParasiteClass>(pFoot);
		}
		else if (pParasiteImUsing)
		{
			if (pParasiteImUsing->Victim)
				pParasiteImUsing->ExitUnit();

			GameDelete(pParasiteImUsing);
			pParasiteImUsing = nullptr;
		}
	}
}

bool TechnoExt::CheckMultiWeapon(TechnoClass* const pThis, AbstractClass* const pTarget, WeaponTypeClass* const pWeaponType)
{
	if (!pThis || !pTarget)
		return false;

	if (!pWeaponType || pWeaponType->NeverUse)
		return false;

	if (pThis->InOpenToppedTransport && !pWeaponType->FireInTransport)
		return false;

	const auto pBulletType = pWeaponType->Projectile;
	const auto pWhatAmI = pTarget->WhatAmI();
	const bool isBuilding = pWhatAmI != AbstractType::Building;
	const auto pWH = pWeaponType->Warhead;
	const auto pOwner = pThis->Owner;

	if (pTarget->IsInAir())
	{
		if (!pBulletType->AA)
			return false;

		if (pWH->Airstrike)
			return false;
	}
	else
	{
		const auto pBulletTypeExt = BulletTypeExt::ExtMap.Find(pBulletType);

		if (pBulletTypeExt->AAOnly.Get())
		{
			return false;
		}
		else if(pWH->ElectricAssault)
		{
			if (isBuilding)
				return false;

			const auto pBuilding = static_cast<BuildingClass*>(pTarget);

			if (!pOwner->IsAlliedWith(pBuilding->Owner) ||
				!pBuilding->Type || !pBuilding->Type->Overpowerable)
				return false;
		}
		else if (pWH->IsLocomotor)
		{
			if (isBuilding)
				return false;
		}
	}

	if (pTarget->AbstractFlags & AbstractFlags::Techno)
	{
		TechnoClass* pTechno = static_cast<TechnoClass*>(pTarget);

		if (pTechno->Health <= 0 || !pTechno->IsAlive)
			return false;

		if (pTechno->AttachedBomb)
		{
			if (pWH->IvanBomb)
				return false;
		}
		else if (pWH->BombDisarm)
		{
			return false;
		}

		const auto pTechnoType = pTechno->GetTechnoType();

		if (pWH->MindControl && (pTechnoType->ImmuneToPsionics ||
			pTechno->IsMindControlled() || pOwner == pTechno->Owner))
			return false;

		if (pWH->Parasite && (isBuilding ||
			static_cast<FootClass*>(pTechno)->ParasiteEatingMe))
			return false;

		if (!pWH->Temporal && pTechno->BeingWarpedOut)
			return false;

		if (pWeaponType->DrainWeapon && (!pTechnoType->Drainable ||
			(pTechno->DrainingMe || pOwner->IsAlliedWith(pTechno->Owner))))
			return false;

		if (const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeaponType))
		{
			if (!pWeaponExt->HasRequiredAttachedEffects(pTechno, pThis))
				return false;
		}

		const auto pTargetExt = TechnoExt::ExtMap.Find(pTechno);
		const auto pShield = pTargetExt->Shield.get();

		if (pShield && pShield->IsActive() &&
			!pShield->CanBeTargeted(pWeaponType))
		{
			return false;
		}
		else if (GeneralUtils::GetWarheadVersusArmor(pWH, pTechno->GetTechnoType()->Armor) == 0.0)
		{
			return false;
		}
	}
	else
	{
		if (pWhatAmI == AbstractType::Terrain)
		{
			if (!pWH->Wood)
				return false;
		}
		else if (pWhatAmI == AbstractType::Cell)
		{
			const auto pCell = static_cast<CellClass*>(pTarget);

			if (pCell && pCell->OverlayTypeIndex >= 0)
			{
				auto overlayType = OverlayTypeClass::Array.GetItem(pCell->OverlayTypeIndex);

				if (overlayType->Wall && !pWH->Wall)
					return false;
			}
		}
	}

	return true;
}
