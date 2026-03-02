#include "Body.h"

#include <TechnoClass.h>

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
	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	auto const pType = pThis->GetTechnoType();
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	for (auto& entry : pTypeExt->AttachmentData)
	{
		pExt->ChildAttachments.push_back(std::make_unique<AttachmentClass>(&entry, pThis, nullptr));
		pExt->ChildAttachments.back()->Initialize();
	}
}

void TechnoExt::DestroyAttachments(TechnoClass* pThis, TechnoClass* pSource)
{
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
	pThisExt->DormantAttachments[oldTypeIndex] = std::move(pThisExt->ChildAttachments);
	pThisExt->ChildAttachments.clear();

	// Step 2: Establish new mount points - restore from dormant or create fresh
	auto dormantIt = pThisExt->DormantAttachments.find(newTypeIndex);
	if (dormantIt != pThisExt->DormantAttachments.end())
	{
		pThisExt->ChildAttachments = std::move(dormantIt->second);
		pThisExt->DormantAttachments.erase(dormantIt);
	}
	else
	{
		for (auto& entry : pNewTypeExt->AttachmentData)
			pThisExt->ChildAttachments.emplace_back(std::make_unique<AttachmentClass>(&entry, pThis, nullptr))->Initialize();
	}

	// Step 3: Match old mount points to new active ones by ID and transfer children
	auto& oldMounts = pThisExt->DormantAttachments[oldTypeIndex];

	// this assumes we only encounter each attachment ID once, which is currently enforced on parsing
	for (auto& pNewMount : pThisExt->ChildAttachments)
	{
		const auto& newID = pNewMount->Data->ID;
		if (!newID)
			continue;

		for (auto& pOldMount : oldMounts)
		{
			if (!pOldMount->Child)
				continue;  // nothing to transfer from this mount point, skip

			const auto& oldID = pOldMount->Data->ID;
			if (!oldID || _strcmpi(oldID, newID) != 0)
				continue;  // IDs don't match or no ID, skip

			// Found a match - transfer child techno from old mount point to new
			TechnoClass* pChild = pOldMount->Child;
			if (pChild)
			{
				auto* oldChildType = resolveChildType(pOldMount->Data->TechnoType);
				auto* newChildType = resolveChildType(pNewMount->Data->TechnoType);

				pOldMount->DetachChildCore();

				bool childMatchesType = pChild->GetTechnoType() == oldChildType;
				bool typesDiffer = oldChildType != newChildType;
				if (childMatchesType && typesDiffer && newChildType)
				{
					if (auto* pChildAsFoot = abstract_cast<FootClass*>(pChild))
						TechnoExt::ConvertToType(pChildAsFoot, newChildType);
				}

				pNewMount->AttachChildCore(pChild);
			}

			pOldMount = nullptr;
			break;
		}
	}

	// Step 4: Limbo all old mount points and unlimbo new ones without children
	for (auto& pOldMount : oldMounts)
		pOldMount->Limbo();

	if (pThis->InLimbo)
		return;  // if parent is in limbo, leave new attachments in limbo as well and skip unlimboing

	for (auto& pNewMount : pThisExt->ChildAttachments)
		pNewMount->Unlimbo();
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
