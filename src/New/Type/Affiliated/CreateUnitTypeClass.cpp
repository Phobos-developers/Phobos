#include "CreateUnitTypeClass.h"

#include <Ext/Techno/Body.h>

void CreateUnitTypeClass::LoadFromINI(CCINIClass* pINI, const char* pSection)
{
	INI_EX exINI(pINI);

	this->Type.Read(exINI, pSection, "CreateUnit");
	this->Facing.Read(exINI, pSection, "CreateUnit.Facing");
	this->InheritDeathFacings.Read(exINI, pSection, "CreateUnit.InheritFacings");
	this->InheritTurretFacings.Read(exINI, pSection, "CreateUnit.InheritTurretFacings");
	this->RemapAnim.Read(exINI, pSection, "CreateUnit.RemapAnim");
	this->UnitMission.Read(exINI, pSection, "CreateUnit.Mission");
	this->AIUnitMission.Read(exINI, pSection, "CreateUnit.AIMission");
	this->Owner.Read(exINI, pSection, "CreateUnit.Owner");
	this->RequireOwner.Read(exINI, pSection, "CreateUnit.RequireOwner");
	this->RandomFacing.Read(exINI, pSection, "CreateUnit.RandomFacing");
	this->AlwaysSpawnOnGround.Read(exINI, pSection, "CreateUnit.AlwaysSpawnOnGround");
	this->SpawnParachutedInAir.Read(exINI, pSection, "CreateUnit.SpawnParachutedInAir");
	this->ConsiderPathfinding.Read(exINI, pSection, "CreateUnit.ConsiderPathfinding");
	this->SpawnAnim.Read(exINI, pSection, "CreateUnit.SpawnAnim");
	this->SpawnHeight.Read(exINI, pSection, "CreateUnit.SpawnHeight");
}

#pragma region(save/load)

template <class T>
bool CreateUnitTypeClass::Serialize(T& stm)
{
	return stm
		.Process(this->Type)
		.Process(this->Facing)
		.Process(this->InheritDeathFacings)
		.Process(this->RemapAnim)
		.Process(this->UnitMission)
		.Process(this->AIUnitMission)
		.Process(this->InheritTurretFacings)
		.Process(this->Owner)
		.Process(this->RequireOwner)
		.Process(this->RandomFacing)
		.Process(this->AlwaysSpawnOnGround)
		.Process(this->SpawnParachutedInAir)
		.Process(this->ConsiderPathfinding)
		.Process(this->SpawnAnim)
		.Process(this->SpawnHeight)
		.Success();
}

bool CreateUnitTypeClass::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return this->Serialize(stm);
}

bool CreateUnitTypeClass::Save(PhobosStreamWriter& stm) const
{
	return const_cast<CreateUnitTypeClass*>(this)->Serialize(stm);
}

#pragma endregion(save/load)
