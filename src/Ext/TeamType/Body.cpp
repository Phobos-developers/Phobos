#include "Body.h"

TeamTypeExt::ExtContainer TeamTypeExt::ExtMap;

// =============================
// load / save

void TeamTypeExt::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;
	INI_EX exINI(pINI);

	this->SetRecruitableOnLiberate.Read(exINI, pSection, "SetRecruitableOnLiberate");
}

template <typename T>
void TeamTypeExt::Serialize(T& Stm)
{
	Stm
		.Process(this->SetRecruitableOnLiberate)
		;
}

void TeamTypeExt::LoadFromStream(PhobosStreamReader& Stm)
{
	AbstractTypeExt::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void TeamTypeExt::SaveToStream(PhobosStreamWriter& Stm)
{
	AbstractTypeExt::SaveToStream(Stm);
	this->Serialize(Stm);
}

// =============================
// container

TeamTypeExt::ExtContainer::ExtContainer() : Container("TeamTypeClass") { }

TeamTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x6F08E6, TeamTypeClass_CTOR, 0x6)
{
	GET(TeamTypeClass*, pItem, EAX);

	TeamTypeExt::ExtMap.TryAllocate(pItem);

	return 0;
}

DEFINE_HOOK(0x6F20D0, TeamTypeClass_DTOR, 0x6)
{
	GET(TeamTypeClass*, pItem, ECX);

	TeamTypeExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x6F1535, TeamTypeClass_LoadFromINI, 0xA)
DEFINE_HOOK(0x6F1528, TeamTypeClass_LoadFromINI, 0xA)
{
	GET(TeamTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0xCC);

	TeamTypeExt::ExtMap.LoadFromINI(pItem, pINI);

	return 0;
}
