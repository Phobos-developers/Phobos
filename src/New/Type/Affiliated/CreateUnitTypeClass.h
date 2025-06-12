#pragma once

#include <Utilities/Template.h>
#include <Utilities/Enum.h>

class CreateUnitTypeClass
{
public:
	Valueable<TechnoTypeClass*> Type { nullptr };
	Valueable<DirType> Facing { DirType::North };
	Valueable<bool> InheritDeathFacings { false };
	Valueable<bool> InheritTurretFacings { false };
	Valueable<bool> RandomFacing { true };
	Valueable<bool> RemapAnim { false };
	Valueable<Mission> UnitMission { Mission::Guard };
	Nullable<Mission> AIUnitMission {};
	Valueable<OwnerHouseKind> Owner { OwnerHouseKind::Victim };
	Valueable<bool> RequireOwner { false };
	Valueable<bool> AlwaysSpawnOnGround { false };
	Valueable<bool> SpawnParachutedInAir { false };
	Valueable<bool> ConsiderPathfinding { false };
	Valueable<AnimTypeClass*> SpawnAnim { nullptr };
	Valueable<int> SpawnHeight { -1 };

	CreateUnitTypeClass() = default;

	void LoadFromINI(CCINIClass* pINI, const char* pSection);
	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

private:

	template <typename T>
	bool Serialize(T& stm);
};
