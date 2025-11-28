#include "Body.h"
#include <Utilities/AresHelper.h>
#include <Ext/TechnoType/Body.h>

// Bugfix: TAction 7,80,107.
DEFINE_HOOK(0x65DF67, TeamTypeClass_CreateMembers_LoadOntoTransport, 0x6)
{
	if(!AresHelper::CanUseAres) // If you're not using Ares you don't deserve a fix
		return 0;

	GET(FootClass* const, pPayload, EAX);
	GET(FootClass* const, pTransport, ESI);
	GET(TeamClass* const, pTeam, EBP);
	GET(TeamTypeClass const*, pThis, EBX);

	auto unmarkPayloadCreated = [](FootClass* member){reinterpret_cast<char*>(member->align_154)[0x9E] = false;};

	if (!pTransport)
	{
		for (auto pNext = pPayload;
		pNext && pNext != pTransport && pNext->Team == pTeam;
		pNext = abstract_cast<FootClass*>(pNext->NextObject))
			unmarkPayloadCreated(pNext);

		return 0x65DFE8;
	}

	unmarkPayloadCreated(pTransport);

	if (!pPayload || !pThis->Full)
		return 0x65E004;

	const auto pType = pTransport->GetTechnoType();
	const bool isTransportOpenTopped = pType->OpenTopped;
	FootClass* pGunner = nullptr;

	for (auto pNext = pPayload;
		pNext && pNext != pTransport && pNext->Team == pTeam;
		pNext = abstract_cast<FootClass*>(pNext->NextObject))
	{
		pPayload->Transporter = pTransport;
		pGunner = pNext;

		if (isTransportOpenTopped)
			pTransport->EnteredOpenTopped(pNext);
	}

	// Add to transport - this will load the payload object and everything linked to it (rest of the team) in reverse order
	pTransport->Passengers.AddPassenger(pPayload);

	// Handle gunner change - this is the 'last' passenger because of reverse order
	if (pType->Gunner && pGunner)
		pTransport->ReceiveGunner(pGunner);

	return 0x65DF8D;
}

DEFINE_HOOK(0x6EA6BE, TeamClass_CanAddMember_Consideration, 0x6)
{
	enum { SkipGameCode = 0x6EA6F2 };

	GET(TeamClass*, pTeam, EBP);
	GET(FootClass*, pFoot, ESI);
	GET(int*, idx, EBX);
	const auto pFootType = pFoot->GetTechnoType();
	const auto pFootTypeExt = TechnoTypeExt::ExtMap.Find(pFootType);
	const auto pTaskForce = pTeam->Type->TaskForce;

	do
	{
		const auto pType = pTaskForce->Entries[*idx].Type;

		if (pType == pFootType || pFootTypeExt->TeamMember_ConsideredAs.Contains(pType))
			break;

		*idx = *idx + 1;
	}
	while (pTaskForce->CountEntries > *idx);

	return SkipGameCode;
}

DEFINE_HOOK(0x6EA8E7, TeamClass_LiberateMember_Consideration, 0x5)
{
	enum { SkipGameCode = 0x6EA91B };

	GET(TeamClass*, pTeam, EDI);
	GET(FootClass*, pMember, EBP);
	int idx = 0;
	const auto pMemberType = pMember->GetTechnoType();
	const auto pMemberTypeExt = TechnoTypeExt::ExtMap.Find(pMemberType);
	const auto pTaskForce = pTeam->Type->TaskForce;

	do
	{
		const auto pSearchType = pTaskForce->Entries[idx].Type;

		if (pSearchType == pMemberType || pMemberTypeExt->TeamMember_ConsideredAs.Contains(pSearchType))
			break;

		++idx;
	}
	while (pTaskForce->CountEntries > idx);

	R->Stack(STACK_OFFSET(0x14, 0x8), idx);
	return SkipGameCode;
}

DEFINE_HOOK(0x6EAD73, TeamClass_Sub_6EAA90_Consideration, 0x7)
{
	enum { ContinueCheck = 0x6EAD8F, SkipThisMember = 0x6EADB3 };

	GET(TeamClass*, pTeam, ECX);
	GET(UnitClass*, pMember, ESI);
	GET_STACK(int, idx, STACK_OFFSET(0x3C, 0x4));
	const auto pMemberType = pMember->Type;
	const auto pTaskForce = pTeam->Type->TaskForce;
	const auto pSearchType = pTaskForce->Entries[idx].Type;

	return pSearchType == pMemberType || TechnoTypeExt::ExtMap.Find(pMemberType)->TeamMember_ConsideredAs.Contains(pSearchType) ? ContinueCheck : SkipThisMember;
}

DEFINE_HOOK(0x6EF57F, TeamClass_GetTaskForceMissingMemberTypes_Consideration, 0x5)
{
	enum { ContinueIn = 0x6EF584, SkipThisMember = 0x6EF5A5 };

	GET(int, idx, EAX);

	if (idx != -1)
		return ContinueIn;

	GET(DynamicVectorClass<TechnoTypeClass*>*, vector, ESI);
	GET(FootClass*, pMember, EDI);
	const auto pMemberTypeExt = TechnoTypeExt::ExtMap.Find(pMember->GetTechnoType());

	for (const auto pConsideType : pMemberTypeExt->TeamMember_ConsideredAs)
	{
		idx = vector->FindItemIndex(pConsideType);

		if (idx != -1)
		{
			R->EAX(idx);
			return ContinueIn;
		}
	}

	return SkipThisMember;
}
