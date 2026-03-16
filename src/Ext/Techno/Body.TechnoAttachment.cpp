#include "Body.h"

#include <TechnoClass.h>

#include <cassert>

// Attaches this techno in a first available attachment "slot".
// Returns true if the attachment is successful.
bool TechnoExt::AttachTo(TechnoClass* pThis, TechnoClass* pParent)
{
	auto const pParentExt = TechnoExt::ExtMap.Find(pParent);

	for (auto const& pAttachment : pParentExt->ChildAttachments)
	{
		if (pAttachment->AttachChild(pThis))
			return true;
	}

	return false;
}

bool TechnoExt::DetachFromParent(TechnoClass* pThis)
{
	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	return pExt->ParentAttachment->DetachChild();
}

void TechnoExt::InitializeAttachments(TechnoClass* pThis)
{
	if (TechnoExt::DeployTransferSource)
		return;  // we handle that as part of the "conversion"

	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	auto const pType = pThis->GetTechnoType();
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	for (auto& entry : pTypeExt->AttachmentData)
		pExt->ChildAttachments.emplace_back(std::make_unique<AttachmentClass>(&entry, pThis, nullptr))->Initialize();
}

void TechnoExt::DestroyAttachments(TechnoClass* pThis, TechnoClass* pSource)
{
	// During deploy transfer the source object goes through Remove_This -> KillCargo after
	// attachments were moved. The vector is empty so this is normally a no-op, but guard for safety.
	if (TechnoExt::DeployTransferSource == pThis)
		return;

	auto const& pExt = TechnoExt::ExtMap.Find(pThis);

	for (auto const& pAttachment : pExt->ChildAttachments)
		pAttachment->Destroy(pSource);

	// TODO I am not sure, without clearing the attachments it sometimes crashes under
	// weird circumstances, like if the techno exists but the parent attachment isn't,
	// in particular in can enter cell hook, this may be a bandaid fix for something
	// way worse like improper occupation clearance or whatever - Kerbiter
	pExt->ChildAttachments.clear();
}

void TechnoExt::HandleDestructionAsChild(TechnoClass* pThis)
{
	// During deploy transfer the source goes through Remove_This which would notify the parent
	// that the child was destroyed. The source is being replaced, not destroyed.
	if (TechnoExt::DeployTransferSource == pThis)
		return;

	auto const& pExt = TechnoExt::ExtMap.Find(pThis);

	if (pExt->ParentAttachment)
		pExt->ParentAttachment->ChildDestroyed();
}

void TechnoExt::UnlimboAttachments(TechnoClass* pThis)
{
	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	for (auto const& pAttachment : pExt->ChildAttachments)
		pAttachment->Unlimbo();
}

void TechnoExt::LimboAttachments(TechnoClass* pThis)
{
	// During deploy transfer the source object is Limbo'd before the transfer hook fires
	// (building->unit direction). Skip limbo-ing children - they will be moved to the new object.
	if (TechnoExt::DeployTransferSource == pThis)
		return;

	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	for (auto const& pAttachment : pExt->ChildAttachments)
		pAttachment->Limbo();
}

void TechnoExt::TransferAttachments(TechnoClass* pThis, TechnoClass* pThat)
{
	auto const pThisExt = TechnoExt::ExtMap.Find(pThis);
	auto const pThatExt = TechnoExt::ExtMap.Find(pThat);

	for (auto& pAttachment : pThisExt->ChildAttachments)
	{
		pAttachment->Parent = pThat;
		pThatExt->ChildAttachments.push_back(std::move(pAttachment));
	}

	pThisExt->ChildAttachments.clear();
}

void TechnoExt::HandleAttachmentConversion(TechnoClass* pThis, TechnoTypeClass* pOldType, TechnoTypeClass* pNewType)
{
	auto const pThisExt = TechnoExt::ExtMap.Find(pThis);
	auto const pNewTypeExt = TechnoTypeExt::ExtMap.Find(pNewType);

	const int oldTypeIndex = TechnoTypeClass::Array.FindItemIndex(pOldType);
	const int newTypeIndex = TechnoTypeClass::Array.FindItemIndex(pNewType);

	// Helper to resolve a NullableIdx<TechnoTypeClass> to a TechnoTypeClass pointer
	auto resolveChildType = [](const NullableIdx<TechnoTypeClass>& idx) -> TechnoTypeClass*
	{
		return idx.isset() ? TechnoTypeClass::Array[idx] : nullptr;
	};

	// Step 1: Store current (old type) mount points as dormant (without limboing yet)
	// We preserve old attachments as is so we can then restore them as is
	pThisExt->DormantAttachments[oldTypeIndex] = std::move(pThisExt->ChildAttachments);

	// Step 2: Establish new mount points - restore from dormant or create fresh
	if (auto node = pThisExt->DormantAttachments.extract(newTypeIndex))
	{
		pThisExt->ChildAttachments = std::move(node.mapped());
	}
	else
	{
		for (auto& entry : pNewTypeExt->AttachmentData)
			pThisExt->ChildAttachments.emplace_back(std::make_unique<AttachmentClass>(&entry, pThis, nullptr))->Initialize();
	}

	// Step 3: Match old mount points to new active ones by ID; transfer children and synchronize timers.
	// Assumes each attachment ID is unique per TechnoType, which is enforced on parsing.
	auto& oldMounts = pThisExt->DormantAttachments[oldTypeIndex];

	// Shallow copy of old mount pointers for consumed-entry tracking; originals remain in DormantAttachments.
	std::vector<AttachmentClass*> oldMountsCopy;
	std::ranges::transform(oldMounts, std::back_inserter(oldMountsCopy), [](const auto& p) { return p.get(); });

	for (auto& pNewMount : pThisExt->ChildAttachments)
	{
		const auto& newID = pNewMount->Data->ID;
		if (!newID)
			continue;

		for (auto it = oldMountsCopy.begin(); it != oldMountsCopy.end(); ++it)
		{
			const auto& oldID = (*it)->Data->ID;
			if (!oldID || _strcmpi(oldID, newID) != 0)
				continue;

			// Transfer child techno if present
			assert(!pNewMount->Child && "ID-matched new attachment mount already has a child before conversion illegally!");
			if (TechnoClass* pChild = (*it)->Child)
			{
				auto* oldChildType = resolveChildType((*it)->Data->TechnoType);
				auto* newChildType = resolveChildType(pNewMount->Data->TechnoType);

				(*it)->DetachChildCore();

				bool childMatchesType = pChild->GetTechnoType() == oldChildType;
				bool typesDiffer = oldChildType != newChildType;
				if (childMatchesType && typesDiffer && newChildType)
				{
					if (auto* pChildAsFoot = abstract_cast<FootClass*>(pChild))
						TechnoExt::ConvertToType(pChildAsFoot, newChildType);
				}

				pNewMount->AttachChildCore(pChild);
			}

			// Synchronize respawn timer if both attachment types have respawn enabled.
			// Preserves the completion percentage: newRemaining/newDelay == oldRemaining/oldDelay.
			int oldDelay = (*it)->GetType()->RespawnDelay;
			int newDelay = pNewMount->GetType()->RespawnDelay;
			if (oldDelay > 0 && newDelay > 0 && (*it)->RespawnTimer.HasStarted())
			{
				int oldRemaining = (*it)->RespawnTimer.GetTimeLeft();
				int newRemaining = (oldRemaining * newDelay) / oldDelay;
				pNewMount->RespawnTimer.TimeLeft = newDelay;
				pNewMount->RespawnTimer.StartTime = static_cast<int>(Unsorted::CurrentFrame) - (newDelay - newRemaining);
			}

			oldMountsCopy.erase(it);
			break;
		}
	}

	// Step 4: Limbo all old mount points (matched ones have no child, so Limbo is a no-op for them);
	// unlimbo the new active ones
	for (auto& pOldMount : oldMounts)
		pOldMount->Limbo();

	if (pThis->InLimbo)
		return;  // if parent is in limbo, leave new attachments in limbo as well and skip unlimboing

	for (auto& pNewMount : pThisExt->ChildAttachments)
		pNewMount->Unlimbo();
}

void TechnoExt::HandleAttachmentDeployTransfer(TechnoClass* pFrom, TechnoClass* pTo)
{
	auto const pFromExt = TechnoExt::ExtMap.Find(pFrom);
	auto const pToExt = TechnoExt::ExtMap.Find(pTo);

	// The flag is consumed here - clear it now that the transfer is happening.
	TechnoExt::DeployTransferSource = nullptr;
	assert(pToExt->ChildAttachments.empty() && "pTo should have no mounts before deploy transfer");

	// Move pFrom's active and dormant attachments into pTo so they live on the surviving object.
	pToExt->ChildAttachments = std::move(pFromExt->ChildAttachments);
	pToExt->DormantAttachments = std::move(pFromExt->DormantAttachments);

	// Re-parent all active mounts to point at the new parent techno.
	for (auto& pAttachment : pToExt->ChildAttachments)
		pAttachment->Parent = pTo;

	// Re-parent dormant mounts as well, since they may be restored on a future conversion.
	for (auto& [typeIdx, mounts] : pToExt->DormantAttachments)
	{
		for (auto& pAttachment : mounts)
			pAttachment->Parent = pTo;
	}

	// Now handle conversion from pFrom's type to pTo's type on the new object.
	HandleAttachmentConversion(pTo, pFrom->GetTechnoType(), pTo->GetTechnoType());
}

bool TechnoExt::IsAttached(TechnoClass* pThis)
{
	auto const& pExt = TechnoExt::ExtMap.Find(pThis);
	return pExt && pExt->ParentAttachment;
}

bool TechnoExt::HasAttachmentLoco(FootClass* pThis)
{
	IPersistPtr pPersist = pThis->Locomotor;
	CLSID locoCLSID {};
	return pPersist && SUCCEEDED(pPersist->GetClassID(&locoCLSID))
		&& locoCLSID == __uuidof(AttachmentLocomotionClass);
}

bool TechnoExt::DoesntOccupyCellAsChild(TechnoClass* pThis)
{
	auto const& pExt = TechnoExt::ExtMap.Find(pThis);
	return pExt && pExt->ParentAttachment
		&& !pExt->ParentAttachment->GetType()->OccupiesCell;
}

bool TechnoExt::IsChildOf(TechnoClass* pThis, TechnoClass* pParent, bool deep)
{
	auto const pThisExt = TechnoExt::ExtMap.Find(pThis);

	return pThis && pThisExt && pParent  // sanity check, sometimes crashes because ext is null - Kerbiter
		&& pThisExt->ParentAttachment
		&& (pThisExt->ParentAttachment->Parent == pParent
			|| (deep && TechnoExt::IsChildOf(pThisExt->ParentAttachment->Parent, pParent)));
}

bool TechnoExt::AreRelatives(TechnoClass* pThis, TechnoClass* pThat)
{
	return TechnoExt::GetTopLevelParent(pThis)
		== TechnoExt::GetTopLevelParent(pThat);
}

// Returns this if no parent.
TechnoClass* TechnoExt::GetTopLevelParent(TechnoClass* pThis)
{
	auto const pThisExt = TechnoExt::ExtMap.Find(pThis);

	return pThis && pThisExt  // sanity check, sometimes crashes because ext is null - Kerbiter
		&& pThisExt->ParentAttachment
		? TechnoExt::GetTopLevelParent(pThisExt->ParentAttachment->Parent)
		: pThis;
}
