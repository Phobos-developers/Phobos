#include "Body.h"

TeamTypeExt::ExtContainer TeamTypeExt::ExtMap;

// =============================
// load / save

void TeamTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;
	INI_EX exINI(pINI);

	this->SetRecruitableOnLiberate.Read(exINI, pSection, "SetRecruitableOnLiberate");
}

template <typename T>
void TeamTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->SetRecruitableOnLiberate)
		;
}

void TeamTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<TeamTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void TeamTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<TeamTypeClass>::SaveToStream(Stm);
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

DEFINE_HOOK_AGAIN(0x6F1BB0, TeamTypeClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x6F1B90, TeamTypeClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(TeamTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	TeamTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x6F1C35, TeamTypeClass_Load_Suffix, 0x5)
{
	TeamTypeExt::ExtMap.LoadStatic();

	return 0;
}

DEFINE_HOOK(0x6F1BAA, TeamTypeClass_Save_Suffix, 0x5)
{
	TeamTypeExt::ExtMap.SaveStatic();

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
