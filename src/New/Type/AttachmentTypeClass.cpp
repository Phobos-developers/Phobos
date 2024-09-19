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

	this->RespawnAtCreation.Read(exINI, section, "RespawnAtCreation");
	this->RespawnDelay.Read(exINI, section, "RespawnDelay");
	this->InheritCommands.Read(exINI, section, "InheritCommands");
	this->InheritCommands_StopCommand.Read(exINI, section, "InheritCommands.StopCommand");
	this->InheritCommands_DeployCommand.Read(exINI, section, "InheritCommands.DeployCommand");
	this->InheritOwner.Read(exINI, section, "InheritOwner");
	this->InheritStateEffects.Read(exINI, section, "InheritStateEffects");
	this->InheritDestruction.Read(exINI, section, "InheritDestruction");
	this->InheritHeightStatus.Read(exINI, section, "InheritHeightStatus");
	this->OccupiesCell.Read(exINI, section, "OccupiesCell");
	this->LowSelectionPriority.Read(exINI, section, "LowSelectionPriority");
	this->TransparentToMouse.Read(exINI, section, "TransparentToMouse");
	this->YSortPosition.Read(exINI, section, "YSortPosition");
	this->DestructionWeapon_Child.Read(exINI, section, "DestructionWeapon.Child");
	this->DestructionWeapon_Parent.Read(exINI, section, "DestructionWeapon.Parent");
	this->ParentDestructionMission.Read(exINI, section, "ParentDestructionMission");
	this->ParentDetachmentMission.Read(exINI, section, "ParentDetachmentMission");
}

template <typename T>
void AttachmentTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->RespawnAtCreation)
		.Process(this->RespawnDelay)
		.Process(this->InheritCommands)
		.Process(this->InheritCommands_StopCommand)
		.Process(this->InheritCommands_DeployCommand)
		.Process(this->InheritOwner)
		.Process(this->InheritStateEffects)
		.Process(this->InheritDestruction)
		.Process(this->InheritHeightStatus)
		.Process(this->OccupiesCell)
		.Process(this->LowSelectionPriority)
		.Process(this->TransparentToMouse)
		.Process(this->YSortPosition)
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
