#include "AttachmentTypeClass.h"

#include <Utilities/TemplateDef.h>

Enumerable<AttachmentTypeClass>::container_t Enumerable<AttachmentTypeClass>::Array;

const char* Enumerable<AttachmentTypeClass>::GetMainSection()
{
	return "AttachmentTypes";
}

void AttachmentTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* section = this->Name;

	INI_EX exINI(pINI);

	this->RestoreAtCreation.Read(exINI, section, "RestoreAtCreation");
	this->InheritTilt.Read(exINI, section, "InheritTilt");
	this->InheritCommands.Read(exINI, section, "InheritCommands");
	this->InheritOwner.Read(exINI, section, "InheritOwner");
	this->InheritStateEffects.Read(exINI, section, "InheritStateEffects");

	this->SyncDamage.Read(exINI, section, "SyncDamage");
	this->SyncDamage_IsRelative.Read(exINI, section, "SyncDamage.IsRelative");
	this->SyncExperienceGain.Read(exINI, section, "SyncExperienceGain");
	this->SyncExperienceGain_IsRelative.Read(exINI, section, "SyncExperienceGain.IsRelative");

	this->CanBeForceDetached.Read(exINI, section, "CanBeForceDetached");
	this->RestoreAtHealth.Read(exINI, section, "RestoreAtHealth");

	this->ForceDetachWeapon_Child.Read(exINI, section, "ForceDetachWeapon.Child");
	this->ForceDetachWeapon_Parent.Read(exINI, section, "ForceDetachWeapon.Parent");
	this->DestructionWeapon_Child.Read(exINI, section, "DestructionWeapon.Child");
	this->DestructionWeapon_Parent.Read(exINI, section, "DestructionWeapon.Parent");

	this->ParentDestructionMission.Read(exINI, section, "ParentDestructionMission");
	this->ParentDetachmentMission.Read(exINI, section, "ParentDetachmentMission");
}

template <typename T>
void AttachmentTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->RestoreAtCreation)
		.Process(this->InheritTilt)
		.Process(this->InheritCommands)
		.Process(this->InheritOwner)
		.Process(this->InheritStateEffects)
		.Process(this->SyncDamage)
		.Process(this->SyncDamage_IsRelative)
		.Process(this->SyncExperienceGain)
		.Process(this->SyncExperienceGain_IsRelative)
		.Process(this->CanBeForceDetached)
		.Process(this->RestoreAtHealth)
		.Process(this->ForceDetachWeapon_Child)
		.Process(this->ForceDetachWeapon_Parent)
		.Process(this->DestructionWeapon_Child)
		.Process(this->DestructionWeapon_Parent)
		.Process(this->ParentDestructionMission)
		.Process(this->ParentDetachmentMission)
		;
}

void AttachmentTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void AttachmentTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}
