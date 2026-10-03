#include "AttachmentClass.h"

#include <Dir.h>
#include <BuildingClass.h>
#include <BulletClass.h>
#include <BulletTypeClass.h>
#include <WarheadTypeClass.h>

#include <ObjBase.h>

#include <Ext/Techno/Body.h>
#include <Locomotion/AttachmentLocomotionClass.h>

std::vector<AttachmentClass*> AttachmentClass::Array;

AttachmentTypeClass* AttachmentClass::GetType()
{
	return AttachmentTypeClass::Array[this->Data->Type].get();
}

TechnoTypeClass* AttachmentClass::GetChildType()
{
	return this->Data->TechnoType.isset()
		? TechnoTypeClass::Array[this->Data->TechnoType]
		: nullptr;
}

CoordStruct AttachmentClass::GetChildLocation()
{
	auto& flh = this->Data->FLH.Get();
	return TechnoExt::GetFLHAbsoluteCoords(this->Parent, flh, this->Data->IsOnTurret);
}

AttachmentClass::~AttachmentClass()
{
	// clean up non-owning references
	if (this->Child)
	{
		auto const& pChildExt = TechnoExt::ExtMap.Find(Child);
		pChildExt->ParentAttachment = nullptr;
	}

	auto position = std::find(Array.begin(), Array.end(), this);
	if (position != Array.end())
		Array.erase(position);
}

void AttachmentClass::OnCreated()
{
	if (this->Child)
		return;

	if (this->GetType()->RespawnAtCreation)
		this->CreateChild();
}

void AttachmentClass::CreateChild()
{
	if (auto const pChildType = this->GetChildType())
	{
		if (pChildType->WhatAmI() != AbstractType::UnitType)
			return;

		if (const auto pTechno = static_cast<TechnoClass*>(pChildType->CreateObject(this->Parent->Owner)))
		{
			this->AttachChild(pTechno);
		}
		else
		{
			Debug::Log("[" __FUNCTION__ "] Failed to create child %s of parent %s!\n",
				pChildType->ID, this->Parent->GetTechnoType()->ID);
		}
	}
}

void AttachmentClass::AI()
{
	AttachmentTypeClass* pType = this->GetType();

	if (!this->Child)
	{
		if (pType->RespawnDelay == 0)
		{
			this->CreateChild();
		}
		else if (pType->RespawnDelay > 0)
		{
			if (!this->RespawnTimer.HasStarted())
			{
				this->RespawnTimer.Start(pType->RespawnDelay);
			}
			else if (this->RespawnTimer.Completed())
			{
				this->CreateChild();
				this->RespawnTimer.Stop();
			}
		}
	}

	if (this->Child)
	{
		bool parentInLimbo = this->Parent->InLimbo;

		if (auto const pBuilding = abstract_cast<BuildingClass*>(this->Parent))
		{
			if (pBuilding->GetCurrentMission() == Mission::Construction
				|| pBuilding->BState == static_cast<int>(BStateType::Construction)
				|| pBuilding->GetCurrentMission() == Mission::Selling)
			{
				parentInLimbo = true;
			}
		}

		if (this->Child->InLimbo && !parentInLimbo)
			this->Unlimbo();
		else if (!this->Child->InLimbo && parentInLimbo)
			this->Limbo();

		if (!this->Child || this->Child->InLimbo)
			return;

		this->Child->SetLocation(this->GetChildLocation());

		const bool parentHasTurret = this->Parent->GetTechnoType()->Turret;
		DirStruct childDir = (this->Data->IsOnTurret && parentHasTurret)
			? this->Parent->SecondaryFacing.Current() : this->Parent->PrimaryFacing.Current();

		childDir.Raw += DirStruct(this->Data->RotationAdjust).Raw; // overflow = free modulo for rotation

		this->Child->PrimaryFacing.SetCurrent(childDir);

		if (this->Child->GetTechnoType()->Turret && !this->Child->Target)
			this->Child->SecondaryFacing.SetCurrent(childDir);

		FootClass* pParentAsFoot = abstract_cast<FootClass*>(this->Parent);
		FootClass* pChildAsFoot = abstract_cast<FootClass*>(this->Child);
		if (pParentAsFoot && pChildAsFoot)
			pChildAsFoot->TubeIndex = pParentAsFoot->TubeIndex;

		if (this->Parent)
		{
			const bool isParentDying = !this->Parent->IsAlive
				|| this->Parent->Health <= 0
				|| this->Parent->IsCrashing
				|| this->Parent->InLimbo;

			if (!isParentDying)
			{
				if (this->Parent->Target)
					this->LastValidParentTarget = this->Parent->Target;

				if (pParentAsFoot)
				{
					AbstractClass* pDest = pParentAsFoot->Destination ? pParentAsFoot->Destination : pParentAsFoot->MegaDestination;
					if (!pDest)
						pDest = pParentAsFoot->LastDestination;

					if (pDest)
						this->LastValidParentDestination = pDest;
				}

				auto const parentMission = this->Parent->GetCurrentMission();
				if (parentMission == Mission::Move || parentMission == Mission::AttackMove || parentMission == Mission::QMove || parentMission == Mission::Patrol || parentMission == Mission::Hunt || parentMission == Mission::Attack)
					this->LastValidParentMission = parentMission;
			}
			else if (!pType->InheritDestruction)
			{
				this->Destroy(nullptr);
				return;
			}
		}

		if (pType->InheritStateEffects)
		{
			this->Child->IsFallingDown = this->Parent->IsFallingDown;
			this->Child->WasFallingDown = this->Parent->WasFallingDown;
			if (this->Child->CloakState != this->Parent->CloakState)
			{
				const auto oldChildCloakState = this->Child->CloakState;
				this->Child->CloakState = this->Parent->CloakState;
				this->Child->Mark(MarkType::Change);

				if ((this->Child->CloakState == CloakState::Cloaking || this->Child->CloakState == CloakState::Cloaked)
					&& (oldChildCloakState == CloakState::Uncloaked || oldChildCloakState == CloakState::Uncloaking))
				{
					reinterpret_cast<void(__thiscall*)(ObjectClass*, bool)>(0x5F5280)(this->Child, false);
				}
			}
			this->Child->CloakProgress = this->Parent->CloakProgress;
			this->Child->WarpingOut = this->Parent->WarpingOut;
			this->Child->unknown_280 = this->Parent->unknown_280; // sth related to teleport
			this->Child->BeingWarpedOut = this->Parent->BeingWarpedOut;
			this->Child->Deactivated = this->Parent->Deactivated;
			this->Child->IsImmobilized = this->Parent->IsImmobilized;
			//this->Child->Flash(this->Parent->Flashing.DurationRemaining);

			this->Child->IronCurtainTimer = this->Parent->IronCurtainTimer;
			this->Child->IdleActionTimer = this->Parent->IdleActionTimer;
			this->Child->IronTintTimer = this->Parent->IronTintTimer;
			this->Child->ForceShielded = this->Parent->ForceShielded;
			this->Child->CloakDelayTimer = this->Parent->CloakDelayTimer;
			this->Child->ChronoLockRemaining = this->Parent->ChronoLockRemaining;
			this->Child->Berzerk = this->Parent->Berzerk;
			this->Child->BerzerkDurationLeft = this->Parent->BerzerkDurationLeft;
			this->Child->ChronoWarpedByHouse = this->Parent->ChronoWarpedByHouse;
			this->Child->EMPLockRemaining = this->Parent->EMPLockRemaining;
			this->Child->ShouldLoseTargetNow = this->Parent->ShouldLoseTargetNow;
		}

		if (pType->InheritOwner)
			this->Child->SetOwningHouse(this->Parent->GetOwningHouse(), false);

		if (pType->InheritTarget)
		{
			AbstractClass* pParentTarget = this->Parent->Target;
			bool isTargetValid = false;

			if (pParentTarget && pParentTarget != this->Child)
			{
				auto const pTargetTechno = abstract_cast<TechnoClass*>(pParentTarget);
				if (!pTargetTechno || !TechnoExt::AreRelatives(this->Child, pTargetTechno))
				{
					if (auto const pTargetObj = abstract_cast<ObjectClass*>(pParentTarget))
						isTargetValid = pTargetObj->IsAlive && !pTargetObj->InLimbo;
					else
						isTargetValid = true;
				}
			}

			if (isTargetValid)
			{
				bool canAttackTarget = false;
				for (int i = 0; i < 2; ++i)
				{
					auto const pWeaponStruct = this->Child->GetWeapon(i);
					if (pWeaponStruct && pWeaponStruct->WeaponType)
					{
						auto const err = this->Child->GetFireError(pParentTarget, i, false);
						if (err == FireError::OK || err == FireError::FACING || err == FireError::REARM || err == FireError::ROTATING)
						{
							canAttackTarget = true;
							break;
						}
					}
				}

				if (canAttackTarget)
				{
					if (this->Child->Target != pParentTarget)
						this->Child->SetTarget(pParentTarget);

					if (this->Child->GetCurrentMission() != Mission::Attack)
						this->Child->QueueMission(Mission::Attack, false);
				}
				else if (this->Parent->GetCurrentMission() == Mission::Attack)
				{
					if (this->Child->Target != pParentTarget)
						this->Child->SetTarget(pParentTarget);
				}
			}
			else if (this->Child->Target && this->Parent->GetCurrentMission() != Mission::Attack)
			{
				const bool isParentDying = !this->Parent->IsAlive
					|| this->Parent->Health <= 0
					|| this->Parent->IsCrashing
					|| this->Parent->InLimbo;

				if (!isParentDying && (this->Child->Target == pParentTarget || !pParentTarget))
				{
					this->Child->SetTarget(nullptr);
					if (this->Child->GetCurrentMission() == Mission::Attack)
						this->Child->QueueMission(Mission::Guard, false);
				}
			}
		}
	}
}

// Called in Kill_Cargo, handles logics for parent destruction on children
void AttachmentClass::Destroy(TechnoClass* pSource)
{
	if (this->Child)
	{
		auto const pChild = this->Child;
		this->Child = nullptr;

		auto const pChildExt = TechnoExt::ExtMap.Find(pChild);
		auto pType = this->GetType();

		if (pType->DestructionWeapon_Child.isset())
			TechnoExt::FireWeaponAtSelf(pChild, pType->DestructionWeapon_Child);

		auto const pChildAsFoot = abstract_cast<FootClass*>(pChild);
		if (pChildAsFoot)
			LocomotionClass::End_Piggyback(pChildAsFoot->Locomotor);

		// Restore original owner if InheritOwner was active
		if (pType->InheritOwner)
		{
			HouseClass* targetOwner = pChild->GetOriginalOwner() ? pChild->GetOriginalOwner() : (this->Parent ? this->Parent->GetOriginalOwner() : nullptr);
			if (targetOwner)
				pChild->SetOwningHouse(targetOwner, false);
		}

		// Clean up state effects if InheritStateEffects was active
		if (pType->InheritStateEffects)
		{
			if (pChild->CloakState != CloakState::Uncloaked && !pChild->GetTechnoType()->Cloakable)
				pChild->Uncloak(false);

			pChild->ForceShielded = false;
		}

		// Synchronize attack target with parent
		AbstractClass* pTargetToInherit = nullptr;
		if (pType->InheritTarget)
		{
			if (this->Parent && this->Parent->Target && this->Parent->Target != pChild)
				pTargetToInherit = this->Parent->Target;
			else if (pChild->Target && pChild->Target != pChild)
				pTargetToInherit = pChild->Target;
			else if (this->LastValidParentTarget && this->LastValidParentTarget != pChild)
				pTargetToInherit = this->LastValidParentTarget;
			else if (this->Parent && this->Parent->LastTarget && this->Parent->LastTarget != pChild)
				pTargetToInherit = this->Parent->LastTarget;
			else if (pChild->LastTarget && pChild->LastTarget != pChild)
				pTargetToInherit = pChild->LastTarget;

			if (pTargetToInherit)
			{
				bool isTargetValid = false;
				if (auto const pTargetObj = abstract_cast<ObjectClass*>(pTargetToInherit))
					isTargetValid = pTargetObj->IsAlive && !pTargetObj->InLimbo;
				else
					isTargetValid = true;

				if (isTargetValid)
					pChild->SetTarget(pTargetToInherit);
				else
					pTargetToInherit = nullptr;
			}
		}

		// Transfer destination and movement commands from parent
		AbstractClass* pDestToInherit = nullptr;
		if (pType->InheritCommands)
		{
			if (auto const pParentFoot = abstract_cast<FootClass*>(this->Parent))
			{
				if (pParentFoot->Destination)
					pDestToInherit = pParentFoot->Destination;
				else if (pParentFoot->MegaDestination)
					pDestToInherit = pParentFoot->MegaDestination;
			}

			if (!pDestToInherit && this->LastValidParentDestination)
				pDestToInherit = this->LastValidParentDestination;

			if (!pDestToInherit && this->Parent)
			{
				if (auto const pParentFoot = abstract_cast<FootClass*>(this->Parent))
				{
					if (pParentFoot->LastDestination)
						pDestToInherit = pParentFoot->LastDestination;
				}
			}

			if (!pDestToInherit && pChildAsFoot)
			{
				if (pChildAsFoot->Destination)
					pDestToInherit = pChildAsFoot->Destination;
				else if (pChildAsFoot->MegaDestination)
					pDestToInherit = pChildAsFoot->MegaDestination;
				else if (pChildAsFoot->LastDestination)
					pDestToInherit = pChildAsFoot->LastDestination;
			}

			if (pDestToInherit && pChildAsFoot)
				pChildAsFoot->SetDestination(pDestToInherit, true);

			if (pChildAsFoot && this->Parent)
			{
				if (auto const pParentFoot = abstract_cast<FootClass*>(this->Parent))
				{
					if (pParentFoot->MegaMission == Mission::AttackMove)
					{
						pChildAsFoot->MegaMission = pParentFoot->MegaMission;
						pChildAsFoot->MegaDestination = pParentFoot->MegaDestination;
						pChildAsFoot->MegaTarget = pParentFoot->MegaTarget;
						pChildAsFoot->HaveAttackMoveTarget = pParentFoot->HaveAttackMoveTarget;
					}
				}
			}
		}

		// Determine child mission upon detachment or parent destruction
		Mission detachmentMission = Mission::Guard;
		if (pType->ParentDestructionMission.isset())
			detachmentMission = pType->ParentDestructionMission.Get();
		else if (pType->InheritTarget && pTargetToInherit)
			detachmentMission = Mission::Attack;
		else if (pType->InheritCommands)
		{
			Mission candidateMission = Mission::None;
			if (this->Parent)
			{
				auto const parentMission = this->Parent->GetCurrentMission();
				if (parentMission == Mission::Move || parentMission == Mission::AttackMove || parentMission == Mission::QMove || parentMission == Mission::Patrol || parentMission == Mission::Hunt)
					candidateMission = parentMission;
			}

			if (candidateMission == Mission::None && this->LastValidParentMission != Mission::None && this->LastValidParentMission != Mission::Attack)
				candidateMission = this->LastValidParentMission;

			if (candidateMission == Mission::None && pDestToInherit)
				candidateMission = Mission::Move;

			if (candidateMission != Mission::None)
				detachmentMission = candidateMission;
		}

		pChild->QueueMission(detachmentMission, false);

		const bool isAirborneChild = pChild->WhatAmI() == AbstractType::Aircraft
			|| pChild->GetTechnoType()->ConsideredAircraft
			|| pChild->GetTechnoType()->JumpJet;

		CellClass* pCell = MapClass::Instance.GetCellAt(pChild->Location);
		if (!pCell)
			pCell = pChild->GetCell();

		const CoordStruct groundLoc = pCell ? pCell->GetCoordsWithBridge() : CoordStruct::Empty;
		const bool isParentAirborne = (this->Parent && (this->Parent->IsInAir() || this->Parent->Location.Z > groundLoc.Z + 64 || this->Parent->GetHeight() > 0));
		const bool isChildElevated = pChild->Location.Z > groundLoc.Z + 64 || pChild->GetHeight() > 0 || isParentAirborne;

		if (isChildElevated && !isAirborneChild)
		{
			if (pChildExt)
			{
				pChildExt->FallingInheritedTarget = pTargetToInherit;
				pChildExt->FallingInheritedDestination = pDestToInherit;
				pChildExt->FallingInheritedMission = detachmentMission;
				if (!pType->InheritDestruction)
					pChildExt->OnParachuted = true;
			}

			pChild->DropAsBomb();
		}
		else
		{
			if (pType->InheritDestruction && pChild->IsAlive)
				TechnoExt::Kill(pChild, pSource);
			else if (pChild->IsAlive && !pChild->InLimbo)
				pChild->ForceMission(detachmentMission);
		}

		if (pChildExt)
			pChildExt->ParentAttachment = nullptr;

		if (this->Parent && !this->Parent->InLimbo)
		{
			if (auto const pParentUnit = abstract_cast<UnitClass*>(this->Parent))
				pParentUnit->MarkAllOccupationBits(this->Parent->Location);
		}
	}
}

void AttachmentClass::ChildDestroyed()
{
	if (this->Child)
	{
		auto const pChild = this->Child;
		this->Child = nullptr;

		AttachmentTypeClass* pType = this->GetType();
		if (pType->DestructionWeapon_Parent.isset())
			TechnoExt::FireWeaponAtSelf(this->Parent, pType->DestructionWeapon_Parent);

		if (auto const pChildAsFoot = abstract_cast<FootClass*>(pChild))
			LocomotionClass::End_Piggyback(pChildAsFoot->Locomotor);

		if (auto const pChildExt = TechnoExt::ExtMap.Find(pChild))
			pChildExt->ParentAttachment = nullptr;

		if (this->Parent && !this->Parent->InLimbo)
		{
			if (auto const pParentUnit = abstract_cast<UnitClass*>(this->Parent))
				pParentUnit->MarkAllOccupationBits(this->Parent->Location);
		}
	}
}

void AttachmentClass::Unlimbo()
{
	if (this->Child)
	{
		CoordStruct childCoord = TechnoExt::GetFLHAbsoluteCoords(
			this->Parent, this->Data->FLH, this->Data->IsOnTurret);

		const bool parentHasTurret = this->Parent->GetTechnoType()->Turret;
		DirStruct childDir = (this->Data->IsOnTurret && parentHasTurret)
			? this->Parent->SecondaryFacing.Current() : this->Parent->PrimaryFacing.Current();

		childDir.Raw += DirStruct(this->Data->RotationAdjust).Raw; // overflow = free modulo for rotation

		++Unsorted::ScenarioInit;
		this->Child->Unlimbo(childCoord, childDir.GetDir());
		--Unsorted::ScenarioInit;

		if (this->Child->GetTechnoType()->Turret)
			this->Child->SecondaryFacing.SetCurrent(childDir);
	}
}

void AttachmentClass::Limbo()
{
	if (this->Child)
		this->Child->Limbo();
}

bool AttachmentClass::AttachChild(TechnoClass* pChild)
{
	if (this->Child)
		return false;

	if (pChild->WhatAmI() != AbstractType::Unit)
		return false;

	if (auto const pChildAsFoot = abstract_cast<FootClass*>(pChild))
	{
		if (IPersistPtr pLocoPersist = pChildAsFoot->Locomotor)
		{
			CLSID locoCLSID { };
			if (SUCCEEDED(pLocoPersist->GetClassID(&locoCLSID))
				&& locoCLSID != __uuidof(AttachmentLocomotionClass))
			{
				LocomotionClass::ChangeLocomotorTo(pChildAsFoot,
					__uuidof(AttachmentLocomotionClass));
			}
		}
	}

	this->AttachChildCore(pChild);

	// bandaid for jitterless drawing. TODO fix properly
	// this->Child->GetTechnoType()->DisableVoxelCache = true;
	// this->Child->GetTechnoType()->DisableShadowCache = true;

	AttachmentTypeClass* pType = this->GetType();

	if (pType->InheritOwner)
	{
		if (auto pController = this->Child->MindControlledBy)
			pController->CaptureManager->FreeUnit(this->Child);
	}

	return true;
}

bool AttachmentClass::DetachChild()
{
	if (this->Child)
	{
		auto const pChild = this->Child;
		AttachmentTypeClass* pType = this->GetType();

		auto const pChildAsFoot = abstract_cast<FootClass*>(pChild);
		if (pChildAsFoot)
			LocomotionClass::End_Piggyback(pChildAsFoot->Locomotor);

		if (!pChild->InLimbo)
		{
			// Synchronize attack target with parent
			AbstractClass* pTargetToInherit = nullptr;
			if (pType->InheritTarget)
			{
				if (this->Parent && this->Parent->Target && this->Parent->Target != pChild)
					pTargetToInherit = this->Parent->Target;
				else if (pChild->Target && pChild->Target != pChild)
					pTargetToInherit = pChild->Target;
				else if (this->LastValidParentTarget && this->LastValidParentTarget != pChild)
					pTargetToInherit = this->LastValidParentTarget;
				else if (this->Parent && this->Parent->LastTarget && this->Parent->LastTarget != pChild)
					pTargetToInherit = this->Parent->LastTarget;
				else if (pChild->LastTarget && pChild->LastTarget != pChild)
					pTargetToInherit = pChild->LastTarget;

				if (pTargetToInherit)
				{
					bool isTargetValid = false;
					if (auto const pTargetObj = abstract_cast<ObjectClass*>(pTargetToInherit))
						isTargetValid = pTargetObj->IsAlive && !pTargetObj->InLimbo;
					else
						isTargetValid = true;

					if (isTargetValid)
						pChild->SetTarget(pTargetToInherit);
					else
						pTargetToInherit = nullptr;
				}
			}

			// Transfer destination and movement commands from parent
			AbstractClass* pDestToInherit = nullptr;
			if (pType->InheritCommands)
			{
				if (auto const pParentFoot = abstract_cast<FootClass*>(this->Parent))
				{
					if (pParentFoot->Destination)
						pDestToInherit = pParentFoot->Destination;
					else if (pParentFoot->MegaDestination)
						pDestToInherit = pParentFoot->MegaDestination;
				}

				if (!pDestToInherit && this->LastValidParentDestination)
					pDestToInherit = this->LastValidParentDestination;

				if (!pDestToInherit && this->Parent)
				{
					if (auto const pParentFoot = abstract_cast<FootClass*>(this->Parent))
					{
						if (pParentFoot->LastDestination)
							pDestToInherit = pParentFoot->LastDestination;
					}
				}

				if (!pDestToInherit && pChildAsFoot)
				{
					if (pChildAsFoot->Destination)
						pDestToInherit = pChildAsFoot->Destination;
					else if (pChildAsFoot->MegaDestination)
						pDestToInherit = pChildAsFoot->MegaDestination;
					else if (pChildAsFoot->LastDestination)
						pDestToInherit = pChildAsFoot->LastDestination;
				}

				if (pDestToInherit && pChildAsFoot)
					pChildAsFoot->SetDestination(pDestToInherit, true);

				if (pChildAsFoot && this->Parent)
				{
					if (auto const pParentFoot = abstract_cast<FootClass*>(this->Parent))
					{
						if (pParentFoot->MegaMission == Mission::AttackMove)
						{
							pChildAsFoot->MegaMission = pParentFoot->MegaMission;
							pChildAsFoot->MegaDestination = pParentFoot->MegaDestination;
							pChildAsFoot->MegaTarget = pParentFoot->MegaTarget;
							pChildAsFoot->HaveAttackMoveTarget = pParentFoot->HaveAttackMoveTarget;
						}
					}
				}
			}

			// Determine child mission upon detachment
			Mission detachmentMission = Mission::Guard;
			if (pType->ParentDetachmentMission.isset())
				detachmentMission = pType->ParentDetachmentMission.Get();
			else if (pType->InheritTarget && pTargetToInherit)
				detachmentMission = Mission::Attack;
			else if (pType->InheritCommands)
			{
				Mission candidateMission = Mission::None;
				if (this->Parent)
				{
					auto const parentMission = this->Parent->GetCurrentMission();
					if (parentMission == Mission::Move || parentMission == Mission::AttackMove || parentMission == Mission::QMove || parentMission == Mission::Patrol || parentMission == Mission::Hunt)
						candidateMission = parentMission;
				}

				if (candidateMission == Mission::None && this->LastValidParentMission != Mission::None && this->LastValidParentMission != Mission::Attack)
					candidateMission = this->LastValidParentMission;

				if (candidateMission == Mission::None && pDestToInherit)
					candidateMission = Mission::Move;

				if (candidateMission != Mission::None)
					detachmentMission = candidateMission;
			}

			pChild->QueueMission(detachmentMission, false);

			const bool isAirborneChild = pChild->WhatAmI() == AbstractType::Aircraft
				|| pChild->GetTechnoType()->ConsideredAircraft
				|| pChild->GetTechnoType()->JumpJet;

			CellClass* pCell = MapClass::Instance.GetCellAt(pChild->Location);
			if (!pCell)
				pCell = pChild->GetCell();

			const CoordStruct groundLoc = pCell ? pCell->GetCoordsWithBridge() : CoordStruct::Empty;
			const bool isParentAirborne = (this->Parent && (this->Parent->IsInAir() || this->Parent->Location.Z > groundLoc.Z + 64 || this->Parent->GetHeight() > 0));
			const bool isChildElevated = pChild->Location.Z > groundLoc.Z + 64 || pChild->GetHeight() > 0 || isParentAirborne;

			if (isChildElevated && !isAirborneChild)
			{
				if (auto const pChildExt = TechnoExt::ExtMap.Find(pChild))
				{
					pChildExt->FallingInheritedTarget = pTargetToInherit;
					pChildExt->FallingInheritedDestination = pDestToInherit;
					pChildExt->FallingInheritedMission = detachmentMission;
					pChildExt->OnParachuted = true;
				}

				pChild->DropAsBomb();
			}
			else
			{
				CoordStruct curLoc = pChild->Location;
				if (CellClass* pDestCell = MapClass::Instance.GetCellAt(curLoc))
				{
					if (!pChild->IsInAir())
						pChild->SetLocation(pDestCell->GetCoordsWithBridge());
				}

				pChild->ForceMission(detachmentMission);
			}
		}

		if (pType->InheritOwner)
		{
			HouseClass* targetOwner = this->ChildOriginalOwner ? this->ChildOriginalOwner : (this->Parent ? this->Parent->GetOriginalOwner() : nullptr);
			if (targetOwner)
				pChild->SetOwningHouse(targetOwner, false);
		}

		if (pType->InheritStateEffects)
		{
			if (pChild->CloakState != CloakState::Uncloaked && !pChild->GetTechnoType()->Cloakable)
				pChild->Uncloak(false);

			pChild->ForceShielded = false;
		}

		this->DetachChildCore();

		return true;
	}

	return false;
}


void AttachmentClass::AttachChildCore(TechnoClass* pChild)
{
	this->Child = pChild;
	this->ChildOriginalOwner = pChild ? pChild->Owner : nullptr;
	TechnoExt::ExtMap.Find(pChild)->ParentAttachment = this;
}

void AttachmentClass::DetachChildCore()
{
	if (this->Child)
	{
		TechnoExt::ExtMap.Find(this->Child)->ParentAttachment = nullptr;
		this->Child = nullptr;
		this->ChildOriginalOwner = nullptr;
	}
}

void AttachmentClass::InvalidatePointer(void* ptr)
{
	AnnounceInvalidPointer(this->Parent, ptr);
	AnnounceInvalidPointer(this->Child, ptr);
	AnnounceInvalidPointer(this->ChildOriginalOwner, ptr);
	AnnounceInvalidPointer(this->LastValidParentTarget, ptr);
	AnnounceInvalidPointer(this->LastValidParentDestination, ptr);
}

#pragma region Save/Load

template <typename T>
bool AttachmentClass::Serialize(T& stm)
{
	return stm
		.Process(this->Data)
		.Process(this->Parent)
		.Process(this->Child)
		.Process(this->ChildOriginalOwner)
		.Process(this->RespawnTimer)
		.Process(this->LastValidParentTarget)
		.Process(this->LastValidParentDestination)
		.Process(this->LastValidParentMission)
		.Success();
}

bool AttachmentClass::Load(PhobosStreamReader& stm, bool RegisterForChange)
{
	return Serialize(stm);
}

bool AttachmentClass::Save(PhobosStreamWriter& stm) const
{
	return const_cast<AttachmentClass*>(this)->Serialize(stm);
}

#pragma endregion
