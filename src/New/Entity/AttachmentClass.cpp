#include "AttachmentClass.h"

#include <Dir.h>
#include <BulletClass.h>
#include <BulletTypeClass.h>
#include <WarheadTypeClass.h>

#include <ObjBase.h>

#include <Ext/Techno/Body.h>
#include <Locomotion/AttachmentLocomotionClass.h>

std::vector<AttachmentClass*> AttachmentClass::Array;

void AttachmentClass::InitCacheData()
{
	this->Cache.TopLevelParent = TechnoExt::GetTopLevelParent(this->Parent);
}

Matrix3D AttachmentClass::GetUpdatedTransform(VoxelIndexKey* pKey, bool isShadow)
{
	Matrix3D& transform = isShadow ? this->Cache.ChildShadowTransform : this->Cache.ChildTransform;
	int& lastUpdateFrame = isShadow ? this->Cache.ShadowLastUpdateFrame : this->Cache.LastUpdateFrame;

	if (Unsorted::CurrentFrame != lastUpdateFrame)
	{
		double& factor = *reinterpret_cast<double*>(0xB1D008);
		auto const flh = this->Data->FLH.Get() * factor;

		Matrix3D mtx = TechnoExt::GetAttachmentTransform(this->Parent, pKey, isShadow);
		mtx = TechnoExt::TransformFLHForTurret(this->Parent, mtx, this->Data->IsOnTurret, factor);
		mtx.Translate((float)flh.X, (float)flh.Y, (float)flh.Z);

		transform = mtx;

		lastUpdateFrame = Unsorted::CurrentFrame;
	}

	return transform;
}

AttachmentTypeClass* AttachmentClass::GetType()
{
	return AttachmentTypeClass::Array[this->Data->Type].get();
}

TechnoTypeClass* AttachmentClass::GetChildType()
{
	return this->Data->TechnoType.isset()
		? TechnoTypeClass::Array()->GetItem(this->Data->TechnoType)
		: nullptr;
}

Matrix3D AttachmentClass::GetChildTransformForLocation()
{
	auto const flh = this->Data->FLH.Get();

	auto const pParentExt = TechnoExt::ExtMap.Find(this->Parent);

	Matrix3D mtx;
	if (pParentExt && pParentExt->ParentAttachment)
		mtx = pParentExt->ParentAttachment->GetChildTransformForLocation();
	else
		mtx = TechnoExt::GetTransform(this->Parent);

	mtx = TechnoExt::TransformFLHForTurret(this->Parent, mtx, this->Data->IsOnTurret);
	mtx.Translate((float)flh.X, (float)flh.Y, (float)flh.Z);

	return mtx;
}

CoordStruct AttachmentClass::GetChildLocation()
{
	auto& flh = this->Data->FLH.Get();
	return TechnoExt::GetFLHAbsoluteCoords(this->Parent, flh, this->Data->IsOnTurret);

	/*
	// TODO it doesn't work correctly for some unexplicable reason
	auto result = this->GetChildTransformForLocation() * Vector3D<float>::Empty;

	// Resulting coords are mirrored along X axis, so we mirror it back
	result.Y *= -1;

	// apply as an offset to global object coords
	CoordStruct location = this->Cache.TopLevelParent->GetCoords();
	location += { std::lround(result.X), std::lround(result.Y), std::lround(result.Z) };

	return location;
	*/
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

void AttachmentClass::Initialize()
{
	if (this->Child)
		return;

	if (this->GetType()->RestoreAtCreation)
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

	if (this->Child)
	{
		this->Child->SetLocation(this->GetChildLocation());

		this->Child->OnBridge = this->Parent->OnBridge;

		DirStruct childDir = this->Data->IsOnTurret
			? this->Parent->SecondaryFacing.Current() : this->Parent->PrimaryFacing.Current();

		this->Child->PrimaryFacing.SetCurrent(childDir);
		// TODO handle secondary facing in case the turret is idle

		FootClass* pParentAsFoot = abstract_cast<FootClass*>(this->Parent);
		FootClass* pChildAsFoot = abstract_cast<FootClass*>(this->Child);
		if (pParentAsFoot && pChildAsFoot)
		{
			pChildAsFoot->TubeIndex = pParentAsFoot->TubeIndex;
		}

		if (pType->InheritStateEffects)
		{
			this->Child->CloakState = this->Parent->CloakState;
			this->Child->BeingWarpedOut = this->Parent->BeingWarpedOut;
			this->Child->Deactivated = this->Parent->Deactivated;
			this->Child->Flash(this->Parent->Flashing.DurationRemaining);

			this->Child->IronCurtainTimer = this->Parent->IronCurtainTimer;
			this->Child->IdleActionTimer = this->Parent->IdleActionTimer;
			this->Child->IronTintTimer = this->Parent->IronTintTimer;
			this->Child->CloakDelayTimer = this->Parent->CloakDelayTimer;
			this->Child->ChronoLockRemaining = this->Parent->ChronoLockRemaining;
			this->Child->Berzerk = this->Parent->Berzerk;
			this->Child->ChronoWarpedByHouse = this->Parent->ChronoWarpedByHouse;
			this->Child->EMPLockRemaining = this->Parent->EMPLockRemaining;
			this->Child->ShouldLoseTargetNow = this->Parent->ShouldLoseTargetNow;
		}

		if (pType->InheritOwner)
			this->Child->SetOwningHouse(this->Parent->GetOwningHouse(), false);
	}
}

// Called in Kill_Cargo, handles logics for parent destruction on children
void AttachmentClass::Destroy(TechnoClass* pSource)
{
	if (this->Child)
	{
		auto pChildExt = TechnoExt::ExtMap.Find(this->Child);
		pChildExt->ParentAttachment = nullptr;

		auto pType = this->GetType();

		if (pType->DestructionWeapon_Child.isset())
			TechnoExt::FireWeaponAtSelf(this->Child, pType->DestructionWeapon_Child);

		if (pType->InheritDestruction && this->Child)
			TechnoExt::Kill(this->Child, pSource);
		else if (!this->Child->InLimbo && pType->ParentDestructionMission.isset())
			this->Child->QueueMission(pType->ParentDestructionMission.Get(), false);

		this->Child = nullptr;
	}
}

void AttachmentClass::ChildDestroyed()
{
	AttachmentTypeClass* pType = this->GetType();
	if (pType->DestructionWeapon_Parent.isset())
		TechnoExt::FireWeaponAtSelf(this->Parent, pType->DestructionWeapon_Parent);

	this->Child = nullptr;
}

void AttachmentClass::Unlimbo()
{
	if (this->Child)
	{
		CoordStruct childCoord = TechnoExt::GetFLHAbsoluteCoords(
			this->Parent, this->Data->FLH, this->Data->IsOnTurret);

		DirType childDir = this->Data->IsOnTurret
			? this->Parent->SecondaryFacing.Current().GetDir() : this->Parent->PrimaryFacing.Current().GetDir();

		++Unsorted::IKnowWhatImDoing;
		this->Child->Unlimbo(childCoord, childDir);
		--Unsorted::IKnowWhatImDoing;
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

	this->Child = pChild;

	auto pChildExt = TechnoExt::ExtMap.Find(this->Child);
	pChildExt->ParentAttachment = this;

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

bool AttachmentClass::DetachChild(bool isForceDetachment)
{
	if (this->Child)
	{
		AttachmentTypeClass* pType = this->GetType();

		if (isForceDetachment)
		{
			// if (pType->ForceDetachWeapon_Parent.isset())
			// 	TechnoExt::FireWeaponAtSelf(this->Parent, pType->DestructionWeapon_Parent);

			// if (pType->ForceDetachWeapon_Child.isset())
			// 	TechnoExt::FireWeaponAtSelf(this->Child, pType->DestructionWeapon_Child);
		}

		if (!this->Child->InLimbo && pType->ParentDetachmentMission.isset())
			this->Child->QueueMission(pType->ParentDetachmentMission.Get(), false);

		// FIXME this won't work probably
		if (pType->InheritOwner)
			this->Child->SetOwningHouse(this->Parent->GetOriginalOwner(), false);

		// remove the attachment locomotor manually just to be safe
		if (auto const pChildAsFoot = abstract_cast<FootClass*>(this->Child))
			LocomotionClass::End_Piggyback(pChildAsFoot->Locomotor);

		auto pChildExt = TechnoExt::ExtMap.Find(this->Child);
		pChildExt->ParentAttachment = nullptr;
		this->Child = nullptr;

		return true;
	}

	return false;
}


void AttachmentClass::InvalidatePointer(void* ptr)
{
	AnnounceInvalidPointer(this->Parent, ptr);
	AnnounceInvalidPointer(this->Child, ptr);
	AnnounceInvalidPointer(this->Cache.TopLevelParent, ptr);
}

#pragma region Save/Load

template <typename T>
bool AttachmentClass::Serialize(T& stm)
{
	return stm
		.Process(this->Data)
		.Process(this->Parent)
		.Process(this->Child)
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
