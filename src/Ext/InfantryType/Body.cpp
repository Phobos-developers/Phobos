#include "Body.h"

InfantryTypeExt::ExtContainer InfantryTypeExt::ExtMap;

// =============================
// load / save

void InfantryTypeExt::LoadFromINIFile(CCINIClass* const pINI)
{
	TechnoTypeExt::LoadFromINIFile(pINI);

	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;
	INI_EX exINI(pINI);

	this->Slaved_OwnerWhenMasterKilled.Read(exINI, pSection, "Slaved.OwnerWhenMasterKilled");
	this->SlavesFreeSound.Read(exINI, pSection, "SlavesFreeSound");
	this->NotHuman_RandomDeathSequence.Read(exINI, pSection, "NotHuman.RandomDeathSequence");
	this->DefaultDisguise.Read(exINI, pSection, "DefaultDisguise");
	this->ProneSpeed.Read(exINI, pSection, "ProneSpeed");
	this->InfantryAutoDeploy.Read(exINI, pSection, "InfantryAutoDeploy");

	const auto pArtINI = &CCINIClass::INI_Art;
	INI_EX exArtINI(pArtINI);
	auto pArtSection = pThis->ImageFile;

	this->ParseBurstFLHs(exArtINI, pArtSection, this->DeployedWeaponBurstFLHs, this->EliteDeployedWeaponBurstFLHs, "Deployed");
	this->ParseBurstFLHs(exArtINI, pArtSection, this->CrouchedWeaponBurstFLHs, this->EliteCrouchedWeaponBurstFLHs, "Prone");

	this->OnlyUseLandSequences.Read(exArtINI, pArtSection, "OnlyUseLandSequences");
	this->SecondaryFireSequenceLandOnly.Read(exArtINI, pArtSection, "SecondaryFireSequenceLandOnly");
	this->PronePrimaryFireFLH.Read(exArtINI, pArtSection, "PronePrimaryFireFLH");
	this->ProneSecondaryFireFLH.Read(exArtINI, pArtSection, "ProneSecondaryFireFLH");
	this->DeployedPrimaryFireFLH.Read(exArtINI, pArtSection, "DeployedPrimaryFireFLH");
	this->DeployedSecondaryFireFLH.Read(exArtINI, pArtSection, "DeployedSecondaryFireFLH");
}

template <typename T>
void InfantryTypeExt::Serialize(T& Stm)
{
	Stm
		.Process(this->Slaved_OwnerWhenMasterKilled)
		.Process(this->SlavesFreeSound)
		.Process(this->NotHuman_RandomDeathSequence)
		.Process(this->DefaultDisguise)
		.Process(this->ProneSpeed)
		.Process(this->OnlyUseLandSequences)
		.Process(this->SecondaryFireSequenceLandOnly)
		.Process(this->PronePrimaryFireFLH)
		.Process(this->ProneSecondaryFireFLH)
		.Process(this->DeployedPrimaryFireFLH)
		.Process(this->DeployedSecondaryFireFLH)
		.Process(this->CrouchedWeaponBurstFLHs)
		.Process(this->EliteCrouchedWeaponBurstFLHs)
		.Process(this->DeployedWeaponBurstFLHs)
		.Process(this->EliteDeployedWeaponBurstFLHs)
		.Process(this->InfantryAutoDeploy)
		;
}

void InfantryTypeExt::LoadFromStream(PhobosStreamReader& Stm)
{
	TechnoTypeExt::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void InfantryTypeExt::SaveToStream(PhobosStreamWriter& Stm)
{
	TechnoTypeExt::SaveToStream(Stm);
	this->Serialize(Stm);
}

// =============================
// container

InfantryTypeExt::ExtContainer::ExtContainer() : Container("InfantryTypeClass") { }
InfantryTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x5236B3, InfantryTypeClass_CTOR, 0xA)
{
	GET(InfantryTypeClass*, pItem, ESI);

	InfantryTypeExt::ExtMap.Allocate(pItem);

	return 0;
}

// The extension chain is read at the end of each concrete type class's LoadFromINI,
// once every native field - inherited and own alike - has been parsed.
DEFINE_HOOK(0x52473F, InfantryTypeClass_LoadFromINI, 0x5)
{
	GET(InfantryTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0xD0);

	if (auto const pExt = InfantryTypeExt::TryFetch(pItem))
		pExt->LoadFromINI(pINI);

	return 0;
}

// Late in every destructor body of the class, right before it chains into the
// base destructor: the last point where the extension is no longer used.
DEFINE_HOOK_AGAIN(0x524E90, InfantryTypeClass_DTOR, 0xE)
DEFINE_HOOK(0x523AF0, InfantryTypeClass_DTOR, 0xE)
{
	GET(InfantryTypeClass*, pItem, ESI);

	InfantryTypeExt::ExtMap.Remove(pItem);

	return 0;
}
